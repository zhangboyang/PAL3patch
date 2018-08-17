#define PLUGIN_INTERNAL_NAME "TCCINIT"
#ifdef BUILD_FOR_PAL3
#define USE_PAL3_DEFINITIONS
#include "PAL3patch.h"
#endif
#ifdef BUILD_FOR_PAL3A
#define USE_PAL3A_DEFINITIONS
#include "PAL3Apatch.h"
#endif
#define USE_TCCPLUGIN_DEFINITIONS
#include "tccplugin.h"

int tccplugin_main()
{
#ifdef BUILD_FOR_PAL3
    PAL3_STRUCT_SELFCHECK();
#endif
#ifdef BUILD_FOR_PAL3A
    PAL3A_STRUCT_SELFCHECK();
#endif
    
    struct cstr filelist;
    cstr_ctor(&filelist);
    
    tccplugin_default_codepage = 936;
    
    search_cplugins("plugins", &filelist);
    
    if (!cstr_empty(&filelist)) {
        MessageBoxW_utf8format(NULL, "已加载以下 C 语言文件：\n\n%s", TCCPLUGIN_MSGBOX_TITLE, MB_ICONINFORMATION, cstr_getstr(&filelist));
    } else {
        MessageBoxW_utf8format(NULL, "没有可供加载的 C 语言文件。", TCCPLUGIN_MSGBOX_TITLE, MB_ICONINFORMATION);
    }
    
    cstr_dtor(&filelist);
    return 0;
}
