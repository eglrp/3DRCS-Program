#if !defined(AFX_SHOWPARADLG_H__2A927E82_CE5C_4093_8CF0_1D894EB6C3D7__INCLUDED_)
#define AFX_SHOWPARADLG_H__2A927E82_CE5C_4093_8CF0_1D894EB6C3D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShowParaDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CShowParaDlg dialog

class CShowParaDlg : public CDialog
{
// Construction
public:
	CShowParaDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CShowParaDlg)
	enum { IDD = IDD_SHOWPARADLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShowParaDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShowParaDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHOWPARADLG_H__2A927E82_CE5C_4093_8CF0_1D894EB6C3D7__INCLUDED_)
