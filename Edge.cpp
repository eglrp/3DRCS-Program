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
	vertex1=-1 ; //ָ��ñߵĵ�һ������
	vertex2=-1 ; //ָ��ñߵĵڶ�������

	facet=-1 ;
	rfacet=-1 ;
	flag=true;   //�����־

	eBoundary = false;
}

Edge::~Edge()
{


}
