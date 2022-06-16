#pragma once
class SpectrumType
{
public:
	SpectrumType(void);
	virtual ~SpectrumType(void);

	double*	period;
	double*	dispSpectrum;
	double*	velSpectrum;
	double*	accSpectrum;
	double*	designSpectrum;
	double	minPeriod, maxPeriod;
	int			nperiod;

	void WriteSpectrum(char* filename, SpectrumType* spectrum, int type);
};

