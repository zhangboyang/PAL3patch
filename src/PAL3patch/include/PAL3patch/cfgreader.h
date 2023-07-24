#ifndef PAL3PATCH_CFGREADER_H
#define PAL3PATCH_CFGREADER_H
// PATCHAPI DEFINITIONS

extern PATCHAPI const char *get_string_from_configfile_unsafe(const char *key);
extern PATCHAPI const char *get_string_from_configfile(const char *key);
extern PATCHAPI int get_int_from_configfile(const char *key);


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define CONFIG_FILE     "PAL3patch.conf"
#define CONFIG_FILE_WAL "PAL3patch.wal"
#define CONFIG_FILE_SUM "PAL3patch.sum"
#define MAX_CONFIG_LINES 1000

extern void read_config_file(void);
extern void dump_all_config(FILE *fp);

#endif
#endif
