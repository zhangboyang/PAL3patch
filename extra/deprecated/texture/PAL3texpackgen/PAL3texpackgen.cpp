#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <string>
#include <map>
#include <vector>
using namespace std;

#define _TOSTR(x) #x
#define TOSTR(x) _TOSTR(x)

#define MAXFILES 200000
#define MAXLINE 4096

static void fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    exit(1);
    va_end(ap);
}

int main(int argc, char *argv[])
{
    char buf[MAXLINE];
    char *texfn;
    char *zpkfn;
    
    if (argc != 3 && argc != 2) {
        printf("usage: \n"
               "       PAL3texpackgen TEXMAP_FROM_PAL3ZPKUNPACK\n"
               "                  ===>  Generate ZPK input list\n" 
               "       PAL3texpackgen TEXMAP_FROM_PAL3ZPKUNPACK ZPKLIST_FROM_ZPKPACKER\n"
               "                  ===>  Generate Texure Pack\n"
               "\n"
               "       output will be write to stdout\n");
        return 1;
    }
    
    texfn = argv[1];
    if (argc == 3) {
        zpkfn = argv[2];
    }

    
    FILE *texfp, *zpkfp = NULL;
    texfp = fopen(texfn, "r"); if (!texfp) fail("can't open '%s'", texfn);
    if (zpkfn) { zpkfp = fopen(zpkfn, "r"); if (!zpkfp) fail("can't open '%s'", zpkfn); }



    map<string, map<string, string> > texmap; // CPK => (PATH => HASH)
    map<string, string> zpklist;
    map<string, map<string, string> >::iterator cpkit;
    map<string, string>::iterator texit, zpkit;
    
    
    // read texmap
    char cpk[MAXLINE];
    char texpath[MAXLINE];
    char hash[MAXLINE];
    while (fscanf(texfp, " %" TOSTR(MAXLINE) "s", buf) == 1) {
        if (strcmp(buf, "CPK") == 0) {
            fscanf(texfp, " \"%" TOSTR(MAXLINE) "[^\"]\"", cpk);
            //printf("current cpk is %s\n", cpk);
        } else if (strcmp(buf, "BEGIN") == 0 || strcmp(buf, "END") == 0) {
            fscanf(texfp, "%*s");
        } else if (strcmp(buf, "TEXTURE") == 0) {
            fscanf(texfp, " \"%" TOSTR(MAXLINE) "[^\"]\"", texpath);
            fscanf(texfp, "%*s");
            fscanf(texfp, "%" TOSTR(MAXLINE) "s", hash);
            fscanf(texfp, "%*[^\n]");
            //printf("found file %s, sha1 %s\n", texpath, hash);
            if (!texmap[string(cpk)].insert(make_pair(string(texpath), string(hash))).second) {
                fail("duplicate file '%s'", texpath);
            }
        } else {
            fail("unknown token %s", buf);
        }
    }
    
    // if we are generating ZPK input list
    if (!zpkfn) {
        map<string, int> fileset;
        vector<string> commonfiles;
        vector<string>::iterator sit;
        
        // find duplicated files
        for (cpkit = texmap.begin(); cpkit != texmap.end(); cpkit++) {
            for (texit = cpkit->second.begin(); texit != cpkit->second.end(); texit++) {
                if (++fileset[texit->second] == 2) {
                    commonfiles.push_back(texit->second);
                }
            }
        }
        
        // write duplicated files first
        for (sit = commonfiles.begin(); sit != commonfiles.end(); sit++) {
            printf("SHARED SHA1 %s\n", sit->c_str());
        }
        
        // write other files
        for (cpkit = texmap.begin(); cpkit != texmap.end(); cpkit++) {
            for (texit = cpkit->second.begin(); texit != cpkit->second.end(); texit++) {
                if (fileset[texit->second] == 1) {
                    printf("CPK \"%s\" FILE \"%s\" SHA1 %s\n", cpkit->first.c_str(), texit->first.c_str(), texit->second.c_str());
                }
            }
        }
        goto done;
    }
    
    
    // if we are generating Generate Texure Pack file
    
    // read zpklist
    
    fscanf(zpkfp, "%*s%*s%*s%*s");
    char newhash[MAXLINE];
    char orgpath[MAXLINE];
    while (fscanf(zpkfp, "%s", buf) == 1) {
        if (strcmp(buf, "HASH") != 0) fail("unknown token %s", buf);
        fscanf(zpkfp, "%" TOSTR(MAXLINE) "s", newhash);
        fscanf(zpkfp, "%*s%*s%*s%*s%*s");
        fscanf(zpkfp, " %" TOSTR(MAXLINE) "[^\n]", orgpath);
        char *oldhash = orgpath;
        if (strrchr(oldhash, '/')) oldhash = strrchr(oldhash, '/') + 1;
        if (strrchr(oldhash, '\\')) oldhash = strrchr(oldhash, '\\') + 1;
        if (strchr(oldhash, '.')) *strchr(oldhash, '.') = 0;
        //printf("zpk sha1 %s => old sha1 %s\n", newhash, oldhash);
        zpklist.insert(make_pair(string(oldhash), string(newhash)));
    }
    
    // write output
    if (strrchr(zpkfn, '/')) zpkfn = strrchr(zpkfn, '/') + 1;
    if (strrchr(zpkfn, '\\')) zpkfn = strrchr(zpkfn, '\\') + 1;
    if (strstr(zpkfn, ".list")) *strstr(zpkfn, ".list") = 0;
    printf("ZPK \"%s\"\n", zpkfn);
    for (cpkit = texmap.begin(); cpkit != texmap.end(); cpkit++) {
        printf("BEGIN CPK \"%s\"\n", cpkit->first.c_str());
        for (texit = cpkit->second.begin(); texit != cpkit->second.end(); texit++) {
            zpkit = zpklist.find(texit->second);
            if (zpkit == zpklist.end()) {
                fail("can't find zpk item for cpk item '%s' (sha1 %s)", texit->first.c_str(), texit->second.c_str());
            }
            printf("  FILE \"%s\" SHA1 %s MAPTO SHA1 %s\n", texit->first.c_str(), texit->second.c_str(), zpkit->second.c_str());
        } 
        printf("END CPK\n");
    }

done:
    if (zpkfp) fclose(zpkfp);
    fclose(texfp);
    return 0;
}
