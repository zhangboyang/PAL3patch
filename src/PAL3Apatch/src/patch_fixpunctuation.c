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
    struct std_basic_string *text = TOPTR(M_DWORD(R_EBP + 0x8));
    int nexti = M_DWORD(R_EBP - 0xC) + 1;
    int fontsize = M_DWORD(R_EBP - 0x1C);
    int *x = TOPTR(R_EBP - 0x24);
    int *y = &R_EBX;
    int *col = TOPTR(R_EBP - 0x10);
    const char *out = TOPTR(R_EBP - 0x8);
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
    RECT *rect = TOPTR(R_ESI);
    char *text = TOPTR(M_DWORD(R_EBP + 0x8));
    int nexti = M_DWORD(R_EBP - 0x10) + 1;
    int fontsize = M_DWORD(R_EBP + 0x14);
    
    int *col = TOPTR(R_EBP - 0x8);
    int *x = TOPTR(R_EBP - 0x14);
    int *lnp1fontsize = TOPTR(R_EBP + 0xC);
    
    const char *out = TOPTR(R_EBP - 0x4);
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
    PUSH_DWORD(TOUINT(s));
    R_ECX = R_EDI;
}*/
/*static MAKE_ASMPATCH(uifillchar)
{
    static char *s = NULL;
    if (!s) {
        s = read_file_as_cstring("testui.txt");
    }
    M_DWORD(R_EBP + 0x8) = TOUINT(s);
    R_ESP -= 0x1C;
    PUSH_DWORD(R_ESI);
    PUSH_DWORD(R_EDI);
}*/

MAKE_PATCHSET(fixpunctuation)
{
    nohead_list = make_punc_list(wstr_punctuation_nohead);
    notail_list = make_punc_list(wstr_punctuation_notail);
    double_list = make_punc_list(wstr_punctuation_double);
    
    INIT_ASMPATCH(UITextArea_Draw_newlinetest, 0x00459D0B, 0x13, "\x7E\x11\x8B\x45\xE4\x83\x65\xF0\x00\x8D\x5C\x03\x02\x8B\x46\x10\x89\x45\xDC");
    INIT_ASMPATCH(UIDrawText_newlinetest, 0x0052A8C5, 0x11, "\x7E\x0F\x8B\x45\x14\x83\x65\xF8\x00\x01\x45\x0C\x8B\x06\x89\x45\xEC");
    
    // fill dialog text and ui text, for debug purpose
    //INIT_ASMPATCH(dlgfillchar, 0x004573DE, 5, "\xFF\x75\x08\x8B\xCF");
    //INIT_ASMPATCH(uifillchar, 0x0052A73F, 5, "\x83\xEC\x1C\x56\x57");
}
