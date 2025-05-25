#include "common.h"

#define MAX_TASKPIC_INDEX 1000
#define MAX_TASKPIC_SIZE (16 * 1024 * 1024)

static const char taskpic_hdr_magic[8] = "TASKPIC";
struct taskpic_hdr {
    char magic[8];
    int count;
};
struct taskpic_pichdr {
    int index;
    int size;
    FILETIME lastwrite;
};
static const FILETIME nulltime;

static void load_taskpic(const char *filename)
{
    FILE *fp = NULL;
    FILE *picfp = NULL;
    int bufsize = 0;
    char *buf = NULL;
    
    int i;
    struct taskpic_hdr hdr;
    
    create_dir("BaseData");
    create_dir("BaseData\\ui");
    create_dir("BaseData\\ui\\task");
    
    fp = fopen(filename, "rb");
    if (!fp) goto fail;
    
    if (fread(&hdr, sizeof(hdr), 1, fp) != 1) goto fail;
    if (memcmp(hdr.magic, taskpic_hdr_magic, sizeof(taskpic_hdr_magic)) != 0) goto fail;
    if (hdr.count > MAX_TASKPIC_INDEX) goto fail;
    
    for (i = 0; i < hdr.count; i++) {
        struct taskpic_pichdr pichdr;
        char picname[128];
        
        if (fread(&pichdr, sizeof(pichdr), 1, fp) != 1) goto fail;
        if (pichdr.index < 0 || pichdr.index >= MAX_TASKPIC_INDEX) goto fail;
        if (pichdr.size < -1 || pichdr.size > MAX_TASKPIC_SIZE) goto fail;
        
        if (pichdr.size < 0) goto skip;
        
        if (pichdr.size > bufsize) {
            free(buf);
            bufsize = pichdr.size;
            buf = malloc(bufsize);
        }
        
        if (pichdr.size > 0) {
            if (fread(buf, 1, pichdr.size, fp) != (size_t) pichdr.size) goto fail;
        }
        
        sprintf(picname, "BaseData\\ui\\task\\task%03d.JPG", pichdr.index);
        reset_attrib(picname);
        picfp = fopen(picname, "wb");
        if (!picfp) goto skip;
        
        if (pichdr.size > 0) {
            if (fwrite(buf, 1, pichdr.size, picfp) != (size_t) pichdr.size) goto skip;
        }
        
        if (memcmp(&pichdr.lastwrite, &nulltime, sizeof(FILETIME)) != 0) {
            SetFileTime((HANDLE) _get_osfhandle(_fileno(picfp)), NULL, NULL, &pichdr.lastwrite);
        }
        
skip:
        safe_fclose(&picfp);
    }
    
fail:
    safe_fclose(&picfp);
    safe_fclose(&fp);
    free(buf);
}

static void save_taskpic(const char *filename)
{
    FILE *fp = NULL;
    FILE *picfp = NULL;
    int bufsize = 0;
    char *buf = NULL;
    
    struct taskpic_hdr hdr;

    fp = robust_fopen(filename, "wb");
    if (!fp) goto fail;
    
    memset(&hdr, 0, sizeof(hdr));
    if (fwrite(&hdr, sizeof(hdr), 1, fp) != 1) goto fail;
    
    memcpy(hdr.magic, taskpic_hdr_magic, sizeof(taskpic_hdr_magic));
    struct RoleTaskLine *it;
    for (it = PlayerMgr_m_RoleTask.m_TaskLine._First; it && it != PlayerMgr_m_RoleTask.m_TaskLine._Last; it++) {
        struct taskpic_pichdr pichdr;
        char picname[128];
        long picsize;

        hdr.count++;
        
        memset(&pichdr, 0, sizeof(pichdr));
        pichdr.index = it->ID;
        
        sprintf(picname, "BaseData\\ui\\task\\task%03d.JPG", pichdr.index);
        picfp = fopen(picname, "rb");
        if (!picfp) goto skip;
        
        if (!GetFileTime((HANDLE) _get_osfhandle(_fileno(picfp)), NULL, NULL, &pichdr.lastwrite)) {
            memset(&pichdr.lastwrite, 0, sizeof(pichdr.lastwrite));
        }
        
        if (fseek(picfp, 0, SEEK_END) != 0) goto skip;
        picsize = ftell(picfp);
        if (picsize < 0 || picsize > MAX_TASKPIC_SIZE) goto skip;
        if (fseek(picfp, 0, SEEK_SET) != 0) goto skip;
        pichdr.size = picsize;
        
        if (pichdr.size > bufsize) {
            free(buf);
            bufsize = pichdr.size;
            buf = malloc(bufsize);
        }
        
        if (pichdr.size > 0) {
            if (fread(buf, 1, pichdr.size, picfp) != (size_t) pichdr.size) goto skip;
        }
        safe_fclose(&picfp);
        
        if (fwrite(&pichdr, sizeof(pichdr), 1, fp) != 1) goto fail;
        if (pichdr.size > 0) {
            if (fwrite(buf, 1, pichdr.size, fp) != (size_t) pichdr.size) goto fail;
        }
        
        continue;
skip:
        safe_fclose(&picfp);
        memset(&pichdr, 0, sizeof(pichdr));
        pichdr.index = it->ID;
        pichdr.size = -1;
        if (fwrite(&pichdr, sizeof(pichdr), 1, fp) != 1) goto fail;
    }
    
    if (fseek(fp, 0, SEEK_SET) != 0) goto fail;
    if (fwrite(&hdr, sizeof(hdr), 1, fp) != 1) goto fail;
    
fail:
    safe_fclose(&picfp);
    safe_fclose(&fp);
    free(buf);
}


