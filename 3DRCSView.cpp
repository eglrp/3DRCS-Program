// 3DRCSView.cpp : implementation of the C3DRCSView class
//

#include "stdafx.h"
#include "3DRCS.h"
#include "3DRCSDoc.h"
#include "3DRCSView.h"
#include "MainFrm.h"
#include "ControlWnd.h"
#include "ProjectView.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


float  xrof=-60.0f,yrof=0.0f,zrof=30.0f;//�����������
float	m_Prox=1.0f,m_Proy=1.0f,m_Proz=1.0f; //��������

/////////////////////////////////////////////////////////////////////////////
// C3DRCSView

extern bool	  gbIsGetData;
extern bool   gbDataIsEmpty;

IMPLEMENT_DYNCREATE(C3DRCSView, CScrollView)

BEGIN_MESSAGE_MAP(C3DRCSView, CScrollView)
//{{AFX_MSG_MAP(C3DRCSView)
ON_WM_CREATE()
ON_WM_DESTROY()
ON_WM_SIZE()
ON_COMMAND(IDC_TURNX, OnTurnx)
ON_COMMAND(IDC_TURNY, OnTurny)
ON_COMMAND(IDC_TURNZ, OnTurnz)
ON_WM_ERASEBKGND()
ON_WM_MOUSEWHEEL()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_CONTEXTMENU()
	ON_WM_KILLFOCUS()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
// Standard printing commands
ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
ON_COMMAND(IDC_FACET, &C3DRCSView::OnFacet)
ON_COMMAND(IDC_EDGE, &C3DRCSView::OnEdge)
ON_COMMAND(IDC_VERTEX, &C3DRCSView::OnVertex)
ON_COMMAND(IDC_TURN, &C3DRCSView::OnTurn)
ON_COMMAND(IDC_TRANSLATE, &C3DRCSView::OnTranslate)
ON_COMMAND(IDC_ZOOMOUT, &C3DRCSView::OnZoomout)
ON_UPDATE_COMMAND_UI(IDC_EDGE, &C3DRCSView::OnUpdateEdge)
ON_UPDATE_COMMAND_UI(IDC_FACET, &C3DRCSView::OnUpdateFacet)
ON_UPDATE_COMMAND_UI(IDC_VERTEX, &C3DRCSView::OnUpdateVertex)
ON_UPDATE_COMMAND_UI(IDC_TURNX, &C3DRCSView::OnUpdateTurnx)
ON_UPDATE_COMMAND_UI(IDC_TURNY, &C3DRCSView::OnUpdateTurny)
ON_UPDATE_COMMAND_UI(IDC_TURNZ, &C3DRCSView::OnUpdateTurnz)
ON_UPDATE_COMMAND_UI(IDC_TURN, &C3DRCSView::OnUpdateTurn)
ON_UPDATE_COMMAND_UI(IDC_TRANSLATE, &C3DRCSView::OnUpdateTranslate)
ON_UPDATE_COMMAND_UI(IDC_ZOOMOUT, &C3DRCSView::OnUpdateZoomout)
ON_COMMAND(IDC_FacetEdge, &C3DRCSView::OnFacetedge)
ON_UPDATE_COMMAND_UI(IDC_FacetEdge, &C3DRCSView::OnUpdateFacetedge)
//ON_COMMAND(IDM_REPAIRHOLE, &C3DRCSView::OnRepairhole)
//ON_COMMAND(IDM_REPAIRHOLE, &C3DRCSView::OnRepairhole)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3DRCSView construction/destruction

C3DRCSView::C3DRCSView()
{
	m_pFr = NULL;
	m_pMainVDC = NULL;
	pStatus = NULL;
	nTurnType = 0;

    m_bShowEdge=false;
	m_bShowFacet=true;
	m_bShowVertex=false;
	m_bShowFacetEdge=false;

	m_bTurnx=true;
	m_bTurny=m_bTurnz=false;

	m_bTranslate=false;
	m_bTurn=true;
	m_bZoomout=false;

	m_FacetNumber=m_VertexNumber=m_EdgeNumber=0;
	m_nSliderCurX=m_nSliderCurY=m_nSliderCurZ=0;

	m_bDown = false;
	m_MouseAction = 0;
	m_ShowType = 2;

	oldx1=oldy1=100.0f;
	oldx2=oldy2=-100.0f;
	newx1=newy1=-100.0f;
	newx2=newy2=100.0f;
	m_MouseMove.x=m_MouseDown.x=m_MouseUp.x=0;
	m_MouseMove.y=m_MouseDown.y=m_MouseUp.y=0;

	xx=1.0, yy=1.0, zz=1.0;
}

