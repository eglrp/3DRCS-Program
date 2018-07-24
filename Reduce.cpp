// Reduce.cpp : 实现文件
//

#include "stdafx.h"
#include "3DRCS.h"
#include "Reduce.h"
#include "afxdialogex.h"


// CReduce 对话框

IMPLEMENT_DYNAMIC(CReduce, CDialog)

CReduce::CReduce(CWnd* pParent /*=NULL*/)
	: CDialog(CReduce::IDD, pParent)
{

	//  m_reducerate3 = 0.0f;
	m_MnReduceAngleThreshold = 0.0f;
	m_MnReduceDistanceThreshold = 0.0f;
	m_reducerate2 = 0.0f;
}

CReduce::~CReduce()
{
}

void CReduce::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//  DDX_Text(pDX, IDC_REDUCERATE2, m_reducerate3);
	DDX_Text(pDX, IDC_MANGLE_THRESHOLD, m_MnReduceAngleThreshold);
	DDX_Text(pDX, IDC_MDISTANCE_THRESHOLD, m_MnReduceDistanceThreshold);
	DDX_Text(pDX, IDC_REDUCERATE2, m_reducerate2);
}


BEGIN_MESSAGE_MAP(CReduce, CDialog)
END_MESSAGE_MAP()


// CReduce 消息处理程序
