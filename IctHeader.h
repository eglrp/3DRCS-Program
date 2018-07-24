//-----------------------------------------------------------------------------
//��ҵCT���ݲɼ�/�����ļ�ͷ����(V2.2)
//�����ѧICT�о�����,���ӿƼ��ֹ�˾.                                2005.01.19
//������άCTɨ�����																	2006.04.20

//2007.07.13�޸�����:
//1. ��SpaceBetweenLayer(���CT���)��LayerThicknessOfDR(DR�ֲ���)�ϲ�ΪLayerThickness(�ֲ���)
//2. ����TotalLayers2(�ڶ���DRɨ���ܲ���)
//3. ����FirstSectStartCoordinateOfDR(��һ��DRɨ����ʼ����X1),SecondSectStartCoordinateOfDR(�ڶ���DRɨ����ʼ����X3)

//2007.07.26�޸�����:
//1. �����ؽ�����

//2007.10.04�޸�����:
//1. ��LayerThickness�����޸�ΪSpaceBetweenLayer
//2. ����2��ɨ���ؽ�����: NumberOfGraduationOfCt2,SerialOfGraduationOfCt2
//3. ����3��ɨ����ʼ�ֶȺ�: graduationBase
//3. ����ˮƽ��������̽�������SerialNo1OfMiddleHorizontalDetector/SerialNo2OfMiddleHorizontalDetector
//   ��������У��ʱȷ���м�̽����(������������ͨ��̽����ʱ���1/2��ͬ;
//   ������������ͨ������̽�����м�ʱ,���1/2���1)

//2008.04.24�޸�����:
//	 ICT_HEADER ��ΪICT_HEADER21��ʾ2.1�汾��ʽ, ��ӦDATAFORMAT21, ����������Ϊ WORD
//   ����ICT_HEADER22��ʾ2.2�汾��ʽ, ��ӦDATAFORMAT22, ������������Ϊ DWORD
//   ɨ�����������NumberOfTable��ʾɨ��ʱʹ�õ�ת̨����
//-----------------------------------------------------------------------------
//2008.05.12�޸�����:
//   DATAFORMAT22��1�ֽڶ���
//   ��������Ĭ��8�ֽڶ��룻

#ifndef __ICTHEADER_H
#define __ICTHEADER_H

#ifndef	ULONG
#define	ULONG	unsigned long
#endif

#ifndef	DWORD
#define	DWORD	unsigned long
#endif

#ifndef	WORD
#define	WORD	unsigned short
#endif

#ifndef	BYTE
#define	BYTE	unsigned char
#endif

#define	MainVersion21	2								//�������汾��
#define	SubVersion21	1								//����ΰ汾��
#define	MainVersion22	2								//�������汾��
#define	SubVersion22	2								//����ΰ汾��
#define  BiDirect       1                               //ɨ�跽��˫��
#define  SigDirect      0                               //ɨ�跽�򣺵���

//����̽��������
enum{
	ARCDETECTOR	=	0,										//����̽����
	LINEDETECTOR,											//ֱ��̽����
	FLATDETECTOR											//ƽ��̽����
};

//ɨ�跽ʽ: 0-���ɨ��,1-���ɨ��,2-���ɨ��,3-DRɨ�裬4-�ֲ�ɨ��,5-����ɨ��,6-׶��ɨ��,7-׶������ɨ��
//����ɨ�跽ʽ
enum{
	SCAN1st	=	0,											//���ɨ��
	SCAN2nd,													//���ɨ��
	SCAN3rd,													//���ɨ��
	SCANdr,													//DRɨ��
	SCANlocal,												//�ֲ�ɨ��
	SCANhelix,												//����ɨ��
	SCANcone,												//׶��ɨ��
	SCANconeHelix											//׶������ɨ��
};

//������������
enum{
	CHARDATA	=	1,											//�ַ���
	INTDATA,													//����
	LONGDATA,												//������
	FLOATDATA,												//������
	DOUBLEDATA												//˫������
};

#define	PWDLEN		128								//�������봮����

