#if !defined(AFX_CONTROLWND_H__C2C40E5D_B598_49A3_8717_4C215A4C5A05__INCLUDED_)
#define AFX_CONTROLWND_H__C2C40E5D_B598_49A3_8717_4C215A4C5A05__INCLUDED_

#include "MainFrm.h"
//#include "WBButton.h"
#include "SaveDlg.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ControlWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CControlWnd form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

//class CPicDataWnd;
class CControlWnd : public CFormView
{
protected:
	CControlWnd();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CControlWnd)
	CMainFrame *m_pFr;		//主框架的指针
	int m_nPicCount;        //读取图像文件的个数

public:
	int     m_i;
	float	m_zZoom;
	float	m_yZoom;
	float	m_xZoom;
	bool    m_bShowCoordinate;    //是否显示坐标系统
	bool    m_bGetPics;
	bool	m_bLighting;          //是否使用光照效果
	bool    m_bDeleteRing;        //是否去除外环
	bool    m_bShowReverse;       //是否将三角面片法向取反（显示）
		
// Form Data

private:
	
	CString m_ConTypeString;
	bool    m_SpinWnd;

public:
	//{{AFX_DATA(CControlWnd)
	enum { IDD = IDD_CONTROLDLG };

	CComboBox	m_ConstructType;
	
	CButton	m_ZoomBack;
	CButton	m_ZoomDel;
	CButton	m_ZoomAdd;
	CButton	m_ShowXpWnd;
	CButton	m_ReloadDataWnd;
	CButton	m_DivideMode;
	CButton	m_CloseWnd;
	CButton	m_ModeInfo;
	CButton	m_OpenPicbtn;
	CButton	m_ShowPara;
	CButton	m_Clearbtn;
	CButton	m_SmoothBtn;
	CButton	m_MeshPatchbtn;
	CButton	m_ReduceMeshbtn;
	CButton	m_GetDatabtn;
	CButton	m_SaveFile;

	CSliderCtrl	m_MCMTthreshold;
	CSliderCtrl pSlidCtrlX;
	CSliderCtrl pSlidCtrlY;
	CSliderCtrl pSlidCtrlZ;
	
	CListBox	m_PicList;

	CEdit	    m_FacetNum;
	CEdit	    m_PointNum;
	CEdit       m_EdgeNum;
	CEdit	    m_ReduceR;
	CEdit	    m_ReduceAngleThresholdEdit;
	CEdit	    m_ThresholdEdit;
	CEdit	    m_ReduceDistanceThresholdEdit;

	float		m_nxZoom;
	float		m_nyZoom;
	float		m_nzZoom;
	float		m_nReduceAngleThreshold;
	float	    m_XconZoom;
	float	    m_YconZoom;
	float	    m_ZconZoom;
	float	    m_nReduceRate;
	float	    m_nReduceDistanceThreshold;
	float	    m_nSmoothRate;

	int 	    m_nSmoothNum;
	//}}AFX_DATA
// Attributes
public:
	
// Operations
public:
	void EnablControlWhileConstruct();
	void DisableControlWhileConstruct();
	void Clear();
	void SetXYZzoom();
	void OpenSTL();
	void OnClearData1() ;

	LRESULT GetData(WPARAM wParam,LPARAM lParam);
	LRESULT DisableButton(WPARAM wParam,LPARAM lParam);
	LRESULT EnableButton(WPARAM wParam,LPARAM lParam);
	//void FillListBox();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CControlWnd)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CControlWnd();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CControlWnd)
	afx_msg void OnCancel() {return;};
	afx_msg void OnOK() {return;};
	afx_msg void OnDblclkAllvalue();
	afx_msg void OnMeshPatch();
	afx_msg void OnRadioturn();
	afx_msg void OnRadioselect();
	afx_msg void OnGetdata();
	afx_msg void OnClearData();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnCloseWnd();
	afx_msg void OnCheckcooldinate();
	afx_msg void OnRadioturnx();
	afx_msg void OnRadioturny();
	afx_msg void OnRadioturnz();
	afx_msg void OnShowMedium();
	afx_msg void OnZoomAdd();
	afx_msg void OnZoomdel();
	afx_msg void OnZoomback();
	afx_msg void OnChecklighting();
	afx_msg void OnSmoothMesh();
	afx_msg void OnCheckFacetreverse();
	afx_msg void OnSavefile();
	afx_msg void OnUpdateZzoom();
	afx_msg void OnUpdateYzoom();
	afx_msg void OnUpdateXzoom();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVertexes();
	afx_msg void OnFacets();
	afx_msg void OnEdges();
	afx_msg void OnReducemesh();
	afx_msg void OnRadiotranslate();
	afx_msg void OnReloadData();
	afx_msg void OnDividemode();
	afx_msg void OnShowpara();
	afx_msg void OnModeinfo();
	afx_msg void OnOpenPics();
	afx_msg void OnReleasedcaptureMcmtthreshold(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeZconzoom();
	afx_msg void OnChangeXconzoom();
	afx_msg void OnChangeYconzoom();
	afx_msg void OnChangeThresholdedit();
//	afx_msg void OnPatchmesh();
	afx_msg void OnCheckring();
	afx_msg void OnDeltaposSpinwond(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditchangeConstructtype();
	afx_msg void OnReduceMesh2();
	afx_msg void OnCalStandard();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedReduceMesh3();
	float m_nReduceAreaThreshold;
	float m_nReduceVolumeThreshold;
	CEdit m_ReduceAreaThresholdEdit;
	CEdit m_ReduceVolumeThresholdEdit;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTROLWND_H__C2C40E5D_B598_49A3_8717_4C215A4C5A05__INCLUDED_)
