#include "common.h"
scAsmDisassembler *scAsmDisassembler::Instance()
{
    static scAsmDisassembler inst;
    return &inst;
}
void scAsmDisassembler::ReadSCEFile(const char *scefile)
{
	size_t i, j;
	FILE *fp = safe_fopen(scefile, "rb");
	scename = std::string(getpathfilepart(scefile));

	// read header
	safe_fread(&header, sizeof(SceHeader), 1, fp);
	assert(memcmp(header.flag, "SCE", 4) == 0);
	assert(header.version == SCE_VERSION);
	//printf("blocknum = %u", header.blocknum);

	// read index
	index.resize(header.blocknum);
	if (header.blocknum > 0) {
		safe_fread(index.data(), sizeof(SceIndex), header.blocknum, fp);
	}
	
	// read each block
	block.resize(header.blocknum);
	for (i = 0; i < header.blocknum; i++) {
		SceIndex &curindex = index[i];
		scScriptBlock &curblock = block[i];
		//printf("  正读取 %8s %-5d %08X %s\n", scename.c_str(), curindex.id, curindex.offset, curindex.desc);
		
		safe_fseek(fp, curindex.offset, SEEK_SET);
		curblock.block_fileoffset = curindex.offset;
		
		// read id
		safe_fread(&curblock.id, sizeof(DWORD), 1, fp);
		assert(curblock.id == curindex.id);

		// read description
		WORD desclen;
		safe_fread(&desclen, sizeof(WORD), 1, fp);
		curblock.desc.resize(desclen);
		if (desclen > 0) {
			safe_fread(curblock.desc.data(), 1, desclen, fp);
		}
		if (curblock.desc.empty() || curblock.desc.back() != 0) curblock.desc.push_back(0);
		//printf("  desc = %s\n", curblock.desc.data());

		// read uservar
		safe_fread(&curblock.uservarnum, sizeof(WORD), 1, fp);
		//printf("  uservarnum = %d\n", (int)curblock.uservarnum);
		curblock.uservar.resize(curblock.uservarnum);
		for (j = 0; j < curblock.uservarnum; j++) {
			scUserVarInfo &curvar = curblock.uservar[j];
			safe_fread(&curvar.type, sizeof(BYTE), 1, fp);
			safe_fread(&curvar.initlen, sizeof(WORD), 1, fp);
			curvar.initdata.resize(curvar.initlen);
			if (curvar.initlen > 0) {
				safe_fread(curvar.initdata.data(), 1, curvar.initlen, fp);
			}
			//printf("    uservar %d %08X\n", (int) curvar.type, (unsigned)curvar.initlen);
		}

		// read code
		safe_fread(&curblock.codesize, sizeof(DWORD), 1, fp);
		curblock.codedata.resize(curblock.codesize);
		curblock.code_fileoffset = ftell(fp);
		//printf("  codesize = %08X\n", (unsigned) curblock.codesize);
		if (curblock.codesize > 0) {
			safe_fread(curblock.codedata.data(), 1, curblock.codesize, fp);
		}

		//printf("  ftell = %08lX\n", ftell(fp));
	}

	fclose(fp);
}

std::vector<char> scAsmDisassembler::scScriptBlock::get_string()
{
	std::vector<char> ret;

	unsigned strstart = codeptr;

	size_t len = get<WORD>();
	bool zero = false;
	for (size_t i = 0; i < len; i++) {
		bool errfuse = false;
		char ch = get<char>(&errfuse);
		if (errfuse) {
			scAsmTool::ReportError(LVL_WARN, "位于文件偏移 %04X+%04X=%04X 处字符串长度超过块长度", (unsigned) code_fileoffset, strstart, (unsigned) code_fileoffset + strstart);
		}

		if (!ch) zero = true;
		ret.push_back(ch);
	}
	assert(zero);
	return ret;
}


