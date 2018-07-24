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

//������Ƭ��
class Facet  
{
public:
	Facet();
	virtual ~Facet();

	float nx ; //��Ƭ��������X����ֵ
	float ny ; //��Ƭ��������Y����ֵ
	float nz ; //��Ƭ��������Z����ֵ

	float x ; //��Ƭ�۵����X����ֵ
	float y ; //��Ƭ�۵����Y����ֵ
	float z ; //��Ƭ�۵����Z����ֵ

	float area ; //��Ƭ�����
	float value;

	int vertex[3] ; //ָ�����Ƭ����������

	bool flag;     //�����־
	
	int edge[3];     //�ߡ�������1��2��2��3��3��1Ϊ�˵�ı�
	int aFacet[3];   //�����档���Զ���1��2��2��3��3��1Ϊ�������������
	int order;

	double StandardT;
	// �߽�������
//	bool eBoundary;
	bool fBoundary;
};

#endif // !defined(AFX_FACET_H__4ECE3411_8730_4AA4_ABD7_F20762A751ED__INCLUDED_)
