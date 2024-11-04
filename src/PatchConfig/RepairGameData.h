#ifndef PAL3PATCHCONFIG_REPAIRGAMEDATA
#define PAL3PATCHCONFIG_REPAIRGAMEDATA

class RepairCommitter {
public:
	virtual ~RepairCommitter();
	virtual int repair(ProgressObject *progress) = 0;
	virtual bool commit() = 0;
};

extern void RepairGameData(CPatchConfigDlg *dlg);

#endif
