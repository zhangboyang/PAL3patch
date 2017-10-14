#include "common.h"

static float fovfactor;

MAKE_PATCHSET(fixfov)
{
    unsigned fovfactor_addr = TOUINT(&fovfactor);
    fovfactor = str2double(get_string_from_configfile("fovfactor"));
    if (game_width * 3 > game_height * 4) {
        fovfactor *= game_width * 0.75f / game_height;
    }
    
    int i;
    const unsigned patch1addr[] = { 0x1002149D };
    char patch1data[][17] = {
        "\xD9\xC0\xD9\xFE\xD8\x49\x10\xD9\xC9\xD9\xFF\xD9\xC9\xD9\xC9\xDE\xF9", "\xD9\xF2\xDD\xD8\xD8\x49\x10\xD8\x0D\x00\x00\x00\x00\x90\x90\x90\x90",
    };
    for (i = 0; i < 1; i++) {
        unsigned base = patch1addr[i] + gboffset;
        char *oldcode = patch1data[i * 2];
        char *newcode = patch1data[i * 2 + 1];
        memcpy(newcode + 9, &fovfactor_addr, 4);
        SIMPLE_PATCH(base, oldcode, newcode, 17);
    }
    const unsigned patch2addr[] = { 0x10021A53, 0x10021B1A };
    char patch2data[][14] = {
        "\xD9\xC0\xD9\xFE\xD9\xC9\xD9\xFF\xD9\xC9\xD9\xC9\xDE\xF9", "\xD9\xF2\xDD\xD8\xD8\x0D\x00\x00\x00\x00\x90\x90\x90\x90",
        "\xD9\xC0\xD9\xFE\xD9\xC9\xD9\xFF\xD9\xC9\xD9\xC9\xDE\xF9", "\xD9\xF2\xDD\xD8\xD8\x0D\x00\x00\x00\x00\x90\x90\x90\x90",
    };
    for (i = 0; i < 2; i++) {
        unsigned base = patch2addr[i] + gboffset;
        char *oldcode = patch2data[i * 2];
        char *newcode = patch2data[i * 2 + 1];
        memcpy(newcode + 6, &fovfactor_addr, 4);
        SIMPLE_PATCH(base, oldcode, newcode, 14);
    }
}
