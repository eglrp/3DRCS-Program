// ParticleSwarm.h: interface for the CParticleSwarm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLESWARM_H__E2A4B76B_A96D_4439_9C87_352F95EA1C3C__INCLUDED_)
#define AFX_PARTICLESWARM_H__E2A4B76B_A96D_4439_9C87_352F95EA1C3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CParticleSwarm  //粒子群类
{
public:
	double gbest; //全局极值的适应值
	double gbest_pos[GANVARS]; //全局极值的坐标
    Particle PSO_pop[PSO_popsize];//单个粒子定义为粒子群类的属性
public: 
	void init();  //初始化种群
	void getGBest(); //获取全局极值
	void search(double *Array); //迭代,col参数是[0,RUNTIME-1]，指示是第几次运行 	  
public:
	CParticleSwarm();
	virtual ~CParticleSwarm();

};

#endif // !defined(AFX_PARTICLESWARM_H__E2A4B76B_A96D_4439_9C87_352F95EA1C3C__INCLUDED_)