static int is_arc(const char *filename)
{
    // "foo/barXY.arc"
    if (strlen(filename) < 6) return 0;
    const char *idpart = filename + (strlen(filename) - 6);
    if (!is_digitchar(idpart[0]) || !is_digitchar(idpart[1])) return 0;
    return stricmp(idpart + 2, ".arc") == 0;
}
static char *fmt_arc(const char *arc, const char *fmt) // fmt should contain one "%s"
{
    // "PAL3_00%s.JPG": "foo/barXY.arc" => "foo/PAL3_00XY.JPG"
    char *ret = malloc(strlen(arc) + strlen(fmt) + 1); // not accurate but safe
    strcpy(ret, arc);
    char *filepart = get_filepart(ret);
    char id[3];
    strncpy(id, filepart + strlen(filepart) - 6, 2);
    id[2] = 0;
    sprintf(filepart, fmt, id);
    return ret;
}

enum {
    ARC_FILE_ARCHIVE,  // palXY.arc
    ARC_FILE_PICTURE,  // PAL3_00XY.JPG
    ARC_FILE_TASKPIC,  // taskXY.dat
    ARC_FILE_COUNT // EOF
};
#define ARC_MODE_PAL3       ((1<<ARC_FILE_ARCHIVE) | (1<<ARC_FILE_PICTURE))
#define ARC_MODE_PAL3A      ((1<<ARC_FILE_ARCHIVE) | (1<<ARC_FILE_PICTURE) | (1<<ARC_FILE_TASKPIC))
#define ARC_MODE_PAL3A_FIN  ((1<<ARC_FILE_ARCHIVE) | (1<<ARC_FILE_TASKPIC))
struct arc_wal {
    int mode;
    char *dst[ARC_FILE_COUNT];
    char *src[ARC_FILE_COUNT];
    char *sum;
    
