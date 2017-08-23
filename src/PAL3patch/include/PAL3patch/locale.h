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





#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS


extern void init_locale_early(void);
extern void init_locale(void);

extern const wchar_t *wstr_about_title;
extern const wchar_t *wstr_about_text;
extern const wchar_t *wstr_confirmquit_text;
extern const wchar_t *wstr_confirmquit_title;
extern const char *ftfont_filename;
extern int ftfont_index;
extern const wchar_t *wstr_defaultfont;
extern DWORD defaultfont_charset;
extern int defaultfont_bold;
extern const wchar_t *wstr_gameloading;
extern const wchar_t *wstr_cantsavereg_text;
extern const wchar_t *wstr_cantsavereg_title;
extern const wchar_t *wstr_nocfgfile_text;
extern const wchar_t *wstr_nocfgfile_title;
extern const wchar_t *wstr_badcfgfile_text;
extern const wchar_t *wstr_badcfgfile_title;

#endif
#endif