//****** ����,ʱ����Ϣ 20 bytes *********************************************************
typedef struct  _DATE_TIME{
	char	Date[11];					     				//��ʽ:2003/07/10,��ASCIZ����
	char	Time[9];					     					//��ʽ:16:35:12,��ASCIZ����
}DATE_TIME;

//****** ����������Ϣ 256 bytes *********************************************************
typedef	struct	_TASKMESSAGE{
	DATE_TIME DateTime;									//�������ڼ�ʱ��(20�ֽ�:2003/07/10 16:35:12 )
	char	Number[32];										//����������(20�ֽ�)
	char	Name[64];										//������������(64�ֽ�),��ʽ:������λ��
	char	Toastmaster[16];								//����������(16�ֽ�),��ʽ:������λ��
	char  SubmitUnit[64];								//�ͼ쵥λ

	BYTE	reserved[60];									//�����ֽ�
}TASKMESSAGE;

//****** ���Թ�����Ϣ 640 bytes *********************************************************
typedef	struct	_WORKPIECEMESSAGE{
	char	WorkpieceName[32];							//��������(20�ֽ�),��ʽ:������λ��
	char	WorkpieceSort[32];							//�������(20�ֽ�),��ʽ:������λ��
	char	WorkpieceNumber[32];							//�������(20�ֽ�),��ʽ:������λ��
	char	WorkpieceModel[32];							//�����ͺ�(32�ֽ�),��ʽ:������λ��
	char	WorkpieceBatch[32];							//��������(32�ֽ�),��ʽ:������λ��
	char	ShellMaterial[64];   						//�������(40�ֽ�),��ʽ:������λ��
	char	FillerSort[64];								//װ��������(64�ֽ�),��ʽ:������λ��
	char	Manufacturer[64];   							//�������쵥λ(64�ֽ�),��ʽ:������λ��
	char	TestPosition[32];								//��ⲿλ(32�ֽ�),��ʽ:������λ��
	char	ZeroPointDefineAlongAxis[32];				//��������������ֶ���,��ʽ:������λ��
	char	ZeroPointDefineAlongCircle[64];			//����Բ��������ֶ���,��ʽ:������λ��
	float	WorkpieceZeroPointAlongAxis;				//���������������(�����ϵͳ�������)
	float	WorkpieceMaxDiameter;						//�������ֱ��,��λ:mm
	float	WorkpieceDiameter;							//������ⲿλ�⾶,��λ:mm
	float	WorkpieceLength;								//��������,��λ:mm
	float	WorkpieceBeginCoordinate;					//������ʼ����ֵ
	float	WorkpieceEndCoordinate;						//������������ֵ
	float	WorkpieceWeight;								//��������,��λ:kg
	float	MaxDensity;			     	               //��������ܶ�,��λ:g/cm3

	BYTE	reserved[128];									//�����ֽ�
}WORKPIECEMESSAGE;

