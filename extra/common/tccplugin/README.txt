编译说明：

  (1) 确保已经安装 tcc，也已编译 PAL3patch
  (2) 修改 tccplugin\Makefile 将其中的
         PAL3PATCH_INCLUDE_PATH
         PAL3PATCH_LIBRARY_PATH
         PAL3PATCH_INCLUDE_PATH
         PAL3PATCH_LIBRARY_PATH
         LIBTCC_INCLUDE_PATH
         LIBTCC_LIBRARY_PATH
      指向正确的目录
  (3) 在 MSYS 中进入 tccplugin 目录，并运行 make
      此后应有 tccplugin\PAL3\tccplugin.dll 和 tccplugin\PAL3A\tccplugin.dll 生成


安装说明：

  以仙三为例，假设游戏安装在 C:\PAL3 目录下
  则应有以下目录/文件：

    C:\PAL3\plugins\tcc.plugin                // TCC 插件描述文件；来自 files\tcc.plugin
    C:\PAL3\plugins\tcc\tccplugin.dll         // TCC 插件 DLL；来自刚编译好的 tccplugin\PAL3\tccplugin.dll
    C:\PAL3\plugins\tcc\init.c                // TCC 初始化源码；来自 files\init.c
    C:\PAL3\plugins\tcc\compile.bat           // 编译工具；来自 files\PAL3\compile.bat
    C:\PAL3\plugins\tcc\to_utf8.c             // UTF-8 转码工具；来自 files\to_utf8.c
    C:\PAL3\plugins\tcc\*                     // TCC 安装目录下的其他文件
    C:\PAL3\plugins\tcc\include\libtcc.def    // libtcc 头文件；来自 TCC 安装目录下 libtcc\libtcc.h
    C:\PAL3\plugins\tcc\lib\libtcc.def        // libtcc 库文件；来自 TCC 安装目录下 libtcc\libtcc.def
    C:\PAL3\plugins\tcc\include\PAL3patch.h   // PAL3patch 头文件
    C:\PAL3\plugins\tcc\include\PAL3patch\*   // PAL3patch 头文件
    C:\PAL3\plugins\tcc\lib\PAL3patch.def     // PAL3patch 库文件
    C:\PAL3\plugins\tcc\include\tccplugin.h   // tccplugin 头文件；来自 tccplugin\tccplugin.h
    C:\PAL3\plugins\tcc\lib\tccplugin.def     // tccplugin 库文件；来自生成的 tccplugin\PAL3\tccplugin.def

  另外还需按照 tcc 的说明，将适当的 winapi 头文件安装到 include/winapi 目录下。


使用说明：

  将 C 源代码形式的插件放入 plugins 下即可，TCC 插件会自动寻找并加载。
  若想将 C 源代码形式的插件编译为 DLL 形式，将 C 源代码文件拖放到 compile.bat 上即可。


注意：

  TCC 须采用修复 fastcall、支持 UTF-8、修正常数 call 的版本，否则会出现问题。v0.9.27 符合这些要求。
