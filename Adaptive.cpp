// Adaptive.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "3DRCS.h"
#include "Adaptive.h"
#include "afxdialogex.h"


// CAdaptive �Ի���

IMPLEMENT_DYNAMIC(CAdaptive, CDialog)

CAdaptive::CAdaptive(CWnd* pParent /*=NULL*/)
	: CDialog(CAdaptive::IDD, pParent)
{

	m_reducerate3 = 0.0f;
}

CAdaptive::~CAdaptive()
{
}

void CAdaptive::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_REDUCERATE3, m_reducerate3);
}


BEGIN_MESSAGE_MAP(CAdaptive, CDialog)
END_MESSAGE_MAP()


// CAdaptive ��Ϣ�������
