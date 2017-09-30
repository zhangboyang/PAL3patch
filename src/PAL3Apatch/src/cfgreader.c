#include "common.h"

struct config_line {
    char *key, *val;
};

static struct config_line cfgdata[MAX_CONFIG_LINES];
static int cfglines;
static int cfg_loaded = 0;

static int config_line_cmp(const void *a, const void *b)
{
    const struct config_line *pa = a, *pb = b;
    return stricmp(pa->key, pb->key);
}

static int is_spacechar(char ch)
{
	return !!strchr(SPACECHAR_LIST, ch);
}

#define badcfg(fmt, ...) fail_with_extra_msg(wstr_badcfgfile_text, wstr_badcfgfile_title, fmt, ## __VA_ARGS__)

void read_config_file()
{
    cfglines = 0;
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp) {
        fail_with_extra_msg(wstr_nocfgfile_text, wstr_nocfgfile_title, "config file '%s' not found.", CONFIG_FILE);
    }
    
    char buf[MAXLINE];
    char *ptr;
    fscanf(fp, UTF8_BOM_STR);
    int linenum = 0;
    while (fgets(buf, sizeof(buf), fp)) {
        linenum++;
        
        // ltrim the line
        for (ptr = buf; *ptr && is_spacechar(*ptr); ptr++);
        memmove(buf, ptr, strlen(ptr) + 1);
        
        // skip empty and comment lines
        if (!buf[0] || buf[0] == ';' || buf[0] == '#' || (buf[0] == '/' && buf[1] == '/')) continue;
        
        // remove '\n' and end of line
        ptr = strchr(buf, '\n');
        if (ptr) *ptr = '\0';
        
        // parse 'key' and 'value'
        ptr = strchr(buf, '=');
        if (!ptr) badcfg("invalid config data at line %d", linenum);
        *ptr = '\0';
        char *keystr = buf, *valstr = ptr + 1;
        
        // rtrim 'key'
        while (ptr > buf && is_spacechar(ptr[-1])) ptr[-1] = '\0', ptr--;
        if (!buf[0]) badcfg("key is empty at line %d", linenum);
        
        // ltrim 'value'
        while (*valstr && is_spacechar(*valstr)) valstr++;
        
        // save this config line to array
        if (cfglines >= MAX_CONFIG_LINES) badcfg("too many config lines.");
        cfgdata[cfglines].key = strdup(keystr);
        cfgdata[cfglines].val = strdup(valstr);
        cfglines++;
    }
    fclose(fp);
    
    // sort the array
    qsort(cfgdata, cfglines, sizeof(struct config_line), config_line_cmp);
    
    // check for duplicate keys
    int i;
    for (i = 1; i < cfglines; i++) {
        int ret = config_line_cmp(&cfgdata[i - 1], &cfgdata[i]);
        if (ret == 0) badcfg("duplicate key '%s'.", cfgdata[i].key);
    }
    
    cfg_loaded = 1;
}

const char *get_string_from_configfile_unsafe(const char *key)
{
    struct config_line tmp;
    tmp.key = (char *) key;
    struct config_line *result;
    result = bsearch(&tmp, cfgdata, cfglines, sizeof(struct config_line), config_line_cmp);
    if (!result) return NULL;
    return result->val;
}

const char *get_string_from_configfile(const char *key)
{
    const char *ret = get_string_from_configfile_unsafe(key);
    if (!ret) badcfg("can't find config line with key '%s'.", key);
    return ret;
}

int get_int_from_configfile(const char *key)
{
    const char *valstr = get_string_from_configfile(key);
    return str2int(valstr);
}

void dump_all_config(FILE *fp)
{
    if (!cfg_loaded) {
        fprintf(fp, "  config file haven't loaded.\n");
        return;
    }
    int i;
    for (i = 0; i < cfglines; i++) {
        fprintf(fp, "  %s=%s\n", cfgdata[i].key, cfgdata[i].val);
    }
}

// no clean up functions, just let these strings leak
