#ifndef PAL3PATCH_LOCALE_H
#define PAL3PATCH_LOCALE_H
// PATCHAPI DEFINITIONS


extern PATCHAPI unsigned system_codepage;
extern PATCHAPI unsigned target_codepage;

enum {
    GAME_LOCALE_UNKNOWN,
    GAME_LOCALE_CHS,
    GAME_LOCALE_CHT,
    GAME_LOCALE_AUTODETECT
};
extern PATCHAPI int game_locale;

#define CODEPAGE_CHS 936
#define CODEPAGE_CHT 950


extern PATCHAPI const char *defaultfont_ftfilename;
extern PATCHAPI int defaultfont_ftindex;
extern PATCHAPI DWORD defaultfont_charset;
extern PATCHAPI int defaultfont_bold;
extern PATCHAPI const wchar_t *wstr_defaultfont;


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS


extern void init_locale_early(void);
extern void init_locale(void);

extern const wchar_t *wstr_about_title;
extern const wchar_t *wstr_about_text;
extern const wchar_t *wstr_confirmquit_text;
extern const wchar_t *wstr_confirmquit_title;
extern const wchar_t *wstr_gameloading;
extern const wchar_t *wstr_cantsavereg_text;
extern const wchar_t *wstr_cantsavereg_title;
extern const wchar_t *wstr_badregfile_text;
extern const wchar_t *wstr_badregfile_title;
extern const wchar_t *wstr_nocfgfile_text;
extern const wchar_t *wstr_nocfgfile_title;
extern const wchar_t *wstr_badcfgfile_text;
extern const wchar_t *wstr_badcfgfile_title;
extern const wchar_t *wstr_resetfailed_text;
extern const wchar_t *wstr_resetfailed_title;
extern const wchar_t *wstr_screenshot_msg;
extern const wchar_t *wstr_punctuation_nohead;
extern const wchar_t *wstr_punctuation_notail;
extern const wchar_t *wstr_punctuation_double;
extern const wchar_t *wstr_pluginerr_loadfailed;
extern const wchar_t *wstr_pluginerr_noentry;
extern const wchar_t *wstr_pluginerr_initfailed;
extern const wchar_t *wstr_pluginreport_title;
extern const wchar_t *wstr_pluginreport_template;
extern const wchar_t *wstr_pluginreport_namepart;
extern const wchar_t *wstr_pluginreport_success;
extern const wchar_t *wstr_pluginreport_failed;

#endif
#endif
