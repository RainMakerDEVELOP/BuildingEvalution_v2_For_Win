#include "stdafx.h"
#include <stdlib.h>
#include <math.h>
#include "wavedataManager.h"
#include "../Include/fft_common.h"
#include "codedef.h"

// #define	DEF_DEBUG


AllCompData_t* 	g_WaveDataHead = NULL;
AllCompData_t* 	g_WaveDataTail = NULL;

// int		g_iDataCount = 0;
// extern int	g_iDebug;


void Init_WaveDataPool()
{
    g_WaveDataHead = (AllCompData_t*)malloc(sizeof(AllCompData_t));
    g_WaveDataHead->next = NULL;

	g_WaveDataHead->ldTime = 0;
								// "2016-09-12T11:32:22.680000"
	strcpy(g_WaveDataHead->szTime, "00000000000000000000000000");
	g_WaveDataHead->iCountZ = 0;
	g_WaveDataHead->dwAccZ	= 0.;
	g_WaveDataHead->iCountN	= 0;
	g_WaveDataHead->dwAccN	= 0.;
	g_WaveDataHead->iCountE	= 0;
	g_WaveDataHead->dwAccE	= 0.;
	
#ifdef	DEF_DEBUG	
    printf_log("## Init WaveData Pool \n");
#endif
}


AllCompData_t* AddUpdateChanData(AllCompData_t* pBaseD, ChanData_t* pChanData)
{
	if( pBaseD == NULL )
	{
		pBaseD = g_WaveDataHead;
	}

#ifdef	_DEBUG	
	//TRACE("if( pBaseD->ldTime[%ld] == pChanData->ldTime[%ld])  [%c]\n"
	//		, pBaseD->ldTime
	//		, pChanData->ldTime
	//		, pChanData->cDType);
#endif
	
	if( pBaseD->ldTime == pChanData->ldTime )
	{
		pBaseD->dwRKtime = pChanData->dwRKtime;

		switch( pChanData->cDType )
		{
		case DEF_ACC_Z :
			pBaseD->iCountZ	= pChanData->iCount;
			pBaseD->dwAccZ	= pChanData->dwAcc;
			pBaseD->cDataStatus |= DEF_ACC_Z_ON;
			break;
			
		case DEF_ACC_N :
		case DEF_ACC_Y :
			pBaseD->iCountN	= pChanData->iCount;
			pBaseD->dwAccN	= pChanData->dwAcc;
			pBaseD->cDataStatus |= DEF_ACC_N_ON;
			break;
			
		case DEF_ACC_E :
		case DEF_ACC_X :
			pBaseD->iCountE	= pChanData->iCount;
			pBaseD->dwAccE	= pChanData->dwAcc;
			pBaseD->cDataStatus |= DEF_ACC_E_ON;
			break;

		default :
			break;
		}
		
#ifdef	_DEBUG	
	//TRACE("### Update [%ld][%c]\n", pBaseD->ldTime, pChanData->cDType);
#endif
		return pBaseD;
	}
	
	AllCompData_t *t;

	if( pBaseD->ldTime > pChanData->ldTime )
	{
		t = FindWaveData(pChanData->ldTime);
		if( t != NULL )
		{
			t->dwRKtime = pChanData->dwRKtime;

			switch( pChanData->cDType )
			{
			case DEF_ACC_Z :
				t->iCountZ 	= pChanData->iCount;
				t->dwAccZ	= pChanData->dwAcc;
				t->cDataStatus |= DEF_ACC_Z_ON;
				break;
				
			case DEF_ACC_N :
			case DEF_ACC_Y :
				t->iCountN	= pChanData->iCount;
				t->dwAccN	= pChanData->dwAcc;
				t->cDataStatus |= DEF_ACC_N_ON;
				break;
				
			case DEF_ACC_E :
			case DEF_ACC_X :
				t->iCountE	= pChanData->iCount;
				t->dwAccE	= pChanData->dwAcc;
				t->cDataStatus |= DEF_ACC_E_ON;
				break;
			}
#ifdef	_DEBUG	
	//TRACE("### SS Update [%ld][%c]\n", t->ldTime, pChanData->cDType);
#endif
			return t;
		}
	}

	t = FindNextWaveData(pBaseD, pChanData->ldTime);
	
	if( t != NULL )
	{
#ifdef	_DEBUG	
		//TRACE("### FindNextWaveData [%ld][%c]\n", t->ldTime, pChanData->cDType);
#endif
		t->dwRKtime = pChanData->dwRKtime;

		switch( pChanData->cDType )
		{
		case DEF_ACC_Z :
			t->iCountZ 	= pChanData->iCount;
			t->dwAccZ	= pChanData->dwAcc;
			t->cDataStatus |= DEF_ACC_Z_ON;
			break;
			
		case DEF_ACC_N :
		case DEF_ACC_Y :
			t->iCountN	= pChanData->iCount;
			t->dwAccN	= pChanData->dwAcc;
			t->cDataStatus |= DEF_ACC_N_ON;
			break;
			
		case DEF_ACC_E :
		case DEF_ACC_X :
			t->iCountE	= pChanData->iCount;
			t->dwAccE	= pChanData->dwAcc;
			t->cDataStatus |= DEF_ACC_E_ON;
			break;
		}
#ifdef	_DEBUG	
	//TRACE("### SS Update [%ld][%c]\n", t->ldTime, pChanData->cDType);
#endif
		return t;
	}
	
	// add new WaveData
	t = (AllCompData_t*)malloc(sizeof(AllCompData_t));
	t->next = NULL;
	
	t->ldTime 	= pChanData->ldTime;
	strcpy(t->szTime, pChanData->szTime);
	t->iCountZ 	= 0;
	t->dwAccZ	= 0.;
	t->iCountN	= 0;
	t->dwAccN	= 0.;
	t->iCountE	= 0;
	t->dwAccE	= 0.;
	t->cDataStatus = 0x00;
	t->dwRKtime = 0.;

	switch( pChanData->cDType )
	{
	case DEF_ACC_Z :
		t->iCountZ 	= pChanData->iCount;
		t->dwAccZ	= pChanData->dwAcc;
		t->cDataStatus |= DEF_ACC_Z_ON;
		t->dwRKtime = pChanData->dwRKtime;
		break;
		
	case DEF_ACC_N :
	case DEF_ACC_Y :
		t->iCountN	= pChanData->iCount;
		t->dwAccN	= pChanData->dwAcc;
		t->cDataStatus |= DEF_ACC_N_ON;
		t->dwRKtime = pChanData->dwRKtime;
		break;
		
	case DEF_ACC_E :
	case DEF_ACC_X :
		t->iCountE	= pChanData->iCount;
		t->dwAccE	= pChanData->dwAcc;
		t->cDataStatus |= DEF_ACC_E_ON;
		t->dwRKtime = pChanData->dwRKtime;
		break;

	default :
		free(t);
		return pBaseD;
	}

	AllCompData_t*	b;
	AllCompData_t*	s;
	s = FindPosWaveData(pBaseD, pChanData->ldTime);

#ifdef	_DEBUG	
	//TRACE("### FindPosWaveData [%ld][%c]\n", s->ldTime, pChanData->cDType);
#endif
	
	b = (AllCompData_t*)s->next;
	s->next = t;
	t->next = b;

#ifdef	_DEBUG	
	//TRACE("### Insert [%ld][%c]\n", t->ldTime, pChanData->cDType);
#endif
	//g_iDataCount++;
	return t;
}

