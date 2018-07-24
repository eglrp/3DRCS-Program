// HoleFill.cpp : implementation file
//

#include "stdafx.h"
#include "Test1.h"
#include "HoleFill.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HoleFill dialog


HoleFill::HoleFill(CWnd* pParent /*=NULL*/)
	: CDialog(HoleFill::IDD, pParent)
{
	//{{AFX_DATA_INIT(HoleFill)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void HoleFill::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(HoleFill)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(HoleFill, CDialog)
	//{{AFX_MSG_MAP(HoleFill)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HoleFill message handlers
