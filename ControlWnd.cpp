// ControlWnd.cpp : implementation file
//

#include "stdafx.h"
#include "3DRCS.h"
#include "ControlWnd.h"
#include "ProjectView.h"

#include "PSOintro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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


/////////////////////////////////////////////////////////////////////////////
// CControlWnd
extern bool gbIsGetData;
extern bool gbDataIsEmpty;

IMPLEMENT_DYNCREATE(CControlWnd, CFormView)

extern float  xrof,yrof,zrof;
extern float	m_Prox,m_Proy,m_Proz; //缩放因子
extern bool   gbDataIsEmpty;
CControlWnd::CControlWnd()
	: CFormView(CControlWnd::IDD)
{
	//{{AFX_DATA_INIT(CControlWnd)
	m_nxZoom = 1.0f;
	m_nyZoom = 1.0f;
	m_nzZoom = 1.0f;
	m_nReduceAngleThreshold = 2.5f;
	m_XconZoom = 1.0f;
	m_YconZoom = 1.0f;
	m_ZconZoom = 1.0f;
	m_nReduceRate = 30.0f;
	m_nReduceDistanceThreshold = 0.1f;
	m_nPicCount=0;
	m_i=-1;
	m_zZoom=1.0f;
	m_yZoom=1.0f;
	m_xZoom=1.0f;
	m_bShowCoordinate = false;
	m_bGetPics=false;
	m_ConTypeString="MC:移动立方体法";
	m_bLighting = false;
	m_bDeleteRing = true;
	m_bShowReverse=false;
	m_SpinWnd=true;
	m_nSmoothRate = 0.1f;
	m_nSmoothNum = 5;
	//}}AFX_DATA_INIT

	m_nReduceAreaThreshold = 0.0f;
	m_nReduceVolumeThreshold = 0.0f;
}

CControlWnd::~CControlWnd()
{

}

void CControlWnd::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CControlWnd)
	DDX_Control(pDX, IDC_DISTANCE_THRESHOLD, m_ReduceDistanceThresholdEdit);
	DDX_Control(pDX, IDC_CONSTRUCTTYPE, m_ConstructType);
	DDX_Control(pDX, IDC_MCMTTHRESHOLD, m_MCMTthreshold);
	DDX_Control(pDX, IDC_ZOOMBACK, m_ZoomBack);
	DDX_Control(pDX, IDC_ZOOMDEL, m_ZoomDel);
	DDX_Control(pDX, IDC_ZOOMADD, m_ZoomAdd);
	DDX_Control(pDX, IDC_SHOWTPWND, m_ShowXpWnd);
	DDX_Control(pDX, IDC_RELOAD_DATA, m_ReloadDataWnd);
	DDX_Control(pDX, IDC_DIVIDE_MODE, m_DivideMode);
	DDX_Control(pDX, IDC_CLOSEWND, m_CloseWnd);
	DDX_Control(pDX, IDC_MODEINFO, m_ModeInfo);
	DDX_Control(pDX, ID_PICS_OPEN, m_OpenPicbtn);
	DDX_Control(pDX, IDC_SHOWPARA, m_ShowPara);
	DDX_Control(pDX, IDC_SECTIONX, pSlidCtrlX);
	DDX_Control(pDX, IDC_SECTIONY, pSlidCtrlY);
	DDX_Control(pDX, IDC_SECTIONZ, pSlidCtrlZ);
	DDX_Control(pDX, IDC_FACETNUM, m_FacetNum);
	DDX_Control(pDX, IDC_VERTEXNUM, m_PointNum);
	DDX_Control(pDX, IDC_EDGENUM, m_EdgeNum);
	DDX_Control(pDX, IDC_REDUCERATE, m_ReduceR);
	DDX_Control(pDX, IDC_CLEAR_DATA, m_Clearbtn);
	DDX_Control(pDX, IDC_SMOOTH_MESH, m_SmoothBtn);
	DDX_Control(pDX, IDC_MESHPATCH, m_MeshPatchbtn);
	DDX_Control(pDX, IDC_REDUCE_MESH, m_ReduceMeshbtn);
	DDX_Control(pDX, IDC_GETDATA, m_GetDatabtn);	
	DDX_Control(pDX, IDC_SAVEFILE, m_SaveFile);
	DDX_Control(pDX, IDC_ALLVALUE, m_PicList);
	DDX_Control(pDX, IDC_ANGLE_THRESHOLD, m_ReduceAngleThresholdEdit);
	DDX_Text(pDX, IDC_XZOOM, m_nxZoom);
	DDX_Text(pDX, IDC_YZOOM, m_nyZoom);
	DDX_Text(pDX, IDC_ZZOOM, m_nzZoom);
	DDX_Text(pDX, IDC_ANGLE_THRESHOLD, m_nReduceAngleThreshold);
	DDX_Control(pDX, IDC_THRESHOLDEDIT, m_ThresholdEdit);
	DDX_Text(pDX, IDC_XCONZOOM, m_XconZoom);
	DDX_Text(pDX, IDC_YCONZOOM, m_YconZoom);
	DDX_Text(pDX, IDC_ZCONZOOM, m_ZconZoom);
	DDX_Text(pDX, IDC_REDUCE_RATE_EDIT, m_nReduceRate);
	DDX_Text(pDX, IDC_DISTANCE_THRESHOLD, m_nReduceDistanceThreshold);
	DDX_Text(pDX, IDC_SMOOTH_RATE_EDIT, m_nSmoothRate);
	DDX_Text(pDX, IDC_SMOOTH_NUM, m_nSmoothNum);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_AERA_THRESHOLD, m_nReduceAreaThreshold);
	DDX_Text(pDX, IDC_VOLUME_THRESHOLD, m_nReduceVolumeThreshold);
	DDX_Control(pDX, IDC_AERA_THRESHOLD, m_ReduceAreaThresholdEdit);
	DDX_Control(pDX, IDC_VOLUME_THRESHOLD, m_ReduceVolumeThresholdEdit);
}


