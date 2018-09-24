#pragma once

class scAsmAssembler {
	class AsmBlock;
	class CodeBuffer {
	public:
		enum RELOCTYPE {
			RELOC_NONE,
			RELOC_ABSOLUTE,
			RELOC_DIFFERENCE,
		};
		class BufferItem : public std::enable_shared_from_this<BufferItem> {
		public:
			unsigned addr = 0;
			std::vector<unsigned char> data;
			RELOCTYPE rtype = RELOC_NONE;
			std::weak_ptr<BufferItem> ptr, ptr2;
			std::shared_ptr<BufferItem> SetData(const void *newdata, unsigned newlen)
			{
				data.resize(newlen);
				memcpy(data.data(), newdata, newlen);
				return shared_from_this();
			}
			std::shared_ptr<BufferItem> AppendData(const void *newdata, unsigned newlen)
			{
				size_t offset = data.size();
				data.resize(offset + newlen);
				memcpy(data.data() + offset, newdata, newlen);
				return shared_from_this();
			}
			template<class T> std::shared_ptr<BufferItem> SetValue(const T &value)
			{
				data.resize(sizeof(T));
				memcpy(data.data(), &value, sizeof(T));
				return shared_from_this();
			}
			template<class T> std::shared_ptr<BufferItem> SetValue(const T &value, RELOCTYPE rtype, std::weak_ptr<BufferItem> ptr = std::weak_ptr<BufferItem>(), std::weak_ptr<BufferItem> ptr2 = std::weak_ptr<BufferItem>())
			{
				SetValue<T>(value);
				this->rtype = rtype;
				this->ptr = ptr;
				this->ptr2 = ptr2;
				return shared_from_this();
			}
		};
	private:
		std::list<std::shared_ptr<BufferItem> > code;

		CodeBuffer(const CodeBuffer&) = delete;  
		CodeBuffer& operator = (const CodeBuffer&) = delete;
	public:
		std::shared_ptr<BufferItem> NewItem();
		void AppendItem(std::shared_ptr<BufferItem> item);
		void AppendData(const void *newdata, unsigned newlen);
		template<class T> void AppendValue(const T &data)
		{
			AppendItem(NewItem()->SetValue<T>(data));
		}
		void Dump();
		void AssignAddress();
		void Relocate();
		std::vector<unsigned char> GetData();
		void WriteFile(const char *outfile);
		CodeBuffer();
	};
	typedef std::shared_ptr<CodeBuffer::BufferItem> BufferItemPtr;


	enum TOKENTYPE {
		TOK_INVALID, // invalid token
		TOK_EOL, // end of line
		TOK_DIRECTIVE,  // #xxxxx
		TOK_IDENTIFIER, // cmdname
		TOK_FLOAT,
		TOK_INT,
		TOK_VAR, // $xxx  $!xxx
		TOK_LOC, // @xxx
		TOK_STR, // "xxx"
		TOK_LP, // (
		TOK_RP, // )
		TOK_COLON, // :

		// special
		RAWTOK_NUM, // will convert to TOK_INT or TOK_FLOAT
		RAWTOK_STR, // will convert to TOK_STR
		ANY_TOK, // any token match flag
	};

	class AsmToken {
	public:
		std::string txtfn;
		std::string txtline;
		enum TOKENTYPE type = TOK_INVALID;
		std::string svalue;
		float fvalue = .0f;
		int ivalue = 0;
		int ln = -1, col = -1, ecol = -1;

		void MacroSubstitute(AsmToken &vtok);
		void PostProcess();
		void Dump();
		void Expect(TOKENTYPE expect_type);
		void Expect(const std::string &expect_svalue);
		void Expect(TOKENTYPE expect_type, const std::string &expect_svalue);
		int GetParamType();
		BufferItemPtr GetParamValue(AsmBlock &block);
	};

	class AsmLexer {
		int ln, col;
		bool IsSpecial(int p);
		bool IsSpace(int p);
		bool IsComment(int p);
		void SkipSpace();

		std::string txtfn;
		std::vector<std::string> text;
		std::string incfn;
		std::unique_ptr<AsmLexer> inclexer;

	public:
		void LoadFile(const std::string &txtfile);
		AsmToken NextToken(TOKENTYPE expect = ANY_TOK);
		bool NextLine();
		void IncludeFile(const std::string &incfile);
		std::string GetCurrentFile();
	};

	class AsmUserVar {
	public:
		std::string name;
		int type = VAR_INT;
		bool initflag = false;
		AsmToken tok;
	};

	
	class AsmInstr {
	public:
		AsmToken idtok;
		
		bool isdb = false;
		std::vector<unsigned char> db;

		bool islabel = false;
		BufferItemPtr labelptr;
		
		int cmdid = -1;
		std::vector<AsmToken> param;
	};



	class AsmBlock {
	public:
		int id;
		std::string desc;
		std::vector<AsmUserVar> uservar;
		std::vector<AsmInstr> instr;
		BufferItemPtr blockbegin;
		BufferItemPtr codebegin;
		BufferItemPtr codeend;
		std::map<std::string, BufferItemPtr> labelindex;

		BufferItemPtr NewLabel(const AsmToken &tok);
		BufferItemPtr FindLabel(AsmToken &tok);
		int GetUserVarID(AsmToken &tok);
	};

	AsmLexer lexer;
	std::vector<AsmBlock> block;
	CodeBuffer code;

	// name -> (macro-token, value-token)
	std::map<std::string, std::pair<AsmToken, AsmToken>> macro;

	void ParseBlock(const AsmToken &begintok);
	void ParseSCE();
	void GenCode();
	void ReportError(int level, const char *fmt, ...);
	void ReportError(int level, const AsmToken &tok, const char *fmt, ...);
	void ReportError(int level, const AsmToken &tok, const char *fmt, va_list ap);
	void DoAssemble(const char *asmfile);
public:
	void AssembleSCE(const char *asmfile, const char *scefile);
	void AssembleSCETest(const char *groundtruth, const char *asmfile);

private:
	scAsmAssembler() {}
public:
	scAsmAssembler(scAsmAssembler const&) = delete;
	void operator=(scAsmAssembler const&) = delete;
	static scAsmAssembler *Instance();
};