#pragma once
class GroundAccType
{
public:
	GroundAccType(void);
	virtual ~GroundAccType(void);

	double*	gtime;
	double*	acc;
	double	maxtime, dt;
	int			gcount;

	double	GetCurrentAcc(GroundAccType earthq, double ctime);
};