BEGIN_MESSAGE_MAP(CControlWnd, CFormView)
	//{{AFX_MSG_MAP(CControlWnd)
	ON_LBN_DBLCLK(IDC_ALLVALUE, OnDblclkAllvalue)
	ON_BN_CLICKED(IDC_MESHPATCH, OnMeshPatch)
	ON_BN_CLICKED(IDC_RADIOTURN, OnRadioturn)
	ON_BN_CLICKED(IDC_RADIOSELECT, OnRadioselect)
	ON_BN_CLICKED(IDC_GETDATA, OnGetdata)
	ON_BN_CLICKED(IDC_CLEAR_DATA, OnClearData)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CLOSEWND, OnCloseWnd)
	ON_BN_CLICKED(IDC_CHECKCOOLDINATE, OnCheckcooldinate)
	ON_BN_CLICKED(IDC_RADIOTURNX, OnRadioturnx)
	ON_BN_CLICKED(IDC_RADIOTURNY, OnRadioturny)
	ON_BN_CLICKED(IDC_RADIOTURNZ, OnRadioturnz)
	ON_BN_CLICKED(IDC_SHOWMEDIUM, OnShowMedium)
	ON_BN_CLICKED(IDC_ZOOMADD, OnZoomAdd)
	ON_BN_CLICKED(IDC_ZOOMDEL, OnZoomdel)
	ON_BN_CLICKED(IDC_ZOOMBACK, OnZoomback)
	ON_BN_CLICKED(IDC_CHECKLIGHTING, OnChecklighting)
	ON_BN_CLICKED(IDC_SMOOTH_MESH, OnSmoothMesh)
	ON_BN_CLICKED(IDC_FACETREVERSE, OnCheckFacetreverse)
	ON_BN_CLICKED(IDC_SAVEFILE, OnSavefile)
	ON_EN_UPDATE(IDC_ZZOOM, OnUpdateZzoom)
	ON_EN_UPDATE(IDC_YZOOM, OnUpdateYzoom)
	ON_EN_UPDATE(IDC_XZOOM, OnUpdateXzoom)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_VERTEXES, OnVertexes)
	ON_BN_CLICKED(IDC_FACETS, OnFacets)
	ON_BN_CLICKED(IDC_EDGES, OnEdges)
	ON_BN_CLICKED(IDC_REDUCE_MESH, OnReducemesh)
	ON_BN_CLICKED(IDC_RADIOTRANSLATE, OnRadiotranslate)
	ON_BN_CLICKED(IDC_RELOAD_DATA, OnReloadData)
	ON_BN_CLICKED(IDC_DIVIDE_MODE, OnDividemode)
	ON_BN_CLICKED(IDC_SHOWPARA, OnShowpara)
	ON_BN_CLICKED(IDC_MODEINFO, OnModeinfo)
	ON_BN_CLICKED(ID_PICS_OPEN, OnOpenPics)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_MCMTTHRESHOLD, OnReleasedcaptureMcmtthreshold)
	ON_EN_CHANGE(IDC_ZCONZOOM, OnChangeZconzoom)
	ON_EN_CHANGE(IDC_XCONZOOM, OnChangeXconzoom)
	ON_EN_CHANGE(IDC_YCONZOOM, OnChangeYconzoom)
	ON_EN_CHANGE(IDC_THRESHOLDEDIT, OnChangeThresholdedit)
//	ON_BN_CLICKED(IDC_PATCHMESH, OnPatchmesh)
	ON_BN_CLICKED(IDC_CHECKRING, OnCheckring)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINWOND, OnDeltaposSpinwond)
	ON_CBN_EDITCHANGE(IDC_CONSTRUCTTYPE, OnEditchangeConstructtype)
	ON_BN_CLICKED(IDC_REDUCE_MESH2, OnReduceMesh2)
	ON_BN_CLICKED(IDC_CalStandard, OnCalStandard)

	//}}AFX_MSG_MAP
	ON_MESSAGE(MESSAGE_ENABLEBTN,EnableButton)
	ON_MESSAGE(MESSAGE_DISABLEBTN, DisableButton)
	ON_MESSAGE(MESSAGE_GETDATAFINISHED,GetData)
	ON_BN_CLICKED(IDC_REDUCE_MESH3, &CControlWnd::OnBnClickedReduceMesh3)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CControlWnd diagnostics

#ifdef _DEBUG
void CControlWnd::AssertValid() const
{
	CFormView::AssertValid();
}

void CControlWnd::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CControlWnd message handlers

/******************************************************************************************************************************
 * 
 * 函数名称：
 *   FillListBox()
 *
 * 参数:
 *   无         -              
 *
 * 说明:
 *       将图像文件罗列于图列表框。
 * 
 ******************************************************************************************************************************/
