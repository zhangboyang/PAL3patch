#include "stdafx.h"

FileFixer::FileFixer(BufferReader &r)
{
	int i;
	int num_files = r.u32();
	fp.reserve(num_files);
	for (i = 0; i < num_files; i++) {
		const char *filepath = r.str();
		unsigned filesize = r.u32();
		fp.push_back(new FileRW(filepath, filesize));
		fp.back()->inc();
	}
}
FileFixer::~FileFixer()
{
	std::vector<FileRW *>::iterator it;
	for (it = fp.begin(); it != fp.end(); it++) {
		(*it)->dec();
	}
}

FileRawFixer::FileRawFixer(BufferReader &r) : FileFixer(r)
{
	int i;
	data.reserve(fp.size());
	for (i = 0; i < fp.size(); i++) {
		const void *filedata = r.adv(fp[i]->size());
		data.push_back(filedata);
	}
}
int FileRawFixer::repair(ProgressObject *progress)
{
	int i;
	unsigned sum = 0;
	for (i = 0; i < fp.size(); i++) {
		sum += fp[i]->size();
	}
	unsigned cur = 0;
	bool dirty = false;
	bad.reserve(fp.size());
	ProgressBinder<unsigned> pb(progress, &cur, sum, 1);
	for (i = 0; i < fp.size(); i++) {
		if (!pb.update()) return -1;
		unsigned sz = fp[i]->size();
		void *buf = Malloc(sz);
		bad.push_back(fp[i]->realsize() != sz || !fp[i]->read(buf, 0, sz) || memcmp(buf, data[i], sz) != 0);
		free(buf);
		dirty = dirty || bad.back();
		cur += sz;
	}
	return dirty ? 1 : 0;
}
bool FileRawFixer::commit()
{
	int i;
	for (i = 0; i < fp.size(); i++) {
		if (!bad[i]) continue;
		if (!fp[i]->write(data[i], 0, fp[i]->size()) || !fp[i]->commit()) return false;
	}
	return true;
}

FileXorFixer::FileXorFixer(BufferReader &r) : FileFixer(r)
{
	blksize = r.u32();
	xorsum = r.adv(blksize);
	r.cpy(&checksum, sizeof(checksum));
	xr = NULL;
}
FileXorFixer::~FileXorFixer()
{
	if (xr) delete xr;
}
int FileXorFixer::repair(ProgressObject *progress)
{
	ConcatRW *cat = new ConcatRW();
	std::vector<FileRW *>::iterator it;
	for (it = fp.begin(); it != fp.end(); it++) {
		cat->append(new CompareRW(*it));
	}
	xr = new XorRepair(cat, checksum, xorsum, blksize);
	return xr->repair(progress);
}
bool FileXorFixer::commit()
{
	if (!xr->commit()) return false;
	std::vector<FileRW *>::iterator it;
	for (it = fp.begin(); it != fp.end(); it++) {
		if (!(*it)->commit()) return false;
	}
	return true;
}
