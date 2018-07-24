#include "StdAfx.h"
#include "PCG.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define BYTE_PER_LINE(w,c) ((((w)*(c)+31)/32)*4)

#define PALETTESIZE(b) (((b) == 8)) ? 256:(((b)==4)?16:0)

CPCG::CPCG(void)
	: m_lpBits(NULL)
	, ImageType(0)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_nBitCount(0)
	, m_lpPalette(NULL)
{
}


CPCG::~CPCG(void)
{
}


void CPCG::Clear()
{
	//�ͷ�ͼ������
	if(m_lpBits)
		delete[] m_lpBits;
	m_lpBits = NULL;

	//�ͷŵ�ɫ������
	if(m_lpPalette)
		delete[] m_lpPalette;
	m_lpPalette = NULL;

	m_nHeight = m_nWidth = 0;
	m_nBitCount = 0;
}


// �ж�ͼ�������Ƿ�Ϊ��
bool CPCG::IsEmpty()
{
	return !m_lpBits;
}


bool CPCG::SaveToFile(CFile *pf)
{
	BYTE *p;
	int nByteWidth = BYTE_PER_LINE(m_nWidth, 8);
	BITMAPFILEHEADER bm;
	bm.bfType = 'M' * 256 + 'B';
	bm.bfSize = nByteWidth * m_nHeight;
	bm.bfReserved1 = 0;
	bm.bfReserved2 = 0;
	bm.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPFILEHEADER) + 256*sizeof(RGBQUAD);
	pf->Write(&bm, sizeof(BITMAPFILEHEADER));

	BITMAPINFOHEADER bmi;
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = m_nWidth;
	bmi.biHeight = m_nHeight;
	bmi.biPlanes = 1;
	bmi.biBitCount = 8;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;
	bmi.biXPelsPerMeter = 0;
	bmi.biYPelsPerMeter = 0;
	bmi.biClrUsed = 0;
	bmi.biClrImportant = 0;
	pf->Write(&bmi, sizeof(BITMAPINFOHEADER));

	m_lpPalette = new BYTE[sizeof(RGBQUAD)*256];

	for (int x = 0; x < 256; x++)
	{
		p = m_lpPalette + x*4;
		p[0] = p[1] = p[2] =x;
		p[3] = 0;
	}
	pf->Write (m_lpPalette,sizeof(RGBQUAD)*256);

	m_lpBits= new BYTE[nByteWidth*m_nHeight];

	for(int i=0 ;i<m_nHeight;i++)
	{
		for(int j=0;j<nByteWidth;j++)
		{
			p=m_lpBits+nByteWidth*i+j;
			if(j>m_nWidth)
				p[0]=0;
			else
				//	p[0]=BYTE(m_fData[i][j]);
				p[0]=BYTE(m_fData[m_nHeight-i-1][j]); //������µߵ������⣡
		}
	}
	pf->Write (m_lpBits,nByteWidth*m_nHeight);

	return TRUE;
}


void CPCG::LoadDLL(CString &NumberStr, int Count)
{
	int m;
	ICT_HEADER22 *uh;
	FILE *hFile;
	int nRet;
	WORD Size;
	hFile = fopen(NumberStr, "rb");
	if (hFile > 0)
	{
		typedef int (*UNChainHeader)(void *srcHead, int *nRet, WORD *Size);
		typedef void (*ReleaseHeaderBuf)();
		typedef void (*Version)(BYTE *mainVer, BYTE *subVer);
		UNChainHeader unchainHeader;
		ReleaseHeaderBuf releaseHeadBuf;
		Version version;
		//�Ƚ��н���
		HINSTANCE hDLL = NULL;
		hDLL = LoadLibrary(_T("CtDispose.dll"));
//		hDLL = ::LoadLibraryA("C:\WINDOWS\system32\CtDispose.dll");
//		hDLL = _T(LoadLibraryEx("CtDispose.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH));
		if (hDLL == NULL)
		{
			MessageBox(0,"�޷�װ����Ӧ�Ķ�̬�⣡", "��ʾ", MB_OK + MB_ICONWARNING + MB_TOPMOST);
			return;
		}
		unchainHeader = (UNChainHeader)GetProcAddress(hDLL,"unchainHeader");
		releaseHeadBuf = (ReleaseHeaderBuf)GetProcAddress(hDLL, "releaseHeadBuf");
		version = (Version)GetProcAddress(hDLL, "eVersion");

		//Ĭ��ʹ��2.2�汾
		fread(&CTHeader, sizeof(ICT_HEADER22), 1, hFile); //�����ļ�ͷ
		uh = (ICT_HEADER22 *)unchainHeader(&CTHeader, &nRet, &Size);//�����ļ�ͷ
		if (nRet != 0) 
		{
			MessageBox(0, "�������", "��ʾ", MB_OK + MB_ICONWARNING + MB_TOPMOST);
			releaseHeadBuf();
			fclose(hFile);
			return;
		}

		//�����ǰͷ�ļ���2.1�汾������ת��Ϊ2.2��
		if (uh->SubVersion == 1) 
		{
			Hd21ToHd22(uh,&CTHeader);
		}
		else
		{
			memcpy(&CTHeader, uh, sizeof(ICT_HEADER22));
		}

		//���������ֽ���
		DWORD nBytes = sizeof(float)*(CTHeader.DataFormat.dataColAtRow                         //ÿ�����ݸ���(��������������)
			+ CTHeader.DataFormat.appendColAtRow)*CTHeader.DataFormat.TotalLines;  //(ÿ����ǰ���������ݸ���)*(����������)
		float *IctData =(float*) malloc(nBytes);
		fseek(hFile, Size, SEEK_SET); //�ƶ��ļ�ָ�뵽���ݿ�ʼ��
		fread(IctData, nBytes, 1, hFile);
		releaseHeadBuf();
		fclose(hFile);
		float radius=CTHeader.ScanParameter.ViewDiameter;
		float layer=CTHeader.ScanParameter.SpaceBetweenLayer;//��־����仰������˵�������Է�����ǰ���뷨�ǶԵģ��ɴ˿��Ի�ȡ���ࡣ2011/11/17
		switch (CTHeader.ScanParameter.ScanMode)
		{
		case 1:   //2��CT
			m_nHeight = CTHeader.DataFormat.dataColAtRow;
			m_nWidth = m_nHeight;
			ImageType = 1;
			break;
		case 2:  //3��CT
			m_nHeight = CTHeader.DataFormat.dataColAtRow;
			m_nWidth = m_nHeight;
			ImageType = 2;
			break;
		case 3:  //DR
			m_nHeight = CTHeader.DataFormat.TotalLines;
			m_nWidth = CTHeader.DataFormat.dataColAtRow;
			ImageType = 3;
		default: ;
		}

		//����ռ䲢��ȡ����
		m_fData.resize(m_nHeight);
		for(m=0; m<m_nHeight; m++)
		{
			m_fData[m].resize(m_nWidth);
		}

		//����ʹ�ö�ά�������忽������Ϊ��ά����ռ���䲻����
		for (int i = 0; i < m_nHeight ; i++) 
		{
			for (int j = 0; j < m_nWidth; j++) 
			{
				m_fData[i][j] = IctData[i*m_nWidth+j];
			}
		}

		//�ͷſռ�
		m_OldData = m_fData;
		free(IctData);
		::FreeLibrary(hDLL);

		CString number;
		number.Format("%d",Count+1); //ȷ��count+1Ϊ ʮ����
		NumberStr = NumberStr.Left(NumberStr.GetLength()-8);
		//		NumberStr.Insert(NumberStr.GetLength(),"-");
		NumberStr += number + ".bmp";
		//		NumberStr.Replace(NumberStr.Right(4),number+".bmp");
		//		NumberStr=NumberStr+".BMP";
		SaveToFile(NumberStr);
		//pDoc->OnOpenDocument(NumberStr);
	}
	else
	{
		MessageBox(0, "�޷����ļ���", "��ʾ", MB_OK + MB_ICONWARNING + MB_TOPMOST);
	}
}


