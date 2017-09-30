#include "common.h"

#define MAX_TEX_NAME 0x200
#define TEXLIB_MAGIC "TEXLIB"

static char *clamp_list[] = {
    "ui\\GameMainUI\\State\\Portrait0.tga",
    "ui\\GameMainUI\\State\\Portrait1.tga",
    "ui\\GameMainUI\\State\\Portrait2.tga",
    "ui\\GameMainUI\\State\\Portrait3.tga",
    "ui\\GameMainUI\\State\\Portrait4.tga",
    
    "ui\\GameMainUI\\Communal\\StpCtl\\StpBarL.tga",
    "ui\\GameMainUI\\Communal\\StpCtl\\StpBarM.tga",
    "ui\\GameMainUI\\Communal\\StpCtl\\StpBarR.tga",
    
    "ui\\GameMainUI\\Communal\\EquipTopFrame\\TopFrameLS.tga",
    "ui\\GameMainUI\\Communal\\EquipTopFrame\\TopFrameLM.tga",
    "ui\\GameMainUI\\Communal\\EquipTopFrame\\TopFrameRM.tga",
    "ui\\GameMainUI\\Communal\\PropTopFrame\\TopFrameRM.tga",
    "ui\\GameMainUI\\Communal\\EquipTopFrame\\TopFrameR.tga",
    "ui\\GameMainUI\\Communal\\EquipTopFrame\\TopFrameM.tga",
    "ui\\GameMainUI\\Communal\\EquipTopFrame\\TopFrameL.tga",
    
    "ui\\GameMainUI\\Communal\\Frame_Line.tga",
    
    "ui\\GameMainUI\\Communal\\Scroll\\scrollbar0.tga",
    "ui\\GameMainUI\\Communal\\Scroll\\scrollbar1.tga",
    "ui\\GameMainUI\\Communal\\Scroll\\scrollbar2.tga",
    
    "ui\\Menu\\yun16.tga",
};
static int in_clamp_list(const char *filename)
{
    int i;
    int clamp_list_cnt = sizeof(clamp_list) / sizeof(clamp_list[0]);
    for (i = 0; i < clamp_list_cnt; i++) {
        if (stricmp(clamp_list[i], filename) == 0) {
            return 1;
        }
    }
    return 0;
}

// texture lib image manager
struct tex_lib_image {
    struct tex_lib_image *next;
    char *lib_name;
    void *bits;
    int width;
    int height;
    int bitcount;
    int div_alpha;
};
static struct tex_lib_image *head = NULL;
// insert a image to linked-list
static void insert_image(struct tex_lib_image *img)
{
    img->next = head;
    head = img;
}
// find a image in linked-list
static struct tex_lib_image *find_image(const char *lib_name)
{
    struct tex_lib_image *ptr;
    for (ptr = head; ptr; ptr = ptr->next) {
        if (strcmp(ptr->lib_name, lib_name) == 0) {
            return ptr;
        }
    }
    return NULL;
}

static int pow2roundup(int x)
{
    int r = 1;
    while (r < x) r <<= 1;
    return r;
}

