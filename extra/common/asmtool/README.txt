SCE 编译/反编译工具 by ZBY

由于使用了 C++ 11 的特性，因此需要使用 VC++ 2017 进行编译。

使用示例（以仙三为例，编译三外时请用 /3A 选项）：
  将 SCE 反编译为 ASM：
    scAsmTool /3 /D foobar.sce foobar.asm
  自我测试（需先反编译）：
    scAsmTool /3 /T foobar.sce foobar.asm
  将 ASM 编译为 SCE：
    scAsmTool /3 /A foobar.asm foobar_new.sce

反编译出的 ASM 文件格式与官方格式有少量差异。更多指令用法请见程序内说明。
