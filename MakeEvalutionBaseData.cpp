#include "stdafx.h"
#include "MakeEvalutionBaseData.h"
#include "wavedataManager.h"
#include "mseed2Gascii.h"
#include <math.h>
#include <complex>
#include <xcomplex>
#include <iostream>
#include <fstream>
#include <direct.h>

//using namespace std;

// fftw ���̺귯�� ���� �Ʒ� �ڵ� �ʿ�
#ifdef DEF_FFT_LIB
#include "../fftw-3.3.5-dll64/fftw3.h"
// ������Ʈ �߰� ���Ӽ��� �߰� �׸� : ..\Lib\libfftw3-3.lib;..\Lib\libfftw3f-3.lib;..\Lib\libfftw3l-3.lib
#endif	// DEF_FFT_LIB


CMakeEvalutionBaseData::CMakeEvalutionBaseData(void)
{
	m_pMainDlgHwnd = NULL;
	iniFunc = NULL;
	logFunc = NULL;

	//m_nListAEMaxWidth = 0;

	m_strDitc = "";
	m_nWarningLimit = 0;
	m_nMinAvailChanCnt = 0;
	m_sEventID = "";
	m_sExecProcKind = "";

	m_strAcc_Val = "";
	m_strSp_Val = "";
	m_strDis_Val = "";
	m_strCav_Val = "";
	m_strStm_Val = "";
	m_strTot_Result = "";
}


CMakeEvalutionBaseData::~CMakeEvalutionBaseData(void)
{
	if (GetStationCount() > 0)
	{
		for (int i = GetStationCount(); 0 <= i; --i)
		{
			Sta_t* pSta = (Sta_t*)m_aryStation.GetAt(i);
			if (pSta == NULL) continue;

			DelAllStationDetail(pSta);
			delete pSta->pArySta_Detail;
			delete pSta;
			m_aryStation.RemoveAt(i);
		}
	}

	if (GetThStationCount() > 0)
	{
		for (int i = GetThStationCount(); 0 <= i; --i)
		{
			ThSta_t* pThSta = (ThSta_t*)m_aryThStation.GetAt(i);
			if (pThSta == NULL) continue;

			DelAllThStationDetail(pThSta);
			delete pThSta->pAryThSta_Detail;
			delete pThSta;
			m_aryThStation.RemoveAt(i);
		}
	}
}

