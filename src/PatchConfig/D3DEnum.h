#ifndef PAL3PATCHCONFIG_D3DENUM
#define PAL3PATCHCONFIG_D3DENUM

extern int CheckDX90SDKVersion();
extern void InitD3DEnumeration();
extern void CleanupD3DEnumeration();

extern class EnumDisplayMode : public ConfigDescOptionListEnum {
public:
	virtual void EnumConfigValues(std::vector<CString> &result);
	virtual CString GetValueTitle(const CString &value);
	virtual CString GetValueDescription(const CString &value);
} EnumDisplayModeInstance;
extern class EnumDepthBuffer : public ConfigDescOptionListEnum {
public:
	virtual void EnumConfigValues(std::vector<CString> &result);
	virtual CString GetValueTitle(const CString &value);
	virtual CString GetValueDescription(const CString &value);
} EnumDepthBufferInstance;
extern class EnumMultisample : public ConfigDescOptionListEnum {
	std::map<CString, CString> descmap;
public:
	virtual void EnumConfigValues(std::vector<CString> &result);
	virtual CString GetValueTitle(const CString &value);
	virtual CString GetValueDescription(const CString &value);
} EnumMultisampleInstance;

#endif
