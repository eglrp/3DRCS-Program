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
extern long    m_FacetNumber;        //������Ƭ������
extern long   m_EdgeNumber;         //�������
extern long    m_VertexNumber;       //�������

extern float x_glMid,x_glMin,x_glMax;
extern float y_glMid,y_glMin,y_glMax;
extern float z_glMid,z_glMin,z_glMax;

extern vector<Vertex> m_Vertex;   //�����ݼ�
extern vector<Edge>   m_Edge;     //�����ݼ�
extern vector<Facet>  m_Facet;    //������Ƭ�����ݼ�


class C3DRCSView : public CScrollView
{
protected: // create from serialization only
	C3DRCSView();
	DECLARE_DYNCREATE(C3DRCSView)
	
	CMainFrame *m_pFr;
	CStatusBar* pStatus;//��ȡ�����״̬��
	CPoint  m_MouseDown, m_MouseMove, m_MouseUp;

	
	int		nTurnType;            //ȷ��ͼ��Χ����ת0X,1Y,2Z;
	long	m_xM,m_yM;            //��¼�������ʱ���ĺ�������
	long	m_xRect, m_yRect;     //��ǰ���ڵĴ�С
		
	bool	m_bDown;              //�������Ƿ���
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


	/*�жϰ�ť����������*/
	bool m_bPICS_OPEN;//����ͼƬ��������ʶ
	bool m_bFILE_OPEN;//���ļ���ʶ
	bool m_bFILE_SAVE;//�����ļ�
	bool m_bCLEAR_DATA;//����ļ�
	bool m_bRELOAD_DATA;//��������
	bool m_bMC_CONSTRUCT;//MC
	bool m_bMT_CONSTRUCT;//MT
	bool m_bREDUCE_MESH;//��
	bool m_bSMOOTH_Mesh;//ƽ��
	bool m_bREDUCE_MESH3;//����Ӧ��
	bool m_bZOOMADD;//�Ŵ�
	bool m_bZOOMDEL;//��С
	bool m_bSHOWMEDIUM;//����
	bool m_bZOOMBACK;//��ԭ

	int     m_ShowType;           //��ʾ���ͣ�����(0)���߿�(1)����Ƭ(2)
	int     m_MouseAction;        //�жϵ�ǰѡ�е���ѡ��ť������ת��ť 0Ϊ��ת,1Ϊѡȡ, 2Ϊƽ��
	double  xx,yy,zz;
	CPoint   m_cpStart;     //���   
	CPoint   m_cpOld;         //�յ�

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