int CMakeEvalutionBaseData::MakeBaseDataProc(char* sDitc, char* sEventID, char* sMseedPath, char* sNet, char* sSta)
{
	int			nRet = 0;
	CString	strLog = "";
	char		szOutFile[2048];
	double	dwSensitivity = 0;
	double	dwResponse = 0;
	double	dwCalib = 0;

	strLog = "";
	strLog.Format("Building Evalution Process Start!!");
	WriteReportLog(strLog);

	if( _access(sMseedPath, 0x00) != 0 )
	{
		strLog.Format("Access Failed MseedPath = '%s'", sMseedPath);
		WriteReportLog(strLog);

#ifdef _DEBUG
		TRACE("Access Failed MseedPath : %s\n", sMseedPath);
#endif	// DEBUG
		return nRet;
	}

	CString strLoc = "";
	CString strLoc_FileNameOnly = "";
	CString strOutFilePath = "";
	CString strLoc_MCOUNTName = "";
	char	sSrcFileFullPath[1024];

	Sta_t* pSta = NULL;
	Sta_Detail_t* pStaDetail = NULL;	

	char sSenId[8];
	char sChannelId[4];	
	char sObsSenName[12];

	double dblSettlement = 0.0;
	double dblCAV = 0.0;

	int	nResSpectrum = -1;

	// 2019.04.19 ���� ���ȭ�� �Ʒ� �ּ� ����(StationInfo.txt �д� �κ�) START
	//CString strStationInfoFileFullPath = "";
	//strStationInfoFileFullPath.Format("%s\\%s", sMseedPath, STATIONINFO_TXT);
	//int nReadRet = ReadStationInfo2Array(strStationInfoFileFullPath);
	// 2019.04.19 ���� ���ȭ�� �Ʒ� �ּ� ����(StationInfo.txt �д� �κ�) END

	// 2019.04.19 ���� ���ȭ�� �Ʒ� �ּ� ����(type{�ü��������ڵ�}_thInfo.txt �д� �κ�) START
	//CString strThresholdsInfoPath = "";
	//strThresholdsInfoPath.Format("%s", sMseedPath);
	//int nReadRet = ReadThresholdsInfo2Array(strThresholdsInfoPath, m_strDitc);
	// 2019.04.19 ���� ���ȭ�� �Ʒ� �ּ� ����(type{�ü��������ڵ�}_thInfo.txt �д� �κ�) END

	double dblLowFreq = 0.0;
	double dblHighFreq = 0.0;

	// BandPass ���͸��� ���� �ü����� Low Freq, High Freq �� ����
	if( sDitc[0] == 'K' )		// ��ũ��Ʈ��
	{

	}
	else if( sDitc[0] == 'D' )	// �ʴ� �� ������
	{
		//// 2019.06.10 �ӵ��� ����ϴ� �˰��� ���� �ּ� ����
		//dblLowFreq = 0.1;
		//dblHighFreq = 50.0;
	}
	else if( sDitc[0] == 'S' || sDitc[0] == 'B' )	// ����-���屳, ����-������
	{
		dblLowFreq = 0.1;
		dblHighFreq = 40.0;
	}

	strOutFilePath.Format("%s\\%s", sMseedPath, PATH_RESULT);
	CreatePathDirectory(strOutFilePath);

	char sOutFilePath[1024];
	memset(sOutFilePath, 0x00, sizeof(sOutFilePath));
	sprintf(sOutFilePath, "%s\\%s", sMseedPath, PATH_RESULT);
	DeleteAllFile(sOutFilePath, sNet, sSta);		// RESULT ���� ����� ���� ��ü ����

	// file find
	CFileFind finder;
	BOOL bWorking = FALSE;

	// 2019.03.29 MCOUNT ���� ���� �κ� �߰�
	char sLoc[1024];
	char sLoc_MCOUNT[1024];
	char sLocFullPath[2048];
	char sLoc_MCOUNTFullPath[2048];

	CString cpFilePath = "";
	cpFilePath.Format("%s\\%s_%s*%s", sMseedPath, sNet, sSta, DEF_MSEED_EXT);

	bWorking = finder.FindFile(cpFilePath);
	
	while( bWorking )
	{
		dblCAV = 0.0;

		bWorking = finder.FindNextFile();
		strLoc = finder.GetFileName();
		strLoc_FileNameOnly = strLoc;//.Replace(".mseed", "");
		strLoc_FileNameOnly.Replace(".mseed", "");
		strLoc_MCOUNTName = strLoc;
		strLoc_MCOUNTName.Replace(".mseed", "_MCOUNT.txt");

		memset(sLoc, 0x00, sizeof(sLoc));
		memset(sLocFullPath, 0x00, sizeof(sLocFullPath));
		sprintf(sLoc, "%s", strLoc);
		sprintf(sLocFullPath, "%s\\%s", sMseedPath, sLoc);

		memset(sLoc_MCOUNT, 0x00, sizeof(sLoc_MCOUNT));		
		memset(sLoc_MCOUNTFullPath, 0x00, sizeof(sLoc_MCOUNTFullPath));
		sprintf(sLoc_MCOUNT, "%s", strLoc_MCOUNTName);
		sprintf(sLoc_MCOUNTFullPath, "%s\\%s", strOutFilePath, sLoc_MCOUNT);
		
		memset(sSenId, 0x00, sizeof(sSenId));
		memset(sChannelId, 0x00, sizeof(sChannelId));
		sprintf(sSenId, "%s", strLoc.Mid(0, 6));
		sprintf(sChannelId, "%s", strLoc.Mid(7, 3));

		if(sChannelId[0] != 'H')		continue;

		if (GetStationCount() > 0)
		{
			BOOL bSetDataYN = FALSE;

			for (int i = 0; i < GetStationCount(); i++)
			{
				if(bSetDataYN == TRUE)
				{
					break;
				}

				pSta = (Sta_t*)m_aryStation.GetAt(i);
				if (pSta == NULL) continue;
				if (strcmp(pSta->sSen_Id, sSenId) == 0 && pSta->bDataAvailability == true)
				{
					for (int j = 0; j < pSta->pArySta_Detail->GetCount(); j++)
					{
						pStaDetail = (Sta_Detail_t*)pSta->pArySta_Detail->GetAt(j);
						if (pStaDetail == NULL) continue;

						if (sChannelId[2] == 'Z')
						{
							dwResponse = (float)(atof(pStaDetail->sZresponse));
							dwSensitivity = (float)(atof(pStaDetail->sZsensitivity));
							bSetDataYN = TRUE;
						}
						else if (sChannelId[2] == 'N' || sChannelId[2] == 'Y')
						{
							dwResponse = (float)(atof(pStaDetail->sNresponse));
							dwSensitivity = (float)(atof(pStaDetail->sNsensitivity));
							bSetDataYN = TRUE;
						}
						else if (sChannelId[2] == 'E' || sChannelId[2] == 'X')
						{
							dwResponse = (float)(atof(pStaDetail->sEresponse));
							dwSensitivity = (float)(atof(pStaDetail->sEsensitivity));
							bSetDataYN = TRUE;
						}
						else
						{
							dwResponse = (float)(atof(pStaDetail->sZresponse));
							dwSensitivity = (float)(atof(pStaDetail->sZsensitivity));
							bSetDataYN = TRUE;
						}
						break;
					}					
				}
			}

			if( bSetDataYN == FALSE )
			{
				strLog = "";
				strLog.Format("sen = '%s', channel = '%s', dataChk = '%d'", sSenId, sChannelId, bSetDataYN);
				WriteReportLog(strLog);
			}
		}		

#ifdef _DEBUG
		//if (sDitc[0] == 'S' || sDitc[0] == 'B')
		//{

		//}
		//else if (sDitc[0] == 'K')
		//{

		//}
		//else if (sDitc[0] == 'D')
		//{

		//}
		//else
		//{
		//	if (pSta == NULL || pStaDetail == NULL)
		//	{
		//		// ���п� ���� ���� ����
		//		continue;
		//	}
		//}
		if( pSta == NULL || pStaDetail == NULL || (int)dwSensitivity == 0 || (int)dwResponse == 0 )
		{
			// ���п� ���� ���� ����
			continue;
		}
#else
		if( pSta == NULL || pStaDetail == NULL || (int)dwSensitivity == 0 || (int)dwResponse == 0 )
		{
			// ���п� ���� ���� ����
			continue;
		}
#endif	// DEBUG

		dwCalib = (((dwSensitivity * 0.000001) / dwResponse) / 9.81) * 981;

#ifdef _DEBUG
		//if( sDitc[0] == 'S' || sDitc[0] == 'B' )	// Ư������ �ϵ��뱳 ������ �׽�Ʈ ����ġ
		//{
		//	dwCalib = 0.000234;
		//}
		//else if( sDitc[0] == 'K' )	// ��ũ��Ʈ�� �׽�Ʈ�� �ϵ��뱳 ������ ����� ����
		//{
		//	dwCalib = 0.000234;
		//}
		//else if( sDitc[0] == 'D' )	// �ʴ� �ӽ� �׽�Ʈ�� ���� Calibration �� ����
		//{
		//	//dwCalib = 0.000234;
		//}
#endif	// DEBUG

		CMseed2Gascii mseed2Gascii;
		if(mseed2Gascii.MakeMseed2Gascii(sLocFullPath, sLoc_MCOUNTFullPath, dwCalib) != 0)
		{
			strLog = "";
			strLog.Format("Make Count Data Failed!! sen = '%s', channel = '%s'", sSenId, sChannelId);
			WriteReportLog(strLog);

			continue;
		}
	}
	
	// MCOUNT ���Ϸ� deTrend, cosineTapering, TogroundTSPModule(�ִ밡�ӵ�, �ִ����ݼӵ�, ���� ���� - 1999 ���׸𸮽� ����) ���������� �����Ͽ� ������ ����
	bWorking = FALSE;
	cpFilePath = "";
	strLoc = "";
	strLoc_FileNameOnly = "";

	cpFilePath.Format("%s\\%s_%s*%s", strOutFilePath, sNet, sSta, DEF_MSEED_COUNT);

	bWorking = finder.FindFile(cpFilePath);

	if( bWorking == 0 )
	{
		strLog = "";
		strLog.Format("Not found MCOUNT file!!\n");
		WriteReportLog(strLog);

		return nRet;
	}

#ifdef _DEBUG
	TRACE("111 pSta->sSen_Id = '%s', dwResponse = '%lf', dwSensitivity = '%lf', dwCalib = '%lf'\n", pSta->sSen_Id, dwResponse, dwSensitivity, dwCalib);
#endif	// DEBUG

	while( bWorking )
	{
		dblCAV = 0.0;

		bWorking = finder.FindNextFile();
		strLoc = finder.GetFileName();
		strLoc_FileNameOnly = strLoc;//.Replace(".mseed", "");
		strLoc_FileNameOnly.Replace("_MCOUNT.txt", "");

		memset(sSrcFileFullPath, 0x00, sizeof(sSrcFileFullPath));
		sprintf(sSrcFileFullPath, "%s\\%s", strOutFilePath, strLoc);

		//char sSenId[8];
		//char sChannelId[4];
		char sLocation[1024];
		char sFileNameOnly[1024];
		memset(sSenId, 0x00, sizeof(sSenId));
		memset(sChannelId, 0x00, sizeof(sChannelId));
		memset(sObsSenName, 0x00, sizeof(sObsSenName));
		memset(sLocation, 0x00, sizeof(sLocation));
		memset(sFileNameOnly, 0x00, sizeof(sFileNameOnly));

		sprintf(sSenId, "%s", strLoc.Mid(0, 6));
		sprintf(sChannelId, "%s", strLoc.Mid(7, 3));
		sprintf(sObsSenName, "%s", strLoc.Mid(0, 10));
		sprintf(sLocation, "%s", strLoc);
		sprintf(sFileNameOnly, "%s", strLoc_FileNameOnly);
		sprintf(szOutFile, "%s\\%s", strOutFilePath, sFileNameOnly);

		if(sChannelId[0] != 'H')		continue;

		//FILE *wFile = NULL;

		//wFile = fopen(szOutFile, "w" );
		//if( wFile == NULL )
		//{
		//	TRACE("ERROR fopen(%s)\n", szOutFile);
		//	continue;
		//}

		//if( _access(szOutFile, 0x00) != 0 )
		//{
		//	TRACE("Error create count file[%s]\n", szOutFile);
		//	continue;
		//}

		Init_WaveDataPool();

		char	cChanID;
		// 0123456789012345678901234
		// GK_OSV_HCY_20161113215257
		cChanID = CheckChannelID(sFileNameOnly[9]);
		if( ReadChanData(sSrcFileFullPath, cChanID) != 1 )
		//if( ReadMseedData2ChanStruct(sSrcFileFullPath, cChanID) != 1 )
		{
			strLog = "";
			strLog.Format("Read Channel Data Error!! path = '%s', channel = '%s'", sSrcFileFullPath, cChanID);
			WriteReportLog(strLog);

			Distory_WaveDataPool();
			remove(szOutFile);
			return nRet;
		}
		//remove(szOutFile);

		// StationInfo.txt ���� �д� �κ�(�� ������ Calib ������)
		pSta = NULL;
		pStaDetail = NULL;
		//strStationInfoFileFullPath = "";

		//strStationInfoFileFullPath.Format("%s\\%s", sMseedPath, STATIONINFO_TXT);
		//int nReadRet = ReadStationInfo2Array(strStationInfoFileFullPath);

		if (GetStationCount() > 0)
		{
			BOOL bSetDataYN = FALSE;

			for (int i = 0; i < GetStationCount(); i++)
			{
				if(bSetDataYN == TRUE)
				{
					break;
				}

				pSta = (Sta_t*)m_aryStation.GetAt(i);
				if (pSta == NULL) continue;
				if (strcmp(pSta->sSen_Id, sSenId) == 0 && pSta->bDataAvailability == true)
				{
					for (int j = 0; j < pSta->pArySta_Detail->GetCount(); j++)
					{
						pStaDetail = (Sta_Detail_t*)pSta->pArySta_Detail->GetAt(j);
						if (pStaDetail == NULL) continue;

						if (sChannelId[2] == 'Z')
						{
							dwResponse = (float)(atof(pStaDetail->sZresponse));
							dwSensitivity = (float)(atof(pStaDetail->sZsensitivity));
							bSetDataYN = TRUE;
						}
						else if (sChannelId[2] == 'N' || sChannelId[2] == 'Y')
						{
							dwResponse = (float)(atof(pStaDetail->sNresponse));
							dwSensitivity = (float)(atof(pStaDetail->sNsensitivity));
							bSetDataYN = TRUE;
						}
						else if (sChannelId[2] == 'E' || sChannelId[2] == 'X')
						{
							dwResponse = (float)(atof(pStaDetail->sEresponse));
							dwSensitivity = (float)(atof(pStaDetail->sEsensitivity));
							bSetDataYN = TRUE;
						}
						else
						{
							dwResponse = (float)(atof(pStaDetail->sZresponse));
							dwSensitivity = (float)(atof(pStaDetail->sZsensitivity));
							bSetDataYN = TRUE;
						}
						break;
					}					
				}
			}
		}

#ifdef _DEBUG
		TRACE("222 pSta->sSen_Id = '%s', sObsSenName = '%s', dwResponse = '%lf', dwSensitivity = '%lf', dwCalib = '%lf'\n", pSta->sSen_Id, sObsSenName, dwResponse, dwSensitivity, dwCalib);
#endif	// DEBUG

#ifdef _DEBUG
		//if (sDitc[0] == 'S' || sDitc[0] == 'B')
		//{
		//}
		//else if (sDitc[0] == 'K')
		//{
		//}
		//else if (sDitc[0] == 'D')
		//{
		//}
		//else
		//{
		//	if (pSta == NULL || pStaDetail == NULL)
		//	{
		//		// ���п� ���� ���� ����
		//		continue;
		//	}
		//}
		if( pSta == NULL || pStaDetail == NULL || (int)dwSensitivity == 0 || (int)dwResponse == 0 )
		{
			// ���п� ���� ���� ����
			continue;
		}
#else
		if( pSta == NULL || pStaDetail == NULL || (int)dwSensitivity == 0 || (int)dwResponse == 0 )
		{
			// ���п� ���� ���� ����
			continue;
		}
#endif	// DEBUG

		dwCalib = (((dwSensitivity * 0.000001) / dwResponse) / 9.81) * 981;

#ifdef _DEBUG
		//if( sDitc[0] == 'S' || sDitc[0] == 'B' )	// Ư������ �ϵ��뱳 ������ �׽�Ʈ ����ġ
		//{
		//	dwCalib = 0.000234;
		//}
		//else if( sDitc[0] == 'K' )	// ��ũ��Ʈ�� �׽�Ʈ�� �ϵ��뱳 ������ ����� ����
		//{
		//	dwCalib = 0.000234;
		//}
		//else if( sDitc[0] == 'D' )	// �ʴ� �ӽ� �׽�Ʈ�� ���� Calibration �� ����
		//{
		//	//dwCalib = 0.000234;
		//}
#endif	// DEBUG

		int	idataCount;
		double* dwDataArray;
	
		double*		real_in;

#ifdef DEF_FFT_LIB
		fftw_complex    *out;
		fftw_complex    *out_freq;
		fftw_complex    *out_period;
		// fftw_complex    *real_in, *out;
		fftw_plan rp;
#endif // DEF_FFT_LIB
	
		double	dwsamplelate = 100.0;
	
		if( sFileNameOnly[7] == DEF_H_SAMPLE )	// 100 ���� �ڷḸ ���
		{
			dwsamplelate = 100.0;
		}
		else if( sFileNameOnly[7] == DEF_B_SAMPLE )	// 20 ���� �ڷ�� ����
		{
			//dwsamplelate = 20.0;
			continue;
		}
		else
		{
			strLog = "";
			strLog.Format("Error unknown data sample code!! filename = '%s'", sFileNameOnly);
			WriteReportLog(strLog);

			TRACE("Error unknown data sample code[%s]\n", sFileNameOnly);
			Distory_WaveDataPool();
			return nRet;
		}
	
		int		i;

		double	dwfftVal;
		double	dwtmp1;
		double	dwtmp2;
		double	dwtmp3;
		double	dwfrequency;

		idataCount = GetRealDataCount(cChanID);
#ifdef _DEBUG
		//TRACE("GetRealDataCount[%c][%d]\n", cChanID, idataCount);
#endif	// DEBUG
		if( idataCount <= 100 )
		{
			strLog = "";
			strLog.Format("Error data low count!! count = '%d'", idataCount);
			WriteReportLog(strLog);

			TRACE("Error data low count[%d]\n", idataCount);
			Distory_WaveDataPool();
			return nRet;
		}
	
		int*	ipDataArray;
		ipDataArray = (int*)malloc(sizeof(int) * idataCount);

		// TRACE("GetCountData(%d)\n", idataCount);
		GetCountData(cChanID, ipDataArray, idataCount);

		dwDataArray = (double*)malloc(sizeof(double) * idataCount);
		for( i=0; i<idataCount; i++ )
		{
			dwDataArray[i] = ipDataArray[i];
			// TRACE("\t%d\n", ipDataArray[i]);
		}

		double*	dwRKTimeArray;
		dwRKTimeArray = (double*)malloc(sizeof(double) * idataCount);

		GetRKtimeData(cChanID, dwRKTimeArray, idataCount);

#ifdef _DEBUG
		//TRACE("dTrend(%d)\n", idataCount);
#endif	// DEBUG
		CFFT_Common::deTrend(dwDataArray, idataCount);

#ifdef _DEBUG
		//TRACE("cosineTapering(%d)\n", idataCount);
#endif	// DEBUG

		double dblMaxPGA = 0.0;
		double dblMaxPGV = 0.0;
		double dblMaxDisp = 0.0;

		CFFT_Common::cosineTapering(dwDataArray, idataCount);

		for( i=0; i<idataCount; i++ )
		{
			ipDataArray[i] = (int)dwDataArray[i];
			//ipDataArray[i] = dwDataArray[i];
			//TRACE("\t[%d] %d\n", i, ipDataArray[i]);
		}
		UpdateRealData(cChanID, DEF_D_KIND_C, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV, dblMaxDisp);

		SampleStruct_t *data;
		data = (SampleStruct_t*)malloc(sizeof(SampleStruct_t) * idataCount);
	
#ifdef _DEBUG
		 //TRACE("data copy for ToGroundTSPModule(%d)\n", idataCount);
#endif	// DEBUG

		for( i=0; i<idataCount; i++ )
		{
			data[i].raw = ipDataArray[i];
		}

#ifdef _DEBUG
		TRACE("ToGroundTSPModule(%d)\n", idataCount);
		//dwCalib = 0.000315784;
#endif	// DEBUG

		CFFT_Common::ToGroundTSPModule(idataCount, dwCalib, data);	// Acc, Vel, Disp ����

#ifdef _DEBUG
		//TRACE("ToGroundTSPModule(%d)\n", idataCount);
#endif	// DEBUG

#ifdef _DEBUG	// 2019.05.14 �ʴ�,������ �׽�Ʈ�� ���� �κ�
		if( TESTDATA )
		{
			CStdioFile file;
			CString strBufferLine = "";
			CString cpFillDemDataPath = "";
			CString strLoc_FillDem = "";
			CString strLoc_FillDem_FileNameOnly = "";
			BOOL bWorking_FillDem = FALSE;

			// �ʴ� �� ������ �׽�Ʈ�� ���
			if (TEST_D)
			{
				cpFillDemDataPath.Format("%s", "D:\\Project\\��.������ �������򰡽ý���\\2019\\�������򰡷���\\�ʴ��������\\20190514\\�������������\\Tabas.txt");
			}
			// ��ũ��Ʈ�� �׽�Ʈ�� ���
			else if (TEST_K)
			{
				cpFillDemDataPath.Format("%s", "D:\\Project\\��.������ �������򰡽ý���\\2019\\�������򰡷���\\��ũ��Ʈ��\\�׽�Ʈ�뵥����\\CAVTestData.txt");
			}
			// Ư������ �׽�Ʈ�� ���
			{
				cpFillDemDataPath.Format("%s", "D:\\Project\\��.������ �������򰡽ý���\\2019\\Source\\Elastic-Spectrum-compare-program_Win04\\ERspectrum\\bin\\Debug\\Test\\Elcentro-NS.dat");
			}

			int nDataCount = 0;
			double* pAccData = NULL;
			double* pVelData = NULL;
			double* pDataArray = NULL;

			if( file.Open(cpFillDemDataPath, CFile::modeRead) )
			{
				while(file.ReadString(strBufferLine))
				{
					strBufferLine.Replace("\r", "");
					nDataCount++;
				}
			}

			file.Close();

			pAccData = (double*)malloc(sizeof(double) * nDataCount);
			pVelData = (double*)malloc(sizeof(double) * nDataCount);
			pDataArray = (double*)malloc(sizeof(double) * nDataCount);
			int nIdx = 0;

			if( file.Open(cpFillDemDataPath, CFile::modeRead) )
			{
				while(file.ReadString(strBufferLine))
				{
					strBufferLine.Replace("\r", "");
					if( strBufferLine.GetAt(0) == 0x20 )
					{
						//strBufferLine.SetAt(0, _T(""));
					}

					BOOL bDataChk = FALSE;

					for(int i = strBufferLine.GetLength(); i > 0; i--)
					{
						if(strBufferLine.GetAt(i) == 0x20 || strBufferLine.GetAt(i) == 0x00 || strBufferLine.GetAt(i) == 0x09)
						{
							if(bDataChk == FALSE) continue;

							CString strAcc = strBufferLine.Mid(i + 1, strBufferLine.GetLength() - 1);
							char* pAcc = new char[strAcc.GetLength() + 1];
							memset(pAcc, 0x00, sizeof(pAcc));
							sprintf(pAcc, "%s", strAcc);
							pAcc[strAcc.GetLength()] = '\0';
							//strncpy(pAcc, strAcc, strAcc.GetLength());
							pAccData[nIdx] = atof(pAcc);// / 100.0;

							if( pAcc )
							{
								delete pAcc;
								pAcc = NULL;
							}
#ifdef _DEBUG
							//TRACE("AccData[%d] = '%lf'\n", nIdx, pAccData[nIdx]);
#endif	// DEBUG
							nIdx++;
							break;
						}
						else
						{
							bDataChk = TRUE;
						}
					}
				}
			}

			file.Close();

			double dblFillMaxPGA = 0.0;
			double dblFillMaxPGV = 0.0;

			if (TEST_K)	// ��ũ��Ʈ�� �׽�Ʈ
			{
				for(int j = 0; j < nDataCount; j++)
				{
					pDataArray[j] = pAccData[j];
				}

				CFFT_Common::deTrend(pDataArray, nDataCount);
			
				for(int j = 0; j < nDataCount; j++)
				{
					if(j == 0)	continue;

					double dblA = fabs(pDataArray[j - 1]);
					double dblB = fabs(pDataArray[j]);

					if(pDataArray[j - 1] > 0 && pDataArray[j] > 0 || pDataArray[j - 1] < 0 && pDataArray[j] < 0)
					{
						dblCAV += ((dblA + dblB) * 0.01) / 2;
					}
					else if(pDataArray[j - 1] < 0 || pDataArray[j] < 0)
					{
						dblCAV += ((0.01 * (dblA / (dblA + dblB)) * dblA) / 2) + ((0.01 * (1 - (dblA / (dblA + dblB))) * dblB) / 2);
					}
#ifdef _DEBUG
				//TRACE("%lf\n", dblCAV);
#endif	// DEBUG
				}
			}

			if (TEST_D)	// �ʴ�(������) �׽�Ʈ
			{
				dblLowFreq = 0.1;
				dblHighFreq = 50.0;

			//BandPassFilter(pDataArray, dwsamplelate, nDataCount, dblLowFreq, dblHighFreq, 2, NULL);

				CFFT_Common::deTrend(pDataArray, nDataCount);

				GetRKtimeData(cChanID, dwRKTimeArray, nDataCount);
				CFFT_Common::FourthRungeKutta(dwRKTimeArray, pDataArray, nDataCount);	// ���ӵ� -> �ӵ� ��ȯ
				//CFFT_Common::FourthRungeKutta(dwRKTimeArray, dwDataArray, idataCount);	// �ӵ� -> ���� ��ȯ

				CFFT_Common::deTrend(pDataArray, nDataCount);

				for(int j = 0; j < nDataCount; j++)
				{
					pVelData[j] = pDataArray[j];
				}

				for(int j = 0; j < nDataCount; j++)
				{
					if(j == 0)
					{
						dblFillMaxPGA = fabs(pAccData[j]);
					}
					else
					{
						if(fabs(pAccData[j]) > dblFillMaxPGA)
						{
							dblFillMaxPGA = fabs(pAccData[j]);
						}
					}
				}

				for(int j = 0; j < nDataCount; j++)
				{
					if(j == 0)
					{
						dblFillMaxPGV = fabs(pVelData[j]);
					}
					else
					{
						if(fabs(pVelData[j]) > dblFillMaxPGV)
						{
							dblFillMaxPGV = fabs(pVelData[j]);
						}
					}
				}
			
				//dblSettlement = (exp( (RESERVOIR_PGA_FACTOR_V1 * dblFillMaxPGA / 981) - RESERVOIR_MINUS_V1 ));	// �ʴ�������� �ջ����� ��� �˰��� 1���� (PGA ���)
				//dblSettlement = RESERVOIR_FACTOR * (dblFillMaxPGA / 981);	// 2019.10.23 �Ѿ�� ������ ������ ���� ������ ����
				dblSettlement = RESERVOIR_FACTOR * (dblFillMaxPGA);	// 2019.11.21 PGA���� gal ������ ��� / 981 ����, g �� ��� * 981 ���
				//int nSettlement = (int)((exp( (RESERVOIR_PGA_FACTOR_V2 * dblFillMaxPGA) + (RESERVOIR_PGV_FACTOR_V2 * dblFillMaxPGV) - RESERVOIR_MINUS_V2)) * 1000);	// �ʴ�������� �ջ����� ��� �˰��� 2���� (PGA, PGV ���)
				//double dblSettlement = dblSettlement * 0.010;
				//dblSettlement = CFFT_Common::Rounding(dblSettlement, 1);

#ifdef _DEBUG
				TRACE("[%s] dblMaxPGA = '%lf', dblMaxPGV = '%lf', dblSettlement = '%.6lf'\n", __FUNCTION__, dblFillMaxPGA, dblFillMaxPGV, dblSettlement);
				TRACE("[%s] Reservoir Settlement = '%.2lf'\n", __FUNCTION__, dblSettlement);
#endif	// DEBUG
			}

			if (TEST_SB)	// Ư������ �׽�Ʈ
			{
				ThSta_t* pThSta = NULL;
				ThSta_Detail_t* pThStaDetail = NULL;

				for (int j = 0; j < m_aryThStation.GetCount(); j++)
				{
					pThSta = (ThSta_t*)m_aryThStation.GetAt(j);
					if (pThSta == NULL) continue;
					if (strstr(pThSta->sSen_Id, sObsSenName) == NULL) continue;

					pThStaDetail = (ThSta_Detail_t*)pThSta->pAryThSta_Detail->GetAt(0);
					if (pThStaDetail == NULL) continue;

					break;
				}

				GroundAccType* earthq = new GroundAccType;
				SpectrumType* spectrum = new SpectrumType;

				double damping = 0.05;
				double gravity = 1 * -1;
				double s1 = 0.068;//0.154;
				//double s1 = 67;	// 2019.11.26 ���� ������ ������ ��û���� ����
				double alpha = 2.8;

				if (pThStaDetail != NULL)
				{
					damping = atof(pThStaDetail->sDampingRatio);
					gravity = atof(pThStaDetail->sGravity) * -1;
					s1 = atof(pThStaDetail->sEffectivePGA);
					alpha = atof(pThStaDetail->sCoefficient);
				}

				char disfile[1024];
				char velfile[1024];
				char accfile[1024];
				char dsgfile[1024];
				memset(disfile, 0x00, sizeof(disfile)); //cpFillDemDataPath
				memset(velfile, 0x00, sizeof(velfile));
				memset(accfile, 0x00, sizeof(accfile));
				memset(dsgfile, 0x00, sizeof(dsgfile));

				sprintf(disfile, "%s", cpFillDemDataPath);
				sprintf(velfile, "%s", cpFillDemDataPath);
				sprintf(accfile, "%s", cpFillDemDataPath);
				sprintf(dsgfile, "%s", cpFillDemDataPath);

				disfile[strlen(disfile) - 4] = '\0';
				velfile[strlen(velfile) - 4] = '\0';
				accfile[strlen(accfile) - 4] = '\0';
				dsgfile[strlen(dsgfile) - 4] = '\0';

				strcat(disfile, ".dis");
				strcat(velfile, ".vel");
				strcat(accfile, ".acc");
				strcat(dsgfile, ".dsg");

				earthq->gcount = nDataCount;
				earthq->gtime = new double[earthq->gcount + 1];
				earthq->acc = new double[earthq->gcount + 1];
				spectrum->nperiod = 501;
				spectrum->period = new double[spectrum->nperiod + 1];
				spectrum->dispSpectrum = new double[spectrum->nperiod + 1];
				spectrum->velSpectrum = new double[spectrum->nperiod + 1];
				spectrum->accSpectrum = new double[spectrum->nperiod + 1];
				spectrum->designSpectrum = new double[spectrum->nperiod + 1];

				for(int j = 0; j < nDataCount; j++)
				{
					pDataArray[j] = pAccData[j];
				}
				
				GetRKtimeData(cChanID, dwRKTimeArray, nDataCount);

				earthq->gtime[0] = 0;
				earthq->acc[0] = 0;

				for(int k = 0; k < nDataCount; k++)
				{
					earthq->gtime[k + 1] = dwRKTimeArray[k];
					earthq->acc[k + 1] = pDataArray[k] * gravity;
				}

				earthq->dt = earthq->gtime[2] - earthq->gtime[1];
				CFFT_Common::deTrend(earthq->acc, nDataCount + 1);
				earthq->maxtime = earthq->gtime[earthq->gcount];
				SpectrumCompute(earthq, damping, spectrum);
				DesignSpectrum(s1, alpha, spectrum);
				if (CompareSpectrum(spectrum))
#ifdef _DEBUG
					TRACE("\n\t N.G \n");
#endif	// DEBUG
				else
#ifdef _DEBUG
					TRACE("\n\t O.K \n");
#endif	// DEBUG

				spectrum->WriteSpectrum(disfile, spectrum, 1);
				spectrum->WriteSpectrum(velfile, spectrum, 2);
				spectrum->WriteSpectrum(accfile, spectrum, 3);
				spectrum->WriteSpectrum(dsgfile, spectrum, 4);

				if (earthq)		delete earthq;
				if (spectrum)	delete spectrum;
			}

			if (pAccData)		free(pAccData);
			if (pVelData)		free(pVelData);
			if (pDataArray)	free(pDataArray);
		}
#endif	// DEBUG

		if( sDitc[0] == 'S' || sDitc[0] == 'B' )	// ����-���屳, ����-�������� ���� RungeKutta �˰������� ���� ����
		//if (false)
		{
			if( sChannelId[0] == 'H' )
			{
				for( i=0; i<idataCount; i++ )
				{
					dwDataArray[i] = data[i].a;
				}

				UpdateRealData(cChanID, DEF_D_KIND_A, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV, dblMaxDisp);

				// 2019.08.09 ���ӵ� ���� ����Ʈ�� �м��� ���� �߰� START
				char	sObsId1[4];
				char	sObsId2[4];
				memset(sObsId1, 0x00, sizeof(sObsId1));
				memset(sObsId2, 0x00, sizeof(sObsId2));
				strncpy(sObsId1, sObsSenName + 5, 3);
				sObsId1[3] = '\0';
				strncpy(sObsId2, sObsSenName + 9, 1);
				sObsId2[1] = '\0';

				ThSta_t* pThSta = NULL;
				ThSta_Detail_t* pThStaDetail = NULL;

				for (int j = 0; j < m_aryThStation.GetCount(); j++)
				{
					pThSta = (ThSta_t*)m_aryThStation.GetAt(j);
					if (pThSta == NULL) continue;
					if (strstr(pThSta->sSen_Id, sObsSenName) == NULL) continue;

					pThStaDetail = (ThSta_Detail_t*)pThSta->pAryThSta_Detail->GetAt(0);
					if (pThStaDetail == NULL) continue;

					break;
				}

				// ���ӵ��� ���� �м��ϴ� �����͸� ������� ����
				if( (strstr(sObsId1, "B_H") != NULL && strstr(sObsId2, "X") != NULL) || (strstr(sObsId1, "B_H") != NULL && strstr(sObsId2, "Y") != NULL) || (strstr(sObsId1, "B_H") != NULL && strstr(sObsId2, "Z") != NULL) ||		// ��ž���� (��ž ���� ��ܸ�)
					(strstr(sObsId1, "G_H") != NULL && strstr(sObsId2, "N") != NULL) || (strstr(sObsId1, "G_H") != NULL && strstr(sObsId2, "E") != NULL) || (strstr(sObsId1, "G_H") != NULL && strstr(sObsId2, "Z") != NULL) ||		// ������
					(strstr(sObsId1, "R_H") != NULL && strstr(sObsId2, "X") != NULL) || (strstr(sObsId1, "R_H") != NULL && strstr(sObsId2, "Y") != NULL) || (strstr(sObsId1, "R_H") != NULL && strstr(sObsId2, "Z") != NULL) )		// ��Ŀ����(������) (��Ŀ��� ���)
				{
					GroundAccType* earthq = new GroundAccType;
					SpectrumType* spectrum = new SpectrumType;

					double damping = 0.05;
					double gravity = 1 * -1;
					double s1 = 0.0;
					
					// 2019.11.26 ���� ������ ������ ��û���� ����
					if( strstr(sObsId2, "Z") != NULL )
					{
						s1 = 52.0;
					}
					else
					{
						s1 = 67.0;
					}

					double alpha = 2.8;

					if (pThStaDetail != NULL)
					{
						damping = atof(pThStaDetail->sDampingRatio);
						gravity = atof(pThStaDetail->sGravity) * -1;
						s1 = atof(pThStaDetail->sEffectivePGA);
						alpha = atof(pThStaDetail->sCoefficient);
					}

					char disfile[1024];
					char velfile[1024];
					char accfile[1024];
					char dsgfile[1024];
					memset(disfile, 0x00, sizeof(disfile)); //cpFillDemDataPath
					memset(velfile, 0x00, sizeof(velfile));
					memset(accfile, 0x00, sizeof(accfile));
					memset(dsgfile, 0x00, sizeof(dsgfile));

					char sResSpectrumFilePath[1024];
					memset(sResSpectrumFilePath, 0x00, sizeof(sResSpectrumFilePath));
					sprintf(sResSpectrumFilePath, "%s\\%s\\%s_%s", sMseedPath, PATH_RESULT, sObsSenName, sEventID);

					sprintf(disfile, "%s", sResSpectrumFilePath);
					sprintf(velfile, "%s", sResSpectrumFilePath);
					sprintf(accfile, "%s", sResSpectrumFilePath);
					sprintf(dsgfile, "%s", sResSpectrumFilePath);

					strcat(disfile, ".dis");
					strcat(velfile, ".vel");
					strcat(accfile, ".acc");
					strcat(dsgfile, ".dsg");

					earthq->gcount = idataCount;
					earthq->gtime = new double[earthq->gcount + 1];
					earthq->acc = new double[earthq->gcount + 1];
					spectrum->nperiod = 501;
					spectrum->period = new double[spectrum->nperiod + 1];
					spectrum->dispSpectrum = new double[spectrum->nperiod + 1];
					spectrum->velSpectrum = new double[spectrum->nperiod + 1];
					spectrum->accSpectrum = new double[spectrum->nperiod + 1];
					spectrum->designSpectrum = new double[spectrum->nperiod + 1];
						
					GetRKtimeData(cChanID, dwRKTimeArray, idataCount);

					earthq->gtime[0] = 0;
					earthq->acc[0] = 0;

					for(int k = 0; k < idataCount; k++)
					{
						earthq->gtime[k + 1] = dwRKTimeArray[k];
						earthq->acc[k + 1] = dwDataArray[k] * gravity;
					}

					earthq->dt = earthq->gtime[2] - earthq->gtime[1];
					CFFT_Common::deTrend(earthq->acc, idataCount + 1);
					earthq->maxtime = earthq->gtime[earthq->gcount];
					SpectrumCompute(earthq, damping, spectrum);
					DesignSpectrum(s1, alpha, spectrum);
					if (CompareSpectrum(spectrum))
					{
						nResSpectrum = 0;
#ifdef _DEBUG
						TRACE("\n\t N.G \n");
#endif	// DEBUG
					}
					else
					{
						nResSpectrum = 1;
#ifdef _DEBUG
						TRACE("\n\t O.K \n");
#endif	// DEBUG
					}

					spectrum->WriteSpectrum(disfile, spectrum, 1);
					spectrum->WriteSpectrum(velfile, spectrum, 2);
					spectrum->WriteSpectrum(accfile, spectrum, 3);
					spectrum->WriteSpectrum(dsgfile, spectrum, 4);

					if (earthq)		delete earthq;
					if (spectrum)	delete spectrum;
				}
				// 2019.08.09 ���ӵ� ���� ����Ʈ�� �м��� ���� �߰� END
				else
				{
					BandPassFilter(dwDataArray, dwsamplelate, idataCount, dblLowFreq, dblHighFreq, 2, NULL);

					//CFFT_Common::deTrend(dwDataArray, idataCount);	// 2019.06.28 ������ �Ǵ� �� �䱸�� ���� ���͸��� ���ӵ� �����ʹ� �߰� deTrend ���͸� ���� ����

					CFFT_Common::FourthRungeKutta(dwRKTimeArray, dwDataArray, idataCount);	// ���ӵ� -> �ӵ� ��ȯ

					CFFT_Common::deTrend(dwDataArray, idataCount);

					CFFT_Common::FourthRungeKutta(dwRKTimeArray, dwDataArray, idataCount);	// �ӵ� -> ���� ��ȯ

					CFFT_Common::deTrend(dwDataArray, idataCount);

					int nCountRet = UpdateRealData(cChanID, DEF_D_KIND_D, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV, dblMaxDisp);

					sprintf(szOutFile, "%s\\%s%s"
						, strOutFilePath
						, sFileNameOnly
						, "_Disp.txt");

					//// ���� ������ ���� ���� ���� START
					//FILE *writeFile = NULL;

					//writeFile = fopen(szOutFile, "w" );
					//if( writeFile == NULL )
					//{
					//	TRACE("ERROR fopen(%s)\n", szOutFile);
					//	return 0;
					//}
					//OutTogroundData_DispOnly(szOutFile, cChanID, 0);		// Disp Only

					//fclose(writeFile);
					//// ���� ������ ���� ���� ���� END
				}

#ifdef _DEBUG
				TRACE("[%s] sObsSenName = '%s', dblMaxPGA = '%lf', dblMaxDisp = '%lf'\n", __FUNCTION__, sObsSenName, dblMaxPGA, dblMaxDisp);
#endif	// DEBUG

				free(ipDataArray);
				free(dwDataArray);
				free(data);
				free(dwRKTimeArray);
			}
		}
		else if (sDitc[0] == 'K')		// ��ũ��Ʈ��
		{
			if( sChannelId[0] == 'H' )
			{
//#ifdef _DEBUG
				for( i=0; i<idataCount; i++ )
				{
					dwDataArray[i] = data[i].a;
				}

				UpdateRealData(cChanID, DEF_D_KIND_A, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV, dblMaxDisp);

				dblCAV = Integral_Acc(data, idataCount);
//#endif	// nDEBUG

#ifdef _DEBUG
				TRACE("[%s] dblMaxPGA = '%lf', dblCAV = '%lf'\n", __FUNCTION__, dblMaxPGA, dblCAV);
#endif	// DEBUG
			}

			free(ipDataArray);
			free(dwDataArray);
			free(data);
			free(dwRKTimeArray);
		}
		else if (sDitc[0] == 'D')		// �ʴ� �� ������
		{
			// �ִ� ���ӵ� ����
			for( i=0; i<idataCount; i++ )
			{
				double dblAcc = fabs(data[i].a);

				if( idataCount == 0)
				{
					dblMaxPGA = dblAcc;
				}
				else
				{
					if(dblMaxPGA < dblAcc)
					{
						dblMaxPGA = dblAcc;
					}
				}
			}

			// �ӵ� ����
			for( i=0; i<idataCount; i++ )
			{
				dwDataArray[i] = data[i].v;
			}

			BandPassFilter(dwDataArray, dwsamplelate, idataCount, dblLowFreq, dblHighFreq, 2, NULL);//, szOutFile);	// ���� ����� ������ ���ڰ��� ������ ��ü��� ���Ե� ���ϸ� ����
			UpdateRealData(cChanID, DEF_D_KIND_V, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV, dblMaxDisp);

			//double a = (RESERVOIR_PGA_FACTOR_V1 * (dblMaxPGA / 981)) - RESERVOIR_MINUS_V1;
			//dblSettlement = exp(a);
			//double b = log(exp(a));

			//dblSettlement = (exp( (RESERVOIR_PGA_FACTOR_V1 * dblMaxPGA / 981) - RESERVOIR_MINUS_V1 ));	// �ʴ�������� �ջ����� ��� �˰��� 1���� (PGA ���)
			//dblSettlement = RESERVOIR_FACTOR * (dblMaxPGA / 981);	// 2019.10.23 �Ѿ�� ������ ������ ���� ������ ����
			dblSettlement = RESERVOIR_FACTOR * (dblMaxPGA);	// 2019.11.21 PGA���� gal ������ ��� / 981 ����, g �� ��� * 981 ���
			//nSettlement = (int)((exp( (RESERVOIR_PGA_FACTOR_V2 * dblMaxPGA) + (RESERVOIR_PGV_FACTOR_V2 * dblMaxPGV) - RESERVOIR_MINUS_V2)) * 1000);	// �ʴ�������� �ջ����� ��� �˰��� 2���� (PGA, PGV ���)
			//double dblSettlement = nSettlement * 0.010;
			//dblSettlement = CFFT_Common::Rounding(dblSettlement, 1);

#ifdef _DEBUG
			TRACE("[%s] dblMaxPGA = '%lf', dblMaxPGV = '%lf', dblSettlement = '%.6lf'\n", __FUNCTION__, dblMaxPGA, dblMaxPGV, dblSettlement);
			TRACE("[%s] Reservoir Settlement = '%.2lf'\n", __FUNCTION__, dblSettlement);
#endif	// DEBUG

			free(ipDataArray);
			free(dwDataArray);
			free(data);
			free(dwRKTimeArray);
		}
		else
		{
			for( i=0; i<idataCount; i++ )
			{
				dwDataArray[i] = data[i].v;
			}
			//UpdateRealData(cChanID, DEF_D_KIND_V, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV);

			//sprintf(szOutFile, "%s\\%s%s"
			//	, strOutFilePath
			//	, sFileNameOnly
			//	, "_Filtered_Vel.txt");

			BandPassFilter(dwDataArray, dwsamplelate, idataCount, dblLowFreq, dblHighFreq, 2, NULL);//, szOutFile);	// ���� ����� ������ ���ڰ��� ������ ��ü��� ���Ե� ���ϸ� ����
			UpdateRealData(cChanID, DEF_D_KIND_V, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV, dblMaxDisp);
	
			for( i=0; i<idataCount; i++ )
			{
				dwDataArray[i] = data[i].d;
			}
			//UpdateRealData(cChanID, DEF_D_KIND_D, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV);

			//sprintf(szOutFile, "%s\\%s%s"
			//	, strOutFilePath
			//	, sFileNameOnly
			//	, "_Filtered_Disp.txt");

			BandPassFilter(dwDataArray, dwsamplelate, idataCount, dblLowFreq, dblHighFreq, 2, NULL);//, szOutFile);
			UpdateRealData(cChanID, DEF_D_KIND_D, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV, dblMaxDisp);

			for( i=0; i<idataCount; i++ )
			{
				dwDataArray[i] = data[i].a;
			}
			//UpdateRealData(cChanID, DEF_D_KIND_A, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV);

			//sprintf(szOutFile, "%s\\%s%s"
			//	, strOutFilePath
			//	, sFileNameOnly
			//	, "_Filtered_Acc.txt");

			BandPassFilter(dwDataArray, dwsamplelate, idataCount, dblLowFreq, dblHighFreq, 2, NULL);//, szOutFile);
			UpdateRealData(cChanID, DEF_D_KIND_A, dwDataArray, idataCount, dblMaxPGA, dblMaxPGV, dblMaxDisp);

#ifdef _DEBUG
			//// ���ӵ�, �ӵ�, ���� ���� ���� ���� ���� ���� START
			//sprintf(szOutFile, "%s\\%s%s"
			//	, strOutFilePath
			//	, sFileNameOnly
			//	, "_Acc.txt");

			//FILE *wFile = NULL;

			//wFile = fopen(szOutFile, "w" );
			//if( wFile == NULL )
			//{
			//	TRACE("ERROR fopen(%s)\n", szOutFile);
			//	return 0;
			//}

			////char	szBuff[1024];
			//OutTogroundData_AccOnly(szOutFile, cChanID, 0);		// Acc Only

			//fclose(wFile);

			//sprintf(szOutFile, "%s\\%s%s"
			//	, strOutFilePath
			//	, sFileNameOnly
			//	, "_Vel.txt");

			//wFile = NULL;

			//wFile = fopen(szOutFile, "w" );
			//if( wFile == NULL )
			//{
			//	TRACE("ERROR fopen(%s)\n", szOutFile);
			//	return 0;
			//}

			////fwrite(szBuff, strlen(szBuff), 1, wFile);

			////TRACE("OutFile[%s]\n", szOutFile);
			//OutTogroundData_VelOnly(szOutFile, cChanID, 0);		// Vel Only

			//fclose(wFile);

			//sprintf(szOutFile, "%s\\%s%s"
			//	, strOutFilePath
			//	, sFileNameOnly
			//	, "_Disp.txt");

			//wFile = NULL;

			//wFile = fopen(szOutFile, "w" );
			//if( wFile == NULL )
			//{
			//	TRACE("ERROR fopen(%s)\n", szOutFile);
			//	return 0;
			//}

			////char	szBuff[1024];
			////fwrite(szBuff, strlen(szBuff), 1, wFile);

			////TRACE("OutFile[%s]\n", szOutFile);
			//OutTogroundData_DispOnly(szOutFile, cChanID, 0);		// Disp Only

			//fclose(wFile);
			//// ���ӵ�, �ӵ�, ���� ���� ���� ���� ���� END
#endif	// DEBUG

#ifdef _DEBUG
			//dblSettlement = (exp( (RESERVOIR_PGA_FACTOR_V1 * dblMaxPGA / 981) - RESERVOIR_MINUS_V1 ));	// �ʴ�������� �ջ����� ��� �˰��� 1���� (PGA ���)
			//dblSettlement = RESERVOIR_FACTOR * (dblMaxPGA / 981);	// 2019.10.23 �Ѿ�� ������ ������ ���� ������ ����
			dblSettlement = RESERVOIR_FACTOR * (dblMaxPGA);	// 2019.11.21 PGA���� gal ������ ��� / 981 ����, g �� ��� * 981 ���
			//nSettlement = (int)((exp( (RESERVOIR_PGA_FACTOR_V2 * dblMaxPGA) + (RESERVOIR_PGV_FACTOR_V2 * dblMaxPGV) - RESERVOIR_MINUS_V2)) * 1000);	// �ʴ�������� �ջ����� ��� �˰��� 2���� (PGA, PGV ���)
			//double dblSettlement = dblSettlement * 0.10;
			//dblSettlement = CFFT_Common::Rounding(dblSettlement, 1);

			TRACE("[%s] FileName = '%s', dblMaxPGA = '%lf', dblMaxPGV = '%lf', dblMaxDisp = '%lf'\n", __FUNCTION__, strLoc, dblMaxPGA, dblMaxPGV, dblMaxDisp);
			TRACE("[%s] Reservoir Settlement = '%.2lf'\n", __FUNCTION__, dblSettlement);
#endif	// DEBUG

			/*
			for( i=0; i<idataCount; i++ )
			{
							// c1, a1, a2, v, d
				TRACE("\t%d\t%.8f\t%.8f\t%.8f\n"
							, ipDataArray[i]
							, data[i].a
							, data[i].v
							, data[i].d);
			}
			*/
	
			free(ipDataArray);
			//free(dwDataArray);
			free(data);
			free(dwRKTimeArray);
	
			//// out put file count, acc, vel, disp
			//if( sFileNameOnly[7] == DEF_H_SAMPLE )
			//{
			//	sprintf(szOutFile, "%s\\%s%s"
			//						, strOutFilePath
			//						, sFileNameOnly
			//						, DEF_MSEED_ASCII_100);
			//	//TRACE("OutFile[%s]\n", szOutFile);
			//	OutTogroundData(szOutFile, cChanID, 0);		// all sample
			//}
			//else if( sFileNameOnly[7] == DEF_B_SAMPLE )
			//{
			//	sprintf(szOutFile, "%s\\%s%s"
			//						, strOutFilePath
			//						, sFileNameOnly
			//						, DEF_MSEED_ASCII_20);
			//	//TRACE("OutFile[%s]\n", szOutFile);
			//	OutTogroundData(szOutFile, cChanID, 0);		// all sample
			//}
	
			//sprintf(szOutFile, "%s\\%s%s"
			//					, strOutFilePath
			//					, sFileNameOnly
			//					, DEF_MSEED_ASCII_001);
			////TRACE("OutFile[%s]\n", szOutFile);
			//OutTogroundData(szOutFile, cChanID, 1);		// 1/sec sample 

			// filtering
			char szBPFOutFile[2048];
			memset(szBPFOutFile, 0x00, sizeof(szBPFOutFile));
			sprintf(szBPFOutFile, "%s\\%s_BP.txt", strOutFilePath, sFileNameOnly);
			//testBandPassFilter(dwDataArray, idataCount, szBPFOutFile);

			char szCmd[2048];
			memset(szCmd, 0x00, sizeof(szCmd));

			//free(dwDataArray);
		
			// 100 sample data, KEVT Check
			//if(    (sFileNameOnly[7] == DEF_H_SAMPLE) )
#ifdef DEF_FFT_LIB
			if(false)
				//&& (strncmp(szKHEVT, DEF_KEVT, strlen(DEF_KEVT)) == 0) )
				// && (strncmp(szKHEVT, DEF_KEVT, strlen(DEF_KEVT)) == 0) || (strncmp(szKHEVT, DEF_HEVT, strlen(DEF_HEVT)) == 0) )
			{
				// FFT
				real_in = (double*) fftw_malloc(sizeof(double) * idataCount);
				// real_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * idataCount);
				out 	= (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * idataCount);   // output buffer
		
				for( i=0; i<idataCount; i++ )
				{
					real_in[i] = dwDataArray[i];
			
					// real_in[i][0] = dwDataArray[i];
					// real_in[i][1] = dwDataArray[i];
				}
				free(dwDataArray);
		
				rp = fftw_plan_dft_r2c_1d(idataCount, real_in, out, FFTW_ESTIMATE);
				 //rp = fftw_plan_dft_1d(idataCount, real_in, out, FFTW_FORWARD, FFTW_ESTIMATE);

				fftw_execute(rp);

				int		ifreqCnt = idataCount / 2;
				out_freq  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * ifreqCnt);
				out_period = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * ifreqCnt);
		
				ifreqCnt--;
				// ���ļ� ���� fftp��
				for( i=0; i<=ifreqCnt; i++ )				// ��� Ȯ��
				{
					dwtmp1 = out[i][0] * out[i][0];
					dwtmp2 = out[i][1] * out[i][1];
					dwtmp3 = dwtmp1 + dwtmp2;
					dwfftVal = sqrt(dwtmp3) / idataCount;
					dwfrequency = i * (dwsamplelate / idataCount);

					// ���ļ� FFT ��
					out_freq[i][0] = dwfrequency;
					out_freq[i][1] = dwfftVal;

					// �ֱ� FFT ��
					if( dwfrequency <= 0. )
					{
						out_period[ifreqCnt-i][0] = 0;
						out_period[ifreqCnt-i][1] = 0;
					}
					else
					{
						out_period[ifreqCnt-i][0] = 1 / dwfrequency;
						out_period[ifreqCnt-i][1] = dwfftVal;
					}
				}
				fftw_destroy_plan(rp);
				fftw_free(real_in);
				fftw_free(out);

				sprintf(szOutFile, "%s\\%s%s"
									, strOutFilePath
									, sFileNameOnly
									, DEF_MSEED_FFT);

#ifdef _DEBUG
				TRACE("OutFile[%s]\n", szOutFile);
#endif	// DEBUG

				// FFT write to file
				FILE *wFile = NULL;

				wFile = fopen(szOutFile, "w" );
				if( wFile == NULL )
				{
					fftw_free(out_freq);
					fftw_free(out_period);
			
					Distory_WaveDataPool();

					TRACE("ERROR fopen(%s)\n", szOutFile);
					return 0;
				}
				sprintf(szCmd, "#frequency, amplitude, period, amplitude\n");
				fwrite(szCmd, strlen(szCmd), 1, wFile);
		
				for( i=0; i<=ifreqCnt; i++ )				// ��� Ȯ��
				{
					// 2016-12-08 ��µǴ� FFT���� 2��� ��� ��û from SEONG-BAE JO<siderique@gmail.com>
					sprintf(szCmd, "%.8f %.8f %.8f %.8f\n"
								, out_freq[i][0]
								, (out_freq[i][1] * 2)
								// , out_freq[i][1]
								, out_period[i][0]
								, (out_period[i][1] * 2));
								// , out_period[i][1]);
					fwrite(szCmd, strlen(szCmd), 1, wFile);			
				}
				fclose(wFile);

				fftw_free(out_freq);
				fftw_free(out_period);
			}
			else
			{
				free(dwDataArray);
			}
#else
			free(dwDataArray);
#endif // DEF_FFT_LIB
		}

		int nWriteCalcResultRet = WriteCalcResult(sDitc, sObsSenName, sNet, sSta, sMseedPath, dblMaxPGA, dblMaxPGV, dblMaxDisp, dblSettlement, dblCAV, nResSpectrum);

		strLog = "";

		switch( nWriteCalcResultRet )
		{
		case 0:	// ����
			strLog.Format("Write Calc Result Failed!! obs = '%s'", sObsSenName);
			break;
		case 1:	// ����			
			strLog.Format("Write Calc Result Success!! obs = '%s'", sObsSenName);
			break;
		case -1:	// ������ �� ���� ����
			strLog.Format("Write Calc Result Pass!! obs = '%s'", sObsSenName);
			break;
		default:
			strLog.Format("Write Calc Result Unknown!! obs = '%s'", sObsSenName);
			break;
		}

		if( strLog.GetLength() > 0 )
		{
			WriteReportLog(strLog);
		}
	
		Distory_WaveDataPool();
	}

	BOOL bWriteResultRet = WriteResult(sDitc, sNet, sSta, sEventID, sMseedPath);
	
	strLog = "";
	strLog.Format("Write Result = '%d'", bWriteResultRet);
	WriteReportLog(strLog);

	if (m_strRemoveAnalDataYN == "Y")
	{
		DeleteCalcData(sOutFilePath, sNet, sSta);
	}

	strLog = "";
	strLog.Format("Building Evalution Process End!!");
	WriteReportLog(strLog);

	if( bWriteResultRet )
	{
		nRet = 1;
	}

	return nRet;
}

