#ifndef PAL3PATCH_CFGREADER_H
#define PAL3PATCH_CFGREADER_H

#define CONFIG_FILE "PAL3patch.conf"
#define MAX_CONFIG_LINES 100

extern void read_config_file();
extern const char *get_string_from_configfile(const char *key);
extern int get_int_from_configfile(const char *key);
extern void get_all_config(char *buf, unsigned size);

#endif
