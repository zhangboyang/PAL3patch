#include "common.h"

static void try_register_install_dir_for_PAL3A()
{
    char installdir[MAXLINE];
    DWORD r = GetCurrentDirectory(sizeof(installdir), installdir);
    if (r == 0 || r >= MAXLINE) return;
    
    const char *gametype;
    switch (game_locale) {
        case GAME_LOCALE_CHS: gametype = "CH"; break;
        case GAME_LOCALE_CHT: gametype = "TW"; break;
        default: return;
    }
    
    char subkey[MAXLINE];
    snprintf(subkey, sizeof(subkey), "SOFTWARE\\SOFTSTAR\\PAL3\\%s", gametype);
    
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, "Install", 0, REG_SZ, (CONST BYTE *) installdir, strlen(installdir) + 1);
        RegCloseKey(hKey);
    }
}

MAKE_PATCHSET(reginstalldir)
{
    try_register_install_dir_for_PAL3A();
}