AllCompData_t* FindPosWaveData(AllCompData_t *f, long ldTime)
{
    AllCompData_t *t;
    AllCompData_t *s;
    // t = g_WaveDataHead->next;
	// s = g_WaveDataHead;
    t = (AllCompData_t*) f->next;
	s = f;
		
    while( t != NULL ) 
	{
/*
#ifdef	DEF_DEBUG	
	printf_log("### strcmp(f->szTime[%s], t->szTime[%s]\n", f->szTime, t->szTime);
#endif
*/
		if( t->ldTime > ldTime )
		{
/*
#ifdef	DEF_DEBUG	
	printf_log("### return t\n");
#endif
*/
			return s;
		}
		else if( f->ldTime == t->ldTime )
		{
			return t;
		}

		s = t;
    	t = (AllCompData_t*) t->next;
/*
#ifdef	DEF_DEBUG	
	printf_log("### t = t->next\n");
#endif
*/
    }
/*
#ifdef	DEF_DEBUG	
	printf_log("Last return s\n");
#endif
*/
	return s;
}

AllCompData_t* FindWaveData(long ldTime)
{
    AllCompData_t *t;
    t = (AllCompData_t*) g_WaveDataHead->next;
	
    while( t != NULL ) 
	{
/*	
#ifdef	DEF_DEBUG	
	printf_log("### strcmp(f->szTime[%s], t->szTime[%s]\n", f->szTime, t->szTime);
#endif
*/
		if( ldTime == t->ldTime )
		{
			return t;
		}
		else if( ldTime < t->ldTime )
		{
			return NULL;
		}
		
    	t = (AllCompData_t*) t->next;
    }
	return NULL;
}

AllCompData_t* FindNextWaveData(AllCompData_t* p, long ldTime)
{
    AllCompData_t *t;
    t = (AllCompData_t*) p->next;
	
    while( t != NULL ) 
	{
/*
#ifdef	DEF_DEBUG	
	printf_log("### FindNextWaveData(t->ldTime[%ld], ldTime[%ld]\n", t->ldTime, ldTime);
#endif
*/
		if( t->ldTime == ldTime )
		{
#ifdef	DEF_DEBUG	
	printf_log("### Found return FindNextWaveData(t->ldTime[%ld], ldTime[%ld]\n", t->ldTime, ldTime);
#endif
			return t;
		}
		else if( t->ldTime > ldTime )
		{
#ifdef	DEF_DEBUG	
	printf_log("### Not Found return FindNextWaveData(t->ldTime[%ld], ldTime[%ld]\n", t->ldTime, ldTime);
#endif
			return NULL;
		}
		
    	t = (AllCompData_t*) t->next;
    }
#ifdef	DEF_DEBUG	
	printf_log("### Not Found return ldTime[%ld]\n", ldTime);
#endif
	return NULL;
}

int GetWaveDataListCount()
{
    AllCompData_t *t;
    t = (AllCompData_t*) g_WaveDataHead->next;
	
	int	idataCount = 0;
    
    while( t != NULL ) 
	{
		idataCount++;
    	t = (AllCompData_t*) t->next;
    }
	
	return idataCount;
}

int GetRealDataCount(char cDType)
{
    AllCompData_t *t;
    t = (AllCompData_t*) g_WaveDataHead->next;
	
	int	idataCount = 0;
    
    while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;
		
		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;
			
		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;
		
		default :
			return 0;
		}
    	t = (AllCompData_t*) t->next;
    }
	
	return idataCount;
}

int GetRealData(char cDType, double* pdwData, int iCnt)
{
    AllCompData_t *t;
    t = (AllCompData_t*) g_WaveDataHead->next;
	
	int	idataCount = 0;
    
    while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				pdwData[idataCount] = t->dwAccZ;
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;
		
		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				pdwData[idataCount] = t->dwAccN;
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;
			
		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				pdwData[idataCount] = t->dwAccE;
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;
		
		default :
			return 0;
		}
    	t = (AllCompData_t*) t->next;
    }
	
	return idataCount;
}

int GetCountData(char cDType, int* piData, int iCnt)
{
    AllCompData_t *t;
    t = (AllCompData_t*) g_WaveDataHead->next;
	
	int	idataCount = 0;
    
    while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				piData[idataCount] = t->iCountZ;
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;
		
		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				piData[idataCount] = t->iCountN;
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;
			
		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				piData[idataCount] = t->iCountE;
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;
		
		default :
			return 0;
		}
    	t = (AllCompData_t*) t->next;
    }
	
	return idataCount;
}

int GetRKtimeData(char cDType, double* pdwData, int iCnt)
{
	AllCompData_t *t;
	t = (AllCompData_t*) g_WaveDataHead->next;

	int	idataCount = 0;

	while( t != NULL ) 
	{
		if(idataCount >= iCnt)
			break;

		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				pdwData[idataCount] = t->dwRKtime;
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;

		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				pdwData[idataCount] = t->dwRKtime;
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;

		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				pdwData[idataCount] = t->dwRKtime;
				idataCount++;
			}
			else
			{
				if( idataCount > 0 )
				{
					return idataCount;
				}
			}
			break;

		default :
			return 0;
		}
		t = (AllCompData_t*) t->next;
	}

	return idataCount;
}

