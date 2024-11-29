#ifndef PAL3PATCHCONFIG_TEMPCOMMAND
#define PAL3PATCHCONFIG_TEMPCOMMAND

extern bool InvokeTempCommand(CWnd *fawnd, bool (*writer)(FILE *), bool (*reader)(FILE *));

#endif
