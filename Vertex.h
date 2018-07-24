// Vertex.h: interface for the Vertex3D class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VERTEX3D_H__A3C398E2_A827_45FB_9646_964354AD1089__INCLUDED_)
#define AFX_VERTEX3D_H__A3C398E2_A827_45FB_9646_964354AD1089__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Facet.h"
#include "Edge.h"
#include <vector>

class Edge;
class Facet;

class Vertex  
{
public:
	Vertex();
	virtual ~Vertex();

public:
	float x ;//顶点的X 坐标值
	float y ;//顶点的Y坐标值
	float z ;//顶点的Z 坐标值

	vector<int> facet; //顶点所在的面片

	float nx ;//顶点的X法向量坐标值
	float ny ;//顶点的Y法向量坐标值
	float nz ;//顶点的Z法向量坐标值

	bool flag;     //处理标志
	bool bBoundary;

};

#endif // !defined(AFX_VERTEX3D_H__A3C398E2_A827_45FB_9646_964354AD1089__INCLUDED_)
