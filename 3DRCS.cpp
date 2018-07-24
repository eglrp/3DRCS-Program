// 3DRCS.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "3DRCS.h"
#include "MainFrm.h"
#include "3DRCSDoc.h"
#include "3DRCSView.h"
//#include "WBButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// C3DRCSApp

BEGIN_MESSAGE_MAP(C3DRCSApp, CWinApp)
	//{{AFX_MSG_MAP(C3DRCSApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
//	ON_COMMAND(IDC_TURN, &C3DRCSApp::OnTurn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3DRCSApp construction

C3DRCSApp::C3DRCSApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only C3DRCSApp object

C3DRCSApp	theApp;
bool	gbIsGetData;          //全局变量,用来判断是否正在读取数据,读取数据时不进行任何图像显示
bool	gbDataIsEmpty;        //全局变量，用来判断数据是否存在．存在用false,不存在用true;

long    m_FacetNumber=0;        //三角面片的总数
long    m_EdgeNumber=0;         //半边总数
long    m_VertexNumber=0;       //点的总数

//定义模型在X、Y和Z方向上的最大和最小值，以及模型旋转中心的x、y和z坐标（即x_glMid，y_glMid和z_glMid）
float x_glMid=0.0f,x_glMin=1024.0f,x_glMax=-1024.0f;
float y_glMid=0.0f,y_glMin=1024.0f,y_glMax=-1024.0f;  
float z_glMid=0.0f,z_glMin=1024.0f,z_glMax=-1024.0f;

vector<Vertex> m_Vertex;   //模型的点数据集
vector<Edge>   m_Edge;     //模型的网格边数据集
vector<Facet>  m_Facet;    //模型的三角面片的数据集
// vector <StartEndPoint> Startendpoint;
/////////////////////////////////////////////////////////////////////////////
// C3DRCSApp initialization

BOOL C3DRCSApp::InitInstance()
{
	AfxEnableControlContainer();
	gbIsGetData = false;
	gbDataIsEmpty = true;
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
#if _MSC_VER <= 1200 // MFC 6.0 or earlier
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif
	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(C3DRCSDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(C3DRCSView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
	m_pMainWnd->SetWindowText("三维重建系统（3DRCS）->未加载数据");

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CButton	m_Ok;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDOK, m_Ok);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// App command to run the dialog
void C3DRCSApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// C3DRCSApp message handlers



BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	//this->m_Ok.LoadBitmaps(IDB_BUTTON,5, 5, 5, 5, 4 );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//void C3DRCSApp::OnTurn()
//{
//	// TODO: 在此添加命令处理程序代码
//
//}


void CAboutDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialog::OnOK();
}
