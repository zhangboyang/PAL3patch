// PatchConfigDlg.cpp : implementation file
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CPatchConfigDlg::OnMouseMoveCallbackWarpper(void *ptr, int btnid)
{
	((CPatchConfigDlg *) ptr)->BtnMouseMove(btnid);
}

void CPatchConfigDlg::SetRadioBtnStyle(int btnid, int bold, int underline)
{
	CMouseMsgButton *rbtn[MAX_CONFIGDESC_OPTIONS] = {&m_RadioBtn1, &m_RadioBtn2, &m_RadioBtn3};
	int updateflag = 0;
	LOGFONT lf;
	rbtn[btnid]->GetFont()->GetLogFont(&lf);
	if (underline >= 0 && !!lf.lfUnderline != !!underline) {
		lf.lfUnderline = !!underline;
		updateflag = 1;
	}
	if (bold >= 0 && lf.lfWeight != (bold ? FW_BOLD : FW_DONTCARE)) {
		lf.lfWeight = (bold ? FW_BOLD : FW_DONTCARE);
		updateflag = 1;
	}
	if (updateflag) {
		m_RadioBtnFont[btnid].DeleteObject();
		m_RadioBtnFont[btnid].CreateFontIndirect(&lf);
		rbtn[btnid]->SetFont(&m_RadioBtnFont[btnid], TRUE);
	}
}
void CPatchConfigDlg::BtnMouseMove(int btnid)
{
	if (m_OptDescShowing != btnid) {
		m_OptDescShowing = btnid;

		LockWindowUpdate();

		int i;

		// update description
		ConfigDescItem *pItem = m_ItemSelected;
		int descid = m_OptDescShowing;
		if (descid == -1) {
			for (i = 0; i < MAX_CONFIGDESC_OPTIONS; i++) {
				ConfigDescOptionItem *pOpt = &pItem->optlist[i];
				if (!pOpt->title) break;
				if (*pItem->pvalue == CString(pOpt->value)) {
					descid = i;
				}
			}
		}
		if (descid == -1) {
			if (pItem->defoptdesc) {
				m_OptDesc = CString(pItem->defoptdesc);
			} else {
				m_OptDesc = STRTABLE(IDS_NODESC);
			}
		} else {
			m_OptDesc = CString(pItem->optlist[descid].description);
		}
		UpdateData(FALSE);

		// set button style
		for (i = 0; i < MAX_CONFIGDESC_OPTIONS; i++) {
			SetRadioBtnStyle(i, -1, (i == m_OptDescShowing));
		}

		UnlockWindowUpdate();
	}
}

void CPatchConfigDlg::ToggleAdvMode(bool reset)
{
	if (reset) m_IsAdvMode = 0;
	else m_IsAdvMode ^= 1;
	m_ToggleAdvBtn.SetWindowText(STRTABLE(m_IsAdvMode ? IDS_HIDEADVOPT : IDS_SHOWADVOPT));
	LoadConfigDescription(reset);
}

void CPatchConfigDlg::LoadConfigDescription(bool reset_expand)
{
	HTREEITEM cur;
	ConfigDescItem *pItem;
	m_ItemSelected = NULL;
	
	// record expand state if needed
	if (!reset_expand) {
		HTREEITEM curnode, nextnode;
		std::deque<HTREEITEM> q;
		nextnode = m_CfgTree.GetRootItem();
		if (nextnode) q.push_back(nextnode);
		while (!q.empty()) {
			for (curnode = q.front(), q.pop_front(); curnode; curnode = m_CfgTree.GetNextSiblingItem(curnode)) {
				nextnode = m_CfgTree.GetChildItem(curnode);
				if (nextnode) q.push_back(nextnode);
				// process curnode
				pItem = (ConfigDescItem *) m_CfgTree.GetItemData(curnode);
				pItem->cur_expand = !!(m_CfgTree.GetItemState(curnode, TVIS_EXPANDED) & TVIS_EXPANDED);
			}
		}
	}

	// load into tree
	LockWindowUpdate(); m_LockSelUpdate = 1;
	m_CfgTree.DeleteAllItems();
	std::vector<std::pair<HTREEITEM, ConfigDescItem *> > treestack;
	treestack.push_back(std::make_pair(TVI_ROOT, (ConfigDescItem *) NULL));
	HTREEITEM selnode = NULL, preferrednode = NULL;
	for (pItem = ConfigDescList; ; pItem++) {
		if (reset_expand) {
			pItem->cur_expand = pItem->def_expand;
		}
		if (!m_IsAdvMode && pItem->is_adv) continue;
		while (pItem->level + 1 < (int) treestack.size()) {
			m_CfgTree.Expand(treestack.back().first, (!treestack.back().second || treestack.back().second->cur_expand) ? TVE_EXPAND : TVE_COLLAPSE);
			treestack.pop_back();
		}
		if (pItem->level < 0) break;
		cur = m_CfgTree.InsertItem(TVIF_TEXT | TVIF_PARAM, pItem->title, 0, 0, 0, 0, (LPARAM) pItem, treestack.back().first, NULL);

		if (pItem == m_LastItemSelected[m_IsAdvMode]) {
			selnode = cur;
		}
		
		if (pItem == m_ModeSwitchPreferredItem) {
			preferrednode = cur;
		}

		if (pItem->level + 1 >= (int) treestack.size()) {
			treestack.push_back(std::make_pair(cur, pItem));
		}
	}

	m_ItemSelected = NULL; // force reload
	if (preferrednode) selnode = preferrednode;
	if (selnode == NULL) selnode = m_CfgTree.GetRootItem();
	m_CfgTree.EnsureVisible(selnode);
	m_CfgTree.SelectItem(selnode);

	m_LockSelUpdate = 0;
	pItem = (ConfigDescItem *) m_CfgTree.GetItemData(selnode);
	SelectConfigItem(pItem);
	m_ModeSwitchPreferredItem = NULL;

	GotoDlgCtrl(&m_OKBtn);
	GotoDlgCtrl(&m_CfgTree);
	UnlockWindowUpdate(); 
}

