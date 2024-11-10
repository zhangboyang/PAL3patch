#include "common.h"

static int gameversion_check_disabled = 0;
void disable_gameversion_check(void)
{
    gameversion_check_disabled = 1;
}

struct gamever_item {
    const char *cpkpath;
    unsigned keycrc;
    const char *hash;
    unsigned mask;
};

static struct gamever_item gamever_CHS[] = {
    { "basedata\\basedata.cpk", 0xb27328d6, "509b64527a50d492f783d346530c8a6f84f40763", 0x1e }, // cbdata\PAL3_Softstar.gdb
    { NULL                    , 0         , "5a6a9d788e4b4f353e34bcd30b636d47e9c37f33", 0x01 },
    { NULL                    , 0x31d9c350, "1577f9d696c960851e4f33c384b8138617cc0dd0", 0x1e }, // ui\GameMainUI\Sword\Data\J131Dst.tga
    { NULL                    , 0         , "9946268fb2873b868a1156fcb6a9f6ea02860230", 0x01 },
    { "scene\\M16.cpk"        , 0x0ff666d2, "81d28ed7304dda9f277f10f4fc4a7b5286f1a554", 0x18 }, // m16.sce
    { NULL                    , 0         , "3abd3a8db8c44eff355818abf6832d5a49c92998", 0x07 },
    { "scene\\Q01.cpk"        , 0xd785e95f, "dfb7ae2dee25411a0bc2df47428cad0271d7bd18", 0x18 }, // Q01.sce
    { NULL                    , 0         , "0b03e8e51cbded366dd2a05499d8d763ceb91c8c", 0x06 },
    { NULL                    , 0         , "7d5f34a7b3453f7a5d95af70684013403da3b4aa", 0x01 },
    { NULL                    , 0x5938d37a, "25a8c2f5e6a7f24fad68074d1d427c6995597a20", 0x1e }, // X.scn
    { NULL                    , 0         , "c11c1ac593d475db276f9427adbbf6aa9ab7f970", 0x01 },
    { "scene\\Q02.cpk"        , 0xc84339e7, "a81b49e8238d868d2bec802efbf6792c41039a06", 0x1c }, // N03.scn
    { NULL                    , 0         , "b79c31aac36b699da7d33adc551602eb0f1259dd", 0x03 },
    { "scene\\Q08.cpk"        , 0xf54c1950, "9832a5f8d93c54934a7578587bc88df348bfff14", 0x18 }, // Q08.sce
    { NULL                    , 0         , "7936c54d583915841636bac08d7e2938d3f26f61", 0x06 },
    { NULL                    , 0         , "3ccf73e8e5fed4d394144bea99999108ba45ab8c", 0x01 },
    { NULL                    , 0x7a9a8b4a, "dc308c0fa2fc75f766891d706aed7c48aa6d05c8", 0x18 }, // Q08P.scn
    { NULL                    , 0         , "d8cda4f736c373603727adc81794f746787da0b1", 0x07 },
    { NULL                    , 0x893e8986, "60851ff7017a15e834fed1a583a1b71937130580", 0x18 }, // object.asm
    { NULL                    , 0         , "bdbd54cde437c13d3d42ff7ef90105d0f1fdd909", 0x07 },
    { "scene\\Q10.cpk"        , 0x015d3534, "e0745d8d5a4eade5f5b7ad3e1fb1ffe13b7ed4d2", 0x18 }, // Q10.sce
    { NULL                    , 0         , "8df2948df929834d3593bad03f4b321da6da31ec", 0x07 },
    { NULL                    , 0x015d353f, "1405beaebe422d158da4e6f7fd58e8e405a234a4", 0x18 }, // Q10.scn
    { NULL                    , 0         , "41b70cc81063bb946219809cfdc4c3d3a818f7e3", 0x04 },
    { NULL                    , 0         , "77199d9761f4b05cdbd55a2422b5f094a7a77b7f", 0x03 },
    { NULL                    , 0x893e8986, "56c4da520cb8fbddd3025e47ac01de3d2d76c99a", 0x18 }, // object.asm
    { NULL                    , 0         , "a5f08174298a11af13df0a63eb765a98eb35a989", 0x07 },
    { NULL                    , 0         , NULL                                      , 0    }  // EOF
};