//****** ɨ�������Ϣ 380 bytes **********************************************************
typedef	struct	_SCANPARAMETER{
	WORD	ScanMode;										//ɨ�跽ʽ: 0-���ɨ��,1-���ɨ��,2-���ɨ��,3-DRɨ�裬4-�ֲ�ɨ��,5-����ɨ��,6-׶��ɨ��
	WORD	Ct2ScanMode;									//2��ɨ��ģʽ:  0-360��,1-180��,2-180����
		//���ɨ��ʱ,˫���ʾƽ��Ϊ������������ɼ�����
		//���ɨ��ʱ,˫���ʾ���������ֵ�����෴
		//DRɨ��ʱ,˫���ʾ�ֲ�Ϊ������������ɼ�����
	WORD	ScanDirectionMode;							//ɨ�跽��: 0-����,1-˫��
	WORD	DataTransferMode;								//���ݴ��ͷ�ʽ: 0-���崫��, 1-��ɨ��ߴ���

	WORD	Pixels;											//ˮƽͼ��ߴ����: 0-64,1-128,2-256,3-512,4-1024,5-2048,6-4096
	float	ViewDiameter;									//�ӳ�ֱ��(mm)
	float	RadialDistanceInLocal;						//�ֲ�ɨ��ʱɨ���ӳ���������ת���ľ���(mm)
	float	AngleInLocal;									//�ֲ�ɨ��ʱ��ת������ɨ���ӳ�����������ˮƽ��������ʼ�н�(��)

	WORD	CollimationSize;								//׼ֱ���ߴ����:0,1,2...   ׼ֱ�����
	float CollimationThickness;						//׼ֱ���߶�/�ϲ���(mm)
	WORD  DetectorStyle;                         //��ǰʹ�õ�̽�����ṹ(0-����, 1-ֱ��, 2-ƽ��)
	WORD	NumberOfSystemHorizontalDetector;		//ˮƽ����ϵͳ̽ͷ��(����, ����/ֱ��̽����)
	WORD	NumberOfValidHorizontalDetector;			//ˮƽ������Ч̽ͷ��(����, ����/ֱ��̽����)
	WORD	SerialNo1OfMiddleHorizontalDetector;	//ˮƽ��������̽�������1(����, ����/ֱ��̽����, ��0��ʼ����)
	WORD	SerialNo2OfMiddleHorizontalDetector;	//ˮƽ��������̽�������2(����, ����/ֱ��̽����, ��0��ʼ����)
	WORD  NumberofSystemVerticalDetector;			//��ֱ����ϵͳ̽������(����)
	WORD  NumberofValidVerticalDetector;			//��ֱ������Ч̽������(����)
	float	SpaceOfHorizontalDetector;					//ˮƽ������Ч̽�������(mm)
	float	SpaceOfVerticalDetector;					//��ֱ������Ч̽�������(mm)(����ɨ�����̽�����ϲ�ʹ�������Ϊ����������������̽��������)
	float	HorizontalSectorAngle;						//ˮƽ������Ч�Ƚ�ֵ(��)	
	float VerticalSectorAngle;							//��ֱ������Ч�Ƚ�ֵ(��)
	float	RadialPosition;								//���㵽��ת���ľ���(����λ��)(mm)    
	float SourceDetectorDistance;						//���㵽̽��������(mm)
	float	Azimuth;											//ɨ��ʱ�ֶ���ʼ��λ��(��)
	WORD	GraduationDirection;							//�ֶȷ���:0-˳ʱ��,1-��ʱ��(������)
	WORD	NumberOfGraduation;							//�ֶȴ���(ָ���ݾࡢ׶��ɨ�衢���άCT��תһ�ֶܷ���)
	WORD	InterpolationFlag;							//��ֵ����־:0-��׼��ֵ,1-��ֵ����1
	WORD	NumberOfInterpolation;						//��ֵ����(��׼��ֵ����)
	WORD	NumberOfTranslation;							//ƽ�Ʋ�������
	WORD	DelaminationMode;								//CT�ֲ㷽ʽ:0-�Ȳ��,1-���Ȳ��,2-��ǰ���
	float HelicalScanPitch;								//����ɨ���ݾ��С,��λ:mm
	WORD	TotalLayers;									//CTɨ���ܲ������һ��DRɨ���ܲ���,��Χ:1-9999
	WORD	TotalLayers2;									//�ڶ���DRɨ���ܲ���,��Χ:0-9999,ֻ��һ��DRɨ��ʱ��д0
	float	CurrentLayerCoordinate;						//��ǰ������,��λ:mm
	float	SpaceBetweenLayer;							//CT���/DR�ֲ���(����,����DRɨ��ʱÿ�β�����ͬ),��λ:mm
	float	FirstSectStartCoordinateOfDR;				//��һ��DRɨ����ʼ����X1, ��λ:mm
	float	SecondSectStartCoordinateOfDR;			//�ڶ���DRɨ����ʼ����X3, ��λ:mm
	WORD	SetupSynchPulseNumber;						//Ԥ��ͬ��������(���������)
	float	SampleTime;										//��������ʱ��, ��λ:��
	WORD	UsedScanTime;									//ʵ��ɨ��ʱ��(s)
	float	TemputreValue;									//̽��ϵͳ�����¶�ֵ(��)
	char	FilenameTemperature[32];					//̽��ϵͳʵʱ�¶ȼ�¼�ļ���
	char	FilenameOfCTdata[32];						//CT�����ļ���,��Windows�ļ���������ַ���(���ڴ��CT��DR��׶��ɨ������)
	char	FilenameOf2CTdata[32];						//������ݱ߲ɱߴ��ļ���(�������ļ������ϱ��(00-31))
	char	FilenameOfPictureData[32];					//ͼ�������ļ���, ��Windows�ļ���������ַ���
	WORD	NumberofTable;									//ɨ����ʹ�õ�ת̨����(ȱʡΪ1, 2��ʾ����ת̨)
	WORD	CenterOfTable;									//ɨ����ʹ�õ�ת̨��(�м�ת̨��)

	BYTE	reserved[120];									//�����ֽ�
}SCANPARAMETER;