/*void CControlWnd::FillListBox()
{
	//首先清空列表框
	m_nPicCount = m_PicList.GetCount();
	for (int n=0; n<m_nPicCount; n++)
		m_PicList.DeleteString(0);

	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

	int m_place;  //文件名中最后一个'\'的位置

	//从文件路径中，获取图片文件的文件名
	m_nPicCount=this->m_pFr->m_pView->GetDocument()->m_nFileNum;
	CString FileName,tempt;
	for(int i=0;i<m_nPicCount;i++)
	{
		FileName.Format("%.3d",i+1); 
		tempt=this->m_pFr->m_pView->GetDocument()->m_FilePathArray.GetAt(i);
		m_place=tempt.ReverseFind('\\');
		tempt.Delete(0,m_place+1);
		FileName=FileName+":"+tempt;
		this->m_PicList.InsertString(i,FileName);
	}

	if(m_nPicCount>0)
	{
		CString Wtext = this->m_pFr->m_pView->GetDocument()->m_FilePathArray.GetAt(m_nPicCount-1);
		Wtext ="三维重建系统（3DRCS）: 文件->" + Wtext.Left(m_place);
		AfxGetMainWnd()->SetWindowText(Wtext);
	}

}*/

void CControlWnd::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();	

	m_nMapMode=-1;    //去除对话框滚动条；

	this->m_pFr = (CMainFrame*)AfxGetApp()->m_pMainWnd;

	m_bGetPics=false;
	this->m_pFr->m_pView->m_MouseAction=0;
	this->m_pFr->m_pView->m_ShowType=2;
	this->m_bShowCoordinate=false;
	this->m_bLighting=false;
	this->m_bDeleteRing=true;
	this->m_bShowReverse=false;
	this->m_ReduceAngleThresholdEdit.SetWindowText("2.5");
	this->m_ReduceDistanceThresholdEdit.SetWindowText("0.1");
	this->m_ReduceAreaThresholdEdit.SetWindowText("0.1");
	this->m_ReduceVolumeThresholdEdit.SetWindowText("0.1");



	//this->FillListBox();	
	CButton* btn = (CButton*)GetDlgItem(IDC_RADIOTURN);
	btn->SetCheck(1);

	btn = (CButton*)GetDlgItem(IDC_RADIOTRANSLATE);
	btn->SetCheck(0);

	btn = (CButton*)GetDlgItem(IDC_RADIOSELECT);
	btn->SetCheck(0);
	
	btn = (CButton*)GetDlgItem(IDC_FACETS);
	btn->SetCheck(1);

	btn = (CButton*)GetDlgItem(IDC_EDGES);
	btn->SetCheck(0);

	btn = (CButton*)GetDlgItem(IDC_VERTEXES);
	btn->SetCheck(0);

	btn = (CButton*)GetDlgItem(IDC_CHECKCOOLDINATE);
	btn->SetCheck(0);

	btn = (CButton*)GetDlgItem(IDC_CHECKRING);
	btn->SetCheck(1);

	btn = (CButton*)GetDlgItem(IDC_CHECKLIGHTING);
	btn->SetCheck(0);

	btn = (CButton*)GetDlgItem(IDC_FACETREVERSE);
	btn->SetCheck(0);

	btn = (CButton*)GetDlgItem(IDC_RADIOTURNX);
	btn->SetCheck(1);

	btn = (CButton*)GetDlgItem(IDC_RADIOTURNY);
	btn->SetCheck(0);

	btn = (CButton*)GetDlgItem(IDC_RADIOTURNZ);
	btn->SetCheck(0);
	

	 /*MoveWindow(0,0,0,0);
ShowWindow(SW_HIDE);
ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);*/
	/*this->m_MeshPatchbtn.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_ReduceMeshbtn.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_GetDatabtn.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_SaveFile.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_SmoothBtn.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_Clearbtn.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_ReloadDataWnd.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_CloseWnd.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_ModeInfo.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_ShowPara.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_DivideMode.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_OpenPicbtn.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_ShowXpWnd.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_ZoomAdd.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_ZoomDel.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	this->m_ZoomBack.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );*/

	this->m_MeshPatchbtn.EnableWindow(FALSE);
	this->m_ReduceMeshbtn.EnableWindow(FALSE);
	this->m_SaveFile.EnableWindow(FALSE);
	this->m_SmoothBtn.EnableWindow(FALSE);
	this->m_Clearbtn.EnableWindow(FALSE);
	this->m_ModeInfo.EnableWindow(FALSE);
	this->m_ReloadDataWnd.EnableWindow(FALSE);
	this->m_DivideMode.EnableWindow(FALSE);

	this->m_ReduceAngleThresholdEdit.EnableWindow(FALSE);
	this->m_ReduceDistanceThresholdEdit.EnableWindow(FALSE);

	GetDlgItem(IDC_XCONZOOM)->EnableWindow(FALSE);
	GetDlgItem(IDC_YCONZOOM)->EnableWindow(FALSE);
	GetDlgItem(IDC_ZCONZOOM)->EnableWindow(FALSE);
	GetDlgItem(IDC_CONSTRUCTTYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_THRESHOLDEDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_MCMTTHRESHOLD)->EnableWindow(FALSE);

	m_ConstructType.AddString("MC:移动立方体法");
	m_ConstructType.AddString("MT:移动四面体法");
	m_ConstructType.SetCurSel(0);


	pSlidCtrlX.SetRange(-100,100,TRUE);//设置滑动条范围
	pSlidCtrlX.SetPos(0);//设置滑动条位置
	pSlidCtrlX.SetTicFreq(25);
	pSlidCtrlX.SetPageSize(5);         //设置在按一下右或左箭头时滑块的移动量

	pSlidCtrlY.SetRange(-100,100,TRUE);//设置滑动条范围
	pSlidCtrlY.SetPos(0);             //设置滑动条位置
	pSlidCtrlY.SetTicFreq(25);
	pSlidCtrlY.SetPageSize(5);         //设置在按一下右或左箭头时滑块的移动量

	pSlidCtrlZ.SetRange(-100,100,TRUE);//设置滑动条范围
	pSlidCtrlZ.SetPos(0);//设置滑动条位置
	pSlidCtrlZ.SetTicFreq(25);
	pSlidCtrlZ.SetPageSize(5);         //设置在按一下右或左箭头时滑块的移动量

	m_MCMTthreshold.SetRange(0,255,TRUE);//设置滑动条范围;
	m_MCMTthreshold.SetPos(64);         //设置滑动条位置
	m_MCMTthreshold.SetTicFreq(32);
	m_MCMTthreshold.SetPageSize(8);     //设置在按一下右或左箭头时滑块的移动量

	this->m_ThresholdEdit.SetWindowText("64");
	this->m_PointNum.SetWindowText("0");
	this->m_EdgeNum.SetWindowText("0");
	this->m_FacetNum.SetWindowText("0");
	this->m_ReduceR.SetWindowText("0");

}

 LRESULT CControlWnd::EnableButton(WPARAM wParam,LPARAM lParam)
{
	this->m_OpenPicbtn.EnableWindow(TRUE);
	this->m_GetDatabtn.EnableWindow(TRUE);
	this->m_Clearbtn.EnableWindow(TRUE);
	this->m_ReloadDataWnd.EnableWindow(TRUE);
	this->m_SaveFile.EnableWindow(TRUE);
	
	this->m_MeshPatchbtn.EnableWindow(TRUE);
	this->m_SmoothBtn.EnableWindow(TRUE);
	this->m_ReduceMeshbtn.EnableWindow(TRUE);
	this->m_DivideMode.EnableWindow(TRUE);

	this->m_ModeInfo.EnableWindow(TRUE);

	this->m_ReduceAngleThresholdEdit.EnableWindow(TRUE);
	this->m_ReduceDistanceThresholdEdit.EnableWindow(TRUE);
	return TRUE;
}

 LRESULT CControlWnd::DisableButton(WPARAM wParam,LPARAM lParam)
{
	this->m_Clearbtn.EnableWindow(FALSE);
	this->m_ReloadDataWnd.EnableWindow(FALSE);
	this->m_SaveFile.EnableWindow(FALSE);

	this->m_MeshPatchbtn.EnableWindow(FALSE);
	this->m_SmoothBtn.EnableWindow(FALSE);
	this->m_ReduceMeshbtn.EnableWindow(FALSE);
	this->m_DivideMode.EnableWindow(FALSE);

	this->m_ModeInfo.EnableWindow(FALSE);
	
	this->m_ReduceAngleThresholdEdit.EnableWindow(FALSE);
	this->m_ReduceDistanceThresholdEdit.EnableWindow(FALSE);
	return TRUE;
}

