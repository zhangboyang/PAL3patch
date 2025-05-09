#ifndef PAL3PATCHCONFIG_CONFIGDESCDATA
#define PAL3PATCHCONFIG_CONFIGDESCDATA



#define MAX_CONFIGDESC_OPTIONS 3

struct ConfigDescOptionItem {
	LPCTSTR title;
	LPCTSTR description;
	LPCTSTR value;
};

class ConfigDescOptionListEnum {
public:
	virtual void EnumConfigValues(std::vector<CString> &result) = 0;
	virtual bool TranslateSelectedValue(CWnd *listWnd, CString &value);
	virtual bool IsValueEqual(const CString &lstval, const CString &selval);
	virtual bool IsValueLegal(const std::vector<CString> &lst, const CString &selval);
	virtual CString GetValueTitle(const CString &value);
	virtual CString GetValueDescription(const CString &value);
	virtual CString GetFallbackValue();
};

class CPatchConfigDlg;

struct ConfigDescItem {
	signed char level;  // tree depth, level -1 means end of list
	bool is_adv;        // is advanced option
	bool def_expand;    // is default expanded

	LPCTSTR key; // if key == NULL, this is not a "option item"

	LPCTSTR title;
	LPCTSTR description;
	LPCTSTR defoptdesc; // can be NULL

	// if this is a "function item"
	//    here is the function will be invoked when button is clicked
	void (*runfunc)(CPatchConfigDlg *dlg);
	LPCTSTR btntext;

	// if this is a "option item"
	//    if item type is "radio", here is the list
	//       list ends with { NULL }, or MAX_CONFIGDESC_OPTIONS
	//    otherwise, item type is "text", optlist must be { { NULL } }
	ConfigDescOptionItem optlist[MAX_CONFIGDESC_OPTIONS];

	// if this is a "option item"
	//    if item type is "choose from list", here is the enum function
	ConfigDescOptionListEnum *enumobj;

	// called when value about to change
	//    return false to reject current change
	bool (*onchange)(CPatchConfigDlg *dlg, const CString &oldvalue, const CString &newvalue);

	// where to register current item
	ConfigDescItem **slot;

	// run-time data
	CString *pvalue; // current config value
	bool cur_expand; // lazy, only updated when tree reload
};

struct ConfigDescListIndex {
	LPCTSTR name;
	ConfigDescItem *list;
};

extern ConfigDescItem *ConfigDescList;
extern bool ConfigDirty;

void LoadConfigDescList(LPCTSTR name);

#endif
