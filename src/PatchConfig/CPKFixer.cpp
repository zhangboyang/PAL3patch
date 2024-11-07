#include "stdafx.h"

static unsigned gbCrc32Compute(const char *str)
{
	static const unsigned table[] = {
		0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
		0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
		0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
		0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
		0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
		0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
		0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
		0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
		0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
		0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
		0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
		0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
		0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
		0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
		0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
		0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
		0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
		0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
		0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
		0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
		0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
		0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
		0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
		0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
		0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
		0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
		0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
		0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
		0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
		0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
		0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
		0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
	};
	const unsigned char *s = (const unsigned char *) str;
	unsigned x = 0;
	int i;
	for (i = 0; i < 4; i++) {
		x = (x << 8);
		if (*s) x |= *s++;
	}
	x = ~x;
	while (*s) {
		x = table[x >> 24] ^ (*s++ | (x << 8));
	}
	return ~x;
}

static char *chinese_strlwr(char *str)
{
    char *s;
    for (s = str; *s; s++) {
        if (*s >> 7) {
            if (s[1]) s++;
        } else {
            if ('A' <= *s && *s <= 'Z') *s = *s - 'A' + 'a';
        }
    }
    return str;
}

static unsigned cpk_crc32(const char *path)
{
	char *str = chinese_strlwr(strdup(path));
	unsigned ret = gbCrc32Compute(str);
	free(str);
	return ret;
}

void CPKFixer::init(BufferReader &r)
{
	int i, j;

	blksize = r.u32();
	
	int num_files = r.u16();
	files.reserve(num_files);
	names.reserve(num_files);
	
	int num_special = r.u16();
	special_files.reserve(num_special);

	tagCPKTable item;
	item.dwCRC = 0;
	item.dwFatherCRC = 0xffff;
	item.dwExtraInfoSize = 0;
	for (i = 0; i < num_files; i++) {
		item.dwStartPos = 0;
		unsigned a = r.u32();
		if ((a & 0x80000000)) {
			unsigned b = r.u32();
			item.dwPackedSize = a & 0x3fffffff;
			item.dwOriginSize = b & 0x3fffffff;
			if ((b & 0x80000000)) {
				item.dwFlag = 0x00010005;
			} else if (item.dwPackedSize != 0) {
				item.dwFlag = 0x00020001;
			} else {
				item.dwFlag = 0x00000003;
			}
			if ((a & 0x40000000)) {
				item.dwStartPos = r.u32();
			}
			if ((b & 0x40000000)) {
				item.dwFatherCRC = r.u16();
			}
		} else {
			item.dwFlag = 0x00020001;
			item.dwPackedSize = a >> 16;
			item.dwOriginSize = a & 0x0000ffff;
		}
		files.push_back(item);
		if (item.dwFlag == 0x00000003) {
			item.dwFatherCRC = i;
		}
	}

	for (i = 0; i < num_files; i++) {
		names.push_back(r.str());
	}

	DWORD dwStartPos = 0x000e0080;
	for (i = 0; i < num_files; i++) {
		tagCPKTable &cur = files[i];
		cur.dwExtraInfoSize = strlen(names[i]) + 2 + 56;
		if (cur.dwStartPos) {
			dwStartPos = cur.dwStartPos;
		} else {
			cur.dwStartPos = dwStartPos;
		}
		dwStartPos += cur.dwPackedSize + cur.dwExtraInfoSize;
	}

	for (i = 0; i < num_files; i++) {
		std::string path = names[i];
		for (j = files[i].dwFatherCRC; j != 0xffff; j = files[j].dwFatherCRC) {
			path = std::string(names[j]) + '\\' + path;
		}
		files[i].dwCRC = cpk_crc32(path.c_str());
	}

	for (i = 0; i < num_files; i++) {
		tagCPKTable &cur = files[i];
		cur.dwFatherCRC = cur.dwFatherCRC != 0xffff ? files[cur.dwFatherCRC].dwCRC : 0;
	}

	xorsum = r.adv(blksize);
	r.cpy(&checksum, sizeof(checksum));

	item.dwStartPos = 0;
	for (i = 0; i < num_special; i++) {
		item.dwCRC = r.u32();
		item.dwFlag = r.u32();
		item.dwFatherCRC = r.u32();
		item.dwPackedSize = r.u32();
		item.dwOriginSize = r.u32();
		const void *data = r.adv(item.dwPackedSize);
		item.dwExtraInfoSize = strlen(r.str()) + 2 + 56;
		special_files.push_back(std::make_pair(item, data));
	}
}

