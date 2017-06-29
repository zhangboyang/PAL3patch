#ifndef PAL3PATCHCONFIG_CONFIGDESCDATA
#define PAL3PATCHCONFIG_CONFIGDESCDATA


#define MAX_CONFIGDESC_OPTIONS 3

struct ConfigDescOptionItem {
	LPCTSTR title;
	LPCTSTR description;
	LPCTSTR value;
};

struct ConfigDescItem {
	signed char level; // level -1 means end of list
	bool is_adv;
	LPCTSTR key;
	LPCTSTR title;
	LPCTSTR description;
	LPCTSTR defoptdesc; // can be NULL

	// if item type is "radio", here is the list
	// list ends with { NULL }, or MAX_CONFIGDESC_OPTIONS
	// otherwise, item type is "text", optlist must be { { NULL } }
	ConfigDescOptionItem optlist[MAX_CONFIGDESC_OPTIONS];

	void (*enumfunc)(std::vector<std::pair<CString, std::pair<CString, CString> > > &result);


	// run-time data
	CString *pvalue;
};

extern ConfigDescItem *ConfigDescList;

#endif
