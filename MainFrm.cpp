// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "3DRCS.h"

#include "ControlWnd.h"
#include "ProjectView.h"
#include "MainFrm.h"
#include "PSOintro.h"
#include "PCG.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame
#define BYTE_PER_LINE(w,c)  ((((w)*(c)+31)/32)*4)

extern bool gbIsGetData;
extern bool gbDataIsEmpty;
extern float	m_Prox,m_Proy,m_Proz; //缩放因子
extern float  xrof,yrof,zrof;

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
//{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_VIEW_TOOLBAR, OnViewToolbar)
	ON_COMMAND(ID_VIEW_STATUS_BAR, OnViewStatusBar)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpenSTL)
// 	ON_COMMAND(IDD_PSOIntroduction, OnPSOIntroduction)
	//}}AFX_MSG_MAP

	ON_COMMAND(IDM_PICS_OPEN, &CMainFrame::OnPicsOpen)
	ON_COMMAND(IDC_MC_CONSTRUCT, &CMainFrame::OnMcConstruct)
	ON_COMMAND(IDC_MT_CONSTRUCT, &CMainFrame::OnMtConstruct)
	ON_COMMAND(IDM_REDUCE_MESH3, &CMainFrame::OnReduceMesh3)
	ON_COMMAND(IDM_CLEAR_DATA, &CMainFrame::OnClearData)
	ON_COMMAND(IDM_REDUCE_MESH, &CMainFrame::OnReduceMesh)
	ON_COMMAND(IDM_SMOOTH_Mesh, &CMainFrame::OnSmoothMesh)
	ON_COMMAND(IDM_RELOAD_DATA, &CMainFrame::OnReloadData)
//	ON_UPDATE_COMMAND_UI(IDC_FACET, &CMainFrame::OnUpdateFacet)
//	ON_UPDATE_COMMAND_UI(IDC_EDGE, &CMainFrame::OnUpdateEdge)
ON_COMMAND(IDM_CHECKLIGHTING, &CMainFrame::OnChecklighting)
ON_UPDATE_COMMAND_UI(IDM_CHECKLIGHTING, &CMainFrame::OnUpdateChecklighting)
ON_COMMAND(IDM_FACETREVERSE, &CMainFrame::OnFacetreverse)
ON_UPDATE_COMMAND_UI(IDM_FACETREVERSE, &CMainFrame::OnUpdateFacetreverse)
ON_COMMAND(IDM_CHECKCOOLDINATE, &CMainFrame::OnCheckcooldinate)
ON_UPDATE_COMMAND_UI(IDM_CHECKCOOLDINATE, &CMainFrame::OnUpdateCheckcooldinate)
ON_COMMAND(IDM_ZOOMADD, &CMainFrame::OnZoomadd)
ON_COMMAND(IDM_ZOOMDEL, &CMainFrame::OnZoomdel)
ON_COMMAND(IDM_SHOWMEDIUM, &CMainFrame::OnShowmedium)
ON_COMMAND(IDM_ZOOMBACK, &CMainFrame::OnZoomback)
ON_COMMAND(IDM_ModInfo, &CMainFrame::OnModinfo)
ON_COMMAND(IDM_CalStandard, &CMainFrame::OnCalstandard)
ON_COMMAND(IDM_REPAIRHOLE, &CMainFrame::OnRepairhole)
//ON_COMMAND(IDM_MYMETHOD, &CMainFrame::OnMyMethod)
ON_COMMAND(IDM_PatchHoles, &CMainFrame::OnPatchholes)
END_MESSAGE_MAP()