void CPatchConfigDlg::SelectConfigItem(ConfigDescItem *pItem)
{
	if (m_LockSelUpdate) return;

	int reload = (m_ItemSelected != pItem); // should we reload data
	int i;
	int descflag = 0; // option description updated flag
	
	m_ItemSelected = pItem;

	if (m_LastItemSelected[m_IsAdvMode] != pItem) {
		m_ModeSwitchPreferredItem = pItem;
	}
	m_LastItemSelected[m_IsAdvMode] = pItem;

	if (reload) {
		if (m_IsAdvMode && pItem->key) {
			m_CfgTitle.Format(_T("%s (%s)"), pItem->title, pItem->key);
		} else {
			m_CfgTitle = CString(pItem->title);
		}
	}
	if (reload) {
		m_CfgDesc = CString(pItem->description);
	}
	if (reload) {
		if (pItem->runfunc) {
			m_RunfuncBtn.SetWindowText(pItem->btntext ? pItem->btntext : pItem->title);
			m_RunfuncBtn.ShowWindow(TRUE);
		} else {
			m_RunfuncBtn.ShowWindow(FALSE);
		}
	}

	m_RadioVal = -1;
	int invflag = -1; // first invalid option's id
	CMouseMsgButton *rbtnlist[MAX_CONFIGDESC_OPTIONS] = {&m_RadioBtn1, &m_RadioBtn2, &m_RadioBtn3};
	for (i = 0; i < MAX_CONFIGDESC_OPTIONS; i++) {
		ConfigDescOptionItem *pOpt = &pItem->optlist[i];
		if (!pOpt->title && invflag < 0) {
			invflag = i;
		}
		CWnd *rbtn = rbtnlist[i];
		if (invflag < 0) {
			if (reload) rbtn->SetWindowText(pOpt->title);
			if (reload) rbtn->ShowWindow(SW_SHOW);
			if (*pItem->pvalue == CString(pOpt->value)) {
				m_RadioVal = i;
				SetRadioBtnStyle(i, 1, (reload ? 0 : -1));
				m_OptDesc = CString(pOpt->description);
				descflag = 1;
			} else {
				SetRadioBtnStyle(i, 0, (reload ? 0 : -1));
			}
		} else {
			if (reload) rbtn->ShowWindow(SW_HIDE);
		}
	}

	if (pItem->key && (m_IsAdvMode || invflag == 0)) {
		m_CfgVal = *pItem->pvalue;
		if (reload) m_CfgValEdit.SetReadOnly(!!pItem->onchange);
		if (reload) m_CfgValEdit.ShowWindow(SW_SHOW);
	} else {
		if (reload) m_CfgValEdit.ShowWindow(SW_HIDE);
	}

	if (pItem->enumobj) {
		if (reload) pItem->enumobj->EnumConfigValues(m_EnumList);
		if (pItem->enumobj->IsValueLegal(m_EnumList, *pItem->pvalue)) {
			m_OptDesc = pItem->enumobj->GetValueDescription(*pItem->pvalue);
			descflag = 1;
		}
		if (reload) m_ChooseBtn.ShowWindow(SW_SHOW);
	} else {
		if (reload) m_ChooseBtn.ShowWindow(SW_HIDE);
	}

	if (!descflag) {
		if (pItem->defoptdesc) {
			m_OptDesc = CString(pItem->defoptdesc);
		} else {
			m_OptDesc = STRTABLE(IDS_NODESC);
		}
	}

	if (reload) m_OptDescShowing = -1;

	GetDlgItem(IDC_CFGDESC)->ShowWindow(m_CfgDesc != EMPTYSTR);
	GetDlgItem(IDC_OPTDESC)->ShowWindow(m_OptDesc != EMPTYSTR);

	UpdateData(FALSE);
}

