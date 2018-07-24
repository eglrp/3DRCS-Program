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

extern float	m_Prox,m_Proy,m_Proz; //缩放因子

#define BYTE_PER_LINE(w,c)  ((((w)*(c)+31)/32)*4)

#include "time.h"
#include <sys/timeb.h>
//定义了两个宏，用来计算并显示某段计算步骤所经历的时间
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
	str1.Format ("该步骤所用时间：%d 毫秒",timeused);\
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

	m_ModeSurfaceArea=0.0;  //模型表面积
	m_ModeVolume=0.0;       //模型体积

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
 * 函数名称：
 *   ReadFile()
 *
 * 参数:
 *   LPVOID   lp     - 指向线程调用的类
 *
 * 返回值:
 *   UINT            - 暂无用途
 *
 * 说明:
 *       该函数用于执行读取文件的进程。
 * 
 ******************************************************************************************************************************/
UINT C3DRCSDoc::ReadFile(LPVOID lp)
{
	C3DRCSDoc *pThis = (C3DRCSDoc*)lp;   //获取线程指针
	
	::gbIsGetData = true;         //标识为正在读取文件
	::gbDataIsEmpty = true;       //标识为数据为空

	//提示是否成功读取文件
	if(!(pThis->GetFileData(pThis->m_FileName)))
	{
		pThis->m_pFr->m_pCtrlWnd->Clear() ;
		AfxMessageBox("打开文件失败！");
		return 0L;
	}
	pThis->m_pFr->m_pView->SetTime();
	
	return 0L;
}


/******************************************************************************************************************************
 * 
 * 函数名称：
 *   GetFileData()
 *
 * 参数:
 *   CString    filename   - 欲打开文件的路径
 *
 * 返回值:
 *   bool                  - 返回true 表示成功读取文件，否则返回false
 *
 * 说明:
 *       该函数用于判断打开的文件是ASCII文件或二进制文件，并打开ASCII文件，
 * 同时建立面与点的关系。
 * 
 ******************************************************************************************************************************/
bool C3DRCSDoc::GetFileData(CString filename)
{
	this->m_pFr = (CMainFrame*)AfxGetApp()->m_pMainWnd;   //*****获取框架类指针
	this->m_pFr->m_pView->KillTime();
	this->m_pFr->InvalidateAllWnd();    //刷新窗口
	this->m_pFr->m_wndStatusBar.SetWindowText("正在加载模型，请稍候……");
	
	::gbIsGetData=true;
	FILE * FileData = fopen(filename,"r");//打开需检测的文件
	
	/***********************检测文件头***********************/
	CString TemData;       //临时储存实时读取的字符串
	fscanf(FileData,"%s", TemData); 

	if(TemData!="solid")  //首先以读取ASCII码方式读取文件，如果文件第一个字符串不是"solid"，则试着以二进制方式打开文件。
	{
		fclose(FileData);

		if(ReadBitFile(filename))  //以二进制方式打开文件
		{
			::gbIsGetData = ::gbDataIsEmpty = false;
			this->m_pFr->m_Progress.SetPos(100); this->m_pFr->m_Progress.SetPos(0);

			ClearPointsHash(); //清除点Hash表
			ExtractLines();    //根据已读取的面片和空间点，提取并存储网格边。
			AfxMessageBox( "成功建立网格边！" );
			
			//使控制台按键可用。
			::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_ENABLEBTN,0,0);
			::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);	

			//状态栏提示加载完成。
			this->m_pFr->m_wndStatusBar.SetWindowText("模型加载完成！");
			//标题栏提示加载成功。
			AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已加载数据");

			return true;     //读取成功，返回。
		}	
	
		else
		{ 
			if(ReadAsciiPlyFile(filename)) //以读取ASCII码方式读取PLY文件
			{
			::gbIsGetData = ::gbDataIsEmpty = false;
			this->m_pFr->m_Progress.SetPos(100); this->m_pFr->m_Progress.SetPos(0);

			ClearPointsHash(); //清除点Hash表
			ExtractLines();    //根据已读取的面片和空间点，提取并存储网格边。
			AfxMessageBox( "成功建立网格边！" );

			//使控制台按键可用。
			::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_ENABLEBTN,0,0);
			::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);	

			//状态栏提示加载完成。
			this->m_pFr->m_wndStatusBar.SetWindowText("模型加载完成！");
			//标题栏提示加载成功。
			AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已加载数据");

			return true;     //读取成功，返回。
			}
		}
		//文件头错误情况，处理程序
		AfxMessageBox( "This file is not a STL or ply file!" );
		return false;	

	}
	





//////////////////////////////////////*******以下代码为读取ASCII码STL文件*******//////////////////////////////////////	

	Clear();                             //清理 m_Vertex、m_Facet、m_Edge

	Point3D m_Point;
	Vertex vertex;
	Facet m_facet;
	
	
	//获取文件的大小，单位：字节
	WIN32_FIND_DATA fileInfo;
	HANDLE hFind = FindFirstFile(filename ,&fileInfo); 
	m_FileSize = fileInfo.nFileSizeLow; 
	FindClose(hFind);

	//为点Hash表，分配空间
	m_PointList = new Point3D*[HashMaxLish*2];
	
	//点的散列表清零
	for(int i=0;i<HashMaxLish*2;i++)  {m_PointList[i]=NULL;}

	//预定点、边、面集的容量，（不是严格意义上的设定容器大小）
	m_FileSize/=150;              //粗略估计一下三角面片的个数
	m_Vertex.reserve(m_FileSize/2);
	m_Facet.reserve(m_FileSize);
	m_Edge.reserve(m_FileSize*2);

	/***********************读取数据***********************/
	int m_fileMax=0;          //开头读取的第二行的字符串个数，如果当取大于50个时，仍未找到第一个normal，则表示读取失败
	int m_key=-1;             //Hash函数值（即空间点对应的存储位置），初始化为-1；
	bool m_samePoint=false;   //存在相同点标识器
	
	AfxMessageBox( "这是一个ASCII码的STL文件！" );
	
	while (feof(FileData)==NULL)  //循环读取文件内的字符串
	{
		m_samePoint=false;
		if(m_FacetNumber%1024==0)
			this->m_pFr->m_Progress.SetPos(m_FacetNumber*120.0f/m_FileSize);

		
		fscanf(FileData,"%s", TemData); //取得文件数据

		if(TemData!="facet")	//检测矢量数据的头结构和尾结构
		{
			//判断是否是读取到endsolid，如果是，则显示文件已读取完成。关闭文件，并返回。
			if(TemData=="endsolid")
			{
				fclose(FileData);   //关闭文件
				::gbIsGetData =	::gbDataIsEmpty = false;

				ClearPointsHash(); //清除点Hash表

				this->m_pFr->m_Progress.SetPos(100); this->m_pFr->m_Progress.SetPos(0);

				ExtractLines();  //根据已读取的面片和空间点，提取并存储网格边。
				AfxMessageBox( "成功建立网格边！" );

				//状态栏提示加载完成。
				this->m_pFr->m_wndStatusBar.SetWindowText("模型加载完成！");
				//标题栏提示加载成功。
				AfxGetMainWnd()->SetWindowText("三维重建系统（3DRCS）->已加载数据");

				::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_ENABLEBTN,0,0);
				::SendMessage(this->m_pFr->m_pCtrlWnd->m_hWnd,MESSAGE_GETDATAFINISHED,0,0);	
				return true;
			}

			if(m_fileMax>50)
			{
				//如果寻找了50个字符串，还未找到"Facet"，则退出；
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
	
		//读取读取面片法向量数据，（该数据可能有误，因此在读取三个顶点的坐标后，再重新计算面片的法向量）
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

		//三角面片的顶点数据
		int m_index=-1;
		float data;

		for(int k=0;k<3;k++) //读取三个空间点数据
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


			//*****判断是否已存在相同的点，不存在则插入散列表****//

			m_key=CalculatePKey(m_Point);  //计算该空间点的Hash函数值
			m_samePoint=false;       //m_samePoint初始化

			//判断Key值为m_key是否已有点（从散列表中查找一个元素）
			if(m_PointList[m_key]!=NULL) //如果m_key对应的位置已有存储元素
			{
				Point3D *p=m_PointList[m_key];
				while(p!=NULL)
				{
					//链表式散列表查找，在相同Key值的点中，是否存在着相同点
					if((p->x==m_Point.x)&&(p->y==m_Point.y)&&(p->z==m_Point.z))
					{
						//如果找到了相同点，则读取改点的编号，并将m_samePoint置为true
						m_index=p->index;    
						m_samePoint=true;    
						break;
					}
					else
						p=p->next;
				}
			}

			//如果不存在相同点，这将该点存储在点集中，同时插入到散列表中
			//并为三角面片记录三个顶点的索引
			if(!m_samePoint)
			{
				//将空间点的信息存储于点散列表
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

				//获取XYZ的最大最小值
				if (p->x > x_glMax)	{   x_glMax = p->x;	m_XmaxFacet=m_FacetNumber;}
				if (p->x < x_glMin)		x_glMin = p->x;
				if (p->y > y_glMax)	{   y_glMax = p->y; m_YmaxFacet=m_FacetNumber;}
				if (p->y < y_glMin)		y_glMin = p->y;
				if (p->z > z_glMax) 	z_glMax = p->z;
				if (p->z < z_glMin) 	z_glMin = p->z;

				m_Vertex.push_back(vertex);   //将点存储于点集后头
				m_Vertex[m_VertexNumber].facet.push_back(m_FacetNumber);   //将该三角面片的编号 储存于空间点元素的邻近三角面片集中
				m_VertexNumber++;             //空间点数量（编号）加1
			}
			else   //如果已存有该空间点的信息，则将该点序号记录在三角面片的点集号中。
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
		
		//单个三角面片数据读取结束
		fscanf(FileData,"%s ", TemData);
		if(TemData!="endfacet")
		{
			AfxMessageBox( "Mistake DataStruct:Endfacet!");
			fclose(FileData);
			return FALSE;
		}
		
		CalculateFacetNormal(m_facet);//计算面片的法向。

		m_Facet.push_back(m_facet);   //将三角面片元素存储于面集后头
		m_FacetNumber++;        //读取完一组数据，面集数量（编号）记数加一

    } //while (feof(FileData)==NULL)
	
	fclose(FileData);
	AfxMessageBox( "Error while reading this file!" );
 	return false;

}
/******************************************************************************************************************************
 * 
 * 函数名称：
 *   ReadAsciiPlyFile
 *
 * 参数:
 * 返回值:
 *   CString    filename   - 欲打开文件的路径
 *
 *   bool                  - 返回true 表示成功读取文件，否则返回false
 *
 * 说明:
 *       该函数用于读取ASCII码的PLY文件，并建立面与点的关系。
 * 
 ******************************************************************************************************************************/
 BOOL C3DRCSDoc::ReadAsciiPlyFile(CString filename)
{
	Clear();
	
	Point3D m_Point;
	Vertex vertex;
	Facet m_facet;

	//获取文件的大小，单位：字节
	WIN32_FIND_DATA fileInfo;
	HANDLE hFind = FindFirstFile(filename ,&fileInfo); 
	m_FileSize = fileInfo.nFileSizeLow; 
	FindClose(hFind);

	//为点Hash表，分配空间
	m_PointList = new Point3D*[HashMaxLish*2];
	
	//点的散列表清零
	for(int i=0;i<HashMaxLish*2;i++)  {m_PointList[i]=NULL;}
	

	FILE * FileData = fopen(filename,"r");//打开需检测的文件
	
	/***********************读文件头***********************/
	float m_PointNumber = 0;//保存文件给出点的数目
	float m_FaceNumber = 0;//保存文件给出面的数目
	CString TemData; //临时储存实时读取的字符串
	char string[1000];


	fscanf(FileData,"%s", TemData); 
	if(TemData!="ply") return false;
	AfxMessageBox( "这是一个ASCII码的PLY文件！" );

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

	//预定点、边、面集的容量，（不是严格意义上的设定容器大小）
	m_Vertex.reserve(m_FaceNumber/2);
	m_Facet.reserve(m_FaceNumber);
	m_Edge.reserve(m_FaceNumber*2);
			
	/***********************读取数据***********************/
//	int m_fileMax=0;          //开头读取的第二行的字符串个数，如果当取大于50个时，仍未找到第一个normal，则表示读取失败
	int m_key=-1;             //Hash函数值（即空间点对应的存储位置），初始化为-1；
	bool m_samePoint=false;   //存在相同点标识器
	int m_index=-1;
	int sameNumber=0;
	int i;
	for(i=0;i<m_PointNumber;i++)//取得点的数据	
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

		//*****判断是否已存在相同的点，不存在则插入散列表****//
		m_key=CalculatePKey(m_Point);  //计算该空间点的Hash函数值
		m_samePoint=false;       //m_samePoint初始化
	
	
		//判断Key值为m_key是否已有点（从散列表中查找一个元素）
		if(m_PointList[m_key]!=NULL) //如果m_key对应的位置已有存储元素
		{	
			Point3D *p=m_PointList[m_key];
			while(p!=NULL)
			{
				//链表式散列表查找，在相同Key值的点中，是否存在着相同点
				if((p->x==m_Point.x)&&(p->y==m_Point.y)&&(p->z==m_Point.z))
				{
					//如果找到了相同点，则读取改点的编号，并将m_samePoint置为true
					m_index=p->index;    
					m_samePoint=true;    
					break;
					sameNumber++;
				}
				else
					p=p->next;
			}
		}

	
		//如果不存在相同点，这将该点存储在点集中，同时插入到散列表中
		//并为三角面片记录三个顶点的索引
		if(1)
		{	
		//	AfxMessageBox("不存在相同点");
			//将空间点的信息存储于点散列表
			Point3D *p=new Point3D;
			vertex.x=p->x=m_Point.x;
			vertex.y=p->y=m_Point.y;
			vertex.z=p->z=m_Point.z;
			p->index=m_VertexNumber;
			p->next=m_PointList[m_key];
			m_PointList[m_key]=p;

			//获取XYZ的最大最小值
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

			m_Vertex.push_back(vertex);   //将点存储于点集后头
			m_VertexNumber++;             //空间点数量（编号）加1
		}	
				
	}	

