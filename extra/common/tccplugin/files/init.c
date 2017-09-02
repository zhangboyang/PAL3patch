#define PLUGIN_NAME "TCC"
#include "PAL3patch.h"
#define USE_TCCPLUGIN_DEFINITIONS
#include "tccplugin.h"

int tccplugin_main()
{
    PAL3_STRUCT_SELFCHECK();
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
