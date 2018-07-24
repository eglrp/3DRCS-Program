// Edge.h: interface for the Edge class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDGE_H__292AC8A4_C13C_4A75_BADE_69EDE234273F__INCLUDED_)
#define AFX_EDGE_H__292AC8A4_C13C_4A75_BADE_69EDE234273F__INCLUDED_

#include "Facet.h"
#include "Vertex.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Facet;
class Vertex;

//半边类
class Edge  
{
public:
	Edge();
	virtual ~Edge();
public:
	int vertex1 ;  //指向该边的第一个顶点 顶点1
	int vertex2 ;  //指向该边的第二个顶点 顶点2

	int facet;     //所在三角面片指针
	int rfacet;    //与其对应的半边，即顶点1和顶点2顺序相反

	bool flag;     //处理标志

	float length;  //边的长度；

	bool eBoundary;
	long edgenum;
};

#endif // !defined(AFX_EDGE_H__292AC8A4_C13C_4A75_BADE_69EDE234273F__INCLUDED_)
