#ifndef PAL3PATCHCONFIG_D3DENUM
#define PAL3PATCHCONFIG_D3DENUM

extern ConfigDescItem *p3DAPIConfigItem;

extern void FirstD3DEnumeration();
extern bool On3DAPIConfigChange(CPatchConfigDlg *dlg, const CString &oldvalue, const CString &newvalue);

extern class EnumDisplayMode : public ConfigDescOptionListEnum {
public:
	virtual void EnumConfigValues(std::vector<CString> &result);
	virtual CString GetValueTitle(const CString &value);
	virtual CString GetValueDescription(const CString &value);
} EnumDisplayModeInstance;
extern class EnumDepthStencil : public ConfigDescOptionListEnum {
public:
	virtual void EnumConfigValues(std::vector<CString> &result);
	virtual CString GetValueTitle(const CString &value);
	virtual CString GetValueDescription(const CString &value);
	virtual CString GetFallbackValue();
} EnumDepthStencilInstance;
extern class EnumMultisample : public ConfigDescOptionListEnum {
public:
	virtual void EnumConfigValues(std::vector<CString> &result);
	virtual CString GetValueTitle(const CString &value);
	virtual CString GetValueDescription(const CString &value);
	virtual CString GetFallbackValue();
} EnumMultisampleInstance;

#endif
