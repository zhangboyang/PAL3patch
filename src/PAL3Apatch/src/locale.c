#include "common.h"

// language, see patch_setlocale.c for details
unsigned system_codepage;
unsigned target_codepage;

// game locale information, see GAME_LOCALE_XXX for details
int game_locale;

// CHS
static const wchar_t wstr_about_title_CHS[] = L"关于";
static const wchar_t wstr_about_text_CHS[] = 
    L"欢迎使用《仙剑三外传·问情篇》分辨率补丁 %hs\n" 
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
    L"  https://pal3.zbyzbyzby.com\n"
    L"\n"
    L"编译信息：\n%hs"
;

static const wchar_t wstr_confirmquit_text_CHS[] = L"您确定要退出游戏吗？\n未保存的游戏进度将会丢失。";
static const wchar_t wstr_confirmquit_title_CHS[] = L"退出";

static const char defaultfont_ftfilename_CHS[] = "simsun.ttc";
static const int defaultfont_ftindex_CHS = 0;
static const DWORD defaultfont_charset_CHS = GB2312_CHARSET;
static const int defaultfont_bold_CHS = 48;

static const wchar_t wstr_defaultfont_CHS[] = L"宋体";
static const wchar_t wstr_gameloading_CHS[] = L"游戏加载中，请稍候……";  // 少女祈祷中

static const wchar_t wstr_cantsavereg_text_CHS[] = L"无法保存注册表存档文件，请确认游戏对存档文件有写入权限，是否重试？\n注意：若取消，前尘忆梦的解锁状态、捉鬼小游戏的关卡进度可能会丢失。";
static const wchar_t wstr_cantsavereg_title_CHS[] = L"注册表重定向";

static const wchar_t wstr_badregfile_text_CHS[] = L"注册表存档文件已损坏，是否继续？\n注意：若继续，前尘忆梦的解锁状态、捉鬼小游戏的关卡进度可能会丢失。";
static const wchar_t wstr_badregfile_title_CHS[] = L"注册表重定向";

static const wchar_t wstr_nocfgfile_text_CHS[] = L"无法读取补丁配置文件。请运行“补丁配置工具”来生成默认配置文件。";
static const wchar_t wstr_nocfgfile_title_CHS[] = L"无法加载配置";

static const wchar_t wstr_badcfgfile_text_CHS[] = L"补丁配置文件已损坏。请运行“补丁配置工具”，使用“实用工具——恢复默认设置”功能来重写配置文件。";
static const wchar_t wstr_badcfgfile_title_CHS[] = L"无法加载配置";

static const wchar_t wstr_resetfailed_text_CHS[] = L"无法重置 Direct3D 设备，是否重试？\n\n按【回车】键重试；\n按【ESC】键退出游戏。";
static const wchar_t wstr_resetfailed_title_CHS[] = L"重置失败";

static const wchar_t wstr_screenshot_msg_CHS[] = L"屏幕截图已保存至“%hs”";

static const wchar_t wstr_punctuation_nohead_CHS[] = L"，。！？；：、～”’）》」』】";
static const wchar_t wstr_punctuation_notail_CHS[] = L"“‘（《「『【";
static const wchar_t wstr_punctuation_double_CHS[] = L"—…";

static const wchar_t wstr_pluginerr_loadfailed_CHS[] = L"无法加载模块";
static const wchar_t wstr_pluginerr_noentry_CHS[] = L"无入口点";
static const wchar_t wstr_pluginerr_initfailed_CHS[] = L"初始化失败，返回值为 %d";
static const wchar_t wstr_pluginreport_title_CHS[] = L"插件加载报告";
static const wchar_t wstr_pluginreport_template_CHS[] = L"以下是插件加载情况：\n\n%s\n已成功加载 %d 个插件模块。";
static const wchar_t wstr_pluginreport_namepart_CHS[] = L"%s %hs （%s）";
static const wchar_t wstr_pluginreport_success_CHS[] = L"【成功】 %s\n";
static const wchar_t wstr_pluginreport_failed_CHS[] = L"【失败】 %s （%s）\n";


