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
	wchar_t *path;
	unsigned sz;
	bool rw;
public:
	bool read(void *buffer, unsigned offset, size_t length);
	bool write(const void *buffer, unsigned offset, size_t length);
	unsigned size();
public:
	FileRW(const char *filepath, unsigned filesize);
	~FileRW();
	void enablewrite();
	void resize(unsigned filesize);
	unsigned realsize();
	bool truncate();
	bool sync();
	bool commit();
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

class CompareRW : public ReadWriter {
private:
	ReadWriter *fp;
public:
	bool read(void *buffer, unsigned offset, size_t length);
	bool write(const void *buffer, unsigned offset, size_t length);
	unsigned size();
public:
	CompareRW(ReadWriter *io);
	~CompareRW();
};

class CacheRW : public ReadWriter {
private:
	void *cache;
	unsigned base;
	size_t count;
	ReadWriter *fp;
public:
	bool read(void *buffer, unsigned offset, size_t length);
	bool write(const void *buffer, unsigned offset, size_t length);
	unsigned size();
public:
	CacheRW(ReadWriter *io);
	~CacheRW();
};

#endif
