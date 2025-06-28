#include "common.h"

// language, see patch_setlocale.c for details
unsigned system_codepage;
unsigned target_codepage;

// game locale information, see GAME_LOCALE_XXX for details
int game_locale;



// CHS

static const wchar_t wstr_about_title_CHS[] = L"关于";
static const wchar_t wstr_about_text_CHS[] = 
    L"欢迎使用《仙剑奇侠传三》分辨率补丁 %hs\n" 
    L"\n"
    L"本补丁可以扩大游戏的分辨率，\n"
    L"并修复一些游戏程序的小问题，\n"
    L"还添加了一些实用的小功能，\n" 
    L"详细信息和使用方法请参见补丁说明文件。\n"
    L"\n"
    L"\n"
    L"如果您不想每次启动时看到此信息，\n"
    L"请使用补丁配置工具，将“显示关于”设为“禁用”。\n" 
    L"\n"
    L"\n"
    L"补丁网站：\n"
    L"  https://pal3patch.com\n"
    L"\n"
    L"编译信息：\n%hs"
;

static const wchar_t wstr_confirmquit_text_CHS[] = L"您确定要退出游戏吗？\n未保存的游戏进度将会丢失。";
static const wchar_t wstr_confirmquit_title_CHS[] = L"退出";

static const wchar_t wstr_cantsavereg_text_CHS[] = L"无法保存注册表存档文件，请确认游戏对存档文件有写入权限，是否重试？\n注意：若取消，前尘忆梦、小游戏的解锁状态可能会丢失。";
static const wchar_t wstr_cantsavereg_title_CHS[] = L"注册表重定向";

static const wchar_t wstr_badregfile_text_CHS[] = L"注册表存档文件已损坏，是否继续？\n注意：若继续，前尘忆梦、小游戏的解锁状态可能会丢失。";
static const wchar_t wstr_badregfile_title_CHS[] = L"注册表重定向";

static const wchar_t wstr_cantdeltemp_text_CHS[] = L"游戏目录下存在临时文件 %hs，但无法删除，请手动删除它。";
static const wchar_t wstr_cantdeltemp_title_CHS[] = L"无法删除临时文件";

static const wchar_t wstr_nomutex_text_CHS[] = L"请先关闭正在运行中的游戏（或补丁配置工具）。";
static const wchar_t wstr_nomutex_title_CHS[] = L"检测到冲突";

static const wchar_t wstr_nocfgfile_text_CHS[] = L"无法读取补丁配置文件。请运行“补丁配置工具”来生成默认配置文件。";
static const wchar_t wstr_nocfgfile_title_CHS[] = L"无法加载配置";

static const wchar_t wstr_badcfgfile_text_CHS[] = L"补丁配置文件已损坏。请运行“补丁配置工具”，使用“实用工具——恢复默认设置”功能来重新生成配置文件。";
static const wchar_t wstr_badcfgfile_title_CHS[] = L"无法加载配置";

static const wchar_t wstr_badiconv_text_CHS[] = L"文字编码转换测试不通过，游戏内文字可能会乱码。是否仍要进入游戏？";
static const wchar_t wstr_badiconv_title_CHS[] = L"文字编码转换失败";

static const wchar_t wstr_pathtoolong_text_CHS[] = L"游戏路径太长，请缩短游戏路径长度，以防游戏运行出现问题。";
static const wchar_t wstr_pathtoolong_title_CHS[] = L"游戏路径太长";
static const wchar_t wstr_badpath_text_CHS[] = L"游戏路径含有无效字符，请将游戏置于全英文路径下，以防游戏运行出现问题。";
static const wchar_t wstr_badpath_title_CHS[] = L"游戏路径含有无效字符";

static const wchar_t wstr_resetfailed_text_CHS[] = L"无法重置 Direct3D 设备，是否重试？\n\n按【回车】键重试；\n按【ESC】键退出游戏。";
static const wchar_t wstr_resetfailed_title_CHS[] = L"重置失败";

static const wchar_t wstr_resolutiontoolarge_text_CHS[] = L"将要使用 %dx%d 分辨率进行游戏，但目前未启用 LAA （大地址空间支持）。\n\n若不启用 LAA，过大的分辨率将导致游戏运行不稳定或无法运行。\n\n建议您运行“补丁配置工具”，将“图形设置——图形接口”设置为“Direct3D 9.0c (LAA)”。\n\n是否忽略该警告强行进入游戏？";
static const wchar_t wstr_resolutiontoolarge_title_CHS[] = L"建议更换图形接口";