C3DRCSView::~C3DRCSView()
{
	if (m_pMainVDC)
	{
		delete m_pMainVDC;
		m_pMainVDC = NULL;
	}
}

BOOL C3DRCSView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	
	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// C3DRCSView drawing

void C3DRCSView::OnDraw(CDC* pDC)
{
	C3DRCSDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);		

	if (!::wglMakeCurrent(m_pMainVDC->GetSafeHdc(),m_pMainVRC))
	{
		AfxMessageBox("MakeCurrentError");
		return ;
	}
	
	
	//�����ɫ�������Ȼ���
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	//�õ�λ�����滻��ǰ����
	glLoadIdentity();	
	//��ת�Ƕ�

	glRotatef(xrof,1.0f,0.0f,0.0f);//x��	
	glRotatef(yrof,0.0f,1.0f,0.0f);//y��
	glRotatef(zrof,0.0f,0.0f,1.0f);//z��
    
	
	this->SetLighting();
	
	//����
	this->ShowObjects();
	//����ϵ������
	if (this->m_pFr->m_pCtrlWnd->m_bShowCoordinate)
	{
		this->DrawLines();
		this->DrawText();
	}

	//ǿ�ƻ�ͼ���
	glFinish();
	//��������������
	SwapBuffers(wglGetCurrentDC());
		
	this->m_pFr->ShowControlBar(&(this->m_pFr->m_wndToolBar),this->m_pFr->m_bShowToolbar,FALSE);
	this->m_pFr->m_pProView->Invalidate();

	::wglMakeCurrent(m_pMainVDC->GetSafeHdc(),NULL);
}

/////////////////////////////////////////////////////////////////////////////
// C3DRCSView printing

BOOL C3DRCSView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

void C3DRCSView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void C3DRCSView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// C3DRCSView diagnostics

#ifdef _DEBUG
void C3DRCSView::AssertValid() const
{
	CScrollView::AssertValid();
}

void C3DRCSView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

C3DRCSDoc* C3DRCSView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(C3DRCSDoc)));
	return (C3DRCSDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// C3DRCSView message handlers

int C3DRCSView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	this->IniOpenGL();
//	SetTimer(0,100,NULL);
	return 0;
}

//�ڴ��ͷ�OpenGLʹ�õ��豸����
void C3DRCSView::OnDestroy() 
{
	CScrollView::OnDestroy();
	m_pMainVRC = ::wglGetCurrentContext();
	::wglMakeCurrent(NULL,  NULL);
	if (m_pMainVRC)
		::wglDeleteContext(m_pMainVRC);	
}

//�����ӿ�
void C3DRCSView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	this->m_xRect = cx;
	this->m_yRect = cy;
	SetViewPort();
}