std::string scAsmDisassembler::scScriptBlock::escape_string(const std::vector<char> &str)
{
	bool bad = !str.empty() && MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, str.data(), str.size(), NULL, 0) == 0;
	bool zero = false;
	bool nonzero_after_zero = false;

	std::string ret;
	std::string part1, part2;
	
	//FILE *fp = fopen("strdump.txt", "ab");
	for (char ch: str) {
		if (!zero) {
			//if (!bad) fputc(ch, fp);
			if (ch == 0) {
				zero = true;
			} else {
				if (bad || ch == '\n' || ch == '\"' || ch == '#') {
					unsigned char uch = ch;
					char buf[4];
					sprintf(buf, "#%02X", (unsigned)uch);
					part1 += buf;
				} else {
					part1.push_back(ch);
				}
			}
		}
		if (zero) {
			
			if (ch != 0) {
				nonzero_after_zero = true;
			}

			unsigned char uch = ch;
			char buf[4];
			sprintf(buf, " %02X", (unsigned) uch);
			part2 += buf;

		}
	}
	//fclose(fp);
	
	
	assert(zero);
	if (!nonzero_after_zero) {
		ret = '\"' + part1 + '\"';
	} else {
		ret = '\"' + part1 + "\" /* STRAY:" + part2 + " */";
	}

	return ret;
}


void scAsmDisassembler::scScriptBlock::scAsmInstr::scAsmInstrParam::PrintParam(FILE *fp)
{
	switch (type) {
	case VAR_INT:
		fprintf(fp, "%d", idata);
		break;
	case VAR_FLOAT:
		fprintf(fp, "%.15f", fdata);
		break;
	case VAR_JMPFLAG:
		fprintf(fp, "@loc_%04X", (unsigned)jdata);
		break;
	case VAR_STRING:
		fprintf(fp, "%s", scScriptBlock::escape_string(sdata).c_str());
		break;
	case VAR_USERVAR:
		if ((udata & 0x8000)) {
			// global var, use absolute form
			fprintf(fp, "$!%04X", (unsigned)udata);
		} else {
			fprintf(fp, "$var_%04X", (unsigned)udata);
		}
		break;
	case VAR_LIST: {
		fprintf(fp, "(");
		for (size_t i = 0; i < ldata.size(); i++) {
			if (i) fprintf(fp, ", ");
			ldata[i].PrintParam(fp);
		}
		fprintf(fp, ")");
		break;
	}
	default:
		scAsmTool::ReportError(LVL_FATAL, "unknown param type %d", (int)type);
	}
}
void scAsmDisassembler::scScriptBlock::scAsmInstr::scAsmInstrParam::GetParam(scScriptBlock *block, BYTE t)
{
	type = t;
	switch (type) {
	case VAR_INT:
		idata = block->get<int>();
		if (scAsmTool::dbgflag) printf("    int: %d\n", idata);
		break;
	case VAR_FLOAT:
		fdata = block->get<float>();
		if (scAsmTool::dbgflag) printf("    float: %f\n", fdata);
		break;
	case VAR_JMPFLAG:
		jdata = block->get<DWORD>();
		block->codelabel.insert(jdata);
		if (scAsmTool::dbgflag) printf("    jmpflag: %08X\n", (unsigned)jdata);
		break;
	case VAR_STRING:
		sdata = block->get_string();
		if (scAsmTool::dbgflag) printf("    string: %s\n", scScriptBlock::escape_string(sdata).c_str());
		break;
	case VAR_USERVAR:
		udata = block->get<WORD>();
		if (scAsmTool::dbgflag) printf("    uservar: %04X\n", (unsigned)udata);
		break;
	case VAR_LIST: {
		if (scAsmTool::dbgflag) printf("    varlist: {\n");
		WORD varnum = block->get<WORD>();
		BYTE vartype;
		ldata.clear();
		ldata.resize(varnum);
		for (int i = varnum - 1; i >= 0; i--) {
			vartype = block->get<BYTE>();
			ldata[i].GetParam(block, vartype);
		}
		if (scAsmTool::dbgflag) printf("    }\n");
		break;
	}
	default:
		scAsmTool::ReportError(LVL_FATAL, "unknown param type %d", (int)type);
	}
}