static struct gamever_item gamever_CHT[] = {
    { "basedata\\basedata.cpk", 0xb27328d6, "66f1143b62c6fc254d1e065567ae5194d79ebb21", 0x1e }, // cbdata\PAL3_Softstar.gdb
    { NULL                    , 0         , "2f42d67b478d39581d8ce080c5ba45b16d6fa138", 0x01 },
    { NULL                    , 0xfe15f0ff, "b3c4321596606b8bdf995d4bd63956fe52ceefd6", 0x1e }, // gbdata\unicode20.tft
    { NULL                    , 0         , "bf27c8d10fa992211be3151936754c139245545c", 0x01 },
    { NULL                    , 0x31d9c350, "1577f9d696c960851e4f33c384b8138617cc0dd0", 0x1e }, // ui\GameMainUI\Sword\Data\J131Dst.tga
    { NULL                    , 0         , "9946268fb2873b868a1156fcb6a9f6ea02860230", 0x01 },
    { "scene\\M16.cpk"        , 0x0ff666d2, "729ab124b979e00a68e7e3d7f39cb5f513b1c5f9", 0x18 }, // m16.sce
    { NULL                    , 0         , "34860f654a9b0829ee1d6dd028583809fd0f3924", 0x07 },
    { NULL                    , 0x893e8986, "d145b04c1bb194f11135749c59ec611424e477f1", 0x18 }, // object.asm
    { NULL                    , 0         , "c0b5a2b3d25c45e566c2eb189476244e39880632", 0x07 },
    { "scene\\Q01.cpk"        , 0xd785e95f, "6b8d64f23e9dc5a6b98b77b3ee2b8e31ded3b92a", 0x1e }, // Q01.sce
    { NULL                    , 0         , "0a920be42d4f2bafeedf38b827c608efae0243ea", 0x01 },
    { NULL                    , 0x5938d37a, "25a8c2f5e6a7f24fad68074d1d427c6995597a20", 0x1e }, // X.scn
    { NULL                    , 0         , "c31788703aabb72ecf019f8b4e18adc148b0b2cd", 0x01 },
    { "scene\\Q02.cpk"        , 0xc84339e7, "a81b49e8238d868d2bec802efbf6792c41039a06", 0x1c }, // N03.scn
    { NULL                    , 0         , "161a48da7150380051c434e5e98b36b7f9aac712", 0x03 },
    { "scene\\Q08.cpk"        , 0xf54c1950, "b7d009f74fe664ac3b4249bd3fed6a8d10aac14b", 0x18 }, // Q08.sce
    { NULL                    , 0         , "8f9aec1c72205b638bf90da5c3f98bf0c97cc6d8", 0x06 },
    { NULL                    , 0         , "f13deba85133533d8c4259c731f72206d34b44fc", 0x01 },
    { NULL                    , 0x7a9a8b4a, "dc308c0fa2fc75f766891d706aed7c48aa6d05c8", 0x18 }, // Q08P.scn
    { NULL                    , 0         , "9a681f0b503504bf946a1f3365e36564b60f40f9", 0x07 },
    { NULL                    , 0x893e8986, "35805b4f3dd0d31a2f5b85e073c545dbcd2352d0", 0x18 }, // object.asm
    { NULL                    , 0         , "bcc5edd04acce66d70227f1168d24eed039cc86b", 0x07 },
    { "scene\\Q10.cpk"        , 0x015d3534, "efcf442b0179ff54b46a18049c9c80340c3f5cc0", 0x18 }, // Q10.sce
    { NULL                    , 0         , "17107d13d44395fe31b49be0382ebdb24459e923", 0x07 },
    { NULL                    , 0x893e8986, "08b6fc36fe8bb06aa09f1e02bb38a41729a4320e", 0x18 }, // object.asm
    { NULL                    , 0         , "37bd852fe6c4f9857f6e211ac493fb48103a5f58", 0x07 },
    { NULL                    , 0         , NULL                                      , 0    }  // EOF
};

void check_gameversion(void)
{
    if (gameversion_check_disabled) return;
    if (!get_int_from_configfile("checkgameversion")) return;
    
    struct gamever_item *gamever;
    
    switch (game_locale) {
        case GAME_LOCALE_CHS: gamever = gamever_CHS; break;
        case GAME_LOCALE_CHT: gamever = gamever_CHT; break;
        default: return;
    }

    unsigned mask = -1;
    char buf[41];
    const char *hash = NULL;
    struct rawcpk *rcpk = NULL;
    while (gamever->mask && mask) {
        if (gamever->cpkpath) {
            if (rcpk) rawcpk_close(rcpk);
            rcpk = rawcpk_open(gamever->cpkpath);
        }
        if (gamever->keycrc) {
            hash = rcpk ? rawcpk_hash(rcpk, gamever->keycrc, buf) : NULL;
        }
        if (!hash) {
            mask = 0;
        } else {
            int found = 0;
            do {
                if (!found && strcmp(gamever->hash, hash) == 0) {
                    mask &= gamever->mask;
                    found = 1;
                }
                gamever++;
            } while (gamever->mask && !gamever->keycrc);
            if (!found) {
                mask = 0;
            }
        }
    }
    if (rcpk) rawcpk_close(rcpk);
    
    int i, ver = -1;
    for (i = 0; i <= 4; i++) {
        if ((mask & (1 << i))) ver = i;
    }
    
    if (ver != 4) {
        if (MessageBoxW(NULL, wstr_badgamever_text, wstr_badgamever_title, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND) != IDYES) {
            die(0);
		}
    }
}