int UpdateRealData(char cDType, char cDkind, double* pdwData, int iCnt, double &pMaxPGA, double &pMaxPGV, double &pMaxDisp)
{
    AllCompData_t *t;
    t = (AllCompData_t*) g_WaveDataHead->next;
	
	int	idataCount = 0;
	double dblPGA = 0.0;
	double dblPGV = 0.0;
	double dblDisp = 0.0;
	double dblMaxPGA = 0.0;
	double dblMaxPGV = 0.0;
	double dblMaxDisp = 0.0;
	bool bAccSwich = false;
	bool bDispSwich = false;

	double dblFirstDisp = 0.0;
	double dblLastDisp = 0.0;
    
	// Max값 산출
    while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				switch( cDkind )
				{
				case DEF_D_KIND_C :
					t->iCountZ = (int)pdwData[idataCount];
					idataCount++;
					break;
				case DEF_D_KIND_A :
					t->dwAccZ = pdwData[idataCount];

					dblPGA = fabs(pdwData[idataCount]);
					if(dblPGA > dblMaxPGA)
					{
						dblMaxPGA = dblPGA;

						if( pdwData[idataCount] < 0.0 )
						{
							bAccSwich = true;
						}
						else
						{
							bAccSwich = false;
						}
					}
					idataCount++;
					break;
				case DEF_D_KIND_V :
					t->dwVelZ = pdwData[idataCount];

					dblPGV = fabs(pdwData[idataCount]);
					if(dblPGV > dblMaxPGV)
					{
						dblMaxPGV = dblPGV;
					}
					idataCount++;
					break;
				case DEF_D_KIND_D :
					t->dwDispZ = pdwData[idataCount];

					dblDisp = fabs(pdwData[idataCount]);

					if(dblDisp > dblMaxDisp)
					{
						dblMaxDisp = dblDisp;

						if( pdwData[idataCount] < 0.0 )
						{
							bDispSwich = true;
						}
						else
						{
							bDispSwich = false;
						}
					}

					// 2019.07.02 구간 데이터의 최초, 최종 변위값을 설정하여, 최종 도출된 최대변위값이 해당 값과 같은지 비교하기 위함
					if( idataCount == 0 )
					{
						dblFirstDisp = dblDisp;
					}
					else if( idataCount == iCnt - 1)
					{
						dblLastDisp = dblDisp;
					}

					idataCount++;
					break;
				}
			}
			break;
		
		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				switch( cDkind )
				{
				case DEF_D_KIND_C :
					t->iCountN = (int)pdwData[idataCount];
					idataCount++;
					break;
				case DEF_D_KIND_A :
					t->dwAccN = pdwData[idataCount];

					dblPGA = fabs(pdwData[idataCount]);
					if(dblPGA > dblMaxPGA)
					{
						dblMaxPGA = dblPGA;

						if( pdwData[idataCount] < 0.0 )
						{
							bAccSwich = true;
						}
						else
						{
							bAccSwich = false;
						}
					}
					idataCount++;
					break;
				case DEF_D_KIND_V :
					t->dwVelN = pdwData[idataCount];

					dblPGV = fabs(pdwData[idataCount]);
					if(dblPGV > dblMaxPGV)
					{
						dblMaxPGV = dblPGV;
					}
					idataCount++;
					break;
				case DEF_D_KIND_D :
					t->dwDispN = pdwData[idataCount];

					dblDisp = fabs(pdwData[idataCount]);

					if(dblDisp > dblMaxDisp)
					{
						dblMaxDisp = dblDisp;

						if( pdwData[idataCount] < 0.0 )
						{
							bDispSwich = true;
						}
						else
						{
							bDispSwich = false;
						}
					}

					// 2019.07.02 구간 데이터의 최초, 최종 변위값을 설정하여, 최종 도출된 최대변위값이 해당 값과 같은지 비교하기 위함
					if( idataCount == 0 )
					{
						dblFirstDisp = dblDisp;
					}
					else if( idataCount == iCnt - 1)
					{
						dblLastDisp = dblDisp;
					}

					idataCount++;
					break;
				}
			}
			break;
			
		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				switch( cDkind )
				{
				case DEF_D_KIND_C :
					t->iCountE = (int)pdwData[idataCount];
					idataCount++;
					break;
				case DEF_D_KIND_A :
					t->dwAccE = pdwData[idataCount];

					dblPGA = fabs(pdwData[idataCount]);
					if(dblPGA > dblMaxPGA)
					{
						dblMaxPGA = dblPGA;

						if( pdwData[idataCount] < 0.0 )
						{
							bAccSwich = true;
						}
						else
						{
							bAccSwich = false;
						}
					}
					idataCount++;
					break;
				case DEF_D_KIND_V :
					t->dwVelE = pdwData[idataCount];

					dblPGV = fabs(pdwData[idataCount]);
					if(dblPGV > dblMaxPGV)
					{
						dblMaxPGV = dblPGV;
					}
					idataCount++;
					break;
				case DEF_D_KIND_D :
					t->dwDispE = pdwData[idataCount];

					dblDisp = fabs(pdwData[idataCount]);
					if(dblDisp > dblMaxDisp)
					{
						dblMaxDisp = dblDisp;

						if( pdwData[idataCount] < 0.0 )
						{
							bDispSwich = true;
						}
						else
						{
							bDispSwich = false;
						}
					}

					// 2019.07.02 구간 데이터의 최초, 최종 변위값을 설정하여, 최종 도출된 최대변위값이 해당 값과 같은지 비교하기 위함
					if( idataCount == 0 )
					{
						dblFirstDisp = dblDisp;
					}
					else if( idataCount == iCnt - 1)
					{
						dblLastDisp = dblDisp;
					}
					idataCount++;
					break;
				}
			}
			break;
		
		default :
			return 0;
		}
    	t = (AllCompData_t*) t->next;
    }

	if( cDkind == DEF_D_KIND_V )
	{
		pMaxPGV = dblMaxPGV;
	}
	else if( cDkind == DEF_D_KIND_A )
	{
		pMaxPGA = dblMaxPGA;
	}
	else if( cDkind == DEF_D_KIND_D )
	{
		pMaxDisp = dblMaxDisp;
	}

	t = (AllCompData_t*) g_WaveDataHead->next;

	idataCount = 0;
	dblPGA = 0.0;
	dblPGV = 0.0;
	dblDisp = 0.0;
	double dblMinPGA = 0.0;
	double dblMinDisp = 0.0;

	while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				switch( cDkind )
				{
				case DEF_D_KIND_C :
					break;
				case DEF_D_KIND_A :
					dblPGA = pdwData[idataCount];
					
					if( bAccSwich == true )
					{
						if( dblPGA > dblMinPGA )
						{
							dblMinPGA = dblPGA;
						}
					}
					else
					{
						if( dblPGA < dblMinPGA )
						{
							dblMinPGA = dblPGA;
						}
					}
					idataCount++;
					break;
				case DEF_D_KIND_V :
					break;
				case DEF_D_KIND_D :
					dblDisp = pdwData[idataCount];

					if( bDispSwich == true )
					{
						if( dblDisp > dblMinDisp )
						{
							dblMinDisp = dblDisp;
						}
					}
					else
					{
						if( dblDisp < dblMinDisp )
						{
							dblMinDisp = dblDisp;
						}
					}
					idataCount++;
					break;
				}
			}
			break;

		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				switch( cDkind )
				{
				case DEF_D_KIND_C :
					break;
				case DEF_D_KIND_A :
					dblPGA = pdwData[idataCount];

					if( bAccSwich == true )
					{
						if( dblPGA > dblMinPGA )
						{
							dblMinPGA = dblPGA;
						}
					}
					else
					{
						if( dblPGA < dblMinPGA )
						{
							dblMinPGA = dblPGA;
						}
					}
					idataCount++;
					break;
				case DEF_D_KIND_V :
					break;
				case DEF_D_KIND_D :
					dblDisp = pdwData[idataCount];

					if( bDispSwich == true )
					{
						if( dblDisp > dblMinDisp )
						{
							dblMinDisp = dblDisp;
						}
					}
					else
					{
						if( dblDisp < dblMinDisp )
						{
							dblMinDisp = dblDisp;
						}
					}
					idataCount++;
					break;
				}
			}
			break;

		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				switch( cDkind )
				{
				case DEF_D_KIND_C :
					break;
				case DEF_D_KIND_A :
					dblPGA = pdwData[idataCount];

					if( bAccSwich == true )
					{
						if( dblPGA > dblMinPGA )
						{
							dblMinPGA = dblPGA;
						}
					}
					else
					{
						if( dblPGA < dblMinPGA )
						{
							dblMinPGA = dblPGA;
						}
					}
					idataCount++;
					break;
				case DEF_D_KIND_V :
					break;
				case DEF_D_KIND_D :
					dblDisp = pdwData[idataCount];

					if( bDispSwich == true )
					{
						if( dblDisp > dblMinDisp )
						{
							dblMinDisp = dblDisp;
						}
					}
					else
					{
						if( dblDisp < dblMinDisp )
						{
							dblMinDisp = dblDisp;
						}
					}
					idataCount++;
					break;
				}
			}
			break;

		default :
			return 0;
		}
		t = (AllCompData_t*) t->next;
	}

	if( cDkind == DEF_D_KIND_A )
	{
		if( bAccSwich == true )
		{
			dblMinPGA = dblMinPGA * -1;
		}
	}
	else if( cDkind == DEF_D_KIND_D )
	{
		if( bDispSwich == true )
		{
			dblMinDisp = dblMinDisp * -1;
		}

		if( dblFirstDisp == dblMaxDisp || dblLastDisp == dblMaxDisp )
		{
			dblMaxDisp = -999;
		}
	}

