#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// use brute-force to generate possible console passwords

#define MAXSTACK 100
//#define SIGMA "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJHKLMNOPQRSTUVWXYZ_"
#define SIGMA "0123456789"
#define TARGET 0xD5E4C8F8

static unsigned (*gbCrc32Compute)(const char *);
static void (*gbCrc32Init)(void);

static void test(const char *s)
{
	//printf("%08X %s\n", gbCrc32Compute(s), s);
	if (gbCrc32Compute(s) == TARGET) {
		printf("FOUND: %s\n", s);
	}
}

int main(int argc, char *argv[])
{
	HMODULE hGBENGINE = LoadLibrary("GBENGINE.DLL");
	assert(hGBENGINE);
	gbCrc32Compute = (void *) GetProcAddress(hGBENGINE, "?gbCrc32Compute@@YAIPBD@Z");
	assert(gbCrc32Compute);
	gbCrc32Init = (void *) GetProcAddress(hGBENGINE, "?gbCrc32Init@@YAXXZ");
	assert(gbCrc32Init);
	gbCrc32Init();
	
	const char *sigma = SIGMA;
	if (argc > 1) sigma = argv[1];
	int sz = strlen(sigma);
	int a[MAXSTACK];
	char s[MAXSTACK + 1];

	printf("INFO: maxlen=%d sigma=%s\n", MAXSTACK, sigma);
	printf("INFO: target=0x%08X\n", TARGET);

	int l, z, f;
	double sp = 1;
	for (l = 1; l <= MAXSTACK; l++) { 
		sp *= sz;
		printf("INFO: now searching: len=%-2d space=%e\n", l, sp);

		z = 0;
		f = -1;
		s[l] = 0;
		do if (++f >= sz || z >= l)	{
			f = a[--z];
		} else {
			s[z] = sigma[f];
			a[z++] = f;
			f = -1;
			if (z >= l) {
				test(s);
			}
		} while (z >= 0);
	}
	return 0;
}