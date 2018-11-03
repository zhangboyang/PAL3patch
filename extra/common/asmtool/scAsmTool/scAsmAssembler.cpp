#include "common.h"

scAsmAssembler *scAsmAssembler::Instance()
{
    static scAsmAssembler inst;
    return &inst;
}

bool scAsmAssembler::AsmLexer::IsSpecial(int p)
{
	assert(p <= (int)text[ln].length());
	return !!strchr("#@$:()\"", text[ln][p]);
}
bool scAsmAssembler::AsmLexer::IsSpace(int p)
{
	assert(p <= (int)text[ln].length());
	return text[ln][p] == ',' || isspace(text[ln][p]);
}
bool scAsmAssembler::AsmLexer::IsLineComment(int p)
{
	assert(p <= (int)text[ln].length());
	return text[ln][p] == '/' && text[ln][p + 1] == '/';
}
bool scAsmAssembler::AsmLexer::IsBlockCommentBegin(int p)
{
	assert(p <= (int)text[ln].length());
	return text[ln][p] == '/' && text[ln][p + 1] == '*';
}
bool scAsmAssembler::AsmLexer::IsBlockCommentEnd(int p)
{
	assert(p <= (int)text[ln].length());
	return text[ln][p] == '*' && text[ln][p + 1] == '/';
}

