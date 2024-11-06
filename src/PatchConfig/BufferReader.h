#ifndef PAL3PATCHCONFIG_BUFFERREADER
#define PAL3PATCHCONFIG_BUFFERREADER

class BufferReader {
private:
	const void *ptr;
public:
	BufferReader();
	BufferReader(const void *buf);
	const void *adv(size_t n);
	void *cpy(void *dst, size_t n);
	unsigned u32();
	unsigned short u16();
	const char *str();
};
#endif
