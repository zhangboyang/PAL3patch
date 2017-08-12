// MouseMsgButton.cpp : implementation file
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMouseMsgButton

CMouseMsgButton::CMouseMsgButton()
{
	m_BtnID = -1;
	m_CallbackParameter = NULL;
	m_OnMouseMoveCallback = NULL;
}

CMouseMsgButton::~CMouseMsgButton()
{
}


BEGIN_MESSAGE_MAP(CMouseMsgButton, CButton)
	//{{AFX_MSG_MAP(CMouseMsgButton)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMouseMsgButton message handlers

void CMouseMsgButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (m_OnMouseMoveCallback) m_OnMouseMoveCallback(m_CallbackParameter, m_BtnID);
	CButton::OnMouseMove(nFlags, point);
}
