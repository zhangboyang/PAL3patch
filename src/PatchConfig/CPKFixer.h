#ifndef PAL3PATCHCONFIG_CPKFIXER
#define PAL3PATCHCONFIG_CPKFIXER

struct tagCPKHeader {
	DWORD dwLable;
	DWORD dwVersion;
	DWORD dwTableStart;
	DWORD dwDataStart;
	DWORD dwMaxFileNum;
	DWORD dwFileNum;
	DWORD dwIsFormatted;
	DWORD dwSizeOfHeader;
	DWORD dwValidTableNum;
	DWORD dwMaxTableNum;
	DWORD dwFragmentNum;
	DWORD dwPackageSize;
	DWORD dwReserved[20];
};

struct tagCPKTable {
	DWORD dwCRC;
	DWORD dwFlag;
	DWORD dwFatherCRC;
	DWORD dwStartPos;
	DWORD dwPackedSize;
	DWORD dwOriginSize;
	DWORD dwExtraInfoSize;

	bool check(const tagCPKTable &other) const;
	bool valid() const;
	bool operator<(const tagCPKTable &other) const;
};

class CPKExtraFixer : virtual public ReferenceCounter {
protected:
	ReadWriter *fp;
	unsigned base;
	unsigned sz;
private:
	unsigned extra;
	const char *str;
	int state;
public:
	virtual bool check();
	virtual bool flush();
public:
	CPKExtraFixer(ReadWriter *io, DWORD dwStartPos, DWORD dwPackedSize, DWORD dwExtraInfoSize, const char *filename);
	~CPKExtraFixer();
};

class CPKFileFixer : public ReadWriter, public CPKExtraFixer {
public:
	bool read(void *buffer, unsigned offset, size_t length);
	bool write(const void *buffer, unsigned offset, size_t length);
	unsigned size();
public:
	CPKFileFixer(ReadWriter *io, DWORD dwStartPos, DWORD dwPackedSize, DWORD dwExtraInfoSize, const char *filename);
};

class CPKSpecialFixer : public CPKExtraFixer {
private:
	const void *raw;
	int state;
public:
	CPKSpecialFixer(ReadWriter *io, DWORD dwStartPos, DWORD dwPackedSize, DWORD dwExtraInfoSize, const void *data);
	bool check();
	bool flush();
};

class CPKFixer {
private:
	const char *cpkpath;
	unsigned blksize;
	const void *xorsum;
	SHA1Hash checksum;
	std::vector<tagCPKTable> files;
	std::vector<const char *> names;
	std::vector<std::pair<tagCPKTable, const void *> > special_files;

	FileRW *fp;
	tagCPKHeader hdr;
	tagCPKTable tbl[0x8000];
	bool dirty;

	XorRepair *xr;
	std::vector<CPKExtraFixer *> fixers;

	void load_repair(BufferReader &r);
	void reset();
	bool check_tbl();
public:
	CPKFixer::CPKFixer(const char *cpk, BufferReader &r);
	~CPKFixer();
	bool load(bool rebuild);
	int check(ProgressObject *progress);
	bool fix();
};

#endif
