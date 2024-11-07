#ifndef PAL3PATCHCONFIG_XORREPAIR
#define PAL3PATCHCONFIG_XORREPAIR

class XorRepair : public RepairCommitter {
private:
	ReadWriter *fp;
	size_t bw;
	unsigned n;
	SHA1Hash cksum;
	unsigned long *sum;
	unsigned long *blk;
	unsigned long *buf;
	SHA1Hash *hash;
	unsigned idx;
	unsigned cidx;
	bool cret;
	bool precise;
	bool bad;
	unsigned char shift;
private:
	bool loadblk(unsigned i);
	void xorw(unsigned long *c, const unsigned long *a, const unsigned long *b, size_t w);
	void xorw(unsigned long *b, const unsigned long *a, size_t w);
	bool tryfix(bool skip);
	bool check(ProgressObject *progress);
	bool fix(ProgressObject *progress);
public:
	XorRepair(ReadWriter *io, const SHA1Hash &checksum, const void *xorsum, size_t blksize);
	~XorRepair();
	int repair(ProgressObject *progress);
	bool commit();
};

#endif
