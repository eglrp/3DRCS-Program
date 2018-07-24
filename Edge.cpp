// Edge.cpp: implementation of the Edge class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "3DRCS.h"
#include "Edge.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Edge::Edge()
	: edgenum(0)
{
	vertex1=-1 ; //指向该边的第一个顶点
	vertex2=-1 ; //指向该边的第二个顶点

	facet=-1 ;
	rfacet=-1 ;
	flag=true;   //处理标志

	eBoundary = false;
}

Edge::~Edge()
{


}