bool tagCPKTable::check(const tagCPKTable &other) const
{
	return dwCRC == other.dwCRC &&
	       dwFlag == other.dwFlag &&
		   dwFatherCRC == other.dwFatherCRC &&
		   dwPackedSize == other.dwPackedSize &&
		   dwOriginSize == other.dwOriginSize &&
		   dwExtraInfoSize == other.dwExtraInfoSize;
}

bool tagCPKTable::valid() const
{
	return (dwFlag & 0x1) && !(dwFlag & 0x10);
}

bool tagCPKTable::operator<(const tagCPKTable &other) const
{
	if (dwCRC != other.dwCRC) return dwCRC < other.dwCRC;
	return !valid() && other.valid();
}

CPKFixer::CPKFixer(const char *cpk, BufferReader &r)
{
	assert(sizeof(hdr) == 0x80);
	assert(sizeof(tbl) == 0xe0000);
	cpkpath = cpk;
	init(r);
	fp = NULL;
	xr = NULL;
}

CPKFixer::~CPKFixer()
{
	reset();
}

#define CHECK_ASSIGN(a, b) do { if (a != b) { dirty = true; a = b; } } while (0)
#define SAFE_LIMIT 0x40000000u

bool CPKFixer::check_tbl()
{
	int i, j;

	if (hdr.dwValidTableNum > hdr.dwMaxTableNum) return false;

	for (i = 1; i < hdr.dwValidTableNum; i++) {
		tagCPKTable &a = tbl[i - 1];
		tagCPKTable &b = tbl[i];
		if (a < b) {
			continue;
		} else if (b < a) {
			return false;
		} else {
			if (a.valid()) return false;
		}
	}

	DWORD dwFileNum = 0;
	DWORD dwFragmentNum = 0;
	for (i = 0; i < hdr.dwValidTableNum; i++) {
		tagCPKTable &cur = tbl[i];
		if (cur.valid()) {
			if (cur.dwFlag != 0x00020001 && cur.dwFlag != 0x00010005 && cur.dwFlag != 0x00000003) return false;
			dwFileNum++;
		} else {
			if (cur.dwFlag != 0x00020011 && cur.dwFlag != 0x00010015 && cur.dwFlag != 0x00000013) return false;
			dwFragmentNum++;
		}
	}
	CHECK_ASSIGN(hdr.dwFileNum, dwFileNum);
	CHECK_ASSIGN(hdr.dwFragmentNum, dwFragmentNum);

	std::vector<std::pair<DWORD, DWORD> > ranges;
	ranges.reserve(hdr.dwValidTableNum);
	for (i = 0; i < hdr.dwValidTableNum; i++) {
		tagCPKTable &cur = tbl[i];
		if (cur.dwStartPos >= SAFE_LIMIT) return false;
		if (cur.dwPackedSize >= SAFE_LIMIT) return false;
		if (cur.dwExtraInfoSize >= SAFE_LIMIT) return false;
		DWORD low = cur.dwStartPos;
		DWORD high = low + cur.dwPackedSize + cur.dwExtraInfoSize;
		if (high >= SAFE_LIMIT) return false;
		ranges.push_back(std::make_pair(low, high));
	}
	std::sort(ranges.begin(), ranges.end());
	hdr.dwPackageSize = sizeof(hdr) + sizeof(tbl);
	for (i = 0; i < hdr.dwValidTableNum; i++) {
		DWORD low = ranges[i].first;
		DWORD high = ranges[i].second;
		if (low < hdr.dwPackageSize) return false;
		hdr.dwPackageSize = high;
	}

	std::map<DWORD, int> rank;
	std::map<DWORD, int>::iterator it;
	for (i = 0; i < hdr.dwValidTableNum; i++) {
		if (tbl[i].valid()) rank[tbl[i].dwCRC] = i;
	}
	int num_files = files.size();
	for (i = 0; i < num_files; i++) {
		tagCPKTable &cur = files[i];
		if ((it = rank.find(cur.dwCRC)) == rank.end()) return false;
		if (!cur.check(tbl[it->second])) return false;
	}
	int num_special = special_files.size();
	bool need_sort = false;
	for (i = 0; i < num_special; i++) {
		tagCPKTable &cur = special_files[i].first;
		if ((it = rank.find(cur.dwCRC)) == rank.end()) {
			if (hdr.dwValidTableNum >= hdr.dwMaxTableNum) return false;
			j = hdr.dwValidTableNum++;
			hdr.dwFileNum++;
			need_sort = true;
		} else if (!cur.check(tbl[it->second])) {
			j = it->second;
		} else {
			continue;
		}
		dirty = true;
		tbl[j] = cur;
		tbl[j].dwStartPos = hdr.dwPackageSize;
		hdr.dwPackageSize += tbl[j].dwPackedSize + tbl[j].dwExtraInfoSize;
	}
	if (need_sort) {
		std::stable_sort(tbl, tbl + hdr.dwValidTableNum);
	}

	return true;
}