void C3DRCSView::SetViewPort()
{

	if (!::wglMakeCurrent(m_pMainVDC->GetSafeHdc(),m_pMainVRC))
	{
		AfxMessageBox("MakeCurrentError");
		return ;
	}

	//�������Ϊ0
	if(m_yRect==0)
		m_yRect=1;

	glViewport(0,0,m_xRect,m_yRect);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	if(m_xRect>=m_yRect)
	{
		//����һ������ͶӰ�������壬�����еĲ����������ͶӰƽ�������λ�ã�
		glOrtho(newx1, newx2, newy1*(GLfloat)m_yRect/(GLfloat)m_xRect, newy2*(GLfloat)m_yRect/(GLfloat)m_xRect, -500.0,500.0);
	}
	else
	{
		glOrtho(newx1*(GLfloat)m_xRect/(GLfloat)m_yRect, newx2*(GLfloat)m_xRect/(GLfloat)m_yRect, newy1, newy2, -500.0,500.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	::wglMakeCurrent(NULL, NULL);
}



//��������OpenGLʹ�õ����ظ�ʽ
bool C3DRCSView::bSetPixelFormat()
{
	//����һ�����ظ�ʽ
	static PIXELFORMATDESCRIPTOR pfd =
	{   
		sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
			1,  // version number
			PFD_DRAW_TO_WINDOW |// support window
			PFD_SUPPORT_OPENGL |  // support OpenGL  ֧��OpenGL
			PFD_DOUBLEBUFFER, // double buffered ֧���ֻ���
			PFD_TYPE_RGBA,  // RGBA typeʹ��RGBAģʽ�����õ�ɫ��
			24, // 24-bit color depth  ʹ��24λ���ɫ
			0, 0, 0, 0, 0, 0,   // color bits ignored
			0,  // no alpha buffer
			0,  // shift bit ignored
			0,  // no accumulation buffer
			0, 0, 0, 0, // accum bits ignored
			32, // 32-bit z-buffer   32λZ�Ỻ��
			0,  // no stencil buffer
			0,  // no auxiliary buffer
			PFD_MAIN_PLANE, // main layer
			0,  // reserved
			0, 0, 0 // layer masks ignored
	};
	int pixelformat;
	
	//������Եõ�ָ����
	if ( (pixelformat = ChoosePixelFormat(m_pMainVDC->GetSafeHdc(), &pfd)) == FALSE )
	{
		MessageBox( "ChoosePixelFormat failed!",  "��Ϣ��ʾ", MB_OK );
		return false;
	}
	
	//������ȡ���ĸ�ʽ�����豸����
	if (SetPixelFormat(m_pMainVDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
	{
		MessageBox( "SetPixelFormat failed!",  "��Ϣ��ʾ", MB_OK );
		return false;
	}
	return true;
}

//��ʼ��OpenGL����
void C3DRCSView::IniOpenGL()
{
	m_pMainVDC = new CClientDC(this);
	PIXELFORMATDESCRIPTOR pfd;
	int n;
	
	//��ʼ����������Ҫ���ǳ�ʼ����һ���ͻ������豸����ָ��
	ASSERT(m_pMainVDC != NULL);
	
	//����Ӧ����������ظ�ʽ�����뵱ǰ�豸�����������
	if (!bSetPixelFormat())
		return;
	
	//�õ�ָ���豸����������ģʽ����
	n = ::GetPixelFormat(m_pMainVDC->GetSafeHdc());
	
	//��������õ�������ֵ������һ������ģʽ
	::DescribePixelFormat(m_pMainVDC->GetSafeHdc(), n, sizeof(pfd), &pfd);
	
	//����һ���������豸����
	m_pMainVRC = wglCreateContext(m_pMainVDC->GetSafeHdc());
	
	//�������ɵ��豸������ָ����Ϊ��ǰ����
	wglMakeCurrent(m_pMainVDC->GetSafeHdc(), m_pMainVRC);		
	
	//�úڱ���
//	glClearColor(0.0,0.0,0.0,0.0);	
	glClearColor(1.0,1.0,1.0,0.0);//��ɫ����
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);	
	glShadeModel(GL_SMOOTH);
	
	// ���û�ɫ����ȡ�ð�͸��Ч��
	glBlendFunc(GL_SRC_ALPHA,GL_ONE); 	
	glEnable(GL_BLEND); 
	
	//ƽ������
    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_BLEND);
	
	//��ʼ��������Ϊ RGBA ģʽ��ͬʱ���� alpha ��ϡ���ʾ������
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_BLEND);
	
	// ������ϸ��͸������
    glHint (GL_POLYGON_SMOOTH_HINT|GL_LINE_SMOOTH_HINT, GL_NICEST);
	
	//������Ȳ���
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); 

	::wglMakeCurrent(NULL, NULL);
}

//��Ϊ��X����ת
void C3DRCSView::OnTurnx() 
{
	nTurnType = 0;
	m_bTurnx=!m_bTurnx;
	m_bTurny=m_bTurnz=false;
	CButton *btn = (CButton*)this->m_pFr->m_pCtrlWnd->GetDlgItem(IDC_RADIOTURNX);
	btn->SetCheck(1);
	btn = (CButton*)this->m_pFr->m_pCtrlWnd->GetDlgItem(IDC_RADIOTURNY);
	btn->SetCheck(0);
	btn = (CButton*)this->m_pFr->m_pCtrlWnd->GetDlgItem(IDC_RADIOTURNZ);
	btn->SetCheck(0);
}

//��Ϊ��Y����ת
void C3DRCSView::OnTurny() 
{
	nTurnType = 1;
	m_bTurny=!m_bTurny;
	m_bTurnx=m_bTurnz=false;
	CButton *btn = (CButton*)this->m_pFr->m_pCtrlWnd->GetDlgItem(IDC_RADIOTURNX);
	btn->SetCheck(0);
	btn = (CButton*)this->m_pFr->m_pCtrlWnd->GetDlgItem(IDC_RADIOTURNY);
	btn->SetCheck(1);
	btn = (CButton*)this->m_pFr->m_pCtrlWnd->GetDlgItem(IDC_RADIOTURNZ);
	btn->SetCheck(0);
}

//��Ϊ��Z����ת
void C3DRCSView::OnTurnz() 
{
	nTurnType = 2;
	m_bTurnz=!m_bTurnz;
	m_bTurny=m_bTurnx=false;
	CButton *btn = (CButton*)this->m_pFr->m_pCtrlWnd->GetDlgItem(IDC_RADIOTURNX);
	btn->SetCheck(0);
	btn = (CButton*)this->m_pFr->m_pCtrlWnd->GetDlgItem(IDC_RADIOTURNY);
	btn->SetCheck(0);
	btn = (CButton*)this->m_pFr->m_pCtrlWnd->GetDlgItem(IDC_RADIOTURNZ);
	btn->SetCheck(1);
}