void CControlWnd::DisableControlWhileConstruct()
{
	this->m_OpenPicbtn.EnableWindow(FALSE);
	this->m_GetDatabtn.EnableWindow(FALSE);
	this->m_Clearbtn.EnableWindow(FALSE);
	this->m_ReloadDataWnd.EnableWindow(FALSE);
	GetDlgItem(IDC_XCONZOOM)->EnableWindow(FALSE);
	GetDlgItem(IDC_YCONZOOM)->EnableWindow(FALSE);
	GetDlgItem(IDC_ZCONZOOM)->EnableWindow(FALSE);
	GetDlgItem(IDC_CONSTRUCTTYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_THRESHOLDEDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_MCMTTHRESHOLD)->EnableWindow(FALSE);
}

void CControlWnd::EnablControlWhileConstruct()
{
	this->m_OpenPicbtn.EnableWindow(TRUE);
	this->m_GetDatabtn.EnableWindow(TRUE);
	this->m_Clearbtn.EnableWindow(TRUE);
	this->m_ReloadDataWnd.EnableWindow(TRUE);
	GetDlgItem(IDC_XCONZOOM)->EnableWindow(TRUE);
	GetDlgItem(IDC_YCONZOOM)->EnableWindow(TRUE);
	GetDlgItem(IDC_ZCONZOOM)->EnableWindow(TRUE);
	GetDlgItem(IDC_CONSTRUCTTYPE)->EnableWindow(TRUE);
	GetDlgItem(IDC_THRESHOLDEDIT)->EnableWindow(TRUE);
	GetDlgItem(IDC_MCMTTHRESHOLD)->EnableWindow(TRUE);
}

void CControlWnd::OnDblclkAllvalue() 
{
	int index = this->m_PicList.GetCurSel();
	CString name;
	this->m_PicList.GetText(index,name);

	int i = name.Find(":");
	name=name.Left(i);
	m_i=atoi(name);
	this->m_pFr->m_pProView->m_bShowBmp=true;
	m_PicList.SetCurSel(m_i-1);
	this->m_pFr->m_pProView->Invalidate();
}


void CControlWnd::OnMeshPatch() 
{
	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

	this->m_OpenPicbtn.EnableWindow(FALSE);
	this->m_GetDatabtn.EnableWindow(FALSE);
	::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);

	this->m_pFr->m_pView->GetDocument()->PatchMesh();

	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已填充数据");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);
}

void CControlWnd::OnRadioturn() 
{
	this->m_pFr->m_pView->m_MouseAction = 0;
	this->m_pFr->m_pView->Invalidate();
}

void CControlWnd::OnRadioselect() 
{
	this->m_pFr->m_pView->m_MouseAction = 2;
	this->m_pFr->m_pView->Invalidate();
}

