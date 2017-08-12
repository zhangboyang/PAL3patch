// ChooseFromListDlg.cpp : implementation file
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseFromListDlg dialog


CChooseFromListDlg::CChooseFromListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseFromListDlg::IDD, pParent)
{
	m_Choosen = -1;
	m_pEnumData = NULL;
	//{{AFX_DATA_INIT(CChooseFromListDlg)
	//}}AFX_DATA_INIT
}


void CChooseFromListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseFromListDlg)
	DDX_Control(pDX, IDC_ENUMLIST, m_EnumList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseFromListDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseFromListDlg)
	ON_LBN_DBLCLK(IDC_ENUMLIST, OnDblclkEnumlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseFromListDlg message handlers

BOOL CChooseFromListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	std::vector<std::pair<CString, std::pair<CString, CString> > >::iterator it;
	for (it = m_pEnumData->begin(); it != m_pEnumData->end(); it++) {
		m_EnumList.AddString(it->second.first);
	}
	m_EnumList.SetCurSel(m_Choosen);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseFromListDlg::OnOK() 
{
	// TODO: Add extra validation here
	m_Choosen = m_EnumList.GetCurSel();
	if (m_Choosen >= 0) {
		CDialog::OnOK();
	} else {
		MessageBox(STRTABLE(IDS_NOCHOOSEN), STRTABLE(IDS_APPTITLE), MB_ICONWARNING);
	}
}

void CChooseFromListDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	m_Choosen = -1;
	CDialog::OnCancel();
}

void CChooseFromListDlg::OnDblclkEnumlist() 
{
	// TODO: Add your control notification handler code here
	OnOK();
}