void scAsmAssembler::AsmLexer::SkipSpace()
{
	if (col < (int)text[ln].length()) {
		while (IsSpace(col)) col++;
		if (IsLineComment(col)) col = text[ln].length();
		if (IsBlockCommentBegin(col)) {
			AsmToken t = MakeEmptyTokenAtCurrentPosition();
			while (1) {
				if (IsBlockCommentEnd(col)) {
					col += 2;
					break;
				}
				col++;
				if (col >= text[ln].length()) {
					t.ecol = text[ln].length() - 1;
					scAsmAssembler::Instance()->ReportError(LVL_ERROR, t, "块注释没有结束（仅支持单行内块注释）");
					break;
				}
			}
			SkipSpace();
		}
	}
}
int scAsmAssembler::AsmToken::GetParamType()
{
	switch (type) {
	case TOK_FLOAT: return VAR_FLOAT;
	case TOK_INT: return VAR_INT;
	case TOK_VAR: return VAR_USERVAR;
	case TOK_LOC: return VAR_JMPFLAG;
	case TOK_STR: return VAR_STRING;
	default: return VAR_NONE;
	}
}
std::shared_ptr<scAsmAssembler::CodeBuffer::BufferItem> scAsmAssembler::AsmToken::GetParamValue(AsmBlock &block)
{
	switch (type) {
	case TOK_FLOAT: return scAsmAssembler::Instance()->code.NewItem()->SetValue<float>(fvalue);
	case TOK_INT: return scAsmAssembler::Instance()->code.NewItem()->SetValue<int>(ivalue);
	case TOK_VAR: return scAsmAssembler::Instance()->code.NewItem()->SetValue<WORD>(block.GetUserVarID(*this));
	case TOK_LOC: return scAsmAssembler::Instance()->code.NewItem()->SetValue<DWORD>(0, CodeBuffer::RELOC_DIFFERENCE, block.codebegin, block.FindLabel(*this));
	case TOK_STR: return scAsmAssembler::Instance()->code.NewItem()->SetValue<WORD>((WORD)(svalue.length() + 1))->AppendData(svalue.c_str(), svalue.length() + 1);;
	default: assert(0); return scAsmAssembler::Instance()->code.NewItem();
	}
}
void scAsmAssembler::AsmToken::Dump()
{
	printf("token t=%-2d i=%-5d f=%-8.3f s=%s\n", (int)type, ivalue, fvalue, svalue.c_str());
}
void scAsmAssembler::AsmToken::Expect(TOKENTYPE expect_type)
{
	if (expect_type != ANY_TOK) {
		if (type != expect_type) {
			scAsmAssembler::Instance()->ReportError(LVL_ERROR, *this, "此位置不应出现该单词");
		}
	}
}
void scAsmAssembler::AsmToken::Expect(const std::string &expect_svalue)
{
	if (svalue.compare(expect_svalue) != 0) {
		scAsmAssembler::Instance()->ReportError(LVL_ERROR, *this, "此位置不应出现该单词");
	}
}
void scAsmAssembler::AsmToken::Expect(TOKENTYPE expect_type, const std::string &expect_svalue)
{
	Expect(expect_type);
	Expect(expect_svalue);
}
void scAsmAssembler::AsmToken::MacroSubstitute(AsmToken &vtok)
{
	assert(type == TOK_IDENTIFIER);
	assert(vtok.type != TOK_IDENTIFIER || svalue.compare(vtok.svalue) != 0);
	type = vtok.type;
	svalue = vtok.svalue;
	fvalue = vtok.fvalue;
	ivalue = vtok.ivalue;
}
void scAsmAssembler::AsmToken::PostProcess()
{
	switch (type) {
	case TOK_IDENTIFIER: {
		auto it = scAsmAssembler::Instance()->macro.find(svalue);
		if (it != scAsmAssembler::Instance()->macro.end()) {
			MacroSubstitute(it->second.second);
			PostProcess();
		}
		break;
	}
	case RAWTOK_NUM: {
		int r;
		if (svalue[0] == '&') {
			type = TOK_INT;
			if ((svalue[1] != 'H' && svalue[1] != 'h') || sscanf(svalue.c_str() + 2, "%x", &ivalue) != 1) {
				scAsmAssembler::Instance()->ReportError(LVL_ERROR, *this, "无法解析为十六进制整数");
			}
		} else if (svalue.find('.') != svalue.npos) {
			type = TOK_FLOAT;
			r = sscanf(svalue.c_str(), "%f", &fvalue);
			if (r != 1) scAsmAssembler::Instance()->ReportError(LVL_ERROR, *this, "无法解析为浮点数");
		} else {
			type = TOK_INT;
			r = sscanf(svalue.c_str(), "%d", &ivalue);
			if (r != 1) scAsmAssembler::Instance()->ReportError(LVL_ERROR, *this, "无法解析为十进制整数");
		}
		break;
	}
	case RAWTOK_STR: {
		assert(svalue.length() >= 2 && svalue.front() == '\"' && svalue.back() == '\"');
		type = TOK_STR;
		svalue = svalue.substr(1, svalue.length() - 2);
		std::replace(svalue.begin(), svalue.end(), quote_escape, '\"');
		break;
	}
	}
}
scAsmAssembler::AsmToken scAsmAssembler::AsmLexer::MakeEmptyTokenAtCurrentPosition()
{
	AsmToken ret;
	ret.txtfn = txtfn;
	ret.txtline = text[ln];
	ret.ln = ln;
	ret.col = col;
	ret.ecol = col;
	return ret;
}
scAsmAssembler::AsmToken scAsmAssembler::AsmLexer::NextToken(TOKENTYPE expect)
{
	if (inclexer) {
		return inclexer->NextToken(expect);
	}

	std::string &line = text[ln];
	SkipSpace();
	assert(col <= (int)line.length());

	AsmToken ret = MakeEmptyTokenAtCurrentPosition();

	TOKENTYPE &type = ret.type;
	int &ecol = ret.ecol;
	std::string &svalue = ret.svalue;
	float &fvalue = ret.fvalue;
	int &ivalue = ret.ivalue;

	if (col == (int)line.length()) {
		type = TOK_EOL;
		goto done;
	}
	switch (line[col]) {
	case '(': type = TOK_LP; goto done;
	case ')': type = TOK_RP; goto done;
	case ':': type = TOK_COLON; goto done;
		
	case '#': type = TOK_DIRECTIVE; goto readtok;
	default:  type = TOK_IDENTIFIER; goto readtok;
	case '$': type = TOK_VAR; goto readtok;
	case '@': type = TOK_LOC; goto readtok;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '.': case '-': case '&':
		type = RAWTOK_NUM;
		goto readtok;

	readtok:
		while (!IsSpecial(ecol + 1) && !IsSpace(ecol + 1) && !IsLineComment(ecol + 1) && !IsBlockCommentBegin(ecol + 1)) ecol++;
		break;
		
	case '\"':
		type = RAWTOK_STR;
		for (ecol++; line[ecol] != '\"'; ecol++);
		break;
	}

done:
	svalue = line.substr(col, ecol - col + 1);
	col = std::min(ecol + 1, (int)line.length());
	ret.PostProcess();

	ret.Expect(expect);
	return ret;
}

bool scAsmAssembler::AsmLexer::NextLine()
{
	if (inclexer) {
		bool r = inclexer->NextLine();
		if (r) return true;
		inclexer.reset();
	}

	if (!incfn.empty()) {
		inclexer = std::make_unique<AsmLexer>();
		inclexer->LoadFile(incfn.c_str());
		incfn.clear();
		return true;
	}

	if (ln + 1 >= (int) text.size()) {
		return false;
	}
	ln++;
	col = 0;
	return true;
}

