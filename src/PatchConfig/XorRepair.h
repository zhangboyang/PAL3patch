#ifndef PAL3PATCHCONFIG_XORREPAIR
#define PAL3PATCHCONFIG_XORREPAIR

class XorRepair {
private:
	ReadWriter *fp;
	unsigned bs;
	size_t n;
	SHA1Hash *hash;
	unsigned long *buf;
	unsigned long *sum;
	SHA1Hash cksum;
	size_t idx;
	bool bad;
	bool hint;
	ProgressObject *po;
private:
	void blkxor(unsigned long *dst, const unsigned long *src);
	bool tryfix();
public:
	XorRepair(ReadWriter *io, const SHA1Hash &checksum, const void *xorsum, size_t blksize, ProgressObject *progress);
	~XorRepair();
	bool check();
	bool fix();
	bool write();
};

#endif