void CControlWnd::OnGetdata() 
{
	if(!m_bGetPics)
	{
		static char szFilter[]="STL ASCII(*.ast),STL Binary(*.stl)|*.ast;*.stl|PLY ASCII(*.ply),PLY Binary(*.ply)|*.ply|所有文件All Files(*.*)|*.*||";	
		
		//文件打开对话框
		CFileDialog StlOpenDlg(TRUE,_T("AST"),_T("*.ast;*.stl;*.bst"),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,szFilter);
		int structsize=88; 
		DWORD dwVersion,dwWindowsMajorVersion,dwWindowsMinorVersion;
		//检测目前的操作系统，GetVersion具体用法详见MSDN 
		dwVersion = GetVersion(); 
		dwWindowsMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
		dwWindowsMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
		
		// 如果运行的操作系统是Windows NT/2000 
		if (dwVersion < 0x80000000)  
			structsize =88;//显示新的文件对话框 
		else 
			//运行的操作系统Windows 95/98  
			structsize =76;//显示老的文件对话框 
		StlOpenDlg.m_ofn.lStructSize=structsize;
		
		if (IDCANCEL == StlOpenDlg.DoModal())
			return;
		
		this->Clear();   

		this->m_pFr->m_pView->GetDocument()->m_FileName = StlOpenDlg.GetPathName();
		
		this->m_OpenPicbtn.EnableWindow(FALSE);
		this->m_GetDatabtn.EnableWindow(FALSE);
		::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0); //将控制板按键设置为不可用
		
		AfxBeginThread(C3DRCSDoc::ReadFile,this->m_pFr->m_pView->GetDocument());
	}
	else
	{
		::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0); //将控制板按键设置为不可用
		if(m_bDeleteRing)
		this->m_pFr->m_pView->GetDocument()->DeleteRing();
		m_ConstructType.GetLBText(m_ConstructType.GetCurSel(),m_ConTypeString);
		if(m_ConTypeString=="MC:移动立方体法")
			AfxBeginThread(C3DRCSDoc::MarchingCubes,this->m_pFr->m_pView->GetDocument());
		else
			AfxBeginThread(C3DRCSDoc::MarchingTetrahedra,this->m_pFr->m_pView->GetDocument());
	}

}

void CControlWnd::OnClearData() 
{
	if(::gbDataIsEmpty)
	{
		MessageBox("当前未加载数据!","提示信息",MB_ICONINFORMATION);
		return;
	}
	Clear();
}



HBRUSH CControlWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);

	if ( nCtlColor == CTLCOLOR_EDIT
		||nCtlColor == CTLCOLOR_LISTBOX 
		|| nCtlColor == CTLCOLOR_STATIC )
	{
		//设置对话框字体颜色		
		pDC->SetTextColor(RGB(10,10,100));
		pDC->SetBkMode(1);
	}
	return hbr;
}

LRESULT CControlWnd::GetData(WPARAM wParam,LPARAM lParam)
{
	float reducerate = this->m_pFr->m_pView->GetDocument()->m_ReduceRate;

	CString str;

	str.Format("%ld",m_VertexNumber);
	this->m_PointNum.SetWindowText(str);
	
	str.Format("%ld",m_EdgeNumber);
	this->m_EdgeNum.SetWindowText(str);
	
	str.Format("%ld",m_FacetNumber);
	this->m_FacetNum.SetWindowText(str);

	str.Format("%0.1f%%",reducerate);
	this->m_ReduceR.SetWindowText(str);
	
	this->m_pFr->InvalidateAllWnd();
	return TRUE;
}



void CControlWnd::OnUpdateZzoom()  
{
	CString   str; 
	GetDlgItem(IDC_ZZOOM)->GetWindowText(str);
	m_Proz = m_nzZoom = (float)atof(str);
	this->m_pFr->m_pView->Invalidate();
}


void CControlWnd::OnUpdateYzoom()  
{
	CString   str; 
	GetDlgItem(IDC_YZOOM)->GetWindowText(str);
	m_Proy = m_nyZoom = (float)atof(str);
	this->m_pFr->m_pView->Invalidate();
}


void CControlWnd::OnUpdateXzoom()  
{
	CString   str; 
	GetDlgItem(IDC_XZOOM)->GetWindowText(str);
	m_Prox = m_nxZoom = (float)atof(str);
	this->m_pFr->m_pView->Invalidate();
}

void CControlWnd::OpenSTL()        {	this->OnGetdata();}
void CControlWnd::OnRadioturnx()   {	this->m_pFr->m_pView->nTurnType = 0;}
void CControlWnd::OnRadioturny()   {	this->m_pFr->m_pView->nTurnType = 1;}
void CControlWnd::OnRadioturnz()   {	this->m_pFr->m_pView->nTurnType = 2;}
void CControlWnd::OnCloseWnd()     {	this->m_pFr->SendMessage(WM_CLOSE);  }
void CControlWnd::OnClearData1()   {	this->OnClearData();}


void CControlWnd::OnSmoothMesh()     
{
	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

	this->m_OpenPicbtn.EnableWindow(FALSE);
	this->m_GetDatabtn.EnableWindow(FALSE);
	::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);

	CString str;
	GetDlgItem(IDC_SMOOTH_RATE_EDIT)->GetWindowText(str);
	m_nSmoothRate = (float)atof(str);

	GetDlgItem(IDC_SMOOTH_NUM)->GetWindowText(str);
	m_nSmoothNum = (float)atof(str);

	this->m_pFr->m_pView->GetDocument()->SmoothMesh();
	
	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已优化模型");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);
}