/*	CString str1,str2,str3;
	str1.Format("m_VertexNumber:%d",m_VertexNumber);
	str2.Format("m_m_PointNumber:%d",i);
	str3.Format("sameNumber:%d",sameNumber);
	AfxMessageBox(str1);	
	AfxMessageBox(str2);
	AfxMessageBox(str3);*/
	AfxMessageBox("读完点！");

	for(i=0;i<m_FaceNumber;i++)//取得面的数据	
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
		
		CalculateFacetNormal(m_facet);//计算面片的法向。

		m_Facet.push_back(m_facet);   //将三角面片元素存储于面集后头
		m_FacetNumber++;        //读取完一组数据，面集数量（编号）记数加一
		
	}
	_gcvt(m_FacetNumber, 9, string );
	AfxMessageBox(string);
	AfxMessageBox("读完面！");

fclose(FileData);
return true;

}


/******************************************************************************************************************************
 * 
 * 函数名称：
 *   ReadBitFile()
 *
 * 参数:
 *   CString    filename   - 欲打开文件的路径
 *
 * 返回值:
 *   bool                  - 返回true 表示成功读取文件，否则返回false
 *
 * 说明:
 *       该函数用于读取二进制STL文件，并建立面与点的关系。
 * 
 ******************************************************************************************************************************/
