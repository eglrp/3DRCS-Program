#pragma once


// CSmooth �Ի���

class CSmooth : public CDialog
{
	DECLARE_DYNAMIC(CSmooth)

public:
	CSmooth(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSmooth();

// �Ի�������
	enum { IDD = IDD_DIALOG4 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	float m_MnSmoothNum;
	float m_MnSmoothRate;
};
