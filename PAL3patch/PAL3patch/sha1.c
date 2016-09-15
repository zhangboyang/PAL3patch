#include "common.h"

static HCRYPTPROV hProv;

void sha1_init()
{
    // use CryptAcquireContextA to make this program KernelEx (for Win9X) compatible
    if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		hProv = 0;
	}
}

void sha1_cleanup()
{
    if (hProv) {
        CryptReleaseContext(hProv, 0);
        hProv = 0;
    }
}

void sha1_hash_buffer(const void *databuf, int datalen, unsigned char *hashbuf)
{
	int ret = 0;
	DWORD hashlen = SHA1_BYTE;
	HCRYPTHASH hHash;

    if (!hProv) goto done;
    
	if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
		hHash = 0;
		goto done;
	}

	if (!CryptHashData(hHash, (void *) databuf, datalen, 0)) {
		goto done;
	}

	if (!CryptGetHashParam(hHash, HP_HASHVAL, hashbuf, &hashlen, 0)) {
		goto done;
	}

	ret = 1;
done:
	if (hHash) CryptDestroyHash(hHash);
	if (!ret) fail("sha1 hash failed.");
}

char *sha1_tostr(const void *sha1buf)
{
    const unsigned char *sha1byte = sha1buf;
    static char buf[SHA1_STR_SIZE];
    int i;
    for (i = 0; i < SHA1_BYTE; i++) {
        sprintf(buf + i * 2, "%02x", sha1byte[i]);
    }
    return buf;
}

void sha1_fromstr(void *sha1buf, const char *sha1str)
{
    unsigned char *sha1byte = sha1buf;
    int i;
    if (strlen(sha1str) != SHA1_BYTE * 2) goto err;
    char buf[3]; buf[2] = 0;
    for (i = 0; i < SHA1_BYTE; i++) {
        unsigned curbyte;
        strncpy(buf, sha1str + i * 2, 2);
        if (sscanf(buf, "%x", &curbyte) != 1) goto err;
        sha1byte[i] = curbyte;
    }
    return;
err:
    fail("invalid sha1 string '%s'.", sha1str);
}