#ifdef _DEBUG
	if( cDkind == DEF_D_KIND_A )
		TRACE("[%s] dblMinPGA = '%lf', dblMaxPGA = '%lf'\n", __FUNCTION__, dblMinPGA, dblMaxPGA);
	else if( cDkind == DEF_D_KIND_D )
		TRACE("[%s] dblMinDisp = '%lf', dblMaxDisp = '%lf'\n", __FUNCTION__, dblMinDisp, dblMaxDisp);
#endif	// DEBUG
	
	return idataCount;
}

//bool CheckPickDispValPoint(char cDType, char cDkind, double dblMaxDisp)
//{
//	bool bRet = false;
//
//	return bRet;
//}

AllCompData_t* GetFirstWaveData()
{
    return (AllCompData_t*)g_WaveDataHead->next;
}

AllCompData_t* GetNextWaveData(AllCompData_t* p)
{
    return (AllCompData_t*)p->next;
}

void Distory_WaveDataPool()
{
    AllCompData_t *p;
    AllCompData_t *s;
    p = (AllCompData_t*) g_WaveDataHead->next;

	//printf_log("## DataCount[%d]\n", g_iDataCount);

	int	iDelCnt = 0;
    
    while( p != NULL ) 
	{
		s = p;
    	p = (AllCompData_t*) p->next;
    	free(s);
		iDelCnt++;
    }
	
	free(g_WaveDataHead);

#ifdef _DEBUG
	//TRACE("## Distory WaveData List[%d]\n", iDelCnt);
#endif	// DEBUG
}

