#pragma once

//#include <direct.h>
//#include <io.h>
//#include <afxconv.h>
#include <vector>

#include "../Include/fft_common.h"
#include "../Lib/Ary.h"
#include "../include/libmseed.h"
#include "../include/define.h"
#include "../include/LogFunc.h"
#include "codedef.h"
//#include "filt.h"
#include "SpectrumType.h"
#include "GroundAccType.h"

using namespace std;

#define PI 3.14159



struct Sta_t 
{
	char sSen_Id[8];
	CAry* pArySta_Detail;
	bool bDataAvailability;
};

struct Sta_Detail_t 
{
	char sZresponse[32];
	char sNresponse[32];
	char sEresponse[32];
	char sZsensitivity[32];
	char sNsensitivity[32];
	char sEsensitivity[32];
};

struct ThSta_t 
{
	char sSen_Id[16];
	char sType[4];
	CAry* pAryThSta_Detail;
	bool bDataAvailability;
};

struct ThSta_Detail_t 
{
	char sMinVal[32];
	char sMaxVal[32];
	char sDampingRatio[8];
	char sGravity[8];
	char sCoefficient[8];
	char sEffectivePGA[32];
};

#define STATIONINFO_TXT				"stationinfo.txt"
#define THRESHOLDSINFO_D_TXT		"typeD_ThInfo.txt"	// 필댐 및 저수지 관리 기준치 파일
#define THRESHOLDSINFO_B_TXT		"typeB_ThInfo.txt"	// 교량-현수교 관리 기준치 파일
#define THRESHOLDSINFO_S_TXT		"typeS_ThInfo.txt"	// 교량-사장교 관리 기준치 파일
#define THRESHOLDSINFO_K_TXT		"typeK_ThInfo.txt"	// 콘크리트댐 관리 기준치 파일
#define CALC_RESULT						"calc_result.txt"		// 안전성 평가를 위해 각 알고리즘에 의해 계산된 결과 저장 파일
#define BUILDEVA_RESULT				"result.txt"			// 안전성 평가 결과 저장 파일
#define SAMP_N							1000
#define PATH_RESULT						"RESULT"

#define TESTDATA	0
#define TEST_D		0	// 필댐(저수지) 테스트
#define TEST_K		0	// 콘크리트댐 테스트
#define TEST_SB		0	// 특수교량 테스트

class CMakeEvalutionBaseData
{
public:
	CMakeEvalutionBaseData(void);
	virtual ~CMakeEvalutionBaseData(void);	

	HWND	m_pMainDlgHwnd;
	char	g_szAstr[32][25600];
	CAry	m_aryStation;
	CAry	m_aryThStation;
	CString m_strDitc;
	CString m_sEventID;
	CString m_sExecProcKind;

	int		m_nWarningLimit;
	int		m_nMinAvailChanCnt;

	CIniFunc*		iniFunc;
	CLogFunc*	logFunc;

	CString		m_strAcc_Val;
	CString		m_strSp_Val;
	CString		m_strDis_Val;
	CString		m_strCav_Val;
	CString		m_strStm_Val;
	CString		m_strTot_Result;

	CString		m_strRemoveAnalDataYN;

	//CListBox		m_listTransLog;
	//int				m_nListAEMaxWidth;

	int		MakeBaseDataProc(char* sDitc, char* sEventID, char* sMseedPath, char* sNet, char* sSta);	// 차후 개별 모듈화시 main 함수 역할
	//void	testBassPassFilter();
	void	testBandPassFilter(double fps, int dataCount, double lowFreq, double highFreq, int FilterOrder);//
	void	BandPassFilter(double* pDataArray, double fps, int iCount, double dblLowFreq, double dblHighFreq, int nFiltOrd, char* outFileFullPath = NULL);

	int		ReadStationInfo2Array(CString sFilePath);	// stationinfo.txt 읽어서 배열에 저장
	int		ReadThresholdsInfo2Array(CString sFilePath, CString sDitc);	// type{시설물구분코드}_thInfo.txt 읽어서 배열에 저장
	int		ReadEtcConfig(CString sFilePath);	// etc_config.txt 읽어서 변수에 저장

	// Station 정보 관리
	Sta_t*	AddStation(Sta_t* sta);
	int		DelStation(Sta_t* sta);
	Sta_t*	GetStation(int idx);
	Sta_t*	GetStation(char* sSen_Id);
	int		GetStationCount();

	// Station 상세 정보 관리(response, sensitivity 값)
	Sta_Detail_t*	AddStationDetail(Sta_t* pSta, Sta_Detail_t* pDetail);
	int				DelAllStationDetail(Sta_t* pSta);

