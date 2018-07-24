#if !defined(AFX_HOLEFILL_H__0D9BA88B_5314_408F_AE33_5CAB560E55FC__INCLUDED_)
#define AFX_HOLEFILL_H__0D9BA88B_5314_408F_AE33_5CAB560E55FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HoleFill.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// HoleFill dialog

class HoleFill : public CDialog
{
// Construction
public:
	HoleFill(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(HoleFill)
	enum { IDD = IDD_DIALOG1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HoleFill)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(HoleFill)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HOLEFILL_H__0D9BA88B_5314_408F_AE33_5CAB560E55FC__INCLUDED_)
