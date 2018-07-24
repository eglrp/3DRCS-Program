// PSOintro.cpp : implementation file
//

#include "stdafx.h"

#include "PSOintro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPSOintro dialog


CPSOintro::CPSOintro(CWnd* pParent /*=NULL*/)
	: CDialog(CPSOintro::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPSOintro)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPSOintro::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPSOintro)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPSOintro, CDialog)
	//{{AFX_MSG_MAP(CPSOintro)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPSOintro message handlers