void CControlWnd::OnSavefile()     
{
	AfxBeginThread(C3DRCSDoc::SaveSTL,this->m_pFr->m_pView->GetDocument());
}

void CControlWnd::OnShowMedium() 
{	
	m_Prox = m_nxZoom=m_xZoom;
	m_Proy = m_nyZoom=m_xZoom;
	m_Proz = m_nzZoom=m_xZoom;

	if((this->m_pFr->m_pView->newx1!=-100.0f)||(this->m_pFr->m_pView->newy1!=-100.0f))
	{
		this->m_pFr->m_pView->newx1=-100.0f;
		this->m_pFr->m_pView->newx2=100.0f;
		this->m_pFr->m_pView->newy1=-100.0f;
		this->m_pFr->m_pView->newy2=100.0f;
		this->m_pFr->m_pView->SetViewPort();
	}

	this->SetXYZzoom();

	this->m_pFr->m_pView->Invalidate();
}

void CControlWnd::OnZoomAdd() 
{
	float x,y,z;
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

	x=m_Prox;	y=m_Proy;	z=m_Proz;

	CString   str; 
	str.Format("%0.2f",x);
	GetDlgItem(IDC_XZOOM)->SetWindowText(str);

	str.Format("%0.2f",y);
	GetDlgItem(IDC_YZOOM)->SetWindowText(str);

	str.Format("%0.2f",z);
	GetDlgItem(IDC_ZZOOM)->SetWindowText(str);

	this->m_pFr->m_pView->Invalidate();
}

void CControlWnd::OnZoomdel() 
{
	float x,y,z;

	m_Prox=m_Prox/1.25f;
	m_Proy=m_Proy/1.25f;
	m_Proz=m_Proz/1.25f;

	x=m_Prox;	y=m_Proy;	z=m_Proz;

	CString   str; 
	str.Format("%0.2f",x);
	GetDlgItem(IDC_XZOOM)->SetWindowText(str);

	str.Format("%0.2f",y);
	GetDlgItem(IDC_YZOOM)->SetWindowText(str);

	str.Format("%0.2f",z);
	GetDlgItem(IDC_ZZOOM)->SetWindowText(str);
	
}

void CControlWnd::OnZoomback() 
{	
	xrof = -60.0f; yrof = 0.0f; zrof = 30.0f;
	m_nxZoom = m_nyZoom = m_nzZoom = m_xZoom;
	m_Prox = m_Proy = m_Proz =m_xZoom;
	
	this->m_pFr->m_pView->Reback();
	
	if(& this->m_pFr->m_wndStatusBar)
	{
		this->m_pFr->m_wndStatusBar.SetPaneText(4,"X轴旋转角:-060度");
		this->m_pFr->m_wndStatusBar.SetPaneText(5,"Y轴旋转角:000度");
		this->m_pFr->m_wndStatusBar.SetPaneText(6,"Z轴旋转角:030度");
	}
	pSlidCtrlX.SetPos(0); //设置滑动条位置
	pSlidCtrlY.SetPos(0); //设置滑动条位置
	pSlidCtrlZ.SetPos(0); //设置滑动条位置
	
	this->SetXYZzoom();
	this->m_pFr->Invalidate();

}

void CControlWnd::OnChecklighting() 
{
	this->m_bLighting = !this->m_bLighting; 
	this->m_pFr->m_pView->Invalidate();
}


void CControlWnd::OnCheckFacetreverse() 
{
	this->m_bShowReverse = !this->m_bShowReverse; 
	this->m_pFr->m_pView->Invalidate();
}


void CControlWnd::OnCheckcooldinate() 
{
	this->m_bShowCoordinate = !(this->m_bShowCoordinate);
	this->m_pFr->m_pView->Invalidate();
}


void CControlWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	this->m_pFr->m_pView->m_nSliderCurX=pSlidCtrlX.GetPos();//取得当前位置值
	this->m_pFr->m_pView->m_nSliderCurY=pSlidCtrlY.GetPos();//取得当前位置值
	this->m_pFr->m_pView->m_nSliderCurZ=pSlidCtrlZ.GetPos();//取得当前位置值

	CString str;
	str.Format("%ld",m_MCMTthreshold.GetPos());
	this->m_ThresholdEdit.SetWindowText(str);

	this->m_pFr->m_pView->Invalidate();
	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CControlWnd::OnVertexes() 
{
	this->m_pFr->m_pView->m_ShowType = 0;
	this->m_pFr->m_pView->Invalidate();
}

void CControlWnd::OnEdges() 
{
	this->m_pFr->m_pView->m_ShowType = 1;
	this->m_pFr->m_pView->Invalidate();
}

void CControlWnd::OnFacets() 
{
	this->m_pFr->m_pView->m_ShowType = 2;
	this->m_pFr->m_pView->Invalidate();
}


void CControlWnd::OnReducemesh() 
{
	TimeCountStart;

	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

	this->m_OpenPicbtn.EnableWindow(FALSE);
	this->m_GetDatabtn.EnableWindow(FALSE);
	::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);

	CString str;
	GetDlgItem(IDC_ANGLE_THRESHOLD)->GetWindowText(str);
	m_nReduceAngleThreshold = (float)atof(str);

	GetDlgItem(IDC_DISTANCE_THRESHOLD)->GetWindowText(str);
	m_nReduceDistanceThreshold=(float)atof(str);

	GetDlgItem(IDC_REDUCE_RATE_EDIT)->GetWindowText(str);
	m_nReduceRate=(float)atof(str);
	this->m_pFr->m_pView->GetDocument()->ReduceMesh(); 

	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已精简数据");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);

	TimeCountEnd;
}