void AngleCorrcetion(float fAngle)
{
	AllCompData_t* t;
	double	dwAccE;
	double	dwAccN;
	
	t = (AllCompData_t*) g_WaveDataHead->next;
	
    while( t != NULL ) 
	{
		if(    ((t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
		    && ((t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON ) )
		{
			dwAccE = (t->dwAccE * cos(CFFT_Common::DegreesToRadians(fAngle))) + (t->dwAccN * sin(CFFT_Common::DegreesToRadians(fAngle)));
			dwAccN = (t->dwAccE * cos(CFFT_Common::DegreesToRadians(fAngle+90.0))) + (t->dwAccN * sin(CFFT_Common::DegreesToRadians(fAngle+90.0)));
			t->dwAccE = dwAccE;
			t->dwAccN = dwAccN;
		}
		
		t = (AllCompData_t*) t->next;
	}	
}

void VectorSynthesis()
{
	AllCompData_t* t;
	
	t = (AllCompData_t*) g_WaveDataHead->next;
	
    while( t != NULL ) 
	{
		if(    ((t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
		    && ((t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON ) )
		{
			t->dwAccNE = sqrt(pow(t->dwAccN, 2) + pow(t->dwAccE, 2));
			t->dwVelNE = sqrt(pow(t->dwVelN, 2) + pow(t->dwVelE, 2));
			t->dwDispNE = sqrt(pow(t->dwDispN, 2) + pow(t->dwDispE, 2));
		}
		else
		{
			t->dwAccNE = 0.;
			t->dwVelNE = 0.;
			t->dwDispNE = 0.;
		}
		
		if(    ((t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
		    && ((t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
		    && ((t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON ) )
		{
			t->dwAccZNE = sqrt(pow(t->dwAccZ, 2) + pow(t->dwAccN, 2) + pow(t->dwAccE, 2));
			t->dwVelZNE = sqrt(pow(t->dwVelZ, 2) + pow(t->dwVelN, 2) + pow(t->dwVelE, 2));
			t->dwDispZNE = sqrt(pow(t->dwDispZ, 2) + pow(t->dwDispN, 2) + pow(t->dwDispE, 2));
		}
		else
		{
			t->dwAccZNE = 0.;
			t->dwVelZNE = 0.;
			t->dwDispZNE = 0.;
		}
		
		t = (AllCompData_t*) t->next;
	}	
}

AllCompData_t* AddWaveData(long ldTime, char* szDate, char cDType, int iCount, double dwAcc)
{
    AllCompData_t *t;

/*
#ifdef	DEF_DEBUG	
		printf_log("Before AddWaveData [%c] [%s][%d][%f]\n"
				, cDType
				, szDate
				, iCount
				, dwAcc);
#endif
*/
	
	t = FindWaveData(ldTime);
	if( t != NULL )
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			t->iCountZ 	= iCount;
			t->dwAccZ	= dwAcc;
			t->cDataStatus |= DEF_ACC_Z_ON;
			break;
			
		case DEF_ACC_N :
		case DEF_ACC_Y :
			t->iCountN	= iCount;
			t->dwAccN	= dwAcc;
			t->cDataStatus |= DEF_ACC_N_ON;
			break;
			
		case DEF_ACC_E :
		case DEF_ACC_X :
			t->iCountE	= iCount;
			t->dwAccE	= dwAcc;
			t->cDataStatus |= DEF_ACC_E_ON;
			break;

		default :
			return NULL;
		}
/*		
#ifdef	DEF_DEBUG	
		printf_log("## UPDATE WaveData [%c][%s][%d][%f]\n"
				, cDType
				, szDate
				, iCount
				, dwAcc);
#endif
*/
		return t;
	}
	
	// add new WaveData
	t = (AllCompData_t*)malloc(sizeof(AllCompData_t));
	t->next = NULL;
	
	t->ldTime 	= ldTime;
	strcpy(t->szTime, szDate);
	t->iCountZ 	= 0;
	t->dwAccZ	= 0.;
	t->iCountN	= 0;
	t->dwAccN	= 0.;
	t->iCountE	= 0;
	t->dwAccE	= 0.;
	t->cDataStatus = 0x00;

	switch( cDType )
	{
	case DEF_ACC_Z :
		t->iCountZ 	= iCount;
		t->dwAccZ	= dwAcc;
		t->cDataStatus |= DEF_ACC_Z_ON;
		break;
		
	case DEF_ACC_N :
	case DEF_ACC_Y :
		t->iCountN	= iCount;
		t->dwAccN	= dwAcc;
		t->cDataStatus |= DEF_ACC_N_ON;
		break;
		
	case DEF_ACC_E :
	case DEF_ACC_X :
		t->iCountE	= iCount;
		t->dwAccE	= dwAcc;
		t->cDataStatus |= DEF_ACC_E_ON;
		break;

	default :
		free(t);
		return NULL;
	}

	/*
#ifdef	DEF_DEBUG	
	printf_log("## ADD WaveData [%c][%s][%d][%f]\n"
			, cDType
			, szDate
			, iCount
			, dwAcc);
#endif
	*/

	AllCompData_t*	b;
	AllCompData_t*	s;
	s = FindPosWaveData(t, ldTime);
	
	b = (AllCompData_t*) s->next;
	s->next = t;
	t->next = b;
/*
#ifdef	DEF_DEBUG	
	printf_log("### WaveDatas Count[%d]\n", GetWaveDataListCount());
#endif
*/
	//g_iDataCount++;
	
	return t;
}

AllCompData_t* AppendWaveData(long ldTime, char* szDate
							, int iCountZ, double dwAccZ
							, int iCountN, double dwAccN
							, int iCountE, double dwAccE
							, char cDataStatus)
{
    AllCompData_t *t;

	// add new WaveData
	t = (AllCompData_t*)malloc(sizeof(AllCompData_t));
	t->next = NULL;
	
	t->ldTime 	= ldTime;
	strcpy(t->szTime, szDate);
	t->iCountZ 	= iCountZ;
	t->dwAccZ	= dwAccZ;
	t->iCountN	= iCountN;
	t->dwAccN	= dwAccN;
	t->iCountE	= iCountE;
	t->dwAccE	= dwAccE;
	t->cDataStatus = cDataStatus;
	
	/*
	t->cDataStatus = DEF_ACC_Z_ON;
	t->cDataStatus |= DEF_ACC_N_ON;
	t->cDataStatus |= DEF_ACC_E_ON;
	*/
	
	if( g_WaveDataTail == NULL )
	{
		g_WaveDataHead->next = t;
		g_WaveDataTail = t;
	}
	else
	{
		g_WaveDataTail->next = t;
		g_WaveDataTail = t;
	}
/*
#ifdef	DEF_DEBUG	
	printf_log("### WaveDatas Count[%d]\n", GetWaveDataListCount());
#endif
*/
	//g_iDataCount++;
	
	return t;
}

char CheckChannelID(char cChan)
{
	if( cChan == 'Z' )
	{
		return DEF_ACC_Z;
	}
	else if( (cChan == 'N') || cChan == 'Y' )
	{
		return DEF_ACC_N;
	}
	else if( (cChan == 'E') || cChan == 'X' )
	{
		return DEF_ACC_E;
	}
	else
	{
		return DEF_ACC_U;
	}
}

// iMode 1: 1/sec sample, 0:All samples write
int	OutTogroundData(char* szOutFile, char cDType, int iMode)
{
	FILE *wFile = NULL;

	wFile = fopen(szOutFile, "w" );
	if( wFile == NULL )
	{
		TRACE("ERROR fopen(%s)\n", szOutFile);
		return 0;
	}

	char	szBuff[1024];
	sprintf(szBuff, "#time, count, Acc, Vel, Disp\n");
	fwrite(szBuff, strlen(szBuff), 1, wFile);

	long	ldTime	= 0;
	int		iCount	= 0;
	double	dwAcc	= 0;
	double	dwVel	= 0;
	double	dwDisp	= 0;


    AllCompData_t *t;
    t = (AllCompData_t*) g_WaveDataHead->next;
	
    while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountZ) )
					{
						iCount	= t->iCountZ;
					}
					
					if( dwAcc < fabs(t->dwAccZ) )
					{
						dwAcc	= t->dwAccZ;
					}

					if( dwVel < fabs(t->dwVelZ) )
					{
						dwVel	= t->dwVelZ;
					}
					
					if( dwDisp < fabs(t->dwDispZ) )
					{
						dwDisp	= t->dwDispZ;
					}
				}
				else
				{
					iCount	= t->iCountZ;
					dwAcc	= t->dwAccZ;
					dwVel	= t->dwVelZ;
					dwDisp	= t->dwDispZ;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;
		
		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountN) )
					{
						iCount	= t->iCountN;
					}
					
					if( dwAcc < fabs(t->dwAccN) )
					{
						dwAcc	= t->dwAccN;
					}

					if( dwVel < fabs(t->dwVelN) )
					{
						dwVel	= t->dwVelN;
					}
					
					if( dwDisp < fabs(t->dwDispN) )
					{
						dwDisp	= t->dwDispN;
					}
				}
				else
				{
					iCount	= t->iCountN;
					dwAcc	= t->dwAccN;
					dwVel	= t->dwVelN;
					dwDisp	= t->dwDispN;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;
			
		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountE) )
					{
						iCount	= t->iCountE;
					}
					
					if( dwAcc < fabs(t->dwAccE) )
					{
						dwAcc	= t->dwAccE;
					}

					if( dwVel < fabs(t->dwVelE) )
					{
						dwVel	= t->dwVelE;
					}
					
					if( dwDisp < fabs(t->dwDispE) )
					{
						dwDisp	= t->dwDispE;
					}
				}
				else
				{
					iCount	= t->iCountE;
					dwAcc	= t->dwAccE;
					dwVel	= t->dwVelE;
					dwDisp	= t->dwDispE;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;
		
		default :
			return 0;
		}
		
		if( iMode == 1 )
		{
			if( (t->ldTime % 100) == 0 )
			{
				sprintf(szBuff, "%.2f %d %.8f %.8f %.8f\n"
							, (t->ldTime / 100.0)
							, iCount
							, dwAcc
							, dwVel
							, dwDisp);
				iCount	= 0;
				dwAcc	= 0;
				dwVel	= 0;
				dwDisp	= 0;
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
		}
		else
		{
			sprintf(szBuff, "%.2f %d %.8f %.8f %.8f\n"
						, (t->ldTime / 100.0)
						, iCount
						, dwAcc
						, dwVel
						, dwDisp);
		}
		fwrite(szBuff, strlen(szBuff), 1, wFile);
    	t = (AllCompData_t*) t->next;
    }
	
	fclose(wFile);
	
	return 1;
}

// iMode 1: 1/sec sample, 0:All samples write
int	OutTogroundData_AccOnly(char* szOutFile, char cDType, int iMode)
{
	FILE *wFile = NULL;

	wFile = fopen(szOutFile, "w" );
	if( wFile == NULL )
	{
		TRACE("ERROR fopen(%s)\n", szOutFile);
		return 0;
	}

	char	szBuff[1024];

	long	ldTime	= 0;
	int		iCount	= 0;
	double	dwAcc	= 0;
	double	dwVel	= 0;
	double	dwDisp	= 0;
	
	AllCompData_t *t;
	t = (AllCompData_t*) g_WaveDataHead->next;

	while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountZ) )
					{
						iCount	= t->iCountZ;
					}

					if( dwAcc < fabs(t->dwAccZ) )
					{
						dwAcc	= t->dwAccZ;
					}

					if( dwVel < fabs(t->dwVelZ) )
					{
						dwVel	= t->dwVelZ;
					}

					if( dwDisp < fabs(t->dwDispZ) )
					{
						dwDisp	= t->dwDispZ;
					}
				}
				else
				{
					iCount	= t->iCountZ;
					dwAcc	= t->dwAccZ;
					dwVel	= t->dwVelZ;
					dwDisp	= t->dwDispZ;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;

		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountN) )
					{
						iCount	= t->iCountN;
					}

					if( dwAcc < fabs(t->dwAccN) )
					{
						dwAcc	= t->dwAccN;
					}

					if( dwVel < fabs(t->dwVelN) )
					{
						dwVel	= t->dwVelN;
					}

					if( dwDisp < fabs(t->dwDispN) )
					{
						dwDisp	= t->dwDispN;
					}
				}
				else
				{
					iCount	= t->iCountN;
					dwAcc	= t->dwAccN;
					dwVel	= t->dwVelN;
					dwDisp	= t->dwDispN;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;

		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountE) )
					{
						iCount	= t->iCountE;
					}

					if( dwAcc < fabs(t->dwAccE) )
					{
						dwAcc	= t->dwAccE;
					}

					if( dwVel < fabs(t->dwVelE) )
					{
						dwVel	= t->dwVelE;
					}

					if( dwDisp < fabs(t->dwDispE) )
					{
						dwDisp	= t->dwDispE;
					}
				}
				else
				{
					iCount	= t->iCountE;
					dwAcc	= t->dwAccE;
					dwVel	= t->dwVelE;
					dwDisp	= t->dwDispE;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;

		default :
			return 0;
		}

		if( iMode == 1 )
		{
			if( (t->ldTime % 100) == 0 )
			{
				sprintf(szBuff, "%.8f\n"
					, dwAcc);
				iCount	= 0;
				dwAcc	= 0;
				dwVel	= 0;
				dwDisp	= 0;
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
		}
		else
		{
			sprintf(szBuff, "%.8f\n"
				, dwAcc);
		}
		fwrite(szBuff, strlen(szBuff), 1, wFile);
		t = (AllCompData_t*) t->next;
	}

	fclose(wFile);

	return 1;
}