static UINT indicators[] =
{
 	    ID_SEPARATOR,           // status line indicator
		ID_SEPARATOR,
		ID_PROGRESSAREA,
        ID_SEPARATOR,
		ID_INDICATOR_X,
		ID_INDICATOR_Y,
		ID_INDICATOR_Z,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction
#include "time.h"
#include <sys/timeb.h>
//定义了两个宏，用来计算并显示某段计算步骤所经历的时间
#define TimeCountStart CTime startTime = CTime::GetCurrentTime();\
	struct _timeb timebuffer;\
	int mills1,mills2;\
    _ftime(&timebuffer);\
	mills1=timebuffer.millitm;\
	CString str1;\

#define TimeCountEnd CTime endTime = CTime::GetCurrentTime();\
	CTimeSpan elapsedTime = endTime - startTime;\
    _ftime(&timebuffer);\
	mills2=timebuffer.millitm;\
	long int timeused;\
	timeused=elapsedTime.GetTotalSeconds();\
	if (mills1<=mills2)\
		timeused=timeused*1000+(mills2-mills1);\
	else\
		timeused=(timeused-1)*1000+(mills1-mills2);\
	str1.Format ("该步骤所用时间：%d 毫秒",timeused);\
    ::AfxMessageBox(str1);

CMainFrame::CMainFrame()
	
	: m_ReduceRate2(0)
	
{
	m_bShowToolbar=true;
	
	m_bShowLight=true;
	m_bFacetreverse=false;
	m_bShowcoordinate=false;

	m_pView = NULL;
	m_bCreat = FALSE;
	m_ReduceRate3=30;
	m_AngleThreshold=2.5;
	m_DistanceRate=0.1;
	m_msmoothnum=5;
	m_msmoothrate=0.1;
}

CMainFrame::~CMainFrame()
{

}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//创建扩展工具栏
	//if (!this->CreateExToolBar())
    // return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE| CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,  CRect(1,1,1,1), 4002) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
		{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//EnableDocking(CBRS_ALIGN_ANY);
	//DockPane(&m_wndToolBar);
	if (!m_ToolBar1.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE| CBRS_LEFT
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,  CRect(1,1,1,1), 4003) ||
		!m_ToolBar1.LoadToolBar(IDR_TOOLBAR1))
		{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}


	if (!m_wndStatusBar.Create(this) ||	!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetPaneInfo(0,SBPS_DISABLED ,0,10);
	m_wndStatusBar.SetPaneInfo(1,0,0,240);
	m_wndStatusBar.SetPaneInfo(2,0,SBPS_STRETCH,100);
	m_wndStatusBar.SetPaneInfo(3,0,0,100);
	m_wndStatusBar.SetPaneInfo(4,0,0,100);
	m_wndStatusBar.SetPaneInfo(5,0,0,100);
	m_wndStatusBar.SetPaneInfo(6,0,0,100);
		
	this->CreateFont();

	this->GetWndPointer();

	this->CreateProcess(); 

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	
	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		| WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

		
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	

	/*CRect rc;  
  
    // 获取框架窗口客户区的CRect对象   
    GetClientRect(&rc);   
	int cx=rc.Width();
	int cy=rc.Height();//ADD BY WHY on 2014/11/27*/
  
	
	
	
	
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	float m_bKuanPin=float(cy)/float(cx);

	//将主窗口分为一行两列
    if(m_Sp1.CreateStatic(this,1,2)==NULL) 
		return FALSE;

	//将右边的分为两行一列
	if(m_Sp2.CreateStatic(&m_Sp1,2,1,WS_CHILD|WS_VISIBLE, m_Sp1.IdFromRowCol(0, 1))==NULL) 
		return FALSE; 
	//
		
		m_Sp1.CreateView(0,0,RUNTIME_CLASS(C3DRCSView),CSize(cx,cy), pContext);
	    m_Sp2.CreateView(0,0,RUNTIME_CLASS(CControlWnd),CSize(0,0),pContext);
		m_Sp2.CreateView(1,0,RUNTIME_CLASS(CProjectView),CSize(0,0),pContext);
		//why
		
	/*if(m_bKuanPin>=0.7)
		m_Sp1.CreateView(0,0,RUNTIME_CLASS(C3DRCSView),CSize(cx*3/5-15,cy), pContext);
	else
		m_Sp1.CreateView(0,0,RUNTIME_CLASS(C3DRCSView),CSize(cx*2/3,cy), pContext);

	if(m_bKuanPin>=0.70)
	{
		m_Sp2.CreateView(0,0,RUNTIME_CLASS(CControlWnd),CSize(cx*2/5,cy/2+20),pContext);
		m_Sp2.CreateView(1,0,RUNTIME_CLASS(CProjectView),CSize(cx*2/5,200),pContext); 
	}
	else
	{
		m_Sp2.CreateView(0,0,RUNTIME_CLASS(CControlWnd),CSize(cx*2/5,cy/2),pContext);
		m_Sp2.CreateView(1,0,RUNTIME_CLASS(CProjectView),CSize(cx*2/5,200),pContext); 
	}*/
	
    return TRUE;  	
}

void CMainFrame::GetWndPointer()
{
	CWnd *pWnd = NULL;

	pWnd = this->m_Sp1.GetPane(0,0);
	this->m_pView = DYNAMIC_DOWNCAST(C3DRCSView,pWnd);

	pWnd = this->m_Sp2.GetPane(0,0);
	this->m_pCtrlWnd = DYNAMIC_DOWNCAST(CControlWnd, pWnd);

	pWnd = this->m_Sp2.GetPane(1,0);
	this->m_pProView = DYNAMIC_DOWNCAST(CProjectView, pWnd);


}

void CMainFrame::CreateFont()
{
	m_13Font.CreateFont(
		13,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                 // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		"宋体");                   // lpszFacename
}


void CMainFrame::CreateProcess()
{
	RECT rect;
	m_wndStatusBar.GetItemRect(2,&rect);
	if(!m_bCreat)
	{
		m_Progress.Create(WS_VISIBLE|WS_CHILD|PBS_SMOOTH  ,rect,&m_wndStatusBar,10);
		m_Progress.SetRange(0,100);
		m_Progress.SetStep(1);
		m_bCreat=TRUE;
	}

}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
	
	if(m_bCreat)
	{
		RECT rect;
		m_wndStatusBar.GetItemRect(2,&rect);
		m_Progress.SetWindowPos(&wndTop,rect.left ,rect.top,
			rect.right-rect.left,rect.bottom-rect.top,SWP_SHOWWINDOW);
	}	

}


void CMainFrame::InvalidateAllWnd()
{
	this->m_pView->Invalidate();
	m_pProView->Invalidate();
}

void CMainFrame::OnClose() 
{
/*	if (::gbIsGetData)
	{
		MessageBox("正在读取数据,请稍候!","提示信息",MB_ICONINFORMATION);
		return;
	}
//	int nResult = MessageBox("确认退出系统?","提示信息",MB_ICONINFORMATION|MB_YESNO);
//	if (nResult == IDNO)
//		return;
*/
	CFrameWnd::OnClose();
}


BOOL CMainFrame::CreateExToolBar()
{
	CImageList img;
	CString str;
	
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_TRANSPARENT , WS_CHILD  | CBRS_ALIGN_LEFT| CBRS_GRIPPER 
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | CBRS_BORDER_ANY ))
	{
		return -1;      
	}
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	//下面的代码是设置按钮的宽度和长度
	m_wndToolBar.GetToolBarCtrl().SetButtonWidth(40, 40);

	//下面的代码是设置"热"的位图
	img.Create(35, 35, ILC_COLOR8|ILC_MASK,2,2);
	img.SetBkColor(::GetSysColor(COLOR_BTNFACE));
	img.Add(AfxGetApp()->LoadIcon(IDI_OPEN));
	img.Add(AfxGetApp()->LoadIcon(IDI_SAVE));
	img.Add(AfxGetApp()->LoadIcon(IDI_SET));
	img.Add(AfxGetApp()->LoadIcon(IDI_CLEAR));
	img.Add(AfxGetApp()->LoadIcon(IDI_ZOOMOUT));

	m_wndToolBar.GetToolBarCtrl().SetHotImageList(&img);
	img.Detach();

	//下面的代码是设置"冷"的位图
	img.Create(30, 30, ILC_COLOR8|ILC_MASK, 2,2);
	img.SetBkColor(::GetSysColor(COLOR_BTNFACE));
	img.Add(AfxGetApp()->LoadIcon(IDI_OPEN));
	img.Add(AfxGetApp()->LoadIcon(IDI_SAVE));
	img.Add(AfxGetApp()->LoadIcon(IDI_SET));
	img.Add(AfxGetApp()->LoadIcon(IDI_CLEAR));
	img.Add(AfxGetApp()->LoadIcon(IDI_ZOOMOUT));

	m_wndToolBar.GetToolBarCtrl().SetImageList(&img);
	img.Detach();

	//改变属性
	m_wndToolBar.ModifyStyle(0, TBSTYLE_FLAT |CBRS_TOOLTIPS | TBSTYLE_TRANSPARENT|TBBS_CHECKBOX );
	m_wndToolBar.SetButtons(NULL,5); //个数	
	m_wndToolBar.SetButtonInfo(0, ID_FILE_OPEN, TBBS_GROUP   , 0);
	m_wndToolBar.SetButtonText(0, "打开");

	m_wndToolBar.SetButtonInfo(1, ID_FILE_SAVE, TBSTYLE_BUTTON, 1);
	m_wndToolBar.SetButtonText(1, "保存");

	m_wndToolBar.SetButtonInfo(2, ID_PARA_SET, TBSTYLE_BUTTON, 2);
	m_wndToolBar.SetButtonText(2, "设置");

	m_wndToolBar.SetButtonInfo(3, IDC_CLEAR_DATA, TBBS_GROUP, 3);
	m_wndToolBar.SetButtonText(3, "清屏");
	
	m_wndToolBar.SetButtonInfo(4, IDM_ZOOMDEL, TBBS_GROUP, 0);
	m_wndToolBar.SetButtonText(4, "缩小");
