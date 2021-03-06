#pragma once

class scAsmDisassembler {
	
	class scUserVarInfo {
	public:
		BYTE type;
		WORD initlen;
		std::vector<unsigned char> initdata;
		template<class T> T get()
		{
			T ret;
			assert(initdata.size() == sizeof(T));
			memcpy(&ret, initdata.data(), sizeof(T));
			return ret;
		}
	};
	class scScriptBlock {
	private:
		size_t codeptr;
		template<class T> T get(bool *errfuse = NULL)
		{
			T ret;
			if (errfuse != NULL && codeptr + sizeof(T) > codesize) {
				memset(&ret, 0, sizeof(ret));
				*errfuse = true;
			} else {
				assert(codeptr + sizeof(T) <= codesize);
				memcpy(&ret, codedata.data() + codeptr, sizeof(T));
				codeptr += sizeof(T);
			}
			return ret;
		}
		std::vector<char> get_string();
		

	public:
		DWORD	id;	//script id
		DWORD   block_fileoffset;
		DWORD   code_fileoffset;
		std::vector<char> desc;
		DWORD	uservarnum;
		std::vector<scUserVarInfo> uservar;
		DWORD	codesize;
		std::vector<unsigned char> codedata;

		class scAsmInstr {
		public:
			class scAsmInstrParam {
			public:
				BYTE type;
				int idata;
				float fdata;
				std::vector<char> sdata;
				DWORD jdata;
				WORD udata;
				std::vector<scAsmInstrParam> ldata;
				
				void GetParam(scScriptBlock *block, BYTE t);
				void PrintParam(FILE *fp);
				
				



			};
			bool iscomment = false;
			std::string comment;
			WORD cmdid;
			WORD paramflag;
			std::vector<scAsmInstrParam> paramlist;
		};

		std::map<std::pair<unsigned, unsigned>, scAsmInstr> codeasm;
		std::set<unsigned> codelabel;

		static std::string escape_string(const std::vector<char> &str);

		void DisassembleCodeData();
	};

	
	std::string scename;
	SceHeader header;
	std::vector<SceIndex> index;
	std::vector<scScriptBlock> block;

	

private:
	void ReadSCEFile(const char *scefile);
	void DisassembleAllBlocks();
	void WriteASMFile(const char *asmfile);
public:
	void DisassembleSCE(const char *scefile, const char *asmfile);

private:
	scAsmDisassembler() {}
public:
	scAsmDisassembler(scAsmDisassembler const&) = delete;
	void operator=(scAsmDisassembler const&) = delete;
	static scAsmDisassembler *Instance();
};