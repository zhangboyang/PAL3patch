#include "stdafx.h"

BufferReader::BufferReader()
{
}

BufferReader::BufferReader(const void *buf) : ptr(buf)
{
}

const void *BufferReader::adv(size_t n)
{
	const void *ret = ptr;
	ptr = PTRADD(ptr, n);
	return ret;
}

void *BufferReader::cpy(void *dst, size_t n)
{
	return memcpy(dst, adv(n), n);
}

unsigned BufferReader::u32()
{
	unsigned ret;
	memcpy(&ret, adv(sizeof(ret)), sizeof(ret));
	return ret;
}

unsigned short BufferReader::u16()
{
	unsigned short ret;
	memcpy(&ret, adv(sizeof(ret)), sizeof(ret));
	return ret;
}

const char *BufferReader::str()
{
	return (const char *) adv(strlen((const char *) ptr) + 1);
}
