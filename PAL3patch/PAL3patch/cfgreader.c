#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

struct config_line {
    const char *key, *val;
};

static struct config_line cfgdata[MAX_CONFIG_LINES];
static int cfglines;

static int config_line_cmp(const void *a, const void *b)
{
    const struct config_line *pa = a, *pb = b;
    return stricmp(pa->key, pb->key);
}

void read_config_file()
{
    cfglines = 0;
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp) fail("Can't open config file '%s'.", CONFIG_FILE);
    char buf[MAXLINE];
    char *ptr;
    while (fgets(buf, sizeof(buf), fp)) {
        // skip empty and comment lines
        if (!buf[0] || buf[0] == ';' || buf[0] == '#' || (buf[0] == '/' && buf[1] == '/')) continue;

        // skip white space lines
        for (ptr = buf; *ptr && isspace(*ptr); ptr++);
        if (!*ptr) continue;
        
        // remove '\n' and end of line
        ptr = strchr(buf, '\n');
        if (ptr) *ptr = '\0';
        
        // parse 'key' and 'value'
        ptr = strchr(buf, '=');
        if (!ptr) fail("can't parse config line:\n  %s", buf);
        *ptr = '\0';
        char *keystr = buf, *valstr = ptr + 1;
        
        // rtrim 'key'
        ptr--;
        while (ptr >= buf && isspace(*ptr)) *ptr-- = '\0';
        if (!*ptr) fail("keystr is empty");
        
        // ltrim 'value'
        while (*valstr && isspace(*valstr)) valstr++;
        
        // save this config line to array
        if (cfglines >= MAX_CONFIG_LINES) fail("too many config lines.");
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
        if (ret == 0) fail("duplicate key '%s'.", cfgdata[i].key);
    }
}

const char *get_string_from_configfile(const char *key)
{
    struct config_line tmp;
    tmp.key = key;
    struct config_line *result;
    result = bsearch(&tmp, cfgdata, cfglines, sizeof(struct config_line), config_line_cmp);
    if (!result) fail("can't find config line with key '%s'.", key);
    return result->val;
}

int get_int_from_configfile(const char *key)
{
    const char *valstr = get_string_from_configfile(key);
    int result, ret;
    ret = sscanf(valstr, "%d", &result);
    if (ret != 1) fail("can't parse '%s' to integer.", valstr);
    return result;
}

// no clean up functions, just let these strings leak