static void texlib_loader(struct texture_hook_info *thinfo)
{
    // get information string from magic
    char *s = extract_texture_hook_magic(thinfo, TEXLIB_MAGIC);
    if (!s) return;
    
    // parse information string
    int tex_origin_x;
    int tex_origin_y;
    int tex_width;
    int tex_height;
    int tex_libw;
    int tex_libh;
    char tex_name[MAX_TEX_NAME];
    char tex_lib[MAX_TEX_NAME];
    sscanf(s, "%d:%d:%d:%d:%d:%d:%[^:]:%[^:]", &tex_origin_x, &tex_origin_y, &tex_width, &tex_height, &tex_libw, &tex_libh, tex_name, tex_lib);
    
    struct tex_lib_image *img = NULL;
    img = find_image(tex_lib);
    
    if (thinfo->type == TH_PRE_IMAGELOAD) {
        thinfo->interested = 1;
        // if image not found, should load image
        // otherwise, should not load image, for faster speed
        if (!img) {
            strcpy(thinfo->loadpath, tex_lib);
        } else {
            mark_texture_hook_noautoload(thinfo);
        }
    }
    
    if (thinfo->type == TH_POST_IMAGELOAD && !img) {
        img = malloc(sizeof(struct tex_lib_image));
        img->lib_name = strdup(tex_lib);
        unsigned imgsz = thinfo->width * thinfo->height * (thinfo->bitcount / 8);
        img->bits = malloc(imgsz);
        memcpy(img->bits, thinfo->bits, imgsz);
        img->width = thinfo->width;
        img->height = thinfo->height;
        img->bitcount = thinfo->bitcount;
        img->div_alpha = thinfo->div_alpha;
        insert_image(img);
        thinfo->mem_allocator->free(thinfo->bits);
    }
    
    // load image
    if (img) {
        thinfo->width = pow2roundup(tex_width);
        thinfo->height = pow2roundup(tex_height);
        thinfo->bitcount = img->bitcount;
        thinfo->div_alpha = img->div_alpha;
        
        unsigned imgsz = thinfo->width * thinfo->height * (thinfo->bitcount / 8);
        thinfo->bits = thinfo->mem_allocator->malloc(imgsz);
        copy_bits(thinfo->bits, thinfo->width * (thinfo->bitcount / 8), 0, 0, img->bits, img->width * (img->bitcount / 8), tex_origin_x, tex_origin_y - tex_height, tex_width, tex_height, thinfo->bitcount);
        clamp_rect(thinfo->bits, thinfo->width, thinfo->height, thinfo->bitcount, thinfo->width * (thinfo->bitcount / 8), 0, 0, tex_width, tex_height);
        /*int i, j;
        char *bits = thinfo->bits;
        for (i = 0; i < thinfo->width; i++) {
            for (j = 0; j < thinfo->height; j++) {
                unsigned char *c = bits + (i + j * thinfo->width) * (thinfo->bitcount / 8);
                c[0] = rand(); c[1] = rand(); c[2] = rand();
            }
        }*/
    }
}

static MAKE_THISCALL(bool, _TextureLib_Data_GetLibInfo_wrapper, struct _TextureLib_Data *this, const char *filename)
{
    bool ret = _TextureLib_Data_GetLibInfo(this, filename);
    if (ret) {
        int i;
        for (i = 0; i < this->_realNum; i++) {
            struct _Texture_Info *cur = &this->m_pData[i];
            
            // ignore no-need-clamp images
            if (!in_clamp_list(cur->_tex_name)) continue;
            
            // replace lib name
            char buf[MAX_TEX_NAME];
            snprintf(buf, sizeof(buf), "%s" "%d:%d:%d:%d:%d:%d:%s:%s", make_texture_hook_magic(TEXLIB_MAGIC), cur->_tex_origin_x, cur->_tex_origin_y, cur->_tex_width, cur->_tex_height, cur->_tex_libw, cur->_tex_libh, cur->_tex_name, cur->_tex_lib);
            strcpy(cur->_tex_lib, buf);
            
            // adjust lib size
            cur->_tex_libw = pow2roundup(cur->_tex_width);
            cur->_tex_libh = pow2roundup(cur->_tex_height);
            
            // adjust origin
            cur->_tex_origin_x = 0;
            cur->_tex_origin_y = cur->_tex_height + cur->_tex_libh - 1024; // dirty hack, for UIButtonEX(A)::GetChannel
            
            // adjust tex coord
            float left = 0.0f;
            float top = 0.0f;
            float right = (float) cur->_tex_width / cur->_tex_libw;
            float bottom = (float) cur->_tex_height / cur->_tex_libh;
            cur->_tex_uvLT.u = cur->_tex_uvLB.u = left;
            cur->_tex_uvLT.v = cur->_tex_uvRT.v = top;
            cur->_tex_uvRT.u = cur->_tex_uvRB.u = right;
            cur->_tex_uvLB.v = cur->_tex_uvRB.v = bottom;
        }
    }
    return ret;
}

MAKE_PATCHSET(clampuilib)
{
    INIT_WRAPPER_CALL(_TextureLib_Data_GetLibInfo_wrapper, { 0x0044F19A });
    add_texture_hook(texlib_loader);
}