//���ػ�����
BOOL C3DRCSView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;	
}


//������������Ϣ
BOOL C3DRCSView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (this->m_MouseAction==0)
	{		
		switch (nTurnType)
		{
		case 0:
			{
				xrof -= zDelta/20;
				if (xrof > -360 || xrof < 360)
					xrof = ::xrof;
				else
					xrof = (int)xrof%360; 
			}
			break;
		case 1:
			{
				yrof -= zDelta/20;			
				if (yrof >= -360 || yrof < 360)
					yrof = ::yrof;
				else
					yrof = (int)yrof%360; 
			}	
			break;
		case 2:
			{
				zrof -= zDelta/20;				
				if (zrof >=-360 || zrof< 360)
					zrof = ::zrof;
				else
					zrof = (int)zrof%360; 
			}	
			break;
		default:
			break;
		}

		CString str;
		pStatus =& this->m_pFr->m_wndStatusBar;
		if(pStatus)
		{
			str.Format("X����ת��:%.3d��", ((int) ::xrof)%360);
			pStatus->SetPaneText(4,str);
			str.Format("Y����ת��:%.3d��", ((int)::yrof)%360);
			pStatus->SetPaneText(5,str);
			str.Format("Z����ת��:%.3d��", ((int) ::zrof)%360);
			pStatus->SetPaneText(6,str);
		}

		this->Invalidate();
	}
	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

//������ϵ
void C3DRCSView::DrawLines()
{
	//Draw Lines
	glBegin(GL_LINES);
	//X
	glColor3f(1.0f,0.0f,0.0f);
	glVertex3f(-50.0f,0.0f,0.0f);	glVertex3f(50.0f,0.0f,0.0f);
	//Arrow
	glVertex3f(45.0f,2.0f,0.0f);	glVertex3f(50.0f,0.0f,0.0f);
	glVertex3f(45.0f,-2.0f,0.0f);	glVertex3f(50.0f,0.0f,0.0f);

	glVertex3f(45.0f,0.0f,2.0f);	glVertex3f(50.0f,0.0f,0.0f);
	glVertex3f(45.0f,0.0f,-2.0f);	glVertex3f(50.0f,0.0f,0.0f);	

	//Y
	glColor3f(0.0f,1.0f,0.0f);	
	glVertex3f(0.0f,-50.0f,0.0f);	glVertex3f(0.0f,50.0f,0.0f);
	//Arrow
	glVertex3f(-2.0f,45.0f,0.0f);	glVertex3f(0.0f,50.0f,0.0f);
	glVertex3f(2.0f,45.0F,0.0f);	glVertex3f(0.0f,50.0f,0.0f);
	
	glVertex3f(0.0f,45.0f,-2.0f);	glVertex3f(0.0f,50.0f,0.0f);
	glVertex3f(0.0f,45.0F,2.0f);	glVertex3f(0.0f,50.0f,0.0f);
	
	//Z
	glColor3f(0.0f,0.0f,1.0f);
	glVertex3f(0.0f,0.0f,-50.0f);	glVertex3f(0.0f,0.0f,50.0f);
	//Arrow
	glVertex3f(2.0f,0.0f,45.0f);	glVertex3f(0.0f,0.0f,50.0f);
	glVertex3f(-2.0f,0.0f,45.0f);	glVertex3f(0.0f,0.0f,50.0f);
	
	glVertex3f(0.0f,2.0f,45.0f);	glVertex3f(0.0f,0.0f,50.0f);
	glVertex3f(0.0f,-2.0f,45.0f);	glVertex3f(0.0f,0.0f,50.0f);

	glEnd();
	
	glPointSize(2.0);
	glBegin(GL_POINTS); 	
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(25.0f,0.0f,0.0f);		
	glVertex3f(-25.0f,0.0f,0.0f);
	glVertex3f(0.0f,25.0f,0.0f);
	glVertex3f(0.0f,-25.0f,0.0f);
	glVertex3f(0.0f,0.0f,25.0f);
	glVertex3f(0.0f,0.0f,-25.0f);
	glEnd();
}