//	m_wndToolBar.SetButtonInfo(4, IDC_MSG_BUTTONSPLI , TBBS_SEPARATOR  , 0);


	CRect rectToolBar;
	//得到按钮的大小
	m_wndToolBar.GetItemRect(0, &rectToolBar);
	rectToolBar.bottom +=1;
	rectToolBar.right +=1;
	//设置按钮的大小

	m_wndToolBar.SetSizes(rectToolBar.Size(), CSize(30,30));
	
	return TRUE;

}

void CMainFrame::OnViewToolbar() 
{
	// TODO: Add your command handler code here
	m_bShowToolbar=!m_bShowToolbar;
	ShowControlBar(&m_wndToolBar,m_bShowToolbar,FALSE);   
	this->Invalidate();
}

void CMainFrame::OnViewStatusBar() 
{
	m_wndStatusBar.ShowWindow((m_wndStatusBar.GetStyle()&WS_VISIBLE)==0);
	RecalcLayout();	
}

void CMainFrame::OnFileOpenSTL() 
{
	this->m_pCtrlWnd->OpenSTL();
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class

// 	CPSOintro dlg;
// 	dlg.DoModal();
	
	return CFrameWnd::OnCommand(wParam, lParam);
}

void CMainFrame::OnPSOIntroduction() 
{
	// TODO: Add your command handler code here

	
}


