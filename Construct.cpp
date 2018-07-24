// Construct.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "3DRCS.h"
#include "Construct.h"
#include "afxdialogex.h"
#include "IctHeader.h"
#include "PCG.h"

// CConstruct �Ի���

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
	//	DDX_Control(pDX, IDC_EDIT1, CPCG::GetLayerSpace("E:\�о�������\�������\model\pcg\200904221.pcg"));
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


// CConstruct ��Ϣ�������


void CConstruct::OnReleasedcaptureMmcmtthreshold(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	
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
//	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
//	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
//	// ���������� CRichEditCtrl().SetEventMask()��
//	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�
//
//	// TODO:  �ڴ���ӿؼ�֪ͨ����������
//	CString str;
//	this->m_MThresholdEdit.GetWindowText(str);
//	int value=atoi(str);
//	this->m_pFr->m_pView->GetDocument()->m_TargetValue=value;
//	this->m_MMCMTthreshold.SetPos(value);
//}


//void CConstruct::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//{
//	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
//	CString str;
//	str.Format("%ld",m_MMCMTthreshold.GetPos());
//	this->m_MThresholdEdit.SetWindowText(str);
//
//	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
//}


//void CConstruct::OnChangeMxconzoom()
//{
//	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
//	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
//	// ���������� CRichEditCtrl().SetEventMask()��
//	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�
//
//	// TODO:  �ڴ���ӿؼ�֪ͨ����������
//	CString   str; 
//	GetDlgItem(IDC_MXCONZOOM)->GetWindowText(str);
//	m_MXconZoom = (float)atof(str);
//}


//void CConstruct::OnChangeMyconzoom()
//{
//	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
//	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
//	// ���������� CRichEditCtrl().SetEventMask()��
//	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�
//
//	// TODO:  �ڴ���ӿؼ�֪ͨ����������
//	CString   str; 
//	GetDlgItem(IDC_MYCONZOOM)->GetWindowText(str);
//	m_MYconZoom = (float)atof(str);
//}


//void CConstruct::OnChangeMzconzoom()
//{
//	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
//	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
//	// ���������� CRichEditCtrl().SetEventMask()��
//	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�
//
//	// TODO:  �ڴ���ӿؼ�֪ͨ����������
//	CString   str; 
//	GetDlgItem(IDC_MZCONZOOM)->GetWindowText(str);
//	m_MZconZoom = (float)atof(str);
//}


//void CConstruct::OnClickedMcheckring()
//{
//	// TODO: �ڴ���ӿؼ�֪ͨ����������
//	this->m_pFr->m_pCtrlWnd->m_bDeleteRing =!this->m_pFr->m_pCtrlWnd->m_bDeleteRing;
//}


BOOL CConstruct::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
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
	// �쳣: OCX ����ҳӦ���� FALSE
}


void CConstruct::OnClickedMcheckring()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_MbDeleteRing=!m_MbDeleteRing;
}


void CConstruct::OnChangeMthresholdedit()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	m_MMCMTthreshold.SetPos(m_MThreshold);
}




