#ifndef PAL3PATCHCONFIG_XORREPAIR
#define PAL3PATCHCONFIG_XORREPAIR

class XorRepair {
private:
	ReadWriter *fp;
	unsigned bs;
	size_t n;
	SHA1Hash cksum;
	unsigned long *sum;
	unsigned long *cbuf;
	unsigned long *buf;
	SHA1Hash *hash;
	size_t idx;
	size_t cidx;
	bool cret;
	bool precise;
	bool bad;
	bool shift;
	ProgressObject *po;
private:
	bool cache(size_t blk);
	void blkxor(unsigned long *c, const unsigned long *a, const unsigned long *b, size_t w);
	bool tryfix(bool half);
public:
	XorRepair(ReadWriter *io, const SHA1Hash &checksum, const void *xorsum, size_t blksize, ProgressObject *progress);
	~XorRepair();
	bool check();
	bool fix();
	bool commit();
};

#endif