	// ThresholdsStation 정보 관리
	ThSta_t*	AddThStation(ThSta_t* sta);
	int			DelThStation(ThSta_t* sta);
	ThSta_t*	GetThStation(int idx);
	ThSta_t*	GetThStation(char* sSen_Id);
	ThSta_t*	GetThStation(char* sSen_Id, char* sType);
	int			GetThStationCount();
	CAry*		GetAryThStation();

	// ThresholdsStation 상세 정보 관리(minValue, maxValue 값)
	ThSta_Detail_t*	AddThStationDetail(ThSta_t* pSta, ThSta_Detail_t* pDetail);
	int					DelAllThStationDetail(ThSta_t* pSta);

	// 2019.01.07 생성된 구간mseed 자료를 txt 로 변환
	void			MakeMseed2Txt(char* desc, char* szNet, char* szSta);
	int				CheckMseedFile_Samprate(char* desc, char* szNet, char* szSta);

	//void			ShowTransLog(CString strLogMsg, int iWriteFile = 0 );
	void			WriteReportLog(CString strLog);

	int				CreatePathDirectory(char* szTargetDir);
	int				CreatePathDirectory(CString strTargetDir);
	void			DeleteAllFile( char* szDir, char* sNet, char* sSta );
	void			DeleteCalcData( char* szDir, char* sNet, char* sSta);

	//void			mseed2txt(char* szOrgFile, double fCalib, char* szMakeFile);// int argc, char **argv );
	//void			mseed2txt(char* szOrgFile, char* fCalib, char* szMakeFile);// int argc, char **argv );
	int				mseed2txt(char* szOrgFile, double fCalib);//, char* szMakeFile);// int argc, char **argv );
	int				mseed2txt(char* szOrgFile, char* fCalib);//, char* szMakeFile);// int argc, char **argv );
	char			*H_StrDate(time_t ptTime, char *pcDate, int piApplyGMT);

	int				ReadChanData(char* szInFile, char cDType);
	int				ParsingToken(char* szBuff);

	//int				ReadMseedData2ChanStruct(char* szInFile, char cDType);	// mSEED 구간 자료를 읽어 구조체에 할당
	//																						// 0점 보정 (mseed2txt 함수)와 유사

	vector<double> ComputeDenCoeffs(int FilterOrder, double Lcutoff, double Ucutoff);
	vector<double> TrinomialMultiply(int FilterOrder, vector<double> b, vector<double> c);
	vector<double> ComputeNumCoeffs(int FilterOrder, double Lcutoff, double Ucutoff, vector<double> DenC);
	vector<double> ComputeLP(int FilterOrder);
	vector<double> ComputeHP(int FilterOrder);
	//double* filter(double* x, int xCount, vector<double> coeff_b, vector<double> coeff_a);
	void filter(double* x, double* y, int xCount, vector<double> coeff_b, vector<double> coeff_a);

	// 2019.04.11 시설물별 안전성평가 필수 축성분별 데이터 유무 확인
	int			CheckMseedFile_Exists(char* sDesc,  char* sNet, char* sSta, char* sDitc);
	int			IsExistMseedFile(char* sType, char* sDesc,  char* sNet, char* sSta, char* sDitc);

	// 2019.05.21 CAV 계산식 함수로 분리 및 안전성 평가 결과 저장 함수 추가
	double	Integral_Acc(SampleStruct_t* data, int nDataCount);
	int			WriteCalcResult(char* sDitc, char* sObsId, char* sNet, char* sSta, char* sMseedPath, double& dblMaxPGA, double& dblMaxPGV, double& dblMaxDisp, double& dblSettlement, double& dblCAV, int nResSpectrum);	// 안전성 평가를 위한 시설물별 알고리즘에 의한 계산 결과
	BOOL		WriteResult(char* sDitc, char* sNet, char* sSta, char* sEventID, char* sMseedPath);																										// 안전성 평가 결과 저장

	// 2019.07.12 관리기준치 기본값이 있는 시설물의 경우, 기본값으로 설정하는 함수 추가
	void		SetDefault_Thresholds(char* sDitc);

	// 2019.08.09 응답스펙트럽 계산 알고리즘
	void		SpectrumCompute(GroundAccType* earth, double damping, SpectrumType* spectrum);
	void		DesignSpectrum(double s1, double alpha, SpectrumType* spectrum);
	bool		CompareSpectrum(SpectrumType* spectrum);

	// 2019.10.11 윈도우 버전용 다이알로그 메세지 전송 함수
	void		SendTransLog(char* sMessage);
};

