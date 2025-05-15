#include "common.h"

// language, see patch_setlocale.c for details
unsigned system_codepage;
unsigned target_codepage;

// game locale information, see GAME_LOCALE_XXX for details
int game_locale;

// CHS
static const wchar_t wstr_about_title_CHS[] = L"����";
static const wchar_t wstr_about_text_CHS[] = 
    L"��ӭʹ�á��ɽ������������ֱ��ʲ��� %hs\n" 
    L"\n"
    L"����������������Ϸ�ķֱ��ʣ�\n"
    L"���޸�һЩ��Ϸ�����С���⣬\n"
    L"�������һЩʵ�õ�С���ܣ�\n" 
    L"��ϸ��Ϣ��ʹ�÷�����μ�����˵���ļ���\n"
    L"\n"
    L"\n"
    L"���������ÿ������ʱ��������Ϣ��\n"
    L"��ʹ�ò������ù��ߣ�������ʾ���ڡ���Ϊ�����á���\n" 
    L"\n"
    L"\n"
    L"������վ��\n"
    L"  https://pal3.zbyzbyzby.com\n"
    L"\n"
    L"������Ϣ��\n%hs"
;

static const wchar_t wstr_confirmquit_text_CHS[] = L"��ȷ��Ҫ�˳���Ϸ��\nδ�������Ϸ���Ƚ��ᶪʧ��";
static const wchar_t wstr_confirmquit_title_CHS[] = L"�˳�";

static const char defaultfont_ftfilename_CHS[] = "simsun.ttc";
static const int defaultfont_ftindex_CHS = 0;
static const DWORD defaultfont_charset_CHS = GB2312_CHARSET;
static const int defaultfont_bold_CHS = 48;

static const wchar_t wstr_defaultfont_CHS[] = L"SimSun";
static const wchar_t wstr_defaultfont_localized_CHS[] = L"����";
static const wchar_t wstr_gameloading_CHS[] = L"��Ϸ�����У����Ժ򡭡�";

static const wchar_t wstr_cantsavereg_text_CHS[] = L"�޷�����ע���浵�ļ�����ȷ����Ϸ�Դ浵�ļ���д��Ȩ�ޣ��Ƿ����ԣ�\nע�⣺��ȡ����ǰ�����Ρ�С��Ϸ�Ľ���״̬���ܻᶪʧ��";
static const wchar_t wstr_cantsavereg_title_CHS[] = L"ע����ض���";

static const wchar_t wstr_badregfile_text_CHS[] = L"ע���浵�ļ����𻵣��Ƿ������\nע�⣺��������ǰ�����Ρ�С��Ϸ�Ľ���״̬���ܻᶪʧ��";
static const wchar_t wstr_badregfile_title_CHS[] = L"ע����ض���";

static const wchar_t wstr_cantdeltemp_text_CHS[] = L"��ϷĿ¼�´�����ʱ�ļ� %hs�����޷�ɾ�������ֶ�ɾ������";
static const wchar_t wstr_cantdeltemp_title_CHS[] = L"�޷�ɾ����ʱ�ļ�";

static const wchar_t wstr_nomutex_text_CHS[] = L"���ȹر����������е���Ϸ���򲹶����ù��ߣ���";
static const wchar_t wstr_nomutex_title_CHS[] = L"��⵽��ͻ";

static const wchar_t wstr_nocfgfile_text_CHS[] = L"�޷���ȡ���������ļ��������С��������ù��ߡ�������Ĭ�������ļ���";
static const wchar_t wstr_nocfgfile_title_CHS[] = L"�޷���������";

static const wchar_t wstr_badcfgfile_text_CHS[] = L"���������ļ����𻵡������С��������ù��ߡ���ʹ�á�ʵ�ù��ߡ����ָ�Ĭ�����á��������������������ļ���";
static const wchar_t wstr_badcfgfile_title_CHS[] = L"�޷���������";

static const wchar_t wstr_badiconv_text_CHS[] = L"���ֱ���ת�����Բ�ͨ������Ϸ�����ֿ��ܻ����롣�Ƿ���Ҫ������Ϸ��";
static const wchar_t wstr_badiconv_title_CHS[] = L"���ֱ���ת��ʧ��";

