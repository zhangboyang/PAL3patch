#include "common.h"

static int gameversion_check_disabled = 0;
void disable_gameversion_check(void)
{
    gameversion_check_disabled = 1;
}

void check_gameversion(void)
{
    if (gameversion_check_disabled) return;
    if (!get_int_from_configfile("checkgameversion")) return;
    
    char buf[41];
    const char *hash = NULL;
	FILE *fp = fopen("basedata\\cbdata\\flexeff.data", "rb");
	if (fp) {
        int bad = 0;
        unsigned char databuf[4096];
        unsigned char digest[20];
        SHA1_CTX ctx;
    	SHA1Init(&ctx);
        while (!feof(fp)) {
            int datalen = fread(databuf, 1, sizeof(databuf), fp);
            if (ferror(fp)) { bad = 1; break; }
            if (datalen > 0) SHA1Update(&ctx, databuf, datalen);
        }
    	SHA1Final(digest, &ctx);
    	fclose(fp);
    	if (!bad) {
        	unsigned i;
        	for (i = 0; i < sizeof(digest); i++) {
        		sprintf(buf + i * 2, "%02x", (unsigned) digest[i]);
        	}
        	hash = buf;
        }
    }
    
    int ver = -1;
    if (hash) {
        if (strcmp("2a73da7d1f7609bab6bf40a2a35ac5673efb5bf6", hash) == 0) ver = 0;
        if (strcmp("d30057dd4d0cecf794f7e7622f8c98fb6dd658c8", hash) == 0) ver = 21;
    }
    
    if (ver != 21) {
        if (MessageBoxW(NULL, wstr_badgamever_text, wstr_badgamever_title, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND) != IDYES) {
            die(0);
		}
    }
}