void CMakeEvalutionBaseData::testBandPassFilter(double fps, int iCount, double lowFreq, double highFreq, int nFiltOrd)
{
	TCHAR path[2048];
	GetModuleFileName(NULL, path, sizeof(path));

	CString strPath = path;
	int iFind = strPath.ReverseFind('\\');
	strPath = strPath.Left(iFind);

	char inFileFullPath[2048];
	memset(inFileFullPath, 0x00, sizeof(inFileFullPath));
	sprintf(inFileFullPath, "%s\\input.txt", strPath);

	FILE* infile;
	infile = fopen(inFileFullPath, "r");
	vector<double> input, output;

	//double fps = 100;
	//int nFiltOrd = 2;
	//const int N = 10000;

	double dblFrequencyBands[2] = { lowFreq/fps, highFreq/fps };
			
	//double* DenC = 0;
	//double* NumC = 0;
	vector<double> DenC;
	vector<double> NumC;

	//vector<double> *x;//(iCount);
	//vector<double> *y;//(iCount);
	double* x;
	double* y;

	x = (double*)malloc(sizeof(double) * iCount);
	y = (double*)malloc(sizeof(double) * iCount);

	//x = (vector<double>*)malloc(sizeof(vector<double>) * iCount);
	//y = (vector<double>*)malloc(sizeof(vector<double>) * iCount);

	//double samp_dat;
	//int num_read = 0;
	//int nIdx = 0;

	//for(int i = 0; i < iCount; i++)
	//{
	//	x[i] = pDataArray[i];
	//}

	//while(1){
	//	num_read = fread(&samp_dat, sizeof(double), 1, infile);
	//	if(num_read != 1) break;
	//	//out_val = my_filter->do_sample( samp_dat );
	//	//samp_dat = out_val;
	//	//samp_dat = samp_dat * 90e77;

	//	x[nIdx] = samp_dat;//(outText, "%.100f", samp_dat);
	//	//fwrite(outText, strlen(outText), 1, fd_out);
	//	//fprintf(fd_out, "%lf\n", samp_dat);
	//	nIdx++;
	//}

/*	for (int i = 0; i < N; i++)
	{
		ifile >> x[i];
	}*/ 

	DenC = ComputeDenCoeffs(nFiltOrd, dblFrequencyBands[0], dblFrequencyBands[1]);
	for (int k = 0; k<DenC.size(); k++)
	{
		printf("DenC is: %lf\n", DenC[k]);
	}

	NumC = ComputeNumCoeffs(nFiltOrd, dblFrequencyBands[0], dblFrequencyBands[1], DenC);
	for (int k = 0; k<NumC.size(); k++)
	{
		printf("NumC is: %lf\n", NumC[k]);
	}

	//y = filter(x, iCount, NumC, DenC);
	filter(x, y, iCount, NumC, DenC);
	
	ofstream ofile;

	char outFileName[2048];
	memset(outFileName, 0x00, sizeof(outFileName));
	sprintf(outFileName, "%s\\output.txt", strPath);

	ofile.open(outFileName);
	for (int i = 0; i < iCount; i++)
	{
		ofile << y[i] << endl;
	}
	ofile.close();

	free(x);
	free(y);
}

//************************************
// Method:    BandPassFilter
// FullName:  CMakeEvalutionBaseData::BandPassFilter
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: double * pDataArray
// Parameter: double fps
// Parameter: int iCount
// Parameter: double dblLowFreq
// Parameter: double dblHighFreq
// Parameter: int nFiltOrd
// Parameter: char * outFileFullPath = NULL	���� ���Ϸ� �����Ϸ��� ���, ��ü��� ���Ե� ���ϸ� ����
//************************************
void CMakeEvalutionBaseData::BandPassFilter(double* pDataArray, double fps, int iCount, double dblLowFreq, double dblHighFreq, int nFiltOrd, char* outFileFullPath)
{
	vector<double> input, output;

	double dblFrequencyBands[2] = { dblLowFreq / fps * 2, dblHighFreq / fps * 2 };

	vector<double> DenC;
	vector<double> NumC;

	double* x;
	double* y;

	x = (double*)malloc(sizeof(double) * iCount);
	y = (double*)malloc(sizeof(double) * iCount);

	for(int i = 0; i < iCount; i++)
	{
		x[i] = pDataArray[i];
	}

	//while(1){
	//	num_read = fread(&samp_dat, sizeof(double), 1, infile);
	//	if(num_read != 1) break;
	//	//out_val = my_filter->do_sample( samp_dat );
	//	//samp_dat = out_val;
	//	//samp_dat = samp_dat * 90e77;

	//	x[nIdx] = samp_dat;//(outText, "%.100f", samp_dat);
	//	//fwrite(outText, strlen(outText), 1, fd_out);
	//	//fprintf(fd_out, "%lf\n", samp_dat);
	//	nIdx++;
	//}

	DenC = ComputeDenCoeffs(nFiltOrd, dblFrequencyBands[0], dblFrequencyBands[1]);
	for (int k = 0; k<DenC.size(); k++)
	{
		printf("DenC is: %lf\n", DenC[k]);
	}

	NumC = ComputeNumCoeffs(nFiltOrd, dblFrequencyBands[0], dblFrequencyBands[1], DenC);
	for (int k = 0; k<NumC.size(); k++)
	{
		printf("NumC is: %lf\n", NumC[k]);
	}

	//y = filter(x, iCount, NumC, DenC);
	filter(x, y, iCount, NumC, DenC);

	for(int i = 0; i < iCount; i++)
	{
		pDataArray[i] = y[i];
	}

	if(outFileFullPath != NULL)
	{
		ofstream ofile;
		ofile.open(outFileFullPath);
		for (int i = 0; i < iCount; i++)
		{
			ofile << y[i] << endl;
		}
		ofile.close();
	}

	free(x);
	free(y);
}

//************************************
// Method:    ReadStationInfo2Array
// FullName:  ReadStationInfo2Array
// Access:    public 
// Returns:   int
// Returns:			0 : ����
// Returns:			-2 : ���� - Station Calibration ���� ������ ����(StationInfo.txt)
// Qualifier:
// Parameter: char * sFilePath
//************************************
int CMakeEvalutionBaseData::ReadStationInfo2Array(CString sFilePath)
{
	int nRet = 0;
	CFileFind finder;
	BOOL bWorking = FALSE;
	HANDLE hFile;
	char sBuffer[2048];
	DWORD dwRead;
	
#ifdef _DEBUG
	TRACE("[%s] sFilePath = '%s'\n", __FUNCTION__ , sFilePath);
#endif	// DEBUG

	int		ierrCnt = 0;

	double	dwSensitivity = 0;
	double	dwResponse = 0;
	double  dwCalib = 0;

	bWorking = finder.FindFile(sFilePath);

	//FILE *pipe_F = popen(sFilePath, "r");

	if(!bWorking)	// StationInfo.txt ������ ����
	{
		nRet = -2;
		return nRet;
	}

	char strLoc[1024];
	memset(strLoc, 0x00, sizeof(strLoc));

	if(bWorking)	// stationinfo.txt ������ ������
	{
		bWorking = finder.FindNextFile();
#ifdef _DEBUG
		TRACE("[%s] strLoc = '%s'\n", __FUNCTION__, finder.GetFileName());
#endif	// DEBUG

		CString sLoc = finder.GetFileName();
		//memcpy(strLoc, sLoc, sizeof(strLoc));
		//strLoc = sLoc.GetBuffer(sLoc.GetLength());

#ifdef _DEBUG
		TRACE("[%s] strLoc = '%s'\n", __FUNCTION__, sLoc);
#endif	// DEBUG

		//TrimCRLF(strLoc);
		sLoc.Trim();

		hFile = CreateFile(sFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			memset(sBuffer, 0x00, sizeof(sBuffer));
			if( ReadFile(hFile, sBuffer, sizeof(sBuffer), &dwRead, NULL) > 0 )
			{
				//char* pTmpBuffer;
				//int sLen = WideCharToMultiByte(CP_ACP, 0, sBuffer, -1, NULL, 0, NULL, NULL);

				if(dwRead > 0)
				{
					//pTmpBuffer = new char[sLen + 1];
					//memset(pTmpBuffer, 0x00, sizeof(sLen + 1));
					//WideCharToMultiByte(CP_ACP, 0, sBuffer, -1, pTmpBuffer, sLen, NULL, NULL);

					//dwRead = sLen;

					// �����ݷ����� �ڸ� �� ���� �ľ�
					int nAryCnt = 0;

					char* pTmpLine_Cnt = NULL;
					pTmpLine_Cnt = new char[dwRead + 1];
					memset(pTmpLine_Cnt, 0x00, dwRead + 1);
					strncpy(pTmpLine_Cnt, sBuffer, dwRead);

					char *pToken_SemiColon_Cnt = strtok(pTmpLine_Cnt, "\n");
					while (pToken_SemiColon_Cnt != NULL)
					{
						nAryCnt++;
						pToken_SemiColon_Cnt = strtok(NULL, "\n");
					}

					if (nAryCnt > 0)
					{
						//aryStationInfo = new char*[nAryCnt];

						for (int i = 0; i < nAryCnt; i++)
						{
							//aryStationInfo[i] = new char[7];
							//memset(aryStationInfo[i], 0x00, sizeof(char) * 7);
						}					

						int nAryNum = 0;
						bool bDataChk = false;

						char* pTmpLine = NULL;
						char* pTmpSemiColon = NULL;
						char* pTmpColon = NULL;
						char* pTmpComma = NULL;
						pTmpLine = new char[dwRead + 1];
						memset(pTmpLine, 0x00, dwRead + 1);
						strncpy(pTmpLine, sBuffer, dwRead);

	#ifdef _DEBUG
						//char **split;
						//char msg[] = "jsmith:x:1001:1000:Joe Smith,Room 1007,(234)555-8910,(234)555-0044,email:/home/jsmith:/bin/sh";
						//int count = getSplit(msg, ":", &split);

						//for (int i = 0; i < count; ++i)
						//{
						//	printf("[%s] i = '%d', split[i] = '%s'\n", __FUNCTION__, i, split[i]);
						//}
	#endif	// DEBUG
						int nTotalSkipLen = 0;

						char *pToken_SemiColon = strtok(pTmpLine, "\n");
						while (pToken_SemiColon != NULL)
						{
							if (pTmpSemiColon != NULL)
							{
								delete pTmpSemiColon;
								pTmpSemiColon = NULL;
							}
							pTmpSemiColon = new char[strlen(pToken_SemiColon) + 1];
							memset(pTmpSemiColon, 0x00, strlen(pToken_SemiColon) + 1);
							strncpy(pTmpSemiColon, pToken_SemiColon, strlen(pToken_SemiColon));
							pTmpSemiColon[strlen(pToken_SemiColon)] = '\0';

							bDataChk = false;

							Sta_t* pSta = NULL;
							Sta_t sta;
							memset(&sta, 0x00, sizeof(Sta_t));

							int nFieldNum = 0;
							char *pToken_Colon = strtok(pTmpSemiColon, ":");
							while (pToken_Colon != NULL)
							{
								switch (nFieldNum)
								{
								case 0:	// ����ID
									{
										int nLenColon = 0;
										nLenColon = strlen(pToken_Colon) <= sizeof(sta.sSen_Id) ? strlen(pToken_Colon) : sizeof(sta.sSen_Id);
										memcpy(sta.sSen_Id, pToken_Colon, nLenColon);
										pSta = AddStation(&sta);
										//memcpy(aryStationInfo[nAryNum] + nFieldNum, pToken_Colon, strlen(pToken_Colon));
									}								
									break;
								case 1:	// response, sensitivity
									{
										if (pTmpColon != NULL)
										{
											delete pTmpColon;
											pTmpColon = NULL;
										}
										pTmpColon = new char[strlen(pToken_Colon) + 1];
										memset(pTmpColon, 0x00, strlen(pToken_Colon) + 1);
										strncpy(pTmpColon, pToken_Colon, strlen(pToken_Colon));
										pTmpColon[strlen(pToken_Colon)] = '\0';

										Sta_Detail_t staDetail;
										memset(&staDetail, 0x00, sizeof(Sta_Detail_t));

										int nCommaNum = 1;
										char *pToken_Comma = strtok(pTmpColon, ",");
										while (pToken_Comma != NULL)
										{
											int nTokenLen = 0;// strlen(pToken_Comma);
											switch (nCommaNum - 1)
											{
											case 0:	// Z response
												nTokenLen = strlen(pToken_Comma) <= sizeof(staDetail.sZresponse) ? strlen(pToken_Comma) : sizeof(staDetail.sZresponse);
												memcpy(staDetail.sZresponse, pToken_Comma, nTokenLen);
												break;
											case 1:	// N response
												nTokenLen = strlen(pToken_Comma) <= sizeof(staDetail.sNresponse) ? strlen(pToken_Comma) : sizeof(staDetail.sNresponse);
												memcpy(staDetail.sNresponse, pToken_Comma, nTokenLen);
												break;
											case 2:	// E response
												nTokenLen = strlen(pToken_Comma) <= sizeof(staDetail.sEresponse) ? strlen(pToken_Comma) : sizeof(staDetail.sEresponse);
												memcpy(staDetail.sEresponse, pToken_Comma, nTokenLen);
												break;
											case 3:	// Z sensitivity
												nTokenLen = strlen(pToken_Comma) <= sizeof(staDetail.sZsensitivity) ? strlen(pToken_Comma) : sizeof(staDetail.sZsensitivity);
												memcpy(staDetail.sZsensitivity, pToken_Comma, nTokenLen);
												break;
											case 4:	// N sensitivity
												nTokenLen = strlen(pToken_Comma) <= sizeof(staDetail.sNsensitivity) ? strlen(pToken_Comma) : sizeof(staDetail.sNsensitivity);
												memcpy(staDetail.sNsensitivity, pToken_Comma, nTokenLen);
												break;
											case 5:	// E sensitivity
												nTokenLen = strlen(pToken_Comma) <= sizeof(staDetail.sEsensitivity) ? strlen(pToken_Comma) : sizeof(staDetail.sEsensitivity);
												memcpy(staDetail.sEsensitivity, pToken_Comma, nTokenLen);
												break;
											}

#ifdef _DEBUG
											TRACE("[%s] Sen_Id = '%s', nCommaNum = '%d', pToken_Comma = '%s'\n", __FUNCTION__, sta.sSen_Id, nCommaNum - 1, pToken_Comma);
#endif	// DEBUG

											//strcpy(&aryStationInfo[nAryNum][nCommaNum], pToken_Comma);										
											pToken_Comma = strtok(NULL, ",");

											if (pToken_Comma != NULL)
											{
//#ifdef _DEBUG
//												TRACE("[%s] nCommaNum = '%d', pToken_Comma = '%s'\n", __FUNCTION__, nCommaNum, pToken_Comma);
//#endif	// DEBUG
												nCommaNum++;
											}
										}

										if (nCommaNum == 6)
										{
											if (pSta)
											{
												AddStationDetail(pSta, &staDetail);
												pSta->bDataAvailability = true;
											}
											// m_aryStation.AddStationDetail(&staDetail);
											bDataChk = true;
										}
									}
									break;
								}
								nFieldNum++;
								pToken_Colon = strtok(NULL, ":");
							}

							if (bDataChk == true)
							{							
								nAryNum++;
							}

							int nLenTmpLine = strlen(pTmpLine);
							nTotalSkipLen += strlen(pTmpLine) + 1;

							if (nTotalSkipLen > dwRead)
							{
								nTotalSkipLen = dwRead;
							}
							memset(pTmpLine, 0x00, dwRead);
							memcpy(pTmpLine, sBuffer + nTotalSkipLen, dwRead - nTotalSkipLen);
							pToken_SemiColon = strtok(pTmpLine, "\n");
						}

						if (pTmpLine)
						{
							delete pTmpLine;
							pTmpLine = NULL;
						}
						if (pTmpSemiColon)
						{
							delete pTmpSemiColon;
							pTmpSemiColon = NULL;
						}
						if (pTmpColon)
						{
							delete pTmpColon;
							pTmpColon = NULL;
						}
						if (pTmpComma)
						{
							delete pTmpComma;
							pTmpComma = NULL;
						}
					}
					
					if(pTmpLine_Cnt)
					{
						delete pTmpLine_Cnt;
						pTmpLine_Cnt = NULL;
					}

					//if(sLen > 0)
					//{
					//	delete [] pTmpBuffer;
					//}
				}
			}
		}

		CloseHandle(hFile);
	}

	return nRet;
}