void CMainFrame::OnPicsOpen()
{
	// TODO: 在此添加命令处理程序代码
	this->m_pView->GetDocument()->OpenPics();
}


void CMainFrame::OnMcConstruct()
{
	// TODO: 在此添加命令处理程序代码
	// 创建对话框
	CConstruct dlgconstruct;
	
	// 初始化变量值
	dlgconstruct.m_MXconZoom =1;
	dlgconstruct.m_MYconZoom =1;
	dlgconstruct.m_MZconZoom =1;
	
	
	// 显示对话框，提示用户设定平移量
	if (dlgconstruct.DoModal() != IDOK)
	{
		// 返回
		return;
	}

	this->m_pCtrlWnd->m_XconZoom=dlgconstruct.m_MXconZoom;
	this->m_pCtrlWnd->m_YconZoom=dlgconstruct.m_MYconZoom;
	this->m_pCtrlWnd->m_ZconZoom=dlgconstruct.m_MZconZoom;
	int threshold=dlgconstruct.m_MThreshold;
	this->m_pCtrlWnd->m_bDeleteRing=dlgconstruct.m_MbDeleteRing;
	if(this->m_pCtrlWnd->m_bDeleteRing)
		this->m_pView->GetDocument()->DeleteRing();
	this->m_pView->GetDocument()->m_TargetValue=threshold;
	// 获取用户设定的平移量
	
	AfxBeginThread(C3DRCSDoc::MarchingCubes,this->m_pView->GetDocument());
}


