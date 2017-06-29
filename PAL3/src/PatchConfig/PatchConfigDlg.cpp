// PatchConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatchConfig.h"
#include "PatchConfigDlg.h"
#include "ChooseFromListDlg.h"
#include "ConfigData.h"

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
	LoadConfigDescription();
}

void CPatchConfigDlg::LoadConfigDescription()
{
	HTREEITEM cur;
	ConfigDescItem *pItem;
	// save selected item state
	ConfigDescItem *selitem = m_ItemSelected;
	if (!selitem) selitem = ConfigDescList;
	m_ItemSelected = NULL;
	
	// load into tree
	LockWindowUpdate();
	m_CfgTree.DeleteAllItems();
	std::vector<HTREEITEM> treestack;
	treestack.push_back(TVI_ROOT);
	HTREEITEM selnode = NULL;
	for (pItem = ConfigDescList; ; pItem++) {
		if (!m_IsAdvMode && pItem->is_adv) continue;
		while (pItem->level + 1 < (int) treestack.size()) {
			m_CfgTree.Expand(treestack.back(), TVE_EXPAND);
			treestack.pop_back();
		}
		if (pItem->level < 0) break;
		cur = m_CfgTree.InsertItem(TVIF_TEXT | TVIF_PARAM, pItem->title, 0, 0, 0, 0, (LPARAM) pItem, treestack.back(), NULL);
		if (pItem == selitem) {
			selnode = cur;
		}
		if (pItem->level + 1 >= (int) treestack.size()) {
			treestack.push_back(cur);
		}
	}

	m_ItemSelected = NULL; // force reload
	if (selnode == NULL) selnode = m_CfgTree.GetRootItem();
	m_CfgTree.SelectItem(selnode);
	UnlockWindowUpdate();
}

void CPatchConfigDlg::SelectConfigItem(ConfigDescItem *pItem)
{
	int reload = (m_ItemSelected != pItem); // should we reload data
	int i;
	int descflag = 0; // option description updated flag
	
	m_ItemSelected = pItem;

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
	

	CWnd *txtinput = GetDlgItem(IDC_CFGVAL);
	if (pItem->key && (m_IsAdvMode || invflag == 0)) {
		m_CfgVal = *pItem->pvalue;
		if (reload) txtinput->ShowWindow(SW_SHOW);
	} else {
		if (reload) txtinput->ShowWindow(SW_HIDE);
	}

	if (pItem->enumfunc) {
		if (reload) pItem->enumfunc(m_EnumList);
		std::vector<std::pair<CString, std::pair<CString, CString> > >::iterator it;
		for (it = m_EnumList.begin(); it != m_EnumList.end(); it++) {
			if (it->first == *pItem->pvalue) {
				if (it->second.second != EMPTYSTR) {
					m_OptDesc = it->second.second;
				} else {
					m_OptDesc = it->second.first;
				}
				descflag = 1;
				break;
			}
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

void CPatchConfigDlg::UpdateConfigItem()
{
	SelectConfigItem(m_ItemSelected);
}
/////////////////////////////////////////////////////////////////////////////
// CPatchConfigDlg dialog

CPatchConfigDlg::CPatchConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPatchConfigDlg::IDD, pParent)
{
	m_ItemSelected = NULL;
	m_OptDescShowing = -1;

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
	ON_NOTIFY(TVN_SELCHANGED, IDC_CFGTREE, OnSelchangedCfgtree)
	ON_BN_CLICKED(IDC_CHOOSEFROMLIST, OnChoosefromlist)
	ON_EN_CHANGE(IDC_CFGVAL, OnChangeCfgval)
	ON_BN_CLICKED(IDC_RADIO1, OnRadioClicked)
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_RADIO2, OnRadioClicked)
	ON_BN_CLICKED(IDC_RADIO3, OnRadioClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatchConfigDlg message handlers

BOOL CPatchConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	int i;
	CMouseMsgButton *rbtn[MAX_CONFIGDESC_OPTIONS] = {&m_RadioBtn1, &m_RadioBtn2, &m_RadioBtn3};
	for (i = 0; i < MAX_CONFIGDESC_OPTIONS; i++) {
		rbtn[i]->m_BtnID = i;
		rbtn[i]->m_CallbackParameter = this;
		rbtn[i]->m_OnMouseMoveCallback = OnMouseMoveCallbackWarpper;
	}

	ToggleAdvMode(TRUE); // config descriptions will be loaded here

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
	ToggleAdvMode(FALSE);
}


void CPatchConfigDlg::OnSelchangedCfgtree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	ConfigDescItem *pItem = (ConfigDescItem *) pNMTreeView->itemNew.lParam;
	SelectConfigItem(pItem);

	*pResult = 0;
}

void CPatchConfigDlg::OnChoosefromlist() 
{
	// TODO: Add your control notification handler code here
	CChooseFromListDlg enumdlg;
	enumdlg.m_pEnumData = &m_EnumList;

	enumdlg.m_Choosen = -1;
	std::vector<std::pair<CString, std::pair<CString, CString> > >::iterator it;
	for (it = m_EnumList.begin(); it != m_EnumList.end(); it++) {
		if (it->first == m_CfgVal) {
			enumdlg.m_Choosen = it - m_EnumList.begin();
			break;
		}
	}
	int nResponse = enumdlg.DoModal();
	if (nResponse == IDOK) {
		*m_ItemSelected->pvalue = m_EnumList[enumdlg.m_Choosen].first;
		UpdateConfigItem();
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
	*m_ItemSelected->pvalue = m_CfgVal;
	UpdateConfigItem();
}

void CPatchConfigDlg::OnRadioClicked() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_RadioVal >= 0 && m_ItemSelected->optlist[m_RadioVal].description) {
		*m_ItemSelected->pvalue = CString(m_ItemSelected->optlist[m_RadioVal].value);
		UpdateConfigItem();
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
	if (MessageBox(STRTABLE(IDS_CONFIRMQUIT), STRTABLE(IDS_CONFIRMQUIT_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		CDialog::OnCancel();
	}
}



void CPatchConfigDlg::OnOK() 
{
	// TODO: Add extra validation here
	CWnd *pWnd = GetFocus();
	if (pWnd == GetDlgItem(IDOK)) {
		while (!TrySaveConfigData()) {
			int ret = MessageBox(STRTABLE(IDS_CANTSAVE), STRTABLE(IDS_CANTSAVE_TITLE), MB_ICONWARNING | MB_RETRYCANCEL);
			if (ret == IDCANCEL) {
				break;
			}
		}
		CDialog::OnOK();
	}
}