//************************************
// Method:    ReadThresholdsInfo2Array
// FullName:  ReadThresholdsInfo2Array
// Access:    public 
// Returns:   int
// Returns:			0 : ����
// Returns:			-2 : ���� - ��������ġ ���� ������ ����(type{�ü��������ڵ�}_thInfo.txt)
// Qualifier:
// Parameter: CString sFilePath
// Parameter: CString sDitc
//************************************
int CMakeEvalutionBaseData::ReadThresholdsInfo2Array(CString sFilePath, CString sDitc)
{
	int nRet = 0;
	CFileFind finder;
	BOOL bWorking = FALSE;
	HANDLE hFile;
	char sBuffer[2048];
	DWORD dwRead;

	CString sFileFullPath = "";
	sFileFullPath.Format("%s\\type%s_thInfo.txt", sFilePath, sDitc);

#ifdef _DEBUG
	TRACE("[%s] sFileFullPath = '%s'\n", __FUNCTION__ , sFileFullPath);
#endif	// DEBUG

	bWorking = finder.FindFile(sFileFullPath);

	//FILE *pipe_F = popen(sFilePath, "r");

	if(!bWorking)	// type{�ü��������ڵ�}_thInfo.txt ������ ����
	{
		nRet = -1;
		return nRet;
	}

	char strLoc[1024];
	memset(strLoc, 0x00, sizeof(strLoc));

	if(bWorking)	// type{�ü��������ڵ�}_thInfo.txt ������ ������
	{
		bWorking = finder.FindNextFile();
#ifdef _DEBUG
		TRACE("[%s] strLoc = '%s'\n", __FUNCTION__, finder.GetFileName());
#endif	// DEBUG

		CString sLoc = finder.GetFileName();
		//memcpy(strLoc, sLoc, sizeof(strLoc));
		//strLoc = sLoc.GetBuffer(sLoc.GetLength());

#ifdef _DEBUG
		TRACE("[%s] strLoc = '%s'\n", __FUNCTION__, sLoc);
#endif	// DEBUG

		//TrimCRLF(strLoc);
		sLoc.Trim();

		hFile = CreateFile(sFileFullPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			memset(sBuffer, 0x00, sizeof(sBuffer));
			if( ReadFile(hFile, sBuffer, sizeof(sBuffer), &dwRead, NULL) > 0 )
			{
				//char* pTmpBuffer;
				//int sLen = WideCharToMultiByte(CP_ACP, 0, sBuffer, -1, NULL, 0, NULL, NULL);

				if(dwRead > 0)
				{
					//pTmpBuffer = new char[sLen + 1];
					//memset(pTmpBuffer, 0x00, sizeof(sLen + 1));
					//WideCharToMultiByte(CP_ACP, 0, sBuffer, -1, pTmpBuffer, sLen, NULL, NULL);

					//dwRead = sLen;

					// �����ݷ����� �ڸ� �� ���� �ľ�
					int nAryCnt = 0;

					char* pTmpLine_Cnt = NULL;
					pTmpLine_Cnt = new char[dwRead + 1];
					memset(pTmpLine_Cnt, 0x00, dwRead + 1);
					strncpy(pTmpLine_Cnt, sBuffer, dwRead);

					char *pToken_SemiColon_Cnt = strtok(pTmpLine_Cnt, "\n");
					while (pToken_SemiColon_Cnt != NULL)
					{
						nAryCnt++;
						pToken_SemiColon_Cnt = strtok(NULL, "\n");
					}

					if (nAryCnt > 0)
					{
						//aryStationInfo = new char*[nAryCnt];

						for (int i = 0; i < nAryCnt; i++)
						{
							//aryStationInfo[i] = new char[7];
							//memset(aryStationInfo[i], 0x00, sizeof(char) * 7);
						}					

						int nAryNum = 0;
						bool bDataChk = false;

						char* pTmpLine = NULL;
						char* pTmpSemiColon = NULL;
						char* pTmpEqual = NULL;
						char* pTmpColon = NULL;
						char* pTmpComma = NULL;
						pTmpLine = new char[dwRead + 1];
						memset(pTmpLine, 0x00, dwRead + 1);
						strncpy(pTmpLine, sBuffer, dwRead);

	#ifdef _DEBUG
						//char **split;
						//char msg[] = "jsmith:x:1001:1000:Joe Smith,Room 1007,(234)555-8910,(234)555-0044,email:/home/jsmith:/bin/sh";
						//int count = getSplit(msg, ":", &split);

						//for (int i = 0; i < count; ++i)
						//{
						//	printf("[%s] i = '%d', split[i] = '%s'\n", __FUNCTION__, i, split[i]);
						//}
	#endif	// DEBUG
						int nTotalSkipLen = 0;

						char *pToken_SemiColon = strtok(pTmpLine, "\n");
						while (pToken_SemiColon != NULL)
						{
							if (pTmpSemiColon != NULL)
							{
								delete pTmpSemiColon;
								pTmpSemiColon = NULL;
							}
							pTmpSemiColon = new char[strlen(pToken_SemiColon) + 1];
							memset(pTmpSemiColon, 0x00, strlen(pToken_SemiColon) + 1);
							strncpy(pTmpSemiColon, pToken_SemiColon, strlen(pToken_SemiColon));
							pTmpSemiColon[strlen(pToken_SemiColon)] = '\0';

							bDataChk = false;

							ThSta_t* pThSta = NULL;
							ThSta_t thSta;
							memset(&thSta, 0x00, sizeof(ThSta_t));

							// = �� ���� ex) PS_PSM=D:0.1,1.02 -> 0 : PS_PSM, 1 : D:0.1,1.02
							int nFieldNum_Equal = 0;
							char *pToken_Equal = strtok(pTmpSemiColon, "=");
							while (pToken_Equal != NULL)
							{
								switch(nFieldNum_Equal)
								{
								case 0:	// ����ID
									{
										int nLenEqual = 0;
										nLenEqual = strlen(pToken_Equal) <= sizeof(thSta.sSen_Id) ? strlen(pToken_Equal) : sizeof(thSta.sSen_Id);
										memcpy(thSta.sSen_Id, pToken_Equal, nLenEqual);
									}
									break;
								case 1:	// �������� ����(A,V,D), minVal, maxVal
									{
										if (pTmpEqual != NULL)
										{
											delete pTmpEqual;
											pTmpEqual = NULL;
										}
										pTmpEqual = new char[strlen(pToken_Equal) + 1];
										memset(pTmpEqual, 0x00, sizeof(pToken_Equal) + 1);
										strncpy(pTmpEqual, pToken_Equal, strlen(pToken_Equal));
										pTmpEqual[strlen(pToken_Equal)] = '\0';

										int nFieldNum_Colon = 0;
										char *pToken_Colon = strtok(pTmpEqual, ":");
										while(pToken_Colon != NULL)
										{
											switch(nFieldNum_Colon)
											{
											case 0:	// �������� ����(A,V,D)
												{
													int nLenColon = 0;
													nLenColon = strlen(pToken_Colon) <= sizeof(thSta.sType) ? strlen(pToken_Colon) : sizeof(thSta.sType);
													memcpy(thSta.sType, pToken_Colon, nLenColon);
													pThSta = AddThStation(&thSta);
												}
												break;
											case 1:	// minVal, maxVal
												{
													if (pTmpColon != NULL)
													{
														delete pTmpColon;
														pTmpColon = NULL;
													}
													pTmpColon = new char[strlen(pToken_Colon) + 1];
													memset(pTmpColon, 0x00, strlen(pToken_Colon) + 1);
													strncpy(pTmpColon, pToken_Colon, strlen(pToken_Colon));
													pTmpColon[strlen(pToken_Colon)] = '\0';

													ThSta_Detail_t thStaDetail;
													memset(&thStaDetail, 0x00, sizeof(ThSta_Detail_t));

													int nCommaNum = 0;
													int nTotalTokenCnt = 1;

													if( sDitc == "S" || sDitc == "B" )		// Ư������
													{
														for( int nLen_Token_Colon = 0; nLen_Token_Colon < strlen(pToken_Colon); nLen_Token_Colon++ )
														{
															if( pToken_Colon[nLen_Token_Colon] == ',' )
															{
																nTotalTokenCnt++;
															}
														}

														nCommaNum = 1;
														char *pToken_Comma = strtok(pTmpColon, ",");
														while (pToken_Comma != NULL)
														{
															int nTokenLen = 0;// strlen(pToken_Comma);
															switch (nCommaNum - 1)
															{
															case 0:	// Min Value
																nTokenLen = strlen(pToken_Comma) <= sizeof(thStaDetail.sMinVal) ? strlen(pToken_Comma) : sizeof(thStaDetail.sMinVal);
																if (nTokenLen <= 0)
																	strcpy(thStaDetail.sMinVal, "-1");
																else
																	memcpy(thStaDetail.sMinVal, pToken_Comma, nTokenLen);
																break;
															case 1:	// Max Value
																nTokenLen = strlen(pToken_Comma) <= sizeof(thStaDetail.sMaxVal) ? strlen(pToken_Comma) : sizeof(thStaDetail.sMaxVal);
																if (nTokenLen <= 0)
																	strcpy(thStaDetail.sMaxVal, "-1");
																else
																	memcpy(thStaDetail.sMaxVal, pToken_Comma, nTokenLen);
																break;
															case 2:	// �����
																nTokenLen = strlen(pToken_Comma) <= sizeof(thStaDetail.sDampingRatio) ? strlen(pToken_Comma) : sizeof(thStaDetail.sDampingRatio);
																if (nTokenLen <= 0)
																	strcpy(thStaDetail.sDampingRatio, "0.05");
																else
																	memcpy(thStaDetail.sDampingRatio, pToken_Comma, nTokenLen);
																break;
															case 3:	// �߷�
																nTokenLen = strlen(pToken_Comma) <= sizeof(thStaDetail.sGravity) ? strlen(pToken_Comma) : sizeof(thStaDetail.sGravity);
																if (nTokenLen <= 0)
																	strcpy(thStaDetail.sGravity, "1");
																else
																	memcpy(thStaDetail.sGravity, pToken_Comma, nTokenLen);
																break;
															case 4:	// �������
																nTokenLen = strlen(pToken_Comma) <= sizeof(thStaDetail.sCoefficient) ? strlen(pToken_Comma) : sizeof(thStaDetail.sCoefficient);
																if (nTokenLen <= 0)
																	strcpy(thStaDetail.sCoefficient, "2.8");
																else
																	memcpy(thStaDetail.sCoefficient, pToken_Comma, nTokenLen);
																break;
															case 5:	// ��ȿ�������ݰ��ӵ�
																nTokenLen = strlen(pToken_Comma) <= sizeof(thStaDetail.sEffectivePGA) ? strlen(pToken_Comma) : sizeof(thStaDetail.sEffectivePGA);
																if (nTokenLen <= 0)
																{
																	if( strlen(thStaDetail.sMaxVal) > 0 )
																	{
																		//sprintf(thStaDetail.sEffectivePGA, "%.3f", (atof(thStaDetail.sMaxVal)) / 981);
																		sprintf(thStaDetail.sEffectivePGA, "%s", thStaDetail.sMaxVal);	// 2019.11.26 ���� ������ ������ ��û���� ����
																	}
																	else
																	{

																		//strcpy(thStaDetail.sEffectivePGA, "0.068");	// 2019.10.10 ���ӵ� �⺻ ��������ġ�� 67gal -> g ȯ�갪���� �ݿ�(�þȰ� �ڼ���)
																		strcpy(thStaDetail.sEffectivePGA, "67");	// 2019.11.26 ���� ������ ������ ��û���� ����
																	}
																	//strcpy(thStaDetail.sEffectivePGA, "0.154");
																}
																else
																{
																	memcpy(thStaDetail.sEffectivePGA, pToken_Comma, nTokenLen);
																}
																break;
															}

	#ifdef _DEBUG
															TRACE("[%s] Sen_Id = '%s', Type = '%s', nCommaNum = '%d', pToken_Comma = '%s'\n", __FUNCTION__, thSta.sSen_Id, thSta.sType, nCommaNum - 1, pToken_Comma);
	#endif	// DEBUG

															//strcpy(&aryStationInfo[nAryNum][nCommaNum], pToken_Comma);										
															pToken_Comma = strtok(NULL, ",");

															if (pToken_Comma != NULL)
															{
																//#ifdef _DEBUG
																//												TRACE("[%s] nCommaNum = '%d', pToken_Comma = '%s'\n", __FUNCTION__, nCommaNum, pToken_Comma);
																//#endif	// DEBUG
																nCommaNum++;
															}
														}
													}
													else if( sDitc == "K" )
													{
														nCommaNum = 0;

														if( pTmpColon != NULL )
														{
															int nLen = strlen(pTmpColon) <= sizeof(thStaDetail.sMaxVal) ? strlen(pTmpColon) : sizeof(thStaDetail.sMaxVal);
															memcpy(thStaDetail.sMaxVal, pTmpColon, nLen);

															nCommaNum++;
														}
													}

													if ( ((sDitc == "S" || sDitc == "B") && nTotalTokenCnt == 6) ||	// Ư������ ���ӵ��� min, max, �����, �߷�, �������, ��ȿ�������ݰ��ӵ� 6���� ������ ����
														(sDitc == "K" && nCommaNum == 1) )						// ��ũ��Ʈ���� max 1���� ������ ����
													{
														if( ((sDitc == "S" || sDitc == "B") && thSta.sType[0] == 'A' && nTotalTokenCnt == 6) )
														{
															// ����� �⺻�� ����
															if( strlen(thStaDetail.sDampingRatio) <= 0 )
															{
																strcpy(thStaDetail.sDampingRatio, "0.05");
															}

															// �߷� �⺻�� ����
															if( strlen(thStaDetail.sGravity) <= 0 )
															{
																strcpy(thStaDetail.sGravity, "1");
															}

															// ������� �⺻�� ����
															if( strlen(thStaDetail.sCoefficient) <= 0 )
															{
																strcpy(thStaDetail.sCoefficient, "2.8");
															}

															// ��ȿ�������ݰ��ӵ� �⺻�� ����
															if( strlen(thStaDetail.sEffectivePGA) <= 0 )
															{
																if( strlen(thStaDetail.sMaxVal) > 0 )
																{
																	sprintf(thStaDetail.sEffectivePGA, "%.3f", (atof(thStaDetail.sMaxVal)) / 981);
																}
																else
																{
																	strcpy(thStaDetail.sEffectivePGA, "0.068");
																}																
															}
														}

														if (pThSta)
														{
															AddThStationDetail(pThSta, &thStaDetail);
															pThSta->bDataAvailability = true;
														}
														// m_aryStation.AddStationDetail(&staDetail);
														bDataChk = true;
													}
												}												
												break;
											}
											nFieldNum_Colon++;
											pToken_Colon = strtok(NULL, ":");
										}
									}
									break;
								}
								nFieldNum_Equal++;
								pToken_Equal = strtok(NULL, "=");
							}

							if (bDataChk == true)
							{							
								nAryNum++;
							}

							int nLenTmpLine = strlen(pTmpLine);
							nTotalSkipLen += strlen(pTmpLine) + 1;

							if (nTotalSkipLen > dwRead)
							{
								nTotalSkipLen = dwRead;
							}
							memset(pTmpLine, 0x00, dwRead);
							memcpy(pTmpLine, sBuffer + nTotalSkipLen, dwRead - nTotalSkipLen);
							pToken_SemiColon = strtok(pTmpLine, "\n");
						}

						if (pTmpLine)
						{
							delete pTmpLine;
							pTmpLine = NULL;
						}
						if (pTmpSemiColon)
						{
							delete pTmpSemiColon;
							pTmpSemiColon = NULL;
						}
						if (pTmpEqual)
						{
							delete pTmpEqual;
							pTmpEqual = NULL;
						}
						if (pTmpColon)
						{
							delete pTmpColon;
							pTmpColon = NULL;
						}
						if (pTmpComma)
						{
							delete pTmpComma;
							pTmpComma = NULL;
						}
					}
					
					if(pTmpLine_Cnt)
					{
						delete pTmpLine_Cnt;
						pTmpLine_Cnt = NULL;
					}
				}
			}
		}

		CloseHandle(hFile);
	}

	return nRet;
}

int CMakeEvalutionBaseData::ReadEtcConfig(CString sFilePath)
{
	int nRet = 0;
	CFileFind finder;
	BOOL bWorking = FALSE;
	HANDLE hFile;
	char sBuffer[2048];
	DWORD dwRead;

	int		ierrCnt = 0;
	CString strLog = "";

	CString strEtcConfigFilePath = "";
	strEtcConfigFilePath.Format("%s\\etc_config.txt", sFilePath);

	strLog.Format("EtcCfgPath = '%s'", strEtcConfigFilePath);
	WriteReportLog(strLog);
	
#ifdef _DEBUG
	TRACE("[%s] strEtcConfigFilePath = '%s'\n", __FUNCTION__ , strEtcConfigFilePath);
#endif	// DEBUG

	bWorking = finder.FindFile(strEtcConfigFilePath);

	//FILE *pipe_F = popen(sFilePath, "r");

	if(!bWorking)	// etc_config.txt ������ ����
	{
		nRet = -1;
		return nRet;
	}

	char strLoc[1024];
	memset(strLoc, 0x00, sizeof(strLoc));

	if(bWorking)	// etc_config.txt ������ ������
	{
		bWorking = finder.FindNextFile();
#ifdef _DEBUG
		TRACE("[%s] strLoc = '%s'\n", __FUNCTION__, finder.GetFileName());
#endif	// DEBUG

		CString sLoc = finder.GetFileName();
		//memcpy(strLoc, sLoc, sizeof(strLoc));
		//strLoc = sLoc.GetBuffer(sLoc.GetLength());

#ifdef _DEBUG
		TRACE("[%s] strLoc = '%s'\n", __FUNCTION__, sLoc);
#endif	// DEBUG

		//TrimCRLF(strLoc);
		sLoc.Trim();

		hFile = CreateFile(strEtcConfigFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			memset(sBuffer, 0x00, sizeof(sBuffer));
			if( ReadFile(hFile, sBuffer, sizeof(sBuffer), &dwRead, NULL) > 0 )
			{
				if(dwRead > 0)
				{
					char* pTitle = NULL;
					char* pEndContents = NULL;
					char* pContents = NULL;
					int nTitleLen = 0;
					int nContentsLen = 0;

					// WarningLimit (���� �ʿ� ���� ���ġ)
					pTitle = strstr(sBuffer, "WarningLimit=");
					if( pTitle != NULL )
					{
						nTitleLen = strlen("WarningLimit=");
						pEndContents = strstr(pTitle, "\n");

						if( pEndContents != NULL )
						{
							nContentsLen = pEndContents - pTitle - nTitleLen;
							if( nContentsLen > 0 )
							{
								pContents = new char[nContentsLen + 1];
								memset(pContents, 0x00, nContentsLen + 1);
								strncpy(pContents, pTitle + nTitleLen, nContentsLen);
								pContents[nContentsLen] = '\0';

								m_nWarningLimit = atoi(pContents);

								if( pContents )
								{
									delete pContents;
									pContents = NULL;
								}
							}
						}
					}

					// MinAvailChanCnt (���� ������ �� ������ ������ �ּ� ����)
					pTitle = strstr(sBuffer, "MinAvailChanCnt=");
					if( pTitle != NULL )
					{
						nTitleLen = strlen("MinAvailChanCnt=");
						pEndContents = strstr(pTitle, "\n");

						if( pEndContents != NULL )
						{
							nContentsLen = pEndContents - pTitle - nTitleLen;
							if( nContentsLen > 0 )
							{
								pContents = new char[nContentsLen + 1];
								memset(pContents, 0x00, nContentsLen + 1);
								strncpy(pContents, pTitle + nTitleLen, nContentsLen);
								pContents[nContentsLen] = '\0';

								m_nMinAvailChanCnt = atoi(pContents);

								if( pContents )
								{
									delete pContents;
									pContents = NULL;
								}
							}
						}
					}
				}
			}
		}

		CloseHandle(hFile);
	}

	return nRet;
}

Sta_t* CMakeEvalutionBaseData::AddStation(Sta_t* sta)
{
	if (sta == NULL) return NULL;
	Sta_t* pNew = new Sta_t();
	memcpy(pNew, sta, sizeof(Sta_t));
	pNew->pArySta_Detail = new CAry();
	m_aryStation.Add(pNew);

	return pNew;
}

int CMakeEvalutionBaseData::DelStation(Sta_t* sta)
{
	int cnt = m_aryStation.GetCount();
	for (int i = 0; i < cnt; i++)
	{
		Sta_t* pSrc = (Sta_t*)m_aryStation.GetAt(i);
		if (pSrc == NULL)
			continue;
		if (pSrc != sta)
			continue;
		m_aryStation.RemoveAt(i);
		return 0;
	}
	return 0;
}

Sta_t* CMakeEvalutionBaseData::GetStation(int idx)
{
	if (idx < 0 || idx >= m_aryStation.GetCount())
		return 0;
	return (Sta_t*)m_aryStation.GetAt(idx);
}

Sta_t* CMakeEvalutionBaseData::GetStation(char* sSen_Id)
{
	if (sSen_Id == NULL) return NULL;
	if (GetStationCount() <= 0) return NULL;

	for (int i = 0; i < GetStationCount(); i++)
	{
		Sta_t* pSta = (Sta_t*)m_aryStation.GetAt(i);
		if (pSta == NULL) continue;

		if (strcmp(pSta->sSen_Id, sSen_Id) == 0)
			return pSta;
	}
	return NULL;
}

int CMakeEvalutionBaseData::GetStationCount()
{
	return m_aryStation.GetCount();
}

Sta_Detail_t* CMakeEvalutionBaseData::AddStationDetail(Sta_t* pSta, Sta_Detail_t* pDetail)
{
	if (pSta == NULL || pDetail == NULL)
		return NULL;

	Sta_t* pSrc = GetStation(pSta->sSen_Id);
	if (!pSrc)
	{
		return NULL;
	}
	Sta_Detail_t* pNew = new Sta_Detail_t();
	memset(pNew, 0x00, sizeof(Sta_Detail_t));
	memcpy(pNew, pDetail, sizeof(Sta_Detail_t));
	pSta->pArySta_Detail->Add(pNew);
	return pNew;
}

int CMakeEvalutionBaseData::DelAllStationDetail(Sta_t* pSta)
{
	if (pSta == NULL)
		return -1;
	if (pSta->pArySta_Detail == NULL)
		return -1;

	for (int i = pSta->pArySta_Detail->GetCount(); 0 <= i; --i)
	{
		Sta_Detail_t* pStaDetail = (Sta_Detail_t*)pSta->pArySta_Detail->GetAt(i);
		if (pStaDetail == NULL) continue;

		delete pStaDetail;
		pSta->pArySta_Detail->RemoveAt(i);
	}
	return pSta->pArySta_Detail->RemoveAll();
}

ThSta_t* CMakeEvalutionBaseData::AddThStation(ThSta_t* thSta)
{
	if (thSta == NULL) return NULL;
	ThSta_t* pNew = new ThSta_t();
	memcpy(pNew, thSta, sizeof(ThSta_t));
	pNew->pAryThSta_Detail = new CAry();
	m_aryThStation.Add(pNew);

	return pNew;
}

int CMakeEvalutionBaseData::DelThStation(ThSta_t* thSta)
{
	int cnt = m_aryThStation.GetCount();
	for (int i = 0; i < cnt; i++)
	{
		ThSta_t* pSrc = (ThSta_t*)m_aryThStation.GetAt(i);
		if (pSrc == NULL)
			continue;
		if (pSrc != thSta)
			continue;
		m_aryThStation.RemoveAt(i);
		return 0;
	}
	return 0;
}

ThSta_t* CMakeEvalutionBaseData::GetThStation(int idx)
{
	if (idx < 0 || idx >= m_aryThStation.GetCount())
		return 0;
	return (ThSta_t*)m_aryThStation.GetAt(idx);
}

ThSta_t* CMakeEvalutionBaseData::GetThStation(char* sSen_Id)
{
	if (sSen_Id == NULL) return NULL;
	if (GetThStationCount() <= 0) return NULL;

	for (int i = 0; i < GetThStationCount(); i++)
	{
		ThSta_t* pThSta = (ThSta_t*)m_aryThStation.GetAt(i);
		if (pThSta == NULL) continue;

		if (strcmp(pThSta->sSen_Id, sSen_Id) == 0)
			return pThSta;
	}
	return NULL;
}

ThSta_t* CMakeEvalutionBaseData::GetThStation(char* sSen_Id, char* sType)
{
	if (sSen_Id == NULL) return NULL;
	if (sType == NULL) return NULL;
	if (GetThStationCount() <= 0) return NULL;

	for (int i = 0; i < GetThStationCount(); i++)
	{
		ThSta_t* pThSta = (ThSta_t*)m_aryThStation.GetAt(i);
		if (pThSta == NULL) continue;

		if (strcmp(pThSta->sSen_Id, sSen_Id) == 0 && strcmp(pThSta->sType, sType) == 0)
			return pThSta;
	}
	return NULL;
}

int CMakeEvalutionBaseData::GetThStationCount()
{
	return m_aryThStation.GetCount();
}

CAry* CMakeEvalutionBaseData::GetAryThStation()
{
	return &m_aryThStation;
}

ThSta_Detail_t* CMakeEvalutionBaseData::AddThStationDetail(ThSta_t* pThSta, ThSta_Detail_t* pDetail)
{
	if (pThSta == NULL || pDetail == NULL)
		return NULL;

	ThSta_t* pSrc = GetThStation(pThSta->sSen_Id);
	if (!pSrc)
	{
		return NULL;
	}
	ThSta_Detail_t* pNew = new ThSta_Detail_t();
	memset(pNew, 0x00, sizeof(ThSta_Detail_t));
	memcpy(pNew, pDetail, sizeof(ThSta_Detail_t));
	pThSta->pAryThSta_Detail->Add(pNew);
	return pNew;
}

int CMakeEvalutionBaseData::DelAllThStationDetail(ThSta_t* pThSta)
{
	if (pThSta == NULL)
		return -1;
	if (pThSta->pAryThSta_Detail == NULL)
		return -1;

	for (int i = pThSta->pAryThSta_Detail->GetCount(); 0 <= i; --i)
	{
		ThSta_Detail_t* pThStaDetail = (ThSta_Detail_t*)pThSta->pAryThSta_Detail->GetAt(i);
		if (pThStaDetail == NULL) continue;

		delete pThStaDetail;
		pThSta->pAryThSta_Detail->RemoveAt(i);
	}
	return pThSta->pAryThSta_Detail->RemoveAll();
}

//************************************
// Method:    MakeMseed2Txt
// FullName:  CMakeEvalutionBaseData::MakeMseed2Txt
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: char * desc
// Parameter: char * szNet
// Parameter: char * szSta
//************************************
void CMakeEvalutionBaseData::MakeMseed2Txt(char* desc, char* szNet, char* szSta)
{
	//char	szDateUTC[100];
	//char	cpFilePath[1024];
	char	* szFuncName = "[MakeMseed2Txt]";
	CString	strLoc = "";
	CString strOutFilePath = "";

	//strOutFilePath.Format("%s\\AccWeb\\ROOT\\data\\Event\\%4.4s\\%s\\%s\\%s", iniFunc.m_strAccHome, szOriginKST, DEF_HEVT, szOriginKST, DEF_MSEED_DIR);
	strOutFilePath.Format("%s\\%s", desc, PATH_RESULT);
	CreatePathDirectory(strOutFilePath);

	// file find
	CFileFind finder;
	BOOL bWorking = FALSE;
	int  iFileCount = 0;

	double	dwSensitivity = 0;
	double	dwResponse = 0;
	double	dwCalib = 0;

	//stamp2stringT2(iStartTime * 1, szDateUTC);

	int nConvertRet = 0;

	CString cpFilePath = "";
	cpFilePath.Format("%s\\%s_%s*%s", desc, szNet, szSta, DEF_MSEED_EXT);

	bWorking = finder.FindFile(cpFilePath);

	while( bWorking )
	{
		//WindowMsgPump();

		float fResponse = 0, fSensitivity = 0;

		bWorking = finder.FindNextFile();
		strLoc = finder.GetFileName();

		char sSenId[8];
		char sChannelId[4];
		char sLocation[1024];
		memset(sSenId, 0x00, sizeof(sSenId));
		memset(sChannelId, 0x00, sizeof(sChannelId));
		memset(sLocation, 0x00, sizeof(sLocation));

		sprintf(sSenId, "%s", strLoc.Mid(0, 6));
		sprintf(sChannelId, "%s", strLoc.Mid(7, 3));

		Sta_t* pSta = NULL;
		Sta_Detail_t* pStaDetail = NULL;

		CString strStationInfoFileFullPath = "";
		strStationInfoFileFullPath.Format("%s\\%s", desc, STATIONINFO_TXT);
		int nReadRet = ReadStationInfo2Array(strStationInfoFileFullPath);

		if (GetStationCount() > 0)
		{
			BOOL bSetDataYN = FALSE;

			for (int i = 0; i < GetStationCount(); i++)
			{
				if(bSetDataYN == TRUE)
				{
					break;
				}

				pSta = (Sta_t*)m_aryStation.GetAt(i);
				if (pSta == NULL) continue;
				if (strcmp(pSta->sSen_Id, sSenId) == 0 && pSta->bDataAvailability == true)
				{
					for (int j = 0; j < pSta->pArySta_Detail->GetCount(); j++)
					{
						pStaDetail = (Sta_Detail_t*)pSta->pArySta_Detail->GetAt(j);
						if (pStaDetail == NULL) continue;

						if (sChannelId[2] == 'Z')
						{
							dwResponse = (float)(atof(pStaDetail->sZresponse));
							dwSensitivity = (float)(atof(pStaDetail->sZsensitivity));
							bSetDataYN = TRUE;
						}
						else if (sChannelId[2] == 'N' || sChannelId[2] == 'Y')
						{
							dwResponse = (float)(atof(pStaDetail->sNresponse));
							dwSensitivity = (float)(atof(pStaDetail->sNsensitivity));
							bSetDataYN = TRUE;
						}
						else if (sChannelId[2] == 'E' || sChannelId[2] == 'X')
						{
							dwResponse = (float)(atof(pStaDetail->sEresponse));
							dwSensitivity = (float)(atof(pStaDetail->sEsensitivity));
							bSetDataYN = TRUE;
						}
						else
						{
							dwResponse = (float)(atof(pStaDetail->sZresponse));
							dwSensitivity = (float)(atof(pStaDetail->sZsensitivity));
							bSetDataYN = TRUE;
						}
						break;
					}					
				}
			}
		}

		if (pSta == NULL || pStaDetail == NULL)
		{
			// ���п� ���� ���� ����
			continue;
		}

#ifdef _DEBUG
		TRACE("[%s] strChannelId = '%s', dwResponse = '%lf', dwSensitivity = '%lf'\n", __FUNCTION__, sChannelId, dwResponse, dwSensitivity);
#endif	// DEBUG

		memcpy(sLocation, sSenId + 5, 1);

		char szSaveFileName[1024 + 1];
		memset(szSaveFileName, 0x00, sizeof(szSaveFileName));
		sprintf(szSaveFileName, "%s\\%s", desc, strLoc);

		char szMakeOrgFileName[1024 + 1];
		memset(szMakeOrgFileName, 0x00, sizeof(szMakeOrgFileName));
		sprintf(szMakeOrgFileName, "%s\\%s", strOutFilePath, strLoc);

		strLoc.Replace(".mseed", ".txt");

		double dblCalib = 0;

		dblCalib = (((dwSensitivity * 0.000001) / dwResponse) / 9.81) * 981;

		char szMakeFileName[1024 + 1];
		memset(szMakeFileName, 0x00, sizeof(szMakeFileName));
		sprintf(szMakeFileName, "%s\\%s", strOutFilePath, strLoc);

		CString strMakeFileName = "";
		CString strMakeOrgFileName = "";
		strMakeFileName.Format("%s", szMakeFileName);
		strMakeOrgFileName.Format("%s", szMakeOrgFileName);

		DeleteFile(strMakeFileName);		// ������ ������ �𸣴� ����(txt)
		DeleteFile(strMakeOrgFileName);	// ������ ������ �𸣴� ����(mseed)

		BOOL bChk = FALSE;
		BOOL bDataChk = FALSE;
		char sCalib[128];// = NULL;
		memset(sCalib, 0x00, sizeof(sCalib));

		for(int i = 0; i < 128; i++)
		{
			memset(sCalib, 0x00, sizeof(sCalib));

			char sNum[4];
			memset(sNum, 0x00, sizeof(sNum));
			sprintf(sCalib, "%.9f", dblCalib);

			if(strstr(sCalib, "#IND") != NULL)
			{
				strcpy(sCalib, "0");
				break;

				sprintf(sCalib, "%.9f", dblCalib);
				break;
			}

			char sCalib_Org[128];
			memset(sCalib_Org, 0x00, sizeof(sCalib_Org));
			sprintf(sCalib_Org, "%.9f", dblCalib);

			double dblRet = atof(sCalib);

			//if(dblRet == dblCalib)
			if(strcmp(sCalib_Org, sCalib) == 0)
			{
				if(bChk == FALSE)
				{
					bChk = TRUE;
				}
				else
				{
					bDataChk = TRUE;
					break;
				}
			}
		}
		//sprintf(sCalib, "%.20lf", dblCalib);

		//CopyFile(szSaveFileName, szMakeOrgFileName, FALSE);
		//clsDataExtract.mseed2txt(szMakeOrgFileName, dblCalib, szMakeFileName);	// ������ txt ���� ����
		//clsDataExtract.mseed2txt(szSaveFileName, dblCalib, szMakeFileName);	// ������ txt ���� ����
		nConvertRet = mseed2txt(szSaveFileName, sCalib);//, szMakeFileName);	// ������ txt ���� ����
	}
}

