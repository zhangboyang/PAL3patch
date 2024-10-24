#include "stdafx.h"

ReadWriter::ReadWriter()
{
	ref = 0;
}
ReadWriter::~ReadWriter()
{
	assert(!ref);
}
ReadWriter *ReadWriter::inc()
{
	ref++;
	return this;
}
void ReadWriter::dec()
{
	assert(ref);
	if (!--ref) delete this;
}

HANDLE FileRW::handle = INVALID_HANDLE_VALUE;
FileRW *FileRW::owner = NULL;
FileRW::FileRW(const std::string &filepath, unsigned filesize)
{
	path = filepath;
	sz = filesize;
	rw = false;
}
FileRW::~FileRW()
{
	close();
}
bool FileRW::open()
{
	if (owner != this) {
		if (handle != INVALID_HANDLE_VALUE) {
			CloseHandle(handle);
		}
		owner = this;
		if (rw) {
			handle = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		} else {
			handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		}
	}
	return handle != INVALID_HANDLE_VALUE;
}
void FileRW::close()
{
	if (owner == this) {
		if (handle != INVALID_HANDLE_VALUE) {
			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
		}
		owner = NULL;
	}
}
bool FileRW::seek(unsigned offset)
{
	if (!open()) return false;
	LONG lDistLow = offset, lDistHigh = 0;
	DWORD dwPtrLow = SetFilePointer(handle, lDistLow, &lDistHigh, FILE_BEGIN);
	return !(dwPtrLow == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR);
}
bool FileRW::reopen(bool readwrite)
{
	close();
	rw = readwrite;
	return open();
}
void FileRW::resize(unsigned filesize)
{
	sz = filesize;
}
unsigned FileRW::realsize()
{
	if (!open()) return 0;
	DWORD dwSizeLow, dwSizeHigh;
	dwSizeLow = GetFileSize(handle, &dwSizeHigh);
	if (dwSizeLow == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) return 0;
	return !dwSizeHigh ? dwSizeLow : -1;
}
bool FileRW::truncate()
{
	return seek(sz) && SetEndOfFile(handle);
}
bool FileRW::read(void *buffer, unsigned offset, size_t length)
{
	assert(offset + length <= sz);
	DWORD count = 0;
	bool result = false;
	if (seek(offset) && (!length || (ReadFile(handle, buffer, length, &count, NULL) || (count = 0)))) {
		result = count == length;
	}
	if (count < length) memset(PTRADD(buffer, count), 0, length - count);
	return result;
}
bool FileRW::write(const void *buffer, unsigned offset, size_t length)
{
	assert(offset + length <= sz);
	DWORD count;
	return rw && seek(offset) && (!length || (WriteFile(handle, buffer, length, &count, NULL) && count == length));
}
unsigned FileRW::size()
{
	return sz;
}

RangeRW::RangeRW(ReadWriter *io, unsigned rangebase, unsigned rangesize)
{
	fp = io->inc();
	base = rangebase;
	sz = rangesize;
}
RangeRW::~RangeRW()
{
	fp->dec();
}
bool RangeRW::read(void *buffer, unsigned offset, size_t length)
{
	assert(offset + length <= sz);
	return fp->read(buffer, base + offset, length);
}
bool RangeRW::write(const void *buffer, unsigned offset, size_t length)
{
	assert(offset + length <= sz);
	return fp->write(buffer, base + offset, length);
}
unsigned RangeRW::size()
{
	return sz;
}

DummyRW::DummyRW(unsigned fakesize)
{
	sz = fakesize;
}
bool DummyRW::read(void *buffer, unsigned offset, size_t length)
{
	assert(offset + length <= sz);
	if (length) memset(buffer, 0, length);
	return true;
}
bool DummyRW::write(const void *buffer, unsigned offset, size_t length)
{
	assert(offset + length <= sz);
	return true;
}
unsigned DummyRW::size()
{
	return sz;
}

ConcatRW::ConcatRW()
{
	tbl.push_back(0);
}
ConcatRW::~ConcatRW()
{
	size_t i;
	for (i = 0; i < fp.size(); i++) {
		fp[i]->dec();
	}
}
void ConcatRW::append(ReadWriter *io)
{
	fp.push_back(io->inc());
	tbl.push_back(tbl.back() + fp.back()->size());
}
bool ConcatRW::readwrite(void *rbuffer, const void *wbuffer, unsigned offset, size_t length)
{
	assert(offset + length <= tbl.back());
	size_t i = std::upper_bound(tbl.begin(), tbl.end() - 1, offset) - 1 - tbl.begin();
	offset -= tbl[i];
	bool result = true;
	while (length) {
		size_t count = (tbl[i + 1] - tbl[i]) - offset;
		count = length < count ? length : count;
		if (rbuffer) {
			result = fp[i]->read(rbuffer, offset, count) && result;
			rbuffer = PTRADD(rbuffer, count);
		}
		if (wbuffer) {
			result = fp[i]->write(wbuffer, offset, count) && result;
			wbuffer = PTRADD(wbuffer, count);
		}
		length -= count;
		offset = 0;
		i++;
	}
	return result;
}
bool ConcatRW::read(void *buffer, unsigned offset, size_t length)
{
	return readwrite(buffer, NULL, offset, length);
}
bool ConcatRW::write(const void *buffer, unsigned offset, size_t length)
{
	return readwrite(NULL, buffer, offset, length);
}
unsigned ConcatRW::size()
{
	return tbl.back();
}

PaddingRW::PaddingRW(ReadWriter *io, unsigned alignment)
{
	fp = io->inc();
	realsz = fp->size();
	sz = realsz + (alignment - realsz % alignment) % alignment;
}
PaddingRW::~PaddingRW()
{
	fp->dec();
}
bool PaddingRW::read(void *buffer, unsigned offset, size_t length)
{
	assert(offset + length <= sz);
	size_t count = 0;
	bool result = true;
	if (offset < realsz) {
		count = length < realsz - offset ? length : realsz - offset;
		result = fp->read(buffer, offset, count);
	}
	if (count < length) memset(PTRADD(buffer, count), 0, length - count);
	return result;
}
bool PaddingRW::write(const void *buffer, unsigned offset, size_t length)
{
	assert(offset + length <= sz);
	size_t count;
	bool result = true;
	if (offset < realsz) {
		count = length < realsz - offset ? length : realsz - offset;
		result = fp->write(buffer, offset, count);
	}
	return result;
}
unsigned PaddingRW::size()
{
	return sz;
}
