#include "stdafx.h"

#define FILERW_OPENRW_MAXTRY 10
#define FILERW_OPENRW_WAIT   100
HANDLE FileRW::handle = INVALID_HANDLE_VALUE;
FileRW *FileRW::owner = NULL;
FileRW::FileRW(const std::string &filepath, unsigned filesize)
{
	assert(filesize != 0);
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
			int i;
			for (i = 0; i < FILERW_OPENRW_MAXTRY; i++) {
				if (i) Sleep(FILERW_OPENRW_WAIT);
				reset_attrib(path.c_str());
				handle = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (handle != INVALID_HANDLE_VALUE) break;
			}
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
void FileRW::enablewrite()
{
	if (!rw) {
		close();
		rw = true;
	}
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
	enablewrite();
	return seek(sz) && SetEndOfFile(handle);
}
bool FileRW::sync()
{
	enablewrite();
	return open() && FlushFileBuffers(handle);
}
bool FileRW::commit()
{
	return (realsize() == size() || truncate()) && (!rw || sync());
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
	enablewrite();
	return seek(offset) && (!length || (WriteFile(handle, buffer, length, &count, NULL) && count == length));
}
unsigned FileRW::size()
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
	fp.push_back(io);
	fp.back()->inc();
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
	fp = io;
	fp->inc();
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

CompareRW::CompareRW(ReadWriter *io)
{
	fp = io;
	fp->inc();
}
CompareRW::~CompareRW()
{
	fp->dec();
}
bool CompareRW::read(void *buffer, unsigned offset, size_t length)
{
	return fp->read(buffer, offset, length);
}
bool CompareRW::write(const void *buffer, unsigned offset, size_t length)
{
	if (length == 0) return true;
	void *rbuffer = malloc(length);
	bool same = fp->read(rbuffer, offset, length) && memcmp(rbuffer, buffer, length) == 0;
	free(rbuffer);
	return same || fp->write(buffer, offset, length);
}
unsigned CompareRW::size()
{
	return fp->size();
}