void CMainFrame::OnMtConstruct()
{
	// TODO: 在此添加命令处理程序代码
	CConstruct dlgconstruct;

	// 初始化变量值
	dlgconstruct.m_MXconZoom =1;
	dlgconstruct.m_MYconZoom =1;
	dlgconstruct.m_MZconZoom =1;
	
	
	// 显示对话框，提示用户设定平移量
	if (dlgconstruct.DoModal() != IDOK)
	{
		// 返回
		return;
	}

	this->m_pCtrlWnd->m_XconZoom=dlgconstruct.m_MXconZoom;
	this->m_pCtrlWnd->m_YconZoom=dlgconstruct.m_MYconZoom;
	this->m_pCtrlWnd->m_ZconZoom=dlgconstruct.m_MZconZoom;
	int threshold=dlgconstruct.m_MThreshold;
	this->m_pCtrlWnd->m_bDeleteRing=dlgconstruct.m_MbDeleteRing;
	if(this->m_pCtrlWnd->m_bDeleteRing)
		this->m_pView->GetDocument()->DeleteRing();
	this->m_pView->GetDocument()->m_TargetValue=threshold;
	AfxBeginThread(C3DRCSDoc::MarchingTetrahedra,this->m_pView->GetDocument());
}


void CMainFrame::OnReduceMesh3()
{
	// TODO: 在此添加命令处理程序代码
	if (::gbIsGetData)
		return;
	
	if (::gbDataIsEmpty)
		return;
	
	// 创建对话框
	CAdaptive dlgAdaptive;

	// 初始化变量值
	dlgAdaptive.m_reducerate3 =m_ReduceRate3 ;
	


	// 显示对话框，提示用户设定平移量
	if (dlgAdaptive.DoModal() != IDOK)
	{
		// 返回
		return;
	}

	// 获取用户设定的平移量
	m_ReduceRate3 = dlgAdaptive.m_reducerate3;


	//this->m_OpenPicbtn.EnableWindow(FALSE);
	//this->m_GetDatabtn.EnableWindow(FALSE);
	//::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);
	//CString str;
	//GetDlgItem(IDC_ANGLE_THRESHOLD)->GetWindowText(str);
	//m_nReduceAngleThreshold = (float)atof(str);
	
	//GetDlgItem(IDC_AERA_THRESHOLD)->GetWindowText(str);
	//m_nReduceAreaThreshold=(float)atof(str);
	
	//GetDlgItem(IDC_VOLUME_THRESHOLD)->GetWindowText(str);
	//m_nReduceVolumeThreshold=(float)atof(str);
	
	//GetDlgItem(IDC_REDUCE_RATE_EDIT)->GetWindowText(str);
	//ReduceRate=(float)atof(str);
	
	//	DWORD start = GetTickCount();

	TimeCountStart;
	this->m_pView->GetDocument()->ReduceMesh_Adaptive(); 
	TimeCountEnd;

	
	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已精简数据");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);
}


void CMainFrame::OnClearData()
{
	// TODO: 在此添加命令处理程序代码

	this->m_pCtrlWnd->OnClearData1();
}