//************************************
// Method:    CheckMseedFile_Samprate
// FullName:  CMakeEvalutionBaseData::CheckMseedFile_Samprate
// Access:    public 
// Returns:   int
// Returns:			0  : ����
// Returns:			-1 : ���� - �ʼ��� ���� ������ ����
// Returns:			-2 : ���� - Station Calibration ���� ������ ����(StationInfo.txt)
// Returns:			-3 : ���� - mSEED ���� ������ ���Ͽ��� ����
// Returns:			-4 : ���� - �ν��� �� ���� Ÿ���� mSEED ���� ������
// Returns:			-5 : ���� - mSEED ������ ������ ������ ����
// Qualifier:
// Parameter: char * desc
// Parameter: char * szNet
// Parameter: char * szSta
//************************************
int CMakeEvalutionBaseData::CheckMseedFile_Samprate(char* desc, char* szNet, char* szSta)
{
	int nRet = 0;
	//char	szDateUTC[100];
	//char	cpFilePath[1024];
	//char	* szFuncName = "[CheckMseedFile_Samprate]";
	CString	strLoc = "";
	CString strOutFilePath = "";

	//strOutFilePath.Format("%s\\AccWeb\\ROOT\\data\\Event\\%4.4s\\%s\\%s\\%s", iniFunc.m_strAccHome, szOriginKST, DEF_HEVT, szOriginKST, DEF_MSEED_DIR);
	//strOutFilePath.Format("%s\\%s", desc, PATH_RESULT);
	//CreatePathDirectory(strOutFilePath);

	// file find
	CFileFind finder;
	BOOL bWorking = FALSE;
	int  iFileCount = 0;

	double	dwSensitivity = 0;
	double	dwResponse = 0;
	double	dwCalib = 0;

	//stamp2stringT2(iStartTime * 1, szDateUTC);

	int nConvertRet = 0;

	// 2019.04.17 Station Calibration ���� ���� �о �迭�� ����
	CString strStationInfoFileFullPath = "";
	strStationInfoFileFullPath.Format("%s\\%s", desc, STATIONINFO_TXT);
	int nReadRet = ReadStationInfo2Array(strStationInfoFileFullPath);

	if( nReadRet == -2 )	// StationInfo.txt ������ ����
	{
		nRet = nReadRet;
		return nRet;
	}

	char sFileName[1024];
	CString cpFilePath = "";
	cpFilePath.Format("%s\\%s_%s*%s", desc, szNet, szSta, DEF_MSEED_EXT);

	bWorking = finder.FindFile(cpFilePath);

	while( bWorking )
	{
		memset(sFileName, 0x00, sizeof(sFileName));
		//WindowMsgPump();

		float fResponse = 0, fSensitivity = 0;

		bWorking = finder.FindNextFile();
		strLoc = finder.GetFileName();

		sprintf(sFileName, "%s", strLoc);

		char sSenId[8];
		char sChannelId[4];
		char sLocation[1024];
		memset(sSenId, 0x00, sizeof(sSenId));
		memset(sChannelId, 0x00, sizeof(sChannelId));
		memset(sLocation, 0x00, sizeof(sLocation));

		sprintf(sSenId, "%s", strLoc.Mid(0, 6));
		sprintf(sChannelId, "%s", strLoc.Mid(7, 3));

		if(sChannelId[0] != 'H')
		{
			continue;
		}

		Sta_t* pSta = NULL;
		Sta_Detail_t* pStaDetail = NULL;		

		if (GetStationCount() > 0)
		{
			BOOL bSetDataYN = FALSE;

			for (int i = 0; i < GetStationCount(); i++)
			{
				if(bSetDataYN == TRUE)
				{
					break;
				}

				pSta = (Sta_t*)m_aryStation.GetAt(i);
				if (pSta == NULL) continue;
				if (strcmp(pSta->sSen_Id, sSenId) == 0 && pSta->bDataAvailability == true)
				{
					for (int j = 0; j < pSta->pArySta_Detail->GetCount(); j++)
					{
						pStaDetail = (Sta_Detail_t*)pSta->pArySta_Detail->GetAt(j);
						if (pStaDetail == NULL) continue;

						if (sChannelId[2] == 'Z')
						{
							dwResponse = (float)(atof(pStaDetail->sZresponse));
							dwSensitivity = (float)(atof(pStaDetail->sZsensitivity));
							bSetDataYN = TRUE;
						}
						else if (sChannelId[2] == 'N' || sChannelId[2] == 'Y')
						{
							dwResponse = (float)(atof(pStaDetail->sNresponse));
							dwSensitivity = (float)(atof(pStaDetail->sNsensitivity));
							bSetDataYN = TRUE;
						}
						else if (sChannelId[2] == 'E' || sChannelId[2] == 'X')
						{
							dwResponse = (float)(atof(pStaDetail->sEresponse));
							dwSensitivity = (float)(atof(pStaDetail->sEsensitivity));
							bSetDataYN = TRUE;
						}
						else
						{
							dwResponse = (float)(atof(pStaDetail->sZresponse));
							dwSensitivity = (float)(atof(pStaDetail->sZsensitivity));
							bSetDataYN = TRUE;
						}
						break;
					}					
				}
			}
		}

		if (pSta == NULL || pStaDetail == NULL)
		{
			// ���п� ���� ���� ����
			continue;
		}

#ifdef _DEBUG
		TRACE("[%s] strChannelId = '%s', dwResponse = '%lf', dwSensitivity = '%lf'\n", __FUNCTION__, sChannelId, dwResponse, dwSensitivity);
#endif	// DEBUG

		memcpy(sLocation, sSenId + 5, 1);

		char szSaveFileName[1024 + 1];
		memset(szSaveFileName, 0x00, sizeof(szSaveFileName));
		sprintf(szSaveFileName, "%s\\%s", desc, strLoc);

		//char szMakeOrgFileName[1024 + 1];
		//memset(szMakeOrgFileName, 0x00, sizeof(szMakeOrgFileName));
		//sprintf(szMakeOrgFileName, "%s\\%s", strOutFilePath, strLoc);

		strLoc.Replace(".mseed", ".txt");

		double dblCalib = 0;

		dblCalib = (((dwSensitivity * 0.000001) / dwResponse) / 9.81) * 981;

		//char szMakeFileName[1024 + 1];
		//memset(szMakeFileName, 0x00, sizeof(szMakeFileName));
		//sprintf(szMakeFileName, "%s\\%s", strOutFilePath, strLoc);

		//CString strMakeFileName = "";
		//CString strMakeOrgFileName = "";
		//strMakeFileName.Format("%s", szMakeFileName);
		//strMakeOrgFileName.Format("%s", szMakeOrgFileName);

		//DeleteFile(strMakeFileName);		// ������ ������ �𸣴� ����(txt)
		//DeleteFile(strMakeOrgFileName);	// ������ ������ �𸣴� ����(mseed)

		BOOL bChk = FALSE;
		BOOL bDataChk = FALSE;
		char sCalib[128];// = NULL;
		memset(sCalib, 0x00, sizeof(sCalib));

		for(int i = 0; i < 128; i++)
		{
			memset(sCalib, 0x00, sizeof(sCalib));

			char sNum[4];
			memset(sNum, 0x00, sizeof(sNum));
			sprintf(sCalib, "%.9f", dblCalib);

			if(strstr(sCalib, "#IND") != NULL)
			{
				strcpy(sCalib, "0");
				break;

				sprintf(sCalib, "%.9f", dblCalib);
				break;
			}

			char sCalib_Org[128];
			memset(sCalib_Org, 0x00, sizeof(sCalib_Org));
			sprintf(sCalib_Org, "%.9f", dblCalib);

			double dblRet = atof(sCalib);

			//if(dblRet == dblCalib)
			if(strcmp(sCalib_Org, sCalib) == 0)
			{
				if(bChk == FALSE)
				{
					bChk = TRUE;
				}
				else
				{
					bDataChk = TRUE;
					break;
				}
			}
		}
		//sprintf(sCalib, "%.20lf", dblCalib);

		//CopyFile(szSaveFileName, szMakeOrgFileName, FALSE);
		//clsDataExtract.mseed2txt(szMakeOrgFileName, dblCalib, szMakeFileName);	// ������ txt ���� ����
		//clsDataExtract.mseed2txt(szSaveFileName, dblCalib, szMakeFileName);	// ������ txt ���� ����
		//mseed2txt(szSaveFileName, sCalib, szMakeFileName);	// ������ txt ���� ����
		nConvertRet = mseed2txt(szSaveFileName, sCalib);//, szMakeFileName);	// mSEED ���� �������� ���ø� ���� üũ

		// ��ũ��Ʈ���̸� ������ ���򼺺�(X,Y) �����Ͱ� ��� �־�� ����
		if (m_strDitc == "K")
		{
			//if (strstr(sFileName, "G_HGN") != NULL || strstr(sFileName, "G_HGE") != NULL)
			if ( (strstr(sFileName + 5, "G_H") != NULL && (strstr(sFileName + 9, "N") != NULL || strstr(sFileName + 9, "Y") != NULL) ) ||
				(strstr(sFileName + 5, "G_H") != NULL && (strstr(sFileName + 9, "E") != NULL || strstr(sFileName + 9, "X") != NULL) ) )
			{
				if (nConvertRet != 0)
				{
					switch( nConvertRet )
					{
					case -3:	// ���Ͽ��� ����
						break;
					case -4:	// �ν��� �� ���� Ÿ���� ������
						break;
					case -5:	// ������ ������ ����
						break;
					default:	// �˼����� ����
						break;
					}
					nRet = nConvertRet;
					break;
				}
			}
		}
		// �ʴ� �� �������̸� ��� ���򼺺�(X �Ǵ� Y ����) �����Ͱ� �־�� ����
		// 2019.04.25 �Ѿ�� - ��������(Y) ���� ����
		else if (m_strDitc == "D")
		{
			//if (strstr(sFileName, "M_HDY") != NULL)
			if ( (strstr(sFileName + 5, "M_H") != NULL && 
				(strstr(sFileName + 9, "Y") != NULL || strstr(sFileName + 9, "N") != NULL) ) )
			{
				if (nConvertRet != 0)
				{
					switch( nConvertRet )
					{
					case -3:	// ���Ͽ��� ����
						break;
					case -4:	// �ν��� �� ���� Ÿ���� ������
						break;
					case -5:	// ������ ������ ����
						break;
					default:	// �˼����� ����
						break;
					}
					nRet = nConvertRet;
					break;
				}				
			}
		}
		// Ư�������̸� ?? �־�� ����
		else if (m_strDitc == "S" || m_strDitc == "B")
		{

		}
	}

	return nRet;
}

void CMakeEvalutionBaseData::WriteReportLog(CString strLog)
{
	if( iniFunc->m_iLogLevel == 0 )
	{
		return;
	}

	CString strNote = "";
	strNote.Format("(%s) %s", m_sEventID, strLog);

	char*	szlogPtr;
	char	szReportLogFile[1024];

	strcpy(szReportLogFile, "BuildingEvalution_v2.log");

	szlogPtr = strNote.GetBuffer(strNote.GetLength());
	logFunc->WriteLog(szReportLogFile, szlogPtr);

	char sLogMsg[2048];
	memset(sLogMsg, 0x00, sizeof(sLogMsg));
	sprintf(sLogMsg, "%s", szlogPtr);
	SendTransLog(sLogMsg);

	strNote.ReleaseBuffer();
	strLog.ReleaseBuffer();
}

//************************************
// Method:    mseed2txt
// FullName:  CMakeEvalutionBaseData::mseed2txt
// Access:    public 
// Returns:   int
// Returns:			1  : ����
// Returns:			-1 : ���� - ���Ͽ��� ����
// Returns:			-2 : ���� - �ν��� �� ���� Ÿ���� ������
// Returns:			-3 : ���� - ������ ������ ����
// Qualifier:
// Parameter: char * szOrgFile
// Parameter: double fCalib
//************************************
int CMakeEvalutionBaseData::mseed2txt(char* szOrgFile, double fCalib)//, char* szMakeFile)
{
	int ii,kk;//, j;
	int itmp ;
	int nsamp = 0;
	int verbose = 0;
	//FILE *lsFD;
	int ival, samprate;
	float fval;
	double dval, Calib, wk_AccVal;
	char srcname[256];

	float wk_waveform[SAMP_N];
	//float wk_dum[10];
	float wk_avgbuff[100];
	int   avgsamp=0;

	char fname[512];//, oname[512];
	//char *d;
	char time[25];
	long tmpTime;
	double wk_sum;
	//char buf[MAX_BUF_SIZE];
	double wk_time, sampTime;
	float wk_avg, tmp;
	//struct timespec rqtp, rmtp ;
	int nRet = 0;

	memset(fname, 0x00, sizeof(fname));
	//memset(oname, 0x00, sizeof(oname));

	memcpy(fname, szOrgFile, sizeof(fname));
	//memcpy(oname, szMakeFile, sizeof(oname));
	//Calib = strtod(fCalib, &d);
	Calib = fCalib;

	//fname = argv[1];
	//Calib = strtod(argv[2], &d);
	//oname = argv[3];

	//fprintf(stderr,"%lf == %s\n", Calib, argv[2]);
	//exit(0);

	MSRecord *msr = NULL;
	int retcode;
	//char *record;
	//int reclen;

	//if ((lsFD = fopen(oname, "w+")) == NULL)
	//{
	//	fprintf (stderr, "Out File[%s] Open Error\n", oname);
	//	return;
	//}
	//fprintf(lsFD,"#time(epoch)\t time(YMD_KST)\t count\t Acc \n");

	double dbl_MaxPGA = 0.0;
	double dblCAV = 0.0;
	double dblTotalAccVal = 0.0;

	nRet = 1;	// ��ü ������ ó�� �� �̻��� ������ 1�� ���ϵ� �� �ֵ��� ������ ó���� 1�� �⺻ ����

	while ( (retcode = ms_readmsr (&msr, fname, 512, NULL, NULL, 1, 1, verbose)) == MS_NOERROR )
	{
		/* Generate source name for MSRecord */
		sprintf(srcname,"%s_%s_%s", msr->network,  msr->station, msr->channel);

		wk_time =(double) msr->starttime/1000000; 
		ms_hptime2seedtimestr (msr->starttime, time, 1);
		samprate = msr->fsdh->samprate_fact;
		//fprintf(stderr, "\n%s Samples=%d, type=%c, epoch time=%lf[%s] avgsamp=%d  avg=%4.2f\n",srcname,  msr->numsamples, msr->sampletype, wk_time, time, avgsamp, wk_avg);

		if( msr->numsamples > 0)
		{
			int cnt, samplesize;//col, line
			int lines = (msr->numsamples / 6) + 1;
			void *sptr;
			cnt =0;
			nsamp =  msr->numsamples;
			if ((samplesize = ms_samplesize (msr->sampletype)) == 0)
			{
				ms_log (2, "Unrecognized sample type: '%c'\n", msr->sampletype);
				break;
			}

			/* Read every Samples  */
			for (cnt = 0; cnt < msr->numsamples; cnt++)
			{
				sptr = (char *)msr->datasamples + (cnt * samplesize);

				if (msr->sampletype == 'i')
				{
					//  ms_log (0, "%10d  ", *(int32_t *)sptr);
					ival = *(int32_t *)sptr;
					wk_waveform[cnt] = (float)ival;
					//     fprintf (stderr, "[%10d , %d, %f] ", *(int32_t *)sptr, ival, wk_waveform[cnt]);
				}
				else if (msr->sampletype == 'f')
				{
					//  ms_log (0, "%10.8g  ", *(float *)sptr);
					fval = *(float *)sptr;
					wk_waveform[cnt] = fval;
				}
				else if (msr->sampletype == 'd')
				{
					//  ms_log (0, "%10.10g  ", *(double *)sptr);
					dval = *(double *)sptr;
					wk_waveform[cnt] = (float)dval;
				}
			}

			/* Make Acc Value by Calibration  */
			for(kk=0; kk < nsamp; kk++)
			{
				for(ii=avgsamp; ii>0; ii--)  /* Make moving average   */
				{
					wk_avgbuff[ii] = wk_avgbuff[ii-1];
				}
				wk_avgbuff[0] = wk_waveform[kk];

				if(avgsamp!=100)
					avgsamp = avgsamp+1;

				wk_sum = 0.0;
				for(ii=0; ii < avgsamp; ii++)
				{
					wk_sum = wk_sum + wk_avgbuff[ii];
				}
				wk_avg =(float)wk_sum/avgsamp;   /* Make moving average   */

				//fprintf(stderr, "[%d] samp:%f, %f   sum:%lf  avg=%f\n",kk, wk_avgbuff[0], wk_waveform[kk], wk_sum, wk_avg);

				wk_AccVal = (wk_waveform[kk] - wk_avg) * Calib ;
				sampTime = wk_time + (float)kk/samprate ;
				tmpTime = sampTime;
				tmp = sampTime - tmpTime;
				itmp = (tmp+0.00001) * 100.0; 
				//fprintf(lsFD,"%10.2lf %s.%02d %f %lf\n", sampTime, H_StrDate((time_t)sampTime, strTime, 0), itmp , (float)wk_waveform[kk]-wk_avg, wk_AccVal);
			}

		}
	}
	//fclose(lsFD);

	if ( retcode != MS_ENDOFFILE )
		ms_log (2, "Error reading input file %s: %s\n", fname, ms_errorstr(retcode));

	/* Cleanup memory and close file */
	ms_readmsr (&msr, NULL, 0, NULL, NULL, 0, 0, verbose);

	return nRet;
}

//************************************
// Method:    mseed2txt
// FullName:  CMakeEvalutionBaseData::mseed2txt
// Access:    public 
// Returns:   int
// Returns:			0  : ����
// Returns:			-3 : ���� - ���Ͽ��� ����
// Returns:			-4 : ���� - �ν��� �� ���� Ÿ���� ������
// Returns:			-5 : ���� - ������ ������ ����
// Qualifier:
// Parameter: char * szOrgFile
// Parameter: char * fCalib
//************************************
//void CMakeEvalutionBaseData::mseed2txt(char* szOrgFile, char* fCalib, char* szMakeFile)
int CMakeEvalutionBaseData::mseed2txt(char* szOrgFile, char* fCalib)//, char* szMakeFile)
{
	int kk;//, j;
	int itmp ;
	int nsamp = 0;
	int verbose = 0;
	//FILE *lsFD;
	int samprate;
	double Calib;
	char srcname[256];

	//float wk_dum[10];
	int   avgsamp=0;

	char fname[512];
	char *d;
	char time[25];
	long tmpTime;
	//char buf[MAX_BUF_SIZE];
	double wk_time, sampTime;
	float tmp;
	//struct timespec rqtp, rmtp ;
	int nRet = 0;		// ��ü ������ ó�� �� �̻��� ������ 0�� ����

	memset(fname, 0x00, sizeof(fname));
	//memset(oname, 0x00, sizeof(oname));

	memcpy(fname, szOrgFile, sizeof(fname));
	//memcpy(oname, szMakeFile, sizeof(oname));
	Calib = strtod(fCalib, &d);
	//Calib = fCalib;

	//fname = argv[1];
	//Calib = strtod(argv[2], &d);
	//oname = argv[3];

	//fprintf(stderr,"%lf == %s\n", Calib, argv[2]);
	//exit(0);

	MSRecord *msr = NULL;
	int retcode;
	//char *record;
	//int reclen;

	//if ((lsFD = fopen(oname, "w+")) == NULL)
	//{
	//	fprintf (stderr, "Out File[%s] Open Error\n", oname);
	//	return;
	//}
	//fprintf(lsFD,"#time(epoch)\t time(YMD_KST)\t count\t Acc \n");

	double dbl_MaxPGA = 0.0;
	double dblCAV = 0.0;
	double dblTotalAccVal = 0.0;

	//nRet = 1;	// ��ü ������ ó�� �� �̻��� ������ 1�� ���ϵ� �� �ֵ��� ������ ó���� 1�� �⺻ ����

	while ( (retcode = ms_readmsr (&msr, fname, 512, NULL, NULL, 1, 1, verbose)) == MS_NOERROR )
	{
		/* Generate source name for MSRecord */
		sprintf(srcname,"%s_%s_%s", msr->network,  msr->station, msr->channel);

		wk_time =(double) msr->starttime/1000000; 
		ms_hptime2seedtimestr (msr->starttime, time, 1);
		samprate = msr->fsdh->samprate_fact;
		//fprintf(stderr, "\n%s Samples=%d, type=%c, epoch time=%lf[%s] avgsamp=%d  avg=%4.2f\n",srcname,  msr->numsamples, msr->sampletype, wk_time, time, avgsamp, wk_avg);

		if( msr->numsamples > 0)
		{
			int cnt, samplesize;//col, line
			int lines = (msr->numsamples / 6) + 1;
			cnt =0;
			nsamp =  msr->numsamples;
			if ((samplesize = ms_samplesize (msr->sampletype)) == 0)
			{
				//ms_log (2, "Unrecognized sample type: '%c'\n", msr->sampletype);
				nRet = -4;
				break;
			}

			///* Read every Samples  */
			//for (cnt = 0; cnt < msr->numsamples; cnt++)
			//{
			//	sptr = (char *)msr->datasamples + (cnt * samplesize);

			//	if (msr->sampletype == 'i')
			//	{
			//		//  ms_log (0, "%10d  ", *(int32_t *)sptr);
			//		ival = *(int32_t *)sptr;
			//		wk_waveform[cnt] = (float)ival;
			//		//     fprintf (stderr, "[%10d , %d, %f] ", *(int32_t *)sptr, ival, wk_waveform[cnt]);
			//	}
			//	else if (msr->sampletype == 'f')
			//	{
			//		//  ms_log (0, "%10.8g  ", *(float *)sptr);
			//		fval = *(float *)sptr;
			//		wk_waveform[cnt] = fval;
			//	}
			//	else if (msr->sampletype == 'd')
			//	{
			//		//  ms_log (0, "%10.10g  ", *(double *)sptr);
			//		dval = *(double *)sptr;
			//		wk_waveform[cnt] = (float)dval;
			//	}
			//}

			double dbl_PGA = 0.0;
			int nPrevSampTime = 0;
			int nPrevTmp = 0;

			/* Make Acc Value by Calibration  */
			for( kk = 0; kk < nsamp; kk++)
			{
				//for(ii=avgsamp; ii>0; ii--)  /* Make moving average   */
				//{
				//	wk_avgbuff[ii] = wk_avgbuff[ii-1];
				//}
				//wk_avgbuff[0] = wk_waveform[kk];

				//if(avgsamp!=100)
				//	avgsamp = avgsamp+1;

				//wk_sum = 0.0;
				//for(ii=0; ii < avgsamp; ii++)
				//{
				//	wk_sum = wk_sum + wk_avgbuff[ii];
				//}
				//wk_avg =(float)wk_sum/avgsamp;   /* Make moving average   */

				////fprintf(stderr, "[%d] samp:%f, %f   sum:%lf  avg=%f\n",kk, wk_avgbuff[0], wk_waveform[kk], wk_sum, wk_avg);

				//wk_AccVal = (wk_waveform[kk] - wk_avg) * Calib ;

//#ifdef _DEBUG
//				// wk_AccVal �� gal����
//				double dbl_AccVal = 0.0;
//
//				if(wk_AccVal < 0)	dbl_AccVal = wk_AccVal * -1;
//				else					dbl_AccVal = wk_AccVal;
//
//				if(dbl_AccVal > dbl_PGA)
//				{
//					dbl_PGA = dbl_AccVal;
//				}
//#endif	// DEBUG

				sampTime = wk_time + (float)kk/samprate ;
				tmpTime = sampTime;
				tmp = sampTime - tmpTime;
				itmp = (tmp+0.00001) * 100.0; 
				//fprintf(lsFD,"%10.2lf %s.%02d %f %lf\n", sampTime, H_StrDate((time_t)sampTime, strTime, 0), itmp , (float)wk_waveform[kk]-wk_avg, wk_AccVal);

				double dblTimeInterval = 0.0;
				char sTimeInterval[8];
				memset(sTimeInterval, 0x00, sizeof(sTimeInterval));				

				if (kk != 0)
				{
					if (tmpTime != nPrevSampTime)
					{
						if ((tmpTime - 1 != nPrevSampTime) && (itmp + 100 - 1 != nPrevTmp))
						{
							TRACE("111 tmpTime = '%d', nPrevSampTime = '%d', itmp = '%d', nPrevTmp = '%d'\n", tmpTime, nPrevSampTime, itmp, nPrevTmp);
							nRet = -5;
							break;
						}

						sprintf(sTimeInterval, "%.2f", (itmp + 100 - nPrevTmp) * 0.01);						
					}
					else
					{
						if (itmp - 1 != nPrevTmp)
						{
							TRACE("222 tmpTime = '%d', nPrevSampTime = '%d', itmp = '%d', nPrevTmp = '%d'\n", tmpTime, nPrevSampTime, itmp, nPrevTmp);
							nRet = -5;
							break;
						}

						sprintf(sTimeInterval, "%.2f", (itmp - nPrevTmp) * 0.01);
					}
				}

				nPrevSampTime = tmpTime;
				nPrevTmp = itmp;
			}
//#ifdef _DEBUG
//			if(dbl_PGA > dbl_MaxPGA)
//			{
//				dbl_MaxPGA = dbl_PGA;
//			}
//
//			//TRACE("szMakeFile = '%s', dbl_PGA = '%lf'\n", szMakeFile, dbl_PGA);
//#endif	// DEBUG
		}
	}

#ifdef _DEBUG
	TRACE("dbl_MaxPGA = '%lf'\n", dbl_MaxPGA);
#endif	// DEBUG

	//fclose(lsFD);

	if ( retcode != MS_ENDOFFILE )
	{
		ms_log (2, "Error reading input file %s: %s\n", fname, ms_errorstr(retcode));

		nRet = -3;
	}

	/* Cleanup memory and close file */
	ms_readmsr (&msr, NULL, 0, NULL, NULL, 0, 0, verbose);

	return nRet;
}

