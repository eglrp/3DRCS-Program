// ModeInfo.cpp : implementation file
//

#include "stdafx.h"
#include "3DRCS.h"
#include "ModeInfo.h"
#include "3DRCSDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern float x_glMid,x_glMin,x_glMax;
extern float y_glMid,y_glMin,y_glMax;
extern float z_glMid,z_glMin,z_glMax;

extern double _Volume,_Area;

/////////////////////////////////////////////////////////////////////////////
// CModeInfo dialog


CModeInfo::CModeInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CModeInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModeInfo)
	m_nxMax = 0.0f;
	m_nxMid = 0.0f;
	m_nxMin = 0.0f;
	m_nyMax = 0.0f;
	m_nyMid = 0.0f;
	m_nyMin = 0.0f;
	m_nzMax = 0.0f;
	m_nzMid = 0.0f;
	m_nzMin = 0.0f;
	m_xSpan = 0.0f;
	m_ySpan = 0.0f;
	m_zSpan = 0.0f;
	m_ModeSurfaceArea = 0.0;
	m_ModeVolume = 0.0;
	//}}AFX_DATA_INIT
	m_edgenum = 0.0f;
	m_facetnum = 0.0f;
	//  m_vetex = 0.0f;
	m_vetexnum = 0.0f;
}


void CModeInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModeInfo)
	DDX_Text(pDX, IDC_XMAX, m_nxMax);
	DDX_Text(pDX, IDC_XMID, m_nxMid);
	DDX_Text(pDX, IDC_XMIN, m_nxMin);
	DDX_Text(pDX, IDC_YMAX, m_nyMax);
	DDX_Text(pDX, IDC_YMID, m_nyMid);
	DDX_Text(pDX, IDC_YMIN, m_nyMin);
	DDX_Text(pDX, IDC_ZMAX, m_nzMax);
	DDX_Text(pDX, IDC_ZMID, m_nzMid);
	DDX_Text(pDX, IDC_ZMIN, m_nzMin);
	DDX_Text(pDX, IDC_XSPAN, m_xSpan);
	DDX_Text(pDX, IDC_YSPAN, m_ySpan);
	DDX_Text(pDX, IDC_ZSPAN, m_zSpan);
	DDX_Text(pDX, IDC_MODESURFACEAREA, m_ModeSurfaceArea);
	DDX_Text(pDX, IDC_MODEVOLUME, m_ModeVolume);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDGE, m_edgenum);
	DDX_Text(pDX, IDC_FACET, m_facetnum);
	//  DDX_Text(pDX, IDC_VERTEX, m_vetex);
	DDX_Text(pDX, IDC_VERTEX, m_vetexnum);
}


BEGIN_MESSAGE_MAP(CModeInfo, CDialog)
	//{{AFX_MSG_MAP(CModeInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModeInfo message handlers

void CModeInfo::SetEdits()
{
	m_nxMax=int(x_glMax*100000.0f)/100000.0f;
	m_nxMid=int(x_glMid*100000.0f)/100000.0f;
	m_nxMin=int(x_glMin*100000.0f)/100000.0f;
	m_nyMax=int(y_glMax*100000.0f)/100000.0f;
	m_nyMid=int(y_glMid*100000.0f)/100000.0f;
	m_nyMin=int(y_glMin*100000.0f)/100000.0f;
	m_nzMax=int(z_glMax*100000.0f)/100000.0f;
	m_nzMid=int(z_glMid*100000.0f)/100000.0f;
	m_nzMin=int(z_glMin*100000.0f)/100000.0f;
	m_xSpan=int((x_glMax-x_glMin)*100000.0f)/100000.0f;
	m_ySpan=int((y_glMax-y_glMin)*100000.0f)/100000.0f;
	m_zSpan=int((z_glMax-z_glMin)*100000.0f)/100000.0f;

	m_edgenum=m_EdgeNumber;
	m_facetnum=m_FacetNumber;
	m_vetexnum=m_VertexNumber;
	m_ModeVolume=_Volume;
	m_ModeSurfaceArea=_Area;
	
// 	m_ModeSurfaceArea=this->m_pFr->m_pView->GetDocument()->m_ModeSurfaceArea;
// 	m_ModeVolume=this->m_pFr->m_pView->GetDocument()->m_ModeVolume;
}

BOOL CModeInfo::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	this->m_pFr = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
