#pragma once


// CReduce �Ի���

class CReduce : public CDialog
{
	DECLARE_DYNAMIC(CReduce)

public:
	CReduce(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CReduce();

// �Ի�������
	enum { IDD = IDD_DIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
//	float m_reducerate3;
	float m_MnReduceAngleThreshold;
	float m_MnReduceDistanceThreshold;
	float m_reducerate2;
};
