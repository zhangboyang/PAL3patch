#ifndef PAL3PATCH_PLUGIN_H
#define PAL3PATCH_PLUGIN_H
// PATCHAPI DEFINITIONS

extern PATCHAPI void *patch_malloc(size_t size);
extern PATCHAPI void patch_free(void *ptr);
extern PATCHAPI int version_string_compare(const char *a, const char *b);



#define PLUGIN_LOG_FILE "PAL3patch.plugin_log.txt"

extern PATCHAPI int plugin_log_indent;
extern PATCHAPI void plugin_plog(const char *module, int indent, const char *fmt, ...);
extern PATCHAPI void plugin_warning(const char *module, int indent, const char *fmt, ...);
extern PATCHAPI void plugin_fail(const char *module, int indent, const char *fmt, ...);



#define PLUGINSYMBOL_ENTRY           pal3patch_plugin_entry
#define PLUGINSYMBOL_INTERNAL_NAME   pal3patch_plugin_internalname
#define PLUGINSYMBOL_FRIENDLY_NAME   pal3patch_plugin_friendlyname
#define PLUGINSYMBOL_VERSION         pal3patch_plugin_version
#define PLUGINSYMBOL_PLATFORM        pal3patch_plugin_platform
#define PLUGINSYMBOL_BUILTON         pal3patch_plugin_builton
#define PLUGINSYMBOL_COMPILER        pal3patch_plugin_compiler

#define DECL_PLUGINENTRY(name) int (name)(void)


extern PATCHAPI void load_plugin_dll(const char *filename);
extern PATCHAPI void load_plugin_dll_and_dependents(const char *filename);
extern PATCHAPI void load_plugin_library(const char *filename);
extern PATCHAPI void load_plugin_library_and_dependents(const char *filename);
extern PATCHAPI void load_plugin_list(const char *filename);
extern PATCHAPI void search_plugins(const char *dirpath);



#ifdef PATCHAPI_IMPORTS
// DEFINITIONS FOR PLUGINS

#define PLUGINAPI __declspec(dllexport)

#define MAKE_PLUGINABOUT() \
    PLUGINAPI char    PLUGINSYMBOL_INTERNAL_NAME[] = PLUGIN_INTERNAL_NAME; \
    PLUGINAPI wchar_t PLUGINSYMBOL_FRIENDLY_NAME[] = CONCAT(L, PLUGIN_FRIENDLY_NAME); \
    PLUGINAPI char    PLUGINSYMBOL_VERSION[] = PLUGIN_VERSION; \
    PLUGINAPI char    PLUGINSYMBOL_PLATFORM[] = PATCH_VERSION_STRING; \
    PLUGINAPI char    PLUGINSYMBOL_BUILTON[] = BUILD_DATE; \
    PLUGINAPI char    PLUGINSYMBOL_COMPILER[] = BUILD_COMPILER;
    

#define MAKE_PLUGINENTRY() PLUGINAPI DECL_PLUGINENTRY(PLUGINSYMBOL_ENTRY)
extern MAKE_PLUGINENTRY();

#define LOG_INDENT_VAR plugin_log_indent
#define log_enter() (++(LOG_INDENT_VAR))
#define log_leave() (--(LOG_INDENT_VAR))

#ifndef NO_VARIADIC_MACROS
#define fail(fmt, ...) plugin_fail(PLUGIN_INTERNAL_NAME, LOG_INDENT_VAR, fmt, ##__VA_ARGS__)
#define warning(fmt, ...) plugin_warning(PLUGIN_INTERNAL_NAME, LOG_INDENT_VAR, fmt, ##__VA_ARGS__)
#define plog(fmt, ...) plugin_plog(PLUGIN_INTERNAL_NAME, LOG_INDENT_VAR, fmt, ##__VA_ARGS__)
#else
#if defined(_MSC_VER)

#define MAKE_PLUGINLOG_ASM_WRAPPER(newname, oldname) \
__declspec(naked) static void newname(const char *fmt, ...) \
{ \
    static unsigned retaddr; \
    static const char *plugin_internal_name = PLUGIN_INTERNAL_NAME; \
    __asm { \
        __asm pop retaddr \
        __asm mov eax, dword ptr [LOG_INDENT_VAR] \
        __asm push dword ptr [eax] \
        __asm push plugin_internal_name \
        __asm call dword ptr [oldname] \
        __asm add esp, 8 \
        __asm jmp retaddr \
    } \
}
MAKE_PLUGINLOG_ASM_WRAPPER(fail, plugin_fail)
MAKE_PLUGINLOG_ASM_WRAPPER(warning, plugin_warning)
MAKE_PLUGINLOG_ASM_WRAPPER(plog, plugin_plog)

#endif
#endif

#endif






#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define MAX_PLUGIN_LOG_INDENT 30
extern void init_plugins(void);

#endif
#endif