void CPCG::Hd21ToHd22(ICT_HEADER22 * hd21, ICT_HEADER22 * hd22)
{
	DATAFORMAT21 df21;
	DATAFORMAT22 df22;
	memcpy(&df21, &(hd21->DataFormat), sizeof(DATAFORMAT21));
	df22.DataType = df21.DataType;
	df22.TotalLines = df21.TotalLines;
	df22.dataColAtRow = df21.dataColAtRow;
	df22.appendColAtRow = df21.appendColAtRow;
	df22.graduationBase = df21.graduationBase;
	memcpy(&(hd21->DataFormat), &df22, sizeof(DATAFORMAT22));
	memcpy(hd22, hd21, sizeof(ICT_HEADER22));
	(*hd22).SubVersion = 2;
	(*hd22).ScanParameter.NumberofTable = 1;
}


DWORD CPCG::GetNumColors() const     //��ȡ��ɫ��Ŀ
{
	int ColorNum[256];
	int ColorCount = 0;

	for(int m = 0; m < 256; m++)
		ColorNum[m] = 0;

	for (int j = 0; j < m_nHeight; j++)
	{
		for (int k = 0; k < m_nWidth; k++)
		{
			ColorNum[int(m_fData[j][k])] = 1;
		}
	}

	for (int n = 0; n < 256; n++)
	{
		if (ColorNum[n] == 1)
		{
			ColorCount++;
		}
	}

	return ColorCount;
}


long CPCG::GetSize()
{
	return m_nHeight*m_nWidth*sizeof(float) + sizeof(ICT_HEADER22);
}


long CPCG::GetImageSize(void)
{
	return 1078 + m_nWidth*m_nHeight;      //ֻ����8bpp�Ҷ�ͼ��
}


bool CPCG::OnShow(CDC* pDC)
{
	int w = 0;
	int h = 0;
	int Count = 1;
    
	w = int (GetWidth());    //��ȡ��ʾ��ͼ����
	h = int (GetHeight());    //��ȡͼ�����ʵ�߶�

	if (h < 0)      //��ʾλͼ
	{
		for (int i = 0; i < Count; i++)      //height>0��ʾͼƬ�ߵ�
		{
			for (int j = 0; j < 0-h; j++)
			{
				for (int k = 0; k < w; k++)
				{
					int b = m_fData[j][k];
					int g = b;
					int r = b;
					pDC->SetPixel(k + 300*i, h-j, RGB(r, g, b));
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < Count; i++)
		{
			for (int j = 0; j < h; j++)
			{
				for (int k = 0; k < w; k++)
				{
					int b = m_fData[j][k];
					int g = b;
					int r = b;
					pDC->SetPixel(k + 300*i, j, RGB(r, g, b));
				}
			}
		}
	}
	
	return true;
}


bool CPCG::SaveToFile(LPCTSTR filename)
{
	CFile f;
	if(!f.Open(filename, CFile::modeCreate | CFile::modeWrite))
		return false;

	bool r = SaveToFile(&f);

	f.Close();

	return r;
}


float CPCG::GetLayerSpace(LPCTSTR lpszPathName)
{
	float layer = CTHeader.ScanParameter.SpaceBetweenLayer;

	return layer;
}


float CPCG::GetViewDiameter(LPCTSTR lpszPathName)
{
	float radius = CTHeader.ScanParameter.ViewDiameter;

	return radius;
}