void scAsmDisassembler::scScriptBlock::DisassembleCodeData()
{
	//printf("codedata %d %s\n", id, desc.data());
	codeasm.clear();
	codelabel.clear();
	codeptr = 0;
	size_t loc;
	while (codeptr < codesize) {
		scAsmInstr instr;

		loc = codeptr;
		bool errfuse = false;
		instr.cmdid = get<WORD>(&errfuse);
		instr.paramflag = get<WORD>(&errfuse);
		
		if (errfuse || !scCmdDef::IsLegalInstr(instr.cmdid, instr.paramflag)) {
			if (errfuse) {
				scAsmTool::ReportError(LVL_WARN, "在文件偏移 %04X+%04X=%04X 处的剩余数据不足以构成指令 (%04X < %04X)", (unsigned)code_fileoffset, (unsigned)loc, (unsigned)(code_fileoffset + loc), (unsigned)codeptr, (unsigned)codesize);
			} else {
				scAsmTool::ReportError(LVL_WARN, "在文件偏移 %04X+%04X=%04X 处有无效指令 (%04X, %04X)", (unsigned)code_fileoffset, (unsigned)loc, (unsigned)(code_fileoffset + loc), (unsigned)instr.cmdid, (unsigned)instr.paramflag);
			}
			size_t newptr = loc;

			if (errfuse) {
				newptr = codesize;
			} else {
				unsigned x;
				if (scAsmTool::dbgflag) {
					printf("  请输入下一有效指令相对当前的偏移（十六进制）: ");
					scanf("%x", &x);
				} else {
					x = 1;
				}
				newptr += x;
				if (newptr > codesize) newptr = codesize;
				printf("  于块内偏移 %04X 处重新开始反汇编...\n", (unsigned)newptr);
			}
			
			instr.iscomment = true;
			
			char buf[MAXLINE];
			instr.comment = "#db";
			codeptr = loc;
			for (size_t i = loc; i < newptr; i++) {
				sprintf(buf, " &H%02X", (unsigned) get<unsigned char>());
				instr.comment += buf;
			}

			//while (instr.comment.length() < 40) instr.comment += ' ';
			sprintf(buf, " // SKIP: 无效指令 (%04X+%04X=>%04X)", (unsigned)loc, (unsigned)newptr - (unsigned)loc, (unsigned)newptr);
			instr.comment += std::string(buf);

			codeptr = newptr;
			goto nextloop;
		}

		scCmdDef &curcmd = scCmdDef::GetCmdDef(instr.cmdid);
		assert(curcmd.pfun);

		if (scAsmTool::dbgflag) printf("  instr %s(%d) %04X\n", curcmd.pfun, (int) instr.cmdid, (unsigned) instr.paramflag);
		
		instr.paramlist.resize(curcmd.paramnum);
		for (int i = curcmd.paramnum - 1; i >= 0; i--) {
			scAsmInstr::scAsmInstrParam &param = instr.paramlist[i];
			BYTE type = curcmd.paramlist[i];
			if ((instr.paramflag & WORD(1<<i))) {
				type = VAR_USERVAR;
			}
			param.GetParam(this, type);
		}
nextloop:
		codeasm.insert(std::make_pair(std::make_pair(loc, codeptr), instr));
	}

	assert(codeptr == codesize);
}

void scAsmDisassembler::DisassembleAllBlocks()
{
	size_t i;
	for (i = 0; i < header.blocknum; i++) {
		SceIndex &curindex = index[i];
		printf("  正解析 %8s %5d %08X %s\n", scename.c_str(), curindex.id, curindex.offset, curindex.desc);
		block[i].DisassembleCodeData();
	}
}

