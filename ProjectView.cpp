// ProjectView.cpp : implementation file
//

#include "stdafx.h"
#include "3DRCS.h"
#include "ProjectView.h"
#include "MainFrm.h"
#include "ControlWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CDC *   m_pProVDC;
extern HGLRC   m_pProVRC;

extern bool gbIsGetData;
extern bool gbDataIsEmpty;

#define PALETTESIZE(b) (((b)==8)?256:(((b)==4)?16:0))

extern float  xrof,yrof,zrof;//�����������
extern float	m_Prox,m_Proy,m_Proz; //��������
/////////////////////////////////////////////////////////////////////////////
// CProjectView

IMPLEMENT_DYNCREATE(CProjectView, CView)

CProjectView::CProjectView()
{	
	m_bShowBmp=false;
}

CProjectView::~CProjectView()
{


}


BEGIN_MESSAGE_MAP(CProjectView, CView)
	//{{AFX_MSG_MAP(CProjectView)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectView drawing

void CProjectView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();

	if(m_bShowBmp)
	{
		CBrush Brush (BLACK); 
		CBrush* pOldBrush = pDC->SelectObject(&Brush);     
		CRect ViewClip; 
		pDC->GetClipBox(&ViewClip);     
		pDC->PatBlt(ViewClip.left,ViewClip.top,ViewClip.Width(),ViewClip.Height(),PATCOPY);    
		pDC->SelectObject (pOldBrush );
		
		CFont *OldFont = pDC->SelectObject(&this->m_pFr->m_13Font);

		CRect rc;
		GetClientRect(&rc);
		pDC->SetTextAlign(TA_CENTER);
		pDC->SetTextColor(YELLOW);
		pDC->SetBkMode(1);
		
		CPen *pen = new CPen(PS_SOLID,1,GREEN);
		CPen *pOldPen = pDC->SelectObject(pen);	
		pDC->TextOut(10,10,"ͼ");
		pDC->TextOut(10,rc.Height()/3-5,"��");
		pDC->TextOut(10,rc.Height()*2/3-10,"��");
		pDC->TextOut(10,rc.Height()-25,"��");
		ShowPics(pDC);
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(OldFont);
		delete pen;
		
	}
	else
	{
		if (!::wglMakeCurrent(m_pProVDC->GetSafeHdc(),m_pProVRC)){
			AfxMessageBox("MakeCurrentError");
			return ;
		}
		
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPushMatrix();
		
		glRotatef(xrof,1.0f,0.0f,0.0f);//x��	
		glRotatef(yrof,0.0f,1.0f,0.0f);//y��
		glRotatef(zrof,0.0f,0.0f,1.0f);//z��
		
		ShowObjects();
		glPopMatrix();
		
		glFinish();
		SwapBuffers(wglGetCurrentDC());	
		
		::wglMakeCurrent(m_pProVDC->GetSafeHdc(),NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CProjectView diagnostics

#ifdef _DEBUG
void CProjectView::AssertValid() const
{
	CView::AssertValid();
}

void CProjectView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CProjectView message handlers

void CProjectView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();	
	m_pFr = (CMainFrame*)AfxGetApp()->m_pMainWnd;
}

BOOL CProjectView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CProjectView::OnMouseMove(UINT nFlags, CPoint point) 
{	
	CView::OnMouseMove(nFlags, point);
}

BOOL CProjectView::bSetupPixelFormat(HDC hdc)
{
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd 
			1,                     // version number 
			PFD_DRAW_TO_WINDOW |   // support window 
			PFD_SUPPORT_OPENGL |   // support OpenGL 
			PFD_GENERIC_FORMAT |
			PFD_DOUBLEBUFFER,      // double buffered 
			PFD_TYPE_RGBA,         // RGBA type 
			32,                    // 24-bit color depth 
			0, 0, 0, 0, 0, 0,      // color bits ignored 
			0,                     // no alpha buffer 
			0,                     // shift bit ignored 
			8,                     // no accumulation buffer 
			0, 0, 0, 0,            // accum bits ignored 
			32,                    // 32-bit z-buffer	 
			0,                     // no stencil buffer 
			0,                     // no auxiliary buffer 
			PFD_MAIN_PLANE,        // main layer 
			0,                     // reserved 
			0, 0, 0                // layer masks ignored 
    };
    int nGLPixelIndex;
    if((nGLPixelIndex = ::ChoosePixelFormat(hdc, &pfd)) == 0)
	{
        AfxMessageBox("ChoosePixelFormat failed");
        return FALSE;
    }
	
    if(::SetPixelFormat(hdc, nGLPixelIndex, &pfd) == FALSE)
	{
        AfxMessageBox("SetPixelFormat failed");
        return FALSE;
    }
    return TRUE;
}