void CControlWnd::OnRadiotranslate() 
{
	this->m_pFr->m_pView->m_MouseAction = 1;
	this->m_pFr->m_pView->Invalidate();
}

void CControlWnd::OnReloadData() 
{
	if(!m_bGetPics)
	{
		this->Clear();
		AfxBeginThread(C3DRCSDoc::ReadFile,this->m_pFr->m_pView->GetDocument());
	}
	else
	{
		::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);
		if(!m_Vertex.empty())	m_Vertex.clear();
		if(!m_Facet.empty())	m_Facet.clear();
		if(!m_Edge.empty())		m_Edge.clear();
		m_VertexNumber=m_EdgeNumber=m_FacetNumber=0;
		this->m_pFr->m_pView->GetDocument()->m_ReduceRate=0;

		this->m_PointNum.SetWindowText("0");
		this->m_EdgeNum.SetWindowText("0");
		this->m_FacetNum.SetWindowText("0");
		this->m_ReduceR.SetWindowText("0");
		EnablControlWhileConstruct();
		this->m_pFr->InvalidateAllWnd();
	}
}

void CControlWnd::OnDividemode() 
{
	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

	this->m_OpenPicbtn.EnableWindow(FALSE);
	this->m_GetDatabtn.EnableWindow(FALSE);
	::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);

	this->m_pFr->m_pView->GetDocument()->FillHole();

	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已填充数据");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);
}

void CControlWnd::OnShowpara() 
{
	this->m_pFr->m_pView->GetDocument()->SetShowPara();		
}

void CControlWnd::OnModeinfo() 
{
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

	this->m_pFr->m_pView->GetDocument()->GetModeinfo();
		
}

void CControlWnd::OnOpenPics() 
{
	this->m_pFr->m_pView->GetDocument()->OpenPics();

	if(m_bGetPics)
	{
		GetDlgItem(IDC_GETDATA)->SetWindowText("重建模型");
	}
	
}

void CControlWnd::SetXYZzoom()
{	
	m_Prox = m_nxZoom=m_xZoom;
	m_Proy = m_nyZoom=m_yZoom;
	m_Proz = m_nzZoom=m_zZoom;

	CString   str; 
	str.Format("%0.2f",m_xZoom);
	GetDlgItem(IDC_XZOOM)->SetWindowText(str);

	str.Format("%0.2f",m_yZoom);
	GetDlgItem(IDC_YZOOM)->SetWindowText(str);

	str.Format("%0.2f",m_zZoom);
	GetDlgItem(IDC_ZZOOM)->SetWindowText(str);

}

void CControlWnd::Clear()
{
	::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);

	this->OnZoomback();
	this->m_pFr->m_pView->KillTime();
	::gbDataIsEmpty = true;
	x_glMax = y_glMax = z_glMax =-1024.0;
	x_glMid = y_glMid = z_glMid =0.0;
	x_glMin = y_glMin = z_glMin =1024.0;
	m_Prox=m_Proy=m_Proz=1.0;
	m_nxZoom=m_nyZoom=m_nzZoom=1.0;
	m_VertexNumber=m_EdgeNumber=m_FacetNumber=0;


	
	GetDlgItem(IDC_GETDATA)->SetWindowText("加载模型");

	OnInitialUpdate();

	this->m_pFr->m_pView->GetDocument()->m_ReduceRate=0;

	this->m_pFr->m_pView->GetDocument()->Clear();
	this->m_pFr->InvalidateAllWnd();

	/*初始化光照，反向，坐标*/
	this->m_pFr->m_bShowLight=true;
	this->m_pFr->m_bFacetreverse=false;
	this->m_pFr->m_bShowcoordinate=false;
	/*初始化鼠标动作，点显示，滚轮动作*/
	this->m_pFr->m_pView->m_bShowEdge=false;
	 this->m_pFr->m_pView->m_bShowFacet=true;
	this->m_pFr->m_pView->m_bShowVertex=false;

	this->m_pFr->m_pView->m_bTurnx=true;
	this->m_pFr->m_pView->m_bTurny=false;
	this->m_pFr->m_pView->m_bTurnz=false;
	this->m_pFr->m_pView->m_bTranslate=false;
	this->m_pFr->m_pView->m_bTurn=true;
	this->m_pFr->m_pView->m_bZoomout=false;


	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->未加载数据");

	CString str="数据已清空！";
	this->m_pFr->m_wndStatusBar.SetWindowText(str);

}

void CControlWnd::OnReleasedcaptureMcmtthreshold(NMHDR* pNMHDR, LRESULT* pResult) 
{
	this->m_pFr->m_pView->GetDocument()->m_TargetValue=m_MCMTthreshold.GetPos();
	*pResult = 0;
}

void CControlWnd::OnChangeXconzoom() 
{
	CString   str; 
	GetDlgItem(IDC_XCONZOOM)->GetWindowText(str);
	m_XconZoom = (float)atof(str);	
}

void CControlWnd::OnChangeYconzoom() 
{
	CString   str; 
	GetDlgItem(IDC_YCONZOOM)->GetWindowText(str);
	m_YconZoom = (float)atof(str);
}

void CControlWnd::OnChangeZconzoom() 
{
	CString   str; 
	GetDlgItem(IDC_ZCONZOOM)->GetWindowText(str);
	m_ZconZoom = (float)atof(str);
}

void CControlWnd::OnChangeThresholdedit() 
{	
	CString str;
	this->m_ThresholdEdit.GetWindowText(str);
	int value=atoi(str);
	this->m_pFr->m_pView->GetDocument()->m_TargetValue=value;
	this->m_MCMTthreshold.SetPos(value);	
}

