#ifndef PAL3PATCHCONFIG_READWRITER
#define PAL3PATCHCONFIG_READWRITER

class ReadWriter : public virtual ReferenceCounter {
public:
	virtual bool read(void *buffer, unsigned offset, size_t length) = 0;
	virtual bool write(const void *buffer, unsigned offset, size_t length) = 0;
	virtual unsigned size() = 0;
};

class FileRW : public ReadWriter {
private:
	static HANDLE handle;
	static FileRW *owner;
	bool open();
	void close();
	bool seek(unsigned offset);
private:
	std::string path;
	unsigned sz;
	bool rw;
public:
	bool read(void *buffer, unsigned offset, size_t length);
	bool write(const void *buffer, unsigned offset, size_t length);
	unsigned size();
public:
	FileRW(const std::string &filepath, unsigned filesize);
	~FileRW();
	bool reopen(bool readwrite);
	void resize(unsigned filesize);
	unsigned realsize();
	bool truncate();
};

class RangeRW : public ReadWriter {
private:
	ReadWriter *fp;
	unsigned base;
	unsigned sz;
public:
	bool read(void *buffer, unsigned offset, size_t length);
	bool write(const void *buffer, unsigned offset, size_t length);
	unsigned size();
public:
	RangeRW(ReadWriter *io, unsigned rangebase, unsigned rangesize);
	~RangeRW();
};

class DummyRW : public ReadWriter {
private:
	unsigned sz;
public:
	bool read(void *buffer, unsigned offset, size_t length);
	bool write(const void *buffer, unsigned offset, size_t length);
	unsigned size();
public:
	DummyRW(unsigned fakesize);
};

class ConcatRW : public ReadWriter {
private:
	std::vector<ReadWriter *> fp;
	std::vector<unsigned> tbl;
private:
	bool readwrite(void *rbuffer, const void *wbuffer, unsigned offset, size_t length);
public:
	bool read(void *buffer, unsigned offset, size_t length);
	bool write(const void *buffer, unsigned offset, size_t length);
	unsigned size();
public:
	ConcatRW();
	~ConcatRW();
	void append(ReadWriter *io);
};

class PaddingRW : public ReadWriter {
private:
	ReadWriter *fp;
	unsigned realsz;
	unsigned sz;
public:
	bool read(void *buffer, unsigned offset, size_t length);
	bool write(const void *buffer, unsigned offset, size_t length);
	unsigned size();
public:
	PaddingRW(ReadWriter *io, unsigned alignment);
	~PaddingRW();
};

#endif