// CHT
static const char defaultfont_ftfilename_CHT[] = "mingliu.ttc";
static const int defaultfont_ftindex_CHT = 0;
static const DWORD defaultfont_charset_CHT = CHINESEBIG5_CHARSET;
static const int defaultfont_bold_CHT = 32;

static const wchar_t wstr_defaultfont_CHT[] = L"細明體";

// FIXME: add more translations




// string pointers and default values

const wchar_t *wstr_about_title;
const wchar_t *wstr_about_text;
const wchar_t *wstr_confirmquit_text;
const wchar_t *wstr_confirmquit_title;
const char *defaultfont_ftfilename;
int defaultfont_ftindex;
DWORD defaultfont_charset;
int defaultfont_bold;
const wchar_t *wstr_defaultfont;
const wchar_t *wstr_gameloading;
const wchar_t *wstr_cantsavereg_text;
const wchar_t *wstr_cantsavereg_title;
const wchar_t *wstr_badregfile_text;
const wchar_t *wstr_badregfile_title;
const wchar_t *wstr_nocfgfile_text;
const wchar_t *wstr_nocfgfile_title;
const wchar_t *wstr_badcfgfile_text;
const wchar_t *wstr_badcfgfile_title;
const wchar_t *wstr_resetfailed_text;
const wchar_t *wstr_resetfailed_title;
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







static int detect_game_locale();

#define IMPORT_LOCALE_ITEM(lang, symbol) ((symbol) = (CONCAT3(symbol, _, lang)))

void init_locale_early()
{
    // init codepage
    // PATCHSET 'setlocale' may overwrite target_codepage
    target_codepage = system_codepage = GetACP();

    // detect game locale
    game_locale = detect_game_locale();
    
    
    // init early strings
    IMPORT_LOCALE_ITEM(CHS, wstr_nocfgfile_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_nocfgfile_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_badcfgfile_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_badcfgfile_title);
    
    if (target_codepage == CODEPAGE_CHT) { // CHT
        // FIXME
    }
}

void init_locale()
{
    // no translations yet
    IMPORT_LOCALE_ITEM(CHS, wstr_about_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_about_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_confirmquit_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_confirmquit_title);
    IMPORT_LOCALE_ITEM(CHS, defaultfont_ftfilename);
    IMPORT_LOCALE_ITEM(CHS, defaultfont_ftindex);
    IMPORT_LOCALE_ITEM(CHS, defaultfont_charset);
    IMPORT_LOCALE_ITEM(CHS, defaultfont_bold);
    IMPORT_LOCALE_ITEM(CHS, wstr_defaultfont);
    IMPORT_LOCALE_ITEM(CHS, wstr_gameloading);
    IMPORT_LOCALE_ITEM(CHS, wstr_cantsavereg_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_cantsavereg_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_badregfile_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_badregfile_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_resetfailed_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_resetfailed_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_screenshot_msg);
    IMPORT_LOCALE_ITEM(CHS, wstr_punctuation_nohead);
    IMPORT_LOCALE_ITEM(CHS, wstr_punctuation_notail);
    IMPORT_LOCALE_ITEM(CHS, wstr_punctuation_double);
    IMPORT_LOCALE_ITEM(CHS, wstr_pluginerr_loadfailed);
    IMPORT_LOCALE_ITEM(CHS, wstr_pluginerr_noentry);
    IMPORT_LOCALE_ITEM(CHS, wstr_pluginerr_initfailed);
    IMPORT_LOCALE_ITEM(CHS, wstr_pluginreport_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_pluginreport_template);
    IMPORT_LOCALE_ITEM(CHS, wstr_pluginreport_namepart);
    IMPORT_LOCALE_ITEM(CHS, wstr_pluginreport_success);
    IMPORT_LOCALE_ITEM(CHS, wstr_pluginreport_failed);

    if (target_codepage == CODEPAGE_CHT) { // CHT
        IMPORT_LOCALE_ITEM(CHT, defaultfont_ftfilename);
        IMPORT_LOCALE_ITEM(CHT, defaultfont_ftindex);
        IMPORT_LOCALE_ITEM(CHT, defaultfont_charset);
        IMPORT_LOCALE_ITEM(CHT, defaultfont_bold);
        IMPORT_LOCALE_ITEM(CHT, wstr_defaultfont);
        // FIXME
    }
}