//����ͼԪ
void C3DRCSView::ShowObjects()
{
	if (::gbIsGetData)
		return;
	
	if (::gbDataIsEmpty)
		return;

	//X��Y��Z��������ƽ��
	if(m_nSliderCurX<0)	{  xx=-1.0; }   else {  xx=1.0;	}
	if(m_nSliderCurY<0)	{  yy=-1.0;	}   else {  yy=1.0;	}
	if(m_nSliderCurZ<0) {  zz=-1.0;	}	else {  zz=1.0;	}
	double x[4]={xx, 0.0, 0.0, (x_glMid-x_glMin+2-abs(m_nSliderCurX)*(x_glMax-x_glMin)/100.0)*m_Prox};
	glClipPlane(GL_CLIP_PLANE1, x);
	glEnable(GL_CLIP_PLANE1);

	double y[4]={0.0, yy, 0.0, (y_glMid-y_glMin+2-abs(m_nSliderCurY)*(y_glMax-y_glMin)/100.0)*m_Proy};
	glClipPlane(GL_CLIP_PLANE2, y);
	glEnable(GL_CLIP_PLANE2);

    double z[4]={0.0, 0.0, zz, (z_glMid-z_glMin+2-abs(m_nSliderCurZ)*(z_glMax-z_glMin)/100.0)*m_Proz};
	glClipPlane(GL_CLIP_PLANE3, z);
	glEnable(GL_CLIP_PLANE3);

	glScalef(m_Prox,m_Proy,m_Proz);

	//Ϊ�˽����Ƶ������м����������ƽ������ϵ
	glTranslatef(-x_glMid,0.0f,0.0f);	
	glTranslatef(0.0f,-y_glMid,0.0f);
	glTranslatef(0.0f,0.0f,-z_glMid);

/*	glBegin(GL_QUADS);
	glNormal3f(0.0,0.0,1.0);
	glVertex3f(x_glMin-10.0, y_glMin-10.0, z_glMid);
	glVertex3f(x_glMax+10.0, y_glMin-10.0, z_glMid);
	glVertex3f(x_glMax+10.0, y_glMax+10.0, z_glMid);
	glVertex3f(x_glMin-10.0, y_glMax+10.0, z_glMid);
	glEnd();*/

	int i, j;
	switch(m_ShowType)
	{
	case 0:
		glPointSize(2);
		glBegin(GL_POINTS); 
		for ( j=0; j<m_Vertex.size(); j++)
		{	
			if(m_Vertex[j].flag)
			{
				glColor3f(m_Vertex[j].nx, m_Vertex[j].ny, m_Vertex[j].nz);
				glVertex3f(m_Vertex[j].x, m_Vertex[j].y, m_Vertex[j].z);
			}
		}
		glEnd();
		break;

	case 1:
		for( i=0; i<m_Edge.size();i++)
		{
			if(m_Edge[i].flag)
			{
				glColor3f(0.0, 0.0, 0.0);
				glBegin(GL_LINES);
				
				glVertex3f(m_Vertex[m_Edge[i].vertex1].x,
					m_Vertex[m_Edge[i].vertex1].y,
					m_Vertex[m_Edge[i].vertex1].z);
				
				glVertex3f(m_Vertex[m_Edge[i].vertex2].x,
					m_Vertex[m_Edge[i].vertex2].y,
					m_Vertex[m_Edge[i].vertex2].z);
			}
		}
		glEnd();

		break;
/*		for( i=0; i<m_Edge.size();i++)
		{
			if(m_Edge[i].flag)
			{
				glColor3f(0.0, 1.0, 1.0);
				glBegin(GL_LINES);
				
				glVertex3f(m_Vertex[m_Edge[i].vertex1].x,
					m_Vertex[m_Edge[i].vertex1].y,
					m_Vertex[m_Edge[i].vertex1].z);
				
				glVertex3f(m_Vertex[m_Edge[i].vertex2].x,
					m_Vertex[m_Edge[i].vertex2].y,
					m_Vertex[m_Edge[i].vertex2].z);
			}
		}
		glEnd();
		break;
*/
	case 2:
		glBegin(GL_TRIANGLES);
		for( i=0;i<m_Facet.size();i++)
		{
			if(m_Facet[i].flag)
			{
				glColor3f(m_Facet[i].nx, m_Facet[i].ny, m_Facet[i].nz);
				if(this->m_pFr->m_pCtrlWnd->m_bShowReverse)  //������Ƭ����
					glNormal3f(-m_Facet[i].nx,-m_Facet[i].ny,-m_Facet[i].nz);
				else
					glNormal3f(m_Facet[i].nx,m_Facet[i].ny,m_Facet[i].nz);
				
				glVertex3f(m_Vertex[m_Facet[i].vertex[0]].x,
					m_Vertex[m_Facet[i].vertex[0]].y,
					m_Vertex[m_Facet[i].vertex[0]].z);
				
				glVertex3f(m_Vertex[m_Facet[i].vertex[1]].x,
					m_Vertex[m_Facet[i].vertex[1]].y,
					m_Vertex[m_Facet[i].vertex[1]].z);
				
				glVertex3f(m_Vertex[m_Facet[i].vertex[2]].x,
					m_Vertex[m_Facet[i].vertex[2]].y,
					m_Vertex[m_Facet[i].vertex[2]].z);
			}
		}
		glEnd();
		break;
		
	case 3:
		glBegin(GL_TRIANGLES);
		for( i=0;i<m_Facet.size();i++)
		{
			if(m_Facet[i].flag)
			{
				glColor3f(1.0f, 1.0f, 1.0f);
				if(this->m_pFr->m_pCtrlWnd->m_bShowReverse)  //������Ƭ����
					glNormal3f(-m_Facet[i].nx,-m_Facet[i].ny,-m_Facet[i].nz);
				else
					glNormal3f(m_Facet[i].nx,m_Facet[i].ny,m_Facet[i].nz);
				
				glVertex3f(m_Vertex[m_Facet[i].vertex[0]].x,
					m_Vertex[m_Facet[i].vertex[0]].y,
					m_Vertex[m_Facet[i].vertex[0]].z);
				
				glVertex3f(m_Vertex[m_Facet[i].vertex[1]].x,
					m_Vertex[m_Facet[i].vertex[1]].y,
					m_Vertex[m_Facet[i].vertex[1]].z);
				
				glVertex3f(m_Vertex[m_Facet[i].vertex[2]].x,
					m_Vertex[m_Facet[i].vertex[2]].y,
					m_Vertex[m_Facet[i].vertex[2]].z);
			}
		}
		glEnd();

		for( i=0; i<m_Edge.size();i++)
		{
			if(m_Edge[i].flag)
			{
				glColor3f(0.0, 0.0, 0.0);
//				glColor3f(0.0, 1.0, 1.0);
				glBegin(GL_LINES);
				
				glVertex3f(m_Vertex[m_Edge[i].vertex1].x,
					m_Vertex[m_Edge[i].vertex1].y,
					m_Vertex[m_Edge[i].vertex1].z);
				
				glVertex3f(m_Vertex[m_Edge[i].vertex2].x,
					m_Vertex[m_Edge[i].vertex2].y,
					m_Vertex[m_Edge[i].vertex2].z);
			}
		}
		glEnd();

		break;
	}
		
	//������ϵ�ƻ�ԭ��
	glTranslatef(x_glMid,0.0f,0.0f);
	glTranslatef(0.0f,y_glMid,0.0f);
	glTranslatef(0.0f,0.0f,z_glMid);

}


