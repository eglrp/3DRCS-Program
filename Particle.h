// Particle.h: interface for the CParticle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLE_H__79AA5A81_A676_4F72_925B_D565AAC0635D__INCLUDED_)
#define AFX_PARTICLE_H__79AA5A81_A676_4F72_925B_D565AAC0635D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define 

class CParticle  //单个粒子类
{
public:
	double pos[GANVARS]; 		//粒子位置
	double v[GANVARS]; 			//粒子速度
	double pbest; 
	double pbest_pos[GANVARS];//个体最优解的坐标，对应每种证券组合比例的值
	double fitness; //当前算出的一个适应值
public:
	double calcFitness(double pos[]); 
	void updatePosition(); 
	void updatePBest();	 
public:
	CParticle();
	virtual ~CParticle();

};

#endif // !defined(AFX_PARTICLE_H__79AA5A81_A676_4F72_925B_D565AAC0635D__INCLUDED_)
