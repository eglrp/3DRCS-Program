// Vertex.cpp: implementation of the Vertex3D class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "3DRCS.h"
#include "Vertex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Vertex::Vertex()
{
	nx=ny=nz=0.0;
	x=y=z=0.0;
	flag=true;   //处理标志
	bBoundary=false;
}

Vertex::~Vertex()
{

}