static const wchar_t wstr_nogfxmgr_text_CHS[] = L"显卡设备初始化失败，请检查显卡驱动程序是否工作正常（或图形设置是否合适）。";
static const wchar_t wstr_nogfxmgr_badtools_text_CHS[] = L"显卡设备初始化失败，这可能是以下工具导致的：\n\n%s\n请关闭或禁用它们，然后再试一次。";
static const wchar_t wstr_nogfxmgr_title_CHS[] = L"游戏初始化失败";

static const wchar_t wstr_screenshot_msg_CHS[] = L"屏幕截图已保存至“%hs”";

static const wchar_t wstr_punctuation_nohead_CHS[] = L"，。！？；：、～”’）》」』】";
static const wchar_t wstr_punctuation_notail_CHS[] = L"“‘（《「『【";
static const wchar_t wstr_punctuation_double_CHS[] = L"—…";

static const wchar_t wstr_pluginerr_loadfailed_CHS[] = L"无法加载模块";
static const wchar_t wstr_pluginerr_noentry_CHS[] = L"无入口点";
static const wchar_t wstr_pluginerr_initfailed_CHS[] = L"初始化失败，返回值为 %d";
static const wchar_t wstr_pluginreport_title_CHS[] = L"插件加载报告";
static const wchar_t wstr_pluginreport_template_CHS[] = L"以下是插件加载情况：\n\n%s\n已成功加载 %d 个插件模块。";
static const wchar_t wstr_pluginreport_namepart_CHS[] = L"%s%hs%hs";
static const wchar_t wstr_pluginreport_success_CHS[] = L"【成功】 %s\n";
static const wchar_t wstr_pluginreport_failed_CHS[] = L"【失败】 %s （%s）\n";

static const wchar_t wstr_havebadtool_text_CHS[] = L"检测到以下工具似乎正在运行：\n\n%s\n这些工具可能会影响分辨率补丁正常工作，建议您关闭或禁用它们。\n\n如果您已经关闭或禁用了这些工具，可忽略本信息。\n\n是否仍要进入游戏？";
static const wchar_t wstr_havebadtool_title_CHS[] = L"检测到不兼容的工具";
static const wchar_t wstr_badtool_dxwnd_CHS[] = L"DxWnd 窗口化工具";
static const wchar_t wstr_badtool_d3dwindower_CHS[] = L"D3DWindower 窗口化工具";

static const wchar_t wstr_badgamever_text_CHS[] = L"检测到 1.04 补丁未正确安装。\n\n请运行“补丁配置工具”，使用“实用工具——修复游戏数据”功能来解决此问题。\n\n如果您正在同时使用其它 MOD，可忽略本信息。\n\n是否仍要进入游戏？";
static const wchar_t wstr_badgamever_title_CHS[] = L"游戏版本不正确";



// CHT

static const wchar_t wstr_about_title_CHT[] = L"關於";
static const wchar_t wstr_about_text_CHT[] = 
    L"歡迎使用《仙劍奇俠傳三》解析度補丁 %hs\n" 
    L"\n"
    L"本補丁可以擴大遊戲的解析度，\n"
    L"並修復一些遊戲程式的小問題，\n"
    L"還添加了一些實用的小功能，\n" 
    L"詳細資訊和使用方法請參見補丁說明檔案。\n"
    L"\n"
    L"\n"
    L"如果您不想每次啟動時看到此資訊，\n"
    L"請使用補丁設定工具，將「顯示關於」設為「停用」。\n" 
    L"\n"
    L"\n"
    L"補丁網站：\n"
    L"  https://pal3patch.com\n"
    L"\n"
    L"編譯資訊：\n%hs"
;

static const wchar_t wstr_confirmquit_text_CHT[] = L"您確定要結束遊戲嗎？\n未儲存的遊戲進度將會遺失。";
static const wchar_t wstr_confirmquit_title_CHT[] = L"結束";

static const wchar_t wstr_cantsavereg_text_CHT[] = L"無法儲存登錄檔存檔檔案，請確認遊戲對存檔檔案有寫入權限，是否重試？\n注意：若取消，前塵憶夢、小遊戲的解鎖狀態可能會遺失。";
static const wchar_t wstr_cantsavereg_title_CHT[] = L"登錄檔重新導向";

