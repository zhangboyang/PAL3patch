#include "common.h"

int main(int argc, char *argv[])
{
	printf("    scAsmTool 仙三/三外 SCE 反汇编/汇编工具 by ZBY (%s)\n\n", built_on);

	scAsmTool::cmdline.clear();
	for (int i = 0; i < argc; i++) {
		if (i > 0) scAsmTool::cmdline += " ";
		scAsmTool::cmdline += argv[i];
	}

	if (argc < 5) goto usage;

	if (argc >= 6 && stricmp(argv[5], "/DBG") == 0) {
		scAsmTool::dbgflag = true;
	}

	if (stricmp(argv[1], "/3") == 0) {
		InitCmdDef_PAL3();
	} else if (stricmp(argv[1], "/3A") == 0) {
		InitCmdDef_PAL3A();
	} else {
		goto usage;
	}

	if (stricmp(argv[2], "/D") == 0) {
		scAsmDisassembler::Instance()->DisassembleSCE(argv[3], argv[4]);
	} else if (stricmp(argv[2], "/A") == 0) {
		scAsmAssembler::Instance()->AssembleSCE(argv[3], argv[4]);
	} else if (stricmp(argv[2], "/T") == 0) {
		scAsmAssembler::Instance()->AssembleSCETest(argv[3], argv[4]);
	} else {
		goto usage;
	}

	scAsmTool::die(0);
	
usage:
	printf(" 命令行格式：\n");
	printf("    scAsmTool [/3|/3A] [/D|/A] [文件X] [文件Y] [/DBG]\n");
	printf("\n");
	printf(" 版本选项：\n");
	printf("    /3     使用仙三指令定义\n");
	printf("    /3A    使用三外指令定义\n");
	printf("\n");
	printf(" 功能选项：\n");
	printf("    /D     反汇编模式（将 X 反汇编为 Y）\n");
	printf("    /A     汇编模式（将 X 汇编为 Y）\n");
	printf("    /T     测试模式（测试 Y 汇编结果是否 X 相同，不会覆盖文件）\n");
	printf("\n");
	printf(" 调试选项：\n");
	printf("    /DBG   输出调试信息\n");
	printf("\n");
	
	printf(" 使用示例：\n");
	printf("    scAsmTool /3 /D Q01.SCE Q01.ASM      反编译 Q01.SCE 输出为 Q01.ASM\n");
	printf("    scAsmTool /3 /T Q01.SCE Q01.ASM      测试反编译结果是否能精确编译为原文件\n");
	printf("    scAsmTool /3 /A Q01.ASM Q01NEW.SCE   编译 Q01.ASM 输出为 Q01NEW.SCE\n");
	

	printf("\n");
	
	printf(" 部分语法说明：\n");
	printf("  【预处理指令】\n");
	printf("    注意：只能在脚本块（即 #begin 和 #end 围成的区域）以外使用！\n");
	printf("    #include \"foobar.asm\"   包含另一文件\n");
	printf("    #define X Y             定义宏 X 替换为 Y\n");
	printf("  【注释】\n");
	printf("    // ...        单行注释，“//”后内容均被忽略\n");
	printf("    /* ... */     块注释，忽略“/*”与“*/”之间内容（但只能在单行内使用）\n");
	printf("  【字符串】\n");
	printf("    \"...\"         引号内为字符串内容（支持“#XX”十六进制转义，例如使用“#22”代替“\"”）\n");
	


	printf("\n");
	scAsmTool::die(1);
}