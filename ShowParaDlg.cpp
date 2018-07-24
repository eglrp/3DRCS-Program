// ShowParaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "3DRCS.h"
#include "ShowParaDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////// ////////////////////
// CShowParaDlg dialog


CShowParaDlg::CShowParaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowParaDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShowParaDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CShowParaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShowParaDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShowParaDlg, CDialog)
	//{{AFX_MSG_MAP(CShowParaDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShowParaDlg message handlers
