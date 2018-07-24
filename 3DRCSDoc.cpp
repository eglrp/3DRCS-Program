// 3DRCSDoc.cpp : implementation of the C3DRCSDoc class
//

#include "stdafx.h"
#include "3DRCS.h"
#include "MainFrm.h"
#include "3DRCSDoc.h"
#include "ControlWnd.h"
#include "ProjectView.h"
#include "math.h"
#include "ShowParaDlg.h"
#include "ModeInfo.h"
#include <algorithm>

#include "PSOintro.h"

#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDC *   m_pMainVDC=NULL;
HGLRC   m_pMainVRC=NULL;

CDC *   m_pProVDC=NULL;
HGLRC   m_pProVRC=NULL;

extern float	m_Prox,m_Proy,m_Proz; //��������

#define BYTE_PER_LINE(w,c)  ((((w)*(c)+31)/32)*4)

#include "time.h"
#include <sys/timeb.h>
//�����������꣬�������㲢��ʾĳ�μ��㲽����������ʱ��
#define TimeCountStart CTime startTime = CTime::GetCurrentTime();\
	struct _timeb timebuffer;\
	int mills1,mills2;\
    _ftime(&timebuffer);\
	mills1=timebuffer.millitm;\
	CString str1;\

#define TimeCountEnd CTime endTime = CTime::GetCurrentTime();\
	CTimeSpan elapsedTime = endTime - startTime;\
    _ftime(&timebuffer);\
	mills2=timebuffer.millitm;\
	long int timeused;\
	timeused=elapsedTime.GetTotalSeconds();\
	if (mills1<=mills2)\
		timeused=timeused*1000+(mills2-mills1);\
	else\
		timeused=(timeused-1)*1000+(mills1-mills2);\
	str1.Format ("�ò�������ʱ�䣺%d ����",timeused);\
    ::AfxMessageBox(str1);


/////////////////////////////////////////////////////////////////////////////
// C3DRCSDoc

IMPLEMENT_DYNCREATE(C3DRCSDoc, CDocument)

BEGIN_MESSAGE_MAP(C3DRCSDoc, CDocument)
	//{{AFX_MSG_MAP(C3DRCSDoc)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3DRCSDoc construction/destruction

double 	_Volume,_Area;

C3DRCSDoc::C3DRCSDoc()
{
	// TODO: add one-time construction code here
	m_lpBits=NULL;
	m_FileName ="";
	m_PointList=NULL;
	m_LineList=NULL;
	m_FacetList=NULL;
	m_bCalculateFacetValue=false;
	m_AverageArea=0.0;
	m_AverageLength=0.0;
	m_AverageHeigth=0.0;

	m_DownHole=false;
	m_UpHole=false;
	m_XmaxFacet=0;
	m_YmaxFacet=0;
	m_hole = false;

	m_ModeSurfaceArea=0.0;  //ģ�ͱ����
	m_ModeVolume=0.0;       //ģ�����

	m_Pixels=NULL;
	m_nFileNum=0;
	m_nWidth = m_nHeight = 0;
	m_TargetValue=200;

	m_ReduceAngleThreshold=2.5f;
	m_DistanceThreshold=0.1f;
	m_ReduceNum=0;
	m_ReduceRate=0;

	m_SmoothRate = 0.5f;
	m_SmoothNum = 5;

	m_ReverseFacet=false;
	xTranslate=0.0f;
	yTranslate=0.0f;
	zTranslate=0.0f;
	xZoomSave=1.0f;
	yZoomSave=1.0f;
	zZoomSave=1.0f;
}

C3DRCSDoc::~C3DRCSDoc()
{
	this->Clear();
	if(m_pProVDC)
		m_pProVDC=NULL;
	if(m_pMainVDC)
		m_pMainVDC=NULL;
	if(m_pProVRC)
		m_pProVRC=NULL;
	if(m_pMainVRC)
		m_pMainVRC=NULL;
}

BOOL C3DRCSDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// C3DRCSDoc serialization

void C3DRCSDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// C3DRCSDoc diagnostics

#ifdef _DEBUG
void C3DRCSDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void C3DRCSDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// C3DRCSDoc commands

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ReadFile()
 *
 * ����:
 *   LPVOID   lp     - ָ���̵߳��õ���
 *
 * ����ֵ:
 *   UINT            - ������;
 *
 * ˵��:
 *       �ú�������ִ�ж�ȡ�ļ��Ľ��̡�
 * 
 ******************************************************************************************************************************/
UINT C3DRCSDoc::ReadFile(LPVOID lp)
{
	C3DRCSDoc *pThis = (C3DRCSDoc*)lp;   //��ȡ�߳�ָ��
	
	::gbIsGetData = true;         //��ʶΪ���ڶ�ȡ�ļ�
	::gbDataIsEmpty = true;       //��ʶΪ����Ϊ��

	//��ʾ�Ƿ�ɹ���ȡ�ļ�
	if(!(pThis->GetFileData(pThis->m_FileName)))
	{
		pThis->m_pFr->m_pCtrlWnd->Clear() ;
		AfxMessageBox("���ļ�ʧ�ܣ�");
		return 0L;
	}
	pThis->m_pFr->m_pView->SetTime();
	
	return 0L;
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   GetFileData()
 *
 * ����:
 *   CString    filename   - �����ļ���·��
 *
 * ����ֵ:
 *   bool                  - ����true ��ʾ�ɹ���ȡ�ļ������򷵻�false
 *
 * ˵��:
 *       �ú��������жϴ򿪵��ļ���ASCII�ļ���������ļ�������ASCII�ļ���
 * ͬʱ���������Ĺ�ϵ��
 * 
 ******************************************************************************************************************************/
bool C3DRCSDoc::GetFileData(CString filename)
{
	this->m_pFr = (CMainFrame*)AfxGetApp()->m_pMainWnd;   //*****��ȡ�����ָ��
	this->m_pFr->m_pView->KillTime();
	this->m_pFr->InvalidateAllWnd();    //ˢ�´���
	this->m_pFr->m_wndStatusBar.SetWindowText("���ڼ���ģ�ͣ����Ժ򡭡�");
	
	::gbIsGetData=true;
	FILE * FileData = fopen(filename,"r");//��������ļ�
	
	/***********************����ļ�ͷ***********************/
	CString TemData;       //��ʱ����ʵʱ��ȡ���ַ���
	fscanf(FileData,"%s", TemData); 

	if(TemData!="solid")  //�����Զ�ȡASCII�뷽ʽ��ȡ�ļ�������ļ���һ���ַ�������"solid"���������Զ����Ʒ�ʽ���ļ���
	{
		fclose(FileData);

		if(ReadBitFile(filename))  //�Զ����Ʒ�ʽ���ļ�
		{
			::gbIsGetData = ::gbDataIsEmpty = false;
			this->m_pFr->m_Progress.SetPos(100); this->m_pFr->m_Progress.SetPos(0);

			ClearPointsHash(); //�����Hash��
			ExtractLines();    //�����Ѷ�ȡ����Ƭ�Ϳռ�㣬��ȡ���洢����ߡ�
			AfxMessageBox( "�ɹ���������ߣ�" );
			
			//ʹ����̨�������á�
			::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_ENABLEBTN,0,0);
			::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);	

			//״̬����ʾ������ɡ�
			this->m_pFr->m_wndStatusBar.SetWindowText("ģ�ͼ�����ɣ�");
			//��������ʾ���سɹ���
			AfxGetMainWnd()->SetWindowText("��ά�ؽ�ϵͳ��3DRCS��->�Ѽ�������");

			return true;     //��ȡ�ɹ������ء�
		}	
	
		else
		{ 
			if(ReadAsciiPlyFile(filename)) //�Զ�ȡASCII�뷽ʽ��ȡPLY�ļ�
			{
			::gbIsGetData = ::gbDataIsEmpty = false;
			this->m_pFr->m_Progress.SetPos(100); this->m_pFr->m_Progress.SetPos(0);

			ClearPointsHash(); //�����Hash��
			ExtractLines();    //�����Ѷ�ȡ����Ƭ�Ϳռ�㣬��ȡ���洢����ߡ�
			AfxMessageBox( "�ɹ���������ߣ�" );

			//ʹ����̨�������á�
			::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_ENABLEBTN,0,0);
			::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);	

			//״̬����ʾ������ɡ�
			this->m_pFr->m_wndStatusBar.SetWindowText("ģ�ͼ�����ɣ�");
			//��������ʾ���سɹ���
			AfxGetMainWnd()->SetWindowText("��ά�ؽ�ϵͳ��3DRCS��->�Ѽ�������");

			return true;     //��ȡ�ɹ������ء�
			}
		}
		//�ļ�ͷ����������������
		AfxMessageBox( "This file is not a STL or ply file!" );
		return false;	

	}
	





//////////////////////////////////////*******���´���Ϊ��ȡASCII��STL�ļ�*******//////////////////////////////////////	

	Clear();                             //���� m_Vertex��m_Facet��m_Edge

	Point3D m_Point;
	Vertex vertex;
	Facet m_facet;
	
	
	//��ȡ�ļ��Ĵ�С����λ���ֽ�
	WIN32_FIND_DATA fileInfo;
	HANDLE hFind = FindFirstFile(filename ,&fileInfo); 
	m_FileSize = fileInfo.nFileSizeLow; 
	FindClose(hFind);

	//Ϊ��Hash������ռ�
	m_PointList = new Point3D*[HashMaxLish*2];
	
	//���ɢ�б�����
	for(int i=0;i<HashMaxLish*2;i++)  {m_PointList[i]=NULL;}

	//Ԥ���㡢�ߡ��漯���������������ϸ������ϵ��趨������С��
	m_FileSize/=150;              //���Թ���һ��������Ƭ�ĸ���
	m_Vertex.reserve(m_FileSize/2);
	m_Facet.reserve(m_FileSize);
	m_Edge.reserve(m_FileSize*2);

	/***********************��ȡ����***********************/
	int m_fileMax=0;          //��ͷ��ȡ�ĵڶ��е��ַ��������������ȡ����50��ʱ����δ�ҵ���һ��normal�����ʾ��ȡʧ��
	int m_key=-1;             //Hash����ֵ�����ռ���Ӧ�Ĵ洢λ�ã�����ʼ��Ϊ-1��
	bool m_samePoint=false;   //������ͬ���ʶ��
	
	AfxMessageBox( "����һ��ASCII���STL�ļ���" );
	
	while (feof(FileData)==NULL)  //ѭ����ȡ�ļ��ڵ��ַ���
	{
		m_samePoint=false;
		if(m_FacetNumber%1024==0)
			this->m_pFr->m_Progress.SetPos(m_FacetNumber*120.0f/m_FileSize);

		
		fscanf(FileData,"%s", TemData); //ȡ���ļ�����

		if(TemData!="facet")	//���ʸ�����ݵ�ͷ�ṹ��β�ṹ
		{
			//�ж��Ƿ��Ƕ�ȡ��endsolid������ǣ�����ʾ�ļ��Ѷ�ȡ��ɡ��ر��ļ��������ء�
			if(TemData=="endsolid")
			{
				fclose(FileData);   //�ر��ļ�
				::gbIsGetData =	::gbDataIsEmpty = false;

				ClearPointsHash(); //�����Hash��

				this->m_pFr->m_Progress.SetPos(100); this->m_pFr->m_Progress.SetPos(0);

				ExtractLines();  //�����Ѷ�ȡ����Ƭ�Ϳռ�㣬��ȡ���洢����ߡ�
				AfxMessageBox( "�ɹ���������ߣ�" );

				//״̬����ʾ������ɡ�
				this->m_pFr->m_wndStatusBar.SetWindowText("ģ�ͼ�����ɣ�");
				//��������ʾ���سɹ���
				AfxGetMainWnd()->SetWindowText("��ά�ؽ�ϵͳ��3DRCS��->�Ѽ�������");

				::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_ENABLEBTN,0,0);
				::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);	
				return true;
			}

			if(m_fileMax>50)
			{
				//���Ѱ����50���ַ�������δ�ҵ�"Facet"�����˳���
				AfxMessageBox( "Mistake DataStruct:Facet!");
				fclose(FileData);
				return FALSE;
			}
			m_fileMax++;
			continue;		
		}
		
		fscanf(FileData,"%s", TemData);
		if(TemData!="normal")
		{
			AfxMessageBox( "Mistake DataStruct:Normal!" );
			fclose(FileData);
			return FALSE;
		}
	
		//��ȡ��ȡ��Ƭ���������ݣ��������ݿ�����������ڶ�ȡ�������������������¼�����Ƭ�ķ�������
		fscanf(FileData,"%s ", TemData);
		fscanf(FileData,"%s ", TemData);
		fscanf(FileData,"%s ", TemData); 
		fscanf(FileData,"%s ", TemData);

		if(TemData!="outer")
		{
			AfxMessageBox( "Mistake DataStruct:Outer!" );
			fclose(FileData);
			return FALSE;
		}
		
		fscanf(FileData,"%s ", TemData);
		if(TemData!="loop")
		{
			AfxMessageBox( "Mistake DataStruct:Loop!" );
			fclose(FileData);
			return FALSE;
		}

		//������Ƭ�Ķ�������
		int m_index=-1;
		float data;

		for(int k=0;k<3;k++) //��ȡ�����ռ������
		{
			fscanf(FileData,"%s ", TemData);
			fscanf(FileData,"%s ", TemData);
			data=(float)atof(TemData);
			m_Point.x=data;    
			
			fscanf(FileData,"%s ", TemData);
			data=(float)atof(TemData);
			m_Point.y=data;
			
			fscanf(FileData,"%s ", TemData);
			data=(float)atof(TemData);
			m_Point.z=data;


			//*****�ж��Ƿ��Ѵ�����ͬ�ĵ㣬�����������ɢ�б�****//

			m_key=CalculatePKey(m_Point);  //����ÿռ���Hash����ֵ
			m_samePoint=false;       //m_samePoint��ʼ��

			//�ж�KeyֵΪm_key�Ƿ����е㣨��ɢ�б��в���һ��Ԫ�أ�
			if(m_PointList[m_key]!=NULL) //���m_key��Ӧ��λ�����д洢Ԫ��
			{
				Point3D *p=m_PointList[m_key];
				while(p!=NULL)
				{
					//����ʽɢ�б���ң�����ͬKeyֵ�ĵ��У��Ƿ��������ͬ��
					if((p->x==m_Point.x)&&(p->y==m_Point.y)&&(p->z==m_Point.z))
					{
						//����ҵ�����ͬ�㣬���ȡ�ĵ�ı�ţ�����m_samePoint��Ϊtrue
						m_index=p->index;    
						m_samePoint=true;    
						break;
					}
					else
						p=p->next;
				}
			}

			//�����������ͬ�㣬�⽫�õ�洢�ڵ㼯�У�ͬʱ���뵽ɢ�б���
			//��Ϊ������Ƭ��¼�������������
			if(!m_samePoint)
			{
				//���ռ�����Ϣ�洢�ڵ�ɢ�б�
				Point3D *p=new Point3D;
				vertex.x=p->x=m_Point.x;
				vertex.y=p->y=m_Point.y;
				vertex.z=p->z=m_Point.z;
				p->index=m_VertexNumber;
				p->next=m_PointList[m_key];
				m_PointList[m_key]=p;

				if(k==0)		
					m_facet.vertex[0]=m_VertexNumber;
				else
				{
					if(k==1)	m_facet.vertex[1]=m_VertexNumber;
					else		m_facet.vertex[2]=m_VertexNumber;
				}

				//��ȡXYZ�������Сֵ
				if (p->x > x_glMax)	{   x_glMax = p->x;	m_XmaxFacet=m_FacetNumber;}
				if (p->x < x_glMin)		x_glMin = p->x;
				if (p->y > y_glMax)	{   y_glMax = p->y; m_YmaxFacet=m_FacetNumber;}
				if (p->y < y_glMin)		y_glMin = p->y;
				if (p->z > z_glMax) 	z_glMax = p->z;
				if (p->z < z_glMin) 	z_glMin = p->z;

				m_Vertex.push_back(vertex);   //����洢�ڵ㼯��ͷ
				m_Vertex[m_VertexNumber].facet.push_back(m_FacetNumber);   //����������Ƭ�ı�� �����ڿռ��Ԫ�ص��ڽ�������Ƭ����
				m_VertexNumber++;             //�ռ����������ţ���1
			}
			else   //����Ѵ��иÿռ�����Ϣ���򽫸õ���ż�¼��������Ƭ�ĵ㼯���С�
			{
				if(k==0)		m_facet.vertex[0]=m_index;
				else
				{
					if(k==1)		m_facet.vertex[1]=m_index;
					else		m_facet.vertex[2]=m_index;
				}
				m_Vertex[m_index].facet.push_back(m_FacetNumber);
			}
		}
		
		fscanf(FileData,"%s ", TemData);
		if(TemData!="endloop")
		{
			AfxMessageBox( "Mistake DataStruct:Endloop!" );
			fclose(FileData);
			return FALSE;
		}
		
		//����������Ƭ���ݶ�ȡ����
		fscanf(FileData,"%s ", TemData);
		if(TemData!="endfacet")
		{
			AfxMessageBox( "Mistake DataStruct:Endfacet!");
			fclose(FileData);
			return FALSE;
		}
		
		CalculateFacetNormal(m_facet);//������Ƭ�ķ���

		m_Facet.push_back(m_facet);   //��������ƬԪ�ش洢���漯��ͷ
		m_FacetNumber++;        //��ȡ��һ�����ݣ��漯��������ţ�������һ

    } //while (feof(FileData)==NULL)
	
	fclose(FileData);
	AfxMessageBox( "Error while reading this file!" );
 	return false;

}
/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ReadAsciiPlyFile
 *
 * ����:
 * ����ֵ:
 *   CString    filename   - �����ļ���·��
 *
 *   bool                  - ����true ��ʾ�ɹ���ȡ�ļ������򷵻�false
 *
 * ˵��:
 *       �ú������ڶ�ȡASCII���PLY�ļ��������������Ĺ�ϵ��
 * 
 ******************************************************************************************************************************/
 BOOL C3DRCSDoc::ReadAsciiPlyFile(CString filename)
{
	Clear();
	
	Point3D m_Point;
	Vertex vertex;
	Facet m_facet;

	//��ȡ�ļ��Ĵ�С����λ���ֽ�
	WIN32_FIND_DATA fileInfo;
	HANDLE hFind = FindFirstFile(filename ,&fileInfo); 
	m_FileSize = fileInfo.nFileSizeLow; 
	FindClose(hFind);

	//Ϊ��Hash������ռ�
	m_PointList = new Point3D*[HashMaxLish*2];
	
	//���ɢ�б�����
	for(int i=0;i<HashMaxLish*2;i++)  {m_PointList[i]=NULL;}
	

	FILE * FileData = fopen(filename,"r");//��������ļ�
	
	/***********************���ļ�ͷ***********************/
	float m_PointNumber = 0;//�����ļ����������Ŀ
	float m_FaceNumber = 0;//�����ļ����������Ŀ
	CString TemData; //��ʱ����ʵʱ��ȡ���ַ���
	char string[1000];


	fscanf(FileData,"%s", TemData); 
	if(TemData!="ply") return false;
	AfxMessageBox( "����һ��ASCII���PLY�ļ���" );

	fgets(string,200,FileData);
	fgets(string,200,FileData);

	fscanf(FileData,"%s", TemData); 
	fscanf(FileData,"%s", TemData);
	fscanf(FileData,"%s", TemData);
//	AfxMessageBox(TemData);
	float data;
	data=(float)atof(TemData);
	m_PointNumber = data;
	
	
	fgets(string,200,FileData);
	fgets(string,200,FileData);
	fgets(string,200,FileData);
	fgets(string,200,FileData);
	
	fscanf(FileData,"%s", TemData); 
	fscanf(FileData,"%s", TemData);
	fscanf(FileData,"%s", TemData);
//	AfxMessageBox(TemData);
	data=(float)atof(TemData);
	m_FaceNumber = data;

	
	fgets(string,200,FileData);
	fgets(string,200,FileData);
	fgets(string,200,FileData);
//	AfxMessageBox(string);

	//Ԥ���㡢�ߡ��漯���������������ϸ������ϵ��趨������С��
	m_Vertex.reserve(m_FaceNumber/2);
	m_Facet.reserve(m_FaceNumber);
	m_Edge.reserve(m_FaceNumber*2);
			
	/***********************��ȡ����***********************/
//	int m_fileMax=0;          //��ͷ��ȡ�ĵڶ��е��ַ��������������ȡ����50��ʱ����δ�ҵ���һ��normal�����ʾ��ȡʧ��
	int m_key=-1;             //Hash����ֵ�����ռ���Ӧ�Ĵ洢λ�ã�����ʼ��Ϊ-1��
	bool m_samePoint=false;   //������ͬ���ʶ��
	int m_index=-1;
	int sameNumber=0;
	int i;
	for(i=0;i<m_PointNumber;i++)//ȡ�õ������	
	{
		m_samePoint=false;

		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*100/m_PointNumber);

		float v=0;
		fscanf(FileData,"%f", &v); 
		m_Point.x=v;  
	//	_gcvt( v, 9, string );
	//	AfxMessageBox(string);
	
		fscanf(FileData,"%f", &v); 
		m_Point.y=v;  
			
		fscanf(FileData,"%f", &v); 
		m_Point.z=v;  

		fgets(string,200,FileData);

		//*****�ж��Ƿ��Ѵ�����ͬ�ĵ㣬�����������ɢ�б�****//
		m_key=CalculatePKey(m_Point);  //����ÿռ���Hash����ֵ
		m_samePoint=false;       //m_samePoint��ʼ��
	
	
		//�ж�KeyֵΪm_key�Ƿ����е㣨��ɢ�б��в���һ��Ԫ�أ�
		if(m_PointList[m_key]!=NULL) //���m_key��Ӧ��λ�����д洢Ԫ��
		{	
			Point3D *p=m_PointList[m_key];
			while(p!=NULL)
			{
				//����ʽɢ�б���ң�����ͬKeyֵ�ĵ��У��Ƿ��������ͬ��
				if((p->x==m_Point.x)&&(p->y==m_Point.y)&&(p->z==m_Point.z))
				{
					//����ҵ�����ͬ�㣬���ȡ�ĵ�ı�ţ�����m_samePoint��Ϊtrue
					m_index=p->index;    
					m_samePoint=true;    
					break;
					sameNumber++;
				}
				else
					p=p->next;
			}
		}

	
		//�����������ͬ�㣬�⽫�õ�洢�ڵ㼯�У�ͬʱ���뵽ɢ�б���
		//��Ϊ������Ƭ��¼�������������
		if(1)
		{	
		//	AfxMessageBox("��������ͬ��");
			//���ռ�����Ϣ�洢�ڵ�ɢ�б�
			Point3D *p=new Point3D;
			vertex.x=p->x=m_Point.x;
			vertex.y=p->y=m_Point.y;
			vertex.z=p->z=m_Point.z;
			p->index=m_VertexNumber;
			p->next=m_PointList[m_key];
			m_PointList[m_key]=p;

			//��ȡXYZ�������Сֵ
			if (p->x > x_glMax)	
			{   x_glMax = p->x;	//m_XmaxFacet=m_FacetNumber;
			}
			if (p->x < x_glMin)		x_glMin = p->x;
			if (p->y > y_glMax)	
			{   y_glMax = p->y; //m_YmaxFacet=m_FacetNumber;
			}
			if (p->y < y_glMin)		y_glMin = p->y;
			if (p->z > z_glMax) 	z_glMax = p->z;
			if (p->z < z_glMin) 	z_glMin = p->z;

			m_Vertex.push_back(vertex);   //����洢�ڵ㼯��ͷ
			m_VertexNumber++;             //�ռ����������ţ���1
		}	
				
	}	

/*	CString str1,str2,str3;
	str1.Format("m_VertexNumber:%d",m_VertexNumber);
	str2.Format("m_m_PointNumber:%d",i);
	str3.Format("sameNumber:%d",sameNumber);
	AfxMessageBox(str1);	
	AfxMessageBox(str2);
	AfxMessageBox(str3);*/
	AfxMessageBox("����㣡");

	for(i=0;i<m_FaceNumber;i++)//ȡ���������	
	{
		if(i%1024==0)
		this->m_pFr->m_Progress.SetPos((i+1)*100/m_FaceNumber);
		float v=0;
		fscanf(FileData,"%f", &v); 
		fscanf(FileData,"%f", &v);
		m_facet.vertex[0]=v;    
			
		fscanf(FileData,"%f", &v);
		m_facet.vertex[1]=v;    
			
		fscanf(FileData,"%f", &v);
		m_facet.vertex[2]=v;    
		
		fgets(string,200,FileData);

		m_Vertex[m_index].facet.push_back(m_FacetNumber);
		
		CalculateFacetNormal(m_facet);//������Ƭ�ķ���

		m_Facet.push_back(m_facet);   //��������ƬԪ�ش洢���漯��ͷ
		m_FacetNumber++;        //��ȡ��һ�����ݣ��漯��������ţ�������һ
		
	}
	_gcvt(m_FacetNumber, 9, string );
	AfxMessageBox(string);
	AfxMessageBox("�����棡");

fclose(FileData);
return true;

}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ReadBitFile()
 *
 * ����:
 *   CString    filename   - �����ļ���·��
 *
 * ����ֵ:
 *   bool                  - ����true ��ʾ�ɹ���ȡ�ļ������򷵻�false
 *
 * ˵��:
 *       �ú������ڶ�ȡ������STL�ļ��������������Ĺ�ϵ��
 * 
 ******************************************************************************************************************************/
