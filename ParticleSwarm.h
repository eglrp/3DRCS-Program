// ParticleSwarm.h: interface for the CParticleSwarm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLESWARM_H__E2A4B76B_A96D_4439_9C87_352F95EA1C3C__INCLUDED_)
#define AFX_PARTICLESWARM_H__E2A4B76B_A96D_4439_9C87_352F95EA1C3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CParticleSwarm  //����Ⱥ��
{
public:
	double gbest; //ȫ�ּ�ֵ����Ӧֵ
	double gbest_pos[GANVARS]; //ȫ�ּ�ֵ������
    Particle PSO_pop[PSO_popsize];//�������Ӷ���Ϊ����Ⱥ�������
public: 
	void init();  //��ʼ����Ⱥ
	void getGBest(); //��ȡȫ�ּ�ֵ
	void search(double *Array); //����,col������[0,RUNTIME-1]��ָʾ�ǵڼ������� 	  
public:
	CParticleSwarm();
	virtual ~CParticleSwarm();

};

#endif // !defined(AFX_PARTICLESWARM_H__E2A4B76B_A96D_4439_9C87_352F95EA1C3C__INCLUDED_)
