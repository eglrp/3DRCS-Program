#pragma once


// CAdaptive �Ի���

class CAdaptive : public CDialog
{
	DECLARE_DYNAMIC(CAdaptive)

public:
	CAdaptive(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAdaptive();

// �Ի�������
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	float m_reducerate3;
	afx_msg void OnBnClickedOk();
};
