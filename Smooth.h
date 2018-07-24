#pragma once


// CSmooth 对话框

class CSmooth : public CDialog
{
	DECLARE_DYNAMIC(CSmooth)

public:
	CSmooth(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSmooth();

// 对话框数据
	enum { IDD = IDD_DIALOG4 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	float m_MnSmoothNum;
	float m_MnSmoothRate;
};
