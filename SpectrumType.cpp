#include "StdAfx.h"
#include "SpectrumType.h"


SpectrumType::SpectrumType(void)
{
}


SpectrumType::~SpectrumType(void)
{
	if (period)				delete period;
	if (dispSpectrum)	delete dispSpectrum;
	if (velSpectrum)		delete velSpectrum;
	if (accSpectrum)		delete accSpectrum;
	if (designSpectrum)	delete designSpectrum;
}

void SpectrumType::WriteSpectrum(char* filename, SpectrumType* spectrum, int type)
{
	FILE* file = NULL;

	if ((file = fopen(filename, "w+")) == NULL)
	{
		TRACE("[%s] Out File[%s] Open Error", __FUNCTION__, filename);
		return;
	}
	
	char sBuffer[128];

	for (int i = 1; i < spectrum->nperiod + 1; i++)
	{
		memset(sBuffer, 0x00, sizeof(sBuffer));

		if (type == 1)
			sprintf(sBuffer, "%10.15g   \t%10.15g\n", spectrum->period[i], spectrum->dispSpectrum[i]);
		else if (type == 2)
			sprintf(sBuffer, "%10.15g   \t%10.15g\n", spectrum->period[i], spectrum->velSpectrum[i]);
		else if (type == 3)
			sprintf(sBuffer, "%10.15g   \t%10.15g\n", spectrum->period[i], spectrum->accSpectrum[i]);
		else if (type == 4)
			sprintf(sBuffer, "%10.15g   \t%10.15g\n", spectrum->period[i], spectrum->designSpectrum[i]);

		fprintf(file, "%s", sBuffer);
	}

	fclose(file);
}