void CPKFixer::rebuild_tbl()
{
	dirty = true;
	int num_files = files.size();
	int num_special = special_files.size();
	hdr.dwValidTableNum = hdr.dwFileNum = num_files + num_special;
	hdr.dwFragmentNum = 0;
	int i, j;
	for (i = 0; i < num_files; i++) {
		tbl[i] = files[i];
	}
	hdr.dwPackageSize = files.back().dwStartPos + files.back().dwPackedSize + files.back().dwExtraInfoSize;
	for (i = 0; i < num_special; i++) {
		j = num_files + i;
		tbl[j] = special_files[i].first;
		tbl[j].dwStartPos = hdr.dwPackageSize;
		hdr.dwPackageSize += tbl[j].dwPackedSize + tbl[j].dwExtraInfoSize;
	}
	std::sort(tbl, tbl + hdr.dwValidTableNum);
}

void CPKFixer::reset()
{
	if (xr) {
		delete xr;
		xr = NULL;
	}
	int i;
	for (i = 0; i < fixers.size(); i++) {
		fixers[i]->dec();
	}
	fixers.clear();
	if (fp) {
		fp->dec();
		fp = NULL;
	}
	dirty = false;
}

bool CPKFixer::load(bool rebuild)
{
	reset();

	fp = new FileRW(cpkpath, sizeof(hdr) + sizeof(tbl));
	fp->inc();
	if (fp->realsize() < sizeof(hdr) + sizeof(tbl)) return false;

	if (!rebuild) {
		if (!fp->read(&hdr, 0, sizeof(hdr))) return false;
	} else {
		dirty = true;
		memset(&hdr, 0, sizeof(hdr));
	}
    CHECK_ASSIGN(hdr.dwLable, 0x1a545352);
    CHECK_ASSIGN(hdr.dwVersion, 0x00000001);
    CHECK_ASSIGN(hdr.dwTableStart, 0x00000080);
    CHECK_ASSIGN(hdr.dwDataStart, 0x000e0080);
    CHECK_ASSIGN(hdr.dwMaxFileNum, 0x00008000);
    CHECK_ASSIGN(hdr.dwIsFormatted, 0x00000000);
    CHECK_ASSIGN(hdr.dwSizeOfHeader, 0x00000080);
    CHECK_ASSIGN(hdr.dwMaxTableNum, 0x00008000);
	memset(hdr.dwReserved, 0, sizeof(hdr.dwReserved));

	if (!rebuild) {
		if (!fp->read(tbl, sizeof(hdr), sizeof(tbl)) || !check_tbl()) return false;
	} else {
		rebuild_tbl();
	}
	if (hdr.dwValidTableNum < hdr.dwMaxTableNum) {
		memset(tbl + hdr.dwValidTableNum, 0, sizeof(tbl) - hdr.dwValidTableNum * sizeof(tagCPKTable));
	}

	fp->resize(hdr.dwPackageSize);
	return true;
}

CPKExtraFixer::CPKExtraFixer(ReadWriter *io, DWORD dwStartPos, DWORD dwPackedSize, DWORD dwExtraInfoSize, const char *filename)
{
	fp = io;
	fp->inc();
	base = dwStartPos;
	sz = dwPackedSize;
	extra = dwExtraInfoSize;
	str = filename;
	state = 0;
}
CPKExtraFixer::~CPKExtraFixer()
{
	fp->dec();
}
bool CPKExtraFixer::check()
{
	if (!state) {
		char *lwr = chinese_strlwr(strdup(str));
		size_t bufsz = strlen(lwr) + 2;
		char *buf = (char *) Malloc(bufsz);
		state = fp->read(buf, base + sz, bufsz) && buf[bufsz - 1] == 0 && memcmp(chinese_strlwr(buf), lwr, bufsz - 1) == 0 ? 1 : -1;
		free(buf);
		free(lwr);
	}
	return state > 0;
}
bool CPKExtraFixer::flush()
{
	if (!state) check();
	if (state < 0) {
		char *buf = (char *) Malloc(extra);
		strncpy(buf, str, extra);
		state = fp->write(buf, base + sz, extra) ? 1 : -1;
		free(buf);
	}
	return state > 0;
}