void CProjectView::InitOpenGL()
{
	m_pProVDC = new CClientDC(this);

	PIXELFORMATDESCRIPTOR pfd;
	int n;
	
	//��ʼ����������Ҫ���ǳ�ʼ����һ���ͻ������豸����ָ��
	ASSERT(m_pProVDC != NULL);
	
	//����Ӧ����������ظ�ʽ�����뵱ǰ�豸�����������
	if (!bSetupPixelFormat(m_pProVDC->GetSafeHdc()))
		return;
	
	//�õ�ָ���豸����������ģʽ����
	n = ::GetPixelFormat(m_pProVDC->GetSafeHdc());
	
	//��������õ�������ֵ������һ������ģʽ
	::DescribePixelFormat(m_pProVDC->GetSafeHdc(), n, sizeof(pfd), &pfd);
	
	//����һ���������豸����
	m_pProVRC = wglCreateContext(m_pProVDC->GetSafeHdc());
	
	//�������ɵ��豸������ָ����Ϊ��ǰ����
	wglMakeCurrent(m_pProVDC->GetSafeHdc(), m_pProVRC);		
	
	//�úڱ���
//	glClearColor(0.0,0.0,0.0,0.0);
	glClearColor(1.0,1.0,1.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);	
	glShadeModel(GL_SMOOTH);
	
	//��ʼ��������Ϊ RGBA ģʽ��ͬʱ���� alpha ��ϡ���ʾ������
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_BLEND);
	
	
	//������Ȳ���
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); 

	::wglMakeCurrent(NULL, NULL);
}

void CProjectView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	SetViewPort(cx, cy);
	
}

void CProjectView::SetViewPort(int cx, int cy)
{
	this->m_xRect = cx;
	this->m_yRect = cy;

	if (!::wglMakeCurrent(m_pProVDC->GetSafeHdc(),m_pProVRC))
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
		glOrtho(-100.0,100.0,-100.0*(GLfloat)m_yRect/(GLfloat)m_xRect,	100.0*(GLfloat)m_yRect/(GLfloat)m_xRect,-500.0,500.0);
	}
	else
	{
		glOrtho(-100.0*(GLfloat)m_xRect/(GLfloat)m_yRect,100.0*(GLfloat)m_xRect/(GLfloat)m_yRect,-100.0,100.0,-500.0,500.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	::wglMakeCurrent(NULL, NULL);

}

void CProjectView::OnDestroy() 
{
	CView::OnDestroy();
	m_pProVRC = ::wglGetCurrentContext();
	::wglMakeCurrent(NULL,  NULL);
	if (m_pProVRC)
		::wglDeleteContext(m_pProVRC);	
	
}

int CProjectView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	InitOpenGL();
	return 0;
}

