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
	float x ;//�����X ����ֵ
	float y ;//�����Y����ֵ
	float z ;//�����Z ����ֵ

	vector<int> facet; //�������ڵ���Ƭ

	float nx ;//�����X����������ֵ
	float ny ;//�����Y����������ֵ
	float nz ;//�����Z����������ֵ

	bool flag;     //�����־
	bool bBoundary;

};

#endif // !defined(AFX_VERTEX3D_H__A3C398E2_A827_45FB_9646_964354AD1089__INCLUDED_)
