// SaveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "3DRCS.h"
#include "SaveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveDlg dialog


CSaveDlg::CSaveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSaveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaveDlg)
	m_ReverseFacet=false;
	m_xTranslate = 0.0f;
	m_xZoomSave = 1.0f;
	m_yZoomSave = 1.0f;
	m_zZoomSave = 1.0f;
	m_yTranslate = 0.0f;
	m_zTranslate = 0.0f;
	//}}AFX_DATA_INIT
	m_SaveAsPath="新生成文件.stl";
}


void CSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveDlg)
	DDX_Control(pDX, IDC_SAVEPATH, m_SavePathEdit);
	DDX_Text(pDX, IDC_SAVEPATH, m_SaveAsPath);
	DDX_Text(pDX, IDC_XTRANSLATE, m_xTranslate);
	DDX_Text(pDX, IDC_XZOOMSAVE, m_xZoomSave);
	DDX_Text(pDX, IDC_YZOOMSAVE, m_yZoomSave);
	DDX_Text(pDX, IDC_ZZOOMSAVE, m_zZoomSave);
	DDX_Text(pDX, IDC_YTRANSLATE, m_yTranslate);
	DDX_Text(pDX, IDC_ZTRANSLATE, m_zTranslate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveDlg, CDialog)
	//{{AFX_MSG_MAP(CSaveDlg)
	ON_BN_CLICKED(IDC_SAVE_PATH, OnSavePath)
	ON_BN_CLICKED(IDC_SAVEREVERSEFACET, OnSavereversefacet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveDlg message handlers

void CSaveDlg::OnSavePath() 
{
	CFileDialog dlog(FALSE,"文件",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,\
	"STL ASCII(*.ast)|*.ast|STL Binary(*.stl)|*.stl|PLY ASCII(*.ply)|*.ply|All Files(*.*)|*.*||",NULL);
	int structsize=88; 
	DWORD dwVersion;
	dwVersion = GetVersion(); 	//GetVersion检测目前的操作系统	
	if (dwVersion < 0x80000000)  
		structsize =88;//如果运行的操作系统是Windows NT/2000 ，显示新的文件对话框 
	else 
		structsize =76;//运行的操作系统Windows 95/98 ，则显示老的文件对话框
	dlog.m_ofn.lStructSize=structsize; 
	
	if (IDOK == dlog.DoModal())
	{
		m_SaveAsPath=dlog.GetPathName ();
		this->m_SavePathEdit.SetWindowText(m_SaveAsPath);
	}
	
}


void CSaveDlg::OnSavereversefacet() 
{
	this->m_ReverseFacet=!this->m_ReverseFacet;
}

//void CSaveDlg::GetSavePara()
//{

//}