static const wchar_t wstr_badregfile_text_CHT[] = L"登錄檔存檔檔案已損壞，是否繼續？\n注意：若繼續，前塵憶夢、小遊戲的解鎖狀態可能會遺失。";
static const wchar_t wstr_badregfile_title_CHT[] = L"登錄檔重新導向";

static const wchar_t wstr_cantdeltemp_text_CHT[] = L"遊戲目錄下存在暫存檔案 %hs，但無法刪除，請手動刪除它。";
static const wchar_t wstr_cantdeltemp_title_CHT[] = L"無法刪除暫存檔案";

static const wchar_t wstr_nomutex_text_CHT[] = L"請先關閉正在執行中的遊戲（或補丁設定工具）。";
static const wchar_t wstr_nomutex_title_CHT[] = L"偵測到衝突";

static const wchar_t wstr_nocfgfile_text_CHT[] = L"無法讀取補丁設定檔案。請執行「補丁設定工具」來產生預設設定檔案。";
static const wchar_t wstr_nocfgfile_title_CHT[] = L"無法載入設定";

static const wchar_t wstr_badcfgfile_text_CHT[] = L"補丁設定檔案已損壞。請執行「補丁設定工具」，使用「實用工具——恢復預設設定」功能來重新產生設定檔案。";
static const wchar_t wstr_badcfgfile_title_CHT[] = L"無法載入設定";

static const wchar_t wstr_badiconv_text_CHT[] = L"文字編碼轉換測試不通過，遊戲內文字可能會亂碼。是否仍要進入遊戲？";
static const wchar_t wstr_badiconv_title_CHT[] = L"文字編碼轉換失敗";

static const wchar_t wstr_pathtoolong_text_CHT[] = L"遊戲路徑太長，請縮短遊戲路徑長度，以防遊戲執行出現問題。";
static const wchar_t wstr_pathtoolong_title_CHT[] = L"遊戲路徑太長";
static const wchar_t wstr_badpath_text_CHT[] = L"遊戲路徑含有無效字元，請將遊戲置於全英文路徑下，以防遊戲執行出現問題。";
static const wchar_t wstr_badpath_title_CHT[] = L"遊戲路徑含有無效字元";

static const wchar_t wstr_resetfailed_text_CHT[] = L"無法重設 Direct3D 裝置，是否重試？\n\n按【Enter】鍵重試；\n按【ESC】鍵結束遊戲。";
static const wchar_t wstr_resetfailed_title_CHT[] = L"重設失敗";

static const wchar_t wstr_resolutiontoolarge_text_CHT[] = L"將要使用 %dx%d 解析度進行遊戲，但目前未啟用 LAA （大定址空間支援）。\n\n若不啟用 LAA，過大的解析度將導致遊戲執行不穩定或無法執行。\n\n建議您執行「補丁設定工具」，將「圖形設定——繪圖介面」設定為「Direct3D 9.0c (LAA)」。\n\n是否忽略該警告強行進入遊戲？";
static const wchar_t wstr_resolutiontoolarge_title_CHT[] = L"建議更換繪圖介面";

static const wchar_t wstr_nogfxmgr_text_CHT[] = L"顯示卡裝置初始化失敗，請檢查顯示卡驅動程式是否工作正常（或圖形設定是否合適）。";
static const wchar_t wstr_nogfxmgr_badtools_text_CHT[] = L"顯示卡裝置初始化失敗，這可能是以下工具導致的：\n\n%s\n請關閉或停用它們，然後再試一次。";
static const wchar_t wstr_nogfxmgr_title_CHT[] = L"遊戲初始化失敗";

static const wchar_t wstr_screenshot_msg_CHT[] = L"螢幕截圖已儲存至「%hs」";

static const wchar_t wstr_punctuation_nohead_CHT[] = L"，。！？；：、～”’）》」』】";
static const wchar_t wstr_punctuation_notail_CHT[] = L"“‘（《「『【";
static const wchar_t wstr_punctuation_double_CHT[] = L"—…";

