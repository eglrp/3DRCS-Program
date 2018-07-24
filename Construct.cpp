// Construct.cpp : 实现文件
//

#include "stdafx.h"
#include "3DRCS.h"
#include "Construct.h"
#include "afxdialogex.h"
#include "IctHeader.h"
#include "PCG.h"

// CConstruct 对话框

IMPLEMENT_DYNAMIC(CConstruct, CDialog)

CConstruct::CConstruct(CWnd* pParent /*=NULL*/)
	: CDialog(CConstruct::IDD, pParent)
{

	m_MXconZoom = 0.0f;
	m_MYconZoom = 0.0f;
	m_MZconZoom = 0.0f;
	m_MThreshold = 0;
	m_MMCMTthresholdvalue = 0;
	m_ScanParam = 0.0f;
	
}

CConstruct::~CConstruct()
{
}

void CConstruct::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MMCMTTHRESHOLD, m_MMCMTthreshold);
	DDX_Text(pDX, IDC_MXCONZOOM, m_MXconZoom);
	DDX_Text(pDX, IDC_MYCONZOOM, m_MYconZoom);
	DDX_Text(pDX, IDC_MZCONZOOM, m_MZconZoom);
	//  DDX_Control(pDX, IDC_MTHRESHOLDEDIT, m_MThresholdEdit);
	DDX_Text(pDX, IDC_MTHRESHOLDEDIT, m_MThreshold);
	DDX_Slider(pDX, IDC_MMCMTTHRESHOLD, m_MMCMTthresholdvalue);
	//	DDX_Control(pDX, IDC_EDIT1, CPCG::GetLayerSpace("E:\研究生工作\论文相关\model\pcg\200904221.pcg"));
	DDX_Text(pDX, IDC_VIEWDIAMETER, m_ScanParam);
}


BEGIN_MESSAGE_MAP(CConstruct, CDialog)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_MMCMTTHRESHOLD, &CConstruct::OnReleasedcaptureMmcmtthreshold)
//	ON_EN_CHANGE(IDC_MTHRESHOLDEDIT, &CConstruct::OnChangeMthresholdedit)
//	ON_WM_HSCROLL()
//	ON_EN_CHANGE(IDC_MXCONZOOM, &CConstruct::OnChangeMxconzoom)
//	ON_EN_CHANGE(IDC_MYCONZOOM, &CConstruct::OnChangeMyconzoom)
//	ON_EN_CHANGE(IDC_MZCONZOOM, &CConstruct::OnChangeMzconzoom)
//	ON_BN_CLICKED(IDC_MCHECKRING, &CConstruct::OnClickedMcheckring)
ON_BN_CLICKED(IDC_MCHECKRING, &CConstruct::OnClickedMcheckring)
ON_EN_CHANGE(IDC_MTHRESHOLDEDIT, &CConstruct::OnChangeMthresholdedit)
//ON_EN_CHANGE(IDC_EDIT1, &CConstruct::OnEnChangeEdit1)
//ON_EN_CHANGE(IDC_MXCONZOOM, &CConstruct::OnEnChangeMxconzoom)
END_MESSAGE_MAP()


// CConstruct 消息处理程序


void CConstruct::OnReleasedcaptureMmcmtthreshold(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	
	UpdateData(TRUE); 
	if(m_MMCMTthreshold.GetPos()!=0)
	//valueyu=m_MMCMTthreshold.GetPos();
	m_MThreshold=m_MMCMTthreshold.GetPos();
	//m_MMCMTthreshold.SetPos(m_MThreshold);
	
	UpdateData(FALSE);
	
	*pResult = 0;
}


//void CConstruct::OnChangeMthresholdedit()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialog::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	CString str;
//	this->m_MThresholdEdit.GetWindowText(str);
//	int value=atoi(str);
//	this->m_pFr->m_pView->GetDocument()->m_TargetValue=value;
//	this->m_MMCMTthreshold.SetPos(value);
//}


//void CConstruct::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//	CString str;
//	str.Format("%ld",m_MMCMTthreshold.GetPos());
//	this->m_MThresholdEdit.SetWindowText(str);
//
//	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
//}


//void CConstruct::OnChangeMxconzoom()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialog::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	CString   str; 
//	GetDlgItem(IDC_MXCONZOOM)->GetWindowText(str);
//	m_MXconZoom = (float)atof(str);
//}


//void CConstruct::OnChangeMyconzoom()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialog::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	CString   str; 
//	GetDlgItem(IDC_MYCONZOOM)->GetWindowText(str);
//	m_MYconZoom = (float)atof(str);
//}


//void CConstruct::OnChangeMzconzoom()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialog::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	CString   str; 
//	GetDlgItem(IDC_MZCONZOOM)->GetWindowText(str);
//	m_MZconZoom = (float)atof(str);
//}


//void CConstruct::OnClickedMcheckring()
//{
//	// TODO: 在此添加控件通知处理程序代码
//	this->m_pFr->m_pCtrlWnd->m_bDeleteRing =!this->m_pFr->m_pCtrlWnd->m_bDeleteRing;
//}


BOOL CConstruct::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	 m_MMCMTthreshold.SetRange(0,255);
    m_MMCMTthreshold.SetPos(0);
    m_MMCMTthreshold.SetTicFreq(5);
	m_MMCMTthreshold.SetPageSize(10);
	m_MMCMTthreshold.SetLineSize(2);
	m_MMCMTthresholdvalue=m_MMCMTthreshold.GetPos();
	m_MThreshold=m_MMCMTthreshold.GetPos();
	m_MbDeleteRing=TRUE;

	//float  spacedistance;
	//char ch[100];
	//SCANPARAMETER sp;
	//spacedistance = sp.ViewDiameter;
	//itoa(spacedistance, ch, 10);
	//GetDlgItem(IDC_VIEWDIAMETER)->SetWindowText(ch);
	
	CButton* btn = (CButton*)GetDlgItem(IDC_MCHECKRING);
	btn->SetCheck(1);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CConstruct::OnClickedMcheckring()
{
	// TODO: 在此添加控件通知处理程序代码
	m_MbDeleteRing=!m_MbDeleteRing;
}


void CConstruct::OnChangeMthresholdedit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(true);
	m_MMCMTthreshold.SetPos(m_MThreshold);
}