void scAsmDisassembler::WriteASMFile(const char *asmfile)
{
	size_t i;
	FILE *fp = safe_fopen(asmfile, "w");
	

    SYSTEMTIME SystemTime;
    GetLocalTime(&SystemTime);
	fprintf(fp, "// %s ", scename.c_str());
    fprintf(fp, "disassembled at %04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu ", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
	fprintf(fp, "by scAsmTool (%s)\n", built_on);
    fprintf(fp, "// cmdline: %s\n", scAsmTool::cmdline.c_str());
	fprintf(fp, "\n");

	for (i = 0; i < header.blocknum; i++) {
		SceIndex &curindex = index[i];
		//printf("  正输出 %8s %-5d %08X %s\n", scename.c_str(), curindex.id, curindex.offset, curindex.desc);
		scScriptBlock &curblock = block[i];
		fprintf(fp, "#begin %d, %s\n", curblock.id, scScriptBlock::escape_string(curblock.desc).c_str());

		for (unsigned varid = 0; varid < curblock.uservar.size(); varid++) {
			auto &curvar = curblock.uservar[varid];
			assert(curvar.type == VAR_INT);
			fprintf(fp, "#var $var_%04X, INT", varid);
			if (curvar.initlen == 0) {
				fprintf(fp, "\n");
			} else {
				fprintf(fp, ", %d\n", curvar.get<int>());
			}
		}

		for (auto it = curblock.codeasm.begin(); it != curblock.codeasm.end(); it++) {
			scScriptBlock::scAsmInstr &instr = it->second;

			// print label if needed
			for (auto lblit = curblock.codelabel.lower_bound(it->first.first); lblit != curblock.codelabel.end() && *lblit < it->first.second; lblit++) {
				if (*lblit == it->first.first) {
					fprintf(fp, "@loc_%04X:\n", it->first.first);
				} else {
					scAsmTool::ReportError(LVL_WARN, "标号 @loc_%04X 位于指令内部，详细信息请见反汇编结果", *lblit);
					fprintf(fp, "// ERROR: 标号 @loc_%04X 位于下方指令内部 （标号在文件中偏移为 %04X+%04X=%04X；下方指令在文件中偏移为 %04X+%04X=%04X 长度为 %04X）\n",
						*lblit,
						(unsigned)curblock.code_fileoffset, *lblit, (unsigned)curblock.code_fileoffset + *lblit,
						(unsigned)curblock.code_fileoffset, it->first.first, (unsigned)curblock.code_fileoffset + it->first.first,
						it->first.second - it->first.first
					);
				}
			}

			// print instruction
			if (!instr.iscomment) {
				scCmdDef &curcmd = scCmdDef::GetCmdDef(instr.cmdid);
				assert(curcmd.pfun);
				fprintf(fp, "  %-20s ", curcmd.GetCmdName());
				for (auto pit = instr.paramlist.begin(); pit != instr.paramlist.end(); pit++) {
					if (pit != instr.paramlist.begin()) fprintf(fp, ", ");
					pit->PrintParam(fp);
				}
				fprintf(fp, "\n");
			} else {
				fprintf(fp, "%s\n", instr.comment.c_str());
			}
		}
		if (curblock.codelabel.find(curblock.codesize) != curblock.codelabel.end()) {
			fprintf(fp, "@loc_%04X:\n", curblock.codesize);
		}
		//fprintf(fp, "  %08X: -- end --\n", curblock.codesize);
		fprintf(fp, "#end\n\n");
	}

	fclose(fp);
}

void scAsmDisassembler::DisassembleSCE(const char *scefile, const char *asmfile)
{
	printf(" 正在对 %s 进行反汇编并输出到 %s ...\n", scefile, asmfile);
	printf("\n");
	printf(" 读取 SCE 文件 ...\n");
	ReadSCEFile(scefile);
	printf(" 解析代码 ...\n");
	DisassembleAllBlocks();
	printf("\n");
	printf(" 生成 ASM 文件 ...\n");
	WriteASMFile(asmfile);
	printf("\n");
}