void CPatchConfigDlg::UpdateConfigItem(const CString &new_value)
{
	if (*m_ItemSelected->pvalue != new_value) {
		if (!m_ItemSelected->onchange || m_ItemSelected->onchange(this, *m_ItemSelected->pvalue, new_value)) {
			*m_ItemSelected->pvalue = new_value;
			ConfigDirty = true;
		}
	}
	SelectConfigItem(m_ItemSelected);
}
/////////////////////////////////////////////////////////////////////////////
// CPatchConfigDlg dialog

CPatchConfigDlg::CPatchConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPatchConfigDlg::IDD, pParent)
{
	m_ItemSelected = NULL;
	memset(m_LastItemSelected, 0, sizeof(m_LastItemSelected));
	m_ModeSwitchPreferredItem = NULL;
	m_IsAdvMode = 0;
	m_OptDescShowing = -1;
	m_LockSelUpdate = 0;

	//{{AFX_DATA_INIT(CPatchConfigDlg)
	m_CfgTitle = _T("");
	m_CfgDesc = _T("");
	m_CfgVal = _T("");
	m_OptDesc = _T("");
	m_RadioVal = -1;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPatchConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatchConfigDlg)
	DDX_Control(pDX, IDC_CFGVAL, m_CfgValEdit);
	DDX_Control(pDX, IDCANCEL, m_CancelBtn);
	DDX_Control(pDX, IDOK, m_OKBtn);
	DDX_Control(pDX, IDC_RUNFUNC, m_RunfuncBtn);
	DDX_Control(pDX, IDC_RADIO1, m_RadioBtn1);
	DDX_Control(pDX, IDC_RADIO2, m_RadioBtn2);
	DDX_Control(pDX, IDC_RADIO3, m_RadioBtn3);
	DDX_Control(pDX, IDC_TOGGLEADVOPTS, m_ToggleAdvBtn);
	DDX_Control(pDX, IDC_CHOOSEFROMLIST, m_ChooseBtn);
	DDX_Control(pDX, IDC_CFGTREE, m_CfgTree);
	DDX_Text(pDX, IDC_CFGTITLE, m_CfgTitle);
	DDX_Text(pDX, IDC_CFGDESC, m_CfgDesc);
	DDX_Text(pDX, IDC_CFGVAL, m_CfgVal);
	DDX_Text(pDX, IDC_OPTDESC, m_OptDesc);
	DDX_Radio(pDX, IDC_RADIO1, m_RadioVal);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPatchConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CPatchConfigDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_TOGGLEADVOPTS, OnToggleAdvOpts)
	ON_NOTIFY(TVN_SELCHANGEDW, IDC_CFGTREE, OnSelchangedCfgtreeW)
	ON_NOTIFY(TVN_SELCHANGEDA, IDC_CFGTREE, OnSelchangedCfgtreeA)
	ON_BN_CLICKED(IDC_CHOOSEFROMLIST, OnChoosefromlist)
	ON_EN_CHANGE(IDC_CFGVAL, OnChangeCfgval)
	ON_BN_CLICKED(IDC_RADIO1, OnRadioClicked)
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_RADIO2, OnRadioClicked)
	ON_BN_CLICKED(IDC_RADIO3, OnRadioClicked)
	ON_BN_CLICKED(IDC_RUNFUNC, OnRunfunc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatchConfigDlg message handlers

BOOL CPatchConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

#if _MSC_VER >= 1800
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
#endif
	
	// TODO: Add extra initialization here

	SetTopMost(true);

	int i;
	CMouseMsgButton *rbtn[MAX_CONFIGDESC_OPTIONS] = {&m_RadioBtn1, &m_RadioBtn2, &m_RadioBtn3};
	for (i = 0; i < MAX_CONFIGDESC_OPTIONS; i++) {
		rbtn[i]->m_BtnID = i;
		rbtn[i]->m_CallbackParameter = this;
		rbtn[i]->m_OnMouseMoveCallback = OnMouseMoveCallbackWarpper;
	}

	ToggleAdvMode(true); // config descriptions will be loaded here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPatchConfigDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CPatchConfigDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}




void CPatchConfigDlg::OnToggleAdvOpts() 
{
	ToggleAdvMode(false);
}

void CPatchConfigDlg::OnSelchangedCfgtreeW(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEWW* pNMTreeView = (NM_TREEVIEWW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	ConfigDescItem *pItem = (ConfigDescItem *) pNMTreeView->itemNew.lParam;
	SelectConfigItem(pItem);

	*pResult = 0;
}
void CPatchConfigDlg::OnSelchangedCfgtreeA(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEWA* pNMTreeView = (NM_TREEVIEWA*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	ConfigDescItem *pItem = (ConfigDescItem *) pNMTreeView->itemNew.lParam;
	SelectConfigItem(pItem);

	*pResult = 0;
}

void CPatchConfigDlg::OnChoosefromlist() 
{
	// TODO: Add your control notification handler code here
	CChooseFromListDlg enumdlg;
	
	enumdlg.m_pEnumObj = m_ItemSelected->enumobj;
	enumdlg.m_pEnumData = &m_EnumList;

	enumdlg.m_Choosen = -1;
	std::vector<CString>::iterator it;
	for (it = m_EnumList.begin(); it != m_EnumList.end(); it++) {
		if (m_ItemSelected->enumobj->IsValueEqual(*it, m_CfgVal)) {
			enumdlg.m_Choosen = it - m_EnumList.begin();
			break;
		}
	}
	int nResponse = enumdlg.DoModal();
	if (nResponse == IDOK) {
		UpdateConfigItem(enumdlg.m_Result);
	}
}

void CPatchConfigDlg::OnChangeCfgval() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	UpdateConfigItem(m_CfgVal);
}

void CPatchConfigDlg::OnRadioClicked() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_RadioVal >= 0 && m_ItemSelected->optlist[m_RadioVal].description) {
		UpdateConfigItem(CString(m_ItemSelected->optlist[m_RadioVal].value));
	}
}

void CPatchConfigDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	BtnMouseMove(-1);
	CDialog::OnMouseMove(nFlags, point);
}


void CPatchConfigDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	if (!ConfigDirty || MessageBox(STRTABLE(IDS_CONFIRMQUIT), STRTABLE(IDS_CONFIRMQUIT_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		CDialog::OnCancel();
	}
}



void CPatchConfigDlg::OnOK() 
{
	// TODO: Add extra validation here
	CWnd *pWnd = GetFocus();
	if (pWnd == GetDlgItem(IDOK)) {
		if (ConfigDirty) {
			while (!TrySaveConfigData()) {
				int ret = MessageBox(STRTABLE(IDS_CANTSAVE), STRTABLE(IDS_CANTSAVE_TITLE), MB_ICONWARNING | MB_RETRYCANCEL);
				if (ret == IDCANCEL) {
					return;
				}
			}
		}
		CDialog::OnOK();
	}
}

void CPatchConfigDlg::OnRunfunc() 
{
	// TODO: Add your control notification handler code here
	m_ItemSelected->runfunc(this);
}

void CPatchConfigDlg::RestoreAllConfigToDefault(CPatchConfigDlg *dlg)
{
	if (dlg->MessageBox(STRTABLE(IDS_RESTOREDEFAULT_CONFIRM), STRTABLE(IDS_RESTOREDEFAULT_TITLE), MB_OKCANCEL | MB_ICONQUESTION) != IDOK) {
		return;
	}

	while (!(TryRebuildConfigFile() && TryLoadConfigData())) {
		if (dlg->MessageBox(STRTABLE(IDS_RESTOREDEFAULT_RETRY), STRTABLE(IDS_RESTOREDEFAULT_TITLE), MB_RETRYCANCEL | MB_ICONWARNING) == IDCANCEL) {
			dlg->MessageBox(STRTABLE(IDS_RESTOREDEFAULT_FAILED), STRTABLE(IDS_RESTOREDEFAULT_TITLE), MB_ICONERROR);
			die(0);
		}
	}

	On3DAPIConfigReset(dlg);

	dlg->MessageBox(STRTABLE(IDS_RESTOREDEFAULT_SUCCEED), STRTABLE(IDS_RESTOREDEFAULT_TITLE), MB_ICONINFORMATION);
}

void CPatchConfigDlg::SetTopMost(bool en)
{
	SetWindowPos((en ? &wndTopMost : &wndNoTopMost), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}