std::string scAsmAssembler::AsmLexer::GetCurrentFile()
{
	if (inclexer) return inclexer->GetCurrentFile();
	return txtfn;
}

void scAsmAssembler::AsmLexer::IncludeFile(const std::string &incfile)
{
	if (inclexer) {
		inclexer->IncludeFile(incfile);
		return;
	}

	incfn = incfile;
	// file will be included when NextLine() is called
}

void scAsmAssembler::AsmLexer::LoadFile(const std::string &txtfile)
{
	txtfn = txtfile;
	
	text.clear();
	ln = col = 0;
	FILE *fp = safe_fopen(txtfile.c_str(), "r");
	int ch;
	std::string buf;
	while ((ch = fgetc(fp)) != EOF) {
		if (ch == '\n') {
			text.push_back(buf);
			buf.clear();
		} else {
			buf += ch;
		}
	}
	if (!buf.empty()) text.push_back(buf);
	fclose(fp);
}

void scAsmAssembler::ParseBlock(const AsmToken &begintok)
{
	block.push_back(AsmBlock());
	AsmBlock &newblock = block.back();

	AsmToken tok = begintok;
	tok.Expect(TOK_DIRECTIVE, "#begin");
	tok = lexer.NextToken(TOK_INT);
	newblock.id = tok.ivalue;
	tok = lexer.NextToken(TOK_STR);
	newblock.desc = tok.svalue;
	lexer.NextToken(TOK_EOL);

	printf("  正分析  %s %5d  %s\n", getpathfilepart(lexer.GetCurrentFile().c_str()), newblock.id, newblock.desc.c_str());

	while (lexer.NextLine()) {
		tok = lexer.NextToken();
		if (tok.type == TOK_EOL) continue;

		if (tok.type == TOK_DIRECTIVE) {
			if (tok.svalue.compare("#var") == 0) {
				AsmUserVar newvar;
				tok = lexer.NextToken(TOK_VAR);
				newvar.name = tok.svalue;
				tok = lexer.NextToken(TOK_IDENTIFIER);
				newvar.type = VAR_INT; tok.Expect("INT");
				tok = lexer.NextToken();
				if (tok.type != TOK_EOL) {
					tok.Expect(TOK_INT);
					newvar.initflag = true;
					newvar.tok = tok;
				}
				//printf("   var %s init=%d ivalue=%d\n", newvar.name.c_str(), (int)newvar.initflag, newvar.ivalue);
				newblock.uservar.push_back(newvar);
				continue;
			} else if (tok.svalue.compare("#db") == 0) {
				AsmInstr dbinstr;
				dbinstr.isdb = true;
				while ((tok = lexer.NextToken()).type == TOK_INT) {
					dbinstr.db.push_back((unsigned char)tok.ivalue);
				}
				tok.Expect(TOK_EOL);
				newblock.instr.push_back(dbinstr);
				continue;
			} else {
				break;
			}
		}

		// parse real statements here
		AsmToken idtok = tok;
		tok = lexer.NextToken();
		AsmInstr newinstr;
		newinstr.idtok = idtok;

		if (tok.type == TOK_COLON) {
			
			idtok.Expect(TOK_LOC);
			lexer.NextToken(TOK_EOL);
			newinstr.islabel = true;
			//printf("   label %s\n", idtok.svalue.c_str());
		} else {
			
			idtok.Expect(TOK_IDENTIFIER);
			//idtok.Dump();
			newinstr.cmdid = scCmdDef::GetCmdID(idtok.svalue.c_str());
			if (newinstr.cmdid == -1) {
				scAsmAssembler::Instance()->ReportError(LVL_ERROR, idtok, "无此指令");
				continue;
			}

			for (; tok.type != TOK_EOL; tok = lexer.NextToken()) {
				newinstr.param.push_back(tok);
			}
			// check param
			int paramid = 0, rparamid = 0;
			int listflag = 0;
			scCmdDef &cmddef = scCmdDef::GetCmdDef(newinstr.cmdid);
			for (auto it = newinstr.param.rbegin(); it != newinstr.param.rend(); it++) {
				rparamid = cmddef.paramnum - paramid - 1;
				if (rparamid < 0) break;
				int expecttype = cmddef.paramlist[rparamid];
				if (it->type == TOK_RP) {
					listflag++;
					if (listflag > 1) ReportError(LVL_ERROR, *it, "禁止嵌套可变参数表");
					if (expecttype != VAR_LIST) {
						ReportError(LVL_ERROR, *it, "该位置不接受可变参数表");
					}
				} if (it->type == TOK_LP) {
					listflag--;
					paramid++;
				} else {
					if (listflag == 0) {

						int realtype = it->GetParamType();
						if (expecttype != realtype) {
							if (!(expecttype == VAR_INT && realtype == VAR_USERVAR)) {
								ReportError(LVL_ERROR, *it, "参数类型不正确");
							}
						}
						paramid++;
					}
				}
			}
			if (listflag != 0) {
				ReportError(LVL_ERROR, idtok, "可变参数表不完整");
			}
			if (rparamid < 0 || paramid != cmddef.paramnum) {
				ReportError(LVL_ERROR, idtok, "参数数量不正确");
			}

			
		}
		newblock.instr.push_back(newinstr);
	}

	tok.Expect(TOK_DIRECTIVE, "#end");
}