int	CMakeEvalutionBaseData::CreatePathDirectory(char* szTargetDir)
{
	CString strTargetDir = "";
	
	strTargetDir.Format("%s", szTargetDir);

	return CreatePathDirectory(strTargetDir);
}

int	CMakeEvalutionBaseData::CreatePathDirectory(CString strTargetDir)
{
	CStringArray aryDirNodes;
	CString		 strDir1;
	CString		 strDir2;
	CString		 strDriveID = _T("");
	int			 iInx;

	strDir1 = strTargetDir;

	strDir1.Replace('/', '\\');

	do
	{
		iInx = strDir1.Find('\\');
		if( iInx >= 0 )
		{
			strDir2 = strDir1.Left(iInx);

			if( strDir2.Find(':') >= 0 )
			{
				strDriveID = strDir2;
			}
			else
			{
				aryDirNodes.Add(strDir2);
			}

			strDir2 = strDir1.Right(strDir1.GetLength() - (iInx + 1));
			strDir1 = strDir2;
		}
	} while( iInx >= 0 );

	if( strDir1.GetLength() > 0 )
	{
		aryDirNodes.Add(strDir1);
	}

	int	iCnt = aryDirNodes.GetCount();
	if( iCnt <= 0 )
	{
		return 0;
	}
	strDir1 = aryDirNodes.GetAt(0);

	if( strDriveID.GetLength() > 0 )
	{
		strDir2  = strDriveID;
		strDir2 += "\\";
		strDir2 += strDir1;

		strDir1 = strDir2;
	}

	if( strDir1.GetLength() > 0 )
	{
		char sDir1[2048];
		memset(sDir1, 0x00, sizeof(sDir1));
		sprintf(sDir1, "%s", strDir1);

		if( _access(sDir1, 0x00) != 0 )
		{
			if( _mkdir(sDir1) != 0 )
			{
				return 0;
			}	
		}
	}
	else
	{
		aryDirNodes.RemoveAll();
		return 0;
	}

	for( int i=1; i<iCnt; i++ )
	{
		strDir2 = aryDirNodes.GetAt(i);
		if( strDir2.GetLength() <= 0 )
		{
			continue;
		}

		strDir1 += _T("\\");
		strDir1 += strDir2;

		char sDir1[2048];
		memset(sDir1, 0x00, sizeof(sDir1));
		sprintf(sDir1, "%s", strDir1);

		if( _access(sDir1, 0x00) != 0 )
		{
			if( _mkdir(sDir1) != 0 )
			{
				return 0;
			}	
		}
	}

	return 1;
}

void CMakeEvalutionBaseData::DeleteAllFile(char* szDir, char* sNet, char* sSta)
{
	CString strName; 
	strName.Format("%s\\%s_%s*.*", szDir, sNet, sSta );
	CFileFind ff;
	BOOL bFind = ff.FindFile( strName );

	while( bFind )
	{
		bFind = ff.FindNextFile();
		if( ff.IsDots() == TRUE || ff.IsDirectory() == TRUE ) continue;
		DeleteFile( ff.GetFilePath() );
	}
	ff.Close();
}

void CMakeEvalutionBaseData::DeleteCalcData(char* szDir, char* sNet, char* sSta)
{
	CString strName; 
	strName.Format("%s\\%s_%s*.*", szDir, sNet, sSta );
	CFileFind ff;
	BOOL bFind = ff.FindFile( strName );

	CString strResult = "";
	CString strCalcResult = "";

	strResult.Format("%s_%s_result.txt", sNet, sSta);
	strCalcResult.Format("%s_%s_calc_result.txt", sNet, sSta);

	while( bFind )
	{
		bFind = ff.FindNextFile();
		if( ff.IsDots() == TRUE || ff.IsDirectory() == TRUE ) continue;
		if( ff.GetFileName() == strResult || ff.GetFileName() == strCalcResult ) continue;
		DeleteFile( ff.GetFilePath() );
	}
	ff.Close();
}

char* CMakeEvalutionBaseData::H_StrDate(time_t ptTime, char *pcDate, int piApplyGMT)
{
	struct tm *lsCday;
	char   lcBuff[17];

	lsCday = piApplyGMT ? gmtime(&ptTime) : localtime(&ptTime);

	sprintf(lcBuff, "%04d%02d%02d%02d%02d%02d",
		lsCday->tm_year + 1900, lsCday->tm_mon + 1, lsCday->tm_mday,
		lsCday->tm_hour,        lsCday->tm_min,     lsCday->tm_sec);
	strncpy(pcDate, lcBuff, 16);

	return pcDate;
}

int CMakeEvalutionBaseData::ReadChanData(char* szInFile, char cDType)
{
	FILE *rFile = NULL;

	rFile = fopen(szInFile, "r" );
	if( rFile == NULL )
	{
		rFile = NULL;
		TRACE("ERROR fopen(%s)\n", szInFile);
		return 0;
	}
	else
	{
		//if( g_iDebug >= 2 )
		//{
		//	TRACE("fopen(%s)\n", szInFile);
		//}
	}

	char	szBuff[1024];
	ChanData_t	ChanData;
	AllCompData_t*	pBfData = NULL;
	int			iret;
	
	int			iTstCnt = 0;
	
	while( fgets(szBuff, sizeof(szBuff)-1, rFile) )
	{
		CFFT_Common::stripCRLF(szBuff);
		if( strlen(szBuff) <= 0 )
		{
			continue;
		}
	
		if(    (szBuff[0] < '0')
			|| (szBuff[0] > '9') )
		{
			continue;
		}
		
		iret = ParsingToken(szBuff);
		if(	iret < 3 )
		{
			TRACE("Error Field count(%d)[%s]\n", iret, szBuff);
			break;
		}
		
		//           1         2
		// 01234567890123456789012345
		// 2016-09-12T11:32:24.050000
		// strcpy(ChanData.szTime, g_szAstr[0]);
		sprintf(ChanData.szTime, "%10.10s %2.2s:%2.2s:%2.2s.%2.2s"
								, g_szAstr[0]		// YYYY-MM-DD
								, &g_szAstr[0][11]	// HH
								, &g_szAstr[0][14]	// MI
								, &g_szAstr[0][17]	// SS
								, &g_szAstr[0][20]	// ss
								);

		//ChanData.ldTime	= (long)(atof(g_szAstr[1]) * 100);
		ChanData.ldTime		= (atof(g_szAstr[1]) * 100);
		ChanData.ldTime		= (long)ChanData.ldTime;
		ChanData.iCount		= atoi(g_szAstr[2]);
		ChanData.dwAcc		= atof(g_szAstr[3]);
		ChanData.cDType		= cDType;
		ChanData.dwRKtime	= atof(g_szAstr[4]);
		
		pBfData = AddUpdateChanData(pBfData, &ChanData);
		
		/*
		iTstCnt++;
		if( iTstCnt > 200 )
		{
			break;
		}
		*/
	}
	
	fclose(rFile);

	return 1;
}

int	CMakeEvalutionBaseData::ParsingToken(char* szBuff)
{
	int 	iCnt;
	int		iFld = 0;
	int		iVld = 0;
	int		iSpaSt = 1;
	int		i;
	
	iCnt = strlen(szBuff);
	
	for( i=0; i<iCnt; i++ )
	{
		if( szBuff[i] == '\n' )
		{
			continue;
		}
		if( szBuff[i] == '\r' )
		{
			continue;
		}
		if( szBuff[i] == '\t' )
		{
			szBuff[i] = ' ';
		}

		/*
		if( szBuff[i] == ':' )
		{
			continue;
		}
		if( szBuff[i] == '/' )
		{
			szBuff[i] = '-';
		}
		*/
		
		if( szBuff[i] == ' ' )
		{
			if( iSpaSt == 1 )
			{
				continue;
			}
			if( iSpaSt == 0 )
			{
				// fprintf(stdout, "[%s]\n", g_szAstr[iFld]);
				iFld++;
				iSpaSt = 1;
				continue;
			}
		}
		else	// ������ ���
		{
			if( iSpaSt == 1 )
			{
				iSpaSt 	= 0;
				iVld 	= 0;
				g_szAstr[iFld][iVld] = szBuff[i];
				g_szAstr[iFld][iVld+1] = '\0';

				// fprintf(stdout, "iFld[%d] iVld[%d]   SpaSt[%d][%s]\n", iFld, iVld, iSpaSt, g_szAstr[iFld]);
				continue;
			}
			
			if( iSpaSt == 0 )
			{
				iVld++;
				g_szAstr[iFld][iVld] = szBuff[i];
				g_szAstr[iFld][iVld+1] = '\0';
				
				// fprintf(stdout, "iFld[%d] iVld[%d]   SpaSt[%d][%s]\n", iFld, iVld, iSpaSt, g_szAstr[iFld]);
				continue;
			}
		}
	}
	return iFld;
}

vector<double> CMakeEvalutionBaseData::ComputeDenCoeffs(int FilterOrder, double Lcutoff, double Ucutoff)
{
	int k;            // loop variables
	double theta;     // PI * (Ucutoff - Lcutoff) / 2.0
	double cp;        // cosine of phi
	double st;        // sine of theta
	double ct;        // cosine of theta
	double s2t;       // sine of 2*theta
	double c2t;       // cosine 0f 2*theta
	vector<double> RCoeffs(2 * FilterOrder);     // z^-2 coefficients 
	vector<double> TCoeffs(2 * FilterOrder);     // z^-1 coefficients
	vector<double> DenomCoeffs;     // dk coefficients
	double PoleAngle;      // pole angle
	double SinPoleAngle;     // sine of pole angle
	double CosPoleAngle;     // cosine of pole angle
	double a;         // workspace variables

	cp = cos(PI * (Ucutoff + Lcutoff) / 2.0);
	theta = PI * (Ucutoff - Lcutoff) / 2.0;
	st = sin(theta);
	ct = cos(theta);
	s2t = 2.0*st*ct;        // sine of 2*theta
	c2t = 2.0*ct*ct - 1.0;  // cosine of 2*theta

	for (k = 0; k < FilterOrder; ++k)
	{
		PoleAngle = PI * (double)(2 * k + 1) / (double)(2 * FilterOrder);
		SinPoleAngle = sin(PoleAngle);
		CosPoleAngle = cos(PoleAngle);
		a = 1.0 + s2t*SinPoleAngle;
		RCoeffs[2 * k] = c2t / a;
		RCoeffs[2 * k + 1] = s2t*CosPoleAngle / a;
		TCoeffs[2 * k] = -2.0*cp*(ct + st*SinPoleAngle) / a;
		TCoeffs[2 * k + 1] = -2.0*cp*st*CosPoleAngle / a;
	}

	DenomCoeffs = TrinomialMultiply(FilterOrder, TCoeffs, RCoeffs);

	DenomCoeffs[1] = DenomCoeffs[0];
	DenomCoeffs[0] = 1.0;
	for (k = 3; k <= 2 * FilterOrder; ++k)
		DenomCoeffs[k] = DenomCoeffs[2 * k - 2];

	for (int i = DenomCoeffs.size() - 1; i > FilterOrder * 2 + 1; i--)
		DenomCoeffs.pop_back();

	return DenomCoeffs;
}

vector<double> CMakeEvalutionBaseData::TrinomialMultiply(int FilterOrder, vector<double> b, vector<double> c)
{
	int i, j;
	vector<double> RetVal(4 * FilterOrder);

	RetVal[2] = c[0];
	RetVal[3] = c[1];
	RetVal[0] = b[0];
	RetVal[1] = b[1];

	for (i = 1; i < FilterOrder; ++i)
	{
		RetVal[2 * (2 * i + 1)] += c[2 * i] * RetVal[2 * (2 * i - 1)] - c[2 * i + 1] * RetVal[2 * (2 * i - 1) + 1];
		RetVal[2 * (2 * i + 1) + 1] += c[2 * i] * RetVal[2 * (2 * i - 1) + 1] + c[2 * i + 1] * RetVal[2 * (2 * i - 1)];

		for (j = 2 * i; j > 1; --j)
		{
			RetVal[2 * j] += b[2 * i] * RetVal[2 * (j - 1)] - b[2 * i + 1] * RetVal[2 * (j - 1) + 1] +
				c[2 * i] * RetVal[2 * (j - 2)] - c[2 * i + 1] * RetVal[2 * (j - 2) + 1];
			RetVal[2 * j + 1] += b[2 * i] * RetVal[2 * (j - 1) + 1] + b[2 * i + 1] * RetVal[2 * (j - 1)] +
				c[2 * i] * RetVal[2 * (j - 2) + 1] + c[2 * i + 1] * RetVal[2 * (j - 2)];
		}

		RetVal[2] += b[2 * i] * RetVal[0] - b[2 * i + 1] * RetVal[1] + c[2 * i];
		RetVal[3] += b[2 * i] * RetVal[1] + b[2 * i + 1] * RetVal[0] + c[2 * i + 1];
		RetVal[0] += b[2 * i];
		RetVal[1] += b[2 * i + 1];
	}

	return RetVal;
}

vector<double> CMakeEvalutionBaseData::ComputeNumCoeffs(int FilterOrder, double Lcutoff, double Ucutoff, vector<double> DenC)
{
	vector<double> TCoeffs;
	vector<double> NumCoeffs(2 * FilterOrder + 1);
	vector<complex<double>> NormalizedKernel(2 * FilterOrder + 1);

	vector<double> Numbers;
	for (double n = 0; n < FilterOrder * 2 + 1; n++)
		Numbers.push_back(n);
	int i;

	TCoeffs = ComputeHP(FilterOrder);

	for (i = 0; i < FilterOrder; ++i)
	{
		NumCoeffs[2 * i] = TCoeffs[i];
		NumCoeffs[2 * i + 1] = 0.0;
	}
	NumCoeffs[2 * FilterOrder] = TCoeffs[FilterOrder];

	double cp[2];
	double Bw, Wn;
	cp[0] = 2 * 2.0*tan(PI * Lcutoff / 2.0);
	cp[1] = 2 * 2.0*tan(PI * Ucutoff / 2.0);

	Bw = cp[1] - cp[0];
	//center frequency
	Wn = sqrt(cp[0] * cp[1]);
	Wn = 2 * atan2(Wn, 4);
	//double kern;
	const std::complex<double> result = std::complex<double>(-1, 0);

	for (int k = 0; k< FilterOrder * 2 + 1; k++)
	{
		NormalizedKernel[k] = std::exp(-sqrt(result)*Wn*Numbers[k]);
	}
	double b = 0;
	double den = 0;
	for (int d = 0; d < FilterOrder * 2 + 1; d++)
	{
		b += real(NormalizedKernel[d] * NumCoeffs[d]);
		den += real(NormalizedKernel[d] * DenC[d]);
	}
	for (int c = 0; c < FilterOrder * 2 + 1; c++)
	{
		NumCoeffs[c] = (NumCoeffs[c] * den) / b;
	}

	for (int i = NumCoeffs.size() - 1; i > FilterOrder * 2 + 1; i--)
		NumCoeffs.pop_back();

	return NumCoeffs;
}

vector<double> CMakeEvalutionBaseData::ComputeLP(int FilterOrder)
{
	vector<double> NumCoeffs(FilterOrder + 1);
	int m;
	int i;

	NumCoeffs[0] = 1;
	NumCoeffs[1] = FilterOrder;
	m = FilterOrder / 2;
	for (i = 2; i <= m; ++i)
	{
		NumCoeffs[i] = (double)(FilterOrder - i + 1)*NumCoeffs[i - 1] / i;
		NumCoeffs[FilterOrder - i] = NumCoeffs[i];
	}
	NumCoeffs[FilterOrder - 1] = FilterOrder;
	NumCoeffs[FilterOrder] = 1;

	return NumCoeffs;
}

vector<double> CMakeEvalutionBaseData::ComputeHP(int FilterOrder)
{
	vector<double> NumCoeffs;
	int i;

	NumCoeffs = ComputeLP(FilterOrder);

	for (i = 0; i <= FilterOrder; ++i)
		if (i % 2) NumCoeffs[i] = -NumCoeffs[i];

	return NumCoeffs;
}

//double* CMakeEvalutionBaseData::filter(double* x, int xCount, vector<double> coeff_b, vector<double> coeff_a)
void CMakeEvalutionBaseData::filter(double* x, double* y, int xCount, vector<double> coeff_b, vector<double> coeff_a)
{
	int len_x = xCount;//x.size();
	int len_b = coeff_b.size();
	int len_a = coeff_a.size();

	vector<double> zi(len_b);

	//vector<double> filter_x(len_x);
	double* filter_x;
	filter_x = (double*)malloc(sizeof(double) * len_x);

	if (len_a == 1)
	{
		for (int m = 0; m<len_x; m++)
		{
			filter_x[m] = coeff_b[0] * x[m] + zi[0];
			for (int i = 1; i<len_b; i++)
			{
				zi[i - 1] = coeff_b[i] * x[m] + zi[i];//-coeff_a[i]*filter_x[m];
			}
		}
	}
	else
	{
		for (int m = 0; m<len_x; m++)
		{
			filter_x[m] = coeff_b[0] * x[m] + zi[0];
			for (int i = 1; i<len_b; i++)
			{
				zi[i - 1] = coeff_b[i] * x[m] + zi[i] - coeff_a[i] * filter_x[m];
			}
		}
	}

	for(int i = 0; i < len_x; i++)
	{
		y[i] = filter_x[i];
	}

	if(filter_x)
	{
		delete filter_x;
	}

	//return filter_x;
}

//************************************
// Method:    CheckMseedFile_Exists
// FullName:  CMakeEvalutionBaseData::CheckMseedFile_Exists
// Access:    public 
// Returns:   int
// Returns:			0  : ����
// Returns:			-1 : ���� - �ʼ��� ���� ������ ����
// Qualifier:
// Parameter: char * sDesc
// Parameter: char * sNet
// Parameter: char * sSta
// Parameter: char * sDitc
//************************************
int CMakeEvalutionBaseData::CheckMseedFile_Exists(char* sDesc, char* sNet, char* sSta, char* sDitc)
{
	int nRet = 0;
	//char T_HCX[8], T_HCY[8], V_HCY[8], W_HCY[8], G_HGE[8], G_HGN[8], G_HGZ[8], M_HCX[8], M_HCY[8], N_HCY[8], B_HCX[8], B_HCY[8], B_HCZ[8], M_HDY[8];
	char sType[8];
	memset(sType, 0x00, sizeof(sType));

	int nExistCnt = 0;	// �ʼ��� ���� �ڷ� ���� ���� (��ũ��Ʈ�� : 1 �̻� ���, �ʴ� : 1 �̻� ���, Ư������(���屳) : , Ư������(������) : )

	// ��ũ��Ʈ���̸� ������ ���򼺺�(X,Y) �����Ͱ� ��� �־�� ����
	if(sDitc[0] == 'K')
	{
		strcpy(sType, "G_HGN");
		if( IsExistMseedFile(sType, sDesc, sNet, sSta, sDitc) > 0 )
		{
			nExistCnt++;
		}		

		strcpy(sType, "G_HGE");
		if( IsExistMseedFile(sType, sDesc, sNet, sSta, sDitc) > 0 )
		{
			nExistCnt++;
		}

		strcpy(sType, "G_HGY");
		if( IsExistMseedFile(sType, sDesc, sNet, sSta, sDitc) > 0 )
		{
			nExistCnt++;
		}

		strcpy(sType, "G_HGX");
		if( IsExistMseedFile(sType, sDesc, sNet, sSta, sDitc) > 0 )
		{
			nExistCnt++;
		}

		if( nExistCnt < 1 )
		{
			nRet = -1;
		}
	}
	// �ʴ� �� �������̸� ��� ���򼺺�(X �Ǵ� Y ����) �����Ͱ� �־�� ����
	// 2019.04.25 �Ѿ�� - ��������(Y) ���� ����
	else if(sDitc[0] == 'D')
	{
#ifndef _DEBUG
		strcpy(sType, "M_HDY");
		if( IsExistMseedFile(sType, sDesc, sNet, sSta, sDitc) > 0 )
		{
			nExistCnt++;
		}

		strcpy(sType, "M_HDN");
		if( IsExistMseedFile(sType, sDesc, sNet, sSta, sDitc) > 0 )
		{
			nExistCnt++;
		}

		if( nExistCnt < 1 )
		{
			nRet = -1;
		}
#endif	// nDEBUG
	}
	// Ư�������̸� ?? �־�� ����
	else if(sDitc[0] == 'S' || sDitc[0] == 'B')
	{

	}

	return nRet;
}

int CMakeEvalutionBaseData::IsExistMseedFile(char* sType, char* sDesc, char* sNet, char* sSta, char* sDitc)
{
	char	sRet[8];
	memset(sRet, 0x00, sizeof(sRet));
	int		nRet = 0;
	char	sLoc[1024];
	CString	strLoc = "";
	char	strFilePath[1024];
	memset(strFilePath, 0x00, sizeof(strFilePath));

	char	sType1[4];		// ��ġ, ���� �� : G_HGZ -> G_H
	char	sType2[4];		// ���� �� : G_HGZ -> Z
	memset(sType1, 0x00, sizeof(sType1));
	memset(sType2, 0x00, sizeof(sType2));
	strncpy(sType1, sType, 3);
	sType1[3] = '\0';
	strncpy(sType2, sType + 4, 1);
	sType2[1] = '\0';

	// file find
	CFileFind finder;
	BOOL bWorking = FALSE;

	int nConvertRet = 0;

	CString cpFilePath = "";
	cpFilePath.Format("%s\\%s_%s*%s", sDesc, sNet, sSta, DEF_MSEED_EXT);

	bWorking = finder.FindFile(cpFilePath);

	while (bWorking)
	{
		memset(sLoc, 0x00, sizeof(sLoc));
		strLoc = "";

		bWorking = finder.FindNextFile();
		strLoc = finder.GetFileName();

		sprintf(sLoc, "%s", strLoc);

		//char* strDirectoryStartPos = strstr(sLoc, sDesc);

		//if (strDirectoryStartPos != NULL)

		if( strlen(sLoc) > 10 )
		{
			//char* strChannelInfoStartPos = strstr(strDirectoryStartPos + strlen(sDesc) + strlen("/") + 5, sType);
			//char* strChannelInfoStartPos = strstr(sLoc + 5, sType);// strlen(sNet) + 1 + strlen(sSta);
			char* pChannelInfoStartPos = strstr(sLoc + 5, sType1);			

			if (pChannelInfoStartPos != NULL)
			{
				char* pIngredientPos = strstr(pChannelInfoStartPos + 4, sType2);

				if (pIngredientPos != NULL)
				{
					strncpy(sRet, pChannelInfoStartPos, 5);
					sRet[5] = '\0';
					break;
				}
			}
		}
	}

	return strlen(sRet);
}

double CMakeEvalutionBaseData::Integral_Acc(SampleStruct_t* data, int nDataCount)
{
	double dblCAV = 0.0;
	double* dwDataArray = (double*)malloc(sizeof(double) * nDataCount);

	for(int i = 0; i < nDataCount; i++)
	{
		dwDataArray[i] = data[i].a;
	}

	for(int j = 0; j < nDataCount; j++)
	{
		if(j == 0)	continue;

		// 2019.11.15 gal -> g �� ��ȯ�Ͽ� g ������ CAV �� �����ϱ� ���Ͽ�, gal -> g ��ȯ (������ 981) ����
		double dblA = fabs(dwDataArray[j - 1]) / 981;
		double dblB = fabs(dwDataArray[j]) / 981;

		if(dwDataArray[j - 1] > 0 && dwDataArray[j] > 0 || dwDataArray[j - 1] < 0 && dwDataArray[j] < 0)
		{
			dblCAV += ((dblA + dblB) * 0.01) / 2;
		}
		else if(dwDataArray[j - 1] < 0 || dwDataArray[j] < 0)
		{
			dblCAV += ((0.01 * (dblA / (dblA + dblB)) * dblA) / 2) + ((0.01 * (1 - (dblA / (dblA + dblB))) * dblB) / 2);
		}
	}

	free(dwDataArray);

	return dblCAV;
}

