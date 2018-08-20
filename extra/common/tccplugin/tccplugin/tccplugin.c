#define PLUGIN_INTERNAL_NAME "TCCHOST"
#define PLUGIN_FRIENDLY_NAME "TCC 插件"
#define PLUGIN_VERSION       "v1.1" 

#ifdef BUILD_FOR_PAL3
#define USE_PAL3_DEFINITIONS
#include "PAL3patch.h"
#endif

#ifdef BUILD_FOR_PAL3A
#define USE_PAL3A_DEFINITIONS
#include "PAL3Apatch.h"
#endif

#define TCCPLUGINAPI_EXPORTS
#include "tccplugin.h"


UINT tccplugin_default_codepage = 936;


static void tccmsg_to_cstr(void *opaque, const char *msg)
{
    struct cstr *s = opaque;
    cstr_strcat(s, msg);
    cstr_strcat(s, "\n");
}


void cpi_ctor(struct cpi *self)
{
    // constructor
    memset(self, 0, sizeof(*self));
    cstr_ctor(&self->tccmsg);
    cstr_ctor(&self->srclist);
    self->tcc = tcc_new();
    
    // initialize tcc
    tcc_set_error_func(self->tcc, &self->tccmsg, tccmsg_to_cstr);
    tcc_set_output_type(self->tcc, TCC_OUTPUT_MEMORY);
    tcc_set_options(self->tcc, "-Wall -fms-extensions -mms-bitfields");
}
void cpi_dtor(struct cpi *self)
{
    cpi_ask_quit_if_error(self);
    
    // destructor
    if (self->tcc) tcc_delete(self->tcc);
    if (self->runmem) VirtualFree(self->runmem, 0, MEM_RELEASE);
    cstr_dtor(&self->tccmsg);
    cstr_dtor(&self->srclist);
}

int cpi_add_c_source(struct cpi *self, const char *filepath)
{
    struct cstr dllpath; cstr_ctor(&dllpath);
    struct wstr wincpath; wstr_ctor(&wincpath);
    struct cstr incpath; cstr_ctor(&incpath);
    wchar_t *wincpath_filepart;
    int ret = 0;
    char *origtext = NULL;
    char *srctext = NULL;
    int r;
    UINT cp;

    plog("CC %s", filepath);
    log_enter();
    
    cstr_strcat(&self->srclist, get_filepart(filepath));
    cstr_strcat(&self->srclist, "\n");
    
    if (self->err_flag) {
        plog("can't continue due to previous error.");
        goto fail;
    }
    
    if (str_iendswith(filepath, ".c")) {
        cstr_strcpy(&dllpath, filepath);
        cstr_pop(&dllpath, 2);
        cstr_strcat(&dllpath, ".dll");
        if (enum_files(cstr_getstr(&dllpath), NULL, NULL, NULL)) {
            plog("refuse to compile '%s' because '%s' exists.", filepath, cstr_getstr(&dllpath));
            try_goto_desktop();
            MessageBoxW_utf8format(NULL, "拒绝编译和加载 '%s'，因为 '%s' 已经存在，若重复加载可能造成冲突。", TCCPLUGIN_MSGBOX_TITLE, MB_ICONWARNING, filepath, cstr_getstr(&dllpath));
            self->user_ignore_err = 1;
            goto fail;
        }
    }

    // load file data
    origtext = read_file_as_cstring(filepath);
    if (!origtext) {
        plog("can't load file.");
        goto fail;
    }

    cp = str_remove_utf8_bom(origtext) ? CP_UTF8 : tccplugin_default_codepage;
    cs2cs_managed(origtext, cp, CP_UTF8, &srctext);
    
    // add file path to include path
    if (utf8_filepath_to_wstr_fullpath(filepath, wstr_getbuffer(&wincpath, MAXLINE), MAXLINE, &wincpath_filepart)) {
        if (wincpath_filepart) *wincpath_filepart = 0;
        wstr_commitbuffer(&wincpath);
        cstr_wcs2cs(&incpath, wstr_getwcs(&wincpath), CP_ACP);
        if (!cstr_empty(&incpath)) tcc_add_include_path(self->tcc, cstr_getstr(&incpath));
    } else {
        wstr_discardbuffer(&wincpath);
    }
    
    // do compile
    cstr_clear(&self->tccmsg);
    r = tcc_compile_string(self->tcc, srctext);
    cpi_try_dump_tccmsg(self);
    
    if (r == -1) {
        try_goto_desktop();
        MessageBoxW_utf8format(NULL, "编译 '%s' 失败：\n\n%s", TCCPLUGIN_MSGBOX_TITLE, MB_ICONWARNING, filepath, cstr_getstr(&self->tccmsg));
        goto fail;
    }

    if (!cstr_empty(&self->tccmsg)) {
        plog("warnings generated.");
    }
    ret = 1;
    
done:
    log_leave();
    cstr_dtor(&dllpath);
    wstr_dtor(&wincpath);
    cstr_dtor(&incpath);
    patch_free(origtext);
    patch_free(srctext);
    return ret;
fail:
    plog("compile error.");
    self->err_flag = 1;
    goto done;
}