void CMainFrame::OnReduceMesh()
{
	// TODO: 在此添加命令处理程序代码
	

	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

	//this->m_OpenPicbtn.EnableWindow(FALSE);
	//this->m_GetDatabtn.EnableWindow(FALSE);
	//::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);

	//CString str;
	//GetDlgItem(IDC_ANGLE_THRESHOLD)->GetWindowText(str);
	//m_nReduceAngleThreshold = (float)atof(str);

	//GetDlgItem(IDC_DISTANCE_THRESHOLD)->GetWindowText(str);
	//m_nReduceDistanceThreshold=(float)atof(str);

	//(IDC_REDUCE_RATE_EDIT)->GetWindowText(str);
	//m_nReduceRate=(float)atof(str);CAdaptive dlgAdaptive;
	// 创建对话框

	CReduce dlgreduce;
	// 初始化变量值
	dlgreduce.m_reducerate2 =m_ReduceRate2;
	dlgreduce.m_MnReduceAngleThreshold =m_AngleThreshold;
	dlgreduce.m_MnReduceDistanceThreshold =m_DistanceRate;


	// 显示对话框，提示用户设定平移量
	if (dlgreduce.DoModal() != IDOK)
	{
		// 返回
		return;
	}

	// 获取用户设定的平移量
	m_ReduceRate2 = dlgreduce.m_reducerate2;
	m_AngleThreshold=dlgreduce.m_MnReduceAngleThreshold;
	m_DistanceRate=dlgreduce.m_MnReduceDistanceThreshold;
	TimeCountStart;
	this->m_pView->GetDocument()->ReduceMesh(); 
	TimeCountEnd;
	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已精简数据");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);

	
}


void CMainFrame::OnSmoothMesh()
{
	// TODO: 在此添加命令处理程序代码
	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

	//this->m_OpenPicbtn.EnableWindow(FALSE);
	//this->m_GetDatabtn.EnableWindow(FALSE);
	//::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);

	/*CString str;
	GetDlgItem(IDC_SMOOTH_RATE_EDIT)->GetWindowText(str);
	m_nSmoothRate = (float)atof(str);

	GetDlgItem(IDC_SMOOTH_NUM)->GetWindowText(str);
	m_nSmoothNum = (float)atof(str);*/
	CSmooth dlgsmooth;
	// 初始化变量值
	dlgsmooth.m_MnSmoothRate =m_msmoothrate;
	dlgsmooth.m_MnSmoothNum =m_msmoothnum;
	


	// 显示对话框，提示用户设定平移量
	if (dlgsmooth.DoModal() != IDOK)
	{
		// 返回
		return;
	}

	// 获取用户设定的平移量
	m_msmoothrate = dlgsmooth.m_MnSmoothRate;
	m_msmoothnum=dlgsmooth.m_MnSmoothNum;
	
	TimeCountStart;

	this->m_pView->GetDocument()->SmoothMesh();

	TimeCountEnd;

	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已优化模型");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);

}


void CMainFrame::OnReloadData()
{
	// TODO: 在此添加命令处理程序代码
	if(!this->m_pCtrlWnd->m_bGetPics)
	{
		this->m_pCtrlWnd->Clear();
		AfxBeginThread(C3DRCSDoc::ReadFile,this->m_pView->GetDocument());
	}
	else
	{
		::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);
		if(!m_Vertex.empty())	m_Vertex.clear();
		if(!m_Facet.empty())	m_Facet.clear();
		if(!m_Edge.empty())		m_Edge.clear();
		m_VertexNumber=m_EdgeNumber=m_FacetNumber=0;
		this->m_pView->GetDocument()->m_ReduceRate=0;
	

	x_glMax = y_glMax = z_glMax =-1024.0;
	x_glMid = y_glMid = z_glMid =0.0;
	x_glMin = y_glMin = z_glMin =1024.0;
	m_Prox=m_Proy=m_Proz=1.0;
	this->m_pCtrlWnd->m_nxZoom=this->m_pCtrlWnd->m_nyZoom=this->m_pCtrlWnd->m_nzZoom=1.0;

		/*初始化光照，反向，坐标*/
	this->m_bShowLight=true;
	this->m_bFacetreverse=false;
	this->m_bShowcoordinate=false;
	/*初始化鼠标动作，点显示，滚轮动作*/
	this->m_pView->m_bShowEdge=false;
	 this->m_pView->m_bShowFacet=true;
	this->m_pView->m_bShowVertex=false;

	this->m_pView->m_bTurnx=true;
	this->m_pView->m_bTurny=false;
	this->m_pView->m_bTurnz=false;
	this->m_pView->m_bTranslate=false;
	this->m_pView->m_bTurn=true;
	this->m_pView->m_bZoomout=false;

	this->m_pView->m_ShowType=2;
this->m_pView->m_MouseAction = 0;
this->m_pView->nTurnType = 0;


this->m_pCtrlWnd->m_bLighting=false;
this->m_pCtrlWnd->m_bShowReverse=false;
this->m_pCtrlWnd->m_bShowCoordinate=false;

		//this->m_PointNum.SetWindowText("0");
		//this->m_EdgeNum.SetWindowText("0");
		//this->m_FacetNum.SetWindowText("0");
		//this->m_ReduceR.SetWindowText("0");
	//	EnablControlWhileConstruct();
		this->InvalidateAllWnd();
	}

}


