#ifndef PAL3PATCHCONFIG_REPAIRGAMEDATA
#define PAL3PATCHCONFIG_REPAIRGAMEDATA

class RepairCommitter {
public:
	virtual bool commit() = 0;
};

extern void RepairGameData(CPatchConfigDlg *dlg);

#endif
