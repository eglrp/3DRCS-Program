// Smooth.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "3DRCS.h"
#include "Smooth.h"
#include "afxdialogex.h"


// CSmooth �Ի���

IMPLEMENT_DYNAMIC(CSmooth, CDialog)

CSmooth::CSmooth(CWnd* pParent /*=NULL*/) 
	: CDialog(CSmooth::IDD, pParent)
{

	m_MnSmoothNum = 0.0f;
	m_MnSmoothRate = 0.0f;
}

CSmooth::~CSmooth()
{
}

void CSmooth::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MSMOOTH_NUM, m_MnSmoothNum);
	DDX_Text(pDX, IDC_MSMOOTH_RATE, m_MnSmoothRate);
}


BEGIN_MESSAGE_MAP(CSmooth, CDialog)
END_MESSAGE_MAP()


// CSmooth ��Ϣ�������
