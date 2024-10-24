#include "stdafx.h"

XorRepair::XorRepair(ReadWriter *io, const SHA1Hash &checksum, const void *xorsum, size_t blksize, ProgressObject *progress) : po(progress)
{
	bs = blksize;
	if (io->size() % bs) io = new PaddingRW(io, bs);
	fp = io->inc();
	unsigned sz = fp->size();
	n = sz / bs;
	assert(sz % bs == 0);
	hash = new SHA1Hash[n];
	size_t bw = bs / sizeof(unsigned long);
	assert(bs % sizeof(unsigned long) == 0);
	buf = new unsigned long[bw];
	sum = new unsigned long[bw];
	if (xorsum) {
		memcpy(sum, xorsum, bs);
	} else {
		memset(sum, 0, bs);
	}
	cksum = checksum;
	bad = false;
	hint = false;
	po->set_maximum(sz);
}

XorRepair::~XorRepair()
{
	fp->dec();
	delete[] hash;
	delete[] buf;
	delete[] sum;
}

void XorRepair::blkxor(unsigned long *dst, const unsigned long *src)
{
	size_t bw = bs / sizeof(unsigned long);
	while (bw--) *dst++ ^= *src++;
}

bool XorRepair::check()
{
	size_t i;
	ProgressBinder<size_t> pb(po, &i, bs);
	for (i = 0; i < n; i++) {
		if (!pb.update()) return false;
		if (!fp->read(buf, i * bs, bs)) {
			if (hint) {
				bad = true;
				return false;
			} else {
				idx = i;
				hint = true;
			}
		}
		hash[i] = SHA1Hash::hash(buf, bs);
		blkxor(sum, buf);
	}
	return !hint && SHA1Hash::hash_of_hashes(hash, n) == cksum;
}

bool XorRepair::tryfix()
{
	SHA1Hash bak = hash[idx];
	fp->read(buf, idx * bs, bs);
	blkxor(buf, sum);
	hash[idx] = SHA1Hash::hash(buf, bs);
	if (SHA1Hash::hash_of_hashes(hash, n) == cksum) {
		return true;
	} else {
		hash[idx] = bak;
		return false;
	}
}

bool XorRepair::fix()
{
	if (bad) return false;
	if (hint) {
		return tryfix();
	} else {
		ProgressBinder<size_t> pb(po, &idx, bs);
		for (idx = 0; idx < n; idx++) {
			if (!pb.update()) return false;
			if (tryfix()) {
				pb.override(n);
				return true;
			}
		}
		return false;
	}
}

bool XorRepair::write()
{
	return fp->write(buf, idx * bs, bs);
}
