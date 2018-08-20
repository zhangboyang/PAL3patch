#define PLUGIN_INTERNAL_NAME "DISPTEXT"
#define PLUGIN_FRIENDLY_NAME "显示文字"
#define PLUGIN_VERSION "v1.1"
#ifdef BUILD_FOR_PAL3
#define USE_PAL3_DEFINITIONS
#include "PAL3patch.h"
#endif
#ifdef BUILD_FOR_PAL3A
#define USE_PAL3A_DEFINITIONS
#include "PAL3Apatch.h"
#endif

// 控制是否显示文字的变量 
static int display_text_flag = 1; // 默认为开启

// 帧计数器
static int frame_cnt = 0;


// 补丁用窗口消息钩子函数 
static void display_text_wndproc_hook(void *arg)
{
    struct wndproc_hook_data *data = arg;

    // 若F1键被按下 
    if (data->Msg == WM_KEYUP && data->wParam == VK_F1) {

        // 切换控制变量的值
        display_text_flag ^= 1;

        // 设置返回值并中止职责链
        data->retvalue = 0;
        data->processed = 1;

    }
}

// 显示文本的函数 
static void display_text()
{
    // 构造临时字符串 
    struct wstr msg; wstr_ctor(&msg);

    // 增加帧计数器
    frame_cnt++;

    // 如果要显示文字 
    if (display_text_flag) {

        // 开始打印文字
        print_wstring_begin();

        // 打印原始大小的文字
        print_wstring(FONTID_U12, L"U12\nHello World!\n你好，世界！", 100, 100, 0xFFFF0000);
        print_wstring(FONTID_U16, L"U16\nHello World!\n你好，世界！", 300, 100, 0xFFFF0000);
        print_wstring(FONTID_U20, L"U20\nHello World!\n你好，世界！", 500, 100, 0xFFFF0000);

        // 打印随屏幕大小而自动放大的文字
        print_wstring(FONTID_U12_SCALED, L"U12_SCALED\nHello World!\n你好，世界！", 100, 200, 0xFFFF0000);
        print_wstring(FONTID_U16_SCALED, L"U16_SCALED\nHello World!\n你好，世界！", 300, 200, 0xFFFF0000);
        print_wstring(FONTID_U20_SCALED, L"U20_SCALED\nHello World!\n你好，世界！", 500, 200, 0xFFFF0000);

        // 打印说明文字
        wstr_format(&msg, L"当前帧为 %d，按 F1 可切换文字显示", frame_cnt);
        print_wstring(FONTID_U20, wstr_getwcs(&msg), 100, 10, 0xFFFF0000);

        // 结束打印文字
        print_wstring_end();

    }

    // 析构临时字符串 
    wstr_dtor(&msg);
}


MAKE_PLUGINABOUT()
MAKE_PLUGINENTRY() // 插件入口函数，会在插件加载时被调用
{
    // 检查数据结构大小 
#ifdef BUILD_FOR_PAL3
    PAL3_STRUCT_SELFCHECK();
#endif
#ifdef BUILD_FOR_PAL3A
    PAL3A_STRUCT_SELFCHECK();
#endif

    // 在场景绘制结束前调用我们的显示文字函数
    add_preendscene_hook(display_text);

    // 添加窗口消息钩子函数
    add_postwndproc_hook(display_text_wndproc_hook);

    return 0;
}