static const wchar_t wstr_pluginerr_loadfailed_CHT[] = L"無法載入模組";
static const wchar_t wstr_pluginerr_noentry_CHT[] = L"無入口點";
static const wchar_t wstr_pluginerr_initfailed_CHT[] = L"初始化失敗，返回值為 %d";
static const wchar_t wstr_pluginreport_title_CHT[] = L"外掛程式載入報告";
static const wchar_t wstr_pluginreport_template_CHT[] = L"以下是外掛程式載入情況：\n\n%s\n已成功載入 %d 個外掛程式模組。";
static const wchar_t wstr_pluginreport_namepart_CHT[] = L"%s%hs%hs";
static const wchar_t wstr_pluginreport_success_CHT[] = L"【成功】 %s\n";
static const wchar_t wstr_pluginreport_failed_CHT[] = L"【失敗】 %s （%s）\n";

static const wchar_t wstr_havebadtool_text_CHT[] = L"偵測到以下工具似乎正在執行：\n\n%s\n這些工具可能會影響解析度補丁正常工作，建議您關閉或停用它們。\n\n如果您已經關閉或停用了這些工具，可忽略本資訊。\n\n是否仍要進入遊戲？";
static const wchar_t wstr_havebadtool_title_CHT[] = L"偵測到不相容的工具";
static const wchar_t wstr_badtool_dxwnd_CHT[] = L"DxWnd 視窗化工具";
static const wchar_t wstr_badtool_d3dwindower_CHT[] = L"D3DWindower 視窗化工具";

static const wchar_t wstr_badgamever_text_CHT[] = L"偵測到 1.04 補丁未正確安裝。\n\n請執行「補丁設定工具」，使用「實用工具——修復遊戲資料」功能來解決此問題。\n\n如果您正在同時使用其它 MOD，可忽略本資訊。\n\n是否仍要進入遊戲？";
static const wchar_t wstr_badgamever_title_CHT[] = L"遊戲版本不正確";



// CHS

static const char defaultfont_ftfilename_CHS[] = "simsun.ttc";
static const int defaultfont_ftindex_CHS = 0;
static const DWORD defaultfont_charset_CHS = GB2312_CHARSET;
static const int defaultfont_bold_CHS = 48;

static const wchar_t wstr_defaultfont_CHS[] = L"SimSun";
static const wchar_t wstr_defaultfont_localized_CHS[] = L"宋体";
static const wchar_t wstr_gameloading_CHS[] = L"游戏加载中，请稍候……";



// CHT

static const char defaultfont_ftfilename_CHT[] = "mingliu.ttc";
static const int defaultfont_ftindex_CHT = 0;
static const DWORD defaultfont_charset_CHT = CHINESEBIG5_CHARSET;
static const int defaultfont_bold_CHT = 32;

static const wchar_t wstr_defaultfont_CHT[] = L"MingLiU";
static const wchar_t wstr_defaultfont_localized_CHT[] = L"細明體";
static const wchar_t wstr_gameloading_CHT[] = L"遊戲載入中，請稍候……";



// string pointers and default values

const wchar_t *wstr_about_title;
const wchar_t *wstr_about_text;
const wchar_t *wstr_confirmquit_text;
const wchar_t *wstr_confirmquit_title;
const wchar_t *wstr_cantsavereg_text;
const wchar_t *wstr_cantsavereg_title;
const wchar_t *wstr_badregfile_text;
const wchar_t *wstr_badregfile_title;
const wchar_t *wstr_cantdeltemp_text;
const wchar_t *wstr_cantdeltemp_title;
const wchar_t *wstr_nomutex_text;
const wchar_t *wstr_nomutex_title;
const wchar_t *wstr_nocfgfile_text;
const wchar_t *wstr_nocfgfile_title;
const wchar_t *wstr_badcfgfile_text;
const wchar_t *wstr_badcfgfile_title;
const wchar_t *wstr_badiconv_text;
const wchar_t *wstr_badiconv_title;
const wchar_t *wstr_pathtoolong_text;
const wchar_t *wstr_pathtoolong_title;
const wchar_t *wstr_badpath_text;
const wchar_t *wstr_badpath_title;
const wchar_t *wstr_resetfailed_text;
const wchar_t *wstr_resetfailed_title;
const wchar_t *wstr_resolutiontoolarge_text;
const wchar_t *wstr_resolutiontoolarge_title;
const wchar_t *wstr_nogfxmgr_text;
const wchar_t *wstr_nogfxmgr_badtools_text;
const wchar_t *wstr_nogfxmgr_title;
const wchar_t *wstr_screenshot_msg;
const wchar_t *wstr_punctuation_nohead;
const wchar_t *wstr_punctuation_notail;
const wchar_t *wstr_punctuation_double;
const wchar_t *wstr_pluginerr_loadfailed;
const wchar_t *wstr_pluginerr_noentry;
const wchar_t *wstr_pluginerr_initfailed;
const wchar_t *wstr_pluginreport_title;
const wchar_t *wstr_pluginreport_template;
const wchar_t *wstr_pluginreport_namepart;
const wchar_t *wstr_pluginreport_success;
const wchar_t *wstr_pluginreport_failed;
const wchar_t *wstr_havebadtool_text;
const wchar_t *wstr_havebadtool_title;
const wchar_t *wstr_badtool_dxwnd;
const wchar_t *wstr_badtool_d3dwindower;
const wchar_t *wstr_badgamever_text;
const wchar_t *wstr_badgamever_title;

