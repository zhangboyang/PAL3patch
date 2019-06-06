SCE 编译/反编译工具 by ZBY

由于使用了 C++ 11 的特性，因此需要使用 VC++ 2017 或更高版本进行编译。

使用示例（以仙三为例，编译三外时请用 /3A 选项）：
  将 SCE 反编译为 ASM：
    scAsmTool /3 /D foobar.sce foobar.asm
  自我测试（需先反编译）：
    scAsmTool /3 /T foobar.sce foobar.asm
  将 ASM 编译为 SCE：
    scAsmTool /3 /A foobar.asm foobar_new.sce

反编译出的 ASM 文件格式与官方格式有少量差异。更多指令用法请见程序内说明。

四个用于测试的批处理文件：
  使用前请先用 uncpk 工具将 CPK 文件解包，将编译好的 scAsmTool.exe 与批处理文件一起放入游戏目录下，然后运行批处理即可。
    scAsmTool_Dtest_PAL3.bat   对仙三所有 SCE 文件使用 /D 选项
    scAsmTool_Ttest_PAL3.bat   对仙三所有 SCE 文件使用 /T 选项
    scAsmTool_Dtest_PAL3A.bat  对三外所有 SCE 文件使用 /D 选项
    scAsmTool_Ttest_PAL3A.bat  对三外所有 SCE 文件使用 /T 选项

