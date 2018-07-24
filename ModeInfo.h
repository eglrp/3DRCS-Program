#if !defined(AFX_MODEINFO_H__BA897700_3206_4A0F_85BC_31E88B3FECC0__INCLUDED_)
#define AFX_MODEINFO_H__BA897700_3206_4A0F_85BC_31E88B3FECC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModeInfo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CModeInfo dialog

class CMainFrame;
class CModeInfo : public CDialog
{
// Construction
public:
	CModeInfo(CWnd* pParent = NULL);   // standard constructor

	CMainFrame *m_pFr;

// Dialog Data
	//{{AFX_DATA(CModeInfo)
	enum { IDD = IDD_MODEINFODLG };
	float	m_nxMax;
	float	m_nxMid;
	float	m_nxMin;
	float	m_nyMax;
	float	m_nyMid;
	float	m_nyMin;
	float	m_nzMax;
	float	m_nzMid;
	float	m_nzMin;
	float	m_xSpan;
	float	m_ySpan;
	float	m_zSpan;
	float	m_ModeSurfaceArea;
	float	m_ModeVolume;
	//}}AFX_DATA
public:
	void SetEdits();
/*	float x_Min, x_Mid, x_Max;
	float y_Min, y_Mid, y_Max;
	float z_Min, z_Mid, z_Max;*/

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModeInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModeInfo)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	float m_edgenum;
	float m_facetnum;
//	float m_vetex;
	float m_vetexnum;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODEINFO_H__BA897700_3206_4A0F_85BC_31E88B3FECC0__INCLUDED_)
