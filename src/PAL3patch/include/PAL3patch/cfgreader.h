#ifndef PAL3PATCH_CFGREADER_H
#define PAL3PATCH_CFGREADER_H
#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define CONFIG_FILE "PAL3patch.conf"
#define MAX_CONFIG_LINES 1000

extern void read_config_file(void);
extern const char *get_string_from_configfile(const char *key);
extern int get_int_from_configfile(const char *key);
extern void dump_all_config(FILE *fp);

#endif
#endif