//void CMainFrame::OnUpdateFacet(CCmdUI *pCmdUI)
//{
//	// TODO: 在此添加命令更新用户界面处理程序代码
//	  pCmdUI->SetCheck(true);//把菜单选中   
//}


//void CMainFrame::OnUpdateEdge(CCmdUI *pCmdUI)
//{
//	// TODO: 在此添加命令更新用户界面处理程序代码
//	 pCmdUI->SetCheck(true);
//}


void CMainFrame::OnChecklighting()
{
	// TODO: 在此添加命令处理程序代码
	this->m_pCtrlWnd->m_bLighting = !this->m_pCtrlWnd->m_bLighting;
	m_bShowLight=!m_bShowLight;
	this->m_pView->Invalidate();
}


void CMainFrame::OnUpdateChecklighting(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(m_bShowLight);
}


void CMainFrame::OnFacetreverse()
{
	// TODO: 在此添加命令处理程序代码
this->m_pCtrlWnd->m_bShowReverse = !this->m_pCtrlWnd->m_bShowReverse; 
m_bFacetreverse=!m_bFacetreverse;
	this->m_pView->Invalidate();
}


void CMainFrame::OnUpdateFacetreverse(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(m_bFacetreverse);
}


void CMainFrame::OnCheckcooldinate()
{
	// TODO: 在此添加命令处理程序代码
	this->m_pCtrlWnd->m_bShowCoordinate = !(this->m_pCtrlWnd->m_bShowCoordinate);
	m_bShowcoordinate=!m_bShowcoordinate;
	this->m_pView->Invalidate();
}


void CMainFrame::OnUpdateCheckcooldinate(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(m_bShowcoordinate);
}


void CMainFrame::OnZoomadd()
{
	// TODO: 在此添加命令处理程序代码
//	float x,y,z;
	if((m_Prox==0.02f)||(m_Proy==0.02f)||(m_Proz==0.02f))
	{
		m_Prox=m_Prox*1.25f+0.005;
		m_Proy=m_Proy*1.25f+0.005;
		m_Proz=m_Proz*1.25f+0.005;
	}
	else
	{
		m_Prox=m_Prox*1.25f;
		m_Proy=m_Proy*1.25f;
		m_Proz=m_Proz*1.25f;
	}
	
	/*x=m_Prox;	y=m_Proy;	z=m_Proz;

	CString   str; 
	str.Format("%0.2f",x);
	GetDlgItem(IDC_XZOOM)->SetWindowText(str);

	str.Format("%0.2f",y);
	GetDlgItem(IDC_YZOOM)->SetWindowText(str);

	str.Format("%0.2f",z);
	GetDlgItem(IDC_ZZOOM)->SetWindowText(str);*/
	this->m_pView->Invalidate();
}