BOOL C3DRCSDoc::ReadBitFile(CString filename)
{	
	Clear();                             //���� m_Vertex��m_Facet��m_Edge

	Point3D m_Point;
	Vertex vertex;
	Facet m_facet;
	
	
	//��ȡ�ļ��Ĵ�С����λ���ֽ�
	WIN32_FIND_DATA fileInfo;
	HANDLE hFind = FindFirstFile(filename ,&fileInfo); 
	m_FileSize = fileInfo.nFileSizeLow; 
	FindClose(hFind);

	//Ϊ��Hash������ռ�
	m_PointList = new Point3D*[HashMaxLish*2];
	
	//���ɢ�б�����
	for(int k=0;k<HashMaxLish*2;k++)  {m_PointList[k]=NULL;}
	
	bool m_samePoint = false;
	int m_FacetNum = 0; 
	float temptValue = 0; 
	int m_index=-1;
	
	CFile readFile( filename, CFile::modeRead | CFile::typeBinary );
	
	char strValueOut[80] = "\0"; 
	readFile.Read( strValueOut, sizeof(strValueOut)); 
	readFile.Read( &m_FacetNum, 4); 

	if(abs(int((m_FileSize-80)/50-m_FacetNum))>10)   //�ж��ļ��Ĵ�С���ļ��м�¼��������Ƭ�Ƿ����Ӧ
	{
		return false;
	}

	AfxMessageBox("����һ��������STL�ļ���");

	m_Vertex.reserve(m_FacetNum/2);//�����ڴ�ռ�
	m_Facet.reserve(m_FacetNum);
	m_Edge.reserve(m_FacetNum*2);

	for(int i=0;i<m_FacetNum;i++)
	{
		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*100/m_FacetNum);

		readFile.Read( &temptValue, sizeof(float)); 
		readFile.Read( &temptValue, sizeof(float));
		readFile.Read( &temptValue, sizeof(float));
		
		for(int k=0;k<3;k++)
		{
			readFile.Read( &temptValue, sizeof(float));
			m_Point.x=temptValue;
			
			readFile.Read( &temptValue, sizeof(float));
			m_Point.y=temptValue;
			
			readFile.Read( &temptValue, sizeof(float));
			m_Point.z=temptValue;
			
			int m_key=-1;
			m_key=CalculatePKey(m_Point);
			m_samePoint=false;

			if(m_PointList[m_key]!=NULL)
			{
				Point3D *p=m_PointList[m_key];
				while(p!=NULL)
				{
					if((p->x==m_Point.x)&&(p->y==m_Point.y)&&(p->z==m_Point.z))
					{
						m_index=p->index;
						m_samePoint=true;
						break;
					}
					else
						p=p->next;
				}
			}
		
			if(!m_samePoint)
			{
				Point3D *p=new Point3D;
				vertex.x=p->x=m_Point.x;
				vertex.y=p->y=m_Point.y;
				vertex.z=p->z=m_Point.z;
				p->index=m_VertexNumber;
				p->next=m_PointList[m_key];
				m_PointList[m_key]=p;

				if(k==0)	    m_facet.vertex[0]=m_VertexNumber;
				else
				{
					if(k==1)	m_facet.vertex[1]=m_VertexNumber;
					else		m_facet.vertex[2]=m_VertexNumber;
				} 
				if (p->x > x_glMax) {	x_glMax = p->x;	m_XmaxFacet=m_FacetNumber;}
				if (p->x < x_glMin)		x_glMin = p->x;
				if (p->y > y_glMax)	{   y_glMax = p->y; m_YmaxFacet=m_FacetNumber;}
				if (p->y < y_glMin)		y_glMin = p->y;
				if (p->z > z_glMax) 	z_glMax = p->z;
				if (p->z < z_glMin) 	z_glMin = p->z;

				m_Vertex.push_back(vertex);
				m_Vertex[m_VertexNumber].facet.push_back(m_FacetNumber);
				m_VertexNumber++;
			}
			else
			{
				if(k==0)		m_facet.vertex[0]=m_index;
				else
				{
					if(k==1)	m_facet.vertex[1]=m_index;
					else		m_facet.vertex[2]=m_index;
				}
				m_Vertex[m_index].facet.push_back(m_FacetNumber);
			}
		}
		CalculateFacetNormal(m_facet);
		m_FacetNumber++;
		m_Facet.push_back(m_facet);
		readFile.Seek(2,CFile::current);	
	}

	readFile.Close(); 
	return true;
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   Clear()               
 *
 * ˵��:
 *       �ú���������յ㼯���߼����漯��
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::Clear()
{
	//����㡢�ߡ��漯
	if(!m_Vertex.empty())	m_Vertex.clear();
	if(!m_Edge.empty())		m_Edge.clear();
	if(!m_Facet.empty())	m_Facet.clear();
	
	if(!m_UpEdge.empty())   m_UpEdge.clear();   //�ϱ�Ե��
	if(!m_DownEdge.empty()) m_DownEdge.clear(); //�ϱ�Ե��

	if(m_Pixels)   delete m_Pixels;

	ClearFacetsHash();
	

	m_ReduceRate=0.0f;
	m_bCalculateFacetValue=false;
	m_VertexNumber=m_EdgeNumber=m_FacetNumber=0;
	this->m_ReverseFacet=false;
	m_ReduceNum=0;



	x_glMax = y_glMax = z_glMax =-1024.0;
	x_glMid = y_glMid = z_glMid =0.0;
	x_glMin = y_glMin = z_glMin =1024.0;
	m_lpBits=NULL;
	m_Pixels=NULL;
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ClearPointsHash()             
 *
 * ˵��:
 *       �ú���������տռ��ɢ�б�
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::ClearPointsHash()
{
	if(m_PointList==NULL)
		return;

	Point3D *p;
	for(int i=0; i<HashMaxLish*2; i++)
	{
		p=m_PointList[i];
		while(p!=NULL)
		{
			m_PointList[i]=p->next;
			delete p;
			p=m_PointList[i];
		}
	}
	delete m_PointList;
	m_PointList=NULL;
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ClearFacetsHash()              
 *
 * ˵��:
 *       �ú����������������Ƭ��ɢ�б�
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::ClearFacetsHash()
{
	if(m_FacetList==NULL)
		return;

	Face *f;
	for(int i=0; i<HashMaxLish*4; i++)
	{
		f=m_FacetList[i];
		while(f!=NULL)
		{
			m_FacetList[i]=f->next;
			delete f;
			f=m_FacetList[i];
		}
	}
	delete m_FacetList;
	m_FacetList=NULL;
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ClearLinesHash()              
 *
 * ˵��:
 *       �ú���������ձߵ�ɢ�б�
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::ClearLinesHash()
{
	if(m_LineList==NULL)
		return;

	Line *l;
	for(int i=0; i<HashMaxLish*6; i++)
	{
		if(i%100000==0)
			this->m_pFr->m_Progress.SetPos((i+1)*5/(HashMaxLish*6)+60);
		l=m_LineList[i];
		while(l!=NULL)
		{
			m_LineList[i]=l->next;
			delete l;
			l=m_LineList[i];
		}
	}
	delete m_LineList;
	m_LineList=NULL;
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ExtractLines()
 *
 * ˵��:
 *       �ú������ڽ��ߴ��ڱ߼����������������ٽ��㡢��Ĺ�ϵ��
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::ExtractLines()
{
	if (::gbIsGetData)
		return;
	
	if (::gbDataIsEmpty)
		return;

	Edge edge;
	m_Edge.clear();
	bool m_sameLine=false; 
 	SetZoomAndMid();

	this->m_pFr->m_Progress.SetPos(2);

	m_LineList=new Line*[HashMaxLish*6];

	for(int k=0; k<HashMaxLish*6 ;k++)	
	{	
		m_LineList[k]=NULL;	
	}
	this->m_pFr->m_Progress.SetPos(5);

	for(int i=0; i<m_FacetNumber;i++)
	{
		if((i%1024)==0)
			this->m_pFr->m_Progress.SetPos((i+1)*50/m_FacetNumber+5);

		int d[3], v[3];
		d[0]=m_Facet[i].vertex[0]+m_Facet[i].vertex[1];
		d[1]=m_Facet[i].vertex[1]+m_Facet[i].vertex[2];
		d[2]=m_Facet[i].vertex[2]+m_Facet[i].vertex[0];
		v[0]=m_Facet[i].vertex[0];
		v[1]=m_Facet[i].vertex[1];
		v[2]=m_Facet[i].vertex[2];

		int dd;
		for(int j=0; j<3; j++)
		{
			m_sameLine=false;
			dd=*(d+j);
			if(m_LineList[dd]!=NULL)
			{ 
				Line *l=m_LineList[dd];
				while(l!=NULL)
				{
					if((l->vertex1==v[(j+1)%3])&&(l->vertex2==v[j]))
					{
						m_sameLine=true;
						m_Edge[l->index].rfacet=i;
						*(m_Facet[i].edge+j)=l->index;
						break;
					}
					else
						l=l->next;
				}
			}
			if(!m_sameLine)
			{
				Line *l= new Line;
				l->vertex1=edge.vertex1=v[j];
				l->vertex2=edge.vertex2=v[(j+1)%3];
				edge.facet=i;
				l->next=m_LineList[dd]; 
				l->index=m_Edge.size();
				m_LineList[dd]=l;
				edge.edgenum = m_Edge.size();
				m_Edge.push_back(edge);
				*(m_Facet[i].edge+j)=m_Edge.size()-1;     //ȷ����Ƭi�ĵ�j������vector�еĵ����
			}
		}
	}
	m_EdgeNumber=m_Edge.size();
	this->m_pFr->m_Progress.SetPos(60);
	ClearLinesHash();
	RelatePointsEdgesFacets();
	this->m_pFr->m_Progress.SetPos(100); Sleep(100);this->m_pFr->m_Progress.SetPos(0);
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   RelatePointsEdgesFacets()               
 *
 * ˵��:
 *       �ú������ڽ��ߴ��ڱ߼�������������㡢��Ĺ�ϵ��
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::RelatePointsEdgesFacets() 
{
		int i,j;
	/*****************************����� �ĳ���*****************************/
	m_AverageLength=0.0;
	for(i=0; i<m_EdgeNumber; i++)
	{
		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*10/m_EdgeNumber+70);
		m_Edge[i].length=CalculateLength(m_Edge[i].vertex1, m_Edge[i].vertex2);
		m_AverageLength+=m_Edge[i].length;

		if(m_Edge[i].rfacet==-1)
		{
			m_Vertex[m_Edge[i].vertex1].bBoundary=true;
			m_Vertex[m_Edge[i].vertex2].bBoundary=true;
		}
	}
	m_AverageLength/=(double)m_EdgeNumber;


	/*****************��ʶ��������ߵ�������ͬʱ����������****************/
	m_AverageArea=0.0;
	
	for( i=0; i<m_FacetNumber;i++)
	{    
		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*5/m_FacetNumber+80);

		for(j=0; j<3; j++)//���ݱ�ȷ����Ƭ��������Ƭ��λ�ù�ϵ������Ƭi��������Ƭ���
		{
			if(i!=m_Edge[m_Facet[i].edge[j]].rfacet)
				m_Facet[i].aFacet[j]=m_Edge[m_Facet[i].edge[j]].rfacet;	
			else
				m_Facet[i].aFacet[j]=m_Edge[m_Facet[i].edge[j]].facet;
		}
		
		m_Facet[i].area=CalculateArea(m_Facet[i].edge[0],m_Facet[i].edge[1],m_Facet[i].edge[2]);
		m_AverageArea+=m_Facet[i].area;
	}
	m_AverageArea/=(double)m_FacetNumber;

	m_AverageHeigth=m_AverageArea/m_AverageLength;


	/*************************���㶥��Ľ��Ʒ�����**************************/
	
	for( i=0; i<m_VertexNumber; i++)
	{
		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*10/m_VertexNumber+85);

		CalculateVertexNormal(i);
		
	}
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CalculatePKey()
 *
 * ����:
 *   Point3D mPoint     -��������                
 *
 * ˵��:
 *       �ú������ڼ������������ɢ�б��ж�Ӧ��Key��
 * 
 ******************************************************************************************************************************/
int C3DRCSDoc::CalculatePKey(Point3D mPoint)
{
	if(mPoint.x<0)	mPoint.x=0.0-mPoint.x;
	if(mPoint.y<0)	mPoint.y=0.0-mPoint.y;
	if(mPoint.z<0)	mPoint.z=0.0-mPoint.z;
	
	return (int((mPoint.x*3+mPoint.y*5+mPoint.z*7)*PC +0.5))%(HashMaxLish*2);
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CalculateLength()
 *
 * ����:
 *   int v1, int v2         -�������������               
 *
 * ˵��:
 *       �ú��������߶������˵����������䳤��
 * 
 ******************************************************************************************************************************/
float C3DRCSDoc::CalculateLength(int v1, int v2)
{
	if(v1==v2)
	{
//		AfxMessageBox( "ͬ�㣬�޷����㳤��!" );
		return 0.0f;
	}
	float dx=m_Vertex[v2].x-m_Vertex[v1].x;
	float dy=m_Vertex[v2].y-m_Vertex[v1].y;
	float dz=m_Vertex[v2].z-m_Vertex[v1].z;

	return sqrt(dx*dx+dy*dy+dz*dz);
}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CalculateLength()
 *
 * ����:
 *   Vertex v1, int v2         -              
 *
 * ˵��:
 *       �ú��������߶������˵����������䳤��
 * 
 ******************************************************************************************************************************/
float C3DRCSDoc::CalculateLength_PSO(Vertex v1, int v2)
{
	if(v1.x==m_Vertex[v2].x && v1.y==m_Vertex[v2].y && v1.z==m_Vertex[v2].z)
	{
//		AfxMessageBox( "ͬ�㣬�޷����㳤��!" );
		return 0.0f;
	}
	float dx=m_Vertex[v2].x-v1.x;
	float dy=m_Vertex[v2].y-v1.y;
	float dz=m_Vertex[v2].z-v1.z;

	return sqrt(dx*dx+dy*dy+dz*dz);
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CalculateArea()
 *
 * ����:
 *   int a, int b, int c         -�����ߵ�����              
 *
 * ˵��:
 *       ���������������ߵĳ��ȼ��������ε����������������߲��ܹ��������Σ��򷵻�-1��
 * 
 ******************************************************************************************************************************/
float C3DRCSDoc::CalculateArea(int a, int b, int c)
{
	float l1=m_Edge[a].length;
	float l2=m_Edge[b].length;
	float l3=m_Edge[c].length;
	if(((l1+l2)<=l3)||((l2+l3)<=l1)||((l3+l1)<=l2))
	{
//		AfxMessageBox( "���ܹ���������!" );
		return -1.0;
	}
	float l=(l1+l2+l3)/2.0;
	return sqrt(l*(l-l1)*(l-l2)*(l-l3));

}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CalculateArea()   
 *
 * ����:
 *   int a, int b, int c         -�����ߵ��������߳�             
 *
 * ˵��:
 *       ���������������ߵĳ��ȼ��������ε����������������߲��ܹ��������Σ��򷵻�-1��
 * 
 ******************************************************************************************************************************/
float C3DRCSDoc::CalculateArea_PSO(int a, double b, double c)
{
	double l1=m_Edge[a].length;
	float l2=b;
	float l3=c;
	if(((l1+l2)<=l3)||((l2+l3)<=l1)||((l3+l1)<=l2))
	{
//		AfxMessageBox( "���ܹ���������!" );
		return -1.0;
	}
	float l=(l1+l2+l3)/2.0;
	return sqrt(l*(l-l1)*(l-l2)*(l-l3));

}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CalculateFacetNormal()
 *
 * ����:
 *   Facet &f         -�����ߵ�����        
 *
 * ˵��:
 *       �ú���������Ƭ���ⷨ��
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::CalculateFacetNormal(Facet &f)
{
	Vertex a = m_Vertex[f.vertex[0]];
	Vertex b = m_Vertex[f.vertex[1]];
	Vertex c = m_Vertex[f.vertex[2]];
	f.nx = (b.y-a.y)*(c.z-b.z)-(b.z-a.z)*(c.y-b.y);
	f.ny = (b.z-a.z)*(c.x-b.x)-(b.x-a.x)*(c.z-b.z);
	f.nz = (b.x-a.x)*(c.y-b.y)-(b.y-a.y)*(c.x-b.x);

	float m = sqrt(f.nx*f.nx+f.ny*f.ny+f.nz*f.nz);

	f.nx /=m;	f.ny /=m;	f.nz /=m;
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   OnFileSave()                
 *
 * ˵��:
 *       �ú��������ж��Ǳ���ΪASCII�ļ���������ļ���
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::OnFileSave() 
{
	if (::gbIsGetData)
	{
		AfxMessageBox( "δ��������!");
		return;
	}
	
	if (::gbDataIsEmpty)
	{
		AfxMessageBox( "����Ϊ��!" );
		return;
	}

	CSaveDlg m_SaveDialog; Sleep(100);
	if(	m_SaveDialog.DoModal()==IDOK)
	{
		xTranslate=m_SaveDialog.m_xTranslate;
		yTranslate=m_SaveDialog.m_yTranslate;
		zTranslate=m_SaveDialog.m_zTranslate;
		xZoomSave=m_SaveDialog.m_xZoomSave;
		yZoomSave=m_SaveDialog.m_yZoomSave;
		zZoomSave=m_SaveDialog.m_zZoomSave;
		m_SaveAsPath=m_SaveDialog.m_SaveAsPath;
		this->m_ReverseFacet=m_SaveDialog.m_ReverseFacet;
		if(m_SaveAsPath.Right(4)==".ast")
			SaveToASCIIFile(m_SaveAsPath);
		else
		{
			if(m_SaveAsPath.Right(4)==".stl")
			{
				//m_SaveAsPath+=".stl";

			SaveToBinaryFile(m_SaveAsPath);}
			else
			{
				if(m_SaveAsPath.Right(4)==".ply")
				//	m_SaveAsPath+=".ply";
	
				SaveToASCIIPlyFile(m_SaveAsPath);
			}
		}
		
		return ;
	}
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   SaveToASCIIFile()
 *
 * ����:
 *   CString savepath        -Ϊ�����ļ�·��              
 *
 * ˵��:
 *       �ú����������ݱ���ΪASCII�ļ���
 * 
 ******************************************************************************************************************************/
BOOL C3DRCSDoc::SaveToASCIIFile(CString savepath)
{
	BeginWaitCursor();
	int n1,n2,n3;
	CString str;
	fstream m_fs;
	m_fs.open(savepath, ios::out);
	m_fs << "solid Created By Wu Zhifang In ICT" << endl;

	this->m_pFr->m_Progress.SetPos(0);
	for(int i=0; i<m_Facet.size();i++)
	{
		if((i%512)==0)
			this->m_pFr->m_Progress.SetPos((i+1)*100/m_Facet.size());

		if(m_Facet[i].flag)
		{
			if(m_ReverseFacet)
			{
				n1=m_Facet[i].vertex[2];
				n2=m_Facet[i].vertex[1];
				n3=m_Facet[i].vertex[0];
				m_fs << "facet normal " << -m_Facet[i].nx << " " << -m_Facet[i].ny << " " << -m_Facet[i].nz << endl;
				m_fs << "outer loop" << endl;
			}
			else
			{
				n1=m_Facet[i].vertex[0];
				n2=m_Facet[i].vertex[1];
				n3=m_Facet[i].vertex[2];
				m_fs << "facet normal " << m_Facet[i].nx << " " << m_Facet[i].ny << " " << m_Facet[i].nz << endl;
				m_fs << "outer loop" << endl;
			}
			m_fs << "vertex " << m_Vertex[n1].x*xZoomSave+xTranslate <<  " " << m_Vertex[n1].y*yZoomSave+yTranslate <<  " " << m_Vertex[n1].z*zZoomSave+zTranslate << endl;
			m_fs << "vertex " << m_Vertex[n2].x*xZoomSave+xTranslate <<  " " << m_Vertex[n2].y*yZoomSave+yTranslate <<  " " << m_Vertex[n2].z*zZoomSave+zTranslate << endl;
			m_fs << "vertex " << m_Vertex[n3].x*xZoomSave+xTranslate <<  " " << m_Vertex[n3].y*yZoomSave+yTranslate <<  " " << m_Vertex[n3].z*zZoomSave+zTranslate << endl;
			m_fs << "endloop" << endl;
			m_fs << "endfacet" << endl;
		}
	}

	m_fs << "endsolid";
	m_fs.close();
	this->m_pFr->m_Progress.SetPos(100);Sleep(100);
	this->m_pFr->m_Progress.SetPos(0);
	EndWaitCursor();
	return TRUE;
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   SaveToBinaryFile()
 *
 * ����:
 *   CString savepath        -Ϊ�����ļ�·��              
 *
 * ˵��:
 *       �ú����������ݱ���Ϊ�������ļ���
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::SaveToBinaryFile(CString savepath)
{
	BeginWaitCursor();
	int n1,n2,n3;
	CString str;
	CString strValueOut(' ',80); 
	CString strSpace(' ',2);
	strValueOut="3DRCS(Wu Zhifang) Binary STL Output"+strValueOut;
	strValueOut=strValueOut.Left(80);
	fstream m_fs;
	m_fs.open(savepath, ios::out|ios::binary);
	m_fs.seekp(0,ios::beg);
	m_fs.write(strValueOut,80);
	m_fs.write((char*)(&m_FacetNumber),4);
	char kong=' ';
	float nx,ny,nz;
	float v1x,v1y,v1z;
	float v2x,v2y,v2z;
	float v3x,v3y,v3z;

	this->m_pFr->m_Progress.SetPos(0);
	
	for(int i=0; i<m_Facet.size();i++)
	{
		if((i%512)==0)
			this->m_pFr->m_Progress.SetPos((i+1)*100/m_Facet.size());

		if(m_Facet[i].flag)
		{
			if(m_ReverseFacet)
			{
				n1=m_Facet[i].vertex[2];	n2=m_Facet[i].vertex[1];	n3=m_Facet[i].vertex[0];
				nx=-m_Facet[i].nx;	ny=-m_Facet[i].ny;	nz=-m_Facet[i].nz;
			}
			else
			{
				n1=m_Facet[i].vertex[0];	n2=m_Facet[i].vertex[1];	n3=m_Facet[i].vertex[2];
				nx=m_Facet[i].nx;	ny=m_Facet[i].ny;	nz=m_Facet[i].nz;
			}
			
			v1x=m_Vertex[n1].x*xZoomSave+xTranslate;	v1y=m_Vertex[n1].y*yZoomSave+yTranslate;	v1z=m_Vertex[n1].z*zZoomSave+zTranslate;
			v2x=m_Vertex[n2].x*xZoomSave+xTranslate;	v2y=m_Vertex[n2].y*yZoomSave+yTranslate;	v2z=m_Vertex[n2].z*zZoomSave+zTranslate;
			v3x=m_Vertex[n3].x*xZoomSave+xTranslate;	v3y=m_Vertex[n3].y*yZoomSave+yTranslate;	v3z=m_Vertex[n3].z*zZoomSave+zTranslate;
						
			m_fs.write((char*)(&nx),4); 	m_fs.write((char*)(&ny),4);		m_fs.write((char*)(&nz),4);
			m_fs.write((char*)(&v1x),4);	m_fs.write((char*)(&v1y),4);	m_fs.write((char*)(&v1z),4);			
			m_fs.write((char*)(&v2x),4);	m_fs.write((char*)(&v2y),4);	m_fs.write((char*)(&v2z),4);			
			m_fs.write((char*)(&v3x),4);	m_fs.write((char*)(&v3y),4);	m_fs.write((char*)(&v3z),4);			
			m_fs.write((char*)(&kong),1);	m_fs.write((char*)(&kong),1);
		}
	}

	m_fs.close();
	this->m_pFr->m_Progress.SetPos(100);Sleep(100);
	this->m_pFr->m_Progress.SetPos(0);
	EndWaitCursor();
}
/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   SaveToASCIIPlyFile()
 *
 * ����:
 *   CString savepath        -Ϊ�����ļ�·��              
 *
 * ˵��:
 *       �ú����������ݱ���ΪASCII��ʽ��PLY�ļ���
 * 
 ******************************************************************************************************************************/
BOOL C3DRCSDoc::SaveToASCIIPlyFile(CString savepath)
{
	BeginWaitCursor();
	float x, y, z;
	float nx, ny ,nz;
	fstream m_fs;
	m_fs.open(savepath, ios::out);
	m_fs << "ply" << endl;
	m_fs << "format ascii 1.0" << endl;
	m_fs << "comment made by Gao JianBO in ICT" << endl;

	m_fs << "element vertex " << m_Vertex.size() <<endl;
	m_fs << "property float x" << endl;
	m_fs << "property float y" << endl;
	m_fs << "property float z" << endl;
	m_fs << "property float nx" << endl;
	m_fs << "property float ny" << endl;
	m_fs << "property float nz" << endl;
	//m_fs << "element face" << m_Facet.size() << endl;
	//m_fs << "property list uchar int vertex_indices" << endl;
	m_fs << "end_header" << endl;
	this->m_pFr->m_Progress.SetPos(0);
	for(int i=0; i<m_Vertex.size();i++)
	{
		if((i%512)==0)
			this->m_pFr->m_Progress.SetPos((i+1)*100/m_Vertex.size());

		if(m_Vertex[i].flag)
		{
			x = m_Vertex[i].x*xZoomSave+xTranslate;
			y = m_Vertex[i].y*yZoomSave+yTranslate;
			z = m_Vertex[i].z*zZoomSave+zTranslate;
			CalculateVertexNormal(i);
			nx = m_Vertex[i].nx;
			ny = m_Vertex[i].ny;
			nz = m_Vertex[i].nz;
			m_fs << x <<  " " << y <<  " " << z << " " << nx << " " << ny << " " << nz << endl;
		}
	}
	m_fs.close();
	this->m_pFr->m_Progress.SetPos(100);Sleep(100);
	this->m_pFr->m_Progress.SetPos(0);
	EndWaitCursor();
	return TRUE;
}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   SaveSTL()               
 *
 * ˵��:
 *       �ú������ڵ��ñ������ݵĽ��̡�
 * 
 ******************************************************************************************************************************/
UINT C3DRCSDoc::SaveSTL(LPVOID lp)
{
	C3DRCSDoc *pThis = (C3DRCSDoc*)lp;   //��ȡ�߳�ָ��
	
	pThis->OnFileSave();

	return 0L;
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   Optimize()              
 *
 * ˵��:
 *       �ú�������ִ�е������Ż��������򻯡�����
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::Optimize()
{
	
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CalculateValue_Adaptive()  By BaiYang on April 25,2014            
 *
 * ˵��:
 *       �ú������ڼ���ÿ��������Ƭ��ƽ̹��,���㷨��������Ӧ�۵����㷨
 * 
 ******************************************************************************************************************************/

void C3DRCSDoc::CalculateValue_Adaptive()
{
/*//	float value;
	double maxvolume=0.0f,maxangle=0.0f;
	long key;
	long facetsize=m_Facet.size();
	if(!m_bCalculateFacetValue)
		m_FacetList=new Face*[HashMaxLish*4];
	
	for(int k=0; k<HashMaxLish*4 ;k++)
	{	m_FacetList[k]=NULL;	}
	for (int i=0;i<facetsize;i++)
	{
		CalculateFacetValue_Adaptive(i);//��������Ӧ�㷨�����������������ǽǶ���������λ��˻�
	/*	if (maxvolume<m_Facet[i].volume)
			maxvolume=m_Facet[i].volume;
		if (maxangle<m_Facet[i].cosOfangle)
			maxangle=m_Facet[i].cosOfangle;
	}

	for (i=0;i<facetsize;i++)
	{
		m_Facet[i].volume/=(maxvolume*1000);
	//	m_Facet[i].cosOfangle/=maxangle;
		m_Facet[i].cosOfangle*=(180/3.1415926);
	}


	for(i=0; i<facetsize;i++)
	{
		if(!m_Facet[i].flag)
			continue;
		
		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*10/facetsize);
		m_Facet[i].value=m_Facet[i].volume/*+m_Facet[i].cosOfangle;
		
		key=long (m_Facet[i].value*100000.0f);
		if(key>HashMaxLish*4)
			key=HashMaxLish*4;
		Face *f= new Face;
		f->next=m_FacetList[key];
		f->index=i;
		m_FacetList[key]=f;
		m_Facet[i].order=key;
	}
	//�������
	FILE *fp;
	fp=fopen("C:\\Users\\baiyang\\Desktop\\cons.dat","w");
	for (i=0;i<facetsize;i++)
		fprintf(fp,"%4d\tcos:%.5f\tvolume:%.10f\tvalue:%.10f\n",i,m_Facet[i].cosOfangle,m_Facet[i].volume,m_Facet[i].value);
	fclose(fp);
	
	//�������
	
	m_bCalculateFacetValue=true;
	
	  */
	float value;
	long key;
	long facetsize=m_Facet.size();
	if(!m_bCalculateFacetValue)             //�ж��Ƿ��Ѿ������ƽ̹��
		m_FacetList=new Face*[HashMaxLish*4];
	
	for(int k=0; k<HashMaxLish*4 ;k++)
	{	m_FacetList[k]=NULL;	}
	
	for(int i=0; i<facetsize;i++)
	{
		if(!m_Facet[i].flag)
			continue;
		
		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*10/facetsize);
		
		value=CalculateFacetValue(i);
		
		key=long (m_Facet[i].value*100000.0f);
		if(key>HashMaxLish*4)
			key=HashMaxLish*4;
		Face *f= new Face;
		f->next=m_FacetList[key];
		f->index=i;
		m_FacetList[key]=f;
		m_Facet[i].order=key;
	}
	
	m_bCalculateFacetValue=true;

}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CalculateValue()              
 *
 * ˵��:
 *       �ú������ڼ���ÿ��������Ƭ��ƽ̹��
 * 
 ******************************************************************************************************************************/

void C3DRCSDoc::CalculateValue()
{
	float value;
	long key;
	long facetsize=m_Facet.size();
	if(!m_bCalculateFacetValue)
		m_FacetList=new Face*[HashMaxLish*4];

	for(int k=0; k<HashMaxLish*4 ;k++)
	{	m_FacetList[k]=NULL;	}

	for(int i=0; i<facetsize;i++)
	{
		if(!m_Facet[i].flag)
			continue;

		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*10/facetsize);
		 
		value=CalculateFacetValue(i);
		
		key=long (m_Facet[i].value*100000.0f);
		if(key>HashMaxLish*4)
			key=HashMaxLish*4;
		Face *f= new Face;
		f->next=m_FacetList[key];
		f->index=i;
		m_FacetList[key]=f;
		m_Facet[i].order=key;
	}

	m_bCalculateFacetValue=true;

}
/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CalculateValue_PSO()  By BaiYang on March 24,2014            
 *
 * ˵��:
 *       �ú������ڼ���ÿ��������Ƭ��ƽ̹��,���㷨����PSO����Ѱ���㷨
 * 
 ******************************************************************************************************************************/

void C3DRCSDoc::CalculateValue_PSO()
{
	float value;
	long key;
	long facetsize=m_Facet.size();
	if(!m_bCalculateFacetValue)
		m_FacetList=new Face*[HashMaxLish*4];

	for(int k=0; k<HashMaxLish*4 ;k++)
	{	m_FacetList[k]=NULL;	}

	for(int i=0; i<facetsize;i++)
	{
		if(!m_Facet[i].flag)
			continue;

		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*10/facetsize);

		value=CalculateFacetValue_PSO(i);
		
		key=long (m_Facet[i].value*100000.0f);
		if(key>HashMaxLish*4)
			key=HashMaxLish*4;
		Face *f= new Face;
		f->next=m_FacetList[key];
		f->index=i;
		m_FacetList[key]=f;
		m_Facet[i].order=key;
	}

	m_bCalculateFacetValue=true;

}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ReductFacetsHash_Adaptive()  Based on WZF
 *   
 *
 * ˵��:
 *       �ú��������������۵���������ģ�ͼ򻯣�
 *            1������������Ƭ��������Ϊ�µ�����ꣻ
 *            2����������Ƭ��������������������У���������ι���Ķ��������Ϊ�µ����ꣻ
 *            3�����¼�����Щ������Ƭ�����ݣ�������Ƭ�����ߵĳ��ȡ�������Ƭ���ⷨ��
 *            4���жϸĶ�����Щ������Ƭ�Ƿ����ȱ�ݣ�������ϴ�������ǣ�����лع����������򣬼�����
 *            5����������Ƭ��������������������У���������������Ϊ�˵�ıߣ���Ӧ�˵��Ϊ�µ㡣
 *            6�����µ����������У��������������ֱ�ӵ����ڹ�ϵ�����¼�����Щ������Ƭ����Ľ��Ʒ����������¼�����Щ�����ε�ƽָ̹��
 *            7��ɾ���������μ���ֱ�����ڵ�����������Ƭ���������־����Ϊfalse��
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::ReductFacetsHash_Adaptive()
{
	int F[3];      //��¼���������Ƭֱ�����ڵ�����������Ƭ������
	int m_facetnum;//��¼�õ㣨��������Ƭ����������֮һ������������Ƭ�ĸ���
	int mF;        //��¼�õ㣨��������Ƭ����������֮һ��ĳһ����������Ƭ��������
	Vertex vertex; //�µ����ʱ����
	vector<Facet> m_bF; //������ݱ����ĵ�����������Ƭ�����ڻع� 
	bool m_undo=false;
	float nx,ny,nz;                
	float nprocess=0.0f;
	float cosOfangle;
	double zoom=0.0;
	long m_facetsize=m_Facet.size();
	int i,j,k,kk,m_b,m_bbv;

	for(i=0; i<HashMaxLish*4;i++)    //��forѭ������������
	{		
		if(m_FacetList[i]==NULL)
			continue;
	
		nprocess=m_ReduceNum/(float)m_facetsize;

		if(nprocess>=m_ReduceRateEdit)
			break;
		
		int m_index=m_FacetList[i]->index;



		if((m_Facet[m_index].area<=0)||(m_Facet[m_index].order==-1)||
			(m_Facet[m_index].aFacet[0]==-1)||(m_Facet[m_index].aFacet[1]==-1)||(m_Facet[m_index].aFacet[2]==-1))
		{
			m_Facet[m_index].order=-1;
			Face *f=m_FacetList[i];
			m_FacetList[i]=f->next;
			delete f;
			i--;
			continue;
		}     //��������ɾ��Ҫ��������δ������ɾ��
		
		F[0]=m_Facet[m_index].aFacet[0];
		F[1]=m_Facet[m_index].aFacet[1];
		F[2]=m_Facet[m_index].aFacet[2];

		if(m_Vertex[m_Facet[m_index].vertex[0]].bBoundary||m_Vertex[m_Facet[m_index].vertex[1]].bBoundary||m_Vertex[m_Facet[m_index].vertex[2]].bBoundary)
		{
			m_Facet[m_index].order=-1;
			Face *f=m_FacetList[i];
			m_FacetList[i]=f->next;
			delete f;
			i--;
			continue;
		}                  //�������б�Ե��������δ������ɾ��

		//�жϸ��������Ƿ���������ֱ�������������γɱջ�
		long samevertex=JudgeFacetsInLoop(m_index, F[0], F[1], F[2]);   //����ֵ��Ϊ-1��ʾ������m_Facet[m_index]�����������������
		if(samevertex>=0)
		{
			if(DeleteFacetInLoop(samevertex, m_index, F[0], F[1], F[2]))
			{
				i=-1;
				continue;
			}
			else
			{
				m_Facet[m_index].order=-1;
				Face *f=m_FacetList[i];
				m_FacetList[i]=f->next;
				delete f;
				i--;
				continue;
			}
		}
		else
		{
			//�жϸ��������Ƿ��������αջ�ֱ������
			long whichfacet=JudgeFacetsNearLoop(m_index, F[0], F[1], F[2]);
			if(whichfacet>=0)
			{
				F[0]=m_Facet[whichfacet].aFacet[0];
				F[1]=m_Facet[whichfacet].aFacet[1];
				F[2]=m_Facet[whichfacet].aFacet[2];

				samevertex=JudgeFacetsInLoop(whichfacet, F[0], F[1], F[2]);
				if(DeleteFacetInLoop(samevertex, whichfacet, F[0], F[1], F[2]))
				{
					i=-1;
					continue;
				}
				else
				{
					m_Facet[m_index].order=-1;
					Face *f=m_FacetList[i];
					m_FacetList[i]=f->next;
					delete f;
					i--;
					continue;
				}
			}
			
		}

		m_bF.clear();
		m_undo=false;

		vertex.x=m_Facet[m_index].x;
		vertex.y=m_Facet[m_index].y;
		vertex.z=m_Facet[m_index].z;
		
		m_Vertex.push_back(vertex);    //���µ�ѹ��㼯β��
		
		//����m_Facet[m_indexi]�����˵�Ϊ�˵㲻Ϊ��ɾ������������Ƭ���Ϊ�µ������������Ƭ�����µ��û�����m_Facet[m_index]�˵�
		for(k=0; k<3; k++)
		{
			m_facetnum=m_Vertex[m_Facet[m_index].vertex[k]].facet.size();  //��ȡm_Facet[m_index]��Ƭ�ĵ�k���˵����������
			for(j=0; j<m_facetnum; j++)
			{
				mF=m_Vertex[m_Facet[m_index].vertex[k]].facet[j];  //m_Facet[m_index]��Ƭ�ĵ�k���˵�ĵ�j�����������mF
				if((mF!=m_index)&&(mF!=F[0])&&(mF!=F[1])&&(mF!=F[2])) //ȷ������ΪmF����Ƭ��Ϊm_Facet[m_index]��Ƭ����ֱ������
				{
					m_bF.push_back(m_Facet[mF]);
					//�ж�m_Facet[m_index]��Ƭ�ĵ�k���˵㣬����Ϊ����ΪmF����Ƭ�ڼ����˵�

					for( kk=0; kk<3; kk++)
						if(m_Facet[m_index].vertex[k]==m_Facet[mF].vertex[kk])	//�����m_Facet[m_index]��Ƭ�ĵ�k���˵�����mF��Ƭ�ĵ�kk���˵�
							break;

					nx=m_Facet[mF].nx;  ny=m_Facet[mF].ny;  nz=m_Facet[mF].nz;    
			
		            
					m_Facet[mF].vertex[kk]=m_Vertex.size()-1;    //��mF��Ƭ�ĵ�kk���˵��Ϊ�µ㣬���㼯���һ����
					//����ζ��mF��Ƭ�ĵڣ�kk+1��%3���ߣ����øĶ���
						
						//ͬʱ����mF��Ƭ���Ե�kk���˵�Ϊ�˵�������ߣ�����kk����(kk+2)%3���ߣ��Ķ˵㣬Ҳ�ĳ��µ�
					if(m_Edge[m_Facet[mF].edge[kk]].facet==mF)   //�ж�mF��kk����(kk+2)%3����������mF����������mF��ֱ������������
						m_Edge[m_Facet[mF].edge[kk]].vertex1=m_Vertex.size()-1; //����mF����������Ϊ�µ�
					else
						m_Edge[m_Facet[mF].edge[kk]].vertex2=m_Vertex.size()-1; //���򣬽����յ��Ϊ�µ�
						
					if(m_Edge[m_Facet[mF].edge[(kk+2)%3]].facet==mF)
						m_Edge[m_Facet[mF].edge[(kk+2)%3]].vertex2=m_Vertex.size()-1;
					else
						m_Edge[m_Facet[mF].edge[(kk+2)%3]].vertex1=m_Vertex.size()-1;
					
					
					//������Щ��m_Facet[m_index]�������˵�Ϊ�˵�������η��������߳��������������Щ���������Ϊ�µ������������

					
					m_Edge[m_Facet[mF].edge[kk]].length=CalculateLength(m_Facet[mF].vertex[kk],m_Facet[mF].vertex[(kk+1)%3]);
					if(m_Edge[m_Facet[mF].edge[kk]].length<=0.0f)
					{  m_undo=true; break;  }

					m_Edge[m_Facet[mF].edge[(kk+2)%3]].length=CalculateLength(m_Facet[mF].vertex[(kk+2)%3],m_Facet[mF].vertex[kk]);
					if(m_Edge[m_Facet[mF].edge[(kk+2)%3]].length<=0.0f) 
					{  m_undo=true; break;  }
					
					m_Facet[mF].area=CalculateArea(m_Facet[mF].edge[0], m_Facet[mF].edge[1], m_Facet[mF].edge[2]);

					if(m_Facet[mF].area<=0.0f)
					{  m_undo=true; break;  }

					
					CalculateFacetNormal(m_Facet[mF]);

 					cosOfangle=acos(m_Facet[mF].nx*nx+m_Facet[mF].ny*ny+m_Facet[mF].nz*nz);
         
 					if(cosOfangle>m_ReduceAngleThreshold)
 					{  m_undo=true; break;  }
 
					m_Vertex[m_Vertex.size()-1].facet.push_back(mF);
				}
			}

			if(true==m_undo)
				break;
		}

		//������Ϣ���޸ĵĴ��򣬻ع���������Ƭ����������Ƭ����Ϣ
		if(true==m_undo)
		{
			int mm=0; 
			for(m_b=0;m_b<k+1; m_b++)
			{
				m_facetnum=m_Vertex[m_Facet[m_index].vertex[m_b]].facet.size();  
				for(j=0; j<m_facetnum; j++)
				{
					mF=m_Vertex[m_Facet[m_index].vertex[m_b]].facet[j]; 
					if((m_Facet[mF].vertex[0]==m_Vertex.size()-1)||(m_Facet[mF].vertex[1]==m_Vertex.size()-1)||(m_Facet[mF].vertex[2]==m_Vertex.size()-1))
					{
						m_Facet[mF]=m_bF[mm++];
						for(m_bbv=0; m_bbv<3;m_bbv++)
						{
							if(m_Edge[m_Facet[mF].edge[m_bbv]].facet==mF)
							{
								m_Edge[m_Facet[mF].edge[m_bbv]].vertex1=m_Facet[mF].vertex[m_bbv];
								m_Edge[m_Facet[mF].edge[m_bbv]].vertex2=m_Facet[mF].vertex[(m_bbv+1)%3];
							}
							else
							{
								m_Edge[m_Facet[mF].edge[m_bbv]].vertex1=m_Facet[mF].vertex[(m_bbv+1)%3];
								m_Edge[m_Facet[mF].edge[m_bbv]].vertex2=m_Facet[mF].vertex[m_bbv];
							}
						}

						for(m_bbv=0; m_bbv<3; m_bbv++)
							m_Edge[m_Facet[mF].edge[m_bbv]].length=CalculateLength(m_Facet[mF].vertex[m_bbv],m_Facet[mF].vertex[(m_bbv+1)%3]);
					}
				}
			}
			m_Vertex.pop_back(); //���µ�ӵ㼯�е���
			m_Facet[m_index].order=-1;
			Face *f=m_FacetList[i];
			m_FacetList[i]=f->next;
			delete f;
			i--;
			continue;
		}
		
		//������
		CreateEdges(F[0],F[1],F[2],m_index);

		CalculateVertexNormal(m_Vertex.size()-1);

		int Pfacetnum=m_Vertex[m_Vertex.size()-1].facet.size();

		//���¼���߽绷�ϸ����˵�ķ�����
		for(int j=0; j<Pfacetnum; j++)
		{
			for( k=0; k<3; k++)
			{
				int km=m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[k];
				if(km==m_Vertex.size()-1) //jƬ��˵�k��Ϊ�µ�
					break;
			}
			CalculateVertexNormal(m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[(k+1)%3]);
		}

		//���¼������ݶ�����ĺ������Ӱ��������Ƭ��ƽָ̹�������Ǳ߽绷����������Ƭƽֵ̹�仯�������
		int km, mfac;
		vector<int> mm_Plist; mm_Plist.clear();
		bool m_hadP=false;
		for( j=0; j<Pfacetnum; j++)
		{
			for(int k=0; k<3; k++)
			{
				km=m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[k];
				if(km==m_Vertex.size()-1)
					break;
			}
			km=m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[(k+1)%3];
			mfac=m_Vertex[km].facet.size(); //�ñ߽绷��ΪjƬ��˵�k��������Ƭ����
			for(int m=0; m<mfac; m++)
			{
				m_hadP=false;
				for(int mmkk=0; mmkk<mm_Plist.size();mmkk++)
				{
					if(m_Vertex[km].facet[m]==mm_Plist[mmkk])
					{
						m_hadP=true;break;
					}
				}
				if(m_hadP)
					continue;
				mm_Plist.push_back(m_Vertex[km].facet[m]);


 				CalculateFacetValue(m_Vertex[km].facet[m]);
				

				long order = m_Facet[m_Vertex[km].facet[m]].order;
				
				if((order!=-1)&&(m_FacetList[order]!=NULL))
				{
					Face *p=m_FacetList[order],*q;
					if(p->index==m_Vertex[km].facet[m])
					{
						m_FacetList[order]=p->next;
						m_Facet[m_Vertex[km].facet[m]].order=-1;
						delete p;
					}
					else
					{
						q=p->next;
						while(q!=NULL)
						{
							if(q->index==m_Vertex[km].facet[m])
							{
								p->next=q->next;delete q; 
								m_Facet[m_Vertex[km].facet[m]].order=-1;
								break;
							}
							else
							{
								p=q; q=q->next;
							}
						}
					}
				}

				long m_key=long (m_Facet[m_Vertex[km].facet[m]].value*100000.0f);
				if(m_key>HashMaxLish*4)
					m_key=HashMaxLish*4;
				Face *f,*p0;
				f= new Face;
				p0=m_FacetList[m_key];
				m_FacetList[m_key]=f;
				f->next=p0;
				f->index=m_Vertex[km].facet[m];
				m_Facet[m_Vertex[km].facet[m]].order=m_key;

			}
		}
		
		//ɾ��Ԫ��
		DeleteFacetHash(F[0], F[1], F[2], m_index);
		m_Facet[m_index].order=-1;
		Face *f=m_FacetList[i];
		m_FacetList[i]=f->next;
		delete f;
		i=-1;

	}

}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ReductFacetsHash()
 *   
 *
 * ˵��:
 *       �ú��������������۵���������ģ�ͼ򻯣�
 *            1������������Ƭ��������Ϊ�µ�����ꣻ
 *            2����������Ƭ��������������������У���������ι���Ķ��������Ϊ�µ����ꣻ
 *            3�����¼�����Щ������Ƭ�����ݣ�������Ƭ�����ߵĳ��ȡ�������Ƭ���ⷨ��
 *            4���жϸĶ�����Щ������Ƭ�Ƿ����ȱ�ݣ�������ϴ�������ǣ�����лع����������򣬼�����
 *            5����������Ƭ��������������������У���������������Ϊ�˵�ıߣ���Ӧ�˵��Ϊ�µ㡣
 *            6�����µ����������У��������������ֱ�ӵ����ڹ�ϵ�����¼�����Щ������Ƭ����Ľ��Ʒ����������¼�����Щ�����ε�ƽָ̹��
 *            7��ɾ���������μ���ֱ�����ڵ�����������Ƭ���������־����Ϊfalse��
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::ReductFacetsHash()
{
	int F[3];      //��¼�Ը�������Ƭֱ�����ڵ�����������Ƭ������
	int m_facetnum;//��¼�õ㣨��������Ƭ����������֮һ������������Ƭ�ĸ���
	int mF;        //��¼�õ㣨��������Ƭ����������֮һ��ĳһ����������Ƭ��������
	
	Vertex vertex; //�µ����ʱ����
	vector<Facet> m_bF; //������ݱ����ĵ�����������Ƭ�����ڻع�
	bool m_undo=false;
	float nx,ny,nz;
	float nprocess=0.0f;
	float cosOfangle, pingzhi;
	double zoom=0.0;
	long m_facetsize=m_Facet.size();
	int i,j,k,m_bbv;
	for(i=0; i<HashMaxLish*4;i++)   //���
	{
		if(m_FacetList[i]==NULL)
			continue;
	
		nprocess=m_ReduceNum/(float)m_facetsize;
		if(m_ReduceNum%128==0)
		{
			this->m_pFr->m_Progress.SetPos(nprocess*90+10);
		}

		if(nprocess>=m_ReduceRateEdit)
			break;                           //���ﵽɾ���ʣ�����
		
		int m_index=m_FacetList[i]->index;

		if(m_Facet[m_index].value>m_DistanceThreshold)
			continue;
		
		if((m_Facet[m_index].area<=0)||(m_Facet[m_index].order==-1)||
			(m_Facet[m_index].aFacet[0]==-1)||(m_Facet[m_index].aFacet[1]==-1)||(m_Facet[m_index].aFacet[2]==-1))
		{
			m_Facet[m_index].order=-1;
			Face *f=m_FacetList[i];
			m_FacetList[i]=f->next;
			delete f;
			i--;
			continue;
		}
		
		F[0]=m_Facet[m_index].aFacet[0];
		F[1]=m_Facet[m_index].aFacet[1];
		F[2]=m_Facet[m_index].aFacet[2];

		if(m_Vertex[m_Facet[m_index].vertex[0]].bBoundary||m_Vertex[m_Facet[m_index].vertex[1]].bBoundary||m_Vertex[m_Facet[m_index].vertex[2]].bBoundary)
		{
			m_Facet[m_index].order=-1;
			Face *f=m_FacetList[i];
			m_FacetList[i]=f->next;
			delete f;
			i--;
			continue;
		}

		//�жϸ��������Ƿ���������ֱ�������������γɱջ�
		long samevertex=JudgeFacetsInLoop(m_index, F[0], F[1], F[2]);
		if(samevertex>=0)
		{
			if(DeleteFacetInLoop(samevertex, m_index, F[0], F[1], F[2]))
			{
				i=-1;
				continue;
			}
			else
			{
				m_Facet[m_index].order=-1;
				Face *f=m_FacetList[i];
				m_FacetList[i]=f->next;
				delete f;
				i--;
				continue;
			}
		}
		else
		{
			//�жϸ��������Ƿ��������αջ�ֱ������
			long whichfacet=JudgeFacetsNearLoop(m_index, F[0], F[1], F[2]);
			if(whichfacet>=0)
			{
				F[0]=m_Facet[whichfacet].aFacet[0];
				F[1]=m_Facet[whichfacet].aFacet[1];
				F[2]=m_Facet[whichfacet].aFacet[2];

				samevertex=JudgeFacetsInLoop(whichfacet, F[0], F[1], F[2]);
				if(DeleteFacetInLoop(samevertex, whichfacet, F[0], F[1], F[2]))
				{
					i=-1;
					continue;
				}
				else
				{
					m_Facet[m_index].order=-1;
					Face *f=m_FacetList[i];
					m_FacetList[i]=f->next;
					delete f;
					i--;
					continue;
				}
			}
			
		}

		m_bF.clear();
		m_undo=false;

		vertex.x=m_Facet[m_index].x;
		vertex.y=m_Facet[m_index].y;
		vertex.z=m_Facet[m_index].z;
		
		m_Vertex.push_back(vertex);    //���µ�ѹ��㼯β��
		
		//����m_Facet[m_index]�����˵�Ϊ�˵㲻Ϊ��ɾ������������Ƭ���Ϊ�µ������������Ƭ�����µ��û�����m_Facet[m_index]�˵�
		for(k=0; k<3; k++)
		{
			m_facetnum=m_Vertex[m_Facet[m_index].vertex[k]].facet.size();  //��ȡm_Facet[m_index]��Ƭ�ĵ�k���˵����������
			for(j=0; j<m_facetnum; j++)
			{  int kk;
				mF=m_Vertex[m_Facet[m_index].vertex[k]].facet[j];  //m_Facet[m_index]��Ƭ�ĵ�k���˵�ĵ�j�����������mF
				if((mF!=m_index)&&(mF!=F[0])&&(mF!=F[1])&&(mF!=F[2])) //ȷ������ΪmF����Ƭ��Ϊm_Facet[m_index]��Ƭ����ֱ������
				{
					m_bF.push_back(m_Facet[mF]);
					//�ж�m_Facet[m_index]��Ƭ�ĵ�k���˵㣬����Ϊ����ΪmF����Ƭ�ڼ����˵�

					for(kk=0; kk<3; kk++)
						if(m_Facet[m_index].vertex[k]==m_Facet[mF].vertex[kk])	//�����m_Facet[m_index]��Ƭ�ĵ�k���˵�����mF��Ƭ�ĵ�kk���˵�
							break;
					
					m_Facet[mF].vertex[kk]=m_Vertex.size()-1;    //��mF��Ƭ�ĵ�kk���˵��Ϊ�µ㣬���㼯���һ����
					//����ζ��mF��Ƭ�ĵڣ�kk+1��%3���ߣ����øĶ���
						
						//ͬʱ����mF��Ƭ���Ե�kk���˵�Ϊ�˵�������ߣ�����kk����(kk+2)%3���ߣ��Ķ˵㣬Ҳ�ĳ��µ�
					if(m_Edge[m_Facet[mF].edge[kk]].facet==mF)   //�ж�mF��kk����(kk+2)%3����������mF����������mF��ֱ������������
						m_Edge[m_Facet[mF].edge[kk]].vertex1=m_Vertex.size()-1; //����mF����������Ϊ�µ�
					else
						m_Edge[m_Facet[mF].edge[kk]].vertex2=m_Vertex.size()-1; //���򣬽����յ��Ϊ�µ�
						
					if(m_Edge[m_Facet[mF].edge[(kk+2)%3]].facet==mF)
						m_Edge[m_Facet[mF].edge[(kk+2)%3]].vertex2=m_Vertex.size()-1;
					else
						m_Edge[m_Facet[mF].edge[(kk+2)%3]].vertex1=m_Vertex.size()-1;
					
					pingzhi=(m_Edge[m_Facet[mF].edge[kk]].length+m_Edge[m_Facet[mF].edge[(kk+2)%3]].length)/m_Edge[m_Facet[mF].edge[(kk+1)%3]].length;
					//������Щ��m_Facet[m_index]�������˵�Ϊ�˵�������η��������߳��������������Щ���������Ϊ�µ������������
					nx=m_Facet[mF].nx;  ny=m_Facet[mF].ny;  nz=m_Facet[mF].nz;
					
					m_Edge[m_Facet[mF].edge[kk]].length=CalculateLength(m_Facet[mF].vertex[kk],m_Facet[mF].vertex[(kk+1)%3]);
					if(m_Edge[m_Facet[mF].edge[kk]].length<=0.0f)
					{  m_undo=true; break;  }

					m_Edge[m_Facet[mF].edge[(kk+2)%3]].length=CalculateLength(m_Facet[mF].vertex[(kk+2)%3],m_Facet[mF].vertex[kk]);
					if(m_Edge[m_Facet[mF].edge[(kk+2)%3]].length<=0.0f) 
					{  m_undo=true; break;  }
					
					m_Facet[mF].area=CalculateArea(m_Facet[mF].edge[0], m_Facet[mF].edge[1], m_Facet[mF].edge[2]);

					if(m_Facet[mF].area<=0.0f) 
					{  m_undo=true; break;  }
					
					CalculateFacetNormal(m_Facet[mF]);

 				    cosOfangle=acos(m_Facet[mF].nx*nx+m_Facet[mF].ny*ny+m_Facet[mF].nz*nz);
//////////					
					if((cosOfangle>m_ReduceAngleThreshold)&&(pingzhi<=1.2))
					{  m_undo=true; break;  }
					else
						if(cosOfangle>m_ReduceAngleThreshold)
							{  m_undo=true; break;  }


					m_Vertex[m_Vertex.size()-1].facet.push_back(mF);
				}
			}
			if(true==m_undo)
				break;
		}
		
		//������Ϣ���޸ĵĴ��򣬻ع���������Ƭ����������Ƭ����Ϣ
		if(true==m_undo)
		{
			int mm=0;
			for(int m_b=0;m_b<k+1; m_b++)
			{
				m_facetnum=m_Vertex[m_Facet[m_index].vertex[m_b]].facet.size();  
				for(int j=0; j<m_facetnum; j++)
				{
					mF=m_Vertex[m_Facet[m_index].vertex[m_b]].facet[j]; 
					if((m_Facet[mF].vertex[0]==m_Vertex.size()-1)||(m_Facet[mF].vertex[1]==m_Vertex.size()-1)||(m_Facet[mF].vertex[2]==m_Vertex.size()-1))
					{
						m_Facet[mF]=m_bF[mm++];
						for(m_bbv=0; m_bbv<3;m_bbv++)
						{
							if(m_Edge[m_Facet[mF].edge[m_bbv]].facet==mF)
							{
								m_Edge[m_Facet[mF].edge[m_bbv]].vertex1=m_Facet[mF].vertex[m_bbv];
								m_Edge[m_Facet[mF].edge[m_bbv]].vertex2=m_Facet[mF].vertex[(m_bbv+1)%3];
							}
							else
							{
								m_Edge[m_Facet[mF].edge[m_bbv]].vertex1=m_Facet[mF].vertex[(m_bbv+1)%3];
								m_Edge[m_Facet[mF].edge[m_bbv]].vertex2=m_Facet[mF].vertex[m_bbv];
							}
						}

						for(m_bbv=0; m_bbv<3; m_bbv++)
							m_Edge[m_Facet[mF].edge[m_bbv]].length=CalculateLength(m_Facet[mF].vertex[m_bbv],m_Facet[mF].vertex[(m_bbv+1)%3]);
					}
				}
			}
			m_Vertex.pop_back(); //���µ�ӵ㼯�е���
			m_Facet[m_index].order=-1;
			Face *f=m_FacetList[i];
			m_FacetList[i]=f->next;
			delete f;
			i--;
			continue;
		}
		
		//������
		CreateEdges(F[0],F[1],F[2],m_index);

		CalculateVertexNormal(m_Vertex.size()-1);

		int Pfacetnum=m_Vertex[m_Vertex.size()-1].facet.size();

		//���¼���߽绷�ϸ����˵�ķ�����
		for(j=0; j<Pfacetnum; j++)
		{
			for( k=0; k<3; k++)
			{
				int km=m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[k];
				if(km==m_Vertex.size()-1) //jƬ��˵�k��Ϊ�µ�
					break;
			}
			CalculateVertexNormal(m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[(k+1)%3]);
		}

		//���¼������ݶ�����ĺ������Ӱ��������Ƭ��ƽָ̹�������Ǳ߽绷����������Ƭƽֵ̹�仯�������
		int km, mfac;
		vector<int> mm_Plist; mm_Plist.clear();
		bool m_hadP=false;
		for( j=0; j<Pfacetnum; j++)
		{
			for(k=0; k<3; k++)
			{
				km=m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[k];
				if(km==m_Vertex.size()-1)
					break;
			}
			km=m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[(k+1)%3];
			mfac=m_Vertex[km].facet.size(); //�ñ߽绷��ΪjƬ��˵�k��������Ƭ����
			for(int m=0; m<mfac; m++)
			{
				m_hadP=false;
				for(int mmkk=0; mmkk<mm_Plist.size();mmkk++)
				{
					if(m_Vertex[km].facet[m]==mm_Plist[mmkk])
					{
						m_hadP=true;break;
					}
				}
				if(m_hadP)
					continue;
				mm_Plist.push_back(m_Vertex[km].facet[m]);
				
				CalculateFacetValue(m_Vertex[km].facet[m]);
				
				long order = m_Facet[m_Vertex[km].facet[m]].order;
				
				if((order!=-1)&&(m_FacetList[order]!=NULL))
				{
					Face *p=m_FacetList[order],*q;
					if(p->index==m_Vertex[km].facet[m])
					{
						m_FacetList[order]=p->next;
						m_Facet[m_Vertex[km].facet[m]].order=-1;
						delete p;
					}
					else
					{
						q=p->next;
						while(q!=NULL)
						{
							if(q->index==m_Vertex[km].facet[m])
							{
								p->next=q->next;delete q; 
								m_Facet[m_Vertex[km].facet[m]].order=-1;
								break;
							}
							else
							{
								p=q; q=q->next;
							}
						}
					}
				}

				long m_key=long (m_Facet[m_Vertex[km].facet[m]].value*100000.0f);
				if(m_key>HashMaxLish*4)
					m_key=HashMaxLish*4;
				Face *f,*p0;
				f= new Face;
				p0=m_FacetList[m_key];
				m_FacetList[m_key]=f;
				f->next=p0;
				f->index=m_Vertex[km].facet[m];
				m_Facet[m_Vertex[km].facet[m]].order=m_key;

			}
		}
		
		//ɾ��Ԫ��
		DeleteFacetHash(F[0], F[1], F[2], m_index);
		m_Facet[m_index].order=-1;
		Face *f=m_FacetList[i];
		m_FacetList[i]=f->next;
		delete f;
		i=-1;
		
	}
	this->m_pFr->m_Progress.SetPos(100); Sleep(100); this->m_pFr->m_Progress.SetPos(0);
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   DeleteFacet()
 *   DeleteFacetHash()   
 *
 * ����:
 *   int f0, int f1, int f2, int i     -�ֱ�����������Ƭ����ֱ�����ڵ�����������Ƭ
 *
 * ˵��:
 *       �ú���ɾ�����������Ƭ���߼����㣬�������־��Ϊfalse��
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::DeleteFacetHash(int f0, int f1, int f2, int i)
{
	int F[3]={f0,f1,f2};
	//ɾ��m_Facet[i]����������������Ƭ
	for(int j=0; j<3; j++)
	{
		m_Edge[m_Facet[F[j]].edge[0]].flag=false;
		m_Edge[m_Facet[F[j]].edge[1]].flag=false;
		m_Edge[m_Facet[F[j]].edge[2]].flag=false;
		if(m_Facet[F[j]].flag)
		{
			m_Facet[F[j]].flag=false;
			m_ReduceNum++;
		}
		
		long order = m_Facet[F[j]].order;
		
		if((order!=-1)&&(m_FacetList[order]!=NULL))
		{
			Face *p=m_FacetList[order],*q;
			if(p->index==F[j])
			{
				m_FacetList[order]=p->next;
				m_Facet[F[j]].order=-1;
				delete p;
			} 
			else
			{
				q=p->next;
				while(q!=NULL)
				{
					if(q->index==F[j])
					{
						p->next=q->next;
						delete q; 
						m_Facet[F[j]].order=-1;
						break;
					}
					else
					{
						p=q; q=q->next;
					}
				}
			}
		}
	}	
	
	//ɾ��m_Facet[i]������Ƭ�����䶥��
	m_Vertex[m_Facet[i].vertex[0]].flag=false;
	m_Vertex[m_Facet[i].vertex[1]].flag=false;
	m_Vertex[m_Facet[i].vertex[2]].flag=false;
	m_Facet[i].flag=false;
	m_ReduceNum++;
	
	m_FacetNumber-=4;
	m_VertexNumber-=2;
	m_EdgeNumber-=6;
}


/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CreateEdges()   
 *
 * ����:
 *   int f0, int f1, int f2, int i     -�ֱ�����������Ƭ����ֱ�����ڵ�����������Ƭ
 *
 * ˵��:
 *       �ú������ڴ����µ㵽��֮��Ӧ�߽��ıߣ�ͬʱ����ɾ���ĸ�������Ƭ��ֱ������������Ƭ����������Ƭ�����ڹ�ϵ��
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::CreateEdges(int f0, int f1, int f2, int i)
{
	Edge edge;
	int f[3]={f0,f1,f2};
	for(int j=0; j<3; j++)
	{
		for(int m=0; m<3; m++)
		{
			if((m_Facet[f[j]].edge[m]==m_Facet[i].edge[0])||(m_Facet[f[j]].edge[m]==m_Facet[i].edge[1])||
				(m_Facet[f[j]].edge[m]==m_Facet[i].edge[2]))//����������������ε�ĳһ�����Ǹ������εı�
			{
				//����f[0]\f[1]\f[2]��һ�˵����µ�֮��ı�
				edge.vertex1=m_Vertex.size()-1;
				edge.vertex2=m_Facet[f[j]].vertex[(m+2)%3];
				edge.facet=m_Facet[f[j]].aFacet[(m+2)%3];
				edge.rfacet=m_Facet[f[j]].aFacet[(m+1)%3];
				edge.length=CalculateLength(edge.vertex1,edge.vertex2);
				
				/***********************ɾ����Զ˵������һ�������棬��f[j]***********************/
				for(int k=0; k<m_Vertex[m_Facet[f[j]].vertex[(m+2)%3]].facet.size();k++)
				{
					if(m_Vertex[m_Facet[f[j]].vertex[(m+2)%3]].facet[k]==f[j])
					{
						vector<int>::iterator itef = m_Vertex[m_Facet[f[j]].vertex[(m+2)%3]].facet.begin()+k;
						m_Vertex[m_Facet[f[j]].vertex[(m+2)%3]].facet.erase(itef);
						break;
					}
				}
				
				/****************************���µ������������Ƭ��ӱ�****************************/
				if(m_Edge[m_Facet[f[j]].edge[(m+1)%3]].facet==f[j])
				{
					if(m_Facet[m_Edge[m_Facet[f[j]].edge[(m+1)%3]].rfacet].edge[0]==m_Facet[f[j]].edge[(m+1)%3])
						m_Facet[m_Edge[m_Facet[f[j]].edge[(m+1)%3]].rfacet].edge[0]=m_Edge.size();
					else
					{
						if(m_Facet[m_Edge[m_Facet[f[j]].edge[(m+1)%3]].rfacet].edge[1]==m_Facet[f[j]].edge[(m+1)%3])
							m_Facet[m_Edge[m_Facet[f[j]].edge[(m+1)%3]].rfacet].edge[1]=m_Edge.size();
						else
							m_Facet[m_Edge[m_Facet[f[j]].edge[(m+1)%3]].rfacet].edge[2]=m_Edge.size();
					}
				}
				else
				{
					if(m_Facet[m_Edge[m_Facet[f[j]].edge[(m+1)%3]].facet].edge[0]==m_Facet[f[j]].edge[(m+1)%3])
						m_Facet[m_Edge[m_Facet[f[j]].edge[(m+1)%3]].facet].edge[0]=m_Edge.size();
					else
					{
						if(m_Facet[m_Edge[m_Facet[f[j]].edge[(m+1)%3]].facet].edge[1]==m_Facet[f[j]].edge[(m+1)%3])
							m_Facet[m_Edge[m_Facet[f[j]].edge[(m+1)%3]].facet].edge[1]=m_Edge.size();
						else
							m_Facet[m_Edge[m_Facet[f[j]].edge[(m+1)%3]].facet].edge[2]=m_Edge.size();
					}
				}
				
				if(m_Edge[m_Facet[f[j]].edge[(m+2)%3]].facet==f[j])
				{
					if(m_Facet[m_Edge[m_Facet[f[j]].edge[(m+2)%3]].rfacet].edge[0]==m_Facet[f[j]].edge[(m+2)%3])
						m_Facet[m_Edge[m_Facet[f[j]].edge[(m+2)%3]].rfacet].edge[0]=m_Edge.size();
					else
					{
						if(m_Facet[m_Edge[m_Facet[f[j]].edge[(m+2)%3]].rfacet].edge[1]==m_Facet[f[j]].edge[(m+2)%3])
							m_Facet[m_Edge[m_Facet[f[j]].edge[(m+2)%3]].rfacet].edge[1]=m_Edge.size();
						else
							m_Facet[m_Edge[m_Facet[f[j]].edge[(m+2)%3]].rfacet].edge[2]=m_Edge.size();
					}
				}
				else
				{
					if(m_Facet[m_Edge[m_Facet[f[j]].edge[(m+2)%3]].facet].edge[0]==m_Facet[f[j]].edge[(m+2)%3])
						m_Facet[m_Edge[m_Facet[f[j]].edge[(m+2)%3]].facet].edge[0]=m_Edge.size();
					else
					{
						if(m_Facet[m_Edge[m_Facet[f[j]].edge[(m+2)%3]].facet].edge[1]==m_Facet[f[j]].edge[(m+2)%3])
							m_Facet[m_Edge[m_Facet[f[j]].edge[(m+2)%3]].facet].edge[1]=m_Edge.size();
						else
							m_Facet[m_Edge[m_Facet[f[j]].edge[(m+2)%3]].facet].edge[2]=m_Edge.size();
					}
				}
			}
		}
		m_Edge.push_back(edge);

		/*********************************��������������Ƭ�����ڹ�ϵ*********************************/
		for(int k=0; k<3; k++)
		{
			if(m_Facet[f[j]].aFacet[k] ==i)
			{
				if(m_Facet[m_Facet[f[j]].aFacet[(k+1)%3]].aFacet[0] == f[j])
					m_Facet[m_Facet[f[j]].aFacet[(k+1)%3]].aFacet[0]=m_Facet[f[j]].aFacet[(k+2)%3];
				else
				{
					if(m_Facet[m_Facet[f[j]].aFacet[(k+1)%3]].aFacet[1] == f[j])
						m_Facet[m_Facet[f[j]].aFacet[(k+1)%3]].aFacet[1]=m_Facet[f[j]].aFacet[(k+2)%3];
					else
						m_Facet[m_Facet[f[j]].aFacet[(k+1)%3]].aFacet[2]=m_Facet[f[j]].aFacet[(k+2)%3];
				}
				
				
				if(m_Facet[m_Facet[f[j]].aFacet[(k+2)%3]].aFacet[0] == f[j])
					m_Facet[m_Facet[f[j]].aFacet[(k+2)%3]].aFacet[0]=m_Facet[f[j]].aFacet[(k+1)%3];
				else
				{
					if(m_Facet[m_Facet[f[j]].aFacet[(k+2)%3]].aFacet[1] == f[j])
						m_Facet[m_Facet[f[j]].aFacet[(k+2)%3]].aFacet[1]=m_Facet[f[j]].aFacet[(k+1)%3];
					else
						m_Facet[m_Facet[f[j]].aFacet[(k+2)%3]].aFacet[2]=m_Facet[f[j]].aFacet[(k+1)%3];
				}
			}
		}
	}
}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ReduceMesh()
 *
 * ����:
 *   ��              - ָ���̵߳��õ���
 *
 * ����ֵ:
 *   void            - ������;
 *
 * ˵��:
 *       �ú������ڵ���ִ��ģ�ͼ򻯵ġ�
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::ReduceMesh()
{ 
	m_ReduceAngleThreshold=(this->m_pFr->m_AngleThreshold)*3.14159/180.0;
	m_DistanceThreshold=this->m_pFr->m_DistanceRate;
	m_ReduceRateEdit=this->m_pFr->m_ReduceRate2/100.0f;
	this->CalculateValue();
	this->ReductFacetsHash();

//	this->CalculateStandard();
	long mFN=m_Facet.size();
	m_ReduceRate=100.0*m_ReduceNum/(double)mFN;

}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ReduceMesh2()   By BaiYang on March 5,2014
 *
 * ����:
 *   ��              - ָ���̵߳��õ���
 *
 * ����ֵ:
 *   void            - ������;
 *
 * ˵��:
 *       �ú������ڵ���ִ��ģ�ͼ򻯵ġ�
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::ReduceMesh2()
{ 
 	m_ReduceAngleThreshold=(this->m_pFr->m_pCtrlWnd->m_nReduceAngleThreshold)*3.14159/180.0;
 	m_DistanceThreshold=this->m_pFr->m_pCtrlWnd->m_nReduceDistanceThreshold;
 	m_ReduceRateEdit=this->m_pFr->m_pCtrlWnd->m_nReduceRate/100.0f;
	
	this->CalculateValue_PSO();
	this->ReductFacetsHash();

//	this->CalculateStandard();
	long mFN=m_Facet.size();
	m_ReduceRate=100.0*m_ReduceNum/(double)mFN;

//	AfxMessageBox("successful!");

}


void C3DRCSDoc::CalculateStandard()
{ 
	
	//	vector<double> Standard1;
	double temp1;
	int i;
//---------------------------------1������߳�
	
	for (i=0;i<m_Edge.size();i++)
	{
		m_Edge[i].length=CalculateLength(m_Edge[i].vertex1,m_Edge[i].vertex2);
	}
//---------------------------------2���������
	for (i=0;i<m_Facet.size();i++)
	{
		m_Facet[i].area=CalculateArea(m_Facet[i].edge[0],m_Facet[i].edge[1],m_Facet[i].edge[2]);
	}
//---------------------------------3������������
	
	for (i=0;i<m_Facet.size();i++)
	{
		temp1=4*double(sqrt((double)3))*m_Facet[i].area/(m_Edge[m_Facet[i].edge[0]].length*m_Edge[m_Facet[i].edge[0]].length
			                            +m_Edge[m_Facet[i].edge[1]].length*m_Edge[m_Facet[i].edge[1]].length
										+m_Edge[m_Facet[i].edge[2]].length*m_Edge[m_Facet[i].edge[2]].length);
		m_Facet[i].StandardT=temp1;
	}
	//���
	FILE *fp;
	fp=fopen("C:\\Users\\baiyang\\Desktop\\StandradData.txt","w");
// 	fprintf(fp,"����%d����Ƭ\n",m_Facet.size());
	for (i=0;i<m_Facet.size();i++)
	{
		fprintf(fp,"%.5f\n",m_Facet[i].StandardT);
	}
	fclose(fp);
	
	
	
}



bool C3DRCSDoc::TrackContour()
{
	for(int i=0; i<m_Edge.size();i++)
	{
		if((m_Edge[i].flag)&&(m_Edge[i].rfacet==-1))
		{
			if(((m_Vertex[m_Edge[i].vertex1].z-z_glMin)<3.0)&&((m_Vertex[m_Edge[i].vertex2].z-z_glMin)<3.0))
			m_DownEdge.push_back(m_Edge[i]);
			if(((z_glMax-m_Vertex[m_Edge[i].vertex1].z)<3.0)&&((z_glMax-m_Vertex[m_Edge[i].vertex2].z)<3.0))
			m_UpEdge.push_back(m_Edge[i]);
		}
	}
	if(m_UpEdge.size()>=3)
		m_UpHole=true;
	if(m_DownEdge.size()>=3)
		m_DownHole=true;
	if((!m_DownHole)&&(!m_UpHole))
		return false;

	Edge tempt;
	if(m_UpHole)
	{
		for(int i=0; i<m_UpEdge.size()-1;i++)
		{
			for(int j=i+1; j<m_UpEdge.size();j++)
			{
				if(m_UpEdge[i].vertex2==m_UpEdge[j].vertex1)
				{
					tempt=m_UpEdge[i+1];
					m_UpEdge[i+1]=m_UpEdge[j];
					m_UpEdge[j]=tempt;
					break;
				}
			}
			
		}
/*		vector<Edge> ::iterator itE;
		for( i=0; i<m_UpEdge.size();i++)
		{
			itE=m_UpEdge.begin();
			int xxxxx=m_UpEdge.size();
			if(m_UpEdge[(i+1)%xxxxx].length<0.1)
			{
				m_UpEdge[i].vertex2=m_UpEdge[(i+1)%xxxxx].vertex2;
				m_UpEdge.erase(itE+(i+1)%xxxxx);
			}
		}*/
	}

	if(m_DownHole)
	{
		for(int i=0; i<m_DownEdge.size();i++)
		{
			for(int j=i+1; j<m_DownEdge.size();j++)
			{
				if(m_DownEdge[i].vertex2==m_DownEdge[j].vertex1)
				{
					tempt=m_DownEdge[i+1];
					m_DownEdge[i+1]=m_DownEdge[j];
					m_DownEdge[j]=tempt;
					break;
				}
			}
		}

/*		vector<Edge> ::iterator itE;
		for( i=0; i<m_DownEdge.size();i++)
		{
			itE=m_DownEdge.begin();
			int xxxxx=m_DownEdge.size();
			if(m_DownEdge[(i+1)%xxxxx].length<0.1)
			{
				m_DownEdge[i].vertex2=m_DownEdge[(i+1)%xxxxx].vertex2;
				m_DownEdge.erase(itE+(i+1)%xxxxx);
			}
		}*/
	}
	return true;
}


void C3DRCSDoc::PatchMesh()
{
	//����������������
	m_UpEdge.clear(); 
	m_DownEdge.clear();

	if(!TrackContour())   //�������� ����
		return;

	Edge temptE;
	int temptI, siz,i;

	//�ж�ģ��������Ƭ������ָ��ģ���ڲ���������
	int reverseornot;
	if((m_Facet[m_XmaxFacet].nx>0)&&(m_Facet[m_YmaxFacet].ny>0))
		reverseornot=IDYES; 
	else
	{
		if((m_Facet[m_XmaxFacet].nx<0)&&(m_Facet[m_YmaxFacet].ny<0))
			reverseornot=IDNO;
		else
			reverseornot=AfxMessageBox("  δ��ȷ��ģ�͵�������Ƭ����ָ����ȷ����\nָ��ģ����࣬�밴���ǡ���\nָ��ģ���ڲ����밴���񡱡�",MB_YESNOCANCEL|MB_ICONQUESTION);
	}

	switch(reverseornot)
	{
	case IDYES:
		if(m_UpEdge.size()>0)
		{
			for(i=0; i<m_UpEdge.size();i++)
			{
				temptI=m_UpEdge[i].vertex1;
				m_UpEdge[i].vertex1=m_UpEdge[i].vertex2;
				m_UpEdge[i].vertex2=temptI; 
			}
			siz=m_UpEdge.size();
			for(i=0; i<siz/2; i++)
			{
				temptE=m_UpEdge[i];
				m_UpEdge[i]=m_UpEdge[siz-i-1];
				m_UpEdge[siz-i-1]=temptE;
			}
		}
		PatchUpandDownMesh(m_UpEdge, false);
		PatchUpandDownMesh(m_DownEdge, true);
		break;
		
	case IDNO:
		if(m_DownEdge.size()>0)
		{
			for(i=0; i<m_DownEdge.size();i++)
			{
				temptI=m_DownEdge[i].vertex1;
				m_DownEdge[i].vertex1=m_DownEdge[i].vertex2;
				m_DownEdge[i].vertex2=temptI;  
			}
			
			siz=m_DownEdge.size();
			for(i=0; i<siz/2; i++)
			{
				temptE=m_DownEdge[i];
				m_DownEdge[i]=m_DownEdge[siz-i-1];
				m_DownEdge[siz-i-1]=temptE;
			}
		}
		PatchUpandDownMesh(m_UpEdge, true);
		PatchUpandDownMesh(m_DownEdge, false);
		break;
	default :
		return;
	}
	
	ExtractLines();
	RelatePointsEdgesFacets();
	this->m_pFr->m_Progress.SetPos(0);
	
}

int C3DRCSDoc::IsOnLeft(Vertex v, Edge L)
{
	double side, x1, x2, x0, y1, y2, y0;
	x1 = m_Vertex[L.vertex1].x;   y1 = m_Vertex[L.vertex1].y;
	x2 = m_Vertex[L.vertex2].x;   y2 = m_Vertex[L.vertex2].y;
	x0 = v.x;   y0 = v.y;
	
	side = (x2-x1)*(y0-y1)-(y2-y1)*(x0-x1);

	if(side>0.0)
		return 1;            //����L���
	if(side<0.0)
		return -1;           //����L�ұ�
	else 
		return 0;            //����L��
}

int C3DRCSDoc::IsXother(Edge L, int n, vector<Edge> &UDedge)
{
	int rrt=-1;
	for(int i=1; i< UDedge.size(); i++)
	{
		if(n!=i)
		{
			int x1=IsOnLeft(m_Vertex[L.vertex1],UDedge[i]);
			int x2=IsOnLeft(m_Vertex[L.vertex2],UDedge[i]);
			int x3=IsOnLeft(m_Vertex[UDedge[i].vertex1],L);
			int x4=IsOnLeft(m_Vertex[UDedge[i].vertex2],L);
			if((x1*x2==-1)&&(x3*x4==-1))
			{
				rrt = 1;             //��������ȫ�ཻ
				break;
			}
			if((x1*x2==0)&&(x3*x4==0))
			{
				rrt = 0;             //�����߲���ȫ�ཻ
			}
		}
	}
	return rrt;
}


int C3DRCSDoc::IsInEdge(Edge L, vector<Edge> &UDedge)
{
	for(int i=0; i<UDedge.size(); i++)
	{
		if(((L.vertex1==UDedge[i].vertex1)&&(L.vertex2==UDedge[i].vertex2))||
			((L.vertex1==UDedge[i].vertex2)&&(L.vertex2==UDedge[i].vertex1)))
			return i;
	}
	return -1;              //���L��߼��غϣ����ظñ߼���ţ����غϷ���-1
}

double C3DRCSDoc::CalculateCircleRadius( int p1, int p2, int p3)
{
	double x1=m_Vertex[p1].x;
	double y1=m_Vertex[p1].y;
	double x2=m_Vertex[p2].x;
	double y2=m_Vertex[p2].y;
	double x3=m_Vertex[p3].x;
	double y3=m_Vertex[p3].y;
	
	double m1=0.0,  m2=0.0;
	double mx1=0.0, mx2=0.0;
	double my1=0.0, my2=0.0;
	double dx=0.0,  dy=0.0;
	double rsqr=0.0, drsqr=0.0;
	double xc=0.0,  yc=0.0;
	      
	if (abs(y2 - y1) <1.0e-2)
	{
		m2 = -(x3 - x2) /(double) (y3 - y2);
		mx2 = (x2 + x3) / 2.0;
		my2 = (y2 + y3) / 2.0;
		xc = (x2 + x1) / 2.0;
		yc = m2 * (xc - mx2) + my2;
	}
	else if (abs(y3 - y2) < 1.0e-2 )
	{
		m1 = -(x2 - x1) / (double)(y2 - y1);
		mx1 = (x1 + x2) / 2.0;
		my1 = (y1 + y2) / 2.0;
		xc = (x3 + x2) / 2.0;
		yc = m1 * (xc - mx1) + my1;
	}
	else
	{
		m1=-((x2-x1)/(double)(y2-y1));
		m2=-((x3-x2)/(double)(y3-y2));
		mx1=(x1+x2)/2.0;
		mx2=(x2+x3)/2.0;
		my1=(y1+y2)/2.0;
		my2=(y2+y3)/2.0;
		xc=(m1*mx1-m2*mx2+my2-my1)/(m1-m2);
		yc=m1*(xc-mx1)+my1;
	}
	
	dx = x2- xc;
	dy = y2 - yc;
	rsqr = dx * dx + dy * dy;
	return rsqr;
}

void C3DRCSDoc::PatchUpandDownMesh(vector<Edge> &UorDedge, bool reverse)
{
	Edge L12;  
	vector<Edge> m_EofPro; //��ѡ�Ķ������ڵ���

	int huai=0;
	float nnum=UorDedge.size();
	Edge tem;
	int i;
	while(UorDedge.size()>0)
	{
		L12=UorDedge[0];   //L12���Ƕ���εĵ�һ����
		m_EofPro.clear();  //��ѡ���㼯������ѡ�߼��ĵڶ����˵㣩��ա�

		Edge LVL;          //����ѡ�еĻ�ѡ�㣨��ѡ�ߵĵڶ����˵㣩��
		Facet facet;       //��ʱ�����ݣ���Ŵ����������Ϣ�����ڲ��������漯��ĩβ
		vector<Edge> ::iterator itE;
		int xxx=0;
		bool xxb=false;

		//**************�����еı�����ѡ��ѡ�ߣ��㣩**************//
		for(i=1; i<UorDedge.size(); i++)
		{
			Edge LK=UorDedge[i];
			if(IsOnLeft(m_Vertex[LK.vertex2], L12)<=0)  //����õ㲻���߶�L12��࣬��õ㲻�迼��
				continue;
			if(LK.vertex2==L12.vertex1)             //����գ�����
				continue;
			Edge L1k, Lk2;    //�����ѡ�㵽L12�����˵������
			L1k.vertex1=L12.vertex1;
			L1k.vertex2=LK.vertex2;
			Lk2.vertex1=LK.vertex2;
			Lk2.vertex2=L12.vertex2;
		
			int isx1=IsXother(L1k, i, UorDedge);
			if(1==isx1)
				continue;

			int isx2=IsXother(Lk2, i, UorDedge);
			if(1==isx2) //Ҫ����������������һ�������еı��ཻ����õ㲻�迼�ǡ�
				continue;

			if((isx1==0)&&(isx2==0) )			
			{
				int inie1=IsInEdge(L1k, UorDedge);
				int inie2=IsInEdge(Lk2, UorDedge);
				if((inie1!=-1)&&(inie2!=-1))   //�������߶��������б���ȫ�غϣ�����߼�
				{
					m_EofPro.clear();
					m_EofPro.push_back(LK);
					break;
				}

				if((inie1!=-1)||(inie2!=-1))
				{
					if(false==xxb)
					{
						xxx=m_EofPro.size();
						xxb=true;
					}
				}
			}
			m_EofPro.push_back(LK);  //���������������Ļ�ѡ������ѡ�㼯���߼����С�
		}

		if(m_EofPro.size()==0)
		{
			AfxMessageBox("0");
			break;
		}
		
		//�ӻ�ѡ�㼯����ѡ�����ʵĵ㣬��Ϊ����ѡ���㡣
		bool mIn=false,mHad=false,had=false;
		double m_r=1.0e36,r=1.0e36;
		for( i=0; i<m_EofPro.size(); i++)
		{
			Edge L10,L02;            //L10,L02�ֱ��������ѡ����L12�����˵������
			L10.vertex1 = L12.vertex1;
			L10.vertex2 = m_EofPro[i].vertex2;
			L02.vertex1 = m_EofPro[i].vertex2;
			L02.vertex2 = L12.vertex2;
			mIn=false;


           //��֤û�к�ѡ����������������


			for(int k=0; k<m_EofPro.size(); k++)
			{
				
				if(i!=k)
				if((IsOnLeft(m_Vertex[m_EofPro[k].vertex2], L10)<=0)&&(IsOnLeft(m_Vertex[m_EofPro[k].vertex2], L02)<=0))
				{
					mIn=true;//�������Բ��С�ĺ�ѡ��
					break;
				}
			}
			
			if(false==mIn)
			{
				r=CalculateCircleRadius( L12.vertex1, L12.vertex2, m_EofPro[i].vertex2);
				if(had==false)
					m_r=r+100.0f;
				if(m_r>r)
				{
					m_r=r;
					LVL=m_EofPro[i];
					had=mHad=true;
				}
//				break;  
			}
		}

		if(false==mHad)//δ�ܴӻ�ѡ�����ҵ�һ�����㣬ʹ����L12���ɵ����Բ��δ����������ѡ�㡣�򽫸ö���ָ��Ϊ��һ���㡣
			LVL=m_EofPro[xxx];
			
		if((false==mHad)&&(false==xxb))
		{
			AfxMessageBox("wu");
		}


		Edge L10,L02;              //L10,L02�ֱ��������ѡ����L12�����˵������
		L10.vertex1 = L12.vertex1;
		L10.vertex2 = LVL.vertex2;
		L02.vertex1 = LVL.vertex2;
		L02.vertex2 = L12.vertex2;

		//�洢������Ƭ
		if(false==reverse)
		{
			facet.vertex[0]=L12.vertex1;
			facet.vertex[1]=L12.vertex2;
			facet.vertex[2]=LVL.vertex2;
		}
		else
		{
			facet.vertex[0]=LVL.vertex2;
			facet.vertex[1]=L12.vertex2;
			facet.vertex[2]=L12.vertex1;
		}

		m_Vertex[L12.vertex1].facet.push_back(m_FacetNumber);
		m_Vertex[L12.vertex2].facet.push_back(m_FacetNumber);
		m_Vertex[LVL.vertex2].facet.push_back(m_FacetNumber);
		

		m_Facet.push_back(facet);
		CalculateFacetNormal(m_Facet[m_FacetNumber]);
		m_FacetNumber++;


		//�ж���������
		int NL10=IsInEdge(L10, UorDedge);  
		int NL02=IsInEdge(L02, UorDedge);

		if((-1==NL10)&&(-1==NL02))  //L10��L02 �����Ǳ߽���
		{
			Edge newedge;
			newedge.vertex1=L12.vertex1;
			newedge.vertex2=LVL.vertex2;
			UorDedge[0].vertex1=LVL.vertex2;
			UorDedge.push_back(newedge);
		}
		else
		{
			if((-1!=NL10)&&(-1!=NL02))  //L10��L02 ���Ǳ߽��ߡ�
			{
				itE=UorDedge.begin();
				if(NL10>NL02)
				{
					UorDedge.erase(itE+NL10);
					UorDedge.erase(itE+NL02);
				}
				else
				{
					UorDedge.erase(itE+NL02);
					UorDedge.erase(itE+NL10);
				}
				UorDedge.erase(itE);
			}
			else
			{
				if(-1==NL10)  //NL02�Ǳ߽���
				{
					itE=UorDedge.begin();
					UorDedge[0].vertex2=LVL.vertex2;
					UorDedge.erase(itE+NL02); 
				}
				else  //NL10�Ǳ߽���
				{
					itE=UorDedge.begin();
					UorDedge[0].vertex1=LVL.vertex2;
					UorDedge.erase(itE+NL10);
				}
			}  //Lo1��L2o ���Ǳ߽��ߡ�else
		}  //Lo1��L2o �����Ǳ߽��� else

		int pos=UorDedge.size();
		if(pos%10==0)
			this->m_pFr->m_Progress.SetPos(100-int(pos*100/nnum));
	}
}

void C3DRCSDoc::SetShowPara()
{
	CShowParaDlg m_ShowParaDlg; 
	if(	m_ShowParaDlg.DoModal()==IDOK)
	{
		
	}

}



void C3DRCSDoc::GetModeinfo()
{
	CalculatModeVolume(); 
	CalculateSurfaceArea();
	CModeInfo m_MoldinfoDlg; 
	m_MoldinfoDlg.SetEdits();
	m_MoldinfoDlg.DoModal(); 
}

void C3DRCSDoc::SetZoomAndMid()
{ 
	x_glMid = (x_glMax + x_glMin) / 2.0 ;
	y_glMid = (y_glMax + y_glMin) / 2.0 ; 
	z_glMid = (z_glMax + z_glMin) / 2.0 ;
	this->m_pFr->m_pCtrlWnd->m_xZoom=150.f/(float)(x_glMax - x_glMin);
	this->m_pFr->m_pCtrlWnd->m_yZoom=150.f/(float)(y_glMax - y_glMin);
	this->m_pFr->m_pCtrlWnd->m_zZoom=150.f/(float)(z_glMax - z_glMin);

	if(this->m_pFr->m_pCtrlWnd->m_xZoom>this->m_pFr->m_pCtrlWnd->m_yZoom)
		this->m_pFr->m_pCtrlWnd->m_xZoom=this->m_pFr->m_pCtrlWnd->m_yZoom;
	else
		this->m_pFr->m_pCtrlWnd->m_yZoom=this->m_pFr->m_pCtrlWnd->m_xZoom;

	if(this->m_pFr->m_pCtrlWnd->m_xZoom>this->m_pFr->m_pCtrlWnd->m_zZoom)
		this->m_pFr->m_pCtrlWnd->m_xZoom=this->m_pFr->m_pCtrlWnd->m_zZoom;
	else
		this->m_pFr->m_pCtrlWnd->m_zZoom=this->m_pFr->m_pCtrlWnd->m_xZoom;
	this->m_pFr->m_pCtrlWnd->m_xZoom=float(int((this->m_pFr->m_pCtrlWnd->m_xZoom+0.005)*100))/100.0f;
	this->m_pFr->m_pCtrlWnd->m_yZoom=this->m_pFr->m_pCtrlWnd->m_xZoom;
	this->m_pFr->m_pCtrlWnd->m_zZoom=this->m_pFr->m_pCtrlWnd->m_xZoom;

	this->m_pFr->m_pCtrlWnd->SetXYZzoom();
}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   OpenPics()
 *
 * ����:
 *   ��                - ����
 *
 * ����ֵ:
 *   void              - ������;
 *
 * ˵��:
 *       �ú������ڴ�ϵ��BMPͼ��
 * 
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::OpenPics()
{
	this->m_pFr = (CMainFrame*)AfxGetApp()->m_pMainWnd;   //*****��ȡ�����ָ��

	static char mFilter[]="BMPͼ���ļ�(*.bmp)|*.bmp|PCGͼ���ļ�(*.pcg)|*.pcg|�����ļ�All Files(*.*)|*.*||";
		
	//�ļ��򿪶Ի���
	CFileDialog PicOpenDlg(TRUE,_T("bmp"),_T("*.bmp"),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,mFilter);
	int structsize=88; 
	DWORD dwVersion,dwWindowsMajorVersion,dwWindowsMinorVersion; 
	//���Ŀǰ�Ĳ���ϵͳ��GetVersion�����÷����MSDN 
	dwVersion = GetVersion(); 
	dwWindowsMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion))); 
	dwWindowsMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion))); 
	
	// ������еĲ���ϵͳ��Windows NT/2000 
	if (dwVersion < 0x80000000)  
		structsize =88;//��ʾ�µ��ļ��Ի��� 
	else 
		//���еĲ���ϵͳWindows 95/98  
		structsize =76;//��ʾ�ϵ��ļ��Ի��� 
	PicOpenDlg.m_ofn.lStructSize=structsize;

	char *fileNamesBuf = new char[512 * _MAX_PATH]; 
	fileNamesBuf[0] = '\0'; 
	DWORD maxBytes = 512 * _MAX_PATH; 
	PicOpenDlg.m_ofn.lpstrFile = fileNamesBuf; 
	PicOpenDlg.m_ofn.nMaxFile = maxBytes; 
	
	if (IDOK != PicOpenDlg.DoModal())
		return;

	this->Clear();   //���֮ǰ������

	this->m_pFr->m_pCtrlWnd->DisableControlWhileConstruct();
	this->m_pFr->m_wndStatusBar.SetWindowText("����ͼƬ���أ����Ժ򡭡�");

	
	POSITION pos=PicOpenDlg.GetStartPosition();
	m_FilePathArray.RemoveAll(); //��������ͼ��·��ϵ�е�����
	while(pos)
	{
		m_FilePathArray.Add(PicOpenDlg.GetNextPathName(pos));
	}
		
	m_nFileNum=int(m_FilePathArray.GetSize());

	SortFileArry();            /*���ļ�·��ϵ�н�������*/



	CStringArray m_FilePathArrayNew;
//	CPCG m_PCG;
	CString Mid;
	if (strstr(m_FilePathArray[1], ".pcg") != NULL)
	{
		
		for (int i = 0; i<m_nFileNum; i++)
		{
			Mid = m_FilePathArray.GetAt(i);
			m_PCG.LoadDLL(Mid, i);
			m_FilePathArrayNew.Add(Mid);
		}

		float  spacedistance;
		float aaa;
		char ch[30],ch2[30];
		SCANPARAMETER sp;
		SYSTEMPARAMETER st;
	
		spacedistance = sp.RadialPosition;
		aaa = st.RayEngery;
		//CPCG pchr;
		//spacedistance = pchr.GetLayerSpace(m_FilePathArray[0]);
		sprintf(ch,"%f", spacedistance);
		sprintf(ch2,"%f", aaa);
		
		AfxMessageBox(ch);
		
		/*CConstruct dlgCons;
		dlgCons.UpdateData();
		dlgCons.m_ScanParam = sp.TotalLayers;
		dlgCons.UpdateData(FALSE);*/

		m_FilePathArray.RemoveAll(); 
		for (int j = 0; j < m_nFileNum; j++)
		{
			Mid = m_FilePathArrayNew.GetAt(j);
			m_FilePathArray.Add(Mid);
		}

	}




	/*****************��ȡͼƬ��Ϣ*************/
	CFile FileInfo;
	FileInfo.Open(m_FilePathArray.GetAt(0), CFile::modeRead);
	GetFileInfo(&FileInfo);  //���ú�������ȡͼƬ��Ϣ//
	FileInfo.Close();

	m_Pixels = new BYTE[(m_nFileNum+4)*m_nHeight*m_nWidth];  //Ϊ���ص㼯�����ڴ�

	SetStartandEnd();     /*������¸��㼯*/

	/*********��ȡͼ���ļ��еĵ㼯*******/
	CString m_Path;	
	for(int i=0; i<m_nFileNum; i++)
	{
		CFile file;
		m_Path=m_FilePathArray.GetAt(i);
		file.Open(m_Path, CFile::modeRead);
		if(!ReadPicFiles(&file,i+2))
		{
			AfxMessageBox( "�򲻿������ļ�!");
			file.Close();
			return;
		}
		file.Close();
		this->m_pFr->m_Progress.SetPos(i*100/m_nFileNum);
	}

	gbIsGetData=false;
	gbDataIsEmpty=false;

	//this->m_pFr->m_pCtrlWnd->FillListBox(); //��ͼ���ļ�����ʾ���б����
	this->m_pFr->m_pProView->m_bShowBmp=true;
	this->m_pFr->m_pCtrlWnd->m_bGetPics=true;
	this->m_pFr->InvalidateAllWnd();

	this->m_pFr->m_Progress.SetPos(0);
	this->m_pFr->m_wndStatusBar.SetWindowText("ͼƬ������ɣ��������ؽ��������ؽ�ģ��!");
	this->m_pFr->m_pCtrlWnd->EnablControlWhileConstruct();

	return;	

}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   GetFileInfo()
 *
 * ����:
 *   CFile *file       - �ļ�����
 *
 * ����ֵ:
 *   void              - ������;
 *
 * ˵��:
 *       ��ȡͼ�����Ϣ���߶ȡ���ȡ���ɫ��
 * 
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::GetFileInfo(CFile *file)
{
	BYTE *lpBitmap, *lpDib;
	int nLen = file->GetLength();
	lpBitmap= new BYTE[nLen];
	file->Read(lpBitmap, nLen);
	if(lpBitmap[0]!='B' && lpBitmap[1]!='M')
	{
		AfxMessageBox( "��λͼ�ļ�!" );
		delete [] lpBitmap;
		return;
	}
	lpDib =lpBitmap + sizeof(BITMAPFILEHEADER);
	BYTE *lpBits;
	BITMAPINFOHEADER *pInfo;
	RGBQUAD *pPalette;
	pInfo = (BITMAPINFOHEADER *) lpDib;
	lpBits = lpDib+sizeof(BITMAPINFOHEADER);

	m_nWidth = pInfo->biWidth;
	m_nHeight = pInfo->biHeight;
	m_nBitCount=pInfo->biBitCount;
	m_nByteWidth = BYTE_PER_LINE(m_nWidth,m_nBitCount);

	PaletteSize = (1<<pInfo->biBitCount);
	
	if(pInfo->biClrUsed!=0 && pInfo->biClrUsed<PaletteSize)
		PaletteSize =pInfo->biClrUsed;
	
	pPalette= (RGBQUAD *) lpBits;
	
	lpBits += sizeof(RGBQUAD)*PaletteSize;
	
	m_lpPalette = new BYTE[sizeof(RGBQUAD)*PaletteSize];
	memcpy(m_lpPalette,pPalette,sizeof(RGBQUAD)*PaletteSize);
	
	delete lpBitmap;

}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   ReadPicFiles()
 *
 * ����:
 *   CFile *file       - ͼ���ļ�
 *   int    n          - ͼ���ļ������
 *
 * ����ֵ:
 *   BOOL              - ��ȡ�ɹ����� TRUE������ FALSE
 *
 * ˵��:
 *       �ú�����ѭ�����ã����ڶ�ȡͼ���ļ��е�������Ϣ��
 * 
 * 
 ******************************************************************************************************************************/
BOOL C3DRCSDoc::ReadPicFiles(CFile *file, int n)
{
	BYTE *FlpBitmap;
	int nLen = file->GetLength();
	FlpBitmap= new BYTE[nLen];
	file->Read(FlpBitmap, nLen);

	if(FlpBitmap[0]!='B' && FlpBitmap[1]!='M')
	{
		AfxMessageBox( "��λͼ�ļ�!");
		delete [] FlpBitmap;
		return FALSE;
	}
	
	float cr;
	if(m_nBitCount==8)
	{		
		if(m_lpBits)
			delete [] m_lpBits;
				
		FlpBitmap += sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*PaletteSize;			
		m_lpBits =new BYTE[m_nWidth*m_nHeight];		
		memcpy(m_lpBits,FlpBitmap,m_nWidth*m_nHeight);
		
		for(int i=0 ;i<m_nHeight;i++)
		{
			for(int j=0;j<m_nByteWidth;j++)
			{
				if(j<m_nWidth)
				{
					cr = sqrt(float(2*i-m_nHeight)*(2*i-m_nHeight)+(2*j-m_nWidth)*(2*j-m_nWidth))+8;
//					if(cr<m_nHeight)
						m_Pixels[n*m_nHeight*m_nWidth+i*m_nWidth+j]=/*255-*/m_lpBits[m_nByteWidth*i+j];
//					else
//						m_Pixels[n*m_nHeight*m_nWidth+i*m_nWidth+j]=0;
				}
			}
		}
	}
	else
	{
		AfxMessageBox( "��λͼ�ļ�����!" );
		return FALSE;
	}
	
	FlpBitmap -=( sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*PaletteSize);
	delete [] FlpBitmap;
	return TRUE;
}

//MarchingTetrahedra
UINT C3DRCSDoc::MarchingTetrahedra(LPVOID lp)
{
	C3DRCSDoc *pThis = (C3DRCSDoc*)lp;   //��ȡ�߳�ָ��
	pThis->m_pFr->m_pCtrlWnd->DisableControlWhileConstruct();
	CString str="���ڽ���ģ���ؽ������Ժ򡭡�";
	pThis->m_pFr->m_wndStatusBar.SetWindowText(str);

	if(!m_Vertex.empty())	m_Vertex.clear();
	if(!m_Facet.empty())	m_Facet.clear();
	if(!m_Edge.empty())		m_Edge.clear();
	m_FacetNumber=m_EdgeNumber=m_VertexNumber=0;

	pThis->m_PointList = new Point3D*[HashMaxLish*2];
	for(int i=0;i<HashMaxLish*2;i++)  {pThis->m_PointList[i]=NULL;}

	Point3D m_Point;
	Vertex vertex;
	Facet m_facet;
	bool m_samePoint = false; 
	int x, y, z, iX=0, iY=0,iZ=0, m_index=-1;
	int iVertex, iTetrahedron, iVertexInACube, ivertex[3];
	int iEdge, iVert0, iVert1, iEdgeFlags, iTriangle, iCorner, iFlagIndex = 0;
	float  afCubeValue[8], afTetrahedronValue[4];
	float fOffset, fInvOffset, fValue = 0.0;
	pThis->m_XconZoom=pThis->m_pFr->m_pCtrlWnd->m_XconZoom;
	pThis->m_YconZoom=pThis->m_pFr->m_pCtrlWnd->m_YconZoom;
	pThis->m_ZconZoom=pThis->m_pFr->m_pCtrlWnd->m_ZconZoom;
	CVect3 asTetrahedronPosition[4], asCubePosition[8], asEdgeVertex[6], Points[3];
	
	for( iZ = 0; iZ < pThis->m_nFileNum+3; iZ++)
	{
        for( iY = 0; iY < pThis->m_nHeight-1; iY++)
		{
			for( iX = 0; iX < pThis->m_nWidth-1; iX++)
			{
                for(iVertex = 0; iVertex < 8; iVertex++)
				{
					asCubePosition[iVertex].x = iX + a2fVertexOffset[iVertex][0];
					asCubePosition[iVertex].y = iY + a2fVertexOffset[iVertex][1];
					asCubePosition[iVertex].z = iZ + a2fVertexOffset[iVertex][2];
				}
				for(iVertex = 0; iVertex < 8; iVertex++)
				{
					x=asCubePosition[iVertex].x;
					y=asCubePosition[iVertex].y;
					z=asCubePosition[iVertex].z;
					afCubeValue[iVertex] = pThis->m_Pixels[z*pThis->m_nHeight*pThis->m_nWidth+pThis->m_nWidth*y+x];
				}
				
				for(iTetrahedron = 0; iTetrahedron < 6; iTetrahedron++)
				{
					for(iVertex = 0; iVertex < 4; iVertex++)
					{
						iVertexInACube = a2iTetrahedronsInACube[iTetrahedron][iVertex];
						asTetrahedronPosition[iVertex].x = asCubePosition[iVertexInACube].x*pThis->m_XconZoom;
						asTetrahedronPosition[iVertex].y = asCubePosition[iVertexInACube].y*pThis->m_YconZoom;
						asTetrahedronPosition[iVertex].z = asCubePosition[iVertexInACube].z*pThis->m_ZconZoom;
						afTetrahedronValue[iVertex] = afCubeValue[iVertexInACube];
					}
					
					iFlagIndex = 0;
					fValue = 0.0;
					for(iVertex = 0; iVertex < 4; iVertex++)
					{
						if(afTetrahedronValue[iVertex] <= pThis->m_TargetValue) 
							iFlagIndex |= 1<<iVertex;
					}
					iEdgeFlags = aiTetrahedronEdgeFlags[iFlagIndex];
					
					if(iEdgeFlags == 0)
						continue;
					
					for(iEdge = 0; iEdge < 6; iEdge++)
					{
						if(iEdgeFlags & (1<<iEdge))
						{
							iVert0 = a2iTetrahedronEdgeConnection[iEdge][0];
							iVert1 = a2iTetrahedronEdgeConnection[iEdge][1];
							fOffset = pThis->GetOffset(afTetrahedronValue[iVert0], afTetrahedronValue[iVert1], pThis->m_TargetValue);
							fInvOffset = 1.0 - fOffset;
							
							asEdgeVertex[iEdge].x = fInvOffset*asTetrahedronPosition[iVert0].x  +  fOffset*asTetrahedronPosition[iVert1].x;
							asEdgeVertex[iEdge].y = fInvOffset*asTetrahedronPosition[iVert0].y  +  fOffset*asTetrahedronPosition[iVert1].y;
							asEdgeVertex[iEdge].z = fInvOffset*asTetrahedronPosition[iVert0].z  +  fOffset*asTetrahedronPosition[iVert1].z;
						}
					}
					
					for(iTriangle = 0; iTriangle < 2; iTriangle++)
					{
						if(a2iTetrahedronTriangles[iFlagIndex][3*iTriangle] < 0)
							break;

						for(iCorner = 0; iCorner < 3; iCorner++)
						{
							ivertex[iCorner] = a2iTetrahedronTriangles[iFlagIndex][3*iTriangle+iCorner];
							Points[iCorner].x=asEdgeVertex[ivertex[iCorner]].x;
							Points[iCorner].y=asEdgeVertex[ivertex[iCorner]].y;
							Points[iCorner].z=asEdgeVertex[ivertex[iCorner]].z;
						}

						if((Points[0]==Points[1])||(Points[0]==Points[2])||(Points[1]==Points[2]))
							continue;
						
						for(iCorner = 0; iCorner < 3; iCorner++)
						{
							iVertex = ivertex[iCorner];
							m_Point.x=Points[iCorner].x;
							m_Point.y=Points[iCorner].y;
							m_Point.z=Points[iCorner].z;
							int m_key=-1;
							m_key=pThis->CalculatePKey(m_Point);
							m_samePoint=false;
							
							if(pThis->m_PointList[m_key]!=NULL)
							{
								Point3D *p=pThis->m_PointList[m_key];
								while(p!=NULL)
								{
									if((p->x==m_Point.x)&&(p->y==m_Point.y)&&(p->z==m_Point.z))
									{
										m_index=p->index;
										m_samePoint=true;
										break;
									}
									else
										p=p->next;
								}
							}
							
							if(!m_samePoint)
							{
								Point3D *p=new Point3D;
								vertex.x=p->x=m_Point.x;
								vertex.y=p->y=m_Point.y;
								vertex.z=p->z=m_Point.z;
								p->index=m_VertexNumber;
								p->next=pThis->m_PointList[m_key];
								pThis->m_PointList[m_key]=p;
								
								if(iCorner==0)	    m_facet.vertex[0]=m_VertexNumber;
								else
								{
									if(iCorner==1)	m_facet.vertex[1]=m_VertexNumber;
									else		m_facet.vertex[2]=m_VertexNumber;
								}
								if (p->x > x_glMax) {	x_glMax = p->x;	pThis->m_XmaxFacet=m_FacetNumber;}
								if (p->x < x_glMin)		x_glMin = p->x;
								if (p->y > y_glMax)	{   y_glMax = p->y; pThis->m_YmaxFacet=m_FacetNumber;}
								if (p->y < y_glMin)		y_glMin = p->y;
								if (p->z > z_glMax) 	z_glMax = p->z;
								if (p->z < z_glMin) 	z_glMin = p->z;
								
								m_Vertex.push_back(vertex);
								m_Vertex[m_VertexNumber].facet.push_back(m_FacetNumber);
								m_VertexNumber++;
							}
							else
							{
								if(iCorner==0)		m_facet.vertex[0]=m_index;
								else
								{
									if(iCorner==1)	m_facet.vertex[1]=m_index;
									else		m_facet.vertex[2]=m_index;
								}
								m_Vertex[m_index].facet.push_back(m_FacetNumber);
							}
						}
						m_FacetNumber++;
						pThis->CalculateFacetNormal(m_facet);
						m_Facet.push_back(m_facet);
					}
				}
			}
		}
		pThis->m_pFr->m_Progress.SetPos(iZ*100.0f/pThis->m_nFileNum);
	}
	pThis->ClearPointsHash();
	pThis->ExtractLines();
	::SendMessage(pThis->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(pThis->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);	
	pThis->m_pFr->m_pCtrlWnd->EnablControlWhileConstruct();
	str="ģ���ؽ���ɣ�";
	pThis->m_pFr->m_wndStatusBar.SetWindowText(str);
	return 0L;
}


float C3DRCSDoc::GetOffset(float fValue1, float fValue2, float fValueDesired)
{
	double fDelta = fValue2 - fValue1;
	if(fDelta == 0.0)
		return 0.5;
	return (fValueDesired - fValue1)/fDelta;
}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   SortFileArry()
 *
 * ����:
 *   ��                - ����
 *
 * ����ֵ:
 *   void              - ������;
 *
 * ˵��:
 *       �Զ�ȡ��ͼƬ�ļ�·��ϵ�����򡣰��ļ����е����ִ�С����
 * 
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::SortFileArry()
{
	int m_place, n, kkk;
	CString tempt="", tempti="", str;	
	vector<int> flienamenum;   //����������ڴ�ű��

	tempt=m_FilePathArray[0];  //��ʼ��tempt�������ʼ��Ϊ���Ȼ�ȡ��·��

	m_place=tempt.ReverseFind('\\');   //��ȡ�ļ�·���������һ��'\'��λ��

	/*************��ȡ���洢�ļ����а����ı��***************/
	for(int i=0 ; i<m_nFileNum; i++)
	{
		//���ļ�·�������˳�򣬻�ȡ�ļ����а����ı��
		tempti="";
		tempt=m_FilePathArray[i];

		for(int j=m_place; j<tempt.GetLength();j++)
		{
			char ch=tempt.GetAt(j);
			if((ch>='0')&&(ch<='9'))
			{
				str="";
				str.Format("%c",ch);
				tempti+=str;
			}
		}
		n=atoi(tempti);
		flienamenum.push_back(n);   //����Ŵ���������
	}

	/*************����������е�Ԫ�صĴ�С��������***************/
//	int kkk,i,j;
	for(int i=0 ; i<m_nFileNum; i++)
	{
		for(int j=i+1; j<m_nFileNum;j++)
		{
			if(flienamenum[i]>flienamenum[j])
			{
				//�����ļ�·������������Ԫ�ص�λ��
				kkk=flienamenum[i];
				flienamenum[i]=flienamenum[j];
				flienamenum[j]=kkk;

				//ͬʱ��Ӧ�����������������Ԫ�ص�λ��
				tempt=m_FilePathArray[i];
				m_FilePathArray.SetAt(i,m_FilePathArray[j]);
				m_FilePathArray.SetAt(j,tempt);
			}
		}
	}
}

void C3DRCSDoc::SetStartandEnd()
{
	int num[4]={0, 1, m_nFileNum,m_nFileNum+1};

	for(int k=0; k<4;k++)
	{
		for(int i=0 ;i<m_nHeight;i++)
		{
			for(int j=0;j<m_nByteWidth;j++)
			{
				if(j<m_nWidth)
				{
					m_Pixels[num[k]*m_nHeight*m_nWidth+i*m_nWidth+j]=0;
				}
			}
		}
	}

}

UINT C3DRCSDoc::MarchingCubes(LPVOID lp)
{
	TimeCountStart;
	C3DRCSDoc *pThis = (C3DRCSDoc*)lp;   //��ȡ�߳�ָ��
	pThis->m_pFr->m_pCtrlWnd->DisableControlWhileConstruct();
	CString str="���ڽ���ģ���ؽ������Ժ򡭡�";
	pThis->m_pFr->m_wndStatusBar.SetWindowText(str);
//	int i;
	if(!m_Vertex.empty())	
		m_Vertex.clear();
	if(!m_Facet.empty())	
		m_Facet.clear();
	if(!m_Edge.empty())		
		m_Edge.clear();

	m_FacetNumber=m_EdgeNumber=m_VertexNumber=0;

	pThis->m_PointList = new Point3D*[HashMaxLish*2];

	for(int i=0;i<HashMaxLish*2;i++)  
	{
		pThis->m_PointList[i]=NULL;
	}

	pThis->m_XconZoom=pThis->m_pFr->m_pCtrlWnd->m_XconZoom;
	pThis->m_YconZoom=pThis->m_pFr->m_pCtrlWnd->m_YconZoom;
	pThis->m_ZconZoom=pThis->m_pFr->m_pCtrlWnd->m_ZconZoom;

	CVect3 caculatedPosition[12];
	
	GLint index;//����ÿ�����ظ������������ȷ����ű��
	GLint cuts; //Ϊ1��λ��ʾ��λ������ཻ
	//����ÿ������
/*	
	char string[1000];
	int x=pThis->m_nFileNum;
	_gcvt( x, 7, string );
	AfxMessageBox(string);
	x=pThis->m_nHeight;
	_gcvt( x, 7, string );
	AfxMessageBox(string);
	x=pThis->m_nWidth;
	_gcvt( x, 7, string );
	AfxMessageBox(string);
*/

	int SideLength = 1;//������Ԫ�ı߳�
	
	for(int i=0; i< pThis->m_nFileNum+3; i=i+SideLength){
		for(GLint j=0; j< pThis->m_nHeight-1; j=j+SideLength){
			for(GLint k=0; k< pThis->m_nWidth-1; k=k+SideLength){
				//����ÿ�����أ�����ö����Ӧ�ı��
				index = 0;
				if(pThis->Value(i,j,k) >= pThis->m_TargetValue) index |= 1;
				if(pThis->Value(i+SideLength,j,k) >= pThis->m_TargetValue) index |= 2;
				if(pThis->Value(i+SideLength,j,k+SideLength) >= pThis->m_TargetValue) index |= 4;
				if(pThis->Value(i,j,k+SideLength) >= pThis->m_TargetValue) index |= 8;
				if(pThis->Value(i,j+SideLength,k) >= pThis->m_TargetValue) index |= 16;
				if(pThis->Value(i+SideLength,j+SideLength,k) >= pThis->m_TargetValue) index |= 32;
				if(pThis->Value(i+SideLength,j+SideLength,k+SideLength) >= pThis->m_TargetValue) index |= 64;
				if(pThis->Value(i,j+SideLength,k+SideLength) >= pThis->m_TargetValue) index |= 128;

				cuts = edgeTable[index];
				if ( cuts == 0 ) continue; //û�н��㣬��һ������Ԫ��
				if ( cuts & 1 ) {//����0�н���
					caculatedPosition[0] = pThis->interpolate(pThis->Locate(k, j,i),    
					pThis->Value(i,j,k),      pThis->Locate(k,j,i+SideLength),pThis->Value(i+SideLength,j,k));
				}
				if ( cuts & 2 ) {//����1�н���
					caculatedPosition[1] = pThis->interpolate(pThis->Locate(k,j,i+SideLength),     
					pThis->Value(i+SideLength,j,k),    pThis->Locate(k+SideLength,j,i+SideLength),pThis->Value(i+SideLength,j,k+SideLength));
				}
				if ( cuts & 4 ) {//����2�н��� 
					caculatedPosition[2] = pThis->interpolate(pThis->Locate(k+SideLength,j,i+SideLength),   
					pThis->Value(i+SideLength,j,k+SideLength),  pThis->Locate(k+SideLength,j,i),pThis->Value(i,j,k+SideLength));
				}
				if ( cuts & 8 ) {//����3�н���
					caculatedPosition[3] = pThis->interpolate(pThis->Locate(k+SideLength,j,i),     
					pThis->Value(i,j,k+SideLength),    pThis->Locate(k,j,i),pThis->Value(i,j,k));
				}
				if ( cuts & 16 ) {//����4�н���
					caculatedPosition[4] = pThis->interpolate(pThis->Locate(k,j+SideLength,i),     
					pThis->Value(i,j+SideLength,k),    pThis->Locate(k,j+SideLength,i+SideLength),pThis->Value(i+SideLength,j+SideLength,k));
				}
				if ( cuts & 32 ) {//����5�н���
					caculatedPosition[5] = pThis->interpolate(pThis->Locate(k,j+SideLength,i+SideLength),   
					pThis->Value(i+SideLength,j+SideLength,k),  pThis->Locate(k+SideLength,j+SideLength,i+SideLength),pThis->Value(i+SideLength,j+SideLength,k+SideLength));
				}
				if ( cuts & 64 ) {//����6�н���
					caculatedPosition[6] = pThis->interpolate(pThis->Locate(k+SideLength,j+SideLength,i+SideLength), 
					pThis->Value(i+SideLength,j+SideLength,k+SideLength),pThis->Locate(k+SideLength,j+SideLength,i),pThis->Value(i,j+SideLength,k+SideLength));
				}
				if ( cuts & 128 ) {//����7�н���
					caculatedPosition[7] = pThis->interpolate(pThis->Locate(k+SideLength,j+SideLength,i),   
					pThis->Value(i,j+SideLength,k+SideLength),  pThis->Locate(k,j+SideLength,i),pThis->Value(i,j+SideLength,k));
				}
				if ( cuts & 256 ) {//����8�н���
					caculatedPosition[8] = pThis->interpolate(pThis->Locate(k,j,i),       
					pThis->Value(i,j,k),      pThis->Locate(k,j+SideLength,i),pThis->Value(i,j+SideLength,k));
				}
				if ( cuts & 512 ) {//����9�н���
					caculatedPosition[9] = pThis->interpolate(pThis->Locate(k,j,i+SideLength),     
					pThis->Value(i+SideLength,j,k),    pThis->Locate(k,j+SideLength,i+SideLength),pThis->Value(i+SideLength,j+SideLength,k));
				}
				if ( cuts & 1024 ) {//����10�н���
					caculatedPosition[10] = pThis->interpolate(pThis->Locate(k+SideLength,j,i+SideLength),  
					pThis->Value(i+SideLength,j,k+SideLength),  pThis->Locate(k+SideLength,j+SideLength,i+SideLength),pThis->Value(i+SideLength,j+SideLength,k+SideLength));
				}
				if ( cuts & 2048 ) {//����11�н���
					caculatedPosition[11] = pThis->interpolate(pThis->Locate(k+SideLength,j,i),    
					pThis->Value(i,j,k+SideLength),    pThis->Locate(k+SideLength,j+SideLength,i),pThis->Value(i,j+SideLength,k+SideLength));
				}
				//�Ѿ��õ��˱�������Ľ��㣬������������α���
				for(GLint m=0; triTable[index][m]!= -1; m+=3) {
					pThis->ExtractTriangle(caculatedPosition[ triTable[index][m] ],caculatedPosition[ triTable[index][m+1] ],caculatedPosition[ triTable[index][m+2] ]);
				}
			}//for(k
		}//for(j
		pThis->m_pFr->m_Progress.SetPos(i*100.0f/pThis->m_nFileNum);
	}//for(i

	pThis->ClearPointsHash();
	pThis->ExtractLines();
	::SendMessage(pThis->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_ENABLEBTN,0,0);
	::SendMessage(pThis->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);
	pThis->m_pFr->m_pCtrlWnd->EnablControlWhileConstruct();
	str="ģ���ؽ���ɣ�";
	TimeCountEnd;
	pThis->m_pFr->m_wndStatusBar.SetWindowText(str);
	return 0L;

}

CVect3 C3DRCSDoc::interpolate(CVect3 X1, int f1, CVect3 X2, int f2)
{
	CVect3 ret;
	ret.x = X1.x + (X2.x-X1.x)*(m_TargetValue-f1)/(f2-f1);
	ret.y = X1.y + (X2.y-X1.y)*(m_TargetValue-f1)/(f2-f1);
	ret.z = X1.z + (X2.z-X1.z)*(m_TargetValue-f1)/(f2-f1);
	return ret;

}

int C3DRCSDoc::Value(int i, int j, int k)
{
	return this->m_Pixels[i*m_nHeight*m_nWidth+m_nWidth*j+k];
}

void C3DRCSDoc::ExtractTriangle(CVect3 a, CVect3 b, CVect3 c)  ///MarchingCubes����
{
	if((a==b)||(a==c)||(b==c))
		return;
	Point3D m_Point;
	Vertex vertex;
	Facet m_facet;
	bool m_samePoint = false; 
	int  m_index=-1;
	CVect3 V[3]={c,b,a};

	for(int iCorner = 0; iCorner < 3; iCorner++)
	{
		m_Point.x=V[iCorner].x;
		m_Point.y=V[iCorner].y;
		m_Point.z=V[iCorner].z;
		int m_key=-1;
		m_key=this->CalculatePKey(m_Point);
		m_samePoint=false;

		if(this->m_PointList[m_key]!=NULL)
		{
			Point3D *p=this->m_PointList[m_key];
			while(p!=NULL)
			{
				if((p->x==m_Point.x)&&(p->y==m_Point.y)&&(p->z==m_Point.z))
				{
					m_index=p->index;
					m_samePoint=true;
					break;
				}
				else
					p=p->next;
			}
		}

		if(!m_samePoint)
		{
			Point3D *p=new Point3D;
			vertex.x=p->x=m_Point.x;
			vertex.y=p->y=m_Point.y;
			vertex.z=p->z=m_Point.z;
			p->index=m_VertexNumber;
			p->next=this->m_PointList[m_key];
			this->m_PointList[m_key]=p;

			if(iCorner==0)	    m_facet.vertex[0]=m_VertexNumber;
			else
			{
				if(iCorner==1)	m_facet.vertex[1]=m_VertexNumber;
				else		m_facet.vertex[2]=m_VertexNumber;
			}
			if (p->x > x_glMax) {	x_glMax = p->x;	this->m_XmaxFacet=m_FacetNumber;}
			if (p->x < x_glMin)		x_glMin = p->x;
			if (p->y > y_glMax)	{   y_glMax = p->y; this->m_YmaxFacet=m_FacetNumber;}
			if (p->y < y_glMin)		y_glMin = p->y;
			if (p->z > z_glMax) 	z_glMax = p->z;
			if (p->z < z_glMin) 	z_glMin = p->z;

			m_Vertex.push_back(vertex);
			m_Vertex[m_VertexNumber].facet.push_back(m_FacetNumber);
			m_VertexNumber++;
		}
		else
		{
			if(iCorner==0)		m_facet.vertex[0]=m_index;
			else
			{
				if(iCorner==1)	m_facet.vertex[1]=m_index;
				else		m_facet.vertex[2]=m_index;
			}
			m_Vertex[m_index].facet.push_back(m_FacetNumber);
		}
	}
	m_FacetNumber++;
	this->CalculateFacetNormal(m_facet);
	m_Facet.push_back(m_facet);
}

CVect3 C3DRCSDoc::Locate(int x, int y, int z)
{
	return CVect3(x*this->m_XconZoom, y*this->m_YconZoom, z*this->m_ZconZoom);
}


void C3DRCSDoc::SmoothMesh()
{
	m_SmoothRate = this->m_pFr->m_msmoothrate;
	m_SmoothNum = this->m_pFr->m_msmoothnum;

	CVect3 Lei,Pos;
	bool m_undo=false;
	Vertex m_bV;
	vector<Facet> m_bF;
	long mF;
	double area=0.0;
	int m_facetnum;
	CVect3 mFVect;
	int i,nn,k,j;
	//*******************�ƶ�����*******************//  Vi'=Vi+u*Lei;

	long m_Vnum=m_Vertex.size();

	for(nn=0; nn<m_SmoothNum; nn++)
	{
		for(i=0; i<m_Vnum; i++)
		{
			if(i%1024==0)
				this->m_pFr->m_Progress.SetPos((i+1)*100/m_Vnum);
			
			if(m_Vertex[i].bBoundary||!m_Vertex[i].flag)  //���Ϊ�桢�ұ�Ϊ�٣�����һ����ֹͣ
				continue;

			m_facetnum=m_Vertex[i].facet.size();//ĳ����������Ƭ�ĸ���
			if(m_facetnum<3)
				continue;

			m_bV=m_Vertex[i];

			//����Ԥ��
			Pos=CalculatePosWithSOT(m_Vertex[i], i);

			m_Vertex[i].x+=m_SmoothRate*(Pos.x-m_Vertex[i].x);
			m_Vertex[i].y+=m_SmoothRate*(Pos.y-m_Vertex[i].y);
			m_Vertex[i].z+=m_SmoothRate*(Pos.z-m_Vertex[i].z);

			Lei=CalculatMoveVect(m_Vertex[i], i);
			m_Vertex[i].x+=Lei.x*m_SmoothRate*1.5;//(1.0-m_SmoothRate);
			m_Vertex[i].y+=Lei.y*m_SmoothRate*1.5;//(1.0-m_SmoothRate);
			m_Vertex[i].z+=Lei.z*m_SmoothRate*1.5;//(1.0-m_SmoothRate);
			
			
			//*******************������һ���������εķ��򡢱߳������*******************//
			m_undo=false;
			m_bF.clear();

			if(((m_bV.x-m_Vertex[i].x)*(m_bV.x-m_Vertex[i].x)+(m_bV.y-m_Vertex[i].y)*(m_bV.y-m_Vertex[i].y)+(m_bV.z-m_Vertex[i].z)*(m_bV.z-m_Vertex[i].z))>0.25)
				m_undo=true;
			else
			{
				
				m_Vertex[i].nx=m_Vertex[i].ny=m_Vertex[i].nz=0.0;
				
				for(j=0; j<m_facetnum; j++)
				{
					mF=m_Vertex[i].facet[j];
					
					m_bF.push_back(m_Facet[mF]);
					
					mFVect.x=m_Facet[mF].nx; mFVect.y=m_Facet[mF].ny; mFVect.z=m_Facet[mF].nz; 
					
					CalculateFacetNormal(m_Facet[mF]);
					if((mFVect.x*m_Facet[mF].nx+mFVect.y*m_Facet[mF].ny+mFVect.z*m_Facet[mF].nz)<=0.0)
					{   m_undo=true; break;		}
					
					if((mFVect.x*m_Facet[mF].x+mFVect.y*m_Facet[mF].y+mFVect.z*m_Facet[mF].z)>=0.01)
					{   m_undo=true; break;		}
					
					for(k=0; k<3; k++)
					{
						if(m_Facet[mF].vertex[k]==i)
							break;
					}
					m_Edge[m_Facet[mF].edge[k]].length=CalculateLength(m_Facet[mF].vertex[k],m_Facet[mF].vertex[(k+1)%3]);
					if(m_Edge[m_Facet[mF].edge[k]].length<=0.0)
					{	m_undo=true; break; 	}
					
					if(m_undo)
						break;
					
					m_Facet[mF].area=CalculateArea(m_Facet[mF].edge[0],m_Facet[mF].edge[1],m_Facet[mF].edge[2]);
					
					if(m_Facet[mF].area<=0.0)
					{   m_undo=true; break;		}
					
				}
			}

			//********��������ƶ���ģ�ͱ���̫����ع�*********//
			if(m_undo)//////////////////////////////////////////////////////////////////////////�ع�����
			{
				m_Vertex[i]=m_bV;
				for(j=0; j<m_bF.size(); j++)
				{
					mF=m_Vertex[i].facet[j];
					m_Facet[mF]=m_bF[j];
					for(k=0; k<3; k++)
						if(m_Facet[mF].vertex[k]==i)
							break;
					m_Edge[m_Facet[mF].edge[k]].length=CalculateLength(m_Facet[mF].vertex[k],m_Facet[mF].vertex[(k+1)%3]);
				}
				
				continue;
			}

			CalculateVertexNormal(i);
			
			
			//*******************������һ���򶥵㷨��*******************//
			
			//���¼���߽绷�ϸ����˵�ķ�����
			for(j=0; j<m_facetnum; j++)
			{
				for(k=0; k<3; k++)
					if(m_Facet[m_Vertex[i].facet[j]].vertex[k]==i)
						break;
					
				//���whichPС��0������ڵ���3���򱨾���
				CalculateVertexNormal(m_Facet[m_Vertex[i].facet[j]].vertex[(k+1)%3]);
			}
		}
	}
	this->m_pFr->m_Progress.SetPos(0);
}

CVect3 C3DRCSDoc::CalculatePosWithSOT(Vertex vertex, long i)
{
	CVect3 Position(0.0,0.0,0.0);
	vector<long> SOTfacetNum;
	long num;
	bool sameFacet;

	int m_facetnum=vertex.facet.size();
	int j,k,m,nn;

	//��¼����SOT(second order triargle,SOT)������Ƭ�����
	for(j=0; j<m_facetnum; j++)
	{
		for(k=0; k<3; k++)
		{
			int km=m_Facet[vertex.facet[j]].vertex[k];
			if(km!=i)   //jƬ��˵�k��Ϊ�õ�
			{
				int mfac=m_Vertex[km].facet.size(); //�ñ߽绷��ΪjƬ��˵�k��������Ƭ����
				for(m=0; m<mfac; m++)
				{
					num=m_Vertex[km].facet[m];
					sameFacet=false;
					for(nn=0; nn<SOTfacetNum.size(); nn++)
					{
						if(num==SOTfacetNum[nn])
						{
							sameFacet=true;
							break;
						}
					}
					if(!sameFacet)
						SOTfacetNum.push_back(num);
				}
			}
		}
	}

	CVect3 facetNormal, Io, FacetCentor;
	float d1, d2, Gauss1, Gauss2, Gauss=0.0f, u1=3.0f, u2=2.0f;
	for( j=0; j<SOTfacetNum.size(); j++)
	{
		num=SOTfacetNum[j];
		
		//������Ƭ����
		FacetCentor.x=(m_Vertex[m_Facet[num].vertex[0]].x+m_Vertex[m_Facet[num].vertex[1]].x+m_Vertex[m_Facet[num].vertex[2]].x)/3.0;
		FacetCentor.y=(m_Vertex[m_Facet[num].vertex[0]].y+m_Vertex[m_Facet[num].vertex[1]].y+m_Vertex[m_Facet[num].vertex[2]].y)/3.0;
		FacetCentor.z=(m_Vertex[m_Facet[num].vertex[0]].z+m_Vertex[m_Facet[num].vertex[1]].z+m_Vertex[m_Facet[num].vertex[2]].z)/3.0;

		Io=CalculateProjectPoint(vertex, num);

		d1=sqrt((FacetCentor.x-vertex.x)*(FacetCentor.x-vertex.x)+(FacetCentor.y-vertex.y)*(FacetCentor.y-vertex.y)+(FacetCentor.z-vertex.z)*(FacetCentor.z-vertex.z));
		d2=sqrt((Io.x-vertex.x)*(Io.x-vertex.x)+(Io.y-vertex.y)*(Io.y-vertex.y)+(Io.z-vertex.z)*(Io.z-vertex.z));
		Gauss1=exp(0.0-d1*d1/(u1*u1));
		Gauss2=exp(0.0-d2*d2/(u2*u2));

		Position=Position+Io*Gauss1*Gauss2;

		Gauss+=Gauss1*Gauss2;
	}

	Position=Position/Gauss;

	return Position;

}

//Lei=ki*Ni+Lti;
//ki*Ni=0.25* &{[(cot(aj)+cot(bj)]*[Vj-Vi]} /Area
CVect3 C3DRCSDoc::CalculatMoveVect(Vertex vertex, long i)  //CalculateMoveVect����
{
	CVect3 Lei(0.0,0.0,0.0), v(0.0,0.0,0.0);
	CVect3 Lti(0.0,0.0,0.0), Li(0.0,0.0,0.0), N(vertex.nx,vertex.ny,vertex.nz);
	CVect3 kiNi(0.0,0.0,0.0);
	Vertex vertexj;
	int kkv=0,kke; //0<=kkk<=2;
	long facet,rfacet, vj, ve;
	double Area=0.0;
	double l[3], E=0.0;
	double aj=0.0, bj=0.0, ki=0.0;
	
	
	int m_facetnum=vertex.facet.size();

	for(int j=0; j<m_facetnum; j++)
	{
		if(m_Facet[vertex.facet[j]].vertex[0]==i)
			kkv=0;
		if(m_Facet[vertex.facet[j]].vertex[1]==i)
			kkv=1;
		if(m_Facet[vertex.facet[j]].vertex[2]==i)
			kkv=2;

		ve=m_Facet[vertex.facet[j]].edge[kkv];

		facet=m_Edge[ve].facet;
		rfacet=m_Edge[ve].rfacet;
		vj=m_Facet[vertex.facet[j]].vertex[(kkv+1)%3];
		vertexj=m_Vertex[vj];
		v.x+=vertexj.x;
		v.y+=vertexj.y;
		v.z+=vertexj.z;

		if(m_Facet[facet].edge[0]==ve)
			kke=0;
		if(m_Facet[facet].edge[1]==ve)
			kke=1;
		if(m_Facet[facet].edge[2]==ve)
			kke=2;

		//a=(kke+2)%3;
		l[0]=m_Edge[m_Facet[facet].edge[0]].length;
		l[1]=m_Edge[m_Facet[facet].edge[1]].length;
		l[2]=m_Edge[m_Facet[facet].edge[2]].length;

		aj=acos((l[(kke+2)%3]*l[(kke+2)%3]+l[(kke+1)%3]*l[(kke+1)%3]-l[kke]*l[kke])/(2.0*l[(kke+2)%3]*l[(kke+1)%3]));

		if(m_Facet[rfacet].edge[0]==ve)
			kke=0;
		if(m_Facet[rfacet].edge[1]==ve)
			kke=1;
		if(m_Facet[rfacet].edge[2]==ve)
			kke=2;

		//b=(kke+2)%3;
		l[0]=m_Edge[m_Facet[rfacet].edge[0]].length;
		l[1]=m_Edge[m_Facet[rfacet].edge[1]].length;
		l[2]=m_Edge[m_Facet[rfacet].edge[2]].length;

		bj=acos((l[(kke+2)%3]*l[(kke+2)%3]+l[(kke+1)%3]*l[(kke+1)%3]-l[kke]*l[kke])/(2.0*l[(kke+2)%3]*l[(kke+1)%3]));

		ki=(1.0/tan(aj)+1.0/tan(bj));
		kiNi.x+=ki*(vertexj.x-vertex.x);
		kiNi.y+=ki*(vertexj.y-vertex.y);
		kiNi.z+=ki*(vertexj.z-vertex.z);

		Area+=m_Facet[vertex.facet[j]].area;
	}
	v.x/=m_facetnum;
	v.y/=m_facetnum;
	v.z/=m_facetnum;

	//Laplacian ����
	Li.x=v.x-vertex.x;
	Li.y=v.y-vertex.y;
	Li.z=v.z-vertex.z;

	double ln=Li.x*N.x+Li.y*N.y+Li.z*N.z;

	Lti.x=Li.x-ln*N.x;
	Lti.y=Li.y-ln*N.y;
	Lti.z=Li.z-ln*N.z;

	kiNi=kiNi/(4.0*Area);
	Lei=kiNi+Lti;

	return Lei;
}

void C3DRCSDoc::CalculatModeVolume()
{
	m_ModeVolume=0.0;
	CVect3 MA, Mid(x_glMid, y_glMid, z_glMid);
	double Dis=0.0;

	for(int i=0; i<m_Facet.size();i++)
	{
		if(false==m_Facet[i].flag)
			continue;

		MA.x=m_Vertex[m_Facet[i].vertex[0]].x-Mid.x;
		MA.y=m_Vertex[m_Facet[i].vertex[0]].y-Mid.y;
		MA.z=m_Vertex[m_Facet[i].vertex[0]].z-Mid.z;

		Dis=MA.x*m_Facet[i].nx+MA.y*m_Facet[i].ny+MA.z*m_Facet[i].nz;

		m_ModeVolume+=m_Facet[i].area*Dis;
	}

	m_ModeVolume/=6.0;
	m_ModeVolume/=1000.0;
	_Volume=m_ModeVolume;

}

void C3DRCSDoc::CalculateSurfaceArea()
{
	m_ModeSurfaceArea=0.0;
	for(int i=0; i<m_Facet.size();i++)
	{
		if(false==m_Facet[i].flag)
			continue;
		m_ModeSurfaceArea+=m_Facet[i].area;
	}
	m_ModeSurfaceArea/=100.0;
	_Area=m_ModeSurfaceArea;
}


void C3DRCSDoc::CalculateVertexNormal(long i)  //�������Ȩֵ�����ķ���
{
	int m_facetnum=m_Vertex[i].facet.size();
	float area=0.0, S=0.0, l[3];
	vector<float> CosV;
	int kkv;
	
	CosV.clear();
	CosV.resize(m_facetnum);
	m_Vertex[i].nx=m_Vertex[i].ny=m_Vertex[i].nz=0.0;

	float tt;
	int j;
	for(j=0; j<m_facetnum; j++)
	{

		if(m_Facet[m_Vertex[i].facet[j]].vertex[0]==i)
			kkv=0;
		if(m_Facet[m_Vertex[i].facet[j]].vertex[1]==i)
			kkv=1;
		if(m_Facet[m_Vertex[i].facet[j]].vertex[2]==i)
			kkv=2;
		
		l[0]=m_Edge[m_Facet[m_Vertex[i].facet[j]].edge[0]].length;
		l[1]=m_Edge[m_Facet[m_Vertex[i].facet[j]].edge[1]].length;
		l[2]=m_Edge[m_Facet[m_Vertex[i].facet[j]].edge[2]].length;
		//��i�ĵ�j��������������߳���

		if((l[0]+l[1]<=l[2])||(l[1]+l[2]<=l[0])||(l[2]+l[0]<=l[1]))
		{
			continue;
		}
			
		CosV[j]=acos((l[(kkv+2)%3]*l[(kkv+2)%3]+l[kkv]*l[kkv]-l[(kkv+1)%3]*l[(kkv+1)%3])/(2.0*l[(kkv+2)%3]*l[kkv]));
		//��i�ĵ�j��������ı߼н�
		//���CosV[j]С��0������ڵ���Pi���򱨾���
		
	}

	for( j=0; j<m_facetnum; j++)
		S+=CosV[j];
	
	for( j=0; j<m_facetnum; j++)
	{
		CosV[j]/=S;
		m_Vertex[i].nx +=m_Facet[m_Vertex[i].facet[j]].nx*m_Facet[m_Vertex[i].facet[j]].area*CosV[j];
		m_Vertex[i].ny +=m_Facet[m_Vertex[i].facet[j]].ny*m_Facet[m_Vertex[i].facet[j]].area*CosV[j];
		m_Vertex[i].nz +=m_Facet[m_Vertex[i].facet[j]].nz*m_Facet[m_Vertex[i].facet[j]].area*CosV[j];
		area+=m_Facet[m_Vertex[i].facet[j]].area*CosV[j];
	}

	m_Vertex[i].nx /=area;
	m_Vertex[i].ny /=area;
	m_Vertex[i].nz /=area;

	tt=sqrt(m_Vertex[i].nx*m_Vertex[i].nx+m_Vertex[i].ny*m_Vertex[i].ny+m_Vertex[i].nz*m_Vertex[i].nz);

	m_Vertex[i].nx /=tt;
	m_Vertex[i].ny /=tt;
	m_Vertex[i].nz /=tt;


}

CVect3 C3DRCSDoc::CalculateProjectPoint(Vertex vertex, long FacetNum)
{
	CVect3 V, I1, Io;
	float x0, y0, z0, k, a, b, c;

	//ƽ��ax+by+cz+d=0����nx(x-x0)+ny(y-y0)+nz(z-z0)=0, ȷ��a,b,c,d
	//a=nx, b=ny, c=nz, d=-nx*x0-ny*y0-nz*z0;

	a=m_Facet[FacetNum].nx;		        b=m_Facet[FacetNum].ny;		        c=m_Facet[FacetNum].nz;

	x0=m_Vertex[m_Facet[FacetNum].vertex[0]].x;    
	y0=m_Vertex[m_Facet[FacetNum].vertex[0]].y; 
	z0=m_Vertex[m_Facet[FacetNum].vertex[0]].z;

	//��֪һ��ƽ�棬����һ��A��ƽ�淨��N��ƽ����һ��I����I����ƽ���ϵ�ͶӰ�� I,��vertex��
	//I�ڷ���N�ϵ�ͶӰ��I1,     I1 = A + k * N ;     V = I �C A;     k = V.x * N.x + V.y * N.y + V.z * N.z;
	//��I��ƽ���ϵ�ͶӰ����Io����Io = A + I1I��
	V.x = vertex.x-x0;     V.y = vertex.y-y0;     V.z = vertex.z-z0;
	k=V.x * a + V.y * b + V.z * c;
	I1.x= x0 + k*a;     I1.y= y0 + k*b;     I1.z= z0 + k*c;  
	Io.x= x0 +(vertex.x-I1.x);    Io.y= y0 +(vertex.y-I1.y);    Io.z= z0 +(vertex.z-I1.z);

	return Io;

}

long C3DRCSDoc::JudgeFacetsInLoop(long i, long f0, long f1, long f2)
{
	long F[3]={f0, f1, f2};
	vector<long> PointN;
	int j;
	for(j=0; j<3; j++)
	{
		for(int k=0; k<3; k++)  //��m_Facet[F[j]]��m_Facet[i]����ȫ�����������Σ���m_Facet[F[j]]�Ķ��㶼����
		{
			if((m_Facet[F[j]].vertex[k]!=m_Facet[i].vertex[0])&&(m_Facet[F[j]].vertex[k]!=m_Facet[i].vertex[1])&&(m_Facet[F[j]].vertex[k]!=m_Facet[i].vertex[2]))
			{
				PointN.push_back(m_Facet[F[j]].vertex[k]);
			}
		}
	}
	
	for(j=0; j<PointN.size(); j++)
	{
		for(int k=j+1; k<PointN.size(); k++)
		{
			if(PointN[j]==PointN[k])
			{
				return PointN[j];
			}
		}
	}


	return -1;

}


/*            
             /|\
            / | \
           /�� ��\
		  �ϣ�_�ߣ�
*/
bool C3DRCSDoc::DeleteFacetInLoop(long sameVertex, long i, long f0, long f1, long f2)
{
	long F[3]={f0, f1, f2};
	vector<long> Facet2;
	vector<long> Edge2; //�����ߵ�����
	long samepoint=0;   //i,  Facet2[0], Facet2[1] ���������εĹ�ͬ���㣬 �������м�㡣
	float x0, x1, x2, y0, y1, y2, z0, z1, z2 ;

	float value=0.0f;
	int j,k;
	for(j=0; j<3; j++)
	{
		if((m_Facet[F[j]].vertex[0]==sameVertex)||(m_Facet[F[j]].vertex[1]==sameVertex)||(m_Facet[F[j]].vertex[2]==sameVertex))
		{
			Facet2.push_back(F[j]);
		}	
	}

	j=Facet2.size();
	if(Facet2.size()!=2)     //ֻ����һ�����������������ε����
		return false;

	x0=m_Facet[i].nx; y0=m_Facet[i].ny; z0=m_Facet[i].nz;
	x1=m_Facet[Facet2[0]].nx; y1=m_Facet[Facet2[0]].ny; z1=m_Facet[Facet2[0]].nz;
	x2=m_Facet[Facet2[1]].nx; y2=m_Facet[Facet2[1]].ny; z2=m_Facet[Facet2[1]].nz;
	
	value=1.0-(x0*x1+y0*y1+z0*z1+x1*x2+y1*y2+z1*z2+x0*x2+y0*y2+z0*z2)/3.0; //ƽ��
	if(value<0.0f)
		value=0.0f;

	value=acos(value)*3.14159/180.0;
	
	if(value>m_ReduceAngleThreshold*2.0)
		return false;

	int ivertex=-1;
	for(k=0; k<3; k++)
	{
		if(((m_Facet[i].vertex[k]==m_Facet[Facet2[0]].vertex[0])||(m_Facet[i].vertex[k]==m_Facet[Facet2[0]].vertex[1])||(m_Facet[i].vertex[k]==m_Facet[Facet2[0]].vertex[2]))&&\
			((m_Facet[i].vertex[k]==m_Facet[Facet2[1]].vertex[0])||(m_Facet[i].vertex[k]==m_Facet[Facet2[1]].vertex[1])||(m_Facet[i].vertex[k]==m_Facet[Facet2[1]].vertex[2])))
		{
			samepoint=m_Facet[i].vertex[k];
			ivertex=k;
			break;
		}
	}   //ȷ�����ĵ�

	if(ivertex<0)
		return false;

	for(j=0; j<3; j++)
	{
		if(m_Facet[Facet2[0]].vertex[j]==samepoint)
		{
			Edge2.push_back(m_Facet[Facet2[0]].edge[(j+1)%3]);
			break;
		}
	}     //�洢�⻷�ϵĵ�һ����

	for(j=0; j<3; j++)
	{
		if(m_Facet[Facet2[1]].vertex[j]==samepoint)
		{
			Edge2.push_back(m_Facet[Facet2[1]].edge[(j+1)%3]);
			break;
		}
	}        //�洢�⻷�ϵĵڶ�����

	if(Edge2.size()!=2)
		return false;
	if(((m_Edge[Edge2[0]].vertex1!=sameVertex)&&(m_Edge[Edge2[0]].vertex2!=sameVertex))||\
		((m_Edge[Edge2[1]].vertex1!=sameVertex)&&(m_Edge[Edge2[1]].vertex2!=sameVertex)))
		return false;                    //ȷ���洢��ȷ

	m_Facet[i].vertex[ivertex]=sameVertex;
	CalculateFacetNormal(m_Facet[i]);

	
	if((m_Facet[Facet2[0]].edge[0]==Edge2[1])||(m_Facet[Facet2[0]].edge[1]==Edge2[1])||(m_Facet[Facet2[0]].edge[2]==Edge2[1]))
	{
		long tempt=Edge2[1];
		Edge2[1]=Edge2[0];
		Edge2[0]=tempt;
	}       //��֤Edge2������Ԫ�غ�Facet2������Ԫ�طֱ��Ӧ

	int ff0=-1, ff1=-1;

	for(k=0; k<3; k++)
	{
		if(m_Facet[i].aFacet[k]==Facet2[0])
			ff0=k;
		if(m_Facet[i].aFacet[k]==Facet2[1])
			ff1=k;
	}

	if(m_Edge[Edge2[0]].facet==Facet2[0])
	{
		m_Edge[Edge2[0]].facet=i;
		m_Facet[i].aFacet[ff0]=m_Edge[Edge2[0]].rfacet;
		for( k=0; k<3; k++)
		{
			if(m_Facet[m_Edge[Edge2[0]].rfacet].aFacet[k]==Facet2[0])
				m_Facet[m_Edge[Edge2[0]].rfacet].aFacet[k]=i;
		}
		m_Facet[i].edge[ff0]=Edge2[0];
	}
	else
	{
		m_Edge[Edge2[0]].rfacet=i;
		m_Facet[i].aFacet[ff0]=m_Edge[Edge2[0]].facet;
		for( k=0; k<3; k++)
		{
			if(m_Facet[m_Edge[Edge2[0]].facet].aFacet[k]==Facet2[0])
				m_Facet[m_Edge[Edge2[0]].facet].aFacet[k]=i;
		}

		m_Facet[i].edge[ff0]=Edge2[0];
	}


	if(m_Edge[Edge2[1]].facet==Facet2[1])
	{
		m_Edge[Edge2[1]].facet=i;
		m_Facet[i].aFacet[ff1]=m_Edge[Edge2[1]].rfacet;
		for( k=0; k<3; k++)
		{
			if(m_Facet[m_Edge[Edge2[1]].rfacet].aFacet[k]==Facet2[1])
				m_Facet[m_Edge[Edge2[1]].rfacet].aFacet[k]=i;
		}

		m_Facet[i].edge[ff1]=Edge2[1];
	}
	else
	{
		m_Edge[Edge2[1]].rfacet=i;
		m_Facet[i].aFacet[ff1]=m_Edge[Edge2[1]].facet;
		for( k=0; k<3; k++)
		{
			if(m_Facet[m_Edge[Edge2[1]].facet].aFacet[k]==Facet2[1])
				m_Facet[m_Edge[Edge2[1]].facet].aFacet[k]=i;
		}

		m_Facet[i].edge[ff1]=Edge2[1];
	}

	for( j=0; j<3; j++)
	{
		for(int k=0; k<m_Vertex[m_Facet[i].vertex[j]].facet.size();k++)
		{
			if((m_Vertex[m_Facet[i].vertex[j]].facet[k]==Facet2[0])||(m_Vertex[m_Facet[i].vertex[j]].facet[k]==Facet2[1]))
			{
				vector<int>::iterator itef = m_Vertex[m_Facet[i].vertex[j]].facet.begin()+k;
				m_Vertex[m_Facet[i].vertex[j]].facet.erase(itef);
			}
		}
	}

	for( k=0; k<m_Vertex[sameVertex].facet.size();k++)
	{
		if((m_Vertex[sameVertex].facet[k]==Facet2[0])||(m_Vertex[sameVertex].facet[k]==Facet2[1]))
		{
			vector<int>::iterator itef = m_Vertex[sameVertex].facet.begin()+k;
			m_Vertex[sameVertex].facet.erase(itef);
		}
	}
	m_Vertex[sameVertex].facet.push_back(i);

	for( k=0; k<3; k++)
	{
		if(m_Facet[Facet2[0]].vertex[k]==samepoint)
		{
			m_Edge[m_Facet[Facet2[0]].edge[k]].flag=false;
			m_Edge[m_Facet[Facet2[0]].edge[(k+2)%3]].flag=false;
		}

		if(m_Facet[Facet2[1]].vertex[k]==samepoint)
		{
			m_Edge[m_Facet[Facet2[1]].edge[k]].flag=false;
			m_Edge[m_Facet[Facet2[1]].edge[(k+2)%3]].flag=false;
		}
	}
	
	for( k=0; k<2; k++)
	{
		if(m_Facet[Facet2[k]].flag)
		{
			m_Facet[Facet2[k]].flag=false;
			m_ReduceNum++;
		}
		
		long order = m_Facet[Facet2[k]].order;
		
		if((order!=-1)&&(m_FacetList[order]!=NULL))
		{
			Face *p=m_FacetList[order],*q;
			if(p->index==Facet2[k])
			{
				m_FacetList[order]=p->next;
				m_Facet[Facet2[k]].order=-1;
				delete p;
			}
			else
			{
				q=p->next;
				while(q!=NULL)
				{
					if(q->index==Facet2[k])
					{
						p->next=q->next;delete q; 
						m_Facet[Facet2[k]].order=-1;
						break;
					}
					else
					{
						p=q; q=q->next;
					}
				}
			}
		}
	}	
	
	m_Vertex[samepoint].flag=false;
	
	m_FacetNumber-=2;
	m_VertexNumber-=1;
	m_EdgeNumber-=3;
	
	for(j=0; j<3; j++)
	{
		CalculateVertexNormal(m_Facet[i].vertex[j]);
	}
				
	CalculateFacetValue(i);
	
	long order = m_Facet[i].order;
				
	if((order!=-1)&&(m_FacetList[order]!=NULL))
	{
		Face *p=m_FacetList[order],*q;
		if(p->index==i)
		{
			m_FacetList[order]=p->next;
			m_Facet[i].order=-1;
			delete p;
		}
		else
		{
			q=p->next;
			while(q!=NULL)
			{
				if(q->index==i)
				{
					p->next=q->next;delete q; 
					m_Facet[i].order=-1;
					break;
				}
				else
				{
					p=q; q=q->next;
				}
			}
		}
	}
				
	long m_key=long (m_Facet[i].value*100000.0f);
	if(m_key>HashMaxLish*4)
		m_key=HashMaxLish*4;
	Face *f,*p0;
	f= new Face;
	p0=m_FacetList[m_key];
	m_FacetList[m_key]=f;
	f->next=p0;
	f->index=i;
	m_Facet[i].order=m_key;
	return true;

}

long C3DRCSDoc::JudgeFacetsNearLoop(long i, long f0, long f1, long f2)
{
	long ff[3], m_b[3]={-1, -1, -1};

	ff[0]=m_Facet[f0].aFacet[0]; 
	ff[1]=m_Facet[f0].aFacet[1];
	ff[2]=m_Facet[f0].aFacet[2];
	m_b[0]=JudgeFacetsInLoop(f0, ff[0], ff[1], ff[2]);

	if(m_b[0]>=0)
			return f0;

	ff[0]=m_Facet[f1].aFacet[0];
	ff[1]=m_Facet[f1].aFacet[1];
	ff[2]=m_Facet[f1].aFacet[2];
	m_b[1]=JudgeFacetsInLoop(f1, ff[0], ff[1], ff[2]);

	if(m_b[1]>=0)
			return f1;

	ff[0]=m_Facet[f2].aFacet[0];
	ff[1]=m_Facet[f2].aFacet[1];
	ff[2]=m_Facet[f2].aFacet[2];
	m_b[2]=JudgeFacetsInLoop(f2, ff[0], ff[1], ff[2]);
	if(m_b[2]>=0)
			return f2;

	return -1L;
}


float C3DRCSDoc::CalculateDistanceFormVtoF(Vertex V, long F)
{
	float x0, y0, z0, k, a, b, c;

	//ƽ��ax+by+cz+d=0����nx(x-x0)+ny(y-y0)+nz(z-z0)=0, ȷ��a,b,c,d
	//a=nx, b=ny, c=nz, d=-nx*x0-ny*y0-nz*z0;

	a=m_Facet[F].nx;		        b=m_Facet[F].ny;		        c=m_Facet[F].nz;

	x0=m_Vertex[m_Facet[F].vertex[0]].x;    
	y0=m_Vertex[m_Facet[F].vertex[0]].y; 
	z0=m_Vertex[m_Facet[F].vertex[0]].z;

	k=(V.x-x0) * a + (V.y-y0) * b + (V.z-z0) * c;

	if(k<0.0)
		k=0.0-k;

	return k;

}


float C3DRCSDoc::CalculateFacetValue(long f)
{
	Vertex vertex;
	int m_facetnum, mF;
	long F[3];
	vector<long> m_nearF;
	float value;
	double S;
	CVect3 m_ProjectPoint, mPV[3];
	
	S=0.0;
	value=0.0;
	F[0]=m_Facet[f].aFacet[0];//��Ƭ����������
	F[1]=m_Facet[f].aFacet[1];
	F[2]=m_Facet[f].aFacet[2];
	
	m_nearF.clear();
	int nFsize;
	bool mhave;
	int j,k;
	for(k=0; k<3; k++)
	{
		S=0.0;
		vertex.x=vertex.y=vertex.z=0.0f;
		m_facetnum=m_Vertex[m_Facet[f].vertex[(k+1)%3]].facet.size();//��Ƭ�е�(k+1)%3���������ڵ�������Ƭ�ĸ���
		for(j=0; j<m_facetnum; j++)
		{
			mF=m_Vertex[m_Facet[f].vertex[(k+1)%3]].facet[j];//��j����Ƭ�ĺ�
			
			nFsize=m_nearF.size();
			mhave=false;
			for(int nn=0; nn<nFsize; nn++)
			{
				if(m_nearF[nn]==mF)
				{ 
					mhave=true;
					break;
				}
			}
			if(!mhave)
				m_nearF.push_back(mF);//����һ����Ƭ����m_nerF��
		
			if((mF!=f)&&(mF!=F[0])&&(mF!=F[1])&&(mF!=F[2])) //ȷ������ΪmF����Ƭ��Ϊm_Facet[m_index]��Ƭ����ֱ������
			{
				m_ProjectPoint=CalculateProjectPoint(m_Vertex[m_Facet[f].vertex[k]], mF);//����ͶӰ��
				vertex.x+=m_ProjectPoint.x*m_Facet[mF].area;//�µ�Ĳ���
				vertex.y+=m_ProjectPoint.y*m_Facet[mF].area;
				vertex.z+=m_ProjectPoint.z*m_Facet[mF].area;
				S+=m_Facet[mF].area;
			}
		}
		
		m_facetnum=m_Vertex[m_Facet[f].vertex[(k+2)%3]].facet.size();
		for( j=0; j<m_facetnum; j++)
		{
			mF=m_Vertex[m_Facet[f].vertex[(k+2)%3]].facet[j];
			
			nFsize=m_nearF.size();
			mhave=false;
			for(int nn=0; nn<nFsize; nn++)
			{
				if(m_nearF[nn]==mF)
				{
					mhave=true;
					break;
				}
			}
			if(!mhave)
				m_nearF.push_back(mF);
			
			if((mF!=f)&&(mF!=F[0])&&(mF!=F[1])&&(mF!=F[2])) //ȷ������ΪmF����Ƭ��Ϊm_Facet[m_index]��Ƭ����ֱ������
			{
				m_ProjectPoint=CalculateProjectPoint(m_Vertex[m_Facet[f].vertex[k]], mF);
				vertex.x+=m_ProjectPoint.x*m_Facet[mF].area;
				vertex.y+=m_ProjectPoint.y*m_Facet[mF].area;
				vertex.z+=m_ProjectPoint.z*m_Facet[mF].area;
				S+=m_Facet[mF].area;
			}
		}

		mPV[k].x=vertex.x / S;
		mPV[k].y=vertex.y / S;
		mPV[k].z=vertex.z / S;
	}

	vertex.x=vertex.y=vertex.z=0.0f;
	for( k=0; k<3; k++)
	{
/*		vertex.x+=0.0f*m_Vertex[m_Facet[f].vertex[k]].x+2.0f*mPV[k].x;
		vertex.y+=0.0f*m_Vertex[m_Facet[f].vertex[k]].y+2.0f*mPV[k].y;
		vertex.z+=0.0f*m_Vertex[m_Facet[f].vertex[k]].z+2.0f*mPV[k].z;*/

		vertex.x+=0.9f*m_Vertex[m_Facet[f].vertex[k]].x+1.1f*mPV[k].x;
		vertex.y+=0.9f*m_Vertex[m_Facet[f].vertex[k]].y+1.1f*mPV[k].y;
		vertex.z+=0.9f*m_Vertex[m_Facet[f].vertex[k]].z+1.1f*mPV[k].z;
	}
	
	m_Facet[f].x=vertex.x /= 6.0;//�۵�������
	m_Facet[f].y=vertex.y /= 6.0;
	m_Facet[f].z=vertex.z /= 6.0;

	nFsize=m_nearF.size();
	value=CalculateDistanceFormVtoF(vertex, m_nearF[0]);
	float dis;
	
	for(int j=1; j<nFsize; j++)
	{
		dis=CalculateDistanceFormVtoF(vertex, m_nearF[j]);
		if(value<dis)
			value=dis;//������
		
	}

	m_Facet[f].value=value;
	return value;
}

/******************************************************************************************************************************
 * 
 * �������ƣ�
 *   CalculateFacetValue_PSO(long f)   By BaiYang on March 24,2014
 *
 * ����:
 *   f              - ��Ƭ����
 *
 * ����ֵ:
 *   value            - ������
 *
 * ˵��:
 *       �ú������ڵ���ִ��ģ�������µ㣬���������롣
 * 
 ******************************************************************************************************************************/

 //-------------------------------------------PSO��ز����Ķ��壬ȫ�ֱ�������By BaiYang on March 25,2014
	const double ParticleV=0.2;       //�ٶ�����
	const int ParticleNum=20;         //������Ŀ
	const int Dim=3;                  //��������  
	const int IterNum=5;              //��������
	const double Err=0.1;           //�������
	
	const double PI=3.141592653589;	
	const double c1=1.494;            //ѧϰ����
	const double c2=1.494;
	const double w=0.729;             //����ϵ��
	const int   alp=1;                //�޶�����
	const double RangeF=2.0;          //ȡֵ��Χ����
	
	double x_range[2],y_range[2],z_range[2];     //���Ž��ȡֵ��Χ
	double Max_V[3];                             //����ٶȱ仯��Χ
	double Particle[ParticleNum][Dim];           //���弯��
	double Particle_Loc_Best[ParticleNum][Dim];  //ÿ������ֲ���������
	double Particle_Loc_Fit[ParticleNum];        //����ľֲ�������Ӧ��,�оֲ����������������
	double Particle_Glo_Best[Dim];               //ȫ����������
	double GFit;                                 //ȫ��������Ӧ��,��ȫ�����������������  
	double Particle_V[ParticleNum][Dim];         //��¼ÿ������ĵ�ǰ���ٶ�����
	double Particle_Fit[ParticleNum];            //��¼ÿ�����ӵĵ�ǰ����Ӧ��

	vector<double> EdgeLength_power;   //��߳�ƽ����
	vector<double> Triangel_Area;//���������ε����
//-------------------------------------------PSO�����õĲ�������	
	vector<int> m_nearLE;//���������߼���
//-------------------------------------------	
void C3DRCSDoc::Initial_Pso()
{
	int i,j;
	for(i=0; i<ParticleNum; i++)            //�漴��������
	{
		Particle[i][0] = x_range[0]+(x_range[1]-x_range[0])*1.0*rand()/RAND_MAX;    //��ʼ��Ⱥ��_���ֵ
		Particle[i][1] = y_range[0]+(y_range[1]-y_range[0])*1.0*rand()/RAND_MAX;
		Particle[i][2] = z_range[0]+(z_range[1]-z_range[0])*1.0*rand()/RAND_MAX;
		for(j=0; j<Dim; j++)
			Particle_Loc_Best[i][j] = Particle[i][j];               //����ǰ���Ž��д��ֲ����ż���

		Particle_V[i][0] = -Max_V[0]+2*Max_V[0]*1.0*rand()/RAND_MAX;  //��ʼ���ٶ�
		Particle_V[i][1] = -Max_V[1]+2*Max_V[1]*1.0*rand()/RAND_MAX;
		Particle_V[i][2] = -Max_V[2]+2*Max_V[2]*1.0*rand()/RAND_MAX;
	}
	for(i=0; i<ParticleNum; i++)            //����ÿ�����ӵ���Ӧ��
	{
		
		Particle_Fit[i] = FitnessPSO(Particle[i]);
		Particle_Loc_Fit[i] = Particle_Fit[i];
	
	}
	GFit = Particle_Loc_Fit[0];      //�ҳ�ȫ������
	j=0;
	for(i=1; i<ParticleNum; i++)
	{
		if(Particle_Loc_Fit[i]<GFit)
		{
			GFit = Particle_Loc_Fit[i];
			j = i;
		}
	}
	for(i=0; i<Dim; i++)             //����ȫ����������  
	{
		Particle_Glo_Best[i] = Particle_Loc_Best[j][i];
	}
}
	
void C3DRCSDoc::Renew_Particle_Pos()
{
	int i,j;
	for(i=0; i<ParticleNum; i++)            //���¸���λ������λ��
	{
		for(j=0; j<Dim; j++)
			Particle[i][j] +=  alp*Particle_V[i][j];

		if(Particle[i][0] > x_range[1])
			Particle[i][0] = x_range[1];
		if(Particle[i][0] < x_range[0])
			Particle[i][0] = x_range[0];

		if(Particle[i][1] > y_range[1])
			Particle[i][1] = y_range[1];
		if(Particle[i][1] < y_range[0])
			Particle[i][1] = y_range[0];

		if(Particle[i][2] > z_range[1])
			Particle[i][2] = z_range[1];
		if(Particle[i][2] < z_range[0])
			Particle[i][2] = z_range[0];
	}
}
void C3DRCSDoc::Renew_Particle_Val()
{
	int i, j;
	for(i=0; i<ParticleNum; i++)            //����ÿ�����ӵ���Ӧ��
	{
		Particle_Fit[i] = FitnessPSO(Particle[i]);
		if(Particle_Fit[i] < Particle_Loc_Fit[i])      //���¸���ֲ�����ֵ
		{
			Particle_Loc_Fit[i] = Particle_Fit[i];
			for(j=0; j<Dim; j++)       // ���¾ֲ���������
			{
				Particle_Loc_Best[i][j] = Particle[i][j];
			}
		}
	}
	for(i=0,j=-1; i<ParticleNum; i++)                   //����ȫ�ֱ���
	{
		if(Particle_Loc_Fit[i]<GFit)
		{
			GFit = Particle_Loc_Fit[i];
			j = i;
		}
	}
	if(j != -1)
	{
		for(i=0; i<Dim; i++)             //����ȫ����������  
		{
			Particle_Glo_Best[i] = Particle_Loc_Best[j][i];
		}
	}
	for(i=0; i<ParticleNum; i++)    //���¸����ٶ�
	{
		for(j=0; j<Dim; j++)
		{
			Particle_V[i][j]=w*Particle_V[i][j]+
				c1*1.0*rand()/RAND_MAX*(Particle_Loc_Best[i][j]-Particle[i][j])+
				c2*1.0*rand()/RAND_MAX*(Particle_Glo_Best[j]-Particle[i][j]);
			if(Particle_V[i][j] > Max_V[j])
			{
				Particle_V[i][j] = Max_V[j];
			}
			if(Particle_V[i][j] < -Max_V[j])
			{
				Particle_V[i][j] = -Max_V[j];
			}
		}
	}
}
double C3DRCSDoc::FitnessPSO(double a[])
{
	Vertex vertex1;
	int i;
	vector<double> EdgeLength_Vertex1; //���µ㵽�߻����е�һ����ľ���
	vector<double> EdgeLength_Vertex2; //���µ㵽�߻����еڶ�����ľ���

	vertex1.x=a[0];
	vertex1.y=a[1];
	vertex1.z=a[2];
	double Temp=0;
	double TriangelStandard=0;   //�������ι淶��
//-----------------------------------------------1.����㵽��ľ���
	EdgeLength_Vertex1.clear();
	EdgeLength_Vertex2.clear();
	EdgeLength_power.clear();
	Triangel_Area.clear();

	for (i=0;i<m_nearLE.size();i++)
	{
		Temp=CalculateLength_PSO(vertex1,m_Edge[m_nearLE[i]].vertex1);
		EdgeLength_Vertex1.push_back(Temp);
		Temp=CalculateLength_PSO(vertex1,m_Edge[m_nearLE[i]].vertex2);
		EdgeLength_Vertex2.push_back(Temp);
	}
//-----------------------------------------------2.������������Ƭ�����
	for (i=0;i<m_nearLE.size();i++)
	{
		Temp=CalculateArea_PSO(m_nearLE[i],EdgeLength_Vertex1[i],EdgeLength_Vertex2[i]);
		Triangel_Area.push_back(Temp);
	}
//-----------------------------------------------3.����߳�ƽ����
	for (i=0;i<m_nearLE.size();i++)
	{
		Temp=m_Edge[m_nearLE[i]].length*m_Edge[m_nearLE[i]].length+EdgeLength_Vertex1[i]*EdgeLength_Vertex1[i]+EdgeLength_Vertex2[i]*EdgeLength_Vertex2[i];
		EdgeLength_power.push_back(Temp);
	}

//-----------------------------------------------3.����߳�ƽ����

	for (i=0;i<m_nearLE.size();i++)
	{
		TriangelStandard+=4*double(sqrt((double)3))*Triangel_Area[i]/(EdgeLength_power[i]);
	}

	return m_nearLE.size()-TriangelStandard;
}

float C3DRCSDoc::CalculateFacetValue_PSO(long f)
{
	//1��ȷ���µı߻���2��ȷ�����б߳���3��ȷ�������������
	Vertex vertex;
	int m_facetnum, mF;
	float value;
	vector<int> m_nearE;//�������б߼���
	vector<int> m_nearF;//�����������漯
	value=0.0;
	int k,j,nn,i;
	m_nearE.clear();
	m_nearLE.clear();
	m_nearF.clear();

	bool mhave;
	bool mhaveF;
//------------------------------------------------------1���������еı߼�------------------------------------------------//
//	FILE *fp;
	vector <int>::iterator Iter;
//	fp=fopen("d:\\result.txt","w");
	
	vertex.x=vertex.y=vertex.z=0.0f;

	for(k=0; k<3; k++)
	{
		m_facetnum=m_Vertex[m_Facet[f].vertex[k%3]].facet.size();//��Ƭ�е�(k+1)%3���������ڵ�������Ƭ�ĸ���
	//	fprintf(fp,"\n��%d��������%d����Ƭ\n",k,m_facetnum);
	
		for(j=0; j<m_facetnum; j++)
		{
			mF=m_Vertex[m_Facet[f].vertex[k%3]].facet[j];//��j����Ƭ�ĺ�
	//		fprintf(fp,"%d\t%d\t%d\t%d\n",mF,m_Facet[mF].vertex[0],m_Facet[mF].vertex[1],m_Facet[mF].vertex[2]);
		
			mhaveF=false;
			for(nn=0; nn<m_nearF.size(); nn++)
			{
				if(m_nearF[nn]==mF)
				{
					mhaveF=true;
					break;
				}
			}
			if(!mhaveF)
				m_nearF.push_back(mF);//����һ����Ƭ����m_nerF��


			for (nn=0;nn<3;nn++)
			{
				mhave=false;
				for (Iter=m_nearE.begin();Iter!=m_nearE.end();Iter++)
					if (m_Facet[mF].edge[nn]==*Iter)
					{
						mhave=true;
						break;
					}
				if (!mhave)
				{
					m_nearE.push_back(m_Facet[mF].edge[nn]);
				}
				
			}
		}
	}
// 	CString str,str1;
// 	for(int ii=0;ii<m_nearE.size();ii++)
// 	{
// 		fprintf(fp,"\n%d\t%d\t%d",m_nearE[ii],m_Edge[m_nearE[ii]].vertex1,m_Edge[m_nearE[ii]].vertex2);
// 		str.Format("%d",m_nearE[ii]);
// 		str1+=str;
// 		str1+='\t';
// 	}
// 	str.Format("\n%d",m_nearE.size());
// 	str1+=str;
// 	AfxMessageBox(str1);
 	
//------------------------------------------------------2���������еĻ��߼�----------------------------------------------//
	for (i=0;i<m_nearE.size();i++)
		if(m_Facet[f].vertex[0]!=m_Edge[m_nearE[i]].vertex1 && m_Facet[f].vertex[0]!=m_Edge[m_nearE[i]].vertex2
		&& m_Facet[f].vertex[1]!=m_Edge[m_nearE[i]].vertex1 && m_Facet[f].vertex[1]!=m_Edge[m_nearE[i]].vertex2
		&& m_Facet[f].vertex[2]!=m_Edge[m_nearE[i]].vertex1 && m_Facet[f].vertex[2]!=m_Edge[m_nearE[i]].vertex2)
		{
			m_nearLE.push_back(m_nearE[i]);
		}
		

// 	CString str,str1;
// 	for(int nn=0;nn<m_nearLE.size();nn++)
// 	{
// 		str.Format("%d",m_nearLE[nn]);
// 		str1+=str;
// 		str1+='\t';
// 	}
// 	str.Format("\n%d",m_nearLE.size());
// 	str1+=str;
// 	AfxMessageBox(str1);
// 
// 	for (i=0;i<m_nearLE.size();i++)
// 		fprintf(fp,"��%4d���߳�Ϊ%5.8f,ָ��Ķ���1Ϊ%d,����2Ϊ%d\n",m_nearLE[i],m_Edge[m_nearLE[i]].length,m_Edge[m_nearLE[i]].vertex1,m_Edge[m_nearLE[i]].vertex2);
// 
// 	fclose(fp);

//------------------------------------------------------3��PSO�趨�µ��λ�ã���߳������----------------------------------------------//
//------------------------------------------------------3.1��ȷ������ȡֵ��Χ
	x_range[0]=m_Vertex[m_Facet[f].vertex[0]].x;
	y_range[0]=m_Vertex[m_Facet[f].vertex[0]].y;
	z_range[0]=m_Vertex[m_Facet[f].vertex[0]].z;
	for (i=1;i<3;i++)
	{
		if (x_range[0]>m_Vertex[m_Facet[f].vertex[i]].x)
			x_range[0]=m_Vertex[m_Facet[f].vertex[i]].x;
		else
			x_range[1]=m_Vertex[m_Facet[f].vertex[i]].x;

		if (y_range[0]>m_Vertex[m_Facet[f].vertex[i]].y)
			y_range[0]=m_Vertex[m_Facet[f].vertex[i]].y;
		else
			y_range[1]=m_Vertex[m_Facet[f].vertex[i]].y;

		if (z_range[0]>m_Vertex[m_Facet[f].vertex[i]].z)
			z_range[0]=m_Vertex[m_Facet[f].vertex[i]].z;
		else
			z_range[1]=m_Vertex[m_Facet[f].vertex[i]].z;

	}
	Max_V[0]=ParticleV*(x_range[1]-x_range[0]);
	Max_V[1]=ParticleV*(y_range[1]-y_range[0]);
	Max_V[2]=ParticleV*(z_range[1]-z_range[0]);
	for (i=0;i<2;i++)
	{
		x_range[i]=RangeF*x_range[i];
		y_range[i]=RangeF*y_range[i];
		z_range[i]=RangeF*z_range[i];
	}

//------------------------------------------------------3.2������PSO
// 	
	FILE *fp;
	fp=fopen("d:\\result.txt","a+");
	int iternum=0;
	double CurrentError=0.0;

	srand((unsigned)time(NULL));
	Initial_Pso();
	while(iternum < IterNum || GFit<Err)
	{
		Renew_Particle_Pos();
		Renew_Particle_Val();
		iternum++;
	}
	fprintf(fp,"���Ӹ���:%2d\t",ParticleNum);
	fprintf(fp,"ά��Ϊ:%2d\t",Dim);
	fprintf(fp,"����ָ���Ϊ%.10lf\t", m_nearLE.size()-GFit);
	for (i=0;i<m_nearLE.size();i++)
	{
		fprintf(fp,"%.5lf\t",4*double(sqrt((double)3))*Triangel_Area[i]/(EdgeLength_power[i]));
	}
	fprintf(fp,"\n");
	fclose(fp);

//------------------------------------------------------3.3���������ŵ�λ�ã�����value

	vertex.x=Particle_Glo_Best[0];
	vertex.y=Particle_Glo_Best[1];
	vertex.z=Particle_Glo_Best[2];

	m_Facet[f].x=vertex.x;//�۵�������
	m_Facet[f].y=vertex.y;
	m_Facet[f].z=vertex.z;
//------------------------------------------------------3.4������value

	value=CalculateDistanceFormVtoF(vertex, m_nearF[0]);
	float dis;
	
	for(int j=1; j<m_nearF.size(); j++)
	{
		dis=CalculateDistanceFormVtoF(vertex, m_nearF[j]);
		if(value<dis)
			value=dis;//������	
	}
//------------------------------------------------------3.5�����㱻�۵�������Ƭ��������,��������������
// 	double Temp1;
// 	Temp1=4*sqrt(3)*m_Facet[f].area/(m_Edge[m_Facet[f].edge[0]].length*m_Edge[m_Facet[f].edge[0]].length+
// 		                             m_Edge[m_Facet[f].edge[1]].length*m_Edge[m_Facet[f].edge[1]].length+
// 									 m_Edge[m_Facet[f].edge[2]].length*m_Edge[m_Facet[f].edge[2]].length);

	m_Facet[f].value=value;
	return value;

	
//	return 0;
}


//ȥ���⻷��
bool C3DRCSDoc::DeleteRing()
{
	int cr;
	for(int n=0; n< m_nFileNum+4; n++)
	{
		for(int i=0 ;i<m_nHeight;i++)
		{
			for(int j=0;j<m_nByteWidth;j++)
			{
				if(j<m_nWidth)
				{						
					cr=sqrt(double((2*i-m_nHeight)*(2*i-m_nHeight)+(2*j-m_nWidth)*(2*j-m_nWidth)))+8;
					if(cr>=m_nHeight)
						m_Pixels[n*m_nHeight*m_nWidth+i*m_nWidth+j]=0;
				}
			}
		}
	}
	return true;
} 


void C3DRCSDoc::FillHole( )
{
	Vertex vertexa, vertexb;
	Facet facet1, facet2, facet3;

	vector<float>  angle;              //�׶��ߵļн�
	vector<float>  t_area;             //��������ʱ�����ε����
	vector<CVect3> f_normal;           //��������ʱ�����εķ�����
	vector<CVect3> v_normal;           //�߽�㷨ʸ��
	vector<float>  p_angle;            //�߽��ͶӰ�н�
	vector<Vertex> s_vertex;
	vector<int> ted;            //�߽�����Ӧ�����ڱ߽�����ߵ�λ�ù�ϵ

	do{
		float v_flat = 0.0, ave_flat = 0.0;

		angle.clear();
		t_area.clear();
		f_normal.clear();
		v_normal.clear();
		p_angle.clear();
		s_vertex.clear();
		ted.clear();

		ExtractBoundary_DC();

        if (3 == BoundaryEdge.size())
		{
			facet1.vertex[0] = m_Edge[BoundaryEdge[0].edgenum].vertex1;
			facet1.vertex[1] = m_Edge[BoundaryEdge[1].edgenum].vertex1;
			facet1.vertex[2] = m_Edge[BoundaryEdge[2].edgenum].vertex1;
			CalculateFacetNormal(facet1);
			m_Facet.push_back(facet1);
			m_Vertex[BoundaryEdge[0].vertex1].facet.push_back(m_FacetNumber);
			m_Vertex[BoundaryEdge[1].vertex1].facet.push_back(m_FacetNumber);
			m_Vertex[BoundaryEdge[2].vertex1].facet.push_back(m_FacetNumber);
			m_FacetNumber++;
//			return;
			break;
		}

		//����ÿ���߽��Ŀ׶��߼нǡ���ʱ�������������ʱ�����η�����
		for (int i = 0; i < BoundaryEdge.size(); i++)
		{
			for (int j = 0; j < BoundaryEdge.size(); j++)
			{
				if (i == j)
					continue;
				if (BoundaryEdge[i].vertex2 == BoundaryEdge[j].vertex1)
				{
					Vertex v0, v1, v2;
					Edge  t_edge, t_ted;

					if (0 == i)
						v1 = BoundaryVertex.back();
					else
						v1 = BoundaryVertex[i - 1];

					v0 = BoundaryVertex[i];

					if (BoundaryEdge.size() - 1 == i)
					{
						v2 = BoundaryVertex[0];
					}
					else
					{
						v2 = BoundaryVertex[i + 1];
					}

					t_ted.vertex1 = BoundaryEdge[i].vertex1;
					t_ted.vertex2 = BoundaryEdge[j].vertex2;
					int kkj;
					kkj = IsOnLeft(BoundaryVertex[i], t_ted);
					ted.push_back(kkj);

					t_edge.vertex1 = BoundaryEdge[j].vertex2;
					t_edge.vertex2 = BoundaryEdge[i].vertex1;
					t_edge.length = CalculateLength_DC(v1, v2);

					CVect3 a, b;

					a.x = v1.x - v0.x;
					a.y = v1.y - v0.y;
					a.z = v1.z - v0.z;
					b.x = v2.x - v0.x;
					b.y = v2.y - v0.y;
					b.z = v2.z - v0.z;

					double nn;

					nn = (a*b) / sqrt((a*a)*(b*b));

					angle.push_back(acos(nn)*180.0 / 3.1415926);

					t_area.push_back(CalculateArea_DC(t_edge, BoundaryEdge[i], BoundaryEdge[j]));

					f_normal.push_back(CalculateFacetNormal_DC(v0, v2, v1));

					break;
				}
			}
		}

		//����߽��ķ�ʸ��
		CVect3 t_v_normal;
		for (int vv = 0; vv < BoundaryVertex.size(); vv++)
		{
			float t = expf(-angle[vv] * 3.1415926 / 180.0);
			//	float t = pow(2.17828, -(angle[vv]*3.1415926/180.0));
			float v_x = 0.0, v_y = 0.0, v_z = 0.0;

			for (int kk = 0; kk < BoundaryVertex[vv].facet.size(); kk++)
			{
				v_x += m_Facet[BoundaryVertex[vv].facet[kk]].nx * m_Facet[BoundaryVertex[vv].facet[kk]].area;
				v_y += m_Facet[BoundaryVertex[vv].facet[kk]].ny * m_Facet[BoundaryVertex[vv].facet[kk]].area;
				v_z += m_Facet[BoundaryVertex[vv].facet[kk]].nz * m_Facet[BoundaryVertex[vv].facet[kk]].area;
			}

			float sq = sqrt((v_x + t*t_area[vv] * f_normal[vv].x)*(v_x + t*t_area[vv] * f_normal[vv].x) +
				(v_y + t*t_area[vv] * f_normal[vv].y)*(v_y + t*t_area[vv] * f_normal[vv].y) +
				(v_z + t*t_area[vv] * f_normal[vv].z)*(v_z + t*t_area[vv] * f_normal[vv].z));

			t_v_normal.x = (t*t_area[vv] * f_normal[vv].x + v_x) / sq;
			t_v_normal.y = (t*t_area[vv] * f_normal[vv].y + v_y) / sq;
			t_v_normal.z = (t*t_area[vv] * f_normal[vv].z + v_z) / sq;

			v_normal.push_back(t_v_normal);
		}

		//�߽��ͶӰ�н�
		for (int ii = 0; ii < BoundaryEdge.size(); ii++)
		{
			Vertex v0, v1, v2;

			if (0 == ii)
				v1 = BoundaryVertex.back();
			else
				v1 = BoundaryVertex[ii - 1];

			v0 = BoundaryVertex[ii];

			if (BoundaryEdge.size() - 1 == ii)
			{
				v2 = BoundaryVertex[0];
			}
			else
			{
				v2 = BoundaryVertex[ii + 1];
			}

			CVect3 a, b, e1, e2;

			a.x = v1.x - v0.x;
			a.y = v1.y - v0.y;
			a.z = v1.z - v0.z;
			b.x = v2.x - v0.x;
			b.y = v2.y - v0.y;
			b.z = v2.z - v0.z;
	
			e1.x = a.x - (a * v_normal[ii]) * v_normal[ii].x;
			e1.y = a.y - (a * v_normal[ii]) * v_normal[ii].y;
			e1.z = a.z - (a * v_normal[ii]) * v_normal[ii].z;
			e2.x = b.x - (b * v_normal[ii]) * v_normal[ii].x;
			e2.y = b.y - (b * v_normal[ii]) * v_normal[ii].y;
			e2.z = b.z - (b * v_normal[ii]) * v_normal[ii].z;
	
			double kk = 0.0;
			kk = (e1*e2) / sqrt((e1*e1)*(e2*e2));

			p_angle.push_back(acos(kk)*180.0 / 3.1415926);
		}

		//����ƽ��ƽչ��
		for (int ff = 0; ff < BoundaryVertex.size(); ff++)
		{
			ave_flat += CalculateVertexFlatness_DC(BoundaryVertex[ff]);
		}

		ave_flat /= BoundaryVertex.size();

		//	��ƽչ��С��ƽ��ƽչ�ȵĵ��ͶӰ�ǵ����洢
		vector<float>  samll_angle;
		samll_angle.clear();

		for (int ss = 0; ss < p_angle.size(); ss++)
		{
//			v_flat = CalculateVertexFlatness_DC(BoundaryVertex[ss]);

			if (BoundaryVertex[ss].facet.size()>3)
				samll_angle.push_back(p_angle[ss]);
		}

		//ʼ��ָ����С��Ԫ��
		vector<float>::iterator  itr;
/*		if (samll_angle.size() != 0)
			itr = min_element(begin(samll_angle), end(samll_angle));
		else
		{
			for (int sst = 0; sst < p_angle.size(); sst++)
			{
				v_flat = CalculateVertexFlatness_DC(BoundaryVertex[sst]);

				if (v_flat < ave_flat)
					samll_angle.push_back(p_angle[sst]);
			}
			itr = min_element(begin(samll_angle), end(samll_angle));
		}
	*/
		itr = min_element(begin(samll_angle), end(samll_angle));
		//��ʼ�
		for (int i = 0; i < BoundaryVertex.size(); i++)
		{
			if (p_angle[i] > 180.0 || p_angle[i] < 0.0)
				continue;

//			v_flat = CalculateVertexFlatness_DC(BoundaryVertex[i]);

			if (p_angle[i] == *itr)    //�ȴ�ƽչ��С��ƽ��ƽչ����ͶӰ����С�ĵ㿪ʼ�޲�
			{
			    if (p_angle[i] < 90.0 )  //��һ�����
				{
			    	for (int j = 0; j < BoundaryVertex.size(); j++)
					{
						if (i == j)
							continue;

						if (BoundaryEdge[i].vertex2 == BoundaryEdge[j].vertex1)
						{
							facet1.vertex[0] = BoundaryEdge[i].vertex2;
							facet1.vertex[1] = BoundaryEdge[i].vertex1;
							facet1.vertex[2] = BoundaryEdge[j].vertex2;
							CalculateFacetNormal(facet1);
							m_Facet.push_back(facet1);
							m_Vertex[BoundaryEdge[i].vertex2].facet.push_back(m_FacetNumber);
							m_Vertex[BoundaryEdge[i].vertex1].facet.push_back(m_FacetNumber);
							m_Vertex[BoundaryEdge[j].vertex2].facet.push_back(m_FacetNumber);
							m_FacetNumber++;

							ExtractLines_DC();
							break;
	                    }
						else
						    continue;
					}
					break;
				}
				else if (90.0 <= p_angle[i] && p_angle[i] <= 180.0) //�ڶ������
				{
				    for (int j = 0; j < BoundaryVertex.size(); j++)
					{
						if (i == j)
							continue;

						if (BoundaryEdge[i].vertex2 == BoundaryEdge[j].vertex1)
						{
							Vertex v0, v1, v2;
							CVect3 a, b, e, tempt, xcross, org_point, TP_a, TP_b;
							float l, sigema, cosbeta1, cosbeta2, k1, k2;

							if (0 == i)
								v1 = BoundaryVertex.back();
							else
								v1 = BoundaryVertex[i - 1];

							v0 = BoundaryVertex[i];

							if (BoundaryEdge.size() - 1 == i)
								v2 = BoundaryVertex[0];
							else
								v2 = BoundaryVertex[i + 1];

							a.x = v1.x - v0.x;       //���ڱ߽�ߵ�������ʾ
							a.y = v1.y - v0.y;
							a.z = v1.z - v0.z;
							b.x = v2.x - v0.x;
							b.y = v2.y - v0.y;
							b.z = v2.z - v0.z;
							e = (a / sqrt(a * a) + b / sqrt(b * b)) / sqrt((a / sqrt(a * a) + b / sqrt(b * b))*(a / sqrt(a * a) + b / sqrt(b * b)));
						    
							TP_a.x = a.x - (a * v_normal[i]) * v_normal[i].x;       //�߽������ƽ���ϵ�ͶӰ��
							TP_a.y = a.y - (a * v_normal[i]) * v_normal[i].y;
							TP_a.z = a.z - (a * v_normal[i]) * v_normal[i].z;
					        TP_b.x = b.x - (b * v_normal[i]) * v_normal[i].x;
							TP_b.y = b.y - (b * v_normal[i]) * v_normal[i].y;
							TP_b.z = b.z - (b * v_normal[i]) * v_normal[i].z;

					//		e = TP_a / sqrt(TP_a * TP_a) + TP_b / sqrt(TP_b * TP_b);   //�߽������ƽ���ϵ�ͶӰ�ߵĽ�ƽ��������
							
							l = (sqrt(a*a) + sqrt(b*b)) / 2;                //��������v0��ʵ�ʾ���

							cosbeta1 = (v_normal[i] * a) / sqrt((a*a)*(v_normal[i] * v_normal[i])); //�߽����㷨��нǵ�����ֵ
							cosbeta2 = (v_normal[i] * b) / sqrt((b*b)*(v_normal[i] * v_normal[i]));
							double ftt = acos(cosbeta1)*180.0/3.1415926;
							double fta = acos(cosbeta2)*180.0/3.1415926;

							k1 = 2.0 * cosbeta1 / sqrt(a*a);  //�������� 
							k2 = 2.0 * cosbeta2 / sqrt(b*b);

							sigema = (k1 + k2)*l / 8.0;  //������,Ϊ����ʾ����������㷨��н�Ϊ�����õ�Ϊ���㣬Ӧ��н�����ķ��������Ϊ����ʾ����������㷨��н�Ϊ�����õ�Ϊ͹�㣬Ӧ��н���С�ķ������
							double gdd = acos(sigema)*180.0/3.1415926;

							if (sigema >= 0)
							{
								xcross.x = e.y*v_normal[i].z - e.z*v_normal[i].y;
								xcross.y = e.z*v_normal[i].x - e.x*v_normal[i].z;
								xcross.z = e.x*v_normal[i].y - e.y*v_normal[i].x;

								/*							xcross.x = e.y*(v_normal[i].z - BoundaryVertex[i].z) - e.z*(v_normal[i].y - BoundaryVertex[i].y);
								xcross.y = e.z*(v_normal[i].x - BoundaryVertex[i].x) - e.x*(v_normal[i].z - BoundaryVertex[i].z);
								xcross.z = e.x*(v_normal[i].y - BoundaryVertex[i].y) - e.y*(v_normal[i].x - BoundaryVertex[i].x);
								*/
				/*								org_point.x = BoundaryVertex[i].x + l*e.x;
								org_point.y = BoundaryVertex[i].y + l*e.y;
								org_point.z = BoundaryVertex[i].z + l*e.z;
					*/			
						/*		org_point.x = l*(e.x / sqrt(e * e));
								org_point.y = l*(e.y / sqrt(e * e));
								org_point.z = l*(e.z / sqrt(e * e));
							*/	org_point.x = l*e.x;
								org_point.y = l*e.y;
								org_point.z = l*e.z;

								tempt = RotatePoint_DC(org_point, xcross, -sigema);
								
				/*				vertexa.x = tempt.x;
								vertexa.y = tempt.z;
								vertexa.z = tempt.x;
					*/			
								vertexa.x = tempt.x + BoundaryVertex[i].x;
								vertexa.y = tempt.y + BoundaryVertex[i].y;
								vertexa.z = tempt.z + BoundaryVertex[i].z;
								m_Vertex.push_back(vertexa);
								m_VertexNumber++;

								facet1.vertex[0] = m_Edge[BoundaryEdge[i].edgenum].vertex1;
								facet1.vertex[1] = m_Vertex.size() - 1;
								facet1.vertex[2] = m_Edge[BoundaryEdge[i].edgenum].vertex2;
								CalculateFacetNormal(facet1);
								m_Facet.push_back(facet1);
								m_Vertex[BoundaryEdge[i].vertex2].facet.push_back(m_FacetNumber);
								m_Vertex[BoundaryEdge[i].vertex1].facet.push_back(m_FacetNumber);
								m_Vertex[m_Vertex.size() - 1].facet.push_back(m_FacetNumber);
								m_FacetNumber++;

								facet2.vertex[0] = m_Edge[BoundaryEdge[j].edgenum].vertex1;
								facet2.vertex[1] = m_Vertex.size() - 1;
								facet2.vertex[2] = m_Edge[BoundaryEdge[j].edgenum].vertex2;
								CalculateFacetNormal(facet2);
								m_Facet.push_back(facet2);
								m_Vertex[BoundaryEdge[i].vertex2].facet.push_back(m_FacetNumber);
								m_Vertex[BoundaryEdge[j].vertex2].facet.push_back(m_FacetNumber);
								m_Vertex[m_Vertex.size() - 1].facet.push_back(m_FacetNumber);
								m_FacetNumber++;

								ExtractLines_DC();
								break;
							}
							else if (sigema < 0)
							{
								xcross.x = e.y*v_normal[i].z - e.z*v_normal[i].y;
								xcross.y = e.z*v_normal[i].x - e.x*v_normal[i].z;
								xcross.z = e.x*v_normal[i].y - e.y*v_normal[i].x;
								
					/*			xcross.x = e.y*(v_normal[i].z - BoundaryVertex[i].z) - e.z*(v_normal[i].y - BoundaryVertex[i].y);
								xcross.y = e.z*(v_normal[i].x - BoundaryVertex[i].x) - e.x*(v_normal[i].z - BoundaryVertex[i].z);
								xcross.z = e.x*(v_normal[i].y - BoundaryVertex[i].y) - e.y*(v_normal[i].x - BoundaryVertex[i].x);
								*/
								org_point.x = l*e.x;
								org_point.y = l*e.y;
								org_point.z = l*e.z;
					/*			org_point.x = l*(e.x / sqrt(e * e));
								org_point.y = l*(e.y / sqrt(e * e));
								org_point.z = l*(e.z / sqrt(e * e));
								*/
								tempt = RotatePoint_DC(org_point, xcross, -sigema);

								vertexa.x = tempt.x + BoundaryVertex[i].x;
								vertexa.y = tempt.y + BoundaryVertex[i].y;
								vertexa.z = tempt.z + BoundaryVertex[i].z;
								m_Vertex.push_back(vertexa);
								m_VertexNumber++;

								facet1.vertex[0] = m_Edge[BoundaryEdge[i].edgenum].vertex1;
								facet1.vertex[1] = m_Vertex.size() - 1;
								facet1.vertex[2] = m_Edge[BoundaryEdge[i].edgenum].vertex2;
								CalculateFacetNormal(facet1);
 								m_Facet.push_back(facet1);
								m_Vertex[BoundaryEdge[i].vertex2].facet.push_back(m_FacetNumber);
								m_Vertex[BoundaryEdge[i].vertex1].facet.push_back(m_FacetNumber);
								m_Vertex[m_Vertex.size() - 1].facet.push_back(m_FacetNumber);
								m_FacetNumber++;

								facet2.vertex[0] = m_Edge[BoundaryEdge[j].edgenum].vertex1;
								facet2.vertex[1] = m_Vertex.size() - 1;
								facet2.vertex[2] = m_Edge[BoundaryEdge[j].edgenum].vertex2;
								CalculateFacetNormal(facet2);
								m_Facet.push_back(facet2);
								m_Vertex[BoundaryEdge[i].vertex2].facet.push_back(m_FacetNumber);
								m_Vertex[BoundaryEdge[j].vertex2].facet.push_back(m_FacetNumber);
								m_Vertex[m_Vertex.size() - 1].facet.push_back(m_FacetNumber);
								m_FacetNumber++;

								ExtractLines_DC();
								break;
							}
						}
						else
							continue;
					}
					break;
				}
/*				else                //���������
				{
					for (int j = 0; j < BoundaryVertex.size(); j++)
					{
						if (i == j)
							continue;

						if (BoundaryEdge[i].vertex2 == BoundaryEdge[j].vertex1)
						{
							Vertex v0, v1, v2;
							CVect3 a, b, e1, e2, e01, e02, xcross, xcross1, xcross2, org_point1, org_point2, tempt1, tempt2, TP_a, TP_b;
							float l01, l02, sigema1, sigema2, cosbeta1, cosbeta2, k1, k2, k01, k02;

							if (0 == i)
								v1 = BoundaryVertex.back();
							else
								v1 = BoundaryVertex[i - 1];

							v0 = BoundaryVertex[i];

							if (BoundaryEdge.size() - 1 == i)
								v2 = BoundaryVertex[0];
							else
								v2 = BoundaryVertex[i + 1];

							a.y = v1.y - v0.y;
							a.x = v1.x - v0.x;
							a.z = v1.z - v0.z;
							b.x = v2.x - v0.x;
							b.y = v2.y - v0.y;
							b.z = v2.z - v0.z;

							TP_a.x = a.x - (a * v_normal[i]) * v_normal[i].x;       //�߽������ƽ���ϵ�ͶӰ��
							TP_a.y = a.y - (a * v_normal[i]) * v_normal[i].y;
							TP_a.z = a.z - (a * v_normal[i]) * v_normal[i].z;
							TP_b.x = b.x - (b * v_normal[i]) * v_normal[i].x;
							TP_b.y = b.y - (b * v_normal[i]) * v_normal[i].y;
							TP_b.z = b.z - (b * v_normal[i]) * v_normal[i].z;

							xcross.x = TP_a.y*TP_b.z - TP_a.z*TP_b.y;
							xcross.y = TP_a.z*TP_b.x - TP_a.x*TP_b.z;
							xcross.z = TP_a.x*TP_b.y - TP_a.y*TP_b.x;

							e1 = RotatePoint_DC(TP_a, xcross, angle[i] * 3.1415926 / 540.0);//�������ȷ��ߵĵ�λ����
							e2 = RotatePoint_DC(TP_a, xcross, angle[i] * 3.1415926 / 270.0);
							e01 = e1 / sqrt(e1*e1);
							e02 = e2 / sqrt(e2*e2);

							cosbeta1 = v_normal[i] * a / sqrt((a*a)*(v_normal[i] * v_normal[i])); //�߽����㷨��нǵ�����ֵ
							cosbeta2 = v_normal[i] * b / sqrt((b*b)*(v_normal[i] * v_normal[i]));

							k1 = 2.0 * cosbeta1 / sqrt(a*a);  //��������
							k2 = 2.0 * cosbeta2 / sqrt(b*b);
							k01 = 0.666666 * k1 + 0.333333 * k2;
							k02 = 0.333333 * k1 + 0.666666 * k2;

							l01 = 0.666666 * sqrt(a*a) + 0.333333 * sqrt(b*b);
							l02 = 0.333333 * sqrt(a*a) + 0.666666 * sqrt(b*b);

							sigema1 = k01*l01 / 2.0;  //������
							sigema2 = k02*l02 / 2.0;

							xcross1.x = e01.y*v_normal[i].z - e01.z*v_normal[i].y;
							xcross1.y = e01.z*v_normal[i].x - e01.x*v_normal[i].z;
							xcross1.z = e01.x*v_normal[i].y - e01.y*v_normal[i].x;
							xcross2.x = e02.y*v_normal[i].z - e02.z*v_normal[i].y;
							xcross2.y = e02.z*v_normal[i].x - e02.x*v_normal[i].z;
							xcross2.z = e02.x*v_normal[i].y - e02.y*v_normal[i].x;

							org_point1.x = l01*e01.x;
							org_point1.y = l01*e01.y;
							org_point1.z = l01*e01.z;
							org_point2.x = l02*e02.x;
							org_point2.y = l02*e02.y;
							org_point2.z = l02*e02.z;

							tempt1 = RotatePoint_DC(org_point1, xcross1, sigema1);
							tempt2 = RotatePoint_DC(org_point2, xcross2, sigema2);

							vertexa.x = tempt1.x + BoundaryVertex[i].x;
							vertexa.y = tempt1.y + BoundaryVertex[i].y;
							vertexa.z = tempt1.z + BoundaryVertex[i].z;
							vertexb.x = tempt2.x + BoundaryVertex[i].x;
							vertexb.y = tempt2.y + BoundaryVertex[i].y;
							vertexb.z = tempt2.z + BoundaryVertex[i].z;

							m_Vertex.push_back(vertexa);
							m_VertexNumber++;
							m_Vertex.push_back(vertexb);
							m_VertexNumber++;

							facet1.vertex[0] = m_Edge[BoundaryEdge[i].edgenum].vertex1;
							facet1.vertex[1] = m_Vertex.size() - 2;
							facet1.vertex[2] = m_Edge[BoundaryEdge[i].edgenum].vertex2;
							CalculateFacetNormal(facet1);
							m_Facet.push_back(facet1);
							m_Vertex[BoundaryEdge[i].vertex1].facet.push_back(m_FacetNumber);
							m_Vertex[BoundaryEdge[i].vertex2].facet.push_back(m_FacetNumber);
							m_Vertex[m_Vertex.size() - 2].facet.push_back(m_FacetNumber);
							m_FacetNumber++;

							facet2.vertex[0] = m_Vertex.size() - 2;
							facet2.vertex[1] = m_Vertex.size() - 1;
							facet2.vertex[2] = m_Edge[BoundaryEdge[i].edgenum].vertex2;				
							CalculateFacetNormal(facet2);
							m_Facet.push_back(facet2);
							m_Vertex[BoundaryEdge[i].vertex2].facet.push_back(m_FacetNumber);
							m_Vertex[m_Vertex.size() - 2].facet.push_back(m_FacetNumber);
							m_Vertex[m_Vertex.size() - 1].facet.push_back(m_FacetNumber);
							m_FacetNumber++;

							facet3.vertex[0] = m_Vertex.size() - 1;
							facet3.vertex[1] = m_Edge[BoundaryEdge[j].edgenum].vertex2;
							facet3.vertex[2] = m_Edge[BoundaryEdge[j].edgenum].vertex1;			
							CalculateFacetNormal(facet3);
							m_Facet.push_back(facet3);
							m_Vertex[BoundaryEdge[i].vertex2].facet.push_back(m_FacetNumber);
							m_Vertex[BoundaryEdge[j].vertex2].facet.push_back(m_FacetNumber);
							m_Vertex[m_Vertex.size() - 1].facet.push_back(m_FacetNumber);
							m_FacetNumber++;

							ExtractLines_DC();
							break;

						}
						else
							continue;
					}
					break;
				}
*/			}
//			}
		else
			continue;
	    }
		
	}
	while (BoundaryEdge.size() > 3);
}



void C3DRCSDoc::ReduceMesh_Adaptive()
{
 //  m_ReduceAngleThreshold=(this->m_pFr->m_pCtrlWnd->m_nReduceAngleThreshold)*3.14159/180.0;
	//m_AreaThreshold=this->m_pFr->m_pCtrlWnd->m_nReduceAreaThreshold;
	//m_VolumeThreshold=this->m_pFr->m_pCtrlWnd->m_nReduceVolumeThreshold;
	m_ReduceRateEdit=this->m_pFr->m_ReduceRate3/100.0f;



	int AngleLoopControlNum=0;
 	int m_facetsize=m_Facet.size();
 	float nprocess=m_ReduceNum/(float)m_facetsize;
do 
{
	nprocess=m_ReduceNum/(float)m_facetsize;

	if(m_ReduceNum%128==0)
	{
		this->m_pFr->m_Progress.SetPos(nprocess*90+10);
	}

	this->CalculateValue_Adaptive();
	this->ReductFacetsHash_Adaptive();

	AngleLoopControlNum++;
	m_ReduceAngleThreshold=float((5.0+AngleLoopControlNum*3)*3.14159/180.0);
	

}while(nprocess<=m_ReduceRateEdit);

	
	this->m_pFr->m_Progress.SetPos(100); Sleep(100); this->m_pFr->m_Progress.SetPos(0);
	

	long mFN=m_Facet.size();
	m_ReduceRate=100.0*m_ReduceNum/(double)mFN;


}



/********************************************************************************
*
*��������
*       ExtractBoundary_DC()
*
*˵��������DC�㷨��ȡ�߽�㡢�ߡ���
*
********************************************************************************/
void C3DRCSDoc::ExtractBoundary_DC()
{
	Edge b_edge;
	Facet b_facet;
	Vertex b_vertex;
	
	BoundaryEdge.clear();
	BoundaryFacet.clear();
	BoundaryVertex.clear();

	for (int i = 0; i < m_FacetNumber; i++ )  //�洢�߽�㡢�ߡ���
	{
		for (int j = 0; j < 3; j++)
		{
			b_edge = m_Edge[m_Facet[i].edge[j]];

			if (-1 == b_edge.rfacet || -1 == b_edge.facet)
			{
//				b_edge.rfacet = -1;
//				b_edge.facet = i;
				b_edge.eBoundary = true;

				BoundaryEdge.push_back(b_edge);
			}
			else
				continue;
		}
	}

	if(BoundaryEdge.size() >= 3)
		m_hole = true;
	else
	{
		(AfxMessageBox(" �޿׶� "));
		return;
	}
	
	if (m_hole)                //��������߽��
	{
		Edge temp_e;
		Facet temp_f;
		Vertex temp_v;

		for(int m = 0; m < BoundaryEdge.size()-1; m++)
		{
			for (int n = m+1; n < BoundaryEdge.size(); n++)
			{
				if (BoundaryEdge[m].vertex2 == BoundaryEdge[n].vertex1)
				{
					temp_e = BoundaryEdge[m+1];
					BoundaryEdge[m+1] = BoundaryEdge[n];
					BoundaryEdge[n] = temp_e;

					break;
				}
			}
		}

		for (int k = 0; k < BoundaryEdge.size(); k++)   //���������ı߽�㡣��
		{
			b_vertex = m_Vertex[BoundaryEdge[k].vertex2];
			if(BoundaryEdge[k].facet == -1)
			    b_facet = m_Facet[BoundaryEdge[k].rfacet];
			else
				b_facet = m_Facet[BoundaryEdge[k].facet];

			b_vertex.bBoundary = true;
			b_facet.fBoundary = true;

			BoundaryVertex.push_back(b_vertex);
			BoundaryFacet.push_back(b_facet);
		}
	}

}
	
float C3DRCSDoc::CalculateVertexFlatness_DC(Vertex v)   //������ƽչ��
{
	float e_angle = 0.0, ave_angle = 0.0;
	int k = 0;

	vector<Edge> t_edge;
	vector<Edge>::iterator itr1, itr2;

	Edge temp;

	t_edge.clear();

	for (int i = 0; i < v.facet.size(); i++)     //��������ڱߴ���߼�
	{
		for (int j=0; j<3; j++)
		{
			temp = m_Edge[m_Facet[v.facet[i]].edge[j]];

			if ((m_Vertex[temp.vertex1].x == v.x && m_Vertex[temp.vertex1].y == v.y && m_Vertex[temp.vertex1].z == v.z) ||
				(m_Vertex[temp.vertex2].x == v.x && m_Vertex[temp.vertex2].y == v.y && m_Vertex[temp.vertex2].z == v.z))

				t_edge.push_back(temp);
		}
	}

	for (itr1 = t_edge.begin(); itr1 != t_edge.end(); itr1++)  //ɾ���߼����ظ��ı�
	{
		for (itr2 = itr1+1; itr2 != t_edge.end(); )
		{
			if ((itr1->vertex1 == itr2->vertex1 && itr1->vertex2 == itr2->vertex2) ||
				(itr1->vertex1 == itr2->vertex2 && itr1->vertex2 == itr2->vertex1))
				t_edge.erase(itr2);
			else
				itr2++;
		}
	}

	for (itr1 = t_edge.begin(); itr1 != t_edge.end(); itr1++)  //�������ǡ�ƽչ��
	{
		if (itr1->facet != -1 && itr1->rfacet != -1)
		{
			double a1 = 0.0, b1 = 0.0, c1 = 0.0, a2 = 0.0, b2 = 0.0, c2 = 0.0, ddd = 0.0;

			a1 = m_Facet[itr1->facet].nx;
			b1 = m_Facet[itr1->facet].ny;
			c1 = m_Facet[itr1->facet].nz;

			a2 = m_Facet[itr1->rfacet].nx;
			b2 = m_Facet[itr1->rfacet].ny;
			c2 = m_Facet[itr1->rfacet].nz;

			ddd = (a1*a2 + b1*b2 + c1*c2) / sqrt((a1*a1+b1*b1+c1*c1)*(a2*a2+b2*b2+c2*c2));
			
		    e_angle = 180.0 - acos(ddd)*180.0 / 3.1415926;

			k++;
			ave_angle += e_angle;
		}
		else
			continue;
	}

	ave_angle = ave_angle/k;

	return ave_angle;
}


CVect3 C3DRCSDoc::CalculateVertexNormal_DC(Vertex vn)    //����׶��߽��ķ�ʸ��
{
//	ExtractBoundary_DC();

	int m_facetnum;
	int kkv;
	float area=0.0, S=0.0, l[3];

	vector<float> CosV;
	vector<Edge> t_edge;

	Edge temp;

	CosV.clear(); 
	t_edge.clear();

/*	for (int i = 0; i < vn.facet.size(); i++)     //��������ڱߴ���߼�
	{
		for (int j=0; j<3; j++)
		{
			temp = m_Edge[m_Facet[vn.facet[i]].edge[j]];

			if ((m_Vertex[temp.vertex1].x == vn.x && m_Vertex[temp.vertex1].y == vn.y && m_Vertex[temp.vertex1].z == vn.z) ||
				(m_Vertex[temp.vertex2].x == vn.x && m_Vertex[temp.vertex2].y == vn.y && m_Vertex[temp.vertex2].z == vn.z))
			{
				t_edge.push_back(temp);
				break; 
			}
		}
	}

	if (t_edge.size() != vn.facet.size())
	{
		for ()
		{
		}
	}
	else
		return CVect3();
	
*/
	return CVect3();
}


float C3DRCSDoc::CalculateArea_DC(Edge e1, Edge e2, Edge e3)
{
	float l1 = e1.length;
	float l2 = e2.length;
	float l3 = e3.length;
	if(((l1+l2)<=l3)||((l2+l3)<=l1)||((l3+l1)<=l2))
	{
		//		AfxMessageBox( "���ܹ���������!" );
		return -1.0;
	}
	float l=(l1+l2+l3)/2.0;
	return sqrt(l*(l-l1)*(l-l2)*(l-l3));
}


CVect3 C3DRCSDoc::CalculateFacetNormal_DC(Vertex v0, Vertex v1, Vertex v2)
{
	float nx = 0.0, ny = 0.0, nz = 0.0;

	nx = (v1.y-v0.y)*(v2.z-v1.z)-(v1.z-v0.z)*(v2.y-v1.y);
	ny = (v1.z-v0.z)*(v2.x-v1.x)-(v1.x-v0.x)*(v2.z-v1.z);
	nz = (v1.x-v0.x)*(v2.y-v1.y)-(v1.y-v0.y)*(v2.x-v1.x);

	float m = sqrt(nx*nx + ny*ny + nz*nz);

	nx /= m;
	ny /= m; 
	nz /= m;

	return CVect3(nx, ny, nz);
}


float C3DRCSDoc::CalculateLength_DC(Vertex v1, Vertex v2)
{
	float dx = v1.x - v2.x;
	float dy = v1.y - v2.y;
	float dz = v1.z - v2.z;

	return sqrt(dx*dx + dy*dy + dz*dz);
}

/***********************************************************************************
*��������             
*          RotatePoint_DC
*����
*          CVect3 a, CVect3 b, float ang
*˵��
*          �����a����b��ת�Ƕ�ang��ĵ�
*
************************************************************************************/
CVect3 C3DRCSDoc::RotatePoint_DC(CVect3 a, CVect3 b, float ang)
{
	CVect3 t_vect, rpoint;
	float dd, d, sina, cosa;

	t_vect.x = b.x*b.x;
	t_vect.y = b.y*b.y;
	t_vect.z = b.z*b.z;

	dd = b*b;
	d = sqrt(dd);
	sina = sin(ang);
	cosa = cos(ang);

	rpoint.x = (t_vect.x+(t_vect.y+t_vect.z)*cosa)/dd*a.x
		            + (b.x*b.y*(1-cosa)/dd - b.z*sina/d)*a.y
			    	+ (b.x*b.z*(1-cosa)/dd + b.y*sina/d)*a.z;
    rpoint.y = (b.y*b.x*(1-cosa)/dd + b.z*sina/d)*a.x
		            + (t_vect.y+(t_vect.x+t_vect.z)*cosa)/dd*a.y
					+ (b.y*b.z*(1-cosa)/dd - b.z*sina/d)*a.z;
	rpoint.z = (b.z*b.x*(1-cosa)/dd - b.y*sina/d)*a.x
		            + (b.z*b.y*(1-cosa)/dd + b.x*sina/d)*a.y
					+ (t_vect.z+(t_vect.x+t_vect.y)*cosa)/dd*a.z;

	return rpoint;
}


void C3DRCSDoc::ExtractLines_DC()
{
	if (::gbIsGetData)
		return;

	if (::gbDataIsEmpty)
		return;

	Edge edge;
	m_Edge.clear();
	bool m_sameLine = false;
	//	SetZoomAndMid();

	m_LineList = new Line*[HashMaxLish * 6];

	for (int k = 0; k<HashMaxLish * 6; k++)
	{
		m_LineList[k] = NULL;
	}

	for (int i = 0; i < m_FacetNumber; i++)
	{
		int d[3], v[3];
		d[0] = m_Facet[i].vertex[0] + m_Facet[i].vertex[1];
		d[1] = m_Facet[i].vertex[1] + m_Facet[i].vertex[2];
		d[2] = m_Facet[i].vertex[2] + m_Facet[i].vertex[0];
		v[0] = m_Facet[i].vertex[0];
		v[1] = m_Facet[i].vertex[1];
		v[2] = m_Facet[i].vertex[2];

		int dd;
		for (int j = 0; j<3; j++)
		{
			m_sameLine = false;
			dd = *(d + j);
			if (m_LineList[dd] != NULL)
			{
				Line *l = m_LineList[dd];
				while (l != NULL)
				{
					if ((l->vertex1 == v[(j + 1) % 3]) && (l->vertex2 == v[j]))
					{
						m_sameLine = true;
						m_Edge[l->index].rfacet = i;
						*(m_Facet[i].edge + j) = l->index;
						break;
					}
					else
						l = l->next;
				}
			}
			if (!m_sameLine)
			{
				Line *l = new Line;
				l->vertex1 = edge.vertex1 = v[j];
				l->vertex2 = edge.vertex2 = v[(j + 1) % 3];
				edge.facet = i;
				l->next = m_LineList[dd];
				l->index = m_Edge.size();
				m_LineList[dd] = l;
				edge.edgenum = m_Edge.size();
				m_Edge.push_back(edge);
				*(m_Facet[i].edge + j) = m_Edge.size() - 1;     //ȷ����Ƭi�ĵ�j������vector�еĵ����
			}
		}
	}
	m_EdgeNumber = m_Edge.size();
	ClearLinesHash();
	RelatePointsEdgesFacets();
}
