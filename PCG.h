#if !defined(AFX_PCG_H__15225275_BD43_4678_878E_5950B23F57EC__INCLUDED_)
#define  AFX_PCG_H__15225275_BD43_4678_878E_5950B23F57EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include <Windows.h>
#include <stdio.h>
#include "IctHeader.h"
#include <vector>
using namespace std;

class CPCG
{
public:
	CPCG(void);
	virtual ~CPCG(void);
	void Clear();
private:
	// ͼ������ָ��
	BYTE *m_lpBits;
	// 1-CT,2-DR,3-elde
	int ImageType;
	// ͼ��Ŀ�ȣ����أ�
	int m_nWidth;
	// ͼ��ĸ߶ȣ����أ�
	int m_nHeight;
	// ͼ���б�ʾÿ�������õ�λ��
	int m_nBitCount;
	// ��ɫ��ָ��
	BYTE *m_lpPalette;
	// ���ݴ洢���ͣ�char=01��int=02��unsigned long=03��float=04��double=05
	WORD m_DataType;
	ICT_HEADER22 CTHeader;
public:
	bool IsEmpty();       // �ж�ͼ�������Ƿ�Ϊ��
	bool SaveToFile(CFile *pf);
	vector<vector<float>> m_fData;
	vector<vector<float>> m_OldData;  //m_fData���ڱ任��oddData���ڲ�����ԭʼ���ݱ���
	void LoadDLL(CString &NumberStr, int Count);
	void Hd21ToHd22(ICT_HEADER22 * hd21, ICT_HEADER22 * hd22);
	DWORD GetNumColors() const;
	long GetSize();
	long GetImageSize();
	bool OnShow(CDC* pDC);
	bool SaveToFile(LPCTSTR filename);
	float GetLayerSpace(LPCTSTR lpszPathName);
	float GetViewDiameter(LPCTSTR lpszPathName);
	int GetWidth()
	{
		return m_nWidth;
	}
	int GetHeight()
	{
		return m_nHeight;
	}
	int GetBitCount()
	{
		return m_nBitCount;
	}
};
#endif