void CProjectView::ShowObjects()
{
	if (::gbIsGetData)
		return;
	
	if (::gbDataIsEmpty)
		return;

	//Ϊ�˽����Ƶ������м����������ƽ������ϵ
	glTranslatef(-x_glMid*m_Prox,0.0f,0.0f);	
	glTranslatef(0.0f,-y_glMid*m_Proy,0.0f);
	glTranslatef(0.0f,0.0f,-z_glMid*m_Proz);

	glScalef(m_Prox,m_Proy,m_Proz);

	int i, j;
	switch(this->m_pFr->m_pView->m_ShowType)
	{
	case 0:
		glPointSize(2);
		glBegin(GL_POINTS); 
		for ( j=0; j<m_Vertex.size(); j++)
		{	
			if(!m_Vertex[j].flag)
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
			if(!m_Edge[i].flag)
			{
			//	glColor3f(0.0, 1.0, 1.0);
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

	case 2:
		glBegin(GL_TRIANGLES);
		for( i=0;i<m_Facet.size();i++)
		{
			if(!m_Facet[i].flag)
			{
				glColor3f(m_Facet[i].nx, m_Facet[i].ny, m_Facet[i].nz);
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
	}


	//Ϊ�˽����Ƶ������м����������ƽ������ϵ
	glTranslatef(x_glMid*m_Prox,0.0f,0.0f);	
	glTranslatef(0.0f,y_glMid*m_Proy,0.0f);
	glTranslatef(0.0f,0.0f,z_glMid*m_Proz);

}

void CProjectView::ShowPics(CDC *pDC)
{
	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

	if(this->m_pFr->m_pView->GetDocument()->m_Pixels==NULL)
		return;

	CRect rc;
	GetClientRect(&rc);

	BITMAPINFO *pbmi;
	int palsize;

	palsize = PALETTESIZE(this->m_pFr->m_pView->GetDocument()->m_nBitCount);

	pbmi=(BITMAPINFO *) new BYTE[sizeof(BITMAPINFO)+sizeof(RGBQUAD)*palsize];

	memcpy(pbmi->bmiColors, this->m_pFr->m_pView->GetDocument()->m_lpPalette,sizeof(RGBQUAD)*palsize);

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = this->m_pFr->m_pView->GetDocument()->m_nWidth;
	pbmi->bmiHeader.biHeight = this->m_pFr->m_pView->GetDocument()->m_nHeight;
	pbmi->bmiHeader.biBitCount = this->m_pFr->m_pView->GetDocument()->m_nBitCount;
	pbmi->bmiHeader.biPlanes=1;
	pbmi->bmiHeader.biCompression =BI_RGB;
	pbmi->bmiHeader.biSizeImage = 0;
	pbmi->bmiHeader.biXPelsPerMeter=0;
	pbmi->bmiHeader.biYPelsPerMeter =0;
	pbmi->bmiHeader.biClrUsed = 0;
	pbmi->bmiHeader.biClrImportant =0;

	SetStretchBltMode(*pDC,COLORONCOLOR);

	long RC_temp=0;
	long P_temp =0;
	float zoom;
	int ret;

	if (rc.Width()<rc.Height())
		RC_temp=rc.Width();
	else
		RC_temp=rc.Height();

	nWidth =this->m_pFr->m_pView->GetDocument()->m_nWidth ;
	nHeight =this->m_pFr->m_pView->GetDocument()->m_nHeight;

	if(this->m_pFr->m_pCtrlWnd->m_i>0)
		pic_i=this->m_pFr->m_pCtrlWnd->m_i-1+2;

	BYTE *p=this->m_pFr->m_pView->GetDocument()->m_lpBits;

	this->m_pFr->m_pView->GetDocument()->m_lpBits = this->m_pFr->m_pView->GetDocument()->m_Pixels+nWidth*nHeight*pic_i;

	if (nWidth < nHeight)
	{
		P_temp=nWidth;
		zoom =(float) RC_temp / nHeight;

		ret=StretchDIBits(*pDC,(rc.Width()-RC_temp)/2,0,RC_temp,nHeight*zoom,
			0.2*nWidth,0.2*nHeight,0.8*nWidth,0.8*nHeight,
			this->m_pFr->m_pView->GetDocument()->m_lpBits,pbmi,DIB_RGB_COLORS,SRCCOPY);
	}
	else
	{
		P_temp=nHeight;
		zoom =(float) RC_temp / nWidth;

		ret=StretchDIBits(*pDC,(rc.Width()-RC_temp)/2,0,nWidth*zoom,RC_temp,
			0,0,nWidth,nHeight,
			this->m_pFr->m_pView->GetDocument()->m_lpBits,pbmi,DIB_RGB_COLORS,SRCCOPY);
	}


	this->m_pFr->m_pView->GetDocument()->m_lpBits = p;

}

void CProjectView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if(this->m_pFr->m_pView->GetDocument()->m_nFileNum>0)
	{
		this->m_pFr->m_pCtrlWnd->m_i= (this->m_pFr->m_pCtrlWnd->m_i)%this->m_pFr->m_pView->GetDocument()->m_nFileNum;
		m_bShowBmp=true;
		this->Invalidate();
		this->m_pFr->m_pCtrlWnd->m_PicList.SetCurSel(this->m_pFr->m_pCtrlWnd->m_i);
		this->m_pFr->m_pCtrlWnd->m_i += 1;
	}
	
	CView::OnLButtonDblClk(nFlags, point);
}
