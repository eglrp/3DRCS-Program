#pragma once

#include "MainFrm.h"
#include "afxwin.h"
// CConstruct �Ի���

class CConstruct : public CDialog
{
	DECLARE_DYNAMIC(CConstruct)
protected:
	CMainFrame *m_pFr;		//����ܵ�ָ��
public:
	CConstruct(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CConstruct();

// �Ի�������
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
