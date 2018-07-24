// Particle.h: interface for the CParticle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLE_H__79AA5A81_A676_4F72_925B_D565AAC0635D__INCLUDED_)
#define AFX_PARTICLE_H__79AA5A81_A676_4F72_925B_D565AAC0635D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define 

class CParticle  //����������
{
public:
	double pos[GANVARS]; 		//����λ��
	double v[GANVARS]; 			//�����ٶ�
	double pbest; 
	double pbest_pos[GANVARS];//�������Ž�����꣬��Ӧÿ��֤ȯ��ϱ�����ֵ
	double fitness; //��ǰ�����һ����Ӧֵ
public:
	double calcFitness(double pos[]); 
	void updatePosition(); 
	void updatePBest();	 
public:
	CParticle();
	virtual ~CParticle();

};

#endif // !defined(AFX_PARTICLE_H__79AA5A81_A676_4F72_925B_D565AAC0635D__INCLUDED_)
