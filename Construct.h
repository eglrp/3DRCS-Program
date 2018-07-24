#pragma once

#include "MainFrm.h"
#include "afxwin.h"
// CConstruct 对话框

class CConstruct : public CDialog
{
	DECLARE_DYNAMIC(CConstruct)
protected:
	CMainFrame *m_pFr;		//主框架的指针
public:
	CConstruct(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CConstruct();

// 对话框数据
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CSliderCtrl m_MMCMTthreshold;
	float m_MXconZoom;
	float m_MYconZoom;
	float m_MZconZoom;
//	CEdit m_MThresholdEdit;
	afx_msg void OnReleasedcaptureMmcmtthreshold(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnChangeMthresholdedit();
//	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
//	afx_msg void OnChangeMxconzoom();
//	afx_msg void OnChangeMyconzoom();
//	afx_msg void OnChangeMzconzoom();
//	afx_msg void OnClickedMcheckring();
	int m_MThreshold;
	int m_MMCMTthresholdvalue;
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedMcheckring();
	bool m_MbDeleteRing;
	afx_msg void OnChangeMthresholdedit();
	
	float m_ScanParam;
};
