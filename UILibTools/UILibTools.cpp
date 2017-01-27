#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <string>
#include <windows.h>
using namespace std;

#define MAXLIBW 1024
#define MAXLIBH 1024
#define MAXLINE 4096
class uilib_item {
public:
    char name[MAXLINE];
    char lib[MAXLINE];
    int libw;
    int libh;
    int orix;
    int oriy;
    int w;
    int h;
    int m;
    
    void print()
    {
        printf("this=%p\n", this);
        printf("name=%s\n", name);
        printf("lib=%s\n", lib);
        printf("libw=%d\n", libw);
        printf("libh=%d\n", libh);
        printf("orix=%d\n", orix);
        printf("oriy=%d\n", oriy);
        printf("w=%d\n", w);
        printf("h=%d\n", h);
        printf("m=%d\n", m);
    }
};

class tex_pixels {
public:
    int a[MAXLIBW][MAXLIBH];
    int w;
    int h;
    tex_pixels() { memset(a, -1, sizeof(a)); w = h = 0; }
    void init(int new_w, int new_h)
    {
        assert(w == 0 || w == new_w);
        assert(h == 0 || h == new_h);
        w = new_w; h = new_h;
    }
    void assign(int x, int y, int val)
    {
        assert(0 <= x && x < w);
        assert(0 <= y && y < h);
        a[x][y] = val;
    }
    void assign_rect(int left, int top, int right, int bottom, int val)
    {
        assert(right >= left);
        assert(bottom >= top);
        for (int i = left; i < right; i++) {
            for (int j = top; j < bottom; j++) {
                assign(i, j, val);
            }
        }
    }
    void write_bmp(const char *fn)
    {
        if (w == 0 || h == 0) return;
        
        BITMAPINFOHEADER bmiHeader;
        memset(&bmiHeader, 0, sizeof(bmiHeader));
        bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmiHeader.biWidth = w;
        bmiHeader.biHeight = h;
        bmiHeader.biPlanes = 1;
        bmiHeader.biBitCount = 32;
        bmiHeader.biCompression = BI_RGB;
        DWORD dwBmpSize = (w * (bmiHeader.biBitCount / 8) + 3) / 4 * 4 * h;
        
        BITMAPFILEHEADER bmfHeader;
        memset(&bmfHeader, 0, sizeof(bmfHeader));
        bmfHeader.bfType = 0x4D42;
        bmfHeader.bfSize = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        
        FILE *bmpfp = fopen(fn, "wb");
        assert(bmpfp);
        fwrite(&bmfHeader, sizeof(bmfHeader), 1, bmpfp);
        fwrite(&bmiHeader, sizeof(bmiHeader), 1, bmpfp);
        //for (int i = h - 1; i >= 0; i--) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                int v = a[j][i] == -1 ? -1 : 0;
                fwrite(&v, sizeof(v), 1, bmpfp);
            }
            // no padding
        }
        fclose(bmpfp);
    }
};
vector<uilib_item> all_items;
map<string, tex_pixels> tex_map;

FILE *fp;
char token_buf[MAXLINE];
char *read_token()
{
    return fscanf(fp, "%s", token_buf) == 1 ? token_buf : NULL;
}
int read_int()
{
    int ret;
    int r = sscanf(read_token(), "%d", &ret);
    assert(r == 1);
    return ret;
}
int read_item()
{
    uilib_item item;
    if (strcmp(read_token(), "t_name") != 0) return 0;
    strcpy(item.name, read_token());
    if (strcmp(read_token(), "t_lib") != 0) assert(0);
    strcpy(item.lib, read_token());
    strtok(item.lib, ".");
    if (strcmp(read_token(), "t_libw") != 0) assert(0);
    item.libw = read_int();
    assert(item.libw <= MAXLIBW);
    if (strcmp(read_token(), "t_libh") != 0) assert(0);
    item.libh = read_int();
    assert(item.libh <= MAXLIBH);
    if (strcmp(read_token(), "t_orix") != 0) assert(0);
    item.orix = read_int();
    if (strcmp(read_token(), "t_oriy") != 0) assert(0);
    item.oriy = read_int();
    if (strcmp(read_token(), "t_w") != 0) assert(0);
    item.w = read_int();
    if (strcmp(read_token(), "t_h") != 0) assert(0);
    item.h = read_int();
    if (strcmp(read_token(), "t_m") != 0) assert(0);
    item.m = read_int();
    if (strcmp(read_token(), "#End") != 0) assert(0);
    all_items.push_back(item);
    return 1;
}
void query_pixel()
{
    char name[MAXLINE];
    int x, y;
    printf("query pixel: LIBNAME LR TB\n");
    scanf("%s%d%d", name, &x, &y);
    tex_pixels &curpixels = tex_map[string(strtok(name, "."))];
    printf("libw=%d libh=%d\n", curpixels.w, curpixels.h);
    y = curpixels.h - y - 1;
    int id = curpixels.a[x][y];
    printf("id=%d\n", id);
    if (id >= 0) {
        all_items[id].print();
    }
}
void make_usemap()
{
    map<string, tex_pixels>::iterator it;
    for (it = tex_map.begin(); it != tex_map.end(); it++) {
        char fn[MAXLINE];
        sprintf(fn, "%s.map.bmp", it->first.c_str());
        it->second.write_bmp(fn);
    }
}
int main()
{
    fp = fopen("UI_opt.tli", "r");
    printf("signature: %s\n", read_token());
    while (read_item());
    assert(strcmp(token_buf, "#endfile") == 0);
    printf("total %d items.\n", (int) all_items.size());
    
    for (int i = 0; i < (int) all_items.size(); i++) {
        uilib_item &curitem = all_items[i];
        tex_pixels &curpixels = tex_map[string(strtok(curitem.lib, "."))];
        curpixels.init(curitem.libw, curitem.libh);
        curpixels.assign_rect(curitem.orix, curitem.oriy, curitem.orix + curitem.w, curitem.oriy + curitem.h, i);
    }
    
    
    make_usemap();
    
    while (1) query_pixel();
    
    system("pause");
    return 0;
}