// iMode 1: 1/sec sample, 0:All samples write
int	OutTogroundData_VelOnly(char* szOutFile, char cDType, int iMode)
{
	FILE *wFile = NULL;

	wFile = fopen(szOutFile, "w" );
	if( wFile == NULL )
	{
		TRACE("ERROR fopen(%s)\n", szOutFile);
		return 0;
	}

	char	szBuff[1024];

	long	ldTime	= 0;
	int		iCount	= 0;
	double	dwAcc	= 0;
	double	dwVel	= 0;
	double	dwDisp	= 0;

	AllCompData_t *t;
	t = (AllCompData_t*) g_WaveDataHead->next;

	while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountZ) )
					{
						iCount	= t->iCountZ;
					}

					if( dwAcc < fabs(t->dwAccZ) )
					{
						dwAcc	= t->dwAccZ;
					}

					if( dwVel < fabs(t->dwVelZ) )
					{
						dwVel	= t->dwVelZ;
					}

					if( dwDisp < fabs(t->dwDispZ) )
					{
						dwDisp	= t->dwDispZ;
					}
				}
				else
				{
					iCount	= t->iCountZ;
					dwAcc	= t->dwAccZ;
					dwVel	= t->dwVelZ;
					dwDisp	= t->dwDispZ;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;

		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountN) )
					{
						iCount	= t->iCountN;
					}

					if( dwAcc < fabs(t->dwAccN) )
					{
						dwAcc	= t->dwAccN;
					}

					if( dwVel < fabs(t->dwVelN) )
					{
						dwVel	= t->dwVelN;
					}

					if( dwDisp < fabs(t->dwDispN) )
					{
						dwDisp	= t->dwDispN;
					}
				}
				else
				{
					iCount	= t->iCountN;
					dwAcc	= t->dwAccN;
					dwVel	= t->dwVelN;
					dwDisp	= t->dwDispN;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;

		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountE) )
					{
						iCount	= t->iCountE;
					}

					if( dwAcc < fabs(t->dwAccE) )
					{
						dwAcc	= t->dwAccE;
					}

					if( dwVel < fabs(t->dwVelE) )
					{
						dwVel	= t->dwVelE;
					}

					if( dwDisp < fabs(t->dwDispE) )
					{
						dwDisp	= t->dwDispE;
					}
				}
				else
				{
					iCount	= t->iCountE;
					dwAcc	= t->dwAccE;
					dwVel	= t->dwVelE;
					dwDisp	= t->dwDispE;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;

		default :
			return 0;
		}

		if( iMode == 1 )
		{
			if( (t->ldTime % 100) == 0 )
			{
				sprintf(szBuff, "%.8f\n"
					, dwVel);
				iCount	= 0;
				dwAcc	= 0;
				dwVel	= 0;
				dwDisp	= 0;
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
		}
		else
		{
			sprintf(szBuff, "%.8f\n"
				, dwVel);
		}
		fwrite(szBuff, strlen(szBuff), 1, wFile);
		t = (AllCompData_t*) t->next;
	}

	fclose(wFile);

	return 1;
}