//************************************
// Method:    WriteCalcResult
// FullName:  CMakeEvalutionBaseData::WriteCalcResult
// Access:    public 
// Returns:   int		0 : ����
// Returns:			1 : ����
// Returns:			-1 : ������ �� ���� ����
// Qualifier:
// Parameter: char * sDitc
// Parameter: char * sObsId
// Parameter: char * sNet
// Parameter: char * sSta
// Parameter: char * sMseedPath
// Parameter: double & dblMaxPGA
// Parameter: double & dblMaxPGV
// Parameter: double & dblMaxDisp
// Parameter: double & dblSettlement
// Parameter: double & dblCAV
//************************************
int CMakeEvalutionBaseData::WriteCalcResult(char* sDitc, char* sObsId, char* sNet, char* sSta, char* sMseedPath, double& dblMaxPGA, double& dblMaxPGV, double& dblMaxDisp, double& dblSettlement, double& dblCAV, int nResSpectrum)
{
	int nRet = 0;
	CString strLog = "";

	char	sObsId1[4];
	char	sObsId2[4];
	memset(sObsId1, 0x00, sizeof(sObsId1));
	memset(sObsId2, 0x00, sizeof(sObsId2));
	strncpy(sObsId1, sObsId + 5, 3);
	sObsId1[3] = '\0';
	strncpy(sObsId2, sObsId + 9, 1);
	sObsId2[1] = '\0';

	char sCalcResultFilePath[1024];
	memset(sCalcResultFilePath, 0x00, sizeof(sCalcResultFilePath));
	sprintf(sCalcResultFilePath, "%s\\%s\\%s_%s_%s", sMseedPath, PATH_RESULT, sNet, sSta, CALC_RESULT);

	int nFileWriteRet = 0;
	FILE *calcResult_file;
	calcResult_file = fopen(sCalcResultFilePath, "a+");
	if( calcResult_file == NULL )
	{
		strLog = "";
		strLog.Format("Calculation result file make or update failed!!");
		WriteReportLog(strLog);

		return nRet;
	}	

	char sWriteBuffer[1024];
	memset(sWriteBuffer, 0x00, sizeof(sWriteBuffer));

	if( sDitc[0] == 'S' || sDitc[0] == 'B' )	// Ư������
	{
		//if( strstr(sObsId, "A_HBX") != NULL || strstr(sObsId, "A_HBY") != NULL || strstr(sObsId, "A_HBZ") != NULL ||		// ��ž����
		//   strstr(sObsId, "C_HBX") != NULL || strstr(sObsId, "C_HBY") != NULL || strstr(sObsId, "D_HBX") != NULL ||		// ��ž�߰�
		//   strstr(sObsId, "H_HBZ") != NULL || strstr(sObsId, "Q_HBY") != NULL || strstr(sObsId, "Q_HBZ") != NULL )		// �Ŵ��߾�
		//if( (strstr(sObsId1, "A_H") != NULL && strstr(sObsId2, "X") != NULL) || (strstr(sObsId1, "A_H") != NULL && strstr(sObsId2, "Y") != NULL) || (strstr(sObsId1, "A_H") != NULL && strstr(sObsId2, "Z") != NULL) ||		// ��ž���� (��ž ���(A��))
		if( (strstr(sObsId1, "A_H") != NULL && strstr(sObsId2, "X") != NULL) || (strstr(sObsId1, "A_H") != NULL && strstr(sObsId2, "Y") != NULL) ||																						// ��ž���� (��ž ���(A��)) // 2019.06.05 Z���� ����
			(strstr(sObsId1, "C_H") != NULL && strstr(sObsId2, "X") != NULL) || (strstr(sObsId1, "C_H") != NULL && strstr(sObsId2, "Y") != NULL) || (strstr(sObsId1, "D_H") != NULL && strstr(sObsId2, "X") != NULL) ||		// ��ž�߰� (��ž ������ġ1, 2)
			(strstr(sObsId1, "H_H") != NULL && strstr(sObsId2, "Z") != NULL) || (strstr(sObsId1, "Q_H") != NULL && strstr(sObsId2, "Y") != NULL) || (strstr(sObsId1, "Q_H") != NULL && strstr(sObsId2, "Z") != NULL) )		// �Ŵ��߾� (���� �ְ氣�� �߾�1, 2)
		{
			// ������ ����
			sprintf(sWriteBuffer, "%s\t%s\t%.6lf\n", sObsId, "D", dblMaxDisp);
		}
		//else if( strstr(sObsId, "B_HBX") != NULL || strstr(sObsId, "B_HBY") != NULL || strstr(sObsId, "B_HBZ") != NULL ||		// ��ž����
		//		 strstr(sObsId, "G_HGN") != NULL || strstr(sObsId, "G_HGE") != NULL || strstr(sObsId, "G_HGZ") != NULL ||		// ������
		//		 strstr(sObsId, "R_HBX") != NULL || strstr(sObsId, "R_HBY") != NULL || strstr(sObsId, "R_HBZ") != NULL )		// ��Ŀ����(������)
		else if( (strstr(sObsId1, "B_H") != NULL && strstr(sObsId2, "X") != NULL) || (strstr(sObsId1, "B_H") != NULL && strstr(sObsId2, "Y") != NULL) || (strstr(sObsId1, "B_H") != NULL && strstr(sObsId2, "Z") != NULL) ||		// ��ž���� (��ž ���� ��ܸ�)
				(strstr(sObsId1, "G_H") != NULL && strstr(sObsId2, "N") != NULL) || (strstr(sObsId1, "G_H") != NULL && strstr(sObsId2, "E") != NULL) || (strstr(sObsId1, "G_H") != NULL && strstr(sObsId2, "Z") != NULL) ||		// ������
				(strstr(sObsId1, "R_H") != NULL && strstr(sObsId2, "X") != NULL) || (strstr(sObsId1, "R_H") != NULL && strstr(sObsId2, "Y") != NULL) || (strstr(sObsId1, "R_H") != NULL && strstr(sObsId2, "Z") != NULL) )		// ��Ŀ����(������) (��Ŀ��� ���)
		{
			// ���ӵ���, ���佺��Ʈ�� ����� ����
			sprintf(sWriteBuffer, "%s\t%s\t%.6lf\n%s\t%s\t%d\n", sObsId, "A", dblMaxPGA, sObsId, "SP", nResSpectrum);
		}
		else
		{
			// ������ �� ���� ����
			nRet = -1;
		}
	}
	else if( sDitc[0] == 'K' )	// ��ũ��Ʈ��
	{
		//if( strstr(sObsId, "G_HGN") != NULL || strstr(sObsId, "G_HGE") != NULL )	// ������
		if( (strstr(sObsId1, "G_H") != NULL && strstr(sObsId2, "N") != NULL) || (strstr(sObsId1, "G_H") != NULL && strstr(sObsId2, "E") != NULL) )	// ������
		{
			char sTmpWriteBuffer[128];
			memset(sTmpWriteBuffer, 0x00, sizeof(sTmpWriteBuffer));

			// PGA�� ����
			sprintf(sTmpWriteBuffer, "%s\t%s\t%.6lf\n", sObsId, "A", dblMaxPGA);
			// CAV�� ����
			sprintf(sWriteBuffer, "%s%s\t%s\t%.6lf\n", sTmpWriteBuffer, sObsId, "C", dblCAV);
		}
		else
		{
			// ������ �� ���� ����
			nRet = -1;
		}
	}
	else if( sDitc[0] == 'D' )	// �ʴ� �� ������
	{
		//if( strstr(sObsId, "M_HBX") != NULL || strstr(sObsId, "M_HBY") != NULL )	// �� ������ �߾�
		if( (strstr(sObsId1, "M_H") != NULL && strstr(sObsId2, "X") != NULL) || (strstr(sObsId1, "M_H") != NULL && strstr(sObsId2, "Y") != NULL) )	// �� ������ �߾�
		{
			// �ջ������� ����
			sprintf(sWriteBuffer, "%s\t%s\t%.6lf\n", sObsId, "S", dblSettlement);
		}
		else
		{
			// ������ �� ���� ����
			nRet = -1;
		}
	}

	if( strlen(sWriteBuffer) > 0 )
	{
#ifdef _DEBUG
		TRACE("[%s] sWriteBuffer = '%s'\n", __FUNCTION__, sWriteBuffer);
#endif	// DEBUG

		nFileWriteRet = fprintf(calcResult_file, "%s", sWriteBuffer);

		if(nFileWriteRet > 0)
		{
			nRet = 1;
		}
	}

	fclose(calcResult_file);

	return nRet;
}

BOOL CMakeEvalutionBaseData::WriteResult(char* sDitc, char* sNet, char* sSta, char* sEventID, char* sMseedPath)
{
	BOOL bRet = FALSE;
	CString strLog = "";

	char sCalcResultFilePath[1024];
	memset(sCalcResultFilePath, 0x00, sizeof(sCalcResultFilePath));
	sprintf(sCalcResultFilePath, "%s\\%s\\%s_%s_%s", sMseedPath, PATH_RESULT, sNet, sSta, CALC_RESULT);

	long nLen = 0;

	FILE *calcResult_file;
	calcResult_file = fopen(sCalcResultFilePath, "r");
	if( calcResult_file == NULL )
	{
		strLog = "";
		strLog.Format("Calculation result file read failed!!");
		WriteReportLog(strLog);

		return bRet;
	}

	nLen = _filelength(fileno(calcResult_file));
	if( nLen <= 0 )
	{
		strLog = "";
		strLog.Format("Calculation result file data not found!! size = '%d'", nLen);
		WriteReportLog(strLog);

		fclose(calcResult_file);
		return bRet;
	}

	char sResultFilePath[1024];
	memset(sResultFilePath, 0x00, sizeof(sResultFilePath));
	sprintf(sResultFilePath, "%s\\%s\\%s_%s_%s", sMseedPath, PATH_RESULT, sNet, sSta, BUILDEVA_RESULT);

	int nFileWriteRet = 0;
	FILE *Result_file;
	Result_file = fopen(sResultFilePath, "w+");
	if( Result_file == NULL )
	{
		strLog = "";
		strLog.Format("Total result file make or update failed!!");
		WriteReportLog(strLog);

		fclose(calcResult_file);
		return bRet;
	}

	char sReadBuffer[4096];
	memset(sReadBuffer, 0x00, sizeof(sReadBuffer));

	fread(sReadBuffer, 1, nLen, calcResult_file);
	char* pStartPos = NULL;
	int nReadBufferLen = 0;

	char sWriteBuffer[1024];
	char sValueKind[4];
	char sCalcValue[16];

	int nResultFaultCnt = 0;	// ���� �ʿ�� �Ǵܵ� ����
									// 2019.05.22 Ư�������� total ���� ����ϸ�, ��ü ��� ���� ���� ��� ���� �ʿ� ������ ���ϰų�, ���� �ʿ� ���������� �Ǵ� - ���� ����
	int nResultAvailCnt = 0;	// 2019.06.14 ���� �Ǵ� ���� �ʿ�� �Ǵܵ� �� ����

	// �� �׸� �����ʿ� ����
	int nAcc_WarnCnt = 0;
	int nSp_WarnCnt = 0;
	int nDis_WarnCnt = 0;
	int nCav_WarnCnt = 0;
	int nStm_WarnCnt = 0;

	// �� �׸� �򰡺Ұ� ����
	int nAcc_FailCnt = 0;
	int nSp_FailCnt = 0;
	int nDis_FailCnt = 0;
	int nCav_FailCnt = 0;
	int nStm_FailCnt = 0;

	// �� �׸� �򰡽õ� ����
	int nAccCheckCnt = 0;
	int nSpCheckCnt = 0;
	int nDisCheckCnt = 0;
	int nCavCheckCnt = 0;
	int nStmCheckCnt = 0;

	if( sDitc[0] == 'S' || sDitc[0] == 'B' )	// Ư������ �� ���� ���� �� ������ ������ġ�� �Ӱ�ġ�� ��
	{
		BOOL bChkAcc = FALSE;	// ���ӵ� ������ üũ ����
		BOOL bChkSP = FALSE;	// ���ӵ� ���佺��Ʈ�� ������ üũ ����
		char sPrevChkSen_Id[16];	// �ռ� ó���� SensorID
		memset(sPrevChkSen_Id, 0x00, sizeof(sPrevChkSen_Id));

		// �Ӱ�ġ ���� �迭��ŭ �ݺ�
		for(int j = 0; j < GetThStationCount(); j++)
		{
			if(strlen(sReadBuffer) <= 0) break;
			memset(sValueKind, 0x00, sizeof(sValueKind));
			nReadBufferLen = strlen(sReadBuffer);

			pStartPos = sReadBuffer;

			ThSta_t* pThSta = (ThSta_t*)m_aryThStation.GetAt(j);
			if ( pThSta == NULL ) continue;

			// SensorID ���� ������ �� ���� �����͸� sWriteBuffer ���� ã��
			char* pBuffer = strstr(pStartPos, pThSta->sSen_Id);
			if ( pBuffer == NULL ) continue;

			char* pDataEndPoint = strstr(pBuffer, "\n");
			if ( pDataEndPoint == NULL ) break;

			if ( strcmp(sPrevChkSen_Id, pThSta->sSen_Id) != 0 )
			{
				memset(sPrevChkSen_Id, 0x00, sizeof(sPrevChkSen_Id));
				memcpy(sPrevChkSen_Id, pThSta->sSen_Id, sizeof(sPrevChkSen_Id));
				bChkAcc = FALSE;
				bChkSP = FALSE;
			}

			// �ʼ��� ������ ���� ���� �ش��ϴ� ������ �Ӱ�ġ�� �� (�� ��� �Ӱ谪�� ������ '���� �Ұ�')
			char* pValueKind = strstr(pBuffer, "\t");
			if( pValueKind == NULL ) continue;
			memset(sValueKind, 0x00, sizeof(sValueKind));
			strncpy(sValueKind, pValueKind + 1, 2);
			if (sValueKind[1] == '\t')
			{
				sValueKind[1] = 0x00;
			}

			char* pCalcValue = NULL;
			if (sValueKind[1] == 0x00)
				pCalcValue = strstr(pValueKind + 1, "\t");
			else
				pCalcValue = strstr(pValueKind + 2, "\t");
			if( pCalcValue == NULL ) continue;
			
			char* pEndCalcValuePos = strstr(pCalcValue, "\n");
			if( pEndCalcValuePos == NULL ) break;

			int nCopySize = sizeof(sCalcValue) > pEndCalcValuePos - pCalcValue ? pEndCalcValuePos - pCalcValue : sizeof(sCalcValue);
			memset(sCalcValue, 0x00, sizeof(sCalcValue));
			strncpy(sCalcValue, pCalcValue, nCopySize);	

			double dblCalcValue = atof(sCalcValue);
			ThSta_Detail_t* pThStaDetail = (ThSta_Detail_t*)pThSta->pAryThSta_Detail->GetAt(0);

			char sResultMsg[16];
			memset(sResultMsg, 0x00, sizeof(sResultMsg));
			
			if (sValueKind[0] == 'A')
			{
				nAccCheckCnt++;
			}
			else if (sValueKind[0] == 'D')
			{
				nDisCheckCnt++;
			}
			else if (sValueKind[0] == 'S' && sValueKind[1] == 'P')
			{
				nSpCheckCnt++;
			}

			if( pThStaDetail == NULL )
			{
//#ifdef _DEBUG
				if ( sValueKind[0] == 'S' && sValueKind[1] == 'P' )
				{
					int nCalcValue = (int)dblCalcValue;
					if( nCalcValue != 1 )
					{
						sprintf(sResultMsg, "%s\n", "�����ʿ�");
						nResultFaultCnt++;
						nResultAvailCnt++;
					}
					else
					{
						sprintf(sResultMsg, "%s\n", "����");
						nResultAvailCnt++;
					}
				}
				else if( sValueKind[0] == 'A' )
				{
					double dblThValue = 67;

					if( dblCalcValue > dblThValue )	// �Ӱ�ġ �ʰ�
					{
						sprintf(sResultMsg, "%s\n", "�����ʿ�");
						nResultFaultCnt++;
						nResultAvailCnt++;
					}
					else
					{
						sprintf(sResultMsg, "%s\n", "����");
						nResultAvailCnt++;
					}
				}
				else if( sValueKind[0] == 'D' )
				{
					sprintf(sResultMsg, "%s\n", "�� �Ұ�");
					nDis_FailCnt++;
				}
//#else
//				sprintf(sResultMsg, "%s\n", "�� �Ұ�");
//
//				if (sValueKind[0] == 'A')
//				{
//					nAcc_FailCnt++;
//				}
//				else if (sValueKind[0] == 'D')
//				{
//					nDis_FailCnt++;
//				}
//				else if (sValueKind[0] == 'S' && sValueKind[1] == 'P')
//				{
//					nSp_FailCnt++;
//				}
//#endif	// DEBUG
			}
			else
			{
				if (sValueKind[0] == 'S' && sValueKind[1] == 'P')
				{
					if (bChkSP == FALSE)
					{
						nResultAvailCnt++;
					}

					if ((int)dblCalcValue == 0)
					{
						sprintf(sResultMsg, "%s\n", "���� �ʿ�");

						if (bChkSP == FALSE)
						{
							nResultFaultCnt++;					
						}
						nSp_WarnCnt++;
					}
					else
					{
						sprintf(sResultMsg, "%s\n", "����");
					}
				}
				else
				{
					double dblThValue = atof(pThStaDetail->sMaxVal);

					if (bChkSP == FALSE)
					{
						nResultAvailCnt++;
					}

					if( dblCalcValue > dblThValue )	// �Ӱ�ġ �ʰ�
					{
						sprintf(sResultMsg, "%s\n", "���� �ʿ�");

						if (bChkSP == FALSE)
						{
							nResultFaultCnt++;
						}

						if (sValueKind[0] == 'A')
						{							
							nAcc_WarnCnt++;
						}
						else if (sValueKind[0] == 'D')
						{
							nDis_WarnCnt++;
						}
					}
					else
					{
						sprintf(sResultMsg, "%s\n", "����");						
					}
				}
			}

			// Result ���Ͽ� �ش� ������ ���� ���� ����
			memset(sWriteBuffer, 0x00, sizeof(sWriteBuffer));
			//sprintf(sWriteBuffer, "%s\t%s\t%s", pThSta->sSen_Id, pThSta->sType, sResultMsg);
			sprintf(sWriteBuffer, "%s\t%s\t%s", pThSta->sSen_Id, sValueKind, sResultMsg);
			fprintf(Result_file, "%s", sWriteBuffer);

			// ���ӵ� �Ǵ� ���ӵ� ���佺��Ʈ���� ���� ������ �����Ͱ� �ϳ� �� �����ϹǷ�('A', 'SP') �ѹ� �� üũ�ϵ��� ���� ���� -1 ó��
			if ( sValueKind[0] == 'A' )
			{
				bChkAcc = TRUE;
				if ( bChkSP == FALSE )
				{
					j--;
				}
			}
			else if ( sValueKind[0] == 'S' && sValueKind[1] == 'P' )
			{
				bChkSP = TRUE;
				if ( bChkAcc == FALSE )
				{
					j--;
				}
			}

			// ó���� �ʼ��� ������ �����͸� sWriteBuffer �������� ����
			pDataEndPoint = strstr(pBuffer, "\n");
			if( pDataEndPoint == NULL)
			{
				break;				
			}
			else
			{
				char sTmpFirstBuffer[4096];
				memset(sTmpFirstBuffer, 0x00, sizeof(sTmpFirstBuffer));				

				if( pStartPos == pBuffer )
				{
					if( nReadBufferLen - (pDataEndPoint - pBuffer) <= 0 )	break;

					memcpy(sTmpFirstBuffer, pDataEndPoint + 1, nReadBufferLen - (pDataEndPoint - pBuffer));
				}
				else
				{
					if( pDataEndPoint == (pStartPos + nReadBufferLen - 1) )
					{
						memcpy(sTmpFirstBuffer, pStartPos, pBuffer - pStartPos);
					}
					else
					{
						char sTmpSecondBuffer[1024];
						memset(sTmpSecondBuffer, 0x00, sizeof(sTmpSecondBuffer));
						memcpy(sTmpSecondBuffer, pDataEndPoint + 1, nReadBufferLen - (pDataEndPoint - pStartPos));
						
						memcpy(sTmpFirstBuffer, pStartPos, pBuffer - pStartPos);
						strcat(sTmpFirstBuffer, sTmpSecondBuffer);
					}
				}

				memset(sReadBuffer, 0x00, sizeof(sReadBuffer));
				memcpy(sReadBuffer, sTmpFirstBuffer, sizeof(sReadBuffer));
				nReadBufferLen = strlen(sReadBuffer);
			}
		}
	}
	else if( sDitc[0] == 'K' )	// ��ũ��Ʈ��
	{
		// �Ӱ�ġ ���� �迭��ŭ �ݺ�
		for(int j = 0; j < GetThStationCount(); j++)
		{
			ThSta_t* pThSta = (ThSta_t*)m_aryThStation.GetAt(j);
			if( pThSta == NULL ) continue;

			char sSen_Id[16];
			char* pReSearchStartPos = NULL;

			while (strlen(sReadBuffer) > 0)
			{
				if(strlen(sReadBuffer) <= 0) break;
				memset(sSen_Id, 0x00, sizeof(sSen_Id));
				memset(sValueKind, 0x00, sizeof(sValueKind));
				nReadBufferLen = strlen(sReadBuffer);

				pStartPos = sReadBuffer;

				// SensorID ���� ������ �� ���� �����͸� sWriteBuffer ���� ã��
				char* pBuffer = NULL;
				
				if( pReSearchStartPos != NULL )
				{
					pBuffer = strstr(pReSearchStartPos, pThSta->sSen_Id);
				}
				else
				{
					pBuffer = strstr(pStartPos, pThSta->sSen_Id);
				}
				
				if( pBuffer == NULL ) break;

				char* pDataEndPoint = strstr(pBuffer, "\n");
				if( pDataEndPoint == NULL ) break;

				// �ʼ��� ������ ���� ���� �ش��ϴ� ������ �Ӱ�ġ�� �� (�� ��� �Ӱ谪�� ������ '���� �Ұ�')
				char* pValueKind = strstr(pBuffer, "\t");
				if( pValueKind == NULL ) continue;
				memset(sValueKind, 0x00, sizeof(sValueKind));
				strncpy(sValueKind, pValueKind + 1, 1);

				// ��������ġ�� Type �� ������ �����͸� ���� ó��
				if( sValueKind[0] != pThSta->sType[0] )
				{
					pReSearchStartPos = pDataEndPoint + 1;

					if( pReSearchStartPos == NULL )
						break;

					continue;
				}

				char* pCalcValue = strstr(pValueKind + 1, "\t");
				if( pCalcValue == NULL ) continue;

				char* pEndCalcValuePos = strstr(pCalcValue, "\n");
				if( pEndCalcValuePos == NULL ) break;

				if( pValueKind - pBuffer > 0 )
				{
					int nSenIdSize = sizeof(sSen_Id) > pValueKind - pBuffer ? pValueKind - pBuffer : sizeof(sSen_Id);
					strncpy(sSen_Id, pBuffer, nSenIdSize);
				}

				int nCopySize = sizeof(sCalcValue) > pEndCalcValuePos - pCalcValue ? pEndCalcValuePos - pCalcValue : sizeof(sCalcValue);
				memset(sCalcValue, 0x00, sizeof(sCalcValue));
				strncpy(sCalcValue, pCalcValue, nCopySize);	

				double dblCalcValue = atof(sCalcValue);
				ThSta_Detail_t* pThStaDetail = (ThSta_Detail_t*)pThSta->pAryThSta_Detail->GetAt(0);

				if (sValueKind[0] == 'A')
				{
					nAccCheckCnt++;
				}
				else if (sValueKind[0] == 'C')
				{
					nCavCheckCnt++;
				}

				char sResultMsg[16];
				memset(sResultMsg, 0x00, sizeof(sResultMsg));

				if( pThStaDetail == NULL )
				{
					sprintf(sResultMsg, "%s\n", "�� �Ұ�");

					if (sValueKind[0] == 'A')
					{
						nAcc_FailCnt++;
					}
					else if (sValueKind[0] == 'C')
					{
						nCav_FailCnt++;
					}
				}
				else
				{
					double dblThValue = atof(pThStaDetail->sMaxVal);

					if( dblCalcValue > dblThValue )	// �Ӱ�ġ �ʰ�
					{
						sprintf(sResultMsg, "%s\n", "���� �ʿ�");
						nResultFaultCnt++;
						nResultAvailCnt++;

						if (sValueKind[0] == 'A')
						{
							nAcc_WarnCnt++;
						}
						else if (sValueKind[0] == 'C')
						{
							nCav_WarnCnt++;
						}
					}
					else
					{
						sprintf(sResultMsg, "%s\n", "����");
						nResultAvailCnt++;
					}
				}

				// Result ���Ͽ� �ش� ������ ���� ���� ����
				memset(sWriteBuffer, 0x00, sizeof(sWriteBuffer));

				if( strlen(sSen_Id) > 0 )
				{
					sprintf(sWriteBuffer, "%s\t%s\t%s", sSen_Id, sValueKind, sResultMsg);
				}
				else
				{
					sprintf(sWriteBuffer, "%s\t%s\t%s", pThSta->sSen_Id, pThSta->sType, sResultMsg);
				}
				fprintf(Result_file, "%s", sWriteBuffer);

				// ó���� �ʼ��� ������ �����͸� sWriteBuffer �������� ����
				pDataEndPoint = strstr(pBuffer, "\n");
				if( pDataEndPoint == NULL)
				{
					break;				
				}
				else
				{
					char sTmpFirstBuffer[4096];
					memset(sTmpFirstBuffer, 0x00, sizeof(sTmpFirstBuffer));

					if( pStartPos == pBuffer )
					{
						if( nReadBufferLen - (pDataEndPoint - pBuffer) <= 0 )	break;

						memcpy(sTmpFirstBuffer, pDataEndPoint + 1, nReadBufferLen - (pDataEndPoint - pBuffer));
					}
					else
					{
						if( pDataEndPoint == (pStartPos + nReadBufferLen - 1) )
						{
							memcpy(sTmpFirstBuffer, pStartPos, pBuffer - pStartPos);
						}
						else
						{
							char sTmpSecondBuffer[1024];
							memset(sTmpSecondBuffer, 0x00, sizeof(sTmpSecondBuffer));
							memcpy(sTmpSecondBuffer, pDataEndPoint + 1, nReadBufferLen - (pDataEndPoint - pStartPos));

							memcpy(sTmpFirstBuffer, pStartPos, pBuffer - pStartPos);
							strcat(sTmpFirstBuffer, sTmpSecondBuffer);
						}
					}
					
					memset(sReadBuffer, 0x00, sizeof(sReadBuffer));
					memcpy(sReadBuffer, sTmpFirstBuffer, sizeof(sReadBuffer));
					nReadBufferLen = strlen(sReadBuffer);

					pReSearchStartPos = NULL;
				}
			}
		}
	}
	else if( sDitc[0] == 'D' )	// �ʴ� �� ������ �� ����Ŀ� ���� ����� �ջ������� �������� ���� �ջ������� ���Ͽ� ��� �ʴ� �� ������ �ü����� ���� �����ϰ� ������ ��
	{
		int nBefReadBuffer = 0;

		while( strlen(sReadBuffer) > 0 )
		{
			if( nBefReadBuffer == strlen(sReadBuffer) ) break;
			if( strlen(sReadBuffer) <= 0 ) break;
			nBefReadBuffer = strlen(sReadBuffer);
			nReadBufferLen = nBefReadBuffer;

			char* pStartPos = sReadBuffer;
			char* pValueKind = strstr(sReadBuffer, "\t");
			char* pDataEndPos = strstr(sReadBuffer, "\n");

			char sResultMsg[128];
			memset(sResultMsg, 0x00, sizeof(sResultMsg));

			char sSen_Id[16];
			memset(sSen_Id, 0x00, sizeof(sSen_Id));

			BOOL bDataSetYN = FALSE;

			if( pValueKind != NULL && pDataEndPos != NULL && pValueKind > pStartPos  )
			{
				int nCopySize = sizeof(sSen_Id) > pValueKind - pStartPos ? pValueKind - pStartPos : sizeof(sSen_Id);
				strncpy(sSen_Id, pStartPos, nCopySize);

				memset(sValueKind, 0x00, sizeof(sValueKind));
				strncpy(sValueKind, pValueKind + 1, 1);

				if( sValueKind[0] == 'S' )
				{
					nStmCheckCnt++;

					char* pSettlementPos = strstr(pValueKind + 1, "\t");
					if( pSettlementPos != NULL )
					{
						char sSettlement[16];
						memset(sSettlement, 0x00, sizeof(sSettlement));					

						nCopySize = sizeof(sSettlement) > pDataEndPos - pSettlementPos ? pDataEndPos - pSettlementPos : sizeof(sSettlement);
						strncpy(sSettlement, pSettlementPos + 1, nCopySize);

						double dblSettlement = atof(sSettlement);
						//dblSettlement = dblSettlement * 0.010;

						if( dblSettlement >= 0)
						{
							// 2019.11.21 �ջ����� 0.4 �� �����ϴ� gal ���� 143 gal �̹Ƿ� ���� gal ������ ���ϴ� ������ ����� 0.400 => 143 ���� �����Ͽ� ���
							if( dblSettlement > 0.400 )	// ���� �ʿ�
							{
								sprintf(sResultMsg, "%s\t%s\t%s\n", sSen_Id, sValueKind, "���� �ʿ�");
								nResultFaultCnt++;
								nResultAvailCnt++;

								nStm_WarnCnt++;
							}
							else if( dblSettlement <= 0.400 )
							{
								sprintf(sResultMsg, "%s\t%s\t%s\n", sSen_Id, sValueKind, "����");
								nResultAvailCnt++;
							}

							bDataSetYN = TRUE;
						}					
					}
				}			
			}

			if( bDataSetYN == FALSE )
			{
				if( strlen(sSen_Id) <= 0)
				{
					sprintf(sResultMsg, "%s\t%s\t%s\n", "Unknown", "Unknown", "�� �Ұ�");
					nStm_FailCnt++;
				}
				else
				{
					sprintf(sResultMsg, "%s\t%s\t%s\n", sSen_Id, sValueKind, "�� �Ұ�");
					nStm_FailCnt++;
				}
			}
			fprintf(Result_file, "%s", sResultMsg);

			// ó���� �ʼ��� ������ �����͸� sWriteBuffer �������� ����
			if( pDataEndPos == NULL)
			{
				break;				
			}
			else
			{
				char sTmpFirstBuffer[4096];
				memset(sTmpFirstBuffer, 0x00, sizeof(sTmpFirstBuffer));

				if( nReadBufferLen - (pDataEndPos - pStartPos) <= 0 )	break;
				memcpy(sTmpFirstBuffer, pDataEndPos + 1, nReadBufferLen - (pDataEndPos - pStartPos));

				memset(sReadBuffer, 0x00, sizeof(sReadBuffer));
				memcpy(sReadBuffer, sTmpFirstBuffer, sizeof(sReadBuffer));
				nReadBufferLen = strlen(sReadBuffer);
			}
		}
	}

	// �׸� 1�� ���� ��� ó��(acc_val, dis_val, cav_val, stm_val)
	char sValResultMsg[32];
	char sAccValMsg[64];
	char sSpValMsg[64];
	char sDisValMsg[64];
	char sCavValMsg[64];
	char sStmValMsg[64];
	memset(sValResultMsg, 0x00, sizeof(sValResultMsg));
	memset(sAccValMsg, 0x00, sizeof(sAccValMsg));
	memset(sSpValMsg, 0x00, sizeof(sSpValMsg));
	memset(sDisValMsg, 0x00, sizeof(sDisValMsg));
	memset(sCavValMsg, 0x00, sizeof(sCavValMsg));
	memset(sStmValMsg, 0x00, sizeof(sStmValMsg));

	if (sDitc[0] == 'S' || sDitc[0] == 'B')	// Ư������
	{
		// ���ӵ�
		if (nAccCheckCnt > 0)
		{
			if (nAcc_WarnCnt > 0)
			{
				if (nAcc_FailCnt > nAcc_WarnCnt)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "�����ʿ�");
				}
			}
			else
			{
				if (nAcc_FailCnt > 0)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "����");
				}
			}
		}
		else
		{
			strcpy(sValResultMsg, "�򰡺Ұ�");
		}

		m_strAcc_Val.Format("%s", sValResultMsg);
		sprintf(sAccValMsg, "acc_val=%s\n", sValResultMsg);
		fprintf(Result_file, "%s", sAccValMsg);

		// ����
		if (nDisCheckCnt > 0)
		{
			if (nDis_WarnCnt > 0)
			{
				if (nDis_FailCnt > nDis_WarnCnt)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "�����ʿ�");
				}
			}
			else
			{
				if (nDis_FailCnt > 0)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "����");
				}
			}
		}
		else
		{
			//strcpy(sValResultMsg, "�򰡺Ұ�");
			strcpy(sValResultMsg, "����");
		}
		
		m_strDis_Val.Format("%s", sValResultMsg);
		sprintf(sDisValMsg, "dis_val=%s\n", sValResultMsg);
		fprintf(Result_file, "%s", sDisValMsg);

		// ���ӵ� ���佺��Ʈ��
		if (nSpCheckCnt > 0)
		{
			if (nSp_WarnCnt > 0)
			{
				if (nSp_FailCnt > nSp_WarnCnt)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "�����ʿ�");
				}
			}
			else
			{
				if (nSp_FailCnt > 0)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "����");
				}
			}
		}
		else
		{
			strcpy(sValResultMsg, "�򰡺Ұ�");
		}

		m_strSp_Val.Format("%s", sValResultMsg);
		sprintf(sSpValMsg, "sp_val=%s\n", sValResultMsg);
		fprintf(Result_file, "%s", sSpValMsg);

		// ���� ���� �׸��� �� ����
		sprintf(sCavValMsg, "cav_val=\n");
		fprintf(Result_file, "%s", sCavValMsg);
		sprintf(sStmValMsg, "stm_val=\n");
		fprintf(Result_file, "%s", sStmValMsg);
	}
	else if (sDitc[0] == 'K')	// ��ũ��Ʈ��
	{
		// ���ӵ�
		if (nAccCheckCnt > 0)
		{
			if (nAcc_WarnCnt > 0)
			{
				if (nAcc_FailCnt > nAcc_WarnCnt)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "�����ʿ�");
				}
			}
			else
			{
				if (nAcc_FailCnt > 0)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "����");
				}
			}
		}
		else
		{
			strcpy(sValResultMsg, "�򰡺Ұ�");
		}

		m_strAcc_Val.Format("%s", sValResultMsg);
		sprintf(sAccValMsg, "acc_val=%s\n", sValResultMsg);
		fprintf(Result_file, "%s", sAccValMsg);

		// CAV
		if (nCavCheckCnt > 0)
		{
			if (nCav_WarnCnt > 0)
			{
				if (nCav_FailCnt > nCav_WarnCnt)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "�����ʿ�");
				}
			}
			else
			{
				if (nCav_FailCnt > 0)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "����");
				}
			}
		}
		else
		{
			strcpy(sValResultMsg, "�򰡺Ұ�");
		}
		
		m_strCav_Val.Format("%s", sValResultMsg);
		sprintf(sCavValMsg, "cav_val=%s\n", sValResultMsg);
		fprintf(Result_file, "%s", sCavValMsg);

		// ���� ���� �׸��� �� ����
		sprintf(sSpValMsg, "sp_val=\n");
		fprintf(Result_file, "%s", sSpValMsg);
		sprintf(sDisValMsg, "dis_val=\n");
		fprintf(Result_file, "%s", sDisValMsg);
		sprintf(sStmValMsg, "stm_val=\n");
		fprintf(Result_file, "%s", sStmValMsg);
	}
	else if (sDitc[0] == 'D')	// �ʴ�(������)
	{
		// STM
		if (nStmCheckCnt > 0)
		{
			if (nStm_WarnCnt > 0)
			{
				if (nStm_FailCnt > nStm_WarnCnt)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "�����ʿ�");
				}
			}
			else
			{
				if (nStm_FailCnt > 0)
				{
					strcpy(sValResultMsg, "�򰡺Ұ�");
				}
				else
				{
					strcpy(sValResultMsg, "����");
				}
			}
		}
		else
		{
			strcpy(sValResultMsg, "�򰡺Ұ�");
		}

		m_strStm_Val.Format("%s", sValResultMsg);
		sprintf(sStmValMsg, "stm_val=%s\n", sValResultMsg);
		fprintf(Result_file, "%s", sStmValMsg);

		// ���� ���� �׸��� �� ����
		sprintf(sAccValMsg, "acc_val=\n");
		fprintf(Result_file, "%s", sAccValMsg);
		sprintf(sDisValMsg, "dis_val=\n");
		fprintf(Result_file, "%s", sDisValMsg);
		sprintf(sCavValMsg, "cav_val=\n");
		fprintf(Result_file, "%s", sCavValMsg);
		sprintf(sSpValMsg, "sp_val=\n");
		fprintf(Result_file, "%s", sSpValMsg);
	}

	// total ó��
	char sTotalMsg[32];
	memset(sTotalMsg, 0x00, sizeof(sTotalMsg));

	if( nResultFaultCnt == 0 )	// ���� �ʿ� ������ 0
	{
		if( nResultAvailCnt < m_nMinAvailChanCnt )	// ������ �򰡰� ���� ����� ������ ������ �������� ���� ��� '�ý��� ���� �ʿ�'
		{
			m_strTot_Result = "�ý��� ���� �ʿ�";
		}
		else
		{
			m_strTot_Result = "����";			
		}

		sprintf(sTotalMsg, "total=%s\n", m_strTot_Result);
	}
	else
	{
		if( sDitc[0] == 'S' || sDitc[0] == 'B' )	// Ư������
		{
			//// ��ü ��� ���� ��� ���� �ʿ� ���� %�� �Ǵ��� ���
			//double dblResult = nResultFaultCnt / GetThStationCount();
			//if( dblResult > 0.5 )	// ���� �ʿ� ������ 50% �ʰ��� ���
			//{
			//	sprintf(sTotalMsg, "total=%s\n", "���� �ʿ�");
			//}
			//else
			//{
			//	sprintf(sTotalMsg, "total=%s\n", "����");
			//}

			// ���� �ʿ� ���������� �Ǵ��� ���
			if( nResultAvailCnt < m_nMinAvailChanCnt )	// ������ �򰡰� ���� ����� ������ ������ �������� ���� ��� '�ý��� ���� �ʿ�'
			{
				m_strTot_Result = "�ý��� ���� �ʿ�";
			}
			else
			{
				if( nResultFaultCnt >= m_nWarningLimit )
				{
					m_strTot_Result = "�����ʿ�";
				}
				else
				{
					m_strTot_Result = "����";
				}
			}

			sprintf(sTotalMsg, "total=%s\n", m_strTot_Result);
		}
		else	// ��ũ��Ʈ��, �ʴ� �� �������� �Ǵ� �ڷᰡ 1�� �̹Ƿ� ���� �ʿ䰡 1 �̻��̸� ������ ���� �ʿ�
		{
			if( nResultAvailCnt < m_nMinAvailChanCnt )	// ������ �򰡰� ���� ����� ������ ������ �������� ���� ��� '�ý��� ���� �ʿ�'
			{
				m_strTot_Result = "�ý��� ���� �ʿ�";
			}
			else
			{
				m_strTot_Result = "�����ʿ�";
			}

			sprintf(sTotalMsg, "total=%s\n", m_strTot_Result);
		}
	}
	
	fprintf(Result_file, "%s", sTotalMsg);

	fclose(calcResult_file);
	fclose(Result_file);

	return TRUE;
}

