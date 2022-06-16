#include "StdAfx.h"
#include "GroundAccType.h"

#include <math.h>

GroundAccType::GroundAccType(void)
{
	gtime	= NULL;
	acc		= NULL;
	maxtime	= 0.0;
	dt			= 0.0;
	gcount	= 0;
}


GroundAccType::~GroundAccType(void)
{
	if (gtime)	delete gtime;
	if (acc)		delete acc;
}

double GroundAccType::GetCurrentAcc(GroundAccType earthq, double ctime)
{
	int i;
	double curacc = 0.0;
	for (i = 0; i < earthq.gcount - 1; i++)
	{
		if ((earthq.gtime[i] < ctime) & (ctime < earthq.gtime[i + 1]))
		{
			curacc = earthq.acc[i] + (ctime - earthq.gtime[i])
				* (earthq.acc[i + 1] - earthq.acc[i]) / (earthq.gtime[i + 1] - earthq.gtime[i]);
		}
		else if (fabs(earthq.gtime[i] - ctime) < 1e-10)
		{
			curacc = earthq.acc[i];
		}
	}
	return curacc;
}
