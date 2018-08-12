#ifndef PAL3PATCHCONFIG_HASHSHA1
#define PAL3PATCHCONFIG_HASHSHA1


/* ================ sha1.h ================ */
/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
*/

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(uint32_t state[5], const unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, const unsigned char* data, uint32_t len);
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);



#define SHA1_BYTE 20
#define SHA1_STR_SIZE (SHA1_BYTE * 2 + 1)
extern int GetFileSHA1(const char *fn, char *buf);

#endif