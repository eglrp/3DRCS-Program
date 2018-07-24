// 3DRCS.h : main header file for the 3DRCS application
//

#if !defined(AFX_3DRCS_H__48BAF0FF_68E2_4D0D_A8C9_AB9ECAFB6A3B__INCLUDED_)
#define AFX_3DRCS_H__48BAF0FF_68E2_4D0D_A8C9_AB9ECAFB6A3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#ifndef _INC_EdgeContourAPI_a
#define _INC_EdgeContourAPI_a


#include "vector"
 using namespace std;
 
#endif


//允许设置OpenGL
#include <GL/gl.h>
#include <GL/glaux.h>
#include <gl/glu.h>

//定义几种常用颜色
const COLORREF GREEN = RGB(0,255,0);
const COLORREF YELLOW = RGB(255,255,0);
const COLORREF BLACK  = RGB(0,0,0);

//向控制台发送消息使按钮可用
const int MESSAGE_ENABLEBTN = WM_USER + 103;

//向控制台发送消息使按钮不可用
const int MESSAGE_DISABLEBTN = WM_USER + 104;

//通告各窗口数据读取已完成
const int MESSAGE_GETDATAFINISHED = WM_USER+106;

/////////////////////////////////////////////////////////////////////////////
// C3DRCSApp:
// See 3DRCS.cpp for the implementation of this class
//

class C3DRCSApp : public CWinApp
{
public:
	C3DRCSApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(C3DRCSApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(C3DRCSApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
//	afx_msg void OnTurn();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_3DRCS_H__48BAF0FF_68E2_4D0D_A8C9_AB9ECAFB6A3B__INCLUDED_)