CPKFileFixer::CPKFileFixer(ReadWriter *io, DWORD dwStartPos, DWORD dwPackedSize, DWORD dwExtraInfoSize, const char *filename) : CPKExtraFixer(io, dwStartPos, dwPackedSize, dwExtraInfoSize, filename)
{
}
bool CPKFileFixer::read(void *buffer, unsigned offset, size_t length)
{
	assert(offset + length <= sz);
	bool ret = fp->read(buffer, base + offset, length);
	if (offset + length == sz) check();
	return ret;
}
bool CPKFileFixer::write(const void *buffer, unsigned offset, size_t length)
{
	assert(offset + length <= sz);
	bool ret = fp->write(buffer, base + offset, length);
	if (offset + length == sz) flush();
	return ret;
}
unsigned CPKFileFixer::size()
{
	return sz;
}

CPKSpecialFixer::CPKSpecialFixer(ReadWriter *io, DWORD dwStartPos, DWORD dwPackedSize, DWORD dwExtraInfoSize, const void *data) : CPKExtraFixer(io, dwStartPos, dwPackedSize, dwExtraInfoSize, (const char *) PTRADD(data, dwPackedSize))
{
	raw = data;
	state = 0;
}
bool CPKSpecialFixer::check()
{
	if (!state) {
		void *buf = Malloc(sz);
		state = fp->read(buf, base, sz) && memcmp(buf, raw, sz) == 0 ? 1 : -1;
		free(buf);
	}
	return state > 0 && CPKExtraFixer::check();
}
bool CPKSpecialFixer::flush()
{
	if (!state) check();
	if (state < 0) {
		state = fp->write(raw, base, sz) ? 1 : -1;
	}
	return state > 0 && CPKExtraFixer::flush();
}

int CPKFixer::check(ProgressObject *progress)
{
	int i;
	std::map<DWORD, int> rank;
	for (i = 0; i < hdr.dwValidTableNum; i++) {
		if (tbl[i].valid()) rank[tbl[i].dwCRC] = i;
	}
	int num_files = files.size();
	int num_special = special_files.size();
	fixers.reserve(num_files + num_special);
	CacheRW *cache = new CacheRW(fp);
	ConcatRW *cat = new ConcatRW();
	for (i = 0; i < num_files; i++) {
		tagCPKTable &cur = tbl[rank[files[i].dwCRC]];
		CPKFileFixer *ff = new CPKFileFixer(cache, cur.dwStartPos, cur.dwPackedSize, cur.dwExtraInfoSize, names[i]);
		ff->inc();
		fixers.push_back(ff);
		cat->append(ff);
	}
	for (i = 0; i < num_special; i++) {
		std::pair<tagCPKTable, const void *> &item = special_files[i];
		tagCPKTable &cur = tbl[rank[item.first.dwCRC]];
		CPKSpecialFixer *sf = new CPKSpecialFixer(cache, cur.dwStartPos, cur.dwPackedSize, cur.dwExtraInfoSize, item.second);
		sf->inc();
		fixers.push_back(sf);
	}
	xr = new XorRepair(cat, checksum, xorsum, blksize);
	int xr_state = xr->repair(progress);
	if (xr_state == 0) {
		delete xr;
		xr = NULL;
		if (dirty) {
			return 1;
		}
		for (i = 0; i < fixers.size(); i++) {
			if (!fixers[i]->check()) {
				return 1;
			}
		}
		return 0;
	} else {
		return xr_state;
	}
}

int CPKFixer::repair(ProgressObject *progress)
{
	int state;
	if ((load(false) && (state = check(progress)) >= 0) || (!progress->cancelled() && load(true) && (state = check(progress)) >= 0)) {
		return state;
	}
	return -1;
}

bool CPKFixer::commit()
{
	int i;
	if (dirty) {
		if (!fp->write(&hdr, 0, sizeof(hdr))) return false;
		if (!fp->write(tbl, sizeof(hdr), sizeof(tbl))) return false;
	}
	if (xr) {
		if (!xr->commit()) return false;
	}
	for (i = 0; i < fixers.size(); i++) {
		if (!fixers[i]->flush()) {
			return false;
		}
	}
	return fp->commit();
}
