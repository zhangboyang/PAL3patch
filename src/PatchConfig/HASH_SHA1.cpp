#include "stdafx.h"

int GetFileSHA1(const char *fn, char *buf)
{

#define BUFSIZE 4096
	int ret = 0;
	unsigned char databuf[BUFSIZE];
	int datalen;
	unsigned char hashbuf[SHA1_BYTE];
	DWORD hashlen = sizeof(hashbuf);
	FILE *fp = fopen(fn, "rb");
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;

	if (!fp) goto done;
	// use CryptAcquireContextA to make this program KernelEx (for Win9X) compatible
	if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		hProv = 0;
		goto done;
	}
	if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
		hHash = 0;
		goto done;
	}

	while ((datalen = fread(databuf, 1, sizeof(databuf), fp))) {
		if (!CryptHashData(hHash, databuf, datalen, 0)) {
			goto done;
		}
	}

	if (CryptGetHashParam(hHash, HP_HASHVAL, hashbuf, &hashlen, 0)) {
		int i;
		for (i = 0; i < SHA1_BYTE; i++) {
			sprintf(buf + i * 2, "%02x", (unsigned) hashbuf[i]);
		}
	} else {
		goto done;
	}

	ret = 1;
done:
	if (hHash) CryptDestroyHash(hHash);
	if (hProv) CryptReleaseContext(hProv, 0);
	if (fp) fclose(fp);
	return ret;
}

