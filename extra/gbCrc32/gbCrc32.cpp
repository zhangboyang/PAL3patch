#include <windows.h>
#include <cassert>
#include <cstdio>

// gbCrc32 definition
class gbCrc32 {
public:
	static unsigned CrcTbl[0x100];
	static void gbCrc32Init();
	static unsigned gbCrc32Compute(const char *str, int size);
	static unsigned gbCrc32Compute(const char *str);
};


// gbCrc32 implementation
unsigned gbCrc32::CrcTbl[0x100];
void gbCrc32::gbCrc32Init()
{
	unsigned v0 = 0;
	unsigned *v1 = CrcTbl;
	unsigned v2;
	unsigned v3;
	do {
		v2 = 8;
		v3 = v0 << 24;
		do {
			if (v3 & 0x80000000) {
				v3 = 2 * v3 ^ 0x04C11DB7;
			} else {
				v3 *= 2;
			}
			--v2;
		} while (v2);
		*v1 = v3;
		++v1;
		++v0;
	} while (v1 < CrcTbl + 0x100);
}
unsigned gbCrc32::gbCrc32Compute(const char *str, int size)
{
	unsigned char *s = (unsigned char *) str;
	unsigned x = 0;
	int i;
	for (i = 0; i < 4; i++) {
		x = (x << 8);
		if (i < size) x |= *s++;
	}
	x = ~x;
	size -= 4;
	while (size-- > 0) {
		x = CrcTbl[x >> 24] ^ (*s++ | (x << 8));
	}
	return ~x;
}
unsigned gbCrc32::gbCrc32Compute(const char *str)
{
	unsigned char *s = (unsigned char *) str;
	unsigned x = 0;
	int i;
	for (i = 0; i < 4; i++) {
		x = (x << 8);
		if (*s) x |= *s++;
	}
	x = ~x;
	while (*s) {
		x = CrcTbl[x >> 24] ^ (*s++ | (x << 8));
	}
	return ~x;
}



// run self check
int main()
{
	// import functions
	unsigned (*gbCrc32ComputeString)(const char *);
	unsigned (*gbCrc32ComputeBlock)(const char *, int);
	void (*gbCrc32Init)(void);
	HMODULE hGBENGINE = LoadLibrary("GBENGINE.DLL");
	assert(hGBENGINE);
	gbCrc32Init = (decltype(gbCrc32Init)) GetProcAddress(hGBENGINE, "?gbCrc32Init@@YAXXZ");
	assert(gbCrc32Init);
	gbCrc32ComputeString = (decltype(gbCrc32ComputeString)) GetProcAddress(hGBENGINE, "?gbCrc32Compute@@YAIPBD@Z");
	assert(gbCrc32ComputeString);
	gbCrc32ComputeBlock = (decltype(gbCrc32ComputeBlock)) GetProcAddress(hGBENGINE, "?gbCrc32Compute@@YAIPBDH@Z");
	assert(gbCrc32ComputeBlock);

	// init crc tables
	gbCrc32::gbCrc32Init();
	gbCrc32Init();

	// run tests
#define BUFSIZE 0x200
	for (int len = 0; len < BUFSIZE; len++) {
		int r;
		for (r = 0; r < BUFSIZE; r++) {
			char buf[BUFSIZE];
			for (int i = 0; i < BUFSIZE; i++) while (!(buf[i] = rand()));
			unsigned blkcrc = gbCrc32ComputeBlock(buf, len);
			unsigned myblkcrc = gbCrc32::gbCrc32Compute(buf, len);
			buf[len] = 0;
			unsigned strcrc = gbCrc32ComputeString(buf);
			unsigned mystrcrc = gbCrc32::gbCrc32Compute(buf);
			//printf("%05d: %08x %08x %08x %08x\n", len, blkcrc, myblkcrc, strcrc, mystrcrc);
			assert(blkcrc == myblkcrc);
			assert(strcrc == mystrcrc);
			assert(blkcrc == strcrc);
		}
		printf("length %d: %d tests passed.\n", len, r);
	}
	return 0;
}
