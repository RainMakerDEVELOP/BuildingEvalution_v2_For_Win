#ifndef ___WaveData_Manager__H__
#define ___WaveData_Manager__H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define	DEF_TIME_LEN	30

typedef struct 
{
	char 	cDType;
	
	__int64	ldTime;
	char	szTime[DEF_TIME_LEN];

	int		iCount;
	double	dwAcc;
	double	dwVel;
	double	dwDisp;
	double	dwRKtime;	// RungeKutta 알고리즘 실행용 time
	
	//void* 	next;
} ChanData_t;

typedef struct AllCompData
{
	long	ldTime;
	char	szTime[DEF_TIME_LEN];
	char	cDataStatus;

	int		iCountZ;
	int		iCountN;
	int		iCountE;
	int		iCountNE;
	int		iCountZNE;
	
	double	dwAccZ;
	double	dwAccN;
	double	dwAccE;
	double	dwAccNE;
	double	dwAccZNE;

	double	dwVelZ;
	double	dwVelN;
	double	dwVelE;
	double	dwVelNE;
	double	dwVelZNE;

	double	dwDispZ;
	double	dwDispN;
	double	dwDispE;
	double	dwDispNE;
	double	dwDispZNE;

	double	dwRKtime;	// RungeKutta 알고리즘 실행용 time
	double	dwMaxPGA;	// 최대 가속도
	double	dwMaxPGV;	// 최대 속도
	double	dwMaxDisp;	// 최대 변위
	
	void* 	next;
} AllCompData_t;


void Init_WaveDataPool();
int CachingWaveData(long ldTime, char* szDate, char cDType, int iCount, double dwGal);
int ReleaseCacheWaveData(int ifinalFlag);
AllCompData_t* AddUpdateChanData(AllCompData_t* pBaseD, ChanData_t* pChanData);
AllCompData_t* FindWaveData(long ldTime);
AllCompData_t* FindNextWaveData(AllCompData_t* p, long ldTime);
AllCompData_t* FindPosWaveData(AllCompData_t *f, long ldTime);
int GetWaveDataListCount();
int GetRealDataCount(char cDType);
int GetRealData(char cDType, double* pdwData, int iCnt);
int GetCountData(char cDType, int* piData, int iCnt);
int GetRKtimeData(char cDType, double* pdwData, int iCnt);
int UpdateRealData(char cDType, char cDkind, double* pdwData, int iCnt, double &pMaxPGA, double &pMaxPGV, double &pMaxDisp);
//BOOL CheckPickDispValPoint(char cDType, char cDkind, double dblMaxDisp);	// 2019.07.02 최대변위값이 구간자료의 최초 또는 최종 데이터인지 확인
AllCompData_t* GetFirstWaveData();
AllCompData_t* GetNextWaveData(AllCompData_t* p);
void Distory_WaveDataPool();
void AngleCorrcetion(float fAngle);
void VectorSynthesis();

AllCompData_t* AddWaveData(long ldTime, char* szDate, char cDType, int iCount, double dwGal);
AllCompData_t* AppendWaveData(long ldTime, char* szDate, int iCountZ, double dwGalZ, int iCountN, double dwGalN, int iCountE, double dwGalE, char cDataStatus);

char CheckChannelID(char cChan);
int	OutTogroundData(char* szOutFile, char cDType, int iMode);
int	OutTogroundData_AccOnly(char* szOutFile, char cDType, int iMode);
int	OutTogroundData_VelOnly(char* szOutFile, char cDType, int iMode);
int	OutTogroundData_DispOnly(char* szOutFile, char cDType, int iMode);

#endif // ___WaveData_Manager__H__