//****** ϵͳ������Ϣ 192 bytes ***********************************************************
typedef	struct	_SYSTEMPARAMETER{
	char	ModelOfCT[32];									//CT�豸�ͺ�
	WORD	RaySort;											//����Դ����:0-X���,1-������,2-��60,3-�137
	float	RayEngery;										//��������(MeV)
	float	RayDosage;										//���߼���(cGy)
	WORD	SynchFrequency;								//ͬ��Ƶ��(Hz)
	WORD	AmplifyMultiple;								//�����Ŵ���
	WORD	IntegralTime;									//��������ʱ��(us)

	BYTE	reserved[140];									//�����ֽ�
}SYSTEMPARAMETER;

//****** II��CT�ؽ���Ϣ 128 bytes *********************************************************
typedef	struct	_RECONSTRUCTMESSAGE{
	BYTE	Ct2ReconstructPara[64];						//2��CTɨ��ֶ��ؽ���־(ÿ���ֶ�1�ֽ�,0-�÷ֶ�δ�ؽ�,1-�÷ֶ����ؽ�)
	float	Zmax;												//FDK�����ؽ���
	float	Zmin;												//FDK�����ؽ���
	float	RotationAngle;									//�����ؽ�ͼ����ת�Ƕ�
	float	PixelMax;										//ԭʼ�ؽ�ͼ�����ֵ
	float	PixelMin;										//ԭʼ�ؽ�ͼ����Сֵ
	WORD	VolumeLayer;									//��ά�ؽ�����
	WORD	VolumeWidth;									//��ά�ؽ����
	float	SpaceBetweenPixel;							//����֮����
	BYTE	NumberOfGraduationOfCt2;					//�ֶ�����(2��CTɨ��, 2007.10.04����)
	BYTE	SerialOfGraduationOfCt2;					//�ֶ����(2��CTɨ��, 2007.10.04����)
	BYTE	reserved[32];									//�����ֽ�
}RECONSTRUCTMESSAGE;

//****** ͼ��ʶ����Ϣ 64 bytes ***********************************************************
typedef	struct	_IDENTIFYMESSAGE{
	WORD	DefectMode;										//ȱ������: 0-װ��,1-������,2-����(����,�ѷ�,��ճ),Ĭ��Ϊ0
	char	NumberofTemplate[32];						//ʶ��ģ����(�ַ���), ���뷽ʽ����word��ӡҳ���ʽ; ����,1-3��ʾģ���Ŵ�1��3; 1,4,6,8��ʾʶ��ģ��ֱ�Ϊ1,4,6,8
	BYTE	reserved[30];									//�����ֽ�
}IDENTIFYMESSAGE;

//#pragma pack(1)		//���ֽڶ�������
//****** ���ݸ�ʽ��Ϣ 32 bytes  (V2.1�汾��)***********************************************************
typedef	struct	_DATAFORMAT21{
	WORD	DataType;										//���ݴ洢����: char=01,int=02,unsigned long=03,float=04,double=05
	WORD	TotalLines;										//����������  
	WORD	dataColAtRow;									//ÿ�����ݸ���(��������������)
	WORD	appendColAtRow;								//ÿ����ǰ���������ݸ���
	WORD	graduationBase;								//3��ɨ��ֶ���ʼ��(2007.10.04����)
	BYTE	reserved[22];									//�����ֽ�
}DATAFORMAT21;
//****** �����ֽڳ��� *******************************************************************
#define	RESERVEDBYTES	2048-4-2-PWDLEN-sizeof(TASKMESSAGE)-sizeof(WORKPIECEMESSAGE)-sizeof(SCANPARAMETER)-sizeof(SYSTEMPARAMETER)-sizeof(RECONSTRUCTMESSAGE)-sizeof(IDENTIFYMESSAGE)-sizeof(DATAFORMAT21)

