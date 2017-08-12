#ifndef PAL3PATCHCONFIG_CONFIGDESCDATA
#define PAL3PATCHCONFIG_CONFIGDESCDATA



#define MAX_CONFIGDESC_OPTIONS 3

struct ConfigDescOptionItem {
	LPCTSTR title;
	LPCTSTR description;
	LPCTSTR value;
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


	// if this is a "option item"
	//    if item type is "radio", here is the list
	//       list ends with { NULL }, or MAX_CONFIGDESC_OPTIONS
	//    otherwise, item type is "text", optlist must be { { NULL } }
	ConfigDescOptionItem optlist[MAX_CONFIGDESC_OPTIONS];

	// if this is a "option item"
	//    if item type is "choose from list", here is the enum function
	void (*enumfunc)(std::vector<std::pair<CString, std::pair<CString, CString> > > &result);


	// run-time data
	CString *pvalue; // current config value
	bool cur_expand; // lazy, only updated when tree reload
};

extern ConfigDescItem *ConfigDescList;

#endif
