#if !defined(AFX_SAVEDLG_H__05D88C4C_F770_48AE_ACF7_E0E8B8E60A48__INCLUDED_)
#define AFX_SAVEDLG_H__05D88C4C_F770_48AE_ACF7_E0E8B8E60A48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaveDlg dialog

class CSaveDlg : public CDialog
{
// Construction
public:
	void GetSavePara();
	CSaveDlg(CWnd* pParent = NULL);   // standard constructor
	bool m_ReverseFacet;
	

// Dialog Data
	//{{AFX_DATA(CSaveDlg)
	enum { IDD = IDD_SAVEDLG };
	CEdit	m_SavePathEdit;
	CString m_SaveAsPath;
	float	m_xTranslate;
	float	m_xZoomSave;
	float	m_yZoomSave;
	float	m_zZoomSave;
	float	m_yTranslate;
	float	m_zTranslate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSaveDlg)
	afx_msg void OnSavePath();
	afx_msg void OnSavereversefacet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEDLG_H__05D88C4C_F770_48AE_ACF7_E0E8B8E60A48__INCLUDED_)
