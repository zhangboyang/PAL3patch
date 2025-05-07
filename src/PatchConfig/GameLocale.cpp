#include "stdafx.h"

int detect_game_locale()
{
	DWORD key_CRC = 0xCB283888; /* datascript\lang.txt */
	const char *CHS_magic = "\x41\x70\x70\x4E\x61\x6D\x65\x09\x09\x24\xCF\xC9\xBD\xA3\xC6\xE6\xCF\xC0\xB4\xAB\x33\x26";
	const char *CHT_magic = "\x41\x70\x70\x4E\x61\x6D\x65\x09\x09\x24\xA5\x50\xBC\x43\xA9\x5F\xAB\x4C\xB6\xC7\x33\x26";
	size_t CHS_magic_len = strlen(CHS_magic);
	size_t CHT_magic_len = strlen(CHT_magic);

	int result = -1;

	FILE *fp = NULL;
	tagCPKHeader hdr;
	tagCPKTable *tbl = NULL;

	tagCPKTable *item = NULL;
	void *data = NULL;
	unsigned sz;
	unsigned i;

	fp = fopen("basedata\\basedata.cpk", "rb");
	if (!fp) goto done;

	if (fread(&hdr, sizeof(hdr), 1, fp) != 1) goto done;
	if (hdr.dwValidTableNum > 0x8000) goto done;

	tbl = (tagCPKTable *) malloc(hdr.dwValidTableNum * sizeof(tagCPKTable));
	if (!tbl) goto done;
	if (fread(tbl, sizeof(tagCPKTable), hdr.dwValidTableNum, fp) != hdr.dwValidTableNum) goto done;

    for (i = 0; i < hdr.dwValidTableNum; i++) {
        if (tbl[i].dwCRC == key_CRC && (tbl[i].dwFlag & 0x1) && !(tbl[i].dwFlag & 0x10)) {
			item = &tbl[i];
            break;
        }
    }
    if (!item) goto done;
    
	if (item->dwStartPos >= 0x40000000) goto done;
	if (item->dwPackedSize >= 0x100000) goto done;
    sz = item->dwPackedSize;
    data = malloc(sz);
    if (!data) goto done;
    if (fseek(fp, item->dwStartPos, SEEK_SET) != 0) goto done;
    if (fread(data, 1, sz, fp) != sz) goto done;
    
    for (i = 0; i < sz; i++) {
        if (i + CHS_magic_len <= sz && memcmp(PTRADD(data, i), CHS_magic, CHS_magic_len) == 0) {
            result = 0;
            break;
        }
        if (i + CHT_magic_len <= sz && memcmp(PTRADD(data, i), CHT_magic, CHT_magic_len) == 0) {
            result = 1;
            break;
        }
    }
    
done:
    free(data);
    free(tbl);
    if (fp) fclose(fp);
    
    return result;
}