void C3DRCSView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	KillTimer(0);
	this->m_bDown = true;
	this->m_xM = point.x;
	this->m_yM = point.y;
	m_SelRect.bottom = m_SelRect.left = m_SelRect.top = m_SelRect.right = 0;
	m_MouseDown = point;
	m_MouseMove = point;
	m_MouseUp   = point;
	oldx1=newx1;oldx2=newx2;
	oldy1=newy1;oldy2=newy2;

	this->m_pFr->m_pProView->m_bShowBmp=false;

	CScrollView::OnLButtonDown(nFlags, point);
}

void C3DRCSView::OnLButtonUp(UINT nFlags, CPoint point) 
{	
	this->m_bDown = false;
	
	m_MouseUp=point;

	if (this->m_MouseAction==2 )
	{
		m_SelRect = CRect(m_MouseDown, m_MouseUp);
		DrawSelArea();
	}
	if (this->m_MouseAction==1 )
	{
		oldx1=newx1;oldx2=newx2;oldy1=newy1;oldy2=newy2;
	}
	CScrollView::OnLButtonUp(nFlags, point);
}


void C3DRCSView::DrawSelArea()
{
	oldx1=newx1;	oldy1=newy1;	oldx2=newx2;	oldy2=newy2;

	GLfloat kx,ky,x1,y1,x2,y2,tempt, dx,dy;

	kx=(oldx2-oldx1)/m_xRect;
	ky=(oldy2-oldy1)/m_yRect;

	x1=m_MouseDown.x;   x2=m_MouseUp.x;
	y1=m_yRect-m_MouseUp.y; y2=m_yRect-m_MouseDown.y;

	if(x1>x2)
	{
		tempt=x1;	x1=x2;	x2=tempt;
	}

	if(y1>y2)
	{
		tempt=y1;	y1=y2;	y2=tempt;
	}

	dx=x2-x1;dy=y2-y1;

	if(dx<dy)
		x2=x1+dy;
	else
		y2=y1+dx;

	newx1=oldx1+x1*kx;newx2=oldx1+x2*kx;
	newy1=oldy1+y1*ky;newy2=oldy1+y2*ky;

	SetViewPort();	
	this->Invalidate();
}