void scAsmAssembler::ParseSCE()
{
	do {
		AsmToken tok = lexer.NextToken();
		if (tok.type == TOK_EOL) continue;
		tok.Expect(TOK_DIRECTIVE);
		if (tok.svalue.compare("#begin") == 0) {
			ParseBlock(tok);
		} else if (tok.svalue.compare("#include") == 0) {
			AsmToken inctok;
			inctok = lexer.NextToken(TOK_STR);
			lexer.NextToken(TOK_EOL);
			lexer.IncludeFile(inctok.svalue.c_str());
		} else if (tok.svalue.compare("#define") == 0) {
			AsmToken mtok, vtok;
			mtok = lexer.NextToken(TOK_IDENTIFIER);
			vtok = lexer.NextToken();
			if (macro.insert(std::make_pair(mtok.svalue, std::make_pair(mtok, vtok))).second == false) {
				scAsmAssembler::Instance()->ReportError(LVL_ERROR, mtok, "重复的宏定义");
			}
			lexer.NextToken(TOK_EOL);
		} else {
			scAsmAssembler::Instance()->ReportError(LVL_ERROR, tok, "非法指示");
		}
	} while (lexer.NextLine());
}

scAsmAssembler::CodeBuffer::CodeBuffer() {}

std::shared_ptr<scAsmAssembler::CodeBuffer::BufferItem> scAsmAssembler::CodeBuffer::NewItem()
{
	return std::make_shared<BufferItem>();
}



void scAsmAssembler::CodeBuffer::Dump()
{
	unsigned addr = 0;
	for (auto &item: code) {
		printf("%08X: ", addr);
		for (unsigned char byte: item->data) {
			printf("%02X ", (unsigned)byte);
			addr++;
		}
		printf("\n");
	}
}
void scAsmAssembler::CodeBuffer::AssignAddress()
{
	unsigned addr = 0;
	for (auto &item: code) {
		item->addr = addr;
		addr += item->data.size();
	}
}
void scAsmAssembler::CodeBuffer::Relocate()
{
	AssignAddress();
	for (auto &item: code) {
		unsigned value = 0;
		switch (item->rtype) {
		case RELOC_ABSOLUTE: value = item->ptr.lock()->addr; break;
		case RELOC_DIFFERENCE: value = item->ptr2.lock()->addr - item->ptr.lock()->addr; break;
		default: continue;
		}

		switch (item->data.size()) {
		case 2: {
			unsigned short x;
			memcpy(&x, item->data.data(), 2);
			x += value;
			memcpy(item->data.data(), &x, 2);
			break;
		}
		case 4: {
			unsigned x;
			memcpy(&x, item->data.data(), 4);
			x += value;
			memcpy(item->data.data(), &x, 4);
			break;
		}
		default: assert(0);
		}
	}
}
std::vector<unsigned char> scAsmAssembler::CodeBuffer::GetData()
{
	std::vector<unsigned char> r;
	for (auto &item: code) {
		for (unsigned char byte: item->data) {
			r.push_back(byte);
		}
	}
	return r;
}
void scAsmAssembler::CodeBuffer::WriteFile(const char *outfile)
{
	FILE *fp = safe_fopen(outfile, "wb");
	for (auto &item: code) {
		for (unsigned char byte: item->data) {
			fputc(byte, fp);
		}
	}
	fclose(fp);
}

void scAsmAssembler::CodeBuffer::AppendItem(std::shared_ptr<BufferItem> item)
{
	code.push_back(item);
}

