#include "stdafx.h"

// simple write-ahead logging for atomic file replacement

#define DATABUF_SIZE 4096

struct wal_checksum {
    unsigned char digest[20];
};
static int calc_file_checksum(FILE *fp, struct wal_checksum *csum)
{
	int success = 1;
	unsigned char databuf[DATABUF_SIZE];
	size_t datalen;
	SHA1_CTX ctx;

	SHA1Init(&ctx);
	while (!feof(fp)) {
        datalen = fread(databuf, 1, sizeof(databuf), fp);
        if (ferror(fp)) {
            success = 0;
            break;
        }
        if (datalen > 0) {
    		SHA1Update(&ctx, databuf, datalen);
        }
	}
	SHA1Final(csum->digest, &ctx);

    return success;
}

static int copy_file_contents(FILE *dstfp, FILE *srcfp)
{
    int success = 1;
    char databuf[DATABUF_SIZE];
    size_t datalen;
    
    while (!feof(srcfp)) {
        datalen = fread(databuf, 1, sizeof(databuf), srcfp);
        if (ferror(srcfp)) {
            success = 0;
            break;
        }
        if (datalen > 0) {
            if (fwrite(databuf, 1, datalen, dstfp) != datalen) {
                success = 0;
                break;
            }
        }
    }
    
    return success;
}

// internal: overwrite dst with src, then destory checksum
static int do_overwrite(const char *dst[], const char *src[], int n, const char *sum)
{
    int i;
    FILE *srcfp = NULL;
    FILE *dstfp = NULL;
    FILE *sumfp = NULL;
    
    for (i = 0; i < n; i++) {
        srcfp = robust_fopen(src[i], "rb");
        if (!srcfp) goto fail;
        
        dstfp = robust_fopen(dst[i], "wbc");
        if (!dstfp) goto fail;
        
        if (!copy_file_contents(dstfp, srcfp)) goto fail;
        
        if (fflush(dstfp) != 0) goto fail;
        safe_fclose(&dstfp);
        safe_fclose(&srcfp);
    }
    
    sumfp = robust_fopen(sum, "wbc");
    if (!sumfp) goto fail;
    if (fflush(sumfp) != 0) goto fail;
    safe_fclose(&sumfp);
    
    return 1;

fail:
    safe_fclose(&srcfp);
    safe_fclose(&dstfp);
    safe_fclose(&sumfp);
    return 0;
}

// internal: delete checksum and src
static int do_cleanup(const char *src[], int n, const char *sum)
{
    int success = 1;
    int i;
    
    if (robust_unlink(sum) != 0 && errno != ENOENT) success = 0;
    
    for (i = 0; i < n; i++) {
        if (robust_unlink(src[i]) != 0 && errno != ENOENT) success = 0;
    }
    
    return success;
}

// internal: commit changes
static int do_commit(const char *dst[], const char *src[], int n, const char *sum)
{
    return do_overwrite(dst, src, n, sum) && do_cleanup(src, n, sum);
}

// replace dst[] with src[], with temporary checksum(s) stored in sum
static int wal_replaceN(const char *dst[], const char *src[], int n, const char *sum)
{
    FILE *sumfp = NULL;
    FILE *srcfp = NULL;
    int i;
    
    // step 1: write checksum
    sumfp = robust_fopen(sum, "wbc");
    if (!sumfp) goto fail;
    
    for (i = 0; i < n; i++) {
        srcfp = robust_fopen(src[i], "r+bc");
        if (!srcfp) goto fail;
        
        struct wal_checksum csum;
        if (!calc_file_checksum(srcfp, &csum)) goto fail;
        
        if (fwrite(&csum, sizeof(csum), 1, sumfp) != 1) goto fail;
        
        if (fflush(srcfp) != 0) goto fail;
        safe_fclose(&srcfp);
    }
    
    if (fflush(sumfp) != 0) goto fail;
    safe_fclose(&sumfp);

    // step 2: commit changes
    return do_commit(dst, src, n, sum);
    
fail:
    safe_fclose(&srcfp);
    safe_fclose(&sumfp);
    return 0;
}

// check consistency, run undo or redo if necessary
static int wal_checkN(const char *dst[], const char *src[], int n, const char *sum)
{
    FILE *sumfp = NULL;
    FILE *srcfp = NULL;
    int i;

    // step 1: verify checksum
    sumfp = robust_fopen(sum, "rb");
    if (!sumfp) goto undo;

    for (i = 0; i < n; i++) {
        struct wal_checksum csum1, csum2;
        
        if (fread(&csum1, sizeof(csum1), 1, sumfp) != 1) goto undo;

        srcfp = robust_fopen(src[i], "rb");
        if (!srcfp) goto undo;
        
        if (!calc_file_checksum(srcfp, &csum2)) goto undo;

        safe_fclose(&srcfp);
        
        if (memcmp(&csum1, &csum2, sizeof(struct wal_checksum)) != 0) goto undo;
    }
    
    safe_fclose(&sumfp);
    
    // step 2a: redo commit
    return do_commit(dst, src, n, sum);

undo:
    // step 2b: rollback
    safe_fclose(&srcfp);
    safe_fclose(&sumfp);
    return do_cleanup(src, n, sum);
}

int wal_replace(char *dst[], char *src[], int n, const char *sum)
{
    return wal_replaceN((const char **) dst, (const char **) src, n, sum);
}

int wal_check(char *dst[], char *src[], int n, const char *sum)
{
    return wal_checkN((const char **) dst, (const char **) src, n, sum);
}

int wal_replace1(const char *dst, const char *src, const char *sum)
{
    return wal_replaceN(&dst, &src, 1, sum);
}

int wal_check1(const char *dst, const char *src, const char *sum)
{
    return wal_checkN(&dst, &src, 1, sum);
}
