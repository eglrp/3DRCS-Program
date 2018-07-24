// Facet.h: interface for the Facet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FACET_H__4ECE3411_8730_4AA4_ABD7_F20762A751ED__INCLUDED_)
#define AFX_FACET_H__4ECE3411_8730_4AA4_ABD7_F20762A751ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Edge.h"
#include "Vertex.h"
class Edge;
class Vertex;

//三角面片类
class Facet  
{
public:
	Facet();
	virtual ~Facet();

	float nx ; //面片法向量的X坐标值
	float ny ; //面片法向量的Y坐标值
	float nz ; //面片法向量的Z坐标值

	float x ; //面片折叠点的X坐标值
	float y ; //面片折叠点的Y坐标值
	float z ; //面片折叠点的Z坐标值

	float area ; //面片的面积
	float value;

	int vertex[3] ; //指向该面片的三个顶点

	bool flag;     //处理标志
	
	int edge[3];     //边。即顶点1和2、2和3、3和1为端点的边
	int aFacet[3];   //相邻面。即以顶点1和2、2和3、3和1为公共点的相邻面
	int order;

	double StandardT;
	// 边界三角形
//	bool eBoundary;
	bool fBoundary;
};

#endif // !defined(AFX_FACET_H__4ECE3411_8730_4AA4_ABD7_F20762A751ED__INCLUDED_)