    int n;
    char *dstv[ARC_FILE_COUNT];
    char *srcv[ARC_FILE_COUNT];
};
static void arc_wal_init(struct arc_wal *w, const char *arc)
{
    memset(w, 0, sizeof(*w));
    
    if (stricmp(get_filepart(arc), "pal3a98.arc") == 0) {
        w->mode = ARC_MODE_PAL3A_FIN;
    } else if (str_istartswith(get_filepart(arc), "pal3a")) {
        w->mode = ARC_MODE_PAL3A;
    } else {
        w->mode = ARC_MODE_PAL3;
    }
    
    w->dst[ARC_FILE_ARCHIVE] = strdup(arc);                                          // foo/barXY.arc
    w->src[ARC_FILE_ARCHIVE] = replace_extension(w->dst[ARC_FILE_ARCHIVE], ".wal");  // foo/barXY.wal
    
    w->dst[ARC_FILE_PICTURE] = fmt_arc(arc, "PAL3_00%s.JPG");                        // foo/PAL3_00XY.JPG
    w->src[ARC_FILE_PICTURE] = replace_extension(w->dst[ARC_FILE_PICTURE], ".wal");  // foo/PAL3_00XY.wal
    
    w->dst[ARC_FILE_TASKPIC] = fmt_arc(arc, "task%s.dat");                           // foo/taskXY.dat
    w->src[ARC_FILE_TASKPIC] = replace_extension(w->dst[ARC_FILE_TASKPIC], ".wal");  // foo/taskXY.wal
    
    w->sum = replace_extension(w->dst[ARC_FILE_ARCHIVE], ".sum");  // foo/barXY.sum
    
    int i;
    w->n = 0;
    for (i = 0; i < ARC_FILE_COUNT; i++) {
        if ((w->mode & (1 << i))) {
            w->dstv[w->n] = w->dst[i];
            w->srcv[w->n] = w->src[i];
            w->n++;
        }
    }
}
static int arc_wal_check(struct arc_wal *w)
{
    return wal_check(w->dstv, w->srcv, w->n, w->sum);
}
static int arc_wal_replace(struct arc_wal *w)
{
    return wal_replace(w->dstv, w->srcv, w->n, w->sum);
}
static void arc_wal_free(struct arc_wal *w)
{
    int i;
    for (i = 0; i < ARC_FILE_COUNT; i++) {
        free(w->dst[i]);
        free(w->src[i]);
    }
    free(w->sum);
    memset(w, 0, sizeof(*w));
}

static struct arc_wal rd;
static struct arc_wal wr;

static FILE *my_fopen(const char *filename, const char *mode)
{
    if (is_arc(filename)) {
        if (*mode == 'r') {
            arc_wal_free(&rd);
            arc_wal_init(&rd, filename);
            arc_wal_check(&rd);
        }
        if (*mode == 'w') {
            arc_wal_free(&wr);
            arc_wal_init(&wr, filename);
            filename = wr.src[ARC_FILE_ARCHIVE];
            reset_attrib(filename);
        }
    }
    return pal3afsopen(filename, mode, 0x40);
}

static MAKE_THISCALL(BOOL, Archive_Save_wrapper, struct Archive *this, int index)
{
    BOOL ret = Archive_Save(this, index);
    if (ret) {
        if (wr.mode != ARC_MODE_PAL3A_FIN) {
            char *jpgfile = wr.src[ARC_FILE_PICTURE];
            reset_attrib(jpgfile);
            gbImage2D_WriteJpegImage(&PAL3_m_screenImg, jpgfile, 100);
            if (!file_exists(jpgfile)) {
                FILE *fp = fopen(jpgfile, "wb");
                safe_fclose(&fp);
            }
        }
        save_taskpic(wr.src[ARC_FILE_TASKPIC]);
        if (!arc_wal_replace(&wr)) ret = FALSE;

        arc_wal_free(&wr);
    }
    return ret;
}

static MAKE_THISCALL(BOOL, Archive_Load_wrapper, struct Archive *this, int index)
{
    BOOL ret = Archive_Load(this, index);
    if (ret) {
        load_taskpic(rd.dst[ARC_FILE_TASKPIC]);
        arc_wal_free(&rd);
    }
    return ret;
}

static MAKE_THISCALL(BOOL, my_Archive_Delete, struct Archive *this, int index)
{
    struct arc_wal rm;
    char s[128];
    BOOL ret = PrepareDir();
    if (ret) {
        sprintf(s, "save\\pal3a%02d.arc", index);
        arc_wal_init(&rm, s);
        arc_wal_check(&rm);
        if (!robust_delete((const char **) rm.dst, ARC_FILE_COUNT)) ret = FALSE;
        arc_wal_free(&rm);
    }
    return ret;
}

MAKE_PATCHSET(improvearchive)
{
    make_jmp(0x00541DFE, my_fopen);
    
    INIT_WRAPPER_CALL(Archive_Save_wrapper, { 0x004A25B7 });
    SIMPLE_PATCH_NOP(0x004A2676, "\x6A\x64\x50\xFF\x15\x38\x82\x55\x00", 9);
    
    INIT_WRAPPER_CALL(Archive_Load_wrapper, { 0x0045C91A, 0x004A2519 });
    
    make_jmp(0x005236EC, my_Archive_Delete);
}
