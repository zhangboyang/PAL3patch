#include "common.h"

#define MAX_TEX_NAME 0x100
#define TEXLIB_MAGIC "TEXLIB"

static char *clamp_list[] = {
    "ui\\gamemainui\\communal\\stpctl\\battle_array0.tga",
    "ui\\gamemainui\\communal\\stpctl\\battle_array1.tga",
    "ui\\gamemainui\\communal\\stpctl\\battle_array2.tga",
    "ui\\gamemainui\\communal\\stpctl\\equip0.tga",
    "ui\\gamemainui\\communal\\stpctl\\equip1.tga",
    "ui\\gamemainui\\communal\\stpctl\\equip2.tga",
    "ui\\gamemainui\\communal\\stpctl\\exit0.tga",
    "ui\\gamemainui\\communal\\stpctl\\exit1.tga",
    "ui\\gamemainui\\communal\\stpctl\\exit2.tga",
    "ui\\gamemainui\\communal\\stpctl\\magic0.tga",
    "ui\\gamemainui\\communal\\stpctl\\magic1.tga",
    "ui\\gamemainui\\communal\\stpctl\\magic2.tga",
    "ui\\gamemainui\\communal\\stpctl\\prop0.tga",
    "ui\\gamemainui\\communal\\stpctl\\prop1.tga",
    "ui\\gamemainui\\communal\\stpctl\\prop2.tga",
    "ui\\gamemainui\\communal\\stpctl\\rwts0.tga",
    "ui\\gamemainui\\communal\\stpctl\\rwts1.tga",
    "ui\\gamemainui\\communal\\stpctl\\rwts2.tga",
    "ui\\gamemainui\\communal\\stpctl\\state0.tga",
    "ui\\gamemainui\\communal\\stpctl\\state1.tga",
    "ui\\gamemainui\\communal\\stpctl\\state2.tga",
    "ui\\gamemainui\\communal\\stpctl\\system0.tga",
    "ui\\gamemainui\\communal\\stpctl\\system1.tga",
    "ui\\gamemainui\\communal\\stpctl\\system2.tga",
    "ui\\gamemainui\\communal\\stpctl\\unbattle_array.tga",
    "ui\\gamemainui\\communal\\stpctl\\unequip.tga",
    "ui\\gamemainui\\communal\\stpctl\\unexit.tga",
    "ui\\gamemainui\\communal\\stpctl\\unmagic.tga",
    "ui\\gamemainui\\communal\\stpctl\\unprop.tga",
    "ui\\gamemainui\\communal\\stpctl\\unrwts.tga",
    "ui\\gamemainui\\communal\\stpctl\\unstate.tga",
    "ui\\gamemainui\\communal\\stpctl\\unsystem.tga",
    
    "ui\\gamemainui\\state\\portrait0.tga",
    "ui\\gamemainui\\state\\portrait1.tga",
    "ui\\gamemainui\\state\\portrait2.tga",
    "ui\\gamemainui\\state\\portrait3.tga",
    "ui\\gamemainui\\state\\portrait4.tga",
    
    "ui\\gamemainui\\rwts\\tp1.tga",
    "ui\\gamemainui\\rwts\\tp2.tga",
    
    "ui\\gamemainui\\system\\musiceffect.tga",
    "ui\\gamemainui\\system\\musicbar.tga",
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
        
        int left, top, right, bottom;
        get_solid_area(thinfo->bits, tex_width, tex_height, thinfo->bitcount, thinfo->width * (thinfo->bitcount / 8), &left, &top, &right, &bottom);
        clamp_rect(thinfo->bits, thinfo->width, thinfo->height, thinfo->bitcount, thinfo->width * (thinfo->bitcount / 8), left, top, right, bottom);
        make_border_transparent(thinfo->bits, thinfo->width, thinfo->height, thinfo->bitcount, thinfo->width * (thinfo->bitcount / 8), left, top, right, bottom);
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

static MAKE_THISCALL(void, ArtistPlugIn_GetPlugFileInfo_wrapper, struct ArtistPlugIn *this, const char *filename)
{
    ArtistPlugIn_GetPlugFileInfo(this, filename);

    unsigned i;
    for (i = 0; i < this->m_infoNum; i++) {
        struct _PlugInfo *cur = &this->pInfo[i];
        
        // ignore no-need-clamp images
        if (!in_clamp_list(cur->name)) continue;
        
        // replace lib name
        char buf[MAX_TEX_NAME];
        snprintf(buf, sizeof(buf), "%s" "%d:%d:%d:%d:%d:%d:%s:%s", make_texture_hook_magic(TEXLIB_MAGIC), cur->left, cur->top, cur->width, cur->height, cur->libWidth, cur->libHeight, cur->name, cur->libName);
        strcpy(cur->libName, buf);
        
        // adjust lib size
        cur->libWidth = pow2roundup(cur->width);
        cur->libHeight = pow2roundup(cur->height);
        
        // adjust origin
        cur->left = 0;
        cur->top = cur->height + cur->libHeight - 1024; // dirty hack, for UIButtonEX(A)::GetChannel
        
        // adjust tex coord
        float left = 0.0f;
        float top = 0.0f;
        float right = (float) cur->width / cur->libWidth;
        float bottom = (float) cur->height / cur->libHeight;
        cur->_tex_uvLT.u = cur->_tex_uvLB.u = left;
        cur->_tex_uvLT.v = cur->_tex_uvRT.v = top;
        cur->_tex_uvRT.u = cur->_tex_uvRB.u = right;
        cur->_tex_uvLB.v = cur->_tex_uvRB.v = bottom;
    }
}

MAKE_PATCHSET(clampuilib)
{
    INIT_WRAPPER_CALL(ArtistPlugIn_GetPlugFileInfo_wrapper, { 0x004557A8 });
    add_texture_hook(texlib_loader);
}