BOOL C3DRCSDoc::ReadBitFile(CString filename)
{	
	Clear();                             //清理 m_Vertex、m_Facet、m_Edge

	Point3D m_Point;
	Vertex vertex;
	Facet m_facet;
	
	
	//获取文件的大小，单位：字节
	WIN32_FIND_DATA fileInfo;
	HANDLE hFind = FindFirstFile(filename ,&fileInfo); 
	m_FileSize = fileInfo.nFileSizeLow; 
	FindClose(hFind);

	//为点Hash表，分配空间
	m_PointList = new Point3D*[HashMaxLish*2];
	
	//点的散列表清零
	for(int k=0;k<HashMaxLish*2;k++)  {m_PointList[k]=NULL;}
	
	bool m_samePoint = false;
	int m_FacetNum = 0; 
	float temptValue = 0; 
	int m_index=-1;
	
	CFile readFile( filename, CFile::modeRead | CFile::typeBinary );
	
	char strValueOut[80] = "\0"; 
	readFile.Read( strValueOut, sizeof(strValueOut)); 
	readFile.Read( &m_FacetNum, 4); 

	if(abs(int((m_FileSize-80)/50-m_FacetNum))>10)   //判断文件的大小和文件中记录的三角面片是否相对应
	{
		return false;
	}

	AfxMessageBox("这是一个二进制STL文件！");

	m_Vertex.reserve(m_FacetNum/2);//申请内存空间
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
 * 函数名称：
 *   Clear()               
 *
 * 说明:
 *       该函数用于清空点集、边集和面集。
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::Clear()
{
	//清理点、边、面集
	if(!m_Vertex.empty())	m_Vertex.clear();
	if(!m_Edge.empty())		m_Edge.clear();
	if(!m_Facet.empty())	m_Facet.clear();
	
	if(!m_UpEdge.empty())   m_UpEdge.clear();   //上边缘线
	if(!m_DownEdge.empty()) m_DownEdge.clear(); //上边缘线

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
 * 函数名称：
 *   ClearPointsHash()             
 *
 * 说明:
 *       该函数用于清空空间点散列表。
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
 * 函数名称：
 *   ClearFacetsHash()              
 *
 * 说明:
 *       该函数用于清空三角面片的散列表。
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
 * 函数名称：
 *   ClearLinesHash()              
 *
 * 说明:
 *       该函数用于清空边的散列表。
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
 * 函数名称：
 *   ExtractLines()
 *
 * 说明:
 *       该函数用于将边存于边集，并建立边与其临近点、面的关系。
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
				*(m_Facet[i].edge+j)=m_Edge.size()-1;     //确定面片i的第j条边在vector中的的序号
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
 * 函数名称：
 *   RelatePointsEdgesFacets()               
 *
 * 说明:
 *       该函数用于将边存于边集，并建立边与点、面的关系。
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::RelatePointsEdgesFacets() 
{
		int i,j;
	/*****************************计算边 的长度*****************************/
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


	/*****************标识面的三条边的索引，同时计算面的面积****************/
	m_AverageArea=0.0;
	
	for( i=0; i<m_FacetNumber;i++)
	{    
		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*5/m_FacetNumber+80);

		for(j=0; j<3; j++)//根据边确定面片与相邻面片的位置关系，即面片i的相邻面片序号
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


	/*************************计算顶点的近似法向量**************************/
	
	for( i=0; i<m_VertexNumber; i++)
	{
		if(i%1024==0)
			this->m_pFr->m_Progress.SetPos((i+1)*10/m_VertexNumber+85);

		CalculateVertexNormal(i);
		
	}
}


/******************************************************************************************************************************
 * 
 * 函数名称：
 *   CalculatePKey()
 *
 * 参数:
 *   Point3D mPoint     -顶点数据                
 *
 * 说明:
 *       该函数用于计算各个顶点在散列表中对应的Key。
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
 * 函数名称：
 *   CalculateLength()
 *
 * 参数:
 *   int v1, int v2         -两个顶点的索引               
 *
 * 说明:
 *       该函数根据线段两个端点的坐标计算其长度
 * 
 ******************************************************************************************************************************/
float C3DRCSDoc::CalculateLength(int v1, int v2)
{
	if(v1==v2)
	{
//		AfxMessageBox( "同点，无法计算长度!" );
		return 0.0f;
	}
	float dx=m_Vertex[v2].x-m_Vertex[v1].x;
	float dy=m_Vertex[v2].y-m_Vertex[v1].y;
	float dz=m_Vertex[v2].z-m_Vertex[v1].z;

	return sqrt(dx*dx+dy*dy+dz*dz);
}

/******************************************************************************************************************************
 * 
 * 函数名称：
 *   CalculateLength()
 *
 * 参数:
 *   Vertex v1, int v2         -              
 *
 * 说明:
 *       该函数根据线段两个端点的坐标计算其长度
 * 
 ******************************************************************************************************************************/
float C3DRCSDoc::CalculateLength_PSO(Vertex v1, int v2)
{
	if(v1.x==m_Vertex[v2].x && v1.y==m_Vertex[v2].y && v1.z==m_Vertex[v2].z)
	{
//		AfxMessageBox( "同点，无法计算长度!" );
		return 0.0f;
	}
	float dx=m_Vertex[v2].x-v1.x;
	float dy=m_Vertex[v2].y-v1.y;
	float dz=m_Vertex[v2].z-v1.z;

	return sqrt(dx*dx+dy*dy+dz*dz);
}


/******************************************************************************************************************************
 * 
 * 函数名称：
 *   CalculateArea()
 *
 * 参数:
 *   int a, int b, int c         -三条边的索引              
 *
 * 说明:
 *       根据三角形三条边的长度计算三角形的面积。如果这三条边不能构成三角形，则返回-1。
 * 
 ******************************************************************************************************************************/
float C3DRCSDoc::CalculateArea(int a, int b, int c)
{
	float l1=m_Edge[a].length;
	float l2=m_Edge[b].length;
	float l3=m_Edge[c].length;
	if(((l1+l2)<=l3)||((l2+l3)<=l1)||((l3+l1)<=l2))
	{
//		AfxMessageBox( "不能构成三角形!" );
		return -1.0;
	}
	float l=(l1+l2+l3)/2.0;
	return sqrt(l*(l-l1)*(l-l2)*(l-l3));

}

/******************************************************************************************************************************
 * 
 * 函数名称：
 *   CalculateArea()   
 *
 * 参数:
 *   int a, int b, int c         -三条边的索引及边长             
 *
 * 说明:
 *       根据三角形三条边的长度计算三角形的面积。如果这三条边不能构成三角形，则返回-1。
 * 
 ******************************************************************************************************************************/
float C3DRCSDoc::CalculateArea_PSO(int a, double b, double c)
{
	double l1=m_Edge[a].length;
	float l2=b;
	float l3=c;
	if(((l1+l2)<=l3)||((l2+l3)<=l1)||((l3+l1)<=l2))
	{
//		AfxMessageBox( "不能构成三角形!" );
		return -1.0;
	}
	float l=(l1+l2+l3)/2.0;
	return sqrt(l*(l-l1)*(l-l2)*(l-l3));

}


/******************************************************************************************************************************
 * 
 * 函数名称：
 *   CalculateFacetNormal()
 *
 * 参数:
 *   Facet &f         -三条边的索引        
 *
 * 说明:
 *       该函数三角面片的外法向。
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
 * 函数名称：
 *   OnFileSave()                
 *
 * 说明:
 *       该函数用于判断是保存为ASCII文件或二进制文件。
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::OnFileSave() 
{
	if (::gbIsGetData)
	{
		AfxMessageBox( "未加载数据!");
		return;
	}
	
	if (::gbDataIsEmpty)
	{
		AfxMessageBox( "数据为空!" );
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
 * 函数名称：
 *   SaveToASCIIFile()
 *
 * 参数:
 *   CString savepath        -为保存文件路径              
 *
 * 说明:
 *       该函数用于数据保存为ASCII文件。
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
 * 函数名称：
 *   SaveToBinaryFile()
 *
 * 参数:
 *   CString savepath        -为保存文件路径              
 *
 * 说明:
 *       该函数用于数据保存为二进制文件。
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
 * 函数名称：
 *   SaveToASCIIPlyFile()
 *
 * 参数:
 *   CString savepath        -为保存文件路径              
 *
 * 说明:
 *       该函数用于数据保存为ASCII格式的PLY文件。
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
 * 函数名称：
 *   SaveSTL()               
 *
 * 说明:
 *       该函数用于调用保存数据的进程。
 * 
 ******************************************************************************************************************************/
UINT C3DRCSDoc::SaveSTL(LPVOID lp)
{
	C3DRCSDoc *pThis = (C3DRCSDoc*)lp;   //获取线程指针
	
	pThis->OnFileSave();

	return 0L;
}


/******************************************************************************************************************************
 * 
 * 函数名称：
 *   Optimize()              
 *
 * 说明:
 *       该函数用于执行点边面的优化。包括简化、……
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::Optimize()
{
	
}


/******************************************************************************************************************************
 * 
 * 函数名称：
 *   CalculateValue_Adaptive()  By BaiYang on April 25,2014            
 *
 * 说明:
 *       该函数用于计算每个三角面片的平坦度,该算法基于自适应折叠简化算法
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
		CalculateFacetValue_Adaptive(i);//调用自适应算法，返回体积误差与二面角角度误差的无因次化乘积
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
	//结果测试
	FILE *fp;
	fp=fopen("C:\\Users\\baiyang\\Desktop\\cons.dat","w");
	for (i=0;i<facetsize;i++)
		fprintf(fp,"%4d\tcos:%.5f\tvolume:%.10f\tvalue:%.10f\n",i,m_Facet[i].cosOfangle,m_Facet[i].volume,m_Facet[i].value);
	fclose(fp);
	
	//结果测试
	
	m_bCalculateFacetValue=true;
	
	  */
	float value;
	long key;
	long facetsize=m_Facet.size();
	if(!m_bCalculateFacetValue)             //判断是否已经计算过平坦度
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
 * 函数名称：
 *   CalculateValue()              
 *
 * 说明:
 *       该函数用于计算每个三角面片的平坦度
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
 * 函数名称：
 *   CalculateValue_PSO()  By BaiYang on March 24,2014            
 *
 * 说明:
 *       该函数用于计算每个三角面片的平坦度,该算法基于PSO智能寻优算法
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
 * 函数名称：
 *   ReductFacetsHash_Adaptive()  Based on WZF
 *   
 *
 * 说明:
 *       该函数运用三角形折叠方法进行模型简化：
 *            1、根据三角面片的中心作为新点的坐标；
 *            2、该三角面片三个顶点的相邻三角形中，与该三角形共点的顶点坐标改为新点坐标；
 *            3、重新计算这些三角面片的数据（三角面片三条边的长度、三角面片的外法向）
 *            4、判断改动后这些三角面片是否存在缺陷，或产生较大误差。如果是，则进行回滚操作，否则，继续。
 *            5、该三角面片三个顶点的相邻三角形中，将以这三个顶点为端点的边，对应端点改为新点。
 *            6、在新的三角网格中，建立点边面三者直接的相邻关系。重新计算这些三角面片顶点的近似法向量，重新计算这些三角形的平坦指数
 *            7、删除该三角形及其直接相邻的三个三角面片，即将其标志设置为false。
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::ReductFacetsHash_Adaptive()
{
	int F[3];      //记录与该三角面片直接相邻的三个三角面片的索引
	int m_facetnum;//记录该点（该三角面片的三个顶点之一）相邻三角面片的个数
	int mF;        //记录该点（该三角面片的三个顶点之一）某一相邻三角面片的索引数
	Vertex vertex; //新点的临时对象
	vector<Facet> m_bF; //存放数据被更改的相邻三角面片，用于回滚 
	bool m_undo=false;
	float nx,ny,nz;                
	float nprocess=0.0f;
	float cosOfangle;
	double zoom=0.0;
	long m_facetsize=m_Facet.size();
	int i,j,k,kk,m_b,m_bbv;

	for(i=0; i<HashMaxLish*4;i++)    //该for循环至函数结束
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
		}     //将不符合删减要求的三角形从面表中删除
		
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
		}                  //将将含有边缘点的三角形从面表中删除

		//判断该三角形是否与其两个直接相邻三角形形成闭环
		long samevertex=JudgeFacetsInLoop(m_index, F[0], F[1], F[2]);   //返回值不为-1表示存在与m_Facet[m_index]不共点的相邻三角形
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
			//判断该三角形是否与三角形闭环直接相邻
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
		
		m_Vertex.push_back(vertex);    //将新点压入点集尾部
		
		//将以m_Facet[m_indexi]三个端点为端点不为待删的相邻三角面片添加为新点的相邻三角面片，用新点置换三个m_Facet[m_index]端点
		for(k=0; k<3; k++)
		{
			m_facetnum=m_Vertex[m_Facet[m_index].vertex[k]].facet.size();  //获取m_Facet[m_index]面片的第k个端点的邻面总数
			for(j=0; j<m_facetnum; j++)
			{
				mF=m_Vertex[m_Facet[m_index].vertex[k]].facet[j];  //m_Facet[m_index]面片的第k个端点的第j个邻面的索引mF
				if((mF!=m_index)&&(mF!=F[0])&&(mF!=F[1])&&(mF!=F[2])) //确保索引为mF的面片不为m_Facet[m_index]面片及其直接邻面
				{
					m_bF.push_back(m_Facet[mF]);
					//判断m_Facet[m_index]面片的第k个端点，是作为索引为mF的面片第几个端点

					for( kk=0; kk<3; kk++)
						if(m_Facet[m_index].vertex[k]==m_Facet[mF].vertex[kk])	//如果是m_Facet[m_index]面片的第k个端点正是mF面片的第kk个端点
							break;

					nx=m_Facet[mF].nx;  ny=m_Facet[mF].ny;  nz=m_Facet[mF].nz;    
			
		            
					m_Facet[mF].vertex[kk]=m_Vertex.size()-1;    //则将mF面片的第kk个端点改为新点，即点集最后一个点
					//这意味着mF面片的第（kk+1）%3条边，不用改动。
						
						//同时，将mF面片中以第kk个端点为端点的两条边（即第kk、第(kk+2)%3条边）的端点，也改成新点
					if(m_Edge[m_Facet[mF].edge[kk]].facet==mF)   //判断mF第kk、第(kk+2)%3条边是属于mF，还是属于mF的直接相邻三角形
						m_Edge[m_Facet[mF].edge[kk]].vertex1=m_Vertex.size()-1; //属于mF，则将其起点改为新点
					else
						m_Edge[m_Facet[mF].edge[kk]].vertex2=m_Vertex.size()-1; //否则，将其终点改为新点
						
					if(m_Edge[m_Facet[mF].edge[(kk+2)%3]].facet==mF)
						m_Edge[m_Facet[mF].edge[(kk+2)%3]].vertex2=m_Vertex.size()-1;
					else
						m_Edge[m_Facet[mF].edge[(kk+2)%3]].vertex1=m_Vertex.size()-1;
					
					
					//计算这些以m_Facet[m_index]的三个端点为端点的三角形法向量、边长和面积，并将这些三角形添加为新点的相邻三角形

					
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

		//按照信息被修改的次序，回滚该三角面片相邻三角面片的信息
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
			m_Vertex.pop_back(); //将新点从点集中弹出
			m_Facet[m_index].order=-1;
			Face *f=m_FacetList[i];
			m_FacetList[i]=f->next;
			delete f;
			i--;
			continue;
		}
		
		//创建边
		CreateEdges(F[0],F[1],F[2],m_index);

		CalculateVertexNormal(m_Vertex.size()-1);

		int Pfacetnum=m_Vertex[m_Vertex.size()-1].facet.size();

		//重新计算边界环上各个端点的法向量
		for(int j=0; j<Pfacetnum; j++)
		{
			for( k=0; k<3; k++)
			{
				int km=m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[k];
				if(km==m_Vertex.size()-1) //j片面端点k不为新点
					break;
			}
			CalculateVertexNormal(m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[(k+1)%3]);
		}

		//重新计算数据顶点更改后各个受影响三角面片的平坦指数。考虑边界环以外三角面片平坦值变化的情况。
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
			mfac=m_Vertex[km].facet.size(); //该边界环上为j片面端点k的相连面片数量
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
		
		//删除元素
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
 * 函数名称：
 *   ReductFacetsHash()
 *   
 *
 * 说明:
 *       该函数运用三角形折叠方法进行模型简化：
 *            1、根据三角面片的中心作为新点的坐标；
 *            2、该三角面片三个顶点的相邻三角形中，与该三角形共点的顶点坐标改为新点坐标；
 *            3、重新计算这些三角面片的数据（三角面片三条边的长度、三角面片的外法向）
 *            4、判断改动后这些三角面片是否存在缺陷，或产生较大误差。如果是，则进行回滚操作，否则，继续。
 *            5、该三角面片三个顶点的相邻三角形中，将以这三个顶点为端点的边，对应端点改为新点。
 *            6、在新的三角网格中，建立点边面三者直接的相邻关系。重新计算这些三角面片顶点的近似法向量，重新计算这些三角形的平坦指数
 *            7、删除该三角形及其直接相邻的三个三角面片，即将其标志设置为false。
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::ReductFacetsHash()
{
	int F[3];      //记录以该三角面片直接相邻的三个三角面片的索引
	int m_facetnum;//记录该点（该三角面片的三个顶点之一）相邻三角面片的个数
	int mF;        //记录该点（该三角面片的三个顶点之一）某一相邻三角面片的索引数
	
	Vertex vertex; //新点的临时对象
	vector<Facet> m_bF; //存放数据被更改的相邻三角面片，用于回滚
	bool m_undo=false;
	float nx,ny,nz;
	float nprocess=0.0f;
	float cosOfangle, pingzhi;
	double zoom=0.0;
	long m_facetsize=m_Facet.size();
	int i,j,k,m_bbv;
	for(i=0; i<HashMaxLish*4;i++)   //面表
	{
		if(m_FacetList[i]==NULL)
			continue;
	
		nprocess=m_ReduceNum/(float)m_facetsize;
		if(m_ReduceNum%128==0)
		{
			this->m_pFr->m_Progress.SetPos(nprocess*90+10);
		}

		if(nprocess>=m_ReduceRateEdit)
			break;                           //若达到删减率，结束
		
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

		//判断该三角形是否与其两个直接相邻三角形形成闭环
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
			//判断该三角形是否与三角形闭环直接相邻
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
		
		m_Vertex.push_back(vertex);    //将新点压入点集尾部
		
		//将以m_Facet[m_index]三个端点为端点不为待删的相邻三角面片添加为新点的相邻三角面片，用新点置换三个m_Facet[m_index]端点
		for(k=0; k<3; k++)
		{
			m_facetnum=m_Vertex[m_Facet[m_index].vertex[k]].facet.size();  //获取m_Facet[m_index]面片的第k个端点的邻面总数
			for(j=0; j<m_facetnum; j++)
			{  int kk;
				mF=m_Vertex[m_Facet[m_index].vertex[k]].facet[j];  //m_Facet[m_index]面片的第k个端点的第j个邻面的索引mF
				if((mF!=m_index)&&(mF!=F[0])&&(mF!=F[1])&&(mF!=F[2])) //确保索引为mF的面片不为m_Facet[m_index]面片及其直接邻面
				{
					m_bF.push_back(m_Facet[mF]);
					//判断m_Facet[m_index]面片的第k个端点，是作为索引为mF的面片第几个端点

					for(kk=0; kk<3; kk++)
						if(m_Facet[m_index].vertex[k]==m_Facet[mF].vertex[kk])	//如果是m_Facet[m_index]面片的第k个端点正是mF面片的第kk个端点
							break;
					
					m_Facet[mF].vertex[kk]=m_Vertex.size()-1;    //则将mF面片的第kk个端点改为新点，即点集最后一个点
					//这意味着mF面片的第（kk+1）%3条边，不用改动。
						
						//同时，将mF面片中以第kk个端点为端点的两条边（即第kk、第(kk+2)%3条边）的端点，也改成新点
					if(m_Edge[m_Facet[mF].edge[kk]].facet==mF)   //判断mF第kk、第(kk+2)%3条边是属于mF，还是属于mF的直接相邻三角形
						m_Edge[m_Facet[mF].edge[kk]].vertex1=m_Vertex.size()-1; //属于mF，则将其起点改为新点
					else
						m_Edge[m_Facet[mF].edge[kk]].vertex2=m_Vertex.size()-1; //否则，将其终点改为新点
						
					if(m_Edge[m_Facet[mF].edge[(kk+2)%3]].facet==mF)
						m_Edge[m_Facet[mF].edge[(kk+2)%3]].vertex2=m_Vertex.size()-1;
					else
						m_Edge[m_Facet[mF].edge[(kk+2)%3]].vertex1=m_Vertex.size()-1;
					
					pingzhi=(m_Edge[m_Facet[mF].edge[kk]].length+m_Edge[m_Facet[mF].edge[(kk+2)%3]].length)/m_Edge[m_Facet[mF].edge[(kk+1)%3]].length;
					//计算这些以m_Facet[m_index]的三个端点为端点的三角形法向量、边长和面积，并将这些三角形添加为新点的相邻三角形
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
		
		//按照信息被修改的次序，回滚该三角面片相邻三角面片的信息
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
			m_Vertex.pop_back(); //将新点从点集中弹出
			m_Facet[m_index].order=-1;
			Face *f=m_FacetList[i];
			m_FacetList[i]=f->next;
			delete f;
			i--;
			continue;
		}
		
		//创建边
		CreateEdges(F[0],F[1],F[2],m_index);

		CalculateVertexNormal(m_Vertex.size()-1);

		int Pfacetnum=m_Vertex[m_Vertex.size()-1].facet.size();

		//重新计算边界环上各个端点的法向量
		for(j=0; j<Pfacetnum; j++)
		{
			for( k=0; k<3; k++)
			{
				int km=m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[k];
				if(km==m_Vertex.size()-1) //j片面端点k不为新点
					break;
			}
			CalculateVertexNormal(m_Facet[m_Vertex[m_Vertex.size()-1].facet[j]].vertex[(k+1)%3]);
		}

		//重新计算数据顶点更改后各个受影响三角面片的平坦指数。考虑边界环以外三角面片平坦值变化的情况。
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
			mfac=m_Vertex[km].facet.size(); //该边界环上为j片面端点k的相连面片数量
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
		
		//删除元素
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
 * 函数名称：
 *   DeleteFacet()
 *   DeleteFacetHash()   
 *
 * 参数:
 *   int f0, int f1, int f2, int i     -分别代表该三角面片及其直接相邻的三个三角面片
 *
 * 说明:
 *       该函数删除相关三角面片、边及顶点，即将其标志置为false。
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::DeleteFacetHash(int f0, int f1, int f2, int i)
{
	int F[3]={f0,f1,f2};
	//删除m_Facet[i]的三个共边三角面片
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
	
	//删除m_Facet[i]三角面片，及其顶点
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
 * 函数名称：
 *   CreateEdges()   
 *
 * 参数:
 *   int f0, int f1, int f2, int i     -分别代表该三角面片及其直接相邻的三个三角面片
 *
 * 说明:
 *       该函数用于创建新点到与之对应边界点的边，同时建立删除的该三角面片的直接相邻三角面片两侧三角面片的相邻关系。
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
				(m_Facet[f[j]].edge[m]==m_Facet[i].edge[2]))//如果三个相邻三角形的某一条边是该三角形的边
			{
				//创建f[0]\f[1]\f[2]另一端点与新点之间的边
				edge.vertex1=m_Vertex.size()-1;
				edge.vertex2=m_Facet[f[j]].vertex[(m+2)%3];
				edge.facet=m_Facet[f[j]].aFacet[(m+2)%3];
				edge.rfacet=m_Facet[f[j]].aFacet[(m+1)%3];
				edge.length=CalculateLength(edge.vertex1,edge.vertex2);
				
				/***********************删除相对端点的其中一个相邻面，即f[j]***********************/
				for(int k=0; k<m_Vertex[m_Facet[f[j]].vertex[(m+2)%3]].facet.size();k++)
				{
					if(m_Vertex[m_Facet[f[j]].vertex[(m+2)%3]].facet[k]==f[j])
					{
						vector<int>::iterator itef = m_Vertex[m_Facet[f[j]].vertex[(m+2)%3]].facet.begin()+k;
						m_Vertex[m_Facet[f[j]].vertex[(m+2)%3]].facet.erase(itef);
						break;
					}
				}
				
				/****************************向新点的相邻三角面片添加边****************************/
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

		/*********************************建立两个三角面片的相邻关系*********************************/
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
 * 函数名称：
 *   ReduceMesh()
 *
 * 参数:
 *   无              - 指向线程调用的类
 *
 * 返回值:
 *   void            - 暂无用途
 *
 * 说明:
 *       该函数用于调用执行模型简化的。
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
 * 函数名称：
 *   ReduceMesh2()   By BaiYang on March 5,2014
 *
 * 参数:
 *   无              - 指向线程调用的类
 *
 * 返回值:
 *   void            - 暂无用途
 *
 * 说明:
 *       该函数用于调用执行模型简化的。
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
//---------------------------------1、计算边长
	
	for (i=0;i<m_Edge.size();i++)
	{
		m_Edge[i].length=CalculateLength(m_Edge[i].vertex1,m_Edge[i].vertex2);
	}
//---------------------------------2、计算面积
	for (i=0;i<m_Facet.size();i++)
	{
		m_Facet[i].area=CalculateArea(m_Facet[i].edge[0],m_Facet[i].edge[1],m_Facet[i].edge[2]);
	}
//---------------------------------3、计算正则性
	
	for (i=0;i<m_Facet.size();i++)
	{
		temp1=4*double(sqrt((double)3))*m_Facet[i].area/(m_Edge[m_Facet[i].edge[0]].length*m_Edge[m_Facet[i].edge[0]].length
			                            +m_Edge[m_Facet[i].edge[1]].length*m_Edge[m_Facet[i].edge[1]].length
										+m_Edge[m_Facet[i].edge[2]].length*m_Edge[m_Facet[i].edge[2]].length);
		m_Facet[i].StandardT=temp1;
	}
	//输出
	FILE *fp;
	fp=fopen("C:\\Users\\baiyang\\Desktop\\StandradData.txt","w");
// 	fprintf(fp,"共有%d个面片\n",m_Facet.size());
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
	//清理上下轮廓数据
	m_UpEdge.clear(); 
	m_DownEdge.clear();

	if(!TrackContour())   //轮廓跟踪 排序
		return;

	Edge temptE;
	int temptI, siz,i;

	//判断模型三角面片法向量指向模型内部还是外面
	int reverseornot;
	if((m_Facet[m_XmaxFacet].nx>0)&&(m_Facet[m_YmaxFacet].ny>0))
		reverseornot=IDYES; 
	else
	{
		if((m_Facet[m_XmaxFacet].nx<0)&&(m_Facet[m_YmaxFacet].ny<0))
			reverseornot=IDNO;
		else
			reverseornot=AfxMessageBox("  未能确认模型的三角面片法向指向，请确定：\n指向模型外侧，请按“是”；\n指向模型内部，请按“否”。",MB_YESNOCANCEL|MB_ICONQUESTION);
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
		return 1;            //点在L左边
	if(side<0.0)
		return -1;           //点在L右边
	else 
		return 0;            //点在L上
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
				rrt = 1;             //两条线完全相交
				break;
			}
			if((x1*x2==0)&&(x3*x4==0))
			{
				rrt = 0;             //两条线不完全相交
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
	return -1;              //如果L与边集重合，返回该边集序号，不重合返回-1
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
	vector<Edge> m_EofPro; //候选的顶点所在的线

	int huai=0;
	float nnum=UorDedge.size();
	Edge tem;
	int i;
	while(UorDedge.size()>0)
	{
		L12=UorDedge[0];   //L12总是多边形的第一条边
		m_EofPro.clear();  //获选顶点集（即获选边集的第二个端点）清空。

		Edge LVL;          //最终选中的获选点（获选边的第二个端点）；
		Facet facet;       //临时面数据，存放待生成面的信息，用于插入新增面集的末尾
		vector<Edge> ::iterator itE;
		int xxx=0;
		bool xxb=false;

		//**************从现有的边中遴选获选边（点）**************//
		for(i=1; i<UorDedge.size(); i++)
		{
			Edge LK=UorDedge[i];
			if(IsOnLeft(m_Vertex[LK.vertex2], L12)<=0)  //如果该点不在线段L12左侧，则该点不予考虑
				continue;
			if(LK.vertex2==L12.vertex1)             //若封闭，结束
				continue;
			Edge L1k, Lk2;    //构造侯选点到L12两个端点的连线
			L1k.vertex1=L12.vertex1;
			L1k.vertex2=LK.vertex2;
			Lk2.vertex1=LK.vertex2;
			Lk2.vertex2=L12.vertex2;
		
			int isx1=IsXother(L1k, i, UorDedge);
			if(1==isx1)
				continue;

			int isx2=IsXother(Lk2, i, UorDedge);
			if(1==isx2) //要是两条连线其中任一条与现有的边相交，则该点不予考虑。
				continue;

			if((isx1==0)&&(isx2==0) )			
			{
				int inie1=IsInEdge(L1k, UorDedge);
				int inie2=IsInEdge(Lk2, UorDedge);
				if((inie1!=-1)&&(inie2!=-1))   //若两条线都不与现有边完全重合，存入边集
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
			m_EofPro.push_back(LK);  //将符合上述条件的获选点存入获选点集（边集）中。
		}

		if(m_EofPro.size()==0)
		{
			AfxMessageBox("0");
			break;
		}
		
		//从获选点集中遴选出合适的点，作为最终选定点。
		bool mIn=false,mHad=false,had=false;
		double m_r=1.0e36,r=1.0e36;
		for( i=0; i<m_EofPro.size(); i++)
		{
			Edge L10,L02;            //L10,L02分别代表最终选点与L12两个端点的连线
			L10.vertex1 = L12.vertex1;
			L10.vertex2 = m_EofPro[i].vertex2;
			L02.vertex1 = m_EofPro[i].vertex2;
			L02.vertex2 = L12.vertex2;
			mIn=false;


           //保证没有候选点在三角形区域内


			for(int k=0; k<m_EofPro.size(); k++)
			{
				
				if(i!=k)
				if((IsOnLeft(m_Vertex[m_EofPro[k].vertex2], L10)<=0)&&(IsOnLeft(m_Vertex[m_EofPro[k].vertex2], L02)<=0))
				{
					mIn=true;//存在外接圆更小的候选点
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

		if(false==mHad)//未能从获选点中找到一个顶点，使其与L12构成的外接圆内未包含其他获选点。则将该顶点指定为第一个点。
			LVL=m_EofPro[xxx];
			
		if((false==mHad)&&(false==xxb))
		{
			AfxMessageBox("wu");
		}


		Edge L10,L02;              //L10,L02分别代表最终选点与L12两个端点的连线
		L10.vertex1 = L12.vertex1;
		L10.vertex2 = LVL.vertex2;
		L02.vertex1 = LVL.vertex2;
		L02.vertex2 = L12.vertex2;

		//存储三角面片
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


		//判断两连线与
		int NL10=IsInEdge(L10, UorDedge);  
		int NL02=IsInEdge(L02, UorDedge);

		if((-1==NL10)&&(-1==NL02))  //L10、L02 都不是边界线
		{
			Edge newedge;
			newedge.vertex1=L12.vertex1;
			newedge.vertex2=LVL.vertex2;
			UorDedge[0].vertex1=LVL.vertex2;
			UorDedge.push_back(newedge);
		}
		else
		{
			if((-1!=NL10)&&(-1!=NL02))  //L10、L02 都是边界线。
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
				if(-1==NL10)  //NL02是边界线
				{
					itE=UorDedge.begin();
					UorDedge[0].vertex2=LVL.vertex2;
					UorDedge.erase(itE+NL02); 
				}
				else  //NL10是边界线
				{
					itE=UorDedge.begin();
					UorDedge[0].vertex1=LVL.vertex2;
					UorDedge.erase(itE+NL10);
				}
			}  //Lo1、L2o 都是边界线。else
		}  //Lo1、L2o 都不是边界线 else

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
 * 函数名称：
 *   OpenPics()
 *
 * 参数:
 *   无                - 暂无
 *
 * 返回值:
 *   void              - 暂无用途
 *
 * 说明:
 *       该函数用于打开系列BMP图像。
 * 
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::OpenPics()
{
	this->m_pFr = (CMainFrame*)AfxGetApp()->m_pMainWnd;   //*****获取框架类指针

	static char mFilter[]="BMP图像文件(*.bmp)|*.bmp|PCG图像文件(*.pcg)|*.pcg|所有文件All Files(*.*)|*.*||";
		
	//文件打开对话框
	CFileDialog PicOpenDlg(TRUE,_T("bmp"),_T("*.bmp"),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,mFilter);
	int structsize=88; 
	DWORD dwVersion,dwWindowsMajorVersion,dwWindowsMinorVersion; 
	//检测目前的操作系统，GetVersion具体用法详见MSDN 
	dwVersion = GetVersion(); 
	dwWindowsMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion))); 
	dwWindowsMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion))); 
	
	// 如果运行的操作系统是Windows NT/2000 
	if (dwVersion < 0x80000000)  
		structsize =88;//显示新的文件对话框 
	else 
		//运行的操作系统Windows 95/98  
		structsize =76;//显示老的文件对话框 
	PicOpenDlg.m_ofn.lStructSize=structsize;

	char *fileNamesBuf = new char[512 * _MAX_PATH]; 
	fileNamesBuf[0] = '\0'; 
	DWORD maxBytes = 512 * _MAX_PATH; 
	PicOpenDlg.m_ofn.lpstrFile = fileNamesBuf; 
	PicOpenDlg.m_ofn.nMaxFile = maxBytes; 
	
	if (IDOK != PicOpenDlg.DoModal())
		return;

	this->Clear();   //清除之前的数据

	this->m_pFr->m_pCtrlWnd->DisableControlWhileConstruct();
	this->m_pFr->m_wndStatusBar.SetWindowText("正在图片加载，请稍候……");

	
	POSITION pos=PicOpenDlg.GetStartPosition();
	m_FilePathArray.RemoveAll(); //清空欲存放图像路径系列的链表
	while(pos)
	{
		m_FilePathArray.Add(PicOpenDlg.GetNextPathName(pos));
	}
		
	m_nFileNum=int(m_FilePathArray.GetSize());

	SortFileArry();            /*对文件路径系列进行排序*/



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




	/*****************获取图片信息*************/
	CFile FileInfo;
	FileInfo.Open(m_FilePathArray.GetAt(0), CFile::modeRead);
	GetFileInfo(&FileInfo);  //调用函数，获取图片信息//
	FileInfo.Close();

	m_Pixels = new BYTE[(m_nFileNum+4)*m_nHeight*m_nWidth];  //为像素点集分配内存

	SetStartandEnd();     /*添加上下各点集*/

	/*********读取图像文件中的点集*******/
	CString m_Path;	
	for(int i=0; i<m_nFileNum; i++)
	{
		CFile file;
		m_Path=m_FilePathArray.GetAt(i);
		file.Open(m_Path, CFile::modeRead);
		if(!ReadPicFiles(&file,i+2))
		{
			AfxMessageBox( "打不开部分文件!");
			file.Close();
			return;
		}
		file.Close();
		this->m_pFr->m_Progress.SetPos(i*100/m_nFileNum);
	}

	gbIsGetData=false;
	gbDataIsEmpty=false;

	//this->m_pFr->m_pCtrlWnd->FillListBox(); //将图像文件名显示于列表框中
	this->m_pFr->m_pProView->m_bShowBmp=true;
	this->m_pFr->m_pCtrlWnd->m_bGetPics=true;
	this->m_pFr->InvalidateAllWnd();

	this->m_pFr->m_Progress.SetPos(0);
	this->m_pFr->m_wndStatusBar.SetWindowText("图片加载完成，请设置重建参数并重建模型!");
	this->m_pFr->m_pCtrlWnd->EnablControlWhileConstruct();

	return;	

}

/******************************************************************************************************************************
 * 
 * 函数名称：
 *   GetFileInfo()
 *
 * 参数:
 *   CFile *file       - 文件对象
 *
 * 返回值:
 *   void              - 暂无用途
 *
 * 说明:
 *       获取图像的信息：高度、宽度、调色板
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
		AfxMessageBox( "非位图文件!" );
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
 * 函数名称：
 *   ReadPicFiles()
 *
 * 参数:
 *   CFile *file       - 图像文件
 *   int    n          - 图像文件的序号
 *
 * 返回值:
 *   BOOL              - 读取成功返回 TRUE，否则 FALSE
 *
 * 说明:
 *       该函数被循环调用，用于读取图像文件中的像素信息。
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
		AfxMessageBox( "非位图文件!");
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
		AfxMessageBox( "读位图文件出错!" );
		return FALSE;
	}
	
	FlpBitmap -=( sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*PaletteSize);
	delete [] FlpBitmap;
	return TRUE;
}

//MarchingTetrahedra
UINT C3DRCSDoc::MarchingTetrahedra(LPVOID lp)
{
	C3DRCSDoc *pThis = (C3DRCSDoc*)lp;   //获取线程指针
	pThis->m_pFr->m_pCtrlWnd->DisableControlWhileConstruct();
	CString str="正在进行模型重建，请稍候……";
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
	str="模型重建完成！";
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
 * 函数名称：
 *   SortFileArry()
 *
 * 参数:
 *   无                - 暂无
 *
 * 返回值:
 *   void              - 暂无用途
 *
 * 说明:
 *       对读取的图片文件路径系列排序。按文件名中的数字从小到大。
 * 
 * 
 ******************************************************************************************************************************/
void C3DRCSDoc::SortFileArry()
{
	int m_place, n, kkk;
	CString tempt="", tempti="", str;	
	vector<int> flienamenum;   //编号链表，用于存放编号

	tempt=m_FilePathArray[0];  //初始化tempt，将其初始化为最先获取的路径

	m_place=tempt.ReverseFind('\\');   //获取文件路径当中最后一个'\'的位置

	/*************获取并存储文件名中包含的编号***************/
	for(int i=0 ; i<m_nFileNum; i++)
	{
		//按文件路径链表的顺序，获取文件名中包含的编号
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
		flienamenum.push_back(n);   //将编号存入编号链表
	}

	/*************按编号链表中的元素的大小进行排序***************/
//	int kkk,i,j;
	for(int i=0 ; i<m_nFileNum; i++)
	{
		for(int j=i+1; j<m_nFileNum;j++)
		{
			if(flienamenum[i]>flienamenum[j])
			{
				//交换文件路径链表中两个元素的位置
				kkk=flienamenum[i];
				flienamenum[i]=flienamenum[j];
				flienamenum[j]=kkk;

				//同时，应交互编号链表中两个元素的位置
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
	C3DRCSDoc *pThis = (C3DRCSDoc*)lp;   //获取线程指针
	pThis->m_pFr->m_pCtrlWnd->DisableControlWhileConstruct();
	CString str="正在进行模型重建，请稍候……";
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
	
	GLint index;//根据每个体素个顶点的正负来确定编号编号
	GLint cuts; //为1的位表示该位与表面相交
	//处理每个体素
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

	int SideLength = 1;//定义体元的边长
	
	for(int i=0; i< pThis->m_nFileNum+3; i=i+SideLength){
		for(GLint j=0; j< pThis->m_nHeight-1; j=j+SideLength){
			for(GLint k=0; k< pThis->m_nWidth-1; k=k+SideLength){
				//对于每个体素，计算该定点对应的编号
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
				if ( cuts == 0 ) continue; //没有交点，下一个（体元）
				if ( cuts & 1 ) {//与棱0有交点
					caculatedPosition[0] = pThis->interpolate(pThis->Locate(k, j,i),    
					pThis->Value(i,j,k),      pThis->Locate(k,j,i+SideLength),pThis->Value(i+SideLength,j,k));
				}
				if ( cuts & 2 ) {//与棱1有交点
					caculatedPosition[1] = pThis->interpolate(pThis->Locate(k,j,i+SideLength),     
					pThis->Value(i+SideLength,j,k),    pThis->Locate(k+SideLength,j,i+SideLength),pThis->Value(i+SideLength,j,k+SideLength));
				}
				if ( cuts & 4 ) {//与棱2有交点 
					caculatedPosition[2] = pThis->interpolate(pThis->Locate(k+SideLength,j,i+SideLength),   
					pThis->Value(i+SideLength,j,k+SideLength),  pThis->Locate(k+SideLength,j,i),pThis->Value(i,j,k+SideLength));
				}
				if ( cuts & 8 ) {//与棱3有交点
					caculatedPosition[3] = pThis->interpolate(pThis->Locate(k+SideLength,j,i),     
					pThis->Value(i,j,k+SideLength),    pThis->Locate(k,j,i),pThis->Value(i,j,k));
				}
				if ( cuts & 16 ) {//与棱4有交点
					caculatedPosition[4] = pThis->interpolate(pThis->Locate(k,j+SideLength,i),     
					pThis->Value(i,j+SideLength,k),    pThis->Locate(k,j+SideLength,i+SideLength),pThis->Value(i+SideLength,j+SideLength,k));
				}
				if ( cuts & 32 ) {//与棱5有交点
					caculatedPosition[5] = pThis->interpolate(pThis->Locate(k,j+SideLength,i+SideLength),   
					pThis->Value(i+SideLength,j+SideLength,k),  pThis->Locate(k+SideLength,j+SideLength,i+SideLength),pThis->Value(i+SideLength,j+SideLength,k+SideLength));
				}
				if ( cuts & 64 ) {//与棱6有交点
					caculatedPosition[6] = pThis->interpolate(pThis->Locate(k+SideLength,j+SideLength,i+SideLength), 
					pThis->Value(i+SideLength,j+SideLength,k+SideLength),pThis->Locate(k+SideLength,j+SideLength,i),pThis->Value(i,j+SideLength,k+SideLength));
				}
				if ( cuts & 128 ) {//与棱7有交点
					caculatedPosition[7] = pThis->interpolate(pThis->Locate(k+SideLength,j+SideLength,i),   
					pThis->Value(i,j+SideLength,k+SideLength),  pThis->Locate(k,j+SideLength,i),pThis->Value(i,j+SideLength,k));
				}
				if ( cuts & 256 ) {//与棱8有交点
					caculatedPosition[8] = pThis->interpolate(pThis->Locate(k,j,i),       
					pThis->Value(i,j,k),      pThis->Locate(k,j+SideLength,i),pThis->Value(i,j+SideLength,k));
				}
				if ( cuts & 512 ) {//与棱9有交点
					caculatedPosition[9] = pThis->interpolate(pThis->Locate(k,j,i+SideLength),     
					pThis->Value(i+SideLength,j,k),    pThis->Locate(k,j+SideLength,i+SideLength),pThis->Value(i+SideLength,j+SideLength,k));
				}
				if ( cuts & 1024 ) {//与棱10有交点
					caculatedPosition[10] = pThis->interpolate(pThis->Locate(k+SideLength,j,i+SideLength),  
					pThis->Value(i+SideLength,j,k+SideLength),  pThis->Locate(k+SideLength,j+SideLength,i+SideLength),pThis->Value(i+SideLength,j+SideLength,k+SideLength));
				}
				if ( cuts & 2048 ) {//与棱11有交点
					caculatedPosition[11] = pThis->interpolate(pThis->Locate(k+SideLength,j,i),    
					pThis->Value(i,j,k+SideLength),    pThis->Locate(k+SideLength,j+SideLength,i),pThis->Value(i,j+SideLength,k+SideLength));
				}
				//已经得到了表面与棱的交点，查表，绘制三角形表面
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
	str="模型重建完成！";
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

void C3DRCSDoc::ExtractTriangle(CVect3 a, CVect3 b, CVect3 c)  ///MarchingCubes调用
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
	//*******************移动顶点*******************//  Vi'=Vi+u*Lei;

	long m_Vnum=m_Vertex.size();

	for(nn=0; nn<m_SmoothNum; nn++)
	{
		for(i=0; i<m_Vnum; i++)
		{
			if(i%1024==0)
				this->m_pFr->m_Progress.SetPos((i+1)*100/m_Vnum);
			
			if(m_Vertex[i].bBoundary||!m_Vertex[i].flag)  //左边为真、右边为假，成立一个就停止
				continue;

			m_facetnum=m_Vertex[i].facet.size();//某顶点所属面片的个数
			if(m_facetnum<3)
				continue;

			m_bV=m_Vertex[i];

			//顶点预测
			Pos=CalculatePosWithSOT(m_Vertex[i], i);

			m_Vertex[i].x+=m_SmoothRate*(Pos.x-m_Vertex[i].x);
			m_Vertex[i].y+=m_SmoothRate*(Pos.y-m_Vertex[i].y);
			m_Vertex[i].z+=m_SmoothRate*(Pos.z-m_Vertex[i].z);

			Lei=CalculatMoveVect(m_Vertex[i], i);
			m_Vertex[i].x+=Lei.x*m_SmoothRate*1.5;//(1.0-m_SmoothRate);
			m_Vertex[i].y+=Lei.y*m_SmoothRate*1.5;//(1.0-m_SmoothRate);
			m_Vertex[i].z+=Lei.z*m_SmoothRate*1.5;//(1.0-m_SmoothRate);
			
			
			//*******************调整第一临域三角形的法向、边长、面积*******************//
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

			//********如果顶点移动后模型变形太大，则回滚*********//
			if(m_undo)//////////////////////////////////////////////////////////////////////////回滚出错。
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
			
			
			//*******************调整第一临域顶点法向*******************//
			
			//重新计算边界环上各个端点的法向量
			for(j=0; j<m_facetnum; j++)
			{
				for(k=0; k<3; k++)
					if(m_Facet[m_Vertex[i].facet[j]].vertex[k]==i)
						break;
					
				//如果whichP小于0，或大于等于3，则报警。
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

	//记录所有SOT(second order triargle,SOT)三角面片的序号
	for(j=0; j<m_facetnum; j++)
	{
		for(k=0; k<3; k++)
		{
			int km=m_Facet[vertex.facet[j]].vertex[k];
			if(km!=i)   //j片面端点k不为该点
			{
				int mfac=m_Vertex[km].facet.size(); //该边界环上为j片面端点k的相连面片数量
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
		
		//三角面片重心
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
CVect3 C3DRCSDoc::CalculatMoveVect(Vertex vertex, long i)  //CalculateMoveVect调用
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

	//Laplacian 算子
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


void C3DRCSDoc::CalculateVertexNormal(long i)  //利用面积权值计算点的法向
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
		//点i的第j个所在面的三条边长度

		if((l[0]+l[1]<=l[2])||(l[1]+l[2]<=l[0])||(l[2]+l[0]<=l[1]))
		{
			continue;
		}
			
		CosV[j]=acos((l[(kkv+2)%3]*l[(kkv+2)%3]+l[kkv]*l[kkv]-l[(kkv+1)%3]*l[(kkv+1)%3])/(2.0*l[(kkv+2)%3]*l[kkv]));
		//点i的第j个所在面的边夹角
		//如果CosV[j]小于0，或大于等于Pi，则报警。
		
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

	//平面ax+by+cz+d=0，即nx(x-x0)+ny(y-y0)+nz(z-z0)=0, 确定a,b,c,d
	//a=nx, b=ny, c=nz, d=-nx*x0-ny*y0-nz*z0;

	a=m_Facet[FacetNum].nx;		        b=m_Facet[FacetNum].ny;		        c=m_Facet[FacetNum].nz;

	x0=m_Vertex[m_Facet[FacetNum].vertex[0]].x;    
	y0=m_Vertex[m_Facet[FacetNum].vertex[0]].y; 
	z0=m_Vertex[m_Facet[FacetNum].vertex[0]].z;

	//已知一个平面，其上一点A，平面法线N。平面外一点I，求I点在平面上的投影。 I,即vertex；
	//I在法线N上的投影点I1,     I1 = A + k * N ;     V = I – A;     k = V.x * N.x + V.y * N.y + V.z * N.z;
	//把I在平面上的投影记作Io，则：Io = A + I1I。
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
		for(int k=0; k<3; k++)  //若m_Facet[F[j]]与m_Facet[i]是完全不共点三角形，则将m_Facet[F[j]]的顶点都储存
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
           /／ ＼\
		  ∠＿_＿＼
*/
bool C3DRCSDoc::DeleteFacetInLoop(long sameVertex, long i, long f0, long f1, long f2)
{
	long F[3]={f0, f1, f2};
	vector<long> Facet2;
	vector<long> Edge2; //环两边的两条
	long samepoint=0;   //i,  Facet2[0], Facet2[1] 三个三角形的共同顶点， 及环的中间点。
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
	if(Facet2.size()!=2)     //只保留一个点上有三个三角形的情况
		return false;

	x0=m_Facet[i].nx; y0=m_Facet[i].ny; z0=m_Facet[i].nz;
	x1=m_Facet[Facet2[0]].nx; y1=m_Facet[Facet2[0]].ny; z1=m_Facet[Facet2[0]].nz;
	x2=m_Facet[Facet2[1]].nx; y2=m_Facet[Facet2[1]].ny; z2=m_Facet[Facet2[1]].nz;
	
	value=1.0-(x0*x1+y0*y1+z0*z1+x1*x2+y1*y2+z1*z2+x0*x2+y0*y2+z0*z2)/3.0; //平行
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
	}   //确定中心点

	if(ivertex<0)
		return false;

	for(j=0; j<3; j++)
	{
		if(m_Facet[Facet2[0]].vertex[j]==samepoint)
		{
			Edge2.push_back(m_Facet[Facet2[0]].edge[(j+1)%3]);
			break;
		}
	}     //存储外环上的第一个边

	for(j=0; j<3; j++)
	{
		if(m_Facet[Facet2[1]].vertex[j]==samepoint)
		{
			Edge2.push_back(m_Facet[Facet2[1]].edge[(j+1)%3]);
			break;
		}
	}        //存储外环上的第二个边

	if(Edge2.size()!=2)
		return false;
	if(((m_Edge[Edge2[0]].vertex1!=sameVertex)&&(m_Edge[Edge2[0]].vertex2!=sameVertex))||\
		((m_Edge[Edge2[1]].vertex1!=sameVertex)&&(m_Edge[Edge2[1]].vertex2!=sameVertex)))
		return false;                    //确保存储正确

	m_Facet[i].vertex[ivertex]=sameVertex;
	CalculateFacetNormal(m_Facet[i]);

	
	if((m_Facet[Facet2[0]].edge[0]==Edge2[1])||(m_Facet[Facet2[0]].edge[1]==Edge2[1])||(m_Facet[Facet2[0]].edge[2]==Edge2[1]))
	{
		long tempt=Edge2[1];
		Edge2[1]=Edge2[0];
		Edge2[0]=tempt;
	}       //保证Edge2的两个元素和Facet2的两个元素分别对应

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

	//平面ax+by+cz+d=0，即nx(x-x0)+ny(y-y0)+nz(z-z0)=0, 确定a,b,c,d
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
	F[0]=m_Facet[f].aFacet[0];//面片的三个邻面
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
		m_facetnum=m_Vertex[m_Facet[f].vertex[(k+1)%3]].facet.size();//面片中第(k+1)%3个顶点所在的所有面片的个数
		for(j=0; j<m_facetnum; j++)
		{
			mF=m_Vertex[m_Facet[f].vertex[(k+1)%3]].facet[j];//第j个面片的号
			
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
				m_nearF.push_back(mF);//将第一个面片放入m_nerF中
		
			if((mF!=f)&&(mF!=F[0])&&(mF!=F[1])&&(mF!=F[2])) //确保索引为mF的面片不为m_Facet[m_index]面片及其直接邻面
			{
				m_ProjectPoint=CalculateProjectPoint(m_Vertex[m_Facet[f].vertex[k]], mF);//返回投影点
				vertex.x+=m_ProjectPoint.x*m_Facet[mF].area;//新点的产生
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
			
			if((mF!=f)&&(mF!=F[0])&&(mF!=F[1])&&(mF!=F[2])) //确保索引为mF的面片不为m_Facet[m_index]面片及其直接邻面
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
	
	m_Facet[f].x=vertex.x /= 6.0;//折叠点坐标
	m_Facet[f].y=vertex.y /= 6.0;
	m_Facet[f].z=vertex.z /= 6.0;

	nFsize=m_nearF.size();
	value=CalculateDistanceFormVtoF(vertex, m_nearF[0]);
	float dis;
	
	for(int j=1; j<nFsize; j++)
	{
		dis=CalculateDistanceFormVtoF(vertex, m_nearF[j]);
		if(value<dis)
			value=dis;//最大距离
		
	}

	m_Facet[f].value=value;
	return value;
}

/******************************************************************************************************************************
 * 
 * 函数名称：
 *   CalculateFacetValue_PSO(long f)   By BaiYang on March 24,2014
 *
 * 参数:
 *   f              - 面片索引
 *
 * 返回值:
 *   value            - 最大距离
 *
 * 说明:
 *       该函数用于调用执行模型生成新点，返回最大距离。
 * 
 ******************************************************************************************************************************/

 //-------------------------------------------PSO相关参数的定义，全局变量——By BaiYang on March 25,2014
	const double ParticleV=0.2;       //速度因子
	const int ParticleNum=20;         //粒子数目
	const int Dim=3;                  //参数个数  
	const int IterNum=5;              //迭代次数
	const double Err=0.1;           //收敛误差
	
	const double PI=3.141592653589;	
	const double c1=1.494;            //学习因子
	const double c2=1.494;
	const double w=0.729;             //惯性系数
	const int   alp=1;                //限定因子
	const double RangeF=2.0;          //取值范围因子
	
	double x_range[2],y_range[2],z_range[2];     //最优解的取值范围
	double Max_V[3];                             //最大速度变化范围
	double Particle[ParticleNum][Dim];           //个体集合
	double Particle_Loc_Best[ParticleNum][Dim];  //每个个体局部最优向量
	double Particle_Loc_Fit[ParticleNum];        //个体的局部最优适应度,有局部最优向量计算而来
	double Particle_Glo_Best[Dim];               //全局最优向量
	double GFit;                                 //全局最优适应度,有全局最优向量计算而来  
	double Particle_V[ParticleNum][Dim];         //记录每个个体的当前代速度向量
	double Particle_Fit[ParticleNum];            //记录每个粒子的当前代适应度

	vector<double> EdgeLength_power;   //存边长平方和
	vector<double> Triangel_Area;//存新三角形的面积
//-------------------------------------------PSO中所用的参数定义	
	vector<int> m_nearLE;//存入所环边集；
//-------------------------------------------	
void C3DRCSDoc::Initial_Pso()
{
	int i,j;
	for(i=0; i<ParticleNum; i++)            //随即生成粒子
	{
		Particle[i][0] = x_range[0]+(x_range[1]-x_range[0])*1.0*rand()/RAND_MAX;    //初始化群体_随机值
		Particle[i][1] = y_range[0]+(y_range[1]-y_range[0])*1.0*rand()/RAND_MAX;
		Particle[i][2] = z_range[0]+(z_range[1]-z_range[0])*1.0*rand()/RAND_MAX;
		for(j=0; j<Dim; j++)
			Particle_Loc_Best[i][j] = Particle[i][j];               //将当前最优结果写入局部最优集合

		Particle_V[i][0] = -Max_V[0]+2*Max_V[0]*1.0*rand()/RAND_MAX;  //初始化速度
		Particle_V[i][1] = -Max_V[1]+2*Max_V[1]*1.0*rand()/RAND_MAX;
		Particle_V[i][2] = -Max_V[2]+2*Max_V[2]*1.0*rand()/RAND_MAX;
	}
	for(i=0; i<ParticleNum; i++)            //计算每个粒子的适应度
	{
		
		Particle_Fit[i] = FitnessPSO(Particle[i]);
		Particle_Loc_Fit[i] = Particle_Fit[i];
	
	}
	GFit = Particle_Loc_Fit[0];      //找出全局最优
	j=0;
	for(i=1; i<ParticleNum; i++)
	{
		if(Particle_Loc_Fit[i]<GFit)
		{
			GFit = Particle_Loc_Fit[i];
			j = i;
		}
	}
	for(i=0; i<Dim; i++)             //更新全局最优向量  
	{
		Particle_Glo_Best[i] = Particle_Loc_Best[j][i];
	}
}
	
void C3DRCSDoc::Renew_Particle_Pos()
{
	int i,j;
	for(i=0; i<ParticleNum; i++)            //更新个体位置生成位置
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
	for(i=0; i<ParticleNum; i++)            //计算每个粒子的适应度
	{
		Particle_Fit[i] = FitnessPSO(Particle[i]);
		if(Particle_Fit[i] < Particle_Loc_Fit[i])      //更新个体局部最优值
		{
			Particle_Loc_Fit[i] = Particle_Fit[i];
			for(j=0; j<Dim; j++)       // 更新局部最优向量
			{
				Particle_Loc_Best[i][j] = Particle[i][j];
			}
		}
	}
	for(i=0,j=-1; i<ParticleNum; i++)                   //更新全局变量
	{
		if(Particle_Loc_Fit[i]<GFit)
		{
			GFit = Particle_Loc_Fit[i];
			j = i;
		}
	}
	if(j != -1)
	{
		for(i=0; i<Dim; i++)             //更新全局最优向量  
		{
			Particle_Glo_Best[i] = Particle_Loc_Best[j][i];
		}
	}
	for(i=0; i<ParticleNum; i++)    //更新个体速度
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
	vector<double> EdgeLength_Vertex1; //存新点到边环集中第一个点的距离
	vector<double> EdgeLength_Vertex2; //存新点到边环集中第二个点的距离

	vertex1.x=a[0];
	vertex1.y=a[1];
	vertex1.z=a[2];
	double Temp=0;
	double TriangelStandard=0;   //存三角形规范性
//-----------------------------------------------1.计算点到点的距离
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
//-----------------------------------------------2.计算三角形面片的面积
	for (i=0;i<m_nearLE.size();i++)
	{
		Temp=CalculateArea_PSO(m_nearLE[i],EdgeLength_Vertex1[i],EdgeLength_Vertex2[i]);
		Triangel_Area.push_back(Temp);
	}
//-----------------------------------------------3.计算边长平方和
	for (i=0;i<m_nearLE.size();i++)
	{
		Temp=m_Edge[m_nearLE[i]].length*m_Edge[m_nearLE[i]].length+EdgeLength_Vertex1[i]*EdgeLength_Vertex1[i]+EdgeLength_Vertex2[i]*EdgeLength_Vertex2[i];
		EdgeLength_power.push_back(Temp);
	}

//-----------------------------------------------3.计算边长平方和

	for (i=0;i<m_nearLE.size();i++)
	{
		TriangelStandard+=4*double(sqrt((double)3))*Triangel_Area[i]/(EdgeLength_power[i]);
	}

	return m_nearLE.size()-TriangelStandard;
}

float C3DRCSDoc::CalculateFacetValue_PSO(long f)
{
	//1、确定新的边环；2、确定所有边长；3、确定三角形面积；
	Vertex vertex;
	int m_facetnum, mF;
	float value;
	vector<int> m_nearE;//存入所有边集；
	vector<int> m_nearF;//存入所有邻面集
	value=0.0;
	int k,j,nn,i;
	m_nearE.clear();
	m_nearLE.clear();
	m_nearF.clear();

	bool mhave;
	bool mhaveF;
//------------------------------------------------------1、保存所有的边集------------------------------------------------//
//	FILE *fp;
	vector <int>::iterator Iter;
//	fp=fopen("d:\\result.txt","w");
	
	vertex.x=vertex.y=vertex.z=0.0f;

	for(k=0; k<3; k++)
	{
		m_facetnum=m_Vertex[m_Facet[f].vertex[k%3]].facet.size();//面片中第(k+1)%3个顶点所在的所有面片的个数
	//	fprintf(fp,"\n第%d个顶点有%d个面片\n",k,m_facetnum);
	
		for(j=0; j<m_facetnum; j++)
		{
			mF=m_Vertex[m_Facet[f].vertex[k%3]].facet[j];//第j个面片的号
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
				m_nearF.push_back(mF);//将第一个面片放入m_nerF中


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
 	
//------------------------------------------------------2、保存所有的环边集----------------------------------------------//
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
// 		fprintf(fp,"第%4d个边长为%5.8f,指向的顶点1为%d,顶点2为%d\n",m_nearLE[i],m_Edge[m_nearLE[i]].length,m_Edge[m_nearLE[i]].vertex1,m_Edge[m_nearLE[i]].vertex2);
// 
// 	fclose(fp);

//------------------------------------------------------3、PSO设定新点的位置，求边长，面积----------------------------------------------//
//------------------------------------------------------3.1、确定变量取值范围
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

//------------------------------------------------------3.2、调用PSO
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
	fprintf(fp,"粒子个数:%2d\t",ParticleNum);
	fprintf(fp,"维度为:%2d\t",Dim);
	fprintf(fp,"正则指标和为%.10lf\t", m_nearLE.size()-GFit);
	for (i=0;i<m_nearLE.size();i++)
	{
		fprintf(fp,"%.5lf\t",4*double(sqrt((double)3))*Triangel_Area[i]/(EdgeLength_power[i]));
	}
	fprintf(fp,"\n");
	fclose(fp);

//------------------------------------------------------3.3、返回最优点位置，返回value

	vertex.x=Particle_Glo_Best[0];
	vertex.y=Particle_Glo_Best[1];
	vertex.z=Particle_Glo_Best[2];

	m_Facet[f].x=vertex.x;//折叠点坐标
	m_Facet[f].y=vertex.y;
	m_Facet[f].z=vertex.z;
//------------------------------------------------------3.4、返回value

	value=CalculateDistanceFormVtoF(vertex, m_nearF[0]);
	float dis;
	
	for(int j=1; j<m_nearF.size(); j++)
	{
		dis=CalculateDistanceFormVtoF(vertex, m_nearF[j]);
		if(value<dis)
			value=dis;//最大距离	
	}
//------------------------------------------------------3.5、计算被折叠三角面片的正则性,消灭狭长三角形
// 	double Temp1;
// 	Temp1=4*sqrt(3)*m_Facet[f].area/(m_Edge[m_Facet[f].edge[0]].length*m_Edge[m_Facet[f].edge[0]].length+
// 		                             m_Edge[m_Facet[f].edge[1]].length*m_Edge[m_Facet[f].edge[1]].length+
// 									 m_Edge[m_Facet[f].edge[2]].length*m_Edge[m_Facet[f].edge[2]].length);

	m_Facet[f].value=value;
	return value;

	
//	return 0;
}


//去除外环。
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

	vector<float>  angle;              //孔洞边的夹角
	vector<float>  t_area;             //新增的临时三角形的面积
	vector<CVect3> f_normal;           //新增的临时三角形的法向量
	vector<CVect3> v_normal;           //边界点法矢量
	vector<float>  p_angle;            //边界点投影夹角
	vector<Vertex> s_vertex;
	vector<int> ted;            //边界点与对应两相邻边界点连线的位置关系

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

		//计算每个边界点的孔洞边夹角、临时三角形面积、临时三角形法向量
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

		//计算边界点的法矢量
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

		//边界点投影夹角
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

		//计算平均平展度
		for (int ff = 0; ff < BoundaryVertex.size(); ff++)
		{
			ave_flat += CalculateVertexFlatness_DC(BoundaryVertex[ff]);
		}

		ave_flat /= BoundaryVertex.size();

		//	将平展度小于平均平展度的点的投影角单独存储
		vector<float>  samll_angle;
		samll_angle.clear();

		for (int ss = 0; ss < p_angle.size(); ss++)
		{
//			v_flat = CalculateVertexFlatness_DC(BoundaryVertex[ss]);

			if (BoundaryVertex[ss].facet.size()>3)
				samll_angle.push_back(p_angle[ss]);
		}

		//始终指向最小的元素
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
		//开始填补
		for (int i = 0; i < BoundaryVertex.size(); i++)
		{
			if (p_angle[i] > 180.0 || p_angle[i] < 0.0)
				continue;

//			v_flat = CalculateVertexFlatness_DC(BoundaryVertex[i]);

			if (p_angle[i] == *itr)    //先从平展度小于平均平展度且投影角最小的点开始修补
			{
			    if (p_angle[i] < 90.0 )  //第一种情况
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
				else if (90.0 <= p_angle[i] && p_angle[i] <= 180.0) //第二种情况
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

							a.x = v1.x - v0.x;       //相邻边界边的向量表示
							a.y = v1.y - v0.y;
							a.z = v1.z - v0.z;
							b.x = v2.x - v0.x;
							b.y = v2.y - v0.y;
							b.z = v2.z - v0.z;
							e = (a / sqrt(a * a) + b / sqrt(b * b)) / sqrt((a / sqrt(a * a) + b / sqrt(b * b))*(a / sqrt(a * a) + b / sqrt(b * b)));
						    
							TP_a.x = a.x - (a * v_normal[i]) * v_normal[i].x;       //边界边在切平面上的投影边
							TP_a.y = a.y - (a * v_normal[i]) * v_normal[i].y;
							TP_a.z = a.z - (a * v_normal[i]) * v_normal[i].z;
					        TP_b.x = b.x - (b * v_normal[i]) * v_normal[i].x;
							TP_b.y = b.y - (b * v_normal[i]) * v_normal[i].y;
							TP_b.z = b.z - (b * v_normal[i]) * v_normal[i].z;

					//		e = TP_a / sqrt(TP_a * TP_a) + TP_b / sqrt(TP_b * TP_b);   //边界边在切平面上的投影边的角平分线向量
							
							l = (sqrt(a*a) + sqrt(b*b)) / 2;                //新增点与v0的实际距离

							cosbeta1 = (v_normal[i] * a) / sqrt((a*a)*(v_normal[i] * v_normal[i])); //边界边与点法向夹角的余弦值
							cosbeta2 = (v_normal[i] * b) / sqrt((b*b)*(v_normal[i] * v_normal[i]));
							double ftt = acos(cosbeta1)*180.0/3.1415926;
							double fta = acos(cosbeta2)*180.0/3.1415926;

							k1 = 2.0 * cosbeta1 / sqrt(a*a);  //方向曲率 
							k2 = 2.0 * cosbeta2 / sqrt(b*b);

							sigema = (k1 + k2)*l / 8.0;  //调整角,为正表示新增向量与点法向夹角为正，该点为凹点，应向夹角增大的方向调整；为负表示新增向量与点法向夹角为负，该点为凸点，应向夹角缩小的方向调整
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
/*				else                //第三种情况
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

							TP_a.x = a.x - (a * v_normal[i]) * v_normal[i].x;       //边界边在切平面上的投影边
							TP_a.y = a.y - (a * v_normal[i]) * v_normal[i].y;
							TP_a.z = a.z - (a * v_normal[i]) * v_normal[i].z;
							TP_b.x = b.x - (b * v_normal[i]) * v_normal[i].x;
							TP_b.y = b.y - (b * v_normal[i]) * v_normal[i].y;
							TP_b.z = b.z - (b * v_normal[i]) * v_normal[i].z;

							xcross.x = TP_a.y*TP_b.z - TP_a.z*TP_b.y;
							xcross.y = TP_a.z*TP_b.x - TP_a.x*TP_b.z;
							xcross.z = TP_a.x*TP_b.y - TP_a.y*TP_b.x;

							e1 = RotatePoint_DC(TP_a, xcross, angle[i] * 3.1415926 / 540.0);//两个三等分线的单位向量
							e2 = RotatePoint_DC(TP_a, xcross, angle[i] * 3.1415926 / 270.0);
							e01 = e1 / sqrt(e1*e1);
							e02 = e2 / sqrt(e2*e2);

							cosbeta1 = v_normal[i] * a / sqrt((a*a)*(v_normal[i] * v_normal[i])); //边界边与点法向夹角的余弦值
							cosbeta2 = v_normal[i] * b / sqrt((b*b)*(v_normal[i] * v_normal[i]));

							k1 = 2.0 * cosbeta1 / sqrt(a*a);  //方向曲率
							k2 = 2.0 * cosbeta2 / sqrt(b*b);
							k01 = 0.666666 * k1 + 0.333333 * k2;
							k02 = 0.333333 * k1 + 0.666666 * k2;

							l01 = 0.666666 * sqrt(a*a) + 0.333333 * sqrt(b*b);
							l02 = 0.333333 * sqrt(a*a) + 0.666666 * sqrt(b*b);

							sigema1 = k01*l01 / 2.0;  //调整角
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
*函数名称
*       ExtractBoundary_DC()
*
*说明：基于DC算法提取边界点、线、面
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

	for (int i = 0; i < m_FacetNumber; i++ )  //存储边界点、线、面
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
		(AfxMessageBox(" 无孔洞 "));
		return;
	}
	
	if (m_hole)                //重新排序边界边
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

		for (int k = 0; k < BoundaryEdge.size(); k++)   //保存排序后的边界点。面
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
	
float C3DRCSDoc::CalculateVertexFlatness_DC(Vertex v)   //计算点的平展度
{
	float e_angle = 0.0, ave_angle = 0.0;
	int k = 0;

	vector<Edge> t_edge;
	vector<Edge>::iterator itr1, itr2;

	Edge temp;

	t_edge.clear();

	for (int i = 0; i < v.facet.size(); i++)     //将点的所在边存入边集
	{
		for (int j=0; j<3; j++)
		{
			temp = m_Edge[m_Facet[v.facet[i]].edge[j]];

			if ((m_Vertex[temp.vertex1].x == v.x && m_Vertex[temp.vertex1].y == v.y && m_Vertex[temp.vertex1].z == v.z) ||
				(m_Vertex[temp.vertex2].x == v.x && m_Vertex[temp.vertex2].y == v.y && m_Vertex[temp.vertex2].z == v.z))

				t_edge.push_back(temp);
		}
	}

	for (itr1 = t_edge.begin(); itr1 != t_edge.end(); itr1++)  //删除边集中重复的边
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

	for (itr1 = t_edge.begin(); itr1 != t_edge.end(); itr1++)  //计算二面角、平展度
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


CVect3 C3DRCSDoc::CalculateVertexNormal_DC(Vertex vn)    //计算孔洞边界点的法矢量
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

/*	for (int i = 0; i < vn.facet.size(); i++)     //将点的所在边存入边集
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
		//		AfxMessageBox( "不能构成三角形!" );
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
*函数名称             
*          RotatePoint_DC
*参数
*          CVect3 a, CVect3 b, float ang
*说明
*          计算点a绕轴b旋转角度ang后的点
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
				*(m_Facet[i].edge + j) = m_Edge.size() - 1;     //确定面片i的第j条边在vector中的的序号
			}
		}
	}
	m_EdgeNumber = m_Edge.size();
	ClearLinesHash();
	RelatePointsEdgesFacets();
}