int cpi_link(struct cpi *self)
{
    int ret = 0;
    int r;
    
    const char **pp;
    const char *defaultlib[] = {
        "gdi32", "comdlg32", "user32", "kernel32", "advapi32", "shell32",
#ifdef BUILD_FOR_PAL3
        "PAL3patch",
#endif
#ifdef BUILD_FOR_PAL3A
        "PAL3Apatch",
#endif
        NULL // EOF
    };
    
    plog("LD");
    log_enter();
    if (self->err_flag) {
        plog("can't continue due to previous error.");
        goto fail;
    }
    
    // add default libraries
    cstr_clear(&self->tccmsg);
    for (pp = defaultlib; *pp; pp++) {
        if (tcc_add_library(self->tcc, *pp) == -1) {
            cpi_try_dump_tccmsg(self);
            plog("can't add default library '%s'.", *pp);
            goto fail;
        }
    }
    
    // first pass: get mem size
    cstr_clear(&self->tccmsg);
    r = tcc_relocate(self->tcc, NULL);
    cpi_try_dump_tccmsg(self);
    
    if (r == -1) {
        try_goto_desktop();
        MessageBoxW_utf8format(NULL, "已编译：\n\n%s\n但链接失败：\n\n%s", TCCPLUGIN_MSGBOX_TITLE, MB_ICONWARNING, cstr_getstr(&self->srclist), cstr_getstr(&self->tccmsg));
        goto fail;
    }
    
    // alloc memory
    assert(!self->runmem);
    self->runmem = VirtualAlloc(NULL, r, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!self->runmem) {
        plog("error: VirtualAlloc() failed.");
        goto fail;
    }
    
    // second pass
    r = tcc_relocate(self->tcc, self->runmem);
    if (r == -1) goto fail;
    
done:
    log_leave();
    return ret;
fail:
    plog("link error.");
    self->err_flag = 1;
    goto done;
}

int cpi_run(struct cpi *self, const char *entryname)
{
    int ret = 0;
    int r;
    
    plog("RUN");
    log_enter();
    if (self->err_flag) {
        plog("can't continue due to previous error.");
        goto fail;
    }
    
    self->entry = tcc_get_symbol(self->tcc, entryname);
    if (!self->entry) {
        plog("no such symbol: %s", entryname);
        try_goto_desktop();
        MessageBoxW_utf8format(NULL, "已编译和链接：\n\n%s\n但未找到入口函数 %s()。", TCCPLUGIN_MSGBOX_TITLE, MB_ICONWARNING, cstr_getstr(&self->srclist), entryname);
        goto fail;
    }
    
    r = self->entry();
        
    if (r != 0) {
        plog("procedure returned %d.", r);
        try_goto_desktop();
        MessageBoxW_utf8format(NULL, "已编译和链接：\n\n%s\n但 %s() 运行失败，返回值为 %d。", TCCPLUGIN_MSGBOX_TITLE, MB_ICONWARNING, cstr_getstr(&self->srclist), entryname, r);
        goto fail;
    }
    
    ret = 1;
    
done:
    log_leave();
    return ret;
fail:
    plog("run error.");
    self->err_flag = 1;
    goto done;
}