static const wchar_t wstr_pathtoolong_text_CHS[] = L"��Ϸ·��̫������������Ϸ·�����ȣ��Է���Ϸ���г������⡣";
static const wchar_t wstr_pathtoolong_title_CHS[] = L"��Ϸ·��̫��";
static const wchar_t wstr_pathnotenglish_text_CHS[] = L"��Ϸ·�����з�Ӣ���ַ����뽫��Ϸ����ȫӢ��·���£��Է���Ϸ���г������⡣";
static const wchar_t wstr_pathnotenglish_title_CHS[] = L"��Ӣ����Ϸ·��";

static const wchar_t wstr_resetfailed_text_CHS[] = L"�޷����� Direct3D �豸���Ƿ����ԣ�\n\n�����س��������ԣ�\n����ESC�����˳���Ϸ��";
static const wchar_t wstr_resetfailed_title_CHS[] = L"����ʧ��";

static const wchar_t wstr_resolutiontoolarge_text_CHS[] = L"��Ҫʹ�� %dx%d �ֱ��ʽ�����Ϸ����Ŀǰδ���� LAA �����ַ�ռ�֧�֣���\n\n�������� LAA������ķֱ��ʽ�������Ϸ���в��ȶ����޷����С�\n\n���������С��������ù��ߡ�������ͼ�����á���ͼ�νӿڡ�����Ϊ��Direct3D 9.0c (LAA)����\n\n�Ƿ���Ըþ���ǿ�н�����Ϸ��";
static const wchar_t wstr_resolutiontoolarge_title_CHS[] = L"�������ͼ�νӿ�";

static const wchar_t wstr_nogfxmgr_text_CHS[] = L"�Կ��豸��ʼ��ʧ�ܣ������Կ����������Ƿ�����������ͼ�������Ƿ���ʣ���";
static const wchar_t wstr_nogfxmgr_badtools_text_CHS[] = L"�Կ��豸��ʼ��ʧ�ܣ�����������¹��ߵ��µģ�\n\n%s\n��رջ�������ǣ�Ȼ������һ�Ρ�";
static const wchar_t wstr_nogfxmgr_title_CHS[] = L"��Ϸ��ʼ��ʧ��";

static const wchar_t wstr_screenshot_msg_CHS[] = L"��Ļ��ͼ�ѱ�������%hs��";

static const wchar_t wstr_punctuation_nohead_CHS[] = L"������������������������������";
static const wchar_t wstr_punctuation_notail_CHS[] = L"��������������";
static const wchar_t wstr_punctuation_double_CHS[] = L"����";

static const wchar_t wstr_pluginerr_loadfailed_CHS[] = L"�޷�����ģ��";
static const wchar_t wstr_pluginerr_noentry_CHS[] = L"����ڵ�";
static const wchar_t wstr_pluginerr_initfailed_CHS[] = L"��ʼ��ʧ�ܣ�����ֵΪ %d";
static const wchar_t wstr_pluginreport_title_CHS[] = L"������ر���";
static const wchar_t wstr_pluginreport_template_CHS[] = L"�����ǲ�����������\n\n%s\n�ѳɹ����� %d �����ģ�顣";
static const wchar_t wstr_pluginreport_namepart_CHS[] = L"%s%hs%hs";
static const wchar_t wstr_pluginreport_success_CHS[] = L"���ɹ��� %s\n";
static const wchar_t wstr_pluginreport_failed_CHS[] = L"��ʧ�ܡ� %s ��%s��\n";

static const wchar_t wstr_havebadtool_text_CHS[] = L"��⵽���¹����ƺ��������У�\n\n%s\n��Щ���߿��ܻ�Ӱ��ֱ��ʲ��������������������رջ�������ǡ�\n\n������Ѿ��رջ��������Щ���ߣ��ɺ��Ա���Ϣ��\n\n�Ƿ���Ҫ������Ϸ��";
static const wchar_t wstr_havebadtool_title_CHS[] = L"��⵽�����ݵĹ���";
static const wchar_t wstr_badtool_dxwnd_CHS[] = L"DxWnd ���ڻ�����";
static const wchar_t wstr_badtool_d3dwindower_CHS[] = L"D3DWindower ���ڻ�����";

