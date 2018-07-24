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

//�����
class Edge  
{
public:
	Edge();
	virtual ~Edge();
public:
	int vertex1 ;  //ָ��ñߵĵ�һ������ ����1
	int vertex2 ;  //ָ��ñߵĵڶ������� ����2

	int facet;     //����������Ƭָ��
	int rfacet;    //�����Ӧ�İ�ߣ�������1�Ͷ���2˳���෴

	bool flag;     //�����־

	float length;  //�ߵĳ��ȣ�

	bool eBoundary;
	long edgenum;
};

#endif // !defined(AFX_EDGE_H__292AC8A4_C13C_4A75_BADE_69EDE234273F__INCLUDED_)
