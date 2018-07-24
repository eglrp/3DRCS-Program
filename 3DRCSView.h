// 3DRCSView.h : interface of the C3DRCSView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_3DRCSVIEW_H__5C9CAAD1_A88D_46D3_A756_DC509BDDBE2D__INCLUDED_)
#define AFX_3DRCSVIEW_H__5C9CAAD1_A88D_46D3_A756_DC509BDDBE2D__INCLUDED_
#include "Facet.h"
#include "Edge.h"
#include "Vertex.h"
#include "3DRCSDoc.h"
#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace std;

class CMainFrame;

extern CDC *   m_pMainVDC;
extern HGLRC   m_pMainVRC;
extern long    m_FacetNumber;        //三角面片的总数
extern long   m_EdgeNumber;         //半边总数
extern long    m_VertexNumber;       //点的总数

extern float x_glMid,x_glMin,x_glMax;
extern float y_glMid,y_glMin,y_glMax;
extern float z_glMid,z_glMin,z_glMax;

extern vector<Vertex> m_Vertex;   //点数据集
extern vector<Edge>   m_Edge;     //边数据集
extern vector<Facet>  m_Facet;    //三角面片的数据集


class C3DRCSView : public CScrollView
{
protected: // create from serialization only
	C3DRCSView();
	DECLARE_DYNCREATE(C3DRCSView)
	
	CMainFrame *m_pFr;
	CStatusBar* pStatus;//获取框架类状态栏
	CPoint  m_MouseDown, m_MouseMove, m_MouseUp;

	
	int		nTurnType;            //确定图形围那轴转0X,1Y,2Z;
	long	m_xM,m_yM;            //记录鼠标落下时落点的横纵坐标
	long	m_xRect, m_yRect;     //当前窗口的大小
		
	bool	m_bDown;              //鼠标左键是否按下
	bool m_bShowEdge;
	bool m_bShowFacet;
	bool m_bShowVertex;
	bool m_bShowFacetEdge;
	bool m_bTurnx;
	bool m_bTurny;
	bool m_bTurnz;
	bool m_bTurn;
	bool m_bTranslate;
	bool m_bZoomout;


	/*判断按钮禁用与启动*/
	bool m_bPICS_OPEN;//加载图片可用与否标识
	bool m_bFILE_OPEN;//打开文件标识
	bool m_bFILE_SAVE;//保存文件
	bool m_bCLEAR_DATA;//清空文件
	bool m_bRELOAD_DATA;//重载数据
	bool m_bMC_CONSTRUCT;//MC
	bool m_bMT_CONSTRUCT;//MT
	bool m_bREDUCE_MESH;//简化
	bool m_bSMOOTH_Mesh;//平滑
	bool m_bREDUCE_MESH3;//自适应简化
	bool m_bZOOMADD;//放大
	bool m_bZOOMDEL;//缩小
	bool m_bSHOWMEDIUM;//适中
	bool m_bZOOMBACK;//还原

	int     m_ShowType;           //显示类型：点云(0)、线框(1)或面片(2)
	int     m_MouseAction;        //判断当前选中的是选择按钮还是旋转按钮 0为旋转,1为选取, 2为平移
	double  xx,yy,zz;
	CPoint   m_cpStart;     //起点   
	CPoint   m_cpOld;         //终点

	float oldx1,oldy1,oldx2,oldy2;
	float newx1,newy1,newx2,newy2;

	int m_nSliderCurX;
	int m_nSliderCurY;
	int m_nSliderCurZ;

	CRect   m_SelRect;

// Attributes
public:
	C3DRCSDoc* GetDocument();
	bool IsMouseInWndRect(CPoint point);
private:
	

public:
	
	
// Operations
public:


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(C3DRCSView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	void KillTime();
	void SetTime();
	void Reback();
	void SetViewPort();
	void SetLighting();
	void DrawText();
	void DrawSelArea();
	void ShowObjects();
	void DrawLines();
	void IniOpenGL();
	bool bSetPixelFormat();
	virtual ~C3DRCSView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	

// Generated message map functions
protected:
	//{{AFX_MSG(C3DRCSView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTurnx();
	afx_msg void OnTurny();
	afx_msg void OnTurnz();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFacet();
	afx_msg void OnEdge();
	afx_msg void OnVertex();
	afx_msg void OnTurn();
	afx_msg void OnTranslate();
	afx_msg void OnZoomout();
	afx_msg void OnUpdateEdge(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFacet(CCmdUI *pCmdUI);
	afx_msg void OnUpdateVertex(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTurnx(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTurny(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTurnz(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTurn(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTranslate(CCmdUI *pCmdUI);
	afx_msg void OnUpdateZoomout(CCmdUI *pCmdUI);
	afx_msg void OnFacetedge();
	afx_msg void OnUpdateFacetedge(CCmdUI *pCmdUI);
//	afx_msg void OnRepairhole();
//	afx_msg void OnRepairhole();
};

#ifndef _DEBUG  // debug version in 3DRCSView.cpp
inline C3DRCSDoc* C3DRCSView::GetDocument()
   { return (C3DRCSDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_3DRCSVIEW_H__5C9CAAD1_A88D_46D3_A756_DC509BDDBE2D__INCLUDED_)
