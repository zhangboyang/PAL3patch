#ifndef PAL3PATCH_PLUGIN_H
#define PAL3PATCH_PLUGIN_H
// PATCHAPI DEFINITIONS

extern PATCHAPI int version_string_compare(const char *a, const char *b);

extern PATCHAPI int plugin_log_indent;
extern PATCHAPI void plugin_plog(const char *module, int indent, const char *fmt, ...);
extern PATCHAPI void plugin_warning(const char *module, int indent, const char *fmt, ...);
extern PATCHAPI void plugin_fail(const char *module, int indent, const char *fmt, ...);

#define PLUGIN_ENTRY_NAME pal3patch_plugin_entry
#define DECL_PLUGINENTRY(name) int (name)(void)



extern PATCHAPI void load_plugin_dll(const char *filename);
extern PATCHAPI void load_plugin_dll_and_dependents(const char *filename);
extern PATCHAPI void load_plugin_library(const char *filename);
extern PATCHAPI void load_plugin_library_and_dependents(const char *filename);
extern PATCHAPI void load_plugin_list(const char *filename);
extern PATCHAPI void search_plugins(const char *dirpath);



#ifdef PATCHAPI_IMPORTS
// DEFINITIONS FOR PLUGINS

#ifndef PLUGIN_NAME
#define PLUGIN_NAME __FILE__
#endif

#define PLUGINAPI __declspec(dllexport)
#define MAKE_PLUGINENTRY() PLUGINAPI DECL_PLUGINENTRY(PLUGIN_ENTRY_NAME)
extern MAKE_PLUGINENTRY();

#define LOG_INDENT_VAR plugin_log_indent
#define log_enter() (++(LOG_INDENT_VAR))
#define log_leave() (--(LOG_INDENT_VAR))

#ifndef NO_VARIADIC_MACROS
#define fail(fmt, ...) plugin_fail(PLUGIN_NAME, LOG_INDENT_VAR, fmt, ##__VA_ARGS__)
#define warning(fmt, ...) plugin_warning(PLUGIN_NAME, LOG_INDENT_VAR, fmt, ##__VA_ARGS__)
#define plog(fmt, ...) plugin_plog(PLUGIN_NAME, LOG_INDENT_VAR, fmt, ##__VA_ARGS__)
#endif

#endif






#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define MAX_PLUGIN_LOG_INDENT 30
extern void init_plugins(void);

#endif
#endif