static const wchar_t wstr_badgamever_text_CHS[] = L"��⵽ 1.04 ����δ��ȷ��װ��\n\n�����С��������ù��ߡ���ʹ�á�ʵ�ù��ߡ����޸���Ϸ���ݡ���������������⡣\n\n���������ͬʱʹ������ MOD���ɺ��Ա���Ϣ��\n\n�Ƿ���Ҫ������Ϸ��";
static const wchar_t wstr_badgamever_title_CHS[] = L"��Ϸ�汾����ȷ";


// CHT
static const char defaultfont_ftfilename_CHT[] = "mingliu.ttc";
static const int defaultfont_ftindex_CHT = 0;
static const DWORD defaultfont_charset_CHT = CHINESEBIG5_CHARSET;
static const int defaultfont_bold_CHT = 32;

static const wchar_t wstr_defaultfont_CHT[] = L"MingLiU";
static const wchar_t wstr_defaultfont_localized_CHT[] = L"�����w";
static const wchar_t wstr_gameloading_CHT[] = L"�[���d���У�Ո�Ժ򡭡�";

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
const wchar_t *wstr_pathnotenglish_text;
const wchar_t *wstr_pathnotenglish_title;
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







static int detect_game_locale(void);

#define IMPORT_LOCALE_ITEM(lang, symbol) ((symbol) = (CONCAT3(symbol, _, lang)))

void init_locale_early()
{
    // init codepage
    // PATCHSET 'setlocale' may overwrite target_codepage
    target_codepage = system_codepage = GetACP();

    // detect game locale
    game_locale = detect_game_locale();
    
    
    // init early strings
    IMPORT_LOCALE_ITEM(CHS, wstr_cantdeltemp_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_cantdeltemp_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_nomutex_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_nomutex_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_nocfgfile_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_nocfgfile_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_badcfgfile_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_badcfgfile_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_badiconv_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_badiconv_title);
    
    if (target_codepage == CODEPAGE_CHT) { // CHT
        // FIXME
    }
}

void init_locale()
{
    const wchar_t *wstr_defaultfont_localized;

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
    IMPORT_LOCALE_ITEM(CHS, wstr_defaultfont_localized);
    IMPORT_LOCALE_ITEM(CHS, wstr_gameloading);
    IMPORT_LOCALE_ITEM(CHS, wstr_cantsavereg_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_cantsavereg_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_badregfile_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_badregfile_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_pathtoolong_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_pathtoolong_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_pathnotenglish_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_pathnotenglish_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_resetfailed_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_resetfailed_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_resolutiontoolarge_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_resolutiontoolarge_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_nogfxmgr_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_nogfxmgr_badtools_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_nogfxmgr_title);
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
    IMPORT_LOCALE_ITEM(CHS, wstr_havebadtool_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_havebadtool_title);
    IMPORT_LOCALE_ITEM(CHS, wstr_badtool_dxwnd);
    IMPORT_LOCALE_ITEM(CHS, wstr_badtool_d3dwindower);
    IMPORT_LOCALE_ITEM(CHS, wstr_badgamever_text);
    IMPORT_LOCALE_ITEM(CHS, wstr_badgamever_title);

    if (target_codepage == CODEPAGE_CHT) { // CHT
        IMPORT_LOCALE_ITEM(CHT, defaultfont_ftfilename);
        IMPORT_LOCALE_ITEM(CHT, defaultfont_ftindex);
        IMPORT_LOCALE_ITEM(CHT, defaultfont_charset);
        IMPORT_LOCALE_ITEM(CHT, defaultfont_bold);
        IMPORT_LOCALE_ITEM(CHT, wstr_defaultfont);
        IMPORT_LOCALE_ITEM(CHT, wstr_defaultfont_localized);
        IMPORT_LOCALE_ITEM(CHT, wstr_gameloading);
        // FIXME
    }

    if (is_win9x() && system_codepage == target_codepage) {
        wstr_defaultfont = wstr_defaultfont_localized;
    }
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