void C3DRCSView::OnMouseMove(UINT nFlags, CPoint point) 
{
	
	if (!m_bDown)	
		return;	
	
	if (!this->IsMouseInWndRect(point))
		this->m_bDown = false;

	m_MouseMove=point;
	
	if (this->m_MouseAction==0 )
	{
	
		//����һ����Χ,��֤�����������»�����ת������ϵ5������
		if (this->m_yM - point.y > 5 || this->m_yM - point.y < -5)		
			::xrof = xrof - (this->m_yM - point.y)/20;	
			if (::xrof >=-360 || ::xrof< 360)
				::xrof = ::xrof;
			else
				::xrof = (int)::xrof%360;		
		if ( this->m_xM - point.x > 5 || this->m_xM - point.x < -5 )		
			::yrof = yrof - (this->m_xM - point.x)/20;			
			if (::yrof > -360 || ::yrof< 360)
				::yrof = ::yrof;
			else
				::yrof = (int)::yrof%360;	
			
		CString str;
		pStatus =& this->m_pFr->m_wndStatusBar;
		if(pStatus)
		{
			str.Format("X����ת��:%.3d��", ((int) ::xrof)%360);
			pStatus->SetPaneText(4,str);
			str.Format("Y����ת��:%.3d��", ((int)::yrof)%360);
			pStatus->SetPaneText(5,str);
			str.Format("Z����ת��:%.3d��", ((int) ::zrof)%360);
			pStatus->SetPaneText(6,str);
		}
				
		this->Invalidate();
	}
	else
	{
		m_SelRect = CRect(m_MouseDown, m_MouseMove);
		if (this->m_MouseAction==1 )
		{
			GLfloat dx,dy,x1,y1,x2,y2;
			x1=m_MouseDown.x;   x2=m_MouseMove.x;
			y1=m_MouseDown.y;   y2=m_MouseMove.y;
			dx=(x2-x1)*(newx2-newx1)/((float)m_xRect);
			dy=(y2-y1)*(newy2-newy1)/((float)m_yRect);
			newx1=oldx1-dx; newx2=oldx2-dx;
			newy1=oldy1+dy; newy2=oldy2+dy;	
			SetViewPort();
		}
		if (this->m_MouseAction==2 )
		{
			CClientDC dc(this);			
			dc.DrawFocusRect(&m_SelRect);
			m_SelRect.bottom=m_SelRect.left=m_SelRect.top=m_SelRect.right=0;
		}
		this->Invalidate();
	}
	
	CScrollView::OnMouseMove(nFlags, point);
}


void C3DRCSView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();	
	this->m_pFr =(CMainFrame*)GetParentFrame( );// (CMainFrame*)AfxGetApp()->m_pMainWnd;

}

//�жϵ�ǰ����Ƿ��ڿͻ�����
bool C3DRCSView::IsMouseInWndRect(CPoint point)
{
	bool bInWnd = true;
	if ( point.x <= 10 || point.x >= this->m_xRect-10 || point.y <= 10 || point.y >= this->m_yRect-10 )
		bInWnd = false;
	return bInWnd;
}


void C3DRCSView::DrawText()
{
	glPushMatrix();
	
	glColor3f(1.0f,1.0f,1.0f);
	
	wglUseFontBitmaps(wglGetCurrentDC(),0,255,100);
	
	glListBase(100);
	glRasterPos3f(50.20f,-0.5f,0.0f);
	glCallLists(2,GL_UNSIGNED_BYTE,_T("+x")); 
	
	glRasterPos3f(-0.5,50.2f,0.0f);
	glCallLists(2,GL_UNSIGNED_BYTE,_T("+y")); 
	
	glRasterPos3f(-0.5,0.0f,50.2f);
	glCallLists(2,GL_UNSIGNED_BYTE,_T("+z")); 
	
	glRasterPos3f(0.0,0.0f,0.0f);
	glCallLists(1,GL_UNSIGNED_BYTE,_T("C")); 
	
	glPopMatrix();
}

//���ù���Ч��
void C3DRCSView::SetLighting()
{
	//�ر�ע��ú���
	glColor3f(1.0f,1.0f,1.0f);
	//�������
	glEnable (GL_LIGHTING);
	glEnable(GL_LIGHT0);

	const GLfloat background[4]={1.0f, 1.0f, 1.0f,1.0f};
	glMaterialfv(GL_FRONT, GL_AMBIENT, background);
	glMaterialf(GL_FRONT, GL_SHININESS, 3.f);
	
	//ȡ������
	if (!this->m_pFr->m_pCtrlWnd->m_bLighting)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}
}

