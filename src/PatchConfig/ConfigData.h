#ifndef PAL3PATCHCONFIG_CONFIGDATA
#define PAL3PATCHCONFIG_CONFIGDATA

extern int TryRebuildConfigFile();
extern int TryLoadConfigData();
extern int FallbackConfigData(bool dry_run);
extern bool IsCdPatchEnabled();
extern int TrySaveConfigData();

#endif
