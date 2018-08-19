#include "common.h"

struct punc_node {
    struct punc_node *next;
    char *mbch;
};

static struct punc_node *nohead_list;
static struct punc_node *notail_list;
static struct punc_node *double_list;

static struct punc_node *make_punc_list(const wchar_t *wstr)
{
    wchar_t buf[2];
    buf[1] = 0;
    
    struct punc_node *head = NULL, *tail, *node;
    while (*wstr) {
        buf[0] = *wstr++;
        
        node = malloc(sizeof(struct punc_node));
        node->next = NULL;
        node->mbch = wcs2cs_alloc(buf, target_codepage);
        assert(strlen(node->mbch) == 2);
        
        if (!head) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }
    
    return head;
}

static int in_punc_list(struct punc_node *head, const char *mbstr)
{
    while (head) {
        if (strncmp(mbstr, head->mbch, strlen(head->mbch)) == 0) {
            return 1;
        }
        head = head->next;
    }
    return 0;
}


static int newline_in_advance(const char *tailch, const char *headch)
{
    if (in_punc_list(nohead_list, headch)) return 1;
    if (in_punc_list(notail_list, tailch)) return 1;
    if (in_punc_list(double_list, tailch) && in_punc_list(double_list, headch)) return 1;
    return 0;
}

static int get_mbchar_length(const char *mbstr)
{
    if (0 < TOUCHAR(*mbstr) && TOUCHAR(*mbstr) <= 0x7F) {
        return 1;
    } else if (TOUCHAR(*mbstr) >= 0x80) {
        if (*(mbstr + 1)) {
            return 2;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

static MAKE_ASMPATCH(UITextArea_Draw_newlinetest)
{
    struct UITextArea *this = TOPTR(R_ESI);
    struct std_basic_string *text = TOPTR(M_DWORD(R_ESP + 0x40));
    int nexti = M_DWORD(R_ESP + 0x24) + 1;
    int fontsize = M_DWORD(R_ESP + 0x2C);
    int *x = TOPTR(R_ESP + 0x34);
    int *y = TOPTR(R_ESP + 0x14);
    int *col = TOPTR(R_ESP + 0x18);
    const char *out = TOPTR(R_ESP + 0x10);
    const char *nextstr = text->_Ptr ? text->_Ptr + nexti : "";
    
    int len1 = strlen(out);
    int len2 = get_mbchar_length(nextstr);
    int newline = 0;
    if (*x + fontsize / 2 * len1 > pUIWND(this)->m_rect.right) {
        newline = 1;
    } else {
        if (*x + fontsize / 2 * (len1 + len2) > pUIWND(this)->m_rect.right) {
            newline = newline_in_advance(out, nextstr);
        }
    }
    
    if (newline) {
        *col = 0;
        *y += fontsize + 2;
        *x = pUIWND(this)->m_rect.left;
    }
}

static MAKE_ASMPATCH(UIDrawText_newlinetest)
{
    RECT *rect = TOPTR(M_DWORD(R_ESP + 0x38));
    char *text = TOPTR(M_DWORD(R_ESP + 0x34));
    int nexti = R_EBP + 1;
    int fontsize = M_DWORD(R_ESP + 0x40);
    
    int *col = TOPTR(R_ESP + 0x10);
    int *x = TOPTR(R_ESP + 0x24);
    int *lnp1fontsize = TOPTR(R_ESP + 0x14);
    
    const char *out = TOPTR(R_ESP + 0x44);
    const char *nextstr = text + nexti;
    
    int len1 = strlen(out);
    int len2 = get_mbchar_length(nextstr);
    int newline = 0;
    if (*x + fontsize / 2 * len1 > rect->right) {
        newline = 1;
    } else {
        if (*x + fontsize / 2 * (len1 + len2) > rect->right) {
            newline = newline_in_advance(out, nextstr);
        }
    }
    
    if (newline) {
        *col = 0;
        *x = rect->left;
        *lnp1fontsize += fontsize;
    }
}

/*static MAKE_ASMPATCH(dlgfillchar)
{
    static char *s = NULL;
    if (!s) {
        s = read_file_as_cstring("testdlg.txt");
    }
    R_ECX = R_EBX;
    R_EAX = TOUINT(s);
}*/
/*static MAKE_ASMPATCH(uifillchar)
{
    static char *s = NULL;
    if (!s) {
        s = read_file_as_cstring("testui.txt");
    }
    M_DWORD(R_ESP + 0x2C) = TOUINT(s);
    R_EDI = M_DWORD(R_ESP + 0x2C);
    R_EBP = 0;
}*/

MAKE_PATCHSET(fixpunctuation)
{
    nohead_list = make_punc_list(wstr_punctuation_nohead);
    notail_list = make_punc_list(wstr_punctuation_notail);
    double_list = make_punc_list(wstr_punctuation_double);
    
    INIT_ASMPATCH(UITextArea_Draw_newlinetest, 0x00453776, 0x1D, "\x7E\x1B\x8B\x44\x24\x14\xC7\x44\x24\x18\x00\x00\x00\x00\x8D\x54\x08\x02\x8B\x46\x10\x89\x54\x24\x14\x89\x44\x24\x34");
    INIT_ASMPATCH(UIDrawText_newlinetest, 0x005413A4, 0x18, "\x7E\x16\x8B\x44\x24\x14\xC7\x44\x24\x10\x00\x00\x00\x00\x03\xC1\x89\x5C\x24\x24\x89\x44\x24\x14");
    
    // fill dialog text and ui text, for debug purpose
    //INIT_ASMPATCH(dlgfillchar, 0x00451215, 6, "\x8B\x44\x24\x30\x8B\xCB");
    //INIT_ASMPATCH(uifillchar, 0x00541215, 6, "\x8B\x7C\x24\x2C\x33\xED");
}
