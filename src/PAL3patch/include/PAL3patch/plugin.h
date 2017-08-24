#ifndef PAL3PATCH_PLUGIN_H
#define PAL3PATCH_PLUGIN_H
// PATCHAPI DEFINITIONS


extern PATCHAPI void plugin_plog(const char *module, const char *fmt, ...);
extern PATCHAPI void plugin_warning(const char *module, const char *fmt, ...);
extern PATCHAPI void plugin_fail(const char *module, const char *fmt, ...);

extern PATCHAPI void load_plugins(const wchar_t *cfgline);

#define PLUGIN_ENTRY_NAME pal3patch_plugin_entry
#define DECL_PLUGINENTRY(name) int (name)(void)






#ifdef PATCHAPI_IMPORTS
// DEFINITIONS FOR PLUGINS

#ifndef PLUGIN_NAME
#define PLUGIN_NAME __FILE__
#endif

#define PLUGINAPI __declspec(dllexport)
#define MAKE_PLUGINENTRY() PLUGINAPI DECL_PLUGINENTRY(PLUGIN_ENTRY_NAME)
extern MAKE_PLUGINENTRY();

#ifndef NO_VARIADIC_MACROS
#define fail(fmt, ...) plugin_fail(PLUGIN_NAME, fmt, ##__VA_ARGS__)
#define warning(fmt, ...) plugin_warning(PLUGIN_NAME, fmt, ##__VA_ARGS__)
#define plog(fmt, ...) plugin_plog(PLUGIN_NAME, fmt, ##__VA_ARGS__)
#endif

#endif






#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

extern void init_plugins(void);

#endif
#endif