void C3DRCSView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu	m_XpMenu;
	m_XpMenu.LoadMenu(IDR_MYMENU);	
	CMenu *psub = (CMenu *)m_XpMenu.GetSubMenu(0);		
	psub->TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,this);
	m_XpMenu.DestroyMenu();		
}


void C3DRCSView::OnKillFocus(CWnd* pNewWnd) 
{
	CScrollView::OnKillFocus(pNewWnd);
	this->m_bDown = false;
}

void C3DRCSView::Reback()
{
	this->newx1=-100.0f;
	this->newx2=100.0f;
	this->newy1=-100.0f;
	this->newy2=100.0f;
	this->SetViewPort();
	this->m_nSliderCurX=0;
	this->m_nSliderCurY=0;
	this->m_nSliderCurZ=0;
	m_SelRect.bottom=m_SelRect.left=m_SelRect.top=m_SelRect.right=0;
}

void C3DRCSView::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	zrof=int(zrof+1.0f)%360;
	CString str;
	pStatus =& this->m_pFr->m_wndStatusBar;
	if(pStatus)
	{
		str.Format("X����ת��:%.3d��", ((int) ::xrof)%360);
		pStatus->SetPaneText(4,str);
		str.Format("Y����ת��:%.3d��", ((int)::yrof)%360);
		pStatus->SetPaneText(5,str);
		str.Format("Z����ת��:%.3d��", ((int) ::zrof)%360);
		pStatus->SetPaneText(6,str);
	}
	this->Invalidate();
	CScrollView::OnTimer(nIDEvent);
}

void C3DRCSView::SetTime()
{
	SetTimer(0,200,NULL);
}

void C3DRCSView::KillTime()
{
	KillTimer(0);
}


void C3DRCSView::OnFacet()
{
	// TODO: �ڴ���������������
	m_ShowType=2;
	
	m_bShowFacet=!m_bShowFacet;
	m_bShowFacetEdge=m_bShowEdge=m_bShowVertex=false;
	this->Invalidate();
}


void C3DRCSView::OnEdge()
{
	// TODO: �ڴ���������������
	m_ShowType=1;
	
m_bShowEdge=!m_bShowEdge;
m_bShowFacetEdge=m_bShowFacet=m_bShowVertex=false;
	this->Invalidate();
}


void C3DRCSView::OnVertex()
{
	// TODO: �ڴ���������������
	m_ShowType=0;
	m_bShowVertex=!m_bShowVertex;
	m_bShowFacetEdge=m_bShowEdge=m_bShowFacet=false;
	this->Invalidate();
}


void C3DRCSView::OnTurn()
{
	// TODO: �ڴ���������������
	m_MouseAction = 0;
	m_bTurn=!m_bTurn;
	m_bTranslate=m_bZoomout=false;
	this->Invalidate();
}


void C3DRCSView::OnTranslate()
{
	// TODO: �ڴ���������������
	m_MouseAction = 1;
	m_bTranslate=!m_bTranslate;
	m_bTurn=m_bZoomout=false;
	this->Invalidate();
}


void C3DRCSView::OnZoomout()
{
	// TODO: �ڴ���������������
	m_MouseAction = 2;
	m_bZoomout=!m_bZoomout;
	m_bTurn=m_bTranslate=false;
	this->Invalidate();
}


void C3DRCSView::OnUpdateEdge(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(m_bShowEdge); 
}


void C3DRCSView::OnUpdateFacet(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(m_bShowFacet);
}


void C3DRCSView::OnUpdateVertex(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(m_bShowVertex);
}


void C3DRCSView::OnUpdateTurnx(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(m_bTurnx);
}


void C3DRCSView::OnUpdateTurny(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(m_bTurny);
}


void C3DRCSView::OnUpdateTurnz(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(m_bTurnz);
}


void C3DRCSView::OnUpdateTurn(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(m_bTurn);
}


void C3DRCSView::OnUpdateTranslate(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(m_bTranslate);
}


void C3DRCSView::OnUpdateZoomout(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(m_bZoomout);
}


void C3DRCSView::OnFacetedge()
{
	// TODO: �ڴ���������������
	m_ShowType=3;
	
	m_bShowFacetEdge=!m_bShowFacetEdge;
	m_bShowFacet=m_bShowEdge=m_bShowVertex=false;
	this->Invalidate();

}


void C3DRCSView::OnUpdateFacetedge(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(m_bShowFacetEdge); 
}


//void C3DRCSView::OnRepairhole()
//{
//	// TODO: �ڴ���������������
//}


//void C3DRCSView::OnRepairhole()
//{
//	// TODO: �ڴ���������������
//	CDialog dlg;
//	dlg.DoModal();
//	
//}