void CMainFrame::OnZoomdel()
{
	// TODO: 在此添加命令处理程序代码
	float x,y,z;

	m_Prox=m_Prox/1.25f;
	m_Proy=m_Proy/1.25f;
	m_Proz=m_Proz/1.25f;

	x=m_Prox;	y=m_Proy;	z=m_Proz;

	/*CString   str; 
	str.Format("%0.2f",x);
	GetDlgItem(IDC_XZOOM)->SetWindowText(str);

	str.Format("%0.2f",y);
	GetDlgItem(IDC_YZOOM)->SetWindowText(str);

	str.Format("%0.2f",z);
	GetDlgItem(IDC_ZZOOM)->SetWindowText(str);*/
	this->m_pView->Invalidate();
}


void CMainFrame::OnShowmedium()
{
	// TODO: 在此添加命令处理程序代码
	m_Prox =this->m_pCtrlWnd->m_nxZoom=this->m_pCtrlWnd->m_xZoom;
	m_Proy = this->m_pCtrlWnd->m_nyZoom=this->m_pCtrlWnd->m_xZoom;
	m_Proz = this->m_pCtrlWnd->m_nzZoom=this->m_pCtrlWnd->m_xZoom;

	if((this->m_pView->newx1!=-100.0f)||(this->m_pView->newy1!=-100.0f))
	{
		this->m_pView->newx1=-100.0f;
		this->m_pView->newx2=100.0f;
		this->m_pView->newy1=-100.0f;
		this->m_pView->newy2=100.0f;
		this->m_pView->SetViewPort();
	}

	this->m_pCtrlWnd->SetXYZzoom();

	this->m_pView->Invalidate();
}


void CMainFrame::OnZoomback()
{
	// TODO: 在此添加命令处理程序代码
	xrof = -60.0f; yrof = 0.0f; zrof = 30.0f;
	this->m_pCtrlWnd->m_nxZoom =this->m_pCtrlWnd-> m_nyZoom = this->m_pCtrlWnd->m_nzZoom =this->m_pCtrlWnd-> m_xZoom;
	m_Prox = m_Proy = m_Proz =this->m_pCtrlWnd->m_xZoom;
	
	this->m_pView->Reback();
	
	if(& this->m_wndStatusBar)
	{
		this->m_wndStatusBar.SetPaneText(4,"X轴旋转角:-060度");
		this->m_wndStatusBar.SetPaneText(5,"Y轴旋转角:000度");
		this->m_wndStatusBar.SetPaneText(6,"Z轴旋转角:030度");
	}
	//pSlidCtrlX.SetPos(0); //设置滑动条位置
	//pSlidCtrlY.SetPos(0); //设置滑动条位置
	//pSlidCtrlZ.SetPos(0); //设置滑动条位置
	
	this->m_pCtrlWnd->SetXYZzoom();
	this->m_pView->Invalidate();
}


void CMainFrame::OnModinfo()
{
	// TODO: 在此添加命令处理程序代码
	if (::gbIsGetData)
	{
		AfxMessageBox( "未加载数据!");   
		return;
	}
	
	if (::gbDataIsEmpty)
	{
		AfxMessageBox( "数据为空!" );
		return;
	}

	this->m_pView->GetDocument()->GetModeinfo();
	
}


void CMainFrame::OnCalstandard()
{
	// TODO: 在此添加命令处理程序代码
	this->m_pView->GetDocument()->CalculateStandard();
}


void CMainFrame::OnRepairhole()
{
	// TODO: 在此添加命令处理程序代码
//	MessageBox("孔洞修补");
	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

//	this->m_OpenPicbtn.EnableWindow(FALSE);
//	this->m_GetDatabtn.EnableWindow(FALSE);
	::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);
	TimeCountStart;
	  
	this->m_pView->GetDocument()->PatchMesh();

	TimeCountEnd;

	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已填充数据");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);

}


void CMainFrame::OnPatchholes()
{
	// TODO: 在此添加命令处理程序代码
	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

	::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);
	TimeCountStart;

	this->m_pView->GetDocument()->FillHole();

	TimeCountEnd;

	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已修复");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);
	//	this->m_pCtrlWnd->SetXYZzoom();
	this->m_pView->Invalidate();
}
