#pragma once


// CReduce 对话框

class CReduce : public CDialog
{
	DECLARE_DYNAMIC(CReduce)

public:
	CReduce(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CReduce();

// 对话框数据
	enum { IDD = IDD_DIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
//	float m_reducerate3;
	float m_MnReduceAngleThreshold;
	float m_MnReduceDistanceThreshold;
	float m_reducerate2;
};
