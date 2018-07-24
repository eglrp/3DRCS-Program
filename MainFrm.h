// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__98BEE19C_D252_4AB2_9C74_ABC8D7AD9049__INCLUDED_)
#define AFX_MAINFRM_H__98BEE19C_D252_4AB2_9C74_ABC8D7AD9049__INCLUDED_

#include "3DRCSView.h"
#include "Adaptive.h"
#include "Construct.h"
#include "Reduce.h"
#include "Smooth.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CControlWnd;
class CProjectView;
class CConstruct;
class CModeInfo;

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)
//	CMainFrame *m_pFr;
// Attributes
public:
	C3DRCSView		*m_pView;		//��ά�ռ䴰��
	CControlWnd		*m_pCtrlWnd;	//����̨����
	CProjectView	*m_pProView;	//������άͶӰ����
	//CConstruct      *m_construct;     //�ؽ��������öԻ���

	CFont           m_13Font;       //����
	CProgressCtrl	m_Progress;     //������
	BOOL	        m_bCreat;       //�������Ƿ��Ѵ��� 
	bool            m_bShowToolbar; //�Ƿ���ʾ������
	bool            m_bShowLight;//�Ƿ���ʾ����
	bool            m_bFacetreverse;//�Ƿ���Ƭ����
	bool           m_bShowcoordinate;//�Ƿ���ʾ����
	CToolBar     m_wndToolBar,m_ToolBar1;   //������
	CStatusBar      m_wndStatusBar; //״̬��
// Operations
public:


private:
	BOOL CreateExToolBar();   //����������
	void CreateFont();        //��������
	void CreateProcess();     //����������
	void GetWndPointer();     //��ȡ�������ָ��

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	void InvalidateAllWnd();
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
//	CReBar m_wndReBar;
	CSplitterWnd    m_Sp1,m_Sp2;  //�ָ���ĸ�����

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnViewToolbar();
	afx_msg void OnViewStatusBar();
	afx_msg void OnFileOpenSTL();
	afx_msg void OnPSOIntroduction();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPicsOpen();
	afx_msg void OnMcConstruct();
	afx_msg void OnMtConstruct();
	afx_msg void OnReduceMesh3();
	float m_ReduceRate3;
	afx_msg void OnClearData();
	afx_msg void OnReduceMesh();
	float m_ReduceRate2;
	float m_AngleThreshold;
	float m_DistanceRate;
	afx_msg void OnSmoothMesh();
	float m_msmoothrate;
	float m_msmoothnum;
	afx_msg void OnReloadData();
//	afx_msg void OnUpdateFacet(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateEdge(CCmdUI *pCmdUI);
	afx_msg void OnChecklighting();
	afx_msg void OnUpdateChecklighting(CCmdUI *pCmdUI);
	afx_msg void OnFacetreverse();
	afx_msg void OnUpdateFacetreverse(CCmdUI *pCmdUI);
	afx_msg void OnCheckcooldinate();
	afx_msg void OnUpdateCheckcooldinate(CCmdUI *pCmdUI);
	afx_msg void OnZoomadd();
	afx_msg void OnZoomdel();
	afx_msg void OnShowmedium();
	afx_msg void OnZoomback();
	afx_msg void OnModinfo();
	afx_msg void OnCalstandard();
	afx_msg void OnRepairhole();
//	afx_msg void OnMyMethod();
	afx_msg void OnPatchholes();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__98BEE19C_D252_4AB2_9C74_ABC8D7AD9049__INCLUDED_)
