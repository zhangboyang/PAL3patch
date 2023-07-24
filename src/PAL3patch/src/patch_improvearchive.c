#include "common.h"

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
    ARC_FILE_COUNT // EOF
};
struct arc_wal {
    char *dst[ARC_FILE_COUNT];
    char *src[ARC_FILE_COUNT];
    char *sum;
};
static void arc_wal_init(struct arc_wal *w, const char *arc)
{
    w->dst[ARC_FILE_ARCHIVE] = strdup(arc);                                          // foo/barXY.arc
    w->src[ARC_FILE_ARCHIVE] = replace_extension(w->dst[ARC_FILE_ARCHIVE], ".wal");  // foo/barXY.wal
    
    w->dst[ARC_FILE_PICTURE] = fmt_arc(arc, "PAL3_00%s.JPG");                        // foo/PAL3_00XY.JPG
    w->src[ARC_FILE_PICTURE] = replace_extension(w->dst[ARC_FILE_PICTURE], ".wal");  // foo/PAL3_00XY.wal
    
    w->sum = replace_extension(w->dst[ARC_FILE_ARCHIVE], ".sum");  // foo/barXY.sum
}
static int arc_wal_check(struct arc_wal *w)
{
    return wal_check(w->dst, w->src, ARC_FILE_COUNT, w->sum);
}
static int arc_wal_replace(struct arc_wal *w)
{
    return wal_replace(w->dst, w->src, ARC_FILE_COUNT, w->sum);
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

static struct arc_wal wr;

static FILE *my_fopen(const char *filename, const char *mode)
{
    if (is_arc(filename)) {
        if (*mode == 'r') {
            struct arc_wal rd;
            arc_wal_init(&rd, filename);
            arc_wal_check(&rd);
            arc_wal_free(&rd);
        }
        if (*mode == 'w') {
            arc_wal_free(&wr);
            arc_wal_init(&wr, filename);
            filename = wr.src[ARC_FILE_ARCHIVE];
            reset_attrib(filename);
        }
    }
    return pal3fsopen(filename, mode, 0x40);
}

static MAKE_THISCALL(BOOL, Archive_Save_wrapper, struct Archive *this, int index)
{
    BOOL ret = Archive_Save(this, index);
    if (ret) {
        char *jpgfile = wr.src[ARC_FILE_PICTURE];
        reset_attrib(jpgfile);
        gbImage2D_WriteJpegImage(&PAL3_m_screenImg, jpgfile, 100);
        if (!file_exists(jpgfile)) {
            FILE *fp = fopen(jpgfile, "wb");
            safe_fclose(&fp);
        }
        if (!arc_wal_replace(&wr)) ret = FALSE;
        arc_wal_free(&wr);
    }
    return ret;
}

MAKE_PATCHSET(improvearchive)
{
    make_jmp(0x00553A81, my_fopen);
    
    INIT_WRAPPER_CALL(Archive_Save_wrapper, { 0x004B295A });
    SIMPLE_PATCH_NOP(0x004B2A0A, "\x6A\x64\x52\xFF\x15\x9C\xA3\x56\x00", 9);
}
