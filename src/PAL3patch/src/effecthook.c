#include "common.h"

struct effecthook_t {
    char *eff_file;
    char *old_str;
    char *new_str;
};

static struct effecthook_t effhooks[MAX_EFFECTHOOKS];
static int nr_effhooks = 0;

// alloc memory and copy hook description to effhooks[]
// eff_file = "*" means hook apply to any effect
void add_effect_hook(const char *eff_file, const char *old_str, const char *new_str)
{
    if (nr_effhooks >= MAX_EFFECTHOOKS) fail("too many effect hooks.");
    struct effecthook_t *p = &effhooks[nr_effhooks++];
    p->eff_file = strdup(eff_file);
    p->old_str = strdup(old_str);
    p->new_str = strdup(new_str);
}

// apply a single hook, alloc a new buffer for return
static char *do_effhook_replace(struct effecthook_t *p, const char *eff)
{
    int i;
    
    // calc lengths
    int eff_len = strlen(eff);
    int old_len = strlen(p->old_str);
    int new_len = strlen(p->new_str);
    
    // calc new length for effects
    int eff_new_len = eff_len;
    i = 0;
    while (i < eff_len) {
        if (strnicmp(eff + i, p->old_str, old_len) == 0) {
            i += old_len;
            eff_new_len += new_len - old_len;
        } else {
            i++;
        }
    }
    
    // do replace
    int flag = 0;
    char *ret = malloc(eff_new_len + 1);
    char *new_eff = ret;
    i = 0;
    while (i < eff_len) {
        if (strnicmp(eff + i, p->old_str, old_len) == 0) {
            strcpy(new_eff, p->new_str);
            new_eff += new_len;
            i += old_len;
            flag = 1;
        } else {
            *new_eff++ = eff[i++];
        }
    }
    *new_eff = '\0';
    
    if (!flag && strcmp(p->eff_file, "*") != 0) {
        warning("no replace occured. (effhook: file='%s' old='%s' new='%s')", p->eff_file, p->old_str, p->new_str);
    }
    
    assert(strlen(ret) == (unsigned) eff_new_len);
    return ret;
}

// apply all hooks, alloc a new buffer for return
static char *run_all_effect_hooks(const char *fn, const char *eff)
{
    if (strrchr(fn, '\\')) fn = strrchr(fn, '\\') + 1;
    char *str = strdup(eff);
    int i;
    for (i = 0; i < nr_effhooks; i++) {
        if (strcmp(effhooks[i].eff_file, "*") == 0 || stricmp(effhooks[i].eff_file, fn) == 0) {
            char *new_str = do_effhook_replace(&effhooks[i], str);
            free(str);
            str = new_str;
        }
    }
    return str;
}

static MAKE_ASMPATCH(hook_D3DXCreateEffect)
{
    char *eff_filename = TOPTR(M_DWORD(R_ESP + 0x50)); // effect filename
    char *eff_filedata = TOPTR(M_DWORD(R_ESP + 0x4)); // effect content
    unsigned int eff_filelen = M_DWORD(R_ESP + 0x8);
    
    char *old_eff = malloc(eff_filelen + 1);
    memcpy(old_eff, eff_filedata, eff_filelen);
    old_eff[eff_filelen] = '\0';
    
    if (strlen(old_eff) != eff_filelen) {
        warning("invalid effect file '%s', skip.", eff_filename);
    } else {
        //plog("fn=%s, old_eff = %s\n", eff_filename, old_eff);        
        static char *new_eff = NULL;
        free(new_eff);
        new_eff = run_all_effect_hooks(eff_filename, old_eff);
        
        M_DWORD(R_ESP + 0x4) = TOUINT(new_eff);
        M_DWORD(R_ESP + 0x8) = strlen(new_eff);
    }
    
    free(old_eff);
    
    LINK_CALL(gbD3DXCreateEffect);
}

void init_effect_hooks()
{
    INIT_ASMPATCH(hook_D3DXCreateEffect, gboffset + 0x1001D771, 5, "\xE8\x24\x6C\x01\x00");
}
