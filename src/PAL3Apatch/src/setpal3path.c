#include "common.h"

static char pal3path[0x100];

static MAKE_THISCALL(bool, FindArc_ReadReg, struct FindArc *this, char *KEY_ROOT, char *KEY_NAME, char *outstr)
{
    strcpy(outstr, pal3path);
    return true;
}

void init_setpal3path()
{
    char *cfg_utf8_buffer = strdup(get_string_from_configfile("setpal3path"));
    
    if (stricmp(cfg_utf8_buffer, "autodetect") != 0) {
        str_rtrim(cfg_utf8_buffer, "\\/");
        
        char *pal3path_buffer = cs2cs_alloc(cfg_utf8_buffer, CP_UTF8, CP_ACP);
        strncpy(pal3path, pal3path_buffer, sizeof(pal3path));
        pal3path[sizeof(pal3path) - 1] = '\0';
        free(pal3path_buffer);

        make_jmp(0x0051EF05, FindArc_ReadReg);
    }
    
    free(cfg_utf8_buffer);
}