static int detect_game_locale()
{
    // read compressed data of "basedata.cpk\datascript\lang.txt"
    // and use magic string to determine game locale
    
    const char *CHS_magic = "\x41\x70\x70\x4E\x61\x6D\x65\x09\x09\x24\xCF\xC9\xBD\xA3\xC6\xE6\xCF\xC0\xB4\xAB\x33\x26";
    const char *CHT_magic = "\x41\x70\x70\x4E\x61\x6D\x65\x09\x09\x24\xA5\x50\xBC\x43\xA9\x5F\xAB\x4C\xB6\xC7\x33\x26";
    size_t CHS_magic_len = strlen(CHS_magic);
    size_t CHT_magic_len = strlen(CHT_magic);
    ULONG key_CRC = 0xCB283888; // equals gbCrc32Compute("datascript\\lang.txt"), but we can't call gbCrc32Compute() at this time
    
    int result = GAME_LOCALE_UNKNOWN;

    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMapFile = NULL;
    LPVOID lpMapAddress = NULL;

    unsigned i;
    int left, right, mid;
    ULONG base, skip;
    SYSTEM_INFO SysInfo;
    DWORD dwSysGran;
    struct CPKHeader cpkhdr;
    struct CPKTable *cpktbl;
    struct CPKTable cpkitem;
    void *pdata;
    ULONG datasz;
    
    // create file handle
    hFile = CreateFile("basedata\\basedata.cpk", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
    if (hFile == INVALID_HANDLE_VALUE) goto done;
    
    // create file mapping handle
    hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMapFile) goto done;
    
    // get memory allocation granularity
    GetSystemInfo(&SysInfo);
    dwSysGran = SysInfo.dwAllocationGranularity;
    
    // map CPK header
    lpMapAddress = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, sizeof(struct CPKHeader));
    if (!lpMapAddress) goto done;
    
    // read CPK header
    memcpy(&cpkhdr, lpMapAddress, sizeof(cpkhdr));
    
    // map CPK index table
    UnmapViewOfFile(lpMapAddress);
    lpMapAddress = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, sizeof(struct CPKHeader) + sizeof(struct CPKTable) * cpkhdr.dwMaxFileNum);
    if (!lpMapAddress) goto done;
    cpktbl = PTRADD(lpMapAddress, sizeof(struct CPKHeader));
    
    // search CPK table entry
    left = 0;
    right = cpkhdr.dwValidTableNum;
	while (1) {
        if (left == right) goto done;
        mid = left + (right - left) / 2;
        if (cpktbl[mid].dwCRC == key_CRC && (cpktbl[mid].dwFlag & 0x1) && !(cpktbl[mid].dwFlag & 0x10)) {
            break;
        }
        if (left + 1 == right) goto done;
        if (key_CRC >= cpktbl[mid].dwCRC) {
            left = mid;
        } else {
            right = mid;
        }
    }
	memcpy(&cpkitem, &cpktbl[mid], sizeof(cpkitem));
	
	// map file data
	base = ROUND_DOWN(cpkitem.dwStartPos, dwSysGran);
	skip = cpkitem.dwStartPos - base;
    UnmapViewOfFile(lpMapAddress);
    lpMapAddress = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, base, skip + cpkitem.dwPackedSize);
    if (!lpMapAddress) goto done;
    pdata = PTRADD(lpMapAddress, skip);
    datasz = cpkitem.dwPackedSize;
	
	// process data
	for (i = 0; i < datasz; i++) {
        if (i + CHS_magic_len <= datasz && memcmp(PTRADD(pdata, i), CHS_magic, CHS_magic_len) == 0) {
            result = GAME_LOCALE_CHS;
            break;
        }
        if (i + CHT_magic_len <= datasz && memcmp(PTRADD(pdata, i), CHT_magic, CHT_magic_len) == 0) {
            result = GAME_LOCALE_CHT;
            break;
        }
    }
	
done:
    if (lpMapAddress) UnmapViewOfFile(lpMapAddress);
    if (hMapFile) CloseHandle(hMapFile);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    
    return result;
}
