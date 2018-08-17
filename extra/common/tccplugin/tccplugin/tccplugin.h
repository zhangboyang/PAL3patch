#ifndef TCCPLUGIN_H
#define TCCPLUGIN_H

#include "libtcc.h"

#ifdef TCCPLUGINAPI_EXPORTS
#define TCCPLUGINAPI __declspec(dllexport)
#else
#define TCCPLUGINAPI __declspec(dllimport)
#pragma comment(lib, "libtcc")
#pragma comment(lib, "tccplugin")
#endif


extern TCCPLUGINAPI UINT tccplugin_default_codepage;



struct cpi { // C plugin instance
    DECL_PLUGINENTRY(*entry);
    TCCState *tcc;
    void *runmem;
    struct cstr tccmsg;
    struct cstr srclist;
    int err_flag;
    int user_ignore_err;
};

extern TCCPLUGINAPI void cpi_ctor(struct cpi *self);
extern TCCPLUGINAPI void cpi_dtor(struct cpi *self);
extern TCCPLUGINAPI int cpi_add_c_source(struct cpi *self, const char *filepath);
extern TCCPLUGINAPI int cpi_link(struct cpi *self);
extern TCCPLUGINAPI int cpi_run(struct cpi *self, const char *entryname);
extern TCCPLUGINAPI void cpi_make_persist(struct cpi *self);
extern TCCPLUGINAPI void cpi_try_dump_tccmsg(struct cpi *self);
extern TCCPLUGINAPI void cpi_ask_quit_if_error(struct cpi *self);



extern TCCPLUGINAPI int run_c_program(const char *filepath, const char *entrysymbol, int persist);
extern TCCPLUGINAPI void search_cplugins(const char *dirpath, struct cstr *filelist);


#if defined(TCCPLUGINAPI_EXPORTS) || defined(USE_TCCPLUGIN_DEFINITIONS)

#define TCCPLUGIN_MSGBOX_TITLE "TCC Plugin for PAL3patch"
#define TCCPLUGIN_INSTALL_PATH "plugins\\tcc"

#endif

#endif