void CControlWnd::OnCheckring() 
{
	this->m_bDeleteRing=!this->m_bDeleteRing;
	
}

void CControlWnd::OnDeltaposSpinwond(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	CRect rc;
	GetWindowRect(&rc);

	if(m_SpinWnd)
	{
		m_SpinWnd=!m_SpinWnd;
		MoveWindow(0-rc.Width(),3, rc.Width()+rc.Width(),rc.Height(), TRUE);
	}
	else
	{
		m_SpinWnd=!m_SpinWnd;
		MoveWindow(0,3, rc.Width()/2,rc.Height(),TRUE);
	}

/*	
	if(pNMUpDown->iDelta==1)
	{
		m_SpinWnd=!m_SpinWnd;
		MoveWindow(0-rc.Width(),3, rc.Width()+rc.Width(),rc.Height(), TRUE);
	}
	else
	{
		if(pNMUpDown->iDelta==-1)
		{
			m_SpinWnd=!m_SpinWnd;
			MoveWindow(0,3, rc.Width(),rc.Height(),TRUE);
		}
	}
*/	
	*pResult = 0;
/*
	CRect rc;
	GetWindowRect(&rc);

	if(m_SpinWnd)
	{
		m_SpinWnd=!m_SpinWnd;
		MoveWindow(-10, 3, rc.Width()+10, rc.Height(), TRUE);
	}
	else
	{	
		m_SpinWnd=!m_SpinWnd;
		MoveWindow(10, 3, rc.Width()-10, rc.Height(),TRUE);
	}
*/
	
}



void CControlWnd::OnEditchangeConstructtype() 
{
	// TODO: Add your control notification handler code here
	
}

// void CControlWnd::OnStandradMesh() 
// {
// 	// TODO: Add your control notification handler code here
// 	TimeCountStart;
// 	
// 	if (::gbIsGetData)
// 		return;
// 	
// 	if (::gbDataIsEmpty)
// 		return;
// 	
// 	this->m_OpenPicbtn.EnableWindow(FALSE);
// 	this->m_GetDatabtn.EnableWindow(FALSE);
// 	::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);
// 	
// 	CString str;
// 	GetDlgItem(IDC_ANGLE_THRESHOLD)->GetWindowText(str);
// 	m_nReduceAngleThreshold = (float)atof(str);
// 	
// 	GetDlgItem(IDC_DISTANCE_THRESHOLD)->GetWindowText(str);
// 	m_nReduceDistanceThreshold=(float)atof(str);
// 	
// 	GetDlgItem(IDC_REDUCE_RATE_EDIT)->GetWindowText(str);
// 	m_nReduceRate=(float)atof(str);
// 
// 	this->m_pFr->m_pView->GetDocument()->ReduceMesh2(); 
// 	
// 	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已精简数据");
// 	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
// 	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);
// 	
// 	TimeCountEnd;
// 
// }

void CControlWnd::OnReduceMesh2() 
{
	// TODO: Add your control notification handler code here
	TimeCountStart;
	
	if (::gbIsGetData)
		return;
	
	if (::gbDataIsEmpty)
		return;
	
	this->m_OpenPicbtn.EnableWindow(FALSE);
	this->m_GetDatabtn.EnableWindow(FALSE);
	::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);
	
	CString str;
	GetDlgItem(IDC_ANGLE_THRESHOLD)->GetWindowText(str);
	m_nReduceAngleThreshold = (float)atof(str);
	
	GetDlgItem(IDC_DISTANCE_THRESHOLD)->GetWindowText(str);
	m_nReduceDistanceThreshold=(float)atof(str);
	
	GetDlgItem(IDC_REDUCE_RATE_EDIT)->GetWindowText(str);
	m_nReduceRate=(float)atof(str);
	
	this->m_pFr->m_pView->GetDocument()->ReduceMesh2(); 
	
	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已精简数据");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);
	TimeCountEnd;
	
}

void CControlWnd::OnCalStandard() 
{
	// TODO: Add your control notification handler code here
	this->m_pFr->m_pView->GetDocument()->CalculateStandard(); 
	
	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->三角形质量估计完成");

	
}




void CControlWnd::OnBnClickedReduceMesh3()
{
	// TODO: 在此添加控件通知处理程序代码
	if (::gbIsGetData)
		return;
	
	if (::gbDataIsEmpty)
		return;
	
	this->m_OpenPicbtn.EnableWindow(FALSE);
	this->m_GetDatabtn.EnableWindow(FALSE);
	::SendMessage(this->m_hWnd,MESSAGE_DISABLEBTN,0,0);
	CString str;
	GetDlgItem(IDC_ANGLE_THRESHOLD)->GetWindowText(str);
	m_nReduceAngleThreshold = (float)atof(str);
	
	GetDlgItem(IDC_AERA_THRESHOLD)->GetWindowText(str);
	m_nReduceAreaThreshold=(float)atof(str);
	
	GetDlgItem(IDC_VOLUME_THRESHOLD)->GetWindowText(str);
	m_nReduceVolumeThreshold=(float)atof(str);
	
	GetDlgItem(IDC_REDUCE_RATE_EDIT)->GetWindowText(str);
	m_nReduceRate=(float)atof(str);
	
	//	DWORD start = GetTickCount();
	TimeCountStart;
	this->m_pFr->m_pView->GetDocument()->ReduceMesh_Adaptive(); 
	TimeCountEnd;

	
	AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已精简数据");
	::SendMessage(this->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(this->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);
}