void cpi_make_persist(struct cpi *self)
{
    if (!self->err_flag) {
        self->tcc = NULL;
        self->runmem = NULL;
    }
}

void cpi_try_dump_tccmsg(struct cpi *self)
{
    if (!cstr_empty(&self->tccmsg)) {
        log_enter();
        plog("%s", cstr_getstr(&self->tccmsg));
        log_leave();
    }
}

void cpi_ask_quit_if_error(struct cpi *self)
{
    if (self->err_flag && !self->user_ignore_err) {
        try_goto_desktop();
        if (MessageBoxW_utf8format(NULL, "编译加载过程中有错误发生，是否继续？\n具体错误信息请查看日志文件 '%s'。", TCCPLUGIN_MSGBOX_TITLE, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2, PLUGIN_LOG_FILE) == IDNO) {
            fail("用户中止。");
        } else {
            self->user_ignore_err = 1;
        }
    }
}


int run_c_program(const char *filepath, const char *entrysymbol, int persist)
{
    plog("running program '%s', entry '%s'.", filepath, entrysymbol);
    log_enter();
    int ret;
    struct cpi s; cpi_ctor(&s);
    struct cstr filepart_string; cstr_ctor(&filepart_string);
    
    cstr_format(&filepart_string, "\"%s\"", get_filepart(filepath));
    tcc_define_symbol(s.tcc, "TCCPLUGIN_FILE", cstr_getstr(&filepart_string));
    
    #ifdef BUILD_FOR_PAL3
    tcc_define_symbol(s.tcc, "BUILD_FOR_PAL3", "1");
    #endif
    #ifdef BUILD_FOR_PAL3A
    tcc_define_symbol(s.tcc, "BUILD_FOR_PAL3A", "1");
    #endif
    
    cpi_add_c_source(&s, filepath);
    cpi_link(&s);
    cpi_run(&s, entrysymbol);
    if (s.err_flag) {
        plog("error occured.");
        ret = 0;
    } else {
        plog("program finished successfully.");
        ret = 1;
    }
    if (persist) cpi_make_persist(&s);
    cpi_dtor(&s);
    cstr_dtor(&filepart_string);
    log_leave();
    return ret;
}


static void load_cplugin(const char *filepath, void *filelist)
{
    if (run_c_program(filepath, TOSTR(PLUGINSYMBOL_ENTRY), 1)) {
        if (filelist) {
            cstr_strcat(filelist, strrchr(filepath, '\\') ? strrchr(filepath, '\\') + 1 : filepath);
            cstr_strcat(filelist, "\n");
        }
    }
}
void search_cplugins(const char *dirpath, struct cstr *filelist)
{
    plog("searching C sources in directory '%s' ...", dirpath);
    log_enter();
    enum_files(dirpath, "*.c", load_cplugin, filelist);
    plog("search finished.");
    log_leave();
}


MAKE_PLUGINABOUT()

MAKE_PLUGINENTRY()
{
#ifdef BUILD_FOR_PAL3
    PAL3_STRUCT_SELFCHECK();
#endif
#ifdef BUILD_FOR_PAL3A
    PAL3A_STRUCT_SELFCHECK();
#endif

    if (run_c_program(TCCPLUGIN_INSTALL_PATH "\\init.c", "tccplugin_main", 1)) {
        return 0;
    } else {
        return 1;
    }
}
