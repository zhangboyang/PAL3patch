#include "stdafx.h"

XorRepair::XorRepair(ReadWriter *io, const SHA1Hash &checksum, const void *xorsum, size_t blksize, ProgressObject *progress) : po(progress)
{
	bs = blksize;
#define bw (bs / sizeof(unsigned long))
	assert(bs % sizeof(unsigned long) == 0);
	assert(bw % 2 == 0);

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

	cbuf = new unsigned long[bw];
	cidx = -1;

	buf = new unsigned long[bw * 2];
	hash = new SHA1Hash[n];

	po->set_maximum(sz);
}

XorRepair::~XorRepair()
{
	delete[] hash;
	delete[] buf;
	delete[] cbuf;
	delete[] sum;
	fp->dec();
}

bool XorRepair::cache(size_t blk)
{
	if (cidx != blk) {
		cidx = blk;
		cret = fp->read(cbuf, blk * bs, bs);
	}
	return cret;
}

void XorRepair::blkxor(unsigned long *c, const unsigned long *a, const unsigned long *b, size_t w)
{
	while (w--) *c++ = *a++ ^ *b++;
}

bool XorRepair::check()
{
	size_t i;
	ProgressBinder<size_t> pb(po, &i, bs);
	idx = -1;
	bad = false;
	for (i = 0; i < n; i++) {
		if (!pb.update()) return false;
		if (!cache(i)) {
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
		hash[i] = SHA1Hash::hash(cbuf, bs);
		blkxor(sum, sum, cbuf, bw);
	}
	return idx == -1 && SHA1Hash::hash_of_hashes(hash, n) == cksum;
}

bool XorRepair::tryfix(bool half)
{
	SHA1Hash bak = hash[idx];
	cache(idx);
	if (!half) {
		blkxor(buf, cbuf, sum, bw);
		hash[idx] = SHA1Hash::hash(buf, bs);
		if (SHA1Hash::hash_of_hashes(hash, n) == cksum) {
			shift = false;
			return true;
		}
	}
	size_t idx1 = idx < n - 1 ? idx + 1 : 0;
	SHA1Hash bak1 = hash[idx1];
	memcpy(buf, cbuf, bs / 2);
	blkxor(buf + bw / 2, cbuf + bw / 2, sum + bw / 2, bw / 2);
	hash[idx] = SHA1Hash::hash(buf, bs);
	cache(idx1);
	blkxor(buf + bw, cbuf, sum, bw / 2);
	memcpy(buf + bw + bw / 2, cbuf + bw / 2, bs / 2);
	hash[idx1] = SHA1Hash::hash(buf + bw, bs);
	if (SHA1Hash::hash_of_hashes(hash, n) == cksum) {
		shift = true;
		return true;
	}
	hash[idx] = bak;
	hash[idx1] = bak1;
	return false;
}

bool XorRepair::fix()
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
		ProgressBinder<size_t> pb(po, &idx, bs);
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

bool XorRepair::save()
{
	if (!shift) {
		return fp->write(buf, idx * bs, bs);
	} else {
		if (idx != n - 1) {
			return fp->write(buf + bw / 2, idx * bs + bs / 2, bs);
		} else {
			return fp->write(buf + bw / 2, idx * bs + bs / 2, bs / 2) && fp->write(buf + bw, 0, bs / 2);
		}
	}
}