void CMakeEvalutionBaseData::SetDefault_Thresholds(char* sDitc)
{
	// 2019.07.15 ��ũ��Ʈ�︸ �켱 ����(��Ÿ �ü����� ��� ����)
	// 2019.08.12 Ư������ �߰� ����
	if ( sDitc[0] != 'K' && sDitc[0] != 'S' && sDitc[0] != 'B' )
		return;

	ThSta_t* pThSta = NULL;
	ThSta_t thSta;

	for ( int i = 0; i < GetStationCount(); i++ )
	{
		Sta_t* pSta = GetStation(i);

		if ( pSta == NULL ) continue;

		pThSta = NULL;

		memset(&thSta, 0x00, sizeof(ThSta_t));
		memcpy(thSta.sSen_Id, pSta->sSen_Id, sizeof(thSta.sSen_Id));
		
		if ( sDitc[0] == 'K' )	// ��ũ��Ʈ��
		{
			ThSta_Detail_t thStaDetail;
			
			// ���ӵ� �⺻ ��������ġ ����
			strcpy(thSta.sType, "A");

			pThSta = AddThStation(&thSta);
			
			memset(&thStaDetail, 0x00, sizeof(ThSta_Detail_t));

			// 2020.02.19 0.154g -> 0.2g �� default ��������ġ ����
			strcpy(thStaDetail.sMinVal, "196");	// 0.154 g * 981
			strcpy(thStaDetail.sMaxVal, "196");	// 0.154 g * 981
			//strcpy(thStaDetail.sMinVal, "151");	// 0.154 g * 981
			//strcpy(thStaDetail.sMaxVal, "151");	// 0.154 g * 981

			if (pThSta)
			{
				AddThStationDetail(pThSta, &thStaDetail);
				pThSta->bDataAvailability = true;
			}
			
			// CAV �⺻ ��������ġ ����
			strcpy(thSta.sType, "C");

			pThSta = AddThStation(&thSta);

			memset(&thStaDetail, 0x00, sizeof(ThSta_Detail_t));

			// 2020.02.19 0.154g -> 0.2g �� default ��������ġ ����Ǿ� CAV default ��������ġ�� ����
			strcpy(thStaDetail.sMinVal, "0.167");
			strcpy(thStaDetail.sMaxVal, "0.167");
			//strcpy(thStaDetail.sMinVal, "0.129");
			//strcpy(thStaDetail.sMaxVal, "0.129");

			if (pThSta)
			{
				AddThStationDetail(pThSta, &thStaDetail);
				pThSta->bDataAvailability = true;
			}
		}
		else if ( sDitc[0] == 'S' || sDitc[0] == 'B' )	// Ư������
		{
			if ( thSta.sSen_Id[5] != 'B' && thSta.sSen_Id[5] != 'G' && thSta.sSen_Id[5] != 'Q' )
			{
				continue;
			}

			ThSta_Detail_t thStaDetail;

			// ���ӵ� �⺻ ��������ġ ����
			for ( int j = 0; j < 3; j++ )		// �� 3���� �̹Ƿ� ������ 3���� ó��
			{
				memcpy(thSta.sSen_Id, pSta->sSen_Id, sizeof(thSta.sSen_Id));

				strcpy(thSta.sType, "A");

				if ( thSta.sSen_Id[5] == 'B' )	// ��ž����
				{
#ifdef _DEBUG
					//strcpy(thSta.sSen_Id, "KC_WDB");
#endif	// DEBUG
					switch ( j )
					{
					case 0:
						strcat(thSta.sSen_Id, "_HBX");
						break;
					case 1:
						strcat(thSta.sSen_Id, "_HBY");
						break;
					case 2:
						strcat(thSta.sSen_Id, "_HBZ");
						break;
					}					
				}
				else if ( thSta.sSen_Id[5] == 'G' )	// ������
				{
#ifdef _DEBUG
					//strcpy(thSta.sSen_Id, "KC_WDG");
#endif	// DEBUG
					switch ( j )
					{
					case 0:
						strcat(thSta.sSen_Id, "_HGE");
						break;
					case 1:
						strcat(thSta.sSen_Id, "_HGN");
						break;
					case 2:
						strcat(thSta.sSen_Id, "_HGZ");
						break;
					}
				}
				else if ( thSta.sSen_Id[5] == 'Q' )	// ��Ŀ����
				{
#ifdef _DEBUG
					//strcpy(thSta.sSen_Id, "KC_WDQ");
#endif	// DEBUG
					switch ( j )
					{
					case 0:
						strcat(thSta.sSen_Id, "_HBX");
						break;
					case 1:
						strcat(thSta.sSen_Id, "_HBY");
						break;
					case 2:
						strcat(thSta.sSen_Id, "_HBZ");
						break;
					}
				}

				pThSta = AddThStation(&thSta);

				memset(&thStaDetail, 0x00, sizeof(ThSta_Detail_t));

				if( thSta.sSen_Id[9] == 'Z' )
				{
					strcpy(thStaDetail.sMinVal, "52");
					strcpy(thStaDetail.sMaxVal, "52");
				}
				else
				{
					strcpy(thStaDetail.sMinVal, "67");
					strcpy(thStaDetail.sMaxVal, "67");
				}
				
				strcpy(thStaDetail.sDampingRatio, "0.05");
				strcpy(thStaDetail.sGravity, "1");
				strcpy(thStaDetail.sCoefficient, "2.8");
				//strcpy(thStaDetail.sEffectivePGA, "0.154");
				//strcpy(thStaDetail.sEffectivePGA, "0.068");	// 2019.10.10 ���ӵ� �⺻ ��������ġ�� 67gal -> g ȯ�갪���� �ݿ�(�þȰ� �ڼ���)
				strcpy(thStaDetail.sEffectivePGA, thStaDetail.sMaxVal);	// 2019.11.26 ���� ������ ������ ��û���� ����

				if (pThSta)
				{
					AddThStationDetail(pThSta, &thStaDetail);
					pThSta->bDataAvailability = true;
				}
			}
		}
	}
}

void CMakeEvalutionBaseData::SpectrumCompute(GroundAccType* earth, double damping, SpectrumType* spectrum)
{
	double* Tab = new double[9];
	int Npoints, Ifreq, It, Ii, Li;
	double One = 1.0, Two = 2.0, Zero = 0.0;  //Fact = gravitiy acceleration, g
	double Freqin, Freqmin, Freqmax, Damp, Dfreq, dt, T = 0;
	double Vxsi, Freq, Den, Vratio, Freqe, Xsiwt, Etau, Sinwt, Coswt, Txsi, Xsif, Freqi;
	double Qmax, Qpmax, Qppmax, Qabs, Qpabs, Qppabs;
	int Int = spectrum->nperiod;
	//int Nacc = 1000;
	int Nacc = earth->gcount;
	double Frac = One / (Int - 1);

	double A[3][3];
	double B[3][3];

	// we will use 1 based array instead of 0 based array
	double* Q = new double[earth->gcount + 1];
	double* Qp = new double[earth->gcount + 1];
	double* Qpp = new double[earth->gcount + 1];
	double* Fr = new double[spectrum->nperiod + 1];
	double* Qm = new double[spectrum->nperiod + 1];
	double* Qpm = new double[spectrum->nperiod + 1];
	double* Qppm = new double[spectrum->nperiod + 1];

	//** Read ACC array length by line counting
	Npoints = earth->gcount;
	//**  Generate Data For Frequency Ranges Of 0.2-30Hz
	//    Freqmin Must Be Greater Than Zero
	Freqmin = 0.2;
	Freqmax = 30;
	//
	//  Initial damping ratio assigned as 5% instead of 0%        
	//  
	Damp = damping;
	dt = earth->dt;
	Dfreq = Freqmax - Freqmin;
	if (Damp > One)
		TRACE("This Program is written for underdamped cases only\n");
	//
	//  Loop over the each frequency (Hz) between Freqmin and Freqmax
	//     Number of frequencies are set as 500 in this program
	//             Int = 500;
	//
	for (Ifreq = 1; Ifreq < Int + 1; Ifreq++)
	{
		Freqin = Freqmax - Frac * Dfreq * (Ifreq - 1);
		Vxsi = Damp;
		Freq = Freqin * Two * PI;
		Den = sqrt(One - Vxsi * Vxsi);
		Vratio = One / Den;
		Freqe = Den * Freq;
		Xsiwt = Vxsi * Freq * dt;
		Etau = exp(-Xsiwt);
		Sinwt = sin(Freqe * dt);
		Coswt = cos(Freqe * dt);
		A[1][1] = Etau * (Vxsi * Vratio * Sinwt + Coswt);
		A[1][2] = Etau * Sinwt / Freqe;
		A[2][1] = -Etau * Freq * Vratio * Sinwt;
		A[2][2] = Etau * (Coswt - Vxsi * Vratio * Sinwt);
		//
		Txsi = (Two * Vxsi * Vxsi - One) / Freq / Freq / Freqe / dt;
		Xsif = Vxsi / Freq / Freqe;
		Freqi = One / Freq / Freq;
		//
		B[1][1] = Etau * (-(Xsif + Txsi) * Sinwt -
				(Freqi + Two * Vxsi * Freqi / Freq / dt) * Coswt) +
				Two * Vxsi * Freqi / Freq / dt;
		B[1][2] = Etau * (Txsi * Sinwt + Two * Vxsi * Freqi / Freq / dt * Coswt) +
					Freqi - Two * Vxsi * Freqi / Freq / dt;
		B[2][1] = Etau * (-(Freqe * Coswt - Vxsi * Freq * Sinwt) * (Txsi + Xsif) +
					(Freqe * Sinwt + Vxsi * Freq * Coswt) * (Freqi + Two * Vxsi * Freqi /
					Freq / dt)) - Freqi / dt;
		B[2][2] = Etau * ((Freqe * Coswt - Vxsi * Freq * Sinwt) * Txsi -
					(Freqe * Sinwt + Vxsi * Freq * Coswt) * Two * Vxsi * Freqi /
					Freq / dt) + Freqi / dt;
		//
		//  Integrate using Average Acceleration Method
		//
		for (It = 1; It < Nacc + 1; It++)
		{
			if (It == 1)
			{
				//               ** Initial Conditions
				T = 0.0;
				Q[1] = 0.0;
				Qp[1] = 0.0;
				Qpp[1] = 0.0;
			}
			else
			{
				T = T + dt;
				Q[It] = A[1][1] * Q[It - 1] + A[1][2] * Qp[It - 1] + B[1][1] * earth->acc[It - 1]
						+ B[1][2] * earth->acc[It];
				Qp[It] = A[2][1] * Q[It - 1] + A[2][2] * Qp[It - 1] + B[2][1] * earth->acc[It - 1] +
							B[2][2] * earth->acc[It];
				//               Absolute Acceleration Record
				Qpp[It] = -Qp[It] * Two * Vxsi * Freq - Q[It] * Freq * Freq;
			}
		}
		//
		//  Find absolute maxium values in displacement, velocity, acceleration
		//
		Qmax = 0.0;
		Qpmax = 0.0;
		Qppmax = 0.0;
		for (Ii = 1; Ii < Nacc + 1; Ii++)
		{
			Qabs = fabs(Q[Ii]);
			Qpabs = fabs(Qp[Ii]);
			Qppabs = fabs(Qpp[Ii]);
			Qmax = max(Qmax, Qabs);
			Qpmax = max(Qpmax, Qpabs);
			Qppmax = max(Qppmax, Qppabs);
		}
		//
		//  Maximum values (spectra values) at each frequency store into arrays
		//
		Qm[Ifreq] = Qmax;
		Qpm[Ifreq] = Qpmax;
		Qppm[Ifreq] = Qppmax;
		Fr[Ifreq] = Freqin;
	}
	//
	// Store all spectrum into spectrum class
	//
	for (Li = 1; Li < Int + 1; Li++)
	{
		spectrum->period[Li] = 1/Fr[Li];
		spectrum->dispSpectrum[Li] = Qm[Li];
		spectrum->velSpectrum[Li] = Qpm[Li];
		spectrum->accSpectrum[Li] = Qppm[Li];
	}

	if (Tab)	delete Tab;
	if (Q)		delete Q;
	if (Qp)		delete Qp;
	if (Qpp)	delete Qpp;
	if (Fr)		delete Fr;
	if (Qm)	delete Qm;
	if (Qpm)	delete Qpm;
	if (Qppm)	delete Qppm;
}

void CMakeEvalutionBaseData::DesignSpectrum(double s1, double alpha, SpectrumType* spectrum)
{
	double T,beta1, beta2, gamma;
	double T1 = 0.06, T2 = 0.3, T3 = 3;
	for (int i = 1; i < spectrum->nperiod + 1; i++)
	{
		T = spectrum->period[i];
		beta1 = 1 / T2;
		beta2 = 1 / (T3 * T3);
		gamma = alpha * s1 / beta1 * (1 / T3);
		if (T <= T1)
			// linear increasing region
			spectrum->designSpectrum[i] = (alpha * s1 - s1) / T1 * T + s1;
		else if ((T1 < T) & (T <= T2))
			// constant regin
			spectrum->designSpectrum[i] = alpha * s1;
		else if ((T2 < T) & (T <= T3))
			// decreasing by 1/T 
			spectrum->designSpectrum[i] = alpha * s1 / beta1 * (1 / T);
		else if (T3 < T)
			// decreasing by 1/T^2
			spectrum->designSpectrum[i] = gamma / beta2 * (1 / (T * T));
	}
}

bool CMakeEvalutionBaseData::CompareSpectrum(SpectrumType* spectrum)
{
	bool info;

	info = false;
	for (int i = 1; i < spectrum->nperiod + 1; i++)
	{
		if (spectrum->accSpectrum[i] > spectrum->designSpectrum[i])
			// earthquake spectrum exceeds design spetrum
		{
			info = true;
			return info;
		}
	}
	return info;
}

void CMakeEvalutionBaseData::SendTransLog(char* sMessage)
{
	if( sMessage == NULL) return;
	if( strlen(sMessage) <= 0 ) return;

	CString sMsg = "";
	sMsg.Format("%s", sMessage);

	char *pMsg = new char[2048];
	memset(pMsg, 0x00, 2048);
	strcpy(pMsg, sMessage);
	::PostMessage(m_pMainDlgHwnd, UM_SHOWTRANSLOG, (WPARAM)pMsg, 0);
}