const char *defaultfont_ftfilename;
int defaultfont_ftindex;
DWORD defaultfont_charset;
int defaultfont_bold;
const wchar_t *wstr_defaultfont;
const wchar_t *wstr_gameloading;






static LANGID detect_ui_langid(void);
static int detect_game_locale(void);

#define IMPORT_LOCALE_ITEM(symbol) \
    do { \
        symbol = CONCAT(symbol, _CHS); \
        if (lang == 1) symbol = CONCAT(symbol, _CHT); \
    } while (0)

void init_locale_early()
{
    // init codepage
    // PATCHSET 'setlocale' may overwrite target_codepage
    target_codepage = system_codepage = GetACP();

    // detect game locale
    game_locale = detect_game_locale();


    int lang;
    switch (detect_ui_langid()) {
    case 0x0004: case 0x7804: case 0x0804: case 0x1004: lang = 0; break;
    case 0x7C04: case 0x0C04: case 0x1404: case 0x0404: lang = 1; break;
    default: lang = (game_locale != GAME_LOCALE_CHT ? 0 : 1);
    }


    IMPORT_LOCALE_ITEM(wstr_cantdeltemp_text);
    IMPORT_LOCALE_ITEM(wstr_cantdeltemp_title);
    IMPORT_LOCALE_ITEM(wstr_nomutex_text);
    IMPORT_LOCALE_ITEM(wstr_nomutex_title);
    IMPORT_LOCALE_ITEM(wstr_nocfgfile_text);
    IMPORT_LOCALE_ITEM(wstr_nocfgfile_title);
    IMPORT_LOCALE_ITEM(wstr_badcfgfile_text);
    IMPORT_LOCALE_ITEM(wstr_badcfgfile_title);
    IMPORT_LOCALE_ITEM(wstr_badiconv_text);
    IMPORT_LOCALE_ITEM(wstr_badiconv_title);

    IMPORT_LOCALE_ITEM(wstr_about_title);
    IMPORT_LOCALE_ITEM(wstr_about_text);
    IMPORT_LOCALE_ITEM(wstr_confirmquit_text);
    IMPORT_LOCALE_ITEM(wstr_confirmquit_title);
    IMPORT_LOCALE_ITEM(wstr_cantsavereg_text);
    IMPORT_LOCALE_ITEM(wstr_cantsavereg_title);
    IMPORT_LOCALE_ITEM(wstr_badregfile_text);
    IMPORT_LOCALE_ITEM(wstr_badregfile_title);
    IMPORT_LOCALE_ITEM(wstr_pathtoolong_text);
    IMPORT_LOCALE_ITEM(wstr_pathtoolong_title);
    IMPORT_LOCALE_ITEM(wstr_badpath_text);
    IMPORT_LOCALE_ITEM(wstr_badpath_title);
    IMPORT_LOCALE_ITEM(wstr_resetfailed_text);
    IMPORT_LOCALE_ITEM(wstr_resetfailed_title);
    IMPORT_LOCALE_ITEM(wstr_resolutiontoolarge_text);
    IMPORT_LOCALE_ITEM(wstr_resolutiontoolarge_title);
    IMPORT_LOCALE_ITEM(wstr_nogfxmgr_text);
    IMPORT_LOCALE_ITEM(wstr_nogfxmgr_badtools_text);
    IMPORT_LOCALE_ITEM(wstr_nogfxmgr_title);
    IMPORT_LOCALE_ITEM(wstr_screenshot_msg);
    IMPORT_LOCALE_ITEM(wstr_punctuation_nohead);
    IMPORT_LOCALE_ITEM(wstr_punctuation_notail);
    IMPORT_LOCALE_ITEM(wstr_punctuation_double);
    IMPORT_LOCALE_ITEM(wstr_pluginerr_loadfailed);
    IMPORT_LOCALE_ITEM(wstr_pluginerr_noentry);
    IMPORT_LOCALE_ITEM(wstr_pluginerr_initfailed);
    IMPORT_LOCALE_ITEM(wstr_pluginreport_title);
    IMPORT_LOCALE_ITEM(wstr_pluginreport_template);
    IMPORT_LOCALE_ITEM(wstr_pluginreport_namepart);
    IMPORT_LOCALE_ITEM(wstr_pluginreport_success);
    IMPORT_LOCALE_ITEM(wstr_pluginreport_failed);
    IMPORT_LOCALE_ITEM(wstr_havebadtool_text);
    IMPORT_LOCALE_ITEM(wstr_havebadtool_title);
    IMPORT_LOCALE_ITEM(wstr_badtool_dxwnd);
    IMPORT_LOCALE_ITEM(wstr_badtool_d3dwindower);
    IMPORT_LOCALE_ITEM(wstr_badgamever_text);
    IMPORT_LOCALE_ITEM(wstr_badgamever_title);
}