//****** ͷ�ļ���Ϣ(V2.1�汾ʹ��) *********************************************************************
typedef	struct _ICT_HEADER21{
	BYTE						MainVersion;				//�汾��ʶ(���汾��: 0x02)
	BYTE						SubVersion;					//�汾��ʶ(�ΰ汾��: 0x01)
	BYTE						reserved1[2];				//����
	TASKMESSAGE				Task;							//����������Ϣ
	WORKPIECEMESSAGE		Workpiece;					//���Թ�����Ϣ
	SCANPARAMETER			ScanParameter;				//ɨ�����
	SYSTEMPARAMETER		SystemParameter;			//ϵͳ����
	RECONSTRUCTMESSAGE	ReconstructParameter;	//�ؽ�����
	IDENTIFYMESSAGE		IdentifyMessage;			//ͼ��ʶ����Ϣ
	DATAFORMAT21			DataFormat;					//���ݸ�ʽ
	BYTE	       			reserved2[RESERVEDBYTES];//����(316)
	WORD						SizeofHeader;				//�ļ�ͷ�ߴ�: >2048(�ļ�ͷ�ߴ�Ϊ�������)
	BYTE						Password[PWDLEN];			//�����ַ���
}ICT_HEADER21;
#pragma pack(1)		//���ֽڶ�������
//****** ���ݸ�ʽ��Ϣ 32 bytes  (V2.2�汾��)***********************************************************
typedef	struct	_DATAFORMAT{
	WORD	DataType;										//���ݴ洢����: char=01,int=02,unsigned long=03,float=04,double=05
	DWORD	TotalLines;										//����������  .2008-01-23��RORD��ΪDWORD,ͬʱ�汾�Ŵ�V2.1 ��ΪV2.2
	WORD	dataColAtRow;									//ÿ�����ݸ���(��������������)
	WORD	appendColAtRow;								//ÿ����ǰ���������ݸ���
	WORD	graduationBase;								//3��ɨ��ֶ���ʼ��(2007.10.04����)
	BYTE	reserved[20];									//�����ֽ�
}DATAFORMAT22;

#pragma pack(8)		//��ȱʡֵ��������

//****** ͷ�ļ���Ϣ (V2.2�汾ʹ��)*********************************************************************
typedef	struct _ICT_HEADER22{
	BYTE						MainVersion;				//�汾��ʶ(���汾��: 0x02)
	BYTE						SubVersion;					//�汾��ʶ(�ΰ汾��: 0x02)
	BYTE						reserved1[2];				//����
	TASKMESSAGE				Task;							//����������Ϣ
	WORKPIECEMESSAGE		Workpiece;					//���Թ�����Ϣ
	SCANPARAMETER			ScanParameter;				//ɨ�����
	SYSTEMPARAMETER		SystemParameter;			//ϵͳ����
	RECONSTRUCTMESSAGE	ReconstructParameter;   //�ؽ�����
	IDENTIFYMESSAGE		IdentifyMessage;			//ͼ��ʶ����Ϣ
	DATAFORMAT22				DataFormat;					//���ݸ�ʽ
	BYTE	       			reserved2[RESERVEDBYTES];//����(316)
	WORD						SizeofHeader;				//�ļ�ͷ�ߴ�: >2048(�ļ�ͷ�ߴ�Ϊ�������)
	BYTE						Password[PWDLEN];			//�����ַ���
}ICT_HEADER22;
//����CTɨ�蹤��ʵ�ʸ߶� = ɨ���ܲ���*�ݾ�
//�ļ�ͷ֮�������к��е�˳�����δ������
//ϵͳ�ṩ���ɸ���̬�⺯����ɸ�������Ϣ�ļ��ܺͽ���
#endif
