#if !defined(AFX_PROJECTVIEW_H__3BBD1605_8929_46C7_8F1C_C4C15093C7D3__INCLUDED_)
#define AFX_PROJECTVIEW_H__3BBD1605_8929_46C7_8F1C_C4C15093C7D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProjectView.h : header file
//
#include "Vertex.h"
#include "Edge.h"
#include "Facet.h"
/////////////////////////////////////////////////////////////////////////////
// CProjectView view
extern vector<Vertex> m_Vertex;   //点数据集
extern vector<Edge>   m_Edge;     //边数据集
extern vector<Facet>  m_Facet;    //三角面片的数据集

class CMainFrame;
class CProjectView : public CView
{
protected:
	CProjectView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CProjectView)
	CMainFrame *m_pFr;

	CRect	m_oldRect;
	long	m_xRect, m_yRect;     //当前窗口的大小
	int     nWidth ;
	int     nHeight;
	int     pic_i;

public:
	bool	m_bShowBmp;

// Attributes
public:
// Operations
public:
	void ShowPics(CDC *pDC);
	void ShowObjects();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CProjectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CProjectView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SetViewPort(int cx, int cy);
	void InitOpenGL();
	BOOL bSetupPixelFormat(HDC hdc);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTVIEW_H__3BBD1605_8929_46C7_8F1C_C4C15093C7D3__INCLUDED_)