void scAsmAssembler::CodeBuffer::AppendData(const void *newdata, unsigned newlen)
{
	AppendItem(NewItem()->SetData(newdata, newlen));
}

std::shared_ptr<scAsmAssembler::CodeBuffer::BufferItem> scAsmAssembler::AsmBlock::NewLabel(const AsmToken &tok)
{
	BufferItemPtr p = scAsmAssembler::Instance()->code.NewItem();
	if (labelindex.insert(std::make_pair(tok.svalue, p)).second == false) {
		scAsmAssembler::Instance()->ReportError(LVL_ERROR, tok, "重复的标号");
	}
	return p;
}
std::shared_ptr<scAsmAssembler::CodeBuffer::BufferItem> scAsmAssembler::AsmBlock::FindLabel(AsmToken &tok)
{
	auto it = labelindex.find(tok.svalue);
	if (it == labelindex.end()) {
		scAsmAssembler::Instance()->ReportError(LVL_FATAL, tok, "找不到标号");
	}
	return it->second;
}

int scAsmAssembler::AsmBlock::GetUserVarID(AsmToken &tok)
{
	if (tok.svalue[1] == '!') {
		unsigned x;
		sscanf(tok.svalue.c_str() + 2, "%x", &x);
		return x;
	}
	for (int i = 0; i < (int)uservar.size(); i++) {
		if (uservar[i].name.compare(tok.svalue) == 0) {
			return i;
		}
	}
	scAsmAssembler::Instance()->ReportError(LVL_ERROR, tok, "找不到此用户变量");
	return -1;
}

void scAsmAssembler::ReportError(int level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	ReportError(level, AsmToken(), fmt, ap);
	va_end(ap);
}
void scAsmAssembler::ReportError(int level, const AsmToken &tok, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	ReportError(level, tok, fmt, ap);
	va_end(ap);
}
void scAsmAssembler::ReportError(int level, const AsmToken &tok, const char *fmt, va_list ap)
{
	if (tok.ln < 0 || tok.col < 0 || tok.ecol < 0) {
		scAsmTool::ResetLoc();
	} else {
		scAsmTool::SetFile(getpathfilepart(tok.txtfn.c_str()));
		scAsmTool::SetLine(tok.txtline);
		scAsmTool::SetLoc(tok.ln, tok.col, tok.ecol);
	}
	scAsmTool::ReportError(level, fmt, ap);
}

void scAsmAssembler::GenCode()
{
	// gen header
	code.AppendData("SCE", 4);
	code.AppendValue<BYTE>(1);
	code.AppendValue<WORD>((WORD)block.size());
	
	// gen index
	for (auto &curblock: block) {
		curblock.blockbegin = code.NewItem();
		code.AppendValue<DWORD>(curblock.id);
		code.AppendItem(code.NewItem()->SetValue(0, CodeBuffer::RELOC_ABSOLUTE, curblock.blockbegin));
		char desc[64]; strncpy(desc, curblock.desc.data(), sizeof(desc)); desc[64 - 1] = 0;
		code.AppendValue(desc);
	}

	// gen blocks
	for (auto &curblock: block) {
		// block header
		code.AppendItem(curblock.blockbegin);
		code.AppendValue<DWORD>(curblock.id);
		code.AppendValue<WORD>((WORD)(curblock.desc.length() + 1));
		code.AppendData(curblock.desc.c_str(), curblock.desc.length() + 1);

		// uservar
		code.AppendValue<WORD>((WORD)curblock.uservar.size());
		for (auto &curvar: curblock.uservar) {
			assert(curvar.type == VAR_INT);
			code.AppendValue<BYTE>(curvar.type);
			if (curvar.initflag) {
				assert(curvar.tok.type == TOK_INT);
				code.AppendValue<WORD>(4);
				code.AppendValue<int>(curvar.tok.ivalue);
			} else {
				code.AppendValue<WORD>(0);
			}
		}
		
		// byte code
		curblock.codebegin = code.NewItem();
		curblock.codeend = code.NewItem();
		code.AppendItem(code.NewItem()->SetValue(0, CodeBuffer::RELOC_DIFFERENCE, curblock.codebegin, curblock.codeend));
		code.AppendItem(curblock.codebegin);

		for (auto &instr: curblock.instr) { // create label items
			if (instr.islabel) {
				instr.labelptr = curblock.NewLabel(instr.idtok);
			}
		}

		for (auto &instr: curblock.instr) {
			if (instr.islabel) {
				code.AppendItem(instr.labelptr);
			} else if (instr.isdb) {
				code.AppendData(instr.db.data(), instr.db.size());
			} else {
				BufferItemPtr paramflagitem = code.NewItem();
				WORD paramflag = 0;
				std::set<int> rparamflag;

				code.AppendValue<WORD>(instr.cmdid);
				code.AppendItem(paramflagitem);

				BufferItemPtr varnumitem;
				WORD varnum;
				int listflag = 0;
				int paramcnt = 0;
				for (auto it = instr.param.rbegin(); it != instr.param.rend(); it++) {
					if (it->type == TOK_RP) {
						listflag++;
						varnumitem = code.NewItem();
						code.AppendItem(varnumitem);
						varnum = 0;
					} else if (it->type == TOK_LP) {
						listflag--;
						paramcnt++;
						varnumitem->SetValue<WORD>(varnum);
					} else {
						if (listflag == 0) {
							paramcnt++;
							if (it->type == TOK_VAR) {
								rparamflag.insert(paramcnt);
							}
							code.AppendItem(it->GetParamValue(curblock));
						} else {
							assert(it->type != TOK_VAR);
							code.AppendValue<BYTE>(it->GetParamType());
							code.AppendItem(it->GetParamValue(curblock));
							varnum++;
						}
					}
				}

				for (auto rid: rparamflag) {
					paramflag |= 1 << (paramcnt - rid);
				}
				paramflagitem->SetValue<WORD>(paramflag);
			}
		}

		code.AppendItem(curblock.codeend);
	}
}