// iMode 1: 1/sec sample, 0:All samples write
int	OutTogroundData_DispOnly(char* szOutFile, char cDType, int iMode)
{
	FILE *wFile = NULL;

	wFile = fopen(szOutFile, "w" );
	if( wFile == NULL )
	{
		TRACE("ERROR fopen(%s)\n", szOutFile);
		return 0;
	}

	char	szBuff[1024];

	long	ldTime	= 0;
	int		iCount	= 0;
	double	dwAcc	= 0;
	double	dwVel	= 0;
	double	dwDisp	= 0;

	AllCompData_t *t;
	t = (AllCompData_t*) g_WaveDataHead->next;

	while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountZ) )
					{
						iCount	= t->iCountZ;
					}

					if( dwAcc < fabs(t->dwAccZ) )
					{
						dwAcc	= t->dwAccZ;
					}

					if( dwVel < fabs(t->dwVelZ) )
					{
						dwVel	= t->dwVelZ;
					}

					if( dwDisp < fabs(t->dwDispZ) )
					{
						dwDisp	= t->dwDispZ;
					}
				}
				else
				{
					iCount	= t->iCountZ;
					dwAcc	= t->dwAccZ;
					dwVel	= t->dwVelZ;
					dwDisp	= t->dwDispZ;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;

		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountN) )
					{
						iCount	= t->iCountN;
					}

					if( dwAcc < fabs(t->dwAccN) )
					{
						dwAcc	= t->dwAccN;
					}

					if( dwVel < fabs(t->dwVelN) )
					{
						dwVel	= t->dwVelN;
					}

					if( dwDisp < fabs(t->dwDispN) )
					{
						dwDisp	= t->dwDispN;
					}
				}
				else
				{
					iCount	= t->iCountN;
					dwAcc	= t->dwAccN;
					dwVel	= t->dwVelN;
					dwDisp	= t->dwDispN;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;

		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				if( iMode == 1 )
				{
					if( iCount < abs(t->iCountE) )
					{
						iCount	= t->iCountE;
					}

					if( dwAcc < fabs(t->dwAccE) )
					{
						dwAcc	= t->dwAccE;
					}

					if( dwVel < fabs(t->dwVelE) )
					{
						dwVel	= t->dwVelE;
					}

					if( dwDisp < fabs(t->dwDispE) )
					{
						dwDisp	= t->dwDispE;
					}
				}
				else
				{
					iCount	= t->iCountE;
					dwAcc	= t->dwAccE;
					dwVel	= t->dwVelE;
					dwDisp	= t->dwDispE;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;

		default :
			return 0;
		}

		if( iMode == 1 )
		{
			if( (t->ldTime % 100) == 0 )
			{
				sprintf(szBuff, "%.8f\n"
					, dwDisp);
				iCount	= 0;
				dwAcc	= 0;
				dwVel	= 0;
				dwDisp	= 0;
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
		}
		else
		{
			sprintf(szBuff, "%.8f\n"
				, dwDisp);
		}
		fwrite(szBuff, strlen(szBuff), 1, wFile);
		t = (AllCompData_t*) t->next;
	}

	fclose(wFile);

	return 1;
}

