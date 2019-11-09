#ifndef PAL3PATCHCONFIG_FONTENUM
#define PAL3PATCHCONFIG_FONTENUM

extern class EnumFontface : public ConfigDescOptionListEnum {
public:
	std::vector<CString> buf;

	virtual void EnumConfigValues(std::vector<CString> &result);
	virtual bool TranslateSelectedValue(CWnd *listWnd, CString &value);
	virtual bool IsValueEqual(const CString &lstval, const CString &selval);
	virtual CString GetValueTitle(const CString &value);
	virtual CString GetValueDescription(const CString &value);
} EnumFontfaceInstance;

#endif