void scAsmAssembler::DoAssemble(const char *asmfile)
{
	printf(" 词法语法分析 ...\n");
	lexer.LoadFile(asmfile);
	ParseSCE();
	if (scAsmTool::GetErrorCnt() > 0) scAsmTool::die(1);
	printf("\n");
	printf(" 代码生成 ...\n");
	GenCode();
	if (scAsmTool::GetErrorCnt() > 0) scAsmTool::die(1);
	printf(" 重定位 ...\n");
	code.Relocate();
}
void scAsmAssembler::AssembleSCE(const char *asmfile, const char *scefile)
{
	printf(" 正在对 %s 进行汇编并输出到 %s ...\n", asmfile, scefile);
	printf("\n");
	DoAssemble(asmfile);
	printf(" 写输出文件 ...\n");
	//code.Dump();
	code.WriteFile(scefile);
	printf("\n");
}
void scAsmAssembler::AssembleSCETest(const char *groundtruth, const char *asmfile)
{
	printf(" 正在对 %s 进行汇编并与 %s 进行比较 ...\n", asmfile, groundtruth);
	printf("\n");
	DoAssemble(asmfile);
	
	printf(" 正比较结果 ...\n");

	std::vector<unsigned char> asmresult, gtdata;
	asmresult = code.GetData();

	FILE *fp = safe_fopen(groundtruth, "rb");
	int ch;
	while ((ch = fgetc(fp)) != EOF) gtdata.push_back(ch);
	fclose(fp);

	// 清空索引中的名字（因为有些垃圾数据影响比较）
#pragma pack(1)
	struct SceFile {
		struct SceHeader hdr;
		struct SceIndex idx[0];
	};
#pragma pack()
	SceFile *s[] = { (SceFile *)asmresult.data(), (SceFile *)gtdata.data() };
	for (auto sf: s) {
		for (WORD i = 0; i < sf->hdr.blocknum; i++) {
			memset(sf->idx[i].desc, 0xCC, sizeof(sf->idx[i].desc));
		}
	}

	// compare 
	if (asmresult.size() != gtdata.size()) {
		ReportError(LVL_ERROR, "文件大小不匹配 asmresult-> %08X != %08X <-groundtruth", (unsigned)asmresult.size(), (unsigned)gtdata.size());
	}
	if (asmresult.size() == gtdata.size() || scAsmTool::dbgflag) {
		for (size_t i = 0; i < std::min(asmresult.size(), gtdata.size()); i++) {
			if (asmresult[i] != gtdata[i]) {
				ReportError(LVL_ERROR, "[%08X] asmresult-> %02X != %02X <-groundtruth", (unsigned)i, (unsigned)asmresult[i], (unsigned)gtdata[i]);
			}
		}
	}
	if (scAsmTool::GetErrorCnt() > 0) scAsmTool::die(1);

	printf("\n");
}