void init_locale()
{
    const wchar_t *wstr_defaultfont_localized;

    int lang = (game_locale != GAME_LOCALE_CHT ? 0 : 1);

    IMPORT_LOCALE_ITEM(defaultfont_ftfilename);
    IMPORT_LOCALE_ITEM(defaultfont_ftindex);
    IMPORT_LOCALE_ITEM(defaultfont_charset);
    IMPORT_LOCALE_ITEM(defaultfont_bold);
    IMPORT_LOCALE_ITEM(wstr_defaultfont);
    IMPORT_LOCALE_ITEM(wstr_defaultfont_localized);
    IMPORT_LOCALE_ITEM(wstr_gameloading);

    if (is_win9x() && system_codepage == target_codepage) {
        wstr_defaultfont = wstr_defaultfont_localized;
    }
}






static LANGID detect_ui_langid(void)
{
    LANGID (WINAPI *myGetUserDefaultUILanguage)(void) = (void *) GetProcAddress(GetModuleHandle("KERNEL32.DLL"), "GetUserDefaultUILanguage");
    return myGetUserDefaultUILanguage ? myGetUserDefaultUILanguage() : GetUserDefaultLangID();
}

static int detect_game_locale(void)
{
    // read compressed data of "basedata.cpk\datascript\lang.txt"
    // and use magic string to determine game locale
    
    const char *CHS_magic = "\x41\x70\x70\x4E\x61\x6D\x65\x09\x09\x24\xCF\xC9\xBD\xA3\xC6\xE6\xCF\xC0\xB4\xAB\x33\x26";
    const char *CHT_magic = "\x41\x70\x70\x4E\x61\x6D\x65\x09\x09\x24\xA5\x50\xBC\x43\xA9\x5F\xAB\x4C\xB6\xC7\x33\x26";
    size_t CHS_magic_len = strlen(CHS_magic);
    size_t CHT_magic_len = strlen(CHT_magic);
    unsigned key_CRC = 0xCB283888; // equals gbCrc32Compute("datascript\\lang.txt"), but we can't call gbCrc32Compute() at this time
    
    int result = GAME_LOCALE_UNKNOWN;
    
    unsigned i;
    struct rawcpk *rcpk = NULL;
    void *data = NULL;
    unsigned datasz;
    
    rcpk = rawcpk_open("basedata\\basedata.cpk");
    if (!rcpk) goto done;
    
    data = rawcpk_read(rcpk, key_CRC, &datasz);
    if (!data) goto done;
    
    // process data
    for (i = 0; i < datasz; i++) {
        if (i + CHS_magic_len <= datasz && memcmp(PTRADD(data, i), CHS_magic, CHS_magic_len) == 0) {
            result = GAME_LOCALE_CHS;
            break;
        }
        if (i + CHT_magic_len <= datasz && memcmp(PTRADD(data, i), CHT_magic, CHT_magic_len) == 0) {
            result = GAME_LOCALE_CHT;
            break;
        }
    }
    
done:
    free(data);
    if (rcpk) rawcpk_close(rcpk);
    
    return result;
}
