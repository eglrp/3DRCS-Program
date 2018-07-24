// Facet.cpp: implementation of the Facet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "3DRCS.h"
#include "Facet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Facet::Facet()
{
	vertex[0]=-1 ; //指向该面片的第一个顶点
	vertex[1]=-1 ; //指向该面片的第二个顶点
	vertex[2]=-1 ; //指向该面片的第三个顶点
	edge[0]=edge[1]=edge[2]=-1;
	aFacet[0]=aFacet[1]=aFacet[2]=-1;
	nx=ny=nz=0.0f;
	x=y=z=0.0f;
	value=0.0f;
	flag=true;   //处理标志
	order=-1;
	fBoundary = false;

}

Facet::~Facet()
{


}