// detrend for QSCD style
#define	DEF_DTRAND_SEC	10
int	dtrend4MMA(char cDType)
{
	long	ldTime		= 0;
	int		iAccMin		= 0.0;
	int		iAccMax		= 0.0;
	int		iAccAvg		= 0.0;

	int		iAcc10Avg		= 0.0;
	int		iAcc10AvgSum	= 0.0;
	int		iAccAvgAry[DEF_DTRAND_SEC];
	int		i;
	int		initFlag = 1;
	
	iAcc10AvgSum = 0.0;
	for( i=0; i<DEF_DTRAND_SEC; i++ )
	{
		iAccAvgAry[i] = 0;
	}
	
    AllCompData_t *t;
    t = (AllCompData_t*) g_WaveDataHead->next;
	
    while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				if( initFlag == 1 )
				{
					iAccMin	= t->iCountZ;
					iAccMax	= t->iCountZ;
					initFlag = 0;
				}
				else
				{
					if( iAccMin > t->iCountZ )
					{
						iAccMin	= t->iCountZ;
					}
					if( iAccMax < t->iCountZ )
					{
						iAccMax	= t->iCountZ;
					}
				}

				if( iAcc10Avg > 0 )
				{
					t->iCountZ -= iAcc10Avg;
				}
				else
				{
					t->iCountZ += iAcc10Avg;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;
		
		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				if( initFlag == 1 )
				{
					iAccMin	= t->iCountN;
					iAccMax	= t->iCountN;
					initFlag = 0;
				}
				else
				{
					if( iAccMin > t->iCountN )
					{
						iAccMin	= t->iCountN;
					}
					if( iAccMax < t->iCountN )
					{
						iAccMax	= t->iCountN;
					}
				}

				if( iAcc10Avg > 0 )
				{
					t->iCountN -= iAcc10Avg;
				}
				else
				{
					t->iCountN += iAcc10Avg;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;
			
		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				if( initFlag == 1 )
				{
					iAccMin	= t->iCountE;
					iAccMax	= t->iCountE;
					initFlag = 0;
				}
				else
				{
					if( iAccMin > t->iCountE )
					{
						iAccMin	= t->iCountE;
					}
					if( iAccMax < t->iCountE )
					{
						iAccMax	= t->iCountE;
					}
				}

				if( iAcc10Avg > 0 )
				{
					t->iCountE -= iAcc10Avg;
				}
				else
				{
					t->iCountE += iAcc10Avg;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;
		
		default :
			return 0;
		}
		
		if( (t->ldTime % 100) == 0 )
		{
			iAccAvg = (iAccMax + iAccMin) / 2;

			// avg moving
			for( i=0; i<(DEF_DTRAND_SEC-1); i++ )
			{
				iAccAvgAry[i] = iAccAvgAry[i+1];
			}
			iAccAvgAry[DEF_DTRAND_SEC-1] = iAccAvg;
			
			iAcc10AvgSum = 0.0;
			for( i=0; i<DEF_DTRAND_SEC; i++ )
			{
				iAcc10AvgSum += iAccAvgAry[i];
			}
			iAcc10Avg = iAcc10AvgSum / DEF_DTRAND_SEC;

			iAccMax = 0;
			iAccMin = 0;
			initFlag = 1;
		}
    	t = (AllCompData_t*) t->next;
    }
	
	return 1;
}


// iMode 1: 1/sec sample, 0:All samples write
int	OutTogroundData4NEMA(char* szOutFile, char cDType, int iMode)
{
	FILE *wFile = NULL;

	wFile = fopen(szOutFile, "w" );
	if( wFile == NULL )
	{
		TRACE("ERROR fopen(%s)\n", szOutFile);
		return 0;
	}

	char	szBuff[1024];
	sprintf(szBuff, "#epoch time, time, count, Acc\n");
	fwrite(szBuff, strlen(szBuff), 1, wFile);

	long	ldTime	= 0;
	int		iCount	= 0;
	double	dwAcc	= 0;
	double	dwVel	= 0;
	double	dwDisp	= 0;


    AllCompData_t *t;
    t = (AllCompData_t*) g_WaveDataHead->next;
	
    while( t != NULL ) 
	{
		switch( cDType )
		{
		case DEF_ACC_Z :
			if( (t->cDataStatus & DEF_ACC_Z_ON) == DEF_ACC_Z_ON )
			{
				if( iMode == 1 )
				{
					if( abs(iCount) < abs(t->iCountZ) )
					{
						iCount	= t->iCountZ;
					}
					
					if( fabs(dwAcc) < fabs(t->dwAccZ) )
					{
						dwAcc	= t->dwAccZ;
					}

					if( fabs(dwVel) < fabs(t->dwVelZ) )
					{
						dwVel	= t->dwVelZ;
					}
					
					if( fabs(dwDisp) < fabs(t->dwDispZ) )
					{
						dwDisp	= t->dwDispZ;
					}
				}
				else
				{
					iCount	= t->iCountZ;
					dwAcc	= t->dwAccZ;
					dwVel	= t->dwVelZ;
					dwDisp	= t->dwDispZ;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;
		
		case DEF_ACC_N :
		case DEF_ACC_Y :
			if( (t->cDataStatus & DEF_ACC_N_ON) == DEF_ACC_N_ON )
			{
				if( iMode == 1 )
				{
					if( abs(iCount) < abs(t->iCountN) )
					{
						iCount	= t->iCountN;
					}
					
					if( fabs(dwAcc) < fabs(t->dwAccN) )
					{
						dwAcc	= t->dwAccN;
					}

					if( fabs(dwVel) < fabs(t->dwVelN) )
					{
						dwVel	= t->dwVelN;
					}
					
					if( fabs(dwDisp) < fabs(t->dwDispN) )
					{
						dwDisp	= t->dwDispN;
					}
				}
				else
				{
					iCount	= t->iCountN;
					dwAcc	= t->dwAccN;
					dwVel	= t->dwVelN;
					dwDisp	= t->dwDispN;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;
			
		case DEF_ACC_E :
		case DEF_ACC_X :
			if( (t->cDataStatus & DEF_ACC_E_ON) == DEF_ACC_E_ON )
			{
				if( iMode == 1 )
				{
					if( abs(iCount) < abs(t->iCountE) )
					{
						iCount	= t->iCountE;
					}
					
					if( fabs(dwAcc) < fabs(t->dwAccE) )
					{
						dwAcc	= t->dwAccE;
					}

					if( fabs(dwVel) < fabs(t->dwVelE) )
					{
						dwVel	= t->dwVelE;
					}
					
					if( fabs(dwDisp) < fabs(t->dwDispE) )
					{
						dwDisp	= t->dwDispE;
					}
				}
				else
				{
					iCount	= t->iCountE;
					dwAcc	= t->dwAccE;
					dwVel	= t->dwVelE;
					dwDisp	= t->dwDispE;
				}
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
			break;
		
		default :
			return 0;
		}
		
		if( iMode == 1 )
		{
			if( (t->ldTime % 100) == 0 )
			{
                char szHumanTime[100];
                double  dwtimeD;
				double  dwtimeD_UTC;
                int     itimeD;

				dwtimeD_UTC = (t->ldTime / 100.0);
                dwtimeD = (t->ldTime / 100.0) + (60 * 60 * 9);
                itimeD  = (int)dwtimeD;
				CFFT_Common::stamp2HumanStringT(itimeD, szHumanTime);

                itimeD = (dwtimeD - itimeD) * 100;
                
				sprintf(szBuff, "%.2f %s %d %.8f\n"
							, dwtimeD_UTC
							, szHumanTime
							, abs(iCount)
							, fabs(dwAcc));

                /*
				sprintf(szBuff, "%.2f %d %.8f %.8f %.8f\n"
							, (t->ldTime / 100.0)
							, abs(iCount)
							, fabs(dwAcc)
							, dwVel
							, dwDisp);
                */
				iCount	= 0;
				dwAcc	= 0;
				dwVel	= 0;
				dwDisp	= 0;
			}
			else
			{
				t = (AllCompData_t*) t->next;
				continue;
			}
		}
		else
		{
            char szHumanTime[100];
            double  dwtimeD;
            double  dwtimeD_UTC;
            int     itimeD;

			dwtimeD_UTC = (t->ldTime / 100.0);
            dwtimeD = (t->ldTime / 100.0) + (60 * 60 * 9);
            itimeD  = (int)dwtimeD;
			CFFT_Common::stamp2HumanStringT(itimeD, szHumanTime);

            itimeD = (dwtimeD - itimeD) * 100;
            
		    sprintf(szBuff, "%.2f %s.%02d %d %.8f\n"
							, dwtimeD_UTC
							, szHumanTime
                            , itimeD
							, abs(iCount)
							, fabs(dwAcc));

            /*
			sprintf(szBuff, "%.2f %d %.8f %.8f %.8f\n"
						, (t->ldTime / 100.0)
						, iCount
						, dwAcc
						, dwVel
						, dwDisp);
            */
		}
		fwrite(szBuff, strlen(szBuff), 1, wFile);
    	t = (AllCompData_t*) t->next;
    }
	
	fclose(wFile);
	
	return 1;
}
