#include "stdafx.h"

#define NUM_SHIFT 16

XorRepair::XorRepair(ReadWriter *io, const SHA1Hash &checksum, const void *xorsum, size_t blksize)
{
	bw = blksize / sizeof(unsigned long);
	assert(blksize % sizeof(unsigned long) == 0);
	assert(bw % NUM_SHIFT == 0);
#define bs (bw * sizeof(unsigned long))

	if (io->size() % bs) io = new PaddingRW(io, bs);
	fp = io;
	fp->inc();
	unsigned sz = fp->size();
	n = sz / bs;
	assert(sz % bs == 0);
	assert(n >= 3);

	cksum = checksum;
	sum = new unsigned long[bw];
	if (xorsum) {
		memcpy(sum, xorsum, bs);
	} else {
		memset(sum, 0, bs);
	}

	blk = new unsigned long[bw];
	cidx = -1;

	buf = new unsigned long[bw * 2];
	hash = new SHA1Hash[n];
}

XorRepair::~XorRepair()
{
	delete[] hash;
	delete[] buf;
	delete[] blk;
	delete[] sum;
	fp->dec();
}

bool XorRepair::loadblk(unsigned i)
{
	if (cidx != i) {
		cidx = i;
		cret = fp->read(blk, i * bs, bs);
	}
	return cret;
}

void XorRepair::xorw(unsigned long *c, const unsigned long *a, const unsigned long *b, size_t w)
{
	while (w--) *c++ = *a++ ^ *b++;
}

void XorRepair::xorw(unsigned long *b, const unsigned long *a, size_t w)
{
	while (w--) *b++ ^= *a++;
}

bool XorRepair::check(ProgressObject *progress)
{
	unsigned i;
	ProgressBinder<unsigned> pb(progress, &i, n, bs);
	idx = -1;
	bad = false;
	for (i = 0; i < n; i++) {
		if (!pb.update()) return false;
		if (!loadblk(i)) {
			if (idx == -1) {
				idx = i;
				precise = false;
			} else {
				if (i != idx + 1) {
					if (idx == 0 && i == n - 1) {
						idx = n - 1;
					} else {
						bad = true;
						return false;
					}
				}
				precise = true;
			}
		}
		hash[i] = SHA1Hash::hash(blk, bs);
		xorw(sum, blk, bw);
	}
	return idx == -1 && SHA1Hash::hash_of_hashes(hash, n) == cksum;
}

bool XorRepair::tryfix(bool skip)
{
	SHA1Hash bak = hash[idx];
	loadblk(idx);
	xorw(buf, blk, sum, bw);
	if (!skip) {
		hash[idx] = SHA1Hash::hash(buf, bs);
		if (SHA1Hash::hash_of_hashes(hash, n) == cksum) {
			shift = 0;
			return true;
		}
	}
	unsigned idx1 = idx < n - 1 ? idx + 1 : 0;
	SHA1Hash bak1 = hash[idx1];
	loadblk(idx1);
	memcpy(buf + bw, blk, bs);
	for (shift = 1; shift < NUM_SHIFT; shift++) {
		size_t w = bw / NUM_SHIFT;
		size_t sw = shift * w;
		unsigned long *l = buf + (sw - w);
		unsigned long *r = buf + bw + (sw - w);
		unsigned long *s = sum + (sw - w);
		xorw(l, s, w);
		xorw(r, s, w);
		hash[idx] = SHA1Hash::hash(buf, bs);
		hash[idx1] = SHA1Hash::hash(buf + bw, bs);
		if (SHA1Hash::hash_of_hashes(hash, n) == cksum) {
			return true;
		}
	}
	hash[idx] = bak;
	hash[idx1] = bak1;
	return false;
}

bool XorRepair::fix(ProgressObject *progress)
{
	if (bad) return false;
	if (idx != -1) {
		if (precise) {
			return tryfix(true);
		} else {
			if (tryfix(false)) return true;
			idx = idx > 0 ? idx - 1 : n - 1;
			return tryfix(true);
		}
	} else {
		ProgressBinder<unsigned> pb(progress, &idx, n, bs);
		for (idx = 0; idx < n; idx++) {
			if (!pb.update()) return false;
			if (tryfix(false)) {
				pb.override(n);
				return true;
			}
		}
		return false;
	}
}

int XorRepair::repair(ProgressObject *progress)
{
 	if (check(progress)) return 0;
	if (progress->cancelled()) return -1;
	return fix(progress) ? 1 : -1;
}

bool XorRepair::commit()
{
	if (!shift) {
		return fp->write(buf, idx * bs, bs);
	} else {
		size_t sw = shift * (bw / NUM_SHIFT);
		size_t ss = shift * (bs / NUM_SHIFT);
		if (idx != n - 1) {
			return fp->write(buf + sw, idx * bs + ss, bs);
		} else {
			return fp->write(buf + sw, idx * bs + ss, bs - ss) && fp->write(buf + bw, 0, ss);
		}
	}
}
