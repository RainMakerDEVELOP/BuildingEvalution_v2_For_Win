
// BuildingEvalutionDlg.cpp : 구현 파일
//
#include "stdafx.h"
#include "BuildingEvalution.h"
#include "BuildingEvalutionDlg.h"
#include "afxdialogex.h"

//#include "../MySql/include/MySql.h"
#include <my_global.h>
#include <mysql.h>
#include <iostream>

using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <strsafe.h>
#pragma comment(lib, "libmysql.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma pack(push,8)

#include "tlhelp32.h"

#pragma pack(pop)

//typedef std::basic_string<TCHAR> tstring;
#define		CONFIG_INI				_T("../Config/config.ini")
#define		SIGNALPROCFG_INI		_T("../Config/SignalProConfig.ini")
#define		ERROR_FILENAME	_T("ERROR_BUILDINGEVA.log")
#define		TRACE_FILENAME	_T("TRACE_BUILDINGEVA.log")
#define		TMR_DISTROY_DIALOG				23		// 안전성평가 프로세스 셀프 종료
#ifdef _DEBUG
#define		DELAY_MAX_CNT					13		// 5분 * 5초마다 타이머 = 60회
#else
#define		DELAY_MAX_CNT					70		// 5분 * 5초마다 타이머 = 60회
#endif	// DEBUG

// 건물정보 향후 추가 수정 가능
// db column명과 일치시킨다
enum observatory_info
{
	//ROOF_HT		= 0,		// DB 중복 컬럼 제거로 인하여 명칭 변경 (150907 : shkim)
	GROUND_HT	= 0,
	STR_CD		= 1,
	SEIS_CD		= 2,
	EQ_EREA1	= 3,
	EQ_EREA2	= 4,
	BUILD_FLOOR	= 5,
	OBS_NAME	= 6,
	ADDRESS		= 7,
	CHARGE		= 8,
	CONTACT		= 9,
};

enum Query_Type
{
	Insert = 0,
	Update = 1,
};

enum dbEv_Type
{
	res_ok = 0,
	res_error = 1,
	check_error = 2,
	ini_error = 3,
};
// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()

//	virtual void OnCancel();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

DWORD WINAPI BuildlingEvalutionProc( LPVOID lpParam )
{
	CBuildingEvalutionDlg* pBldEvaDlg;
	pBldEvaDlg = (CBuildingEvalutionDlg*)lpParam;
	pBldEvaDlg->m_MakeEvaBD.m_pMainDlgHwnd = pBldEvaDlg->m_hWnd;

	CString strTransLog = "";
	strTransLog.Format("Ditc = '%s', EventID = '%s', MseedBaseDir = '%s', Net = '%s', Sta = '%s', RemoveAnalDataYN = '%s', ProcKind = '%s'", pBldEvaDlg->m_strDitc, pBldEvaDlg->m_sEventID, pBldEvaDlg->m_sMseedBaseDir, pBldEvaDlg->m_strNet, pBldEvaDlg->m_strSta, pBldEvaDlg->m_strRemoveAnalDataYN, pBldEvaDlg->m_sExecProcKind);
	pBldEvaDlg->ShowTransLog(strTransLog);

	//pBldEvaDlg->pIniFunc = NULL;
	//pBldEvaDlg->pLogFunc = NULL;
	//pBldEvaDlg->pIniFunc = new CIniFunc;
	//pBldEvaDlg->pLogFunc = new CLogFunc;

	if (pBldEvaDlg->m_sExecProcKind == "KV")
	{
		pBldEvaDlg->pIniFunc->ReadConfig();
		pBldEvaDlg->GetUserDeptInfo();
	}

	pBldEvaDlg->m_MakeEvaBD.m_strRemoveAnalDataYN = pBldEvaDlg->m_strRemoveAnalDataYN;
	pBldEvaDlg->m_MakeEvaBD.iniFunc = pBldEvaDlg->pIniFunc;
	pBldEvaDlg->m_MakeEvaBD.logFunc = pBldEvaDlg->pLogFunc;

	if (pBldEvaDlg->m_sExecProcKind == "KV")
	{
		pBldEvaDlg->iniIniFile();
		pBldEvaDlg->setDitc(pBldEvaDlg->m_strNet, pBldEvaDlg->m_strSta, pBldEvaDlg->m_strDitc);
	}
	else
	{
		pBldEvaDlg->pIniFunc->m_iLogLevel = 1;

		TCHAR path[2048];
		GetModuleFileName(NULL, path, sizeof(path));

		CString strPath = path;
		int iFind = strPath.ReverseFind('\\');
		strPath = strPath.Left(iFind);
		strPath += "\\Log";

		pBldEvaDlg->pIniFunc->m_strLogDataDefDir = strPath;
	}

//#ifdef _DEBUG
//	if (pBldEvaDlg->m_sExecProcKind == "KV")
//	{
//		pBldEvaDlg->MakeStationInfoTxt(pBldEvaDlg->m_strNet, pBldEvaDlg->m_strSta);	// 2019.04.17 실제 Release 모드에서는 BuildingEvalution_v2 실행 이전에, 해당 경로에 Station Calibration 정보 파일이 생성된 상태로 구동되야 한다.
//	}
//	//m_strDitc = "D";
//#endif	// DEBUG

	CString strInfoFilePath = "";

	if (pBldEvaDlg->m_sExecProcKind == "KV")
	{
		strInfoFilePath = pBldEvaDlg->m_sMseedBaseDir + "\\" + pBldEvaDlg->m_sEventID;
	}
	else
	{
		strInfoFilePath = pBldEvaDlg->m_sMseedBaseDir;
	}

	strTransLog.Format("InfoFilePath = '%s'", strInfoFilePath);
	pBldEvaDlg->ShowTransLog(strTransLog);

	//char* sStationInfoFileFullPath = strStationInfoFileFullPath.GetBuffer(strStationInfoFileFullPath.GetLength());
	//int nReadRet = ReadStationInfo2Array(strStationInfoFileFullPath);
	char* pDesc = NULL;
	char* pNet = NULL;
	char* pSta = NULL;
	int nRet = 0;

	char sDesc[2048];
	char sNet[8];
	char sSta[4];
	char sDitc[4];
	char sEventID[32];
	char sExecProcKind[32];
	memset(sDesc, 0x00, sizeof(sDesc));
	memset(sNet, 0x00, sizeof(sNet));
	memset(sSta, 0x00, sizeof(sSta));
	memset(sDitc, 0x00, sizeof(sDitc));
	memset(sEventID, 0x00, sizeof(sEventID));
	memset(sExecProcKind, 0x00, sizeof(sExecProcKind));

	if (pBldEvaDlg->m_sExecProcKind == "KV")
	{
		sprintf(sDesc, "%s%s%s", pBldEvaDlg->m_sMseedBaseDir, "\\", pBldEvaDlg->m_sEventID);
	}
	else
	{
		sprintf(sDesc, "%s", pBldEvaDlg->m_sMseedBaseDir);
	}	
	sprintf(sNet, "%s", pBldEvaDlg->m_strNet);
	sprintf(sSta, "%s", pBldEvaDlg->m_strSta);
	sprintf(sDitc, "%s", pBldEvaDlg->m_strDitc);
	sprintf(sEventID, "%s", pBldEvaDlg->m_sEventID);

	memset(pBldEvaDlg->m_sMseedFullPath, 0x00, sizeof(pBldEvaDlg->m_sMseedFullPath));
	memcpy(pBldEvaDlg->m_sMseedFullPath, sDesc, sizeof(pBldEvaDlg->m_sMseedFullPath));

	if (pBldEvaDlg->m_sExecProcKind.GetLength() > 0)
	{
		sprintf(sExecProcKind, "%s", pBldEvaDlg->m_sExecProcKind);
	}

	pBldEvaDlg->m_MakeEvaBD.m_sEventID = pBldEvaDlg->m_sEventID;
	pBldEvaDlg->m_MakeEvaBD.m_sExecProcKind = pBldEvaDlg->m_sExecProcKind;

	CString strLog = "";
	strLog.Format("Desc = '%s', Net = '%s', Sta = '%s', ditc = '%s', eventId = '%s', execProcKind = '%s'", sDesc, sNet, sSta, sDitc, sEventID, sExecProcKind);
	pBldEvaDlg->WriteReportLog(strLog);

	strTransLog.Format("MseedFullPath = '%s'", sDesc);
	pBldEvaDlg->ShowTransLog(strTransLog);

	//	m_MakeEvaBD.MakeMseed2Txt(sDesc, sNet, sSta);

	pBldEvaDlg->m_MakeEvaBD.m_strDitc = pBldEvaDlg->m_strDitc;

	// 기타 설정 파일 읽기 Start
	int nReadEtcCfgRet = pBldEvaDlg->m_MakeEvaBD.ReadEtcConfig(strInfoFilePath);

	if( pBldEvaDlg->m_strDitc == "K" || pBldEvaDlg->m_strDitc == "D" )	// 콘크리트댐, 필댐 및 저수지는 안전성 종합 평가의 점검 필요 허용 갯수를 무조건 0 으로 설정
	{
		pBldEvaDlg->m_MakeEvaBD.m_nWarningLimit = 1;
		pBldEvaDlg->m_MakeEvaBD.m_nMinAvailChanCnt = 1;
	}
	else	// 안전성 평가 항목이 여러 성분인 특수교량만 우선 적용
	{
		if (nReadEtcCfgRet == -1)
		{
			pBldEvaDlg->m_MakeEvaBD.m_nWarningLimit = 1;
			pBldEvaDlg->m_MakeEvaBD.m_nMinAvailChanCnt = 3;
		}
	}

	strTransLog.Format("ReadEtcConfig = '%d', WarningLimit = '%d', MinAvailChanCnt = '%d'", nReadEtcCfgRet, pBldEvaDlg->m_MakeEvaBD.m_nWarningLimit, pBldEvaDlg->m_MakeEvaBD.m_nMinAvailChanCnt);
	pBldEvaDlg->ShowTransLog(strTransLog);
	// 기타 설정 파일 읽기 End

	// 안전성 평가를 위한 필수축 성분의 관리기준치 파일이 있는지 체크
	int nReadThInfoRet = 0;

	if (pBldEvaDlg->m_strDitc == "K")	// 콘크리트댐
	{
		nReadThInfoRet = pBldEvaDlg->m_MakeEvaBD.ReadThresholdsInfo2Array(strInfoFilePath, pBldEvaDlg->m_strDitc);
	}
	else if (pBldEvaDlg->m_strDitc == "D")	// 필댐 및 저수지
	{
		//nReadThInfoRet = m_MakeEvaBD.ReadThresholdsInfo2Array(strInfoFilePath, m_strDitc);
	}
	else if (pBldEvaDlg->m_strDitc == "S" || pBldEvaDlg->m_strDitc == "B")	// 교량-현수교, 교량-사장교
	{
		nReadThInfoRet = pBldEvaDlg->m_MakeEvaBD.ReadThresholdsInfo2Array(strInfoFilePath, pBldEvaDlg->m_strDitc);
		//CDialogEx::OnCancel();
		//return TRUE;
	}
	else
	{
		nReadThInfoRet = -1;
	}

	strLog = "";
	CString strErrMsg = "";

	switch(nReadThInfoRet)
	{
	case -1:	// 시설물 구분을 알 수 없음
		// 안전성평가 결과 파일에 오류사항 작성 후 종료		
		strLog.Format("Read Threshold Info Failed!! Unknown Ditc '%d'", nReadThInfoRet);
		pBldEvaDlg->WriteReportLog(strLog);
		strErrMsg = "관리 기준치를 알 수 없음";
		pBldEvaDlg->WriteLog(ERROR_FILENAME, strErrMsg);

		strTransLog.Format("관리 기준치를 알 수 없음[%d]", nReadThInfoRet);
		pBldEvaDlg->ShowTransLog(strTransLog);

		// 2019.07.12 콘크리트댐의 경우, 관리기준치 파일이 없을 경우, 이후 stationinfo 파일 체크 후 기본값으로 설정되도록 함
		// 2019.08.12 특수교량 추가
		if ( pBldEvaDlg->m_strDitc != "K" && pBldEvaDlg->m_strDitc != "S" && pBldEvaDlg->m_strDitc != "B" )
		{
			//CDialogEx::OnCancel();
			return FALSE;
		}
		break;
	case -2:	// 관리기준치 파일이 없음
		// 안전성평가 결과 파일에 오류사항 작성 후 종료
		strLog.Format("Read Threshold Info Failed!! Not found threshold file '%d'", nReadThInfoRet);
		pBldEvaDlg->WriteReportLog(strLog);
		strErrMsg = "관리기준치 파일이 없음";
		pBldEvaDlg->WriteLog(ERROR_FILENAME, strErrMsg);

		strTransLog.Format("관리기준치 파일이 없음[%d]", nReadThInfoRet);
		pBldEvaDlg->ShowTransLog(strTransLog);

		// 2019.07.12 콘크리트댐의 경우, 관리기준치 파일이 없을 경우, 이후 stationinfo 파일 체크 후 기본값으로 설정되도록 함
		if ( pBldEvaDlg->m_strDitc != "K" && pBldEvaDlg->m_strDitc != "S" && pBldEvaDlg->m_strDitc != "B" )
		{
			//CDialogEx::OnCancel();
			return FALSE;
		}
		break;
	}

	// 필수축 성분의 mSEED 구간 데이터가 인자로 넘어온 경로에 존재하는지 체크
	int nMseedExistRet = pBldEvaDlg->m_MakeEvaBD.CheckMseedFile_Exists(sDesc, sNet, sSta, sDitc);	

	if ( nMseedExistRet == 0 )	// 성공
	{
		strTransLog.Format("실행 인자의 경로에 필수축 성분의 mSEED 구간 데이터가 존재[%d]", nMseedExistRet);
		pBldEvaDlg->ShowTransLog(strTransLog);

		// mSEED 구간 데이터가 정상 데이터인지 체크(파일 열기 가능한지, 파싱 가능한 유효 데이터인지, 중간에 누락된 샘플이 없는지)
		int nMseedSampCheckRet = pBldEvaDlg->m_MakeEvaBD.CheckMseedFile_Samprate(sDesc, sNet, sSta);

		if ( nMseedSampCheckRet == 0 )
		{
			// 2019.07.12 콘크리트댐인데, 관리기준치 파일 처리에서 에러가 난 경우, 기본값으로 설정
			// 2019.08.12 특수교량 추가 적용
			if ( pBldEvaDlg->m_strDitc == "K" || pBldEvaDlg->m_strDitc == "S" || pBldEvaDlg->m_strDitc == "B" )
			{
				if ( nReadThInfoRet == -1 || nReadThInfoRet == -2 )
				{
					pBldEvaDlg->m_MakeEvaBD.SetDefault_Thresholds(sDitc);

					strTransLog.Format("관리기준치 파일 문제로 해당 시설 관리기준치는 기본값으로 설정[%s]", pBldEvaDlg->m_strDitc);
					pBldEvaDlg->ShowTransLog(strTransLog);
				}
			}
			int nBuildingEvalutionRet = pBldEvaDlg->m_MakeEvaBD.MakeBaseDataProc(sDitc, sEventID, sDesc, sNet, sSta);	// 안전성 평가 수행 프로세스

			if ( nBuildingEvalutionRet == 1 )		// 안전성 평가 처리 완료
			{
				strLog = "";
				strLog.Format("=========Building Evalution2 Process Succeed");
				pBldEvaDlg->WriteReportLog(strLog);
				pBldEvaDlg->WriteLog(TRACE_FILENAME, pBldEvaDlg->m_sEventID + "=========안전성평가 처리완료");
				strTransLog.Format("=========안전성평가 처리완료 : [%s]", pBldEvaDlg->m_sEventID);
				pBldEvaDlg->ShowTransLog(strTransLog);

				// 가통인 경우 DB에 결과 저장
				if (pBldEvaDlg->m_sExecProcKind == "KV")
				{
					CString strTime = pBldEvaDlg->GetDBSysTime();

					if (pBldEvaDlg->Insert_BuildingEvalutionResult_Detail(res_ok, sNet, sSta, "", strTime) == FALSE)
					{
						strLog = "";
						strLog.Format("Building Evalution2 Detail DB Insert Error");
						pBldEvaDlg->WriteReportLog(strLog);
						pBldEvaDlg->WriteLog(ERROR_FILENAME, "안전성 평가2 상세 DB 저장 실패");

						strTransLog.Format("안전성 평가2 상세 DB 저장 실패");
						pBldEvaDlg->ShowTransLog(strTransLog);

						if (pBldEvaDlg->Insert_BuildingEvalutionResult(res_error, "", strTime) == FALSE)
						{
							strLog = "";
							strLog.Format("Building Evalution2 Result Failed DB Update Error. Ditc = '%s' EventID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
							pBldEvaDlg->WriteReportLog(strLog);
							strErrMsg.Format("안전성 평가2 실패 결과 DB 저장 실패. 시설물 종류 = '%s', 이벤트ID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
							pBldEvaDlg->WriteLog(TRACE_FILENAME, strErrMsg);

							strTransLog.Format("안전성 평가2 실패 결과 DB 저장 실패. 시설물 종류 = '%s', 이벤트ID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
							pBldEvaDlg->ShowTransLog(strTransLog);
						}
						else
						{
							strLog = "";
							strLog.Format("Building Evalution2 Result Failed DB Insert Succeed");
							pBldEvaDlg->WriteReportLog(strLog);
							pBldEvaDlg->WriteLog(ERROR_FILENAME, "안전성 평가2 종합 실패 DB 저장 성공");

							strTransLog.Format("안전성 평가2 종합 실패 DB 저장 성공");
							pBldEvaDlg->ShowTransLog(strTransLog);
						}	
					}
					else
					{
						if (pBldEvaDlg->Insert_BuildingEvalutionResult(res_ok, "", strTime) == FALSE)
						{
							strLog = "";
							strLog.Format("Building Evalution2 Result DB Update Error. Ditc = '%s' EventID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
							pBldEvaDlg->WriteReportLog(strLog);
							strErrMsg.Format("안전성 평가2 성공 결과 DB 저장 실패. 시설물 종류 = '%s', 이벤트ID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
							pBldEvaDlg->WriteLog(TRACE_FILENAME, strErrMsg);

							strTransLog.Format("안전성 평가2 성공 결과 DB 저장 실패. 시설물 종류 = '%s', 이벤트ID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
							pBldEvaDlg->ShowTransLog(strTransLog);
						}
						else
						{
							strLog = "";
							strLog.Format("Building Evalution2 Result DB Insert Succeed");
							pBldEvaDlg->WriteReportLog(strLog);
							strErrMsg.Format("안전성 평가2 종합 DB 저장 성공");
							pBldEvaDlg->WriteLog(ERROR_FILENAME, strErrMsg);

							strTransLog.Format("안전성 평가2 종합 DB 저장 성공");
							pBldEvaDlg->ShowTransLog(strTransLog);
						}
					}
				}

				pBldEvaDlg->MakeReportFile05_v2();		// 댐,저수지 및 교량용 안전성 평가 보고서 작성	
			}
			else	// 안전성 평가 처리 실패
			{
				strLog = "";
				strLog.Format("=========Building Evalution2 Process Failed");
				pBldEvaDlg->WriteReportLog(strLog);
				pBldEvaDlg->WriteLog(TRACE_FILENAME, pBldEvaDlg->m_sEventID + "=========안전성평가 처리실패");

				strTransLog.Format("=========안전성평가 처리실패 : [%s]", pBldEvaDlg->m_sEventID);
				pBldEvaDlg->ShowTransLog(strTransLog);

				//WritePrivateProfileString("result", "total", "평가 계산 실패", m_sMseedDir + "\\analysisSample.ini");
				//WritePrivateProfileString("result", "time", sTime, m_sMseedDir + "\\analysisSample.ini");

				// 가통인 경우 DB에 결과 저장
				if (pBldEvaDlg->m_sExecProcKind == "KV")
				{
					if (pBldEvaDlg->Insert_BuildingEvalutionResult(res_error) == FALSE)
					{
						strLog = "";
						strLog.Format("Building Evalution2 Error Result DB Insert Error");
						pBldEvaDlg->WriteReportLog(strLog);
						strErrMsg.Format("안전성 평가2 실패 결과 DB 저장 실패.");
						pBldEvaDlg->WriteLog(ERROR_FILENAME, strErrMsg);

						strTransLog.Format("안전성 평가2 실패 결과 DB 저장 실패.");
						pBldEvaDlg->ShowTransLog(strTransLog);
					}
					else
					{
						strLog = "";
						strLog.Format("Building Evalution2 Error Result DB Update Succeed. Ditc = '%s' EventID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
						pBldEvaDlg->WriteReportLog(strLog);
						strErrMsg.Format("안전성 평가2 실패 결과 DB 저장 성공. 시설물 구분 = '%s', 이벤트ID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
						pBldEvaDlg->WriteLog(TRACE_FILENAME, strLog);

						strTransLog.Format("안전성 평가2 실패 결과 DB 저장 성공. 시설물 구분 = '%s', 이벤트ID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
						pBldEvaDlg->ShowTransLog(strTransLog);
					}
				}
			}
		}
		else	// 실패
		{
			strLog = "";
			strErrMsg = "";		// DB 저장용 메세지

			// 안전성 평가 결과 파일에 해당 실패 사유 명시
			switch( nMseedSampCheckRet )
			{
			case -1:	// 필수축 성분 데이터 없음				
				strLog.Format("MiniSEED samprate check failed!! No ingredient data required '%d'", nMseedSampCheckRet);
				strErrMsg = "필수축 성분 데이터 없음";
				break;
			case -2:	// StationInfo.txt 파일이 없음
				strLog.Format("MiniSEED samprate check failed!! Not found station info file '%d'", nMseedSampCheckRet);
				strErrMsg = "StationInfo.txt 파일이 없음";
				break;
			case -3:	// mSEED 구간 데이터 파일열기 실패
				strLog.Format("MiniSEED samprate check failed!! Failed to open mSEED file '%d'", nMseedSampCheckRet);
				strErrMsg = "mSEED 구간 데이터 파일열기 실패";
				break;
			case -4:	// 인식할 수 없는 타입의 mSEED 구간 데이터
				strLog.Format("MiniSEED samprate check failed!! Unrecognized types of mSEED file '%d'", nMseedSampCheckRet);
				strErrMsg = "인식할 수 없는 타입의 mSEED 구간 데이터";
				break;
			case -5:	// mSEED 구간에 누락된 데이터 존재
				strLog.Format("MiniSEED samprate check failed!! Missing data in mSEED file '%d'", nMseedSampCheckRet);
				strErrMsg = "mSEED 구간에 누락된 데이터 존재";
				break;
			}

			// 가통인 경우 DB에 결과 저장
			if (pBldEvaDlg->m_sExecProcKind == "KV")
			{
				if (pBldEvaDlg->Insert_BuildingEvalutionResult(res_error, strErrMsg) == FALSE)
				{
					strLog.Append(" : Building Evalution2 Error Result DB Insert Error");
					pBldEvaDlg->WriteReportLog(strLog);
					strErrMsg.Append(" : 안전성 평가2 실패 결과 DB 저장 실패");
					pBldEvaDlg->WriteLog(ERROR_FILENAME, strErrMsg);

					pBldEvaDlg->ShowTransLog(strErrMsg);
				}
				else
				{
					strLog.AppendFormat(" : Building Evalution2 Error Result DB Update Succeed. Ditc = '%s' EventID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
					pBldEvaDlg->WriteReportLog(strLog);
					strErrMsg.Format(" : 안전성 평가2 실패 결과 DB 저장 성공. 시설물 구분 = '%s', 이벤트ID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
					pBldEvaDlg->WriteLog(TRACE_FILENAME, strErrMsg);

					pBldEvaDlg->ShowTransLog(strErrMsg);
				}
			}
			else
			{
				if( strLog.GetLength() > 0 )
				{
					pBldEvaDlg->WriteReportLog(strLog);
					pBldEvaDlg->ShowTransLog(strLog);
				}

				if( strErrMsg.GetLength() > 0 )
				{
					pBldEvaDlg->WriteLog(TRACE_FILENAME, strErrMsg);
					pBldEvaDlg->ShowTransLog(strErrMsg);
				}
			}
		}
	}
	else	// 실패
	{
		// 안전성 평가에 필요한 필수축 성분 mSEED 구간 데이터 파일이 없음
		strLog = "";
		strLog.Format("Exist MiniSEED file check failed!! Not found mSEED data file '%d'", nMseedExistRet);
		strErrMsg = "안전성 평가에 필요한 필수축 성분 mSEED 구간 데이터 파일이 없음";

		// 가통인 경우 DB에 결과 저장
		if (pBldEvaDlg->m_sExecProcKind == "KV")
		{
			if (pBldEvaDlg->Insert_BuildingEvalutionResult(res_error, strErrMsg) == FALSE)
			{
				strLog.Append(" : Building Evalution2 Error Result DB Insert Error");
				pBldEvaDlg->WriteReportLog(strLog);
				strErrMsg.Append(" : 안전성 평가2 실패 결과 DB 저장 실패");
				pBldEvaDlg->WriteLog(ERROR_FILENAME, strErrMsg);

				pBldEvaDlg->ShowTransLog(strErrMsg);
			}
			else
			{
				strLog.AppendFormat(" : Building Evalution2 Error Result DB Update Succeed. Ditc = '%s' EventID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
				pBldEvaDlg->WriteReportLog(strLog);
				strErrMsg.Format(" : 안전성 평가2 실패 결과 DB 저장 성공. 시설물 구분 = '%s', 이벤트ID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
				pBldEvaDlg->WriteLog(TRACE_FILENAME, strErrMsg);

				pBldEvaDlg->ShowTransLog(strErrMsg);
			}
		}
		else
		{
			strErrMsg.AppendFormat(". : 시설물 구분 = '%s', 이벤트ID = '%s'", sDitc, pBldEvaDlg->m_sEventID);
			pBldEvaDlg->WriteReportLog(strLog);
			pBldEvaDlg->WriteLog(TRACE_FILENAME, strErrMsg);
			pBldEvaDlg->ShowTransLog(strErrMsg);
		}
	}

	SetTimer(pBldEvaDlg->m_hWnd, TMR_BUILDINGEVALUTION_END, 500, NULL);
}

// CBuildingEvalutionDlg 대화 상자
CBuildingEvalutionDlg::CBuildingEvalutionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBuildingEvalutionDlg::IDD, pParent)
{
	// 프로세스 실행을 위한 인자값 순서
	// 1 : 시설물 구분 코드
	// 2 : 이벤트ID
	// 3 : 안전성평가를 위한 mSEED(100sps) 구간데이터 경로
	// 4 : Network ID
	// 5 : Station Code
	// 6 : 구간데이터 변환 산출 데이터 삭제 여부(Y : 삭제, N : 유지)
	// 7 : 가통 통합 소프트웨어인지 구분 (가통 소프트웨어인 경우 'KV')

	// * 예외 : 가통 배포판에서는 1, 3 인자값은 DB에 가지고 있기 때문에, 실행시 넘어온 인자값을 사용하지는 않지만,
	//			 아무값이나 설정해야함(가통 배포판 외 프로세스에서의 구동과 일원화하기 위함)

	m_hCreateBldEvaThread = NULL;
	m_dwCreateBldEvaThreadID = 0;

	m_nListAEMaxWidth = 0;

	m_strDitc = "";
	m_sMseedBaseDir = "";

	m_strAcc_Val = "";
	m_strDis_Val = "";
	m_strCav_Val = "";
	m_strStm_Val = "";
	m_strTot_Result = "";

	m_sExecProcKind = "";
	m_strRemoveAnalDataYN = "Y";	

#ifdef	DEF_MULTI_SITE_SUPPORT
	strCmdLine = AfxGetApp()->m_lpCmdLine;
	char	szNet[10];
	char	szObsID[10];
	CString	strTmp;
	int		inx;

	memset(szNet, 0x00, sizeof(szNet));
	memset(szObsID, 0x00, sizeof(szObsID));

	// 1번째 인자값(안전성평가 대상 시설물 종류 D : 필댐 및 저수지, B : 교량(현수교), S : 교량(사장교), K : 콘크리트댐)
	inx = strCmdLine.Find(' ');
	m_strDitc = strCmdLine.Mid(0, inx);

	// 2번째 인자값(안전성평가를 위한 EventID)
	strCmdLine = strCmdLine.Mid(inx+1);
	inx = strCmdLine.Find(' ');
	m_sEventID = strCmdLine.Mid(0, inx);

	// 3번째 인자값(안전성평가를 위한 mSEED 구간자료 경로)
	strCmdLine = strCmdLine.Mid(inx+1);
	inx = strCmdLine.Find(' ');
	m_sMseedBaseDir = strCmdLine.Mid(0, inx);	

	// 4번째 인자값(Network ID)
	strCmdLine = strCmdLine.Mid(inx+1);
	inx = strCmdLine.Find(' ');
	strTmp  = strCmdLine.Mid(0, inx);
	strTmp.Trim();

#ifdef _DEBUG
	//strTmp = "AC";
#endif	// DEBUG

	if(strTmp.GetLength() >= 2)
	{
		szNet[0] = strTmp.GetAt(0);
		szNet[1] = strTmp.GetAt(1);
	}

	// 5번째 인자값(Station Code)
	strCmdLine = strCmdLine.Mid(inx+1);
	inx = strCmdLine.Find(' ');

	if(inx > 0)
	{
		strTmp  = strCmdLine.Mid(0, inx);
	}
	else
	{
		strTmp = strCmdLine;
	}
	strTmp.Trim();

#ifdef _DEBUG
	//strTmp = "TJ";
#endif	// DEBUG

	if(strTmp.GetLength() >= 2)
	{
		szObsID[0] = strTmp.GetAt(0);
		szObsID[1] = strTmp.GetAt(1);
	}

	if(strlen(szNet) >= 2 && strlen(szObsID) >= 2)
	{
		setNetSta(szNet, szObsID);	
	}

	// 6번째 인자값(구간데이터 변환 산출 데이터 삭제 여부)
	if (inx > 0)
	{
		strCmdLine = strCmdLine.Mid(inx+1);
		inx = strCmdLine.Find(' ');

		if (inx > 0)
		{
			strTmp = strCmdLine.Mid(0, inx);
			strTmp.Trim();

			if(strTmp.GetLength() > 0)
			{
				m_strRemoveAnalDataYN = strTmp;
			}
		}
		else
		{
			if (strCmdLine.GetLength() > 0)
			{
				m_strRemoveAnalDataYN = strCmdLine;
			}
		}
	}
	
	// 7번째 인자값(실행 프로세스 코드)
	if(inx > 0)
	{
		strTmp = strCmdLine.Mid(inx+1);
		strTmp.Trim();

		if(strTmp.GetLength() > 0)
		{
			m_sExecProcKind = strTmp;
		}
	}

	//CString strMsg;
	//strMsg.Format(strMsg, "[%s] {%s] [%s]", m_sEventID, szNet, szObsID);
	//AfxMessageBox(strMsg);

	// m_sEventID = AfxGetApp()->m_lpCmdLine; // ==> CApp Class로 이동

	pIniFunc = NULL;
	pLogFunc = NULL;
	pIniFunc = new CIniFunc;
	pLogFunc = new CLogFunc("BuildingEvalution_v2", m_sExecProcKind);

#else
	// m_sEventID = AfxGetApp()->m_lpCmdLine; ==> CApp Class로 이동

#endif
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CBuildingEvalutionDlg::~CBuildingEvalutionDlg()
{
	if( pIniFunc )
	{
		delete pIniFunc;
		pIniFunc = NULL;
	}

	if( pLogFunc )
	{
		delete pLogFunc;
		pLogFunc = NULL;
	}

	if( m_hCreateBldEvaThread != NULL )
	{
		CloseHandle(m_hCreateBldEvaThread);
		m_hCreateBldEvaThread = NULL;
	}
}

void CBuildingEvalutionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRANS_LOG_LIST, m_listTransLog);
}

BEGIN_MESSAGE_MAP(CBuildingEvalutionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_WINDOWPOSCHANGING()
	ON_BN_CLICKED(IDC_BUTTON1, &CBuildingEvalutionDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CBuildingEvalutionDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDCANCEL, &CBuildingEvalutionDlg::OnBnClickedCancel)
	ON_WM_DESTROY()
	ON_MESSAGE(UM_SHOWTRANSLOG, &CBuildingEvalutionDlg::OnUmShowtranslog)
END_MESSAGE_MAP()


// CBuildingEvalutionDlg 메시지 처리기

void CBuildingEvalutionDlg::StartBuildingEvalution()
{
	if( m_hCreateBldEvaThread != NULL )
	{
		CloseHandle(m_hCreateBldEvaThread);
		m_hCreateBldEvaThread = NULL;
	}

	// Event Data Create Thread 생성 및 구동
	m_hCreateBldEvaThread = CreateThread(NULL									// default security attributes
													, 0										// use default stack size  
													, BuildlingEvalutionProc			// thread function 
													, this									// argument to thread function 
													, 0										// use default creation flags 
													, &m_dwCreateBldEvaThreadID);	// returns the thread identifier 

	if( m_hCreateBldEvaThread == NULL )
	{
		CDialogEx::OnCancel();
	}	
}

BOOL CBuildingEvalutionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.
	//AfxMessageBox("test");

	PostMessage(WM_SHOWWINDOW, TRUE, SW_OTHERUNZOOM);

	SetTimer(TMR_BUILDINGEVALUTION_START, 1000, NULL);

	return TRUE;

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	//pIniFunc = NULL;
	//pLogFunc = NULL;
	//pIniFunc = new CIniFunc;
	//pLogFunc = new CLogFunc;

	if(m_sExecProcKind == "KV")
	{
		pIniFunc->ReadConfig();
		GetUserDeptInfo();
	}

	m_MakeEvaBD.m_strRemoveAnalDataYN = m_strRemoveAnalDataYN;
	m_MakeEvaBD.iniFunc = pIniFunc;
	m_MakeEvaBD.logFunc = pLogFunc;

	if(m_sExecProcKind == "KV")
	{
		iniIniFile();
		setDitc(m_strNet, m_strSta, m_strDitc);
	}
	else
	{
		pIniFunc->m_iLogLevel = 1;

		TCHAR path[2048];
		GetModuleFileName(NULL, path, sizeof(path));

		CString strPath = path;
		int iFind = strPath.ReverseFind('\\');
		strPath = strPath.Left(iFind);
		strPath += "\\Log";

		pIniFunc->m_strLogDataDefDir = strPath;
	}

//#ifdef _DEBUG
//	if (m_sExecProcKind == "KV")
//	{
//		MakeStationInfoTxt(m_strNet, m_strSta);	// 2019.04.17 실제 Release 모드에서는 BuildingEvalution_v2 실행 이전에, 해당 경로에 Station Calibration 정보 파일이 생성된 상태로 구동되야 한다.
//	}
//	//m_strDitc = "D";
//#endif	// DEBUG

	CString strInfoFilePath = "";
	//char* sStationInfoFileFullPath = strStationInfoFileFullPath.GetBuffer(strStationInfoFileFullPath.GetLength());
	//int nReadRet = ReadStationInfo2Array(strStationInfoFileFullPath);
	char* pDesc = NULL;
	char* pNet = NULL;
	char* pSta = NULL;
	int nRet = 0;

	char sDesc[2048];
	char sNet[8];
	char sSta[4];
	char sDitc[4];
	char sEventID[32];
	char sExecProcKind[32];
	memset(sDesc, 0x00, sizeof(sDesc));
	memset(sNet, 0x00, sizeof(sNet));
	memset(sSta, 0x00, sizeof(sSta));
	memset(sDitc, 0x00, sizeof(sDitc));
	memset(sEventID, 0x00, sizeof(sEventID));
	memset(sExecProcKind, 0x00, sizeof(sExecProcKind));

	if (m_sExecProcKind == "KV")
	{
		strInfoFilePath = m_sMseedBaseDir + "\\" + m_sEventID;
		sprintf(sDesc, "%s%s%s", m_sMseedBaseDir, "\\", m_sEventID);
	}
	else
	{
		strInfoFilePath = m_sMseedBaseDir;
		sprintf(sDesc, "%s", m_sMseedBaseDir);
	}	
	sprintf(sNet, "%s", m_strNet);
	sprintf(sSta, "%s", m_strSta);
	sprintf(sDitc, "%s", m_strDitc);
	sprintf(sEventID, "%s", m_sEventID);

	memset(m_sMseedFullPath, 0x00, sizeof(m_sMseedFullPath));
	memcpy(m_sMseedFullPath, sDesc, sizeof(m_sMseedFullPath));
	
	if(m_sExecProcKind.GetLength() > 0)
	{
		sprintf(sExecProcKind, "%s", m_sExecProcKind);
	}

	m_MakeEvaBD.m_sEventID = m_sEventID;
	m_MakeEvaBD.m_sExecProcKind = m_sExecProcKind;

	CString strLog = "";
	strLog.Format("Desc = '%s', Net = '%s', Sta = '%s', ditc = '%s', eventId = '%s', execProcKind = '%s'", sDesc, sNet, sSta, sDitc, sEventID, sExecProcKind);
	WriteReportLog(strLog);

//	m_MakeEvaBD.MakeMseed2Txt(sDesc, sNet, sSta);

	m_MakeEvaBD.m_strDitc = m_strDitc;

	// 기타 설정 파일 읽기 Start
	int nReadEtcCfgRet = m_MakeEvaBD.ReadEtcConfig(strInfoFilePath);

	if( m_strDitc == "K" || m_strDitc == "D" )	// 콘크리트댐, 필댐 및 저수지는 안전성 종합 평가의 점검 필요 허용 갯수를 무조건 0 으로 설정
	{
		m_MakeEvaBD.m_nWarningLimit = 1;
		m_MakeEvaBD.m_nMinAvailChanCnt = 1;
	}
	else	// 안전성 평가 항목이 여러 성분인 특수교량만 우선 적용
	{
		if (nReadEtcCfgRet == -1)
		{
			m_MakeEvaBD.m_nWarningLimit = 1;
			m_MakeEvaBD.m_nMinAvailChanCnt = 3;
		}
	}
	// 기타 설정 파일 읽기 End

	// 안전성 평가를 위한 필수축 성분의 관리기준치 파일이 있는지 체크
	int nReadThInfoRet = 0;
	
	if (m_strDitc == "K")	// 콘크리트댐
	{
		nReadThInfoRet = m_MakeEvaBD.ReadThresholdsInfo2Array(strInfoFilePath, m_strDitc);
	}
	else if (m_strDitc == "D")	// 필댐 및 저수지
	{
		//nReadThInfoRet = m_MakeEvaBD.ReadThresholdsInfo2Array(strInfoFilePath, m_strDitc);
	}
	else if (m_strDitc == "S" || m_strDitc == "B")	// 교량-현수교, 교량-사장교
	{
		nReadThInfoRet = m_MakeEvaBD.ReadThresholdsInfo2Array(strInfoFilePath, m_strDitc);
		//CDialogEx::OnCancel();
		//return TRUE;
	}
	else
	{
		nReadThInfoRet = -1;
	}

	strLog = "";
	CString strErrMsg = "";

	switch(nReadThInfoRet)
	{
	case -1:	// 시설물 구분을 알 수 없음
		// 안전성평가 결과 파일에 오류사항 작성 후 종료		
		strLog.Format("Read Threshold Info Failed!! Unknown Ditc '%d'", nReadThInfoRet);
		WriteReportLog(strLog);
		strErrMsg = "시설물 구분을 알 수 없음";
		WriteLog(ERROR_FILENAME, strErrMsg);

		// 2019.07.12 콘크리트댐의 경우, 관리기준치 파일이 없을 경우, 이후 stationinfo 파일 체크 후 기본값으로 설정되도록 함
		// 2019.08.12 특수교량 추가
		if ( m_strDitc != "K" && m_strDitc != "S" && m_strDitc != "B" )
		{
			CDialogEx::OnCancel();
			return TRUE;
		}
		break;
	case -2:	// 관리기준치 파일이 없음
		// 안전성평가 결과 파일에 오류사항 작성 후 종료
		strLog.Format("Read Threshold Info Failed!! Not found threshold file '%d'", nReadThInfoRet);
		WriteReportLog(strLog);
		strErrMsg = "관리기준치 파일이 없음";
		WriteLog(ERROR_FILENAME, strErrMsg);

		// 2019.07.12 콘크리트댐의 경우, 관리기준치 파일이 없을 경우, 이후 stationinfo 파일 체크 후 기본값으로 설정되도록 함
		if ( m_strDitc != "K" && m_strDitc != "S" && m_strDitc != "B" )
		{
			CDialogEx::OnCancel();
			return TRUE;
		}
		break;
	}

	// 필수축 성분의 mSEED 구간 데이터가 인자로 넘어온 경로에 존재하는지 체크
	int nMseedExistRet = m_MakeEvaBD.CheckMseedFile_Exists(sDesc, sNet, sSta, sDitc);

	if ( nMseedExistRet == 0 )	// 성공
	{
		// mSEED 구간 데이터가 정상 데이터인지 체크(파일 열기 가능한지, 파싱 가능한 유효 데이터인지, 중간에 누락된 샘플이 없는지)
		int nMseedSampCheckRet = m_MakeEvaBD.CheckMseedFile_Samprate(sDesc, sNet, sSta);

		if ( nMseedSampCheckRet == 0 )
		{
			// 2019.07.12 콘크리트댐인데, 관리기준치 파일 처리에서 에러가 난 경우, 기본값으로 설정
			// 2019.08.12 특수교량 추가 적용
			if ( m_strDitc == "K" || m_strDitc == "S" || m_strDitc == "B" )
			{
				if ( nReadThInfoRet == -1 || nReadThInfoRet == -2 )
				{
					m_MakeEvaBD.SetDefault_Thresholds(sDitc);
				}
			}
			int nBuildingEvalutionRet = m_MakeEvaBD.MakeBaseDataProc(sDitc, sEventID, sDesc, sNet, sSta);	// 안전성 평가 수행 프로세스

			if ( nBuildingEvalutionRet == 1 )		// 안전성 평가 처리 완료
			{
				strLog = "";
				strLog.Format("=========Building Evalution2 Process Succeed");
				WriteReportLog(strLog);
				WriteLog(TRACE_FILENAME, m_sEventID + "=========안전성평가 처리완료");

				// 가통인 경우 DB에 결과 저장
				if(m_sExecProcKind == "KV")
				{
					CString strTime = GetDBSysTime();

					if(Insert_BuildingEvalutionResult_Detail(res_ok, sNet, sSta, "", strTime) == FALSE)
					{
						strLog = "";
						strLog.Format("Building Evalution2 Detail DB Insert Error");
						WriteReportLog(strLog);
						WriteLog(ERROR_FILENAME, "안전성 평가2 상세 DB 저장 실패");

						if(Insert_BuildingEvalutionResult(res_error, "", strTime) == FALSE)
						{
							strLog = "";
							strLog.Format("Building Evalution2 Result Failed DB Update Error. Ditc = '%s' EventID = '%s'", sDitc, m_sEventID);
							WriteReportLog(strLog);
							strErrMsg.Format("안전성 평가2 실패 결과 DB 저장 실패. 시설물 종류 = '%s', 이벤트ID = '%s'", sDitc, m_sEventID);
							WriteLog(TRACE_FILENAME, strErrMsg);
						}
						else
						{
							strLog = "";
							strLog.Format("Building Evalution2 Result Failed DB Insert Succeed");
							WriteReportLog(strLog);
							WriteLog(ERROR_FILENAME, "안전성 평가2 종합 실패 DB 저장 성공");
						}	
					}
					else
					{
						if(Insert_BuildingEvalutionResult(res_ok, "", strTime) == FALSE)
						{
							strLog = "";
							strLog.Format("Building Evalution2 Result DB Update Error. Ditc = '%s' EventID = '%s'", sDitc, m_sEventID);
							WriteReportLog(strLog);
							strErrMsg.Format("안전성 평가2 성공 결과 DB 저장 실패. 시설물 종류 = '%s', 이벤트ID = '%s'", sDitc, m_sEventID);
							WriteLog(TRACE_FILENAME, strErrMsg);
						}
						else
						{
							strLog = "";
							strLog.Format("Building Evalution2 Result DB Insert Succeed");
							WriteReportLog(strLog);
							strErrMsg.Format("안전성 평가2 종합 DB 저장 성공");
							WriteLog(ERROR_FILENAME, strErrMsg);
						}
					}
				}
				
				MakeReportFile05_v2();		// 댐,저수지 및 교량용 안전성 평가 보고서 작성	
			}
			else	// 안전성 평가 처리 실패
			{
				strLog = "";
				strLog.Format("=========Building Evalution2 Process Failed");
				WriteReportLog(strLog);
				WriteLog(TRACE_FILENAME, m_sEventID + "=========안전성평가 처리실패");

				//WritePrivateProfileString("result", "total", "평가 계산 실패", m_sMseedDir + "\\analysisSample.ini");
				//WritePrivateProfileString("result", "time", sTime, m_sMseedDir + "\\analysisSample.ini");
				
				// 가통인 경우 DB에 결과 저장
				if(m_sExecProcKind == "KV")
				{
					if(Insert_BuildingEvalutionResult(res_error) == FALSE)
					{
						strLog = "";
						strLog.Format("Building Evalution2 Error Result DB Insert Error");
						WriteReportLog(strLog);
						strErrMsg.Format("안전성 평가2 실패 결과 DB 저장 실패.");
						WriteLog(ERROR_FILENAME, strErrMsg);
					}
					else
					{
						strLog = "";
						strLog.Format("Building Evalution2 Error Result DB Update Succeed. Ditc = '%s' EventID = '%s'", sDitc, m_sEventID);
						WriteReportLog(strLog);
						strErrMsg.Format("안전성 평가2 실패 결과 DB 저장 성공. 시설물 구분 = '%s', 이벤트ID = '%s'", sDitc, m_sEventID);
						WriteLog(TRACE_FILENAME, strLog);
					}
				}
			}
		}
		else	// 실패
		{
			strLog = "";
			strErrMsg = "";		// DB 저장용 메세지

			// 안전성 평가 결과 파일에 해당 실패 사유 명시
			switch( nMseedSampCheckRet )
			{
			case -1:	// 필수축 성분 데이터 없음				
				strLog.Format("MiniSEED samprate check failed!! No ingredient data required '%d'", nMseedSampCheckRet);
				strErrMsg = "필수축 성분 데이터 없음";
				break;
			case -2:	// StationInfo.txt 파일이 없음
				strLog.Format("MiniSEED samprate check failed!! Not found station info file '%d'", nMseedSampCheckRet);
				strErrMsg = "StationInfo.txt 파일이 없음";
				break;
			case -3:	// mSEED 구간 데이터 파일열기 실패
				strLog.Format("MiniSEED samprate check failed!! Failed to open mSEED file '%d'", nMseedSampCheckRet);
				strErrMsg = "mSEED 구간 데이터 파일열기 실패";
				break;
			case -4:	// 인식할 수 없는 타입의 mSEED 구간 데이터
				strLog.Format("MiniSEED samprate check failed!! Unrecognized types of mSEED file '%d'", nMseedSampCheckRet);
				strErrMsg = "인식할 수 없는 타입의 mSEED 구간 데이터";
				break;
			case -5:	// mSEED 구간에 누락된 데이터 존재
				strLog.Format("MiniSEED samprate check failed!! Missing data in mSEED file '%d'", nMseedSampCheckRet);
				strErrMsg = "mSEED 구간에 누락된 데이터 존재";
				break;
			}

			// 가통인 경우 DB에 결과 저장
			if(m_sExecProcKind == "KV")
			{
				if(Insert_BuildingEvalutionResult(res_error, strErrMsg) == FALSE)
				{
					strLog.Append(" : Building Evalution2 Error Result DB Insert Error");
					WriteReportLog(strLog);
					strErrMsg.Append(" : 안전성 평가2 실패 결과 DB 저장 실패");
					WriteLog(ERROR_FILENAME, strErrMsg);
				}
				else
				{
					strLog.AppendFormat(" : Building Evalution2 Error Result DB Update Succeed. Ditc = '%s' EventID = '%s'", sDitc, m_sEventID);
					WriteReportLog(strLog);
					strErrMsg.Format(" : 안전성 평가2 실패 결과 DB 저장 성공. 시설물 구분 = '%s', 이벤트ID = '%s'", sDitc, m_sEventID);
					WriteLog(TRACE_FILENAME, strErrMsg);
				}
			}
			else
			{
				if( strLog.GetLength() > 0 )
				{
					WriteReportLog(strLog);
				}

				if( strErrMsg.GetLength() > 0 )
				{
					WriteLog(TRACE_FILENAME, strErrMsg);
				}
			}
		}
	}
	else	// 실패
	{
		// 안전성 평가에 필요한 필수축 성분 mSEED 구간 데이터 파일이 없음
		strLog = "";
		strLog.Format("Exist MiniSEED file check failed!! Not found mSEED data file '%d'", nMseedExistRet);
		strErrMsg = "안전성 평가에 필요한 필수축 성분 mSEED 구간 데이터 파일이 없음";

		// 가통인 경우 DB에 결과 저장
		if(m_sExecProcKind == "KV")
		{
			if(Insert_BuildingEvalutionResult(res_error, strErrMsg) == FALSE)
			{
				strLog.Append(" : Building Evalution2 Error Result DB Insert Error");
				WriteReportLog(strLog);
				strErrMsg.Append(" : 안전성 평가2 실패 결과 DB 저장 실패");
				WriteLog(ERROR_FILENAME, strErrMsg);
			}
			else
			{
				strLog.AppendFormat(" : Building Evalution2 Error Result DB Update Succeed. Ditc = '%s' EventID = '%s'", sDitc, m_sEventID);
				WriteReportLog(strLog);
				strErrMsg.Format(" : 안전성 평가2 실패 결과 DB 저장 성공. 시설물 구분 = '%s', 이벤트ID = '%s'", sDitc, m_sEventID);
				WriteLog(TRACE_FILENAME, strErrMsg);
			}
		}
		else
		{
			WriteReportLog(strLog);
			WriteLog(TRACE_FILENAME, strErrMsg);
		}
	}

	//if(nRet == 1)
	//{
	//	m_MakeEvaBD.MakeBaseDataProc(sDitc, sEventID, sDesc, sNet, sSta);
	//}
	//m_MakeEvaBD.testBandPassFilter(fps, dataCount, lowFreq, highFreq, FilterOrder);

	CDialogEx::OnCancel();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

//void CBuildingEvalutionDlg::OnTimer(UINT nIDEvent) 
//{	
//	CDialog::OnTimer(nIDEvent);
//}

CString CBuildingEvalutionDlg::GetDBSysTime()
{
	CString strRet = "";
	CString sLog = "";
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int num_fields;
	CString sSQL= "";

	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 
	{
		sLog.Format("%s", "MySQL Connect Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return strRet;
	}
	mysql_set_character_set(conn, "euckr");

	sSQL.Format("select SYSDATE()");

	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s : %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return strRet;
	}

	res = mysql_use_result(conn);
	num_fields = mysql_num_fields(res);
	row = mysql_fetch_row(res);
	if (row == false)		strRet = "";
	else					strRet = row[0];
	mysql_free_result(res);

	return strRet;
}

void CBuildingEvalutionDlg::Wait(DWORD dwMillisecond)
{
	MSG msg;
	DWORD dwStart;
	dwStart = GetTickCount();
 
	while(GetTickCount() - dwStart < dwMillisecond)
	{
		while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}


void CBuildingEvalutionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CBuildingEvalutionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CBuildingEvalutionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 실행창숨김처리
void CBuildingEvalutionDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	//BOOL visible = true;			// false:숨김 true:보임

	if(!m_visible)
        lpwndpos->flags &= ~SWP_SHOWWINDOW;

	CDialogEx::OnWindowPosChanging(lpwndpos);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}

void CBuildingEvalutionDlg::OnBnClickedButton1()
{
}

// 2015.05 다중계측소 Thread위함
#ifdef	DEF_MULTI_SITE_SUPPORT

void CBuildingEvalutionDlg::setNetSta(char* strNet, char* strObsID)
#else
void CBuildingEvalutionDlg::setNetSta()
#endif
{
#ifdef	DEF_MULTI_SITE_SUPPORT
	m_strNet  =  CA2T(strNet);	// 해당데이터가 없음
	m_strSta  =  CA2T(strObsID);

//#ifdef _DEBUG
//	m_strNet = "PS";
//	m_strSta = "PS";
//#endif	// DEBUG

#else

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	CString sSQL= "";
	CString sLog;

	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 
	{
		sLog.Format("%s", "MySQL Connect Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	mysql_set_character_set(conn, "euckr");

	if( m_sEventID.GetLength() >= 14 )
	{
		sSQL.Format("select net, obs_id from hevtinfo  where event_id ='%s'", m_sEventID);
		if (mysql_query(conn, (CStringA)sSQL)) {
			sLog.Format("%s: %s", "MySQL query Error", sSQL);
			WriteLog(ERROR_FILENAME, sLog);
			mysql_close(conn);
			return;
		}
		// 출력
		res = mysql_use_result(conn);
		row = mysql_fetch_row(res);
		if (row == false)			
		{
			m_strNet  =  "";	// 해당데이터가 없음
			m_strSta  =  "";
		}
		else	
		{
			m_strNet.Format("%s", CA2T(row[0]));
			m_strSta.Format("%s", CA2T(row[1]));
		}
						
		mysql_free_result(res);
		mysql_close(conn);
	}
	else
	{
		sSQL.Format("select net, obs_id from site_info where use_yn = 'Y' and master_yn = 'Y'", m_sEventID);
		if (mysql_query(conn, (CStringA)sSQL)) {
			sLog.Format("%s: %s", "MySQL query Error", sSQL);
			WriteLog(ERROR_FILENAME, sLog);
			mysql_close(conn);
			return;
		}
		// 출력
		res = mysql_use_result(conn);
		row = mysql_fetch_row(res);
		if (row == false)			
		{
			m_strNet  =  "";	// 해당데이터가 없음
			m_strSta  =  "";
		}
		else	
		{
			m_strNet.Format("%s", CA2T(row[0]));
			m_strSta.Format("%s", CA2T(row[1]));
		}
						
		mysql_free_result(res);
		mysql_close(conn);
	}
#endif
}
//~ 2015.05

void CBuildingEvalutionDlg::setDitc(CString sNet, CString sSta, CString &sDitc)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	CString sSQL= "";
	CString sLog;

	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 
	{
		sLog.Format("%s", "MySQL Connect Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	mysql_set_character_set(conn, "euckr");

	sSQL.Format("select ditc from observatory  where net ='%s' and obs_id = '%s'", sNet, sSta);
	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s: %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	// 출력
	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);
	if (row == false)			
	{
		sDitc = "";
	}
	else	
	{
		sDitc.Format("%s", CA2T(row[0]));
	}

	mysql_free_result(res);
	mysql_close(conn);
}

//************************************
// Method:    MakeStationInfoTxt
// FullName:  CBuildingEvalutionDlg::MakeStationInfoTxt
// Access:    public 
// Returns:   int		1  : 성공
// Returns:			0  : 실패(DB연결 실패)
// Returns:			-1 : 실패(쿼리 실패)
// Returns:			-2 : 실패(쿼리 데이터 없음)
// Returns:			-3 : 실패(파일쓰기실패)
// Qualifier:
// Parameter: CString sNet
// Parameter: CString sSta
//************************************
int CBuildingEvalutionDlg::MakeStationInfoTxt(CString sNet, CString sSta)
{
	int nRet = 0;

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	CString sSQL= "";
	CString sLog;
	CString sQueryResult = "";

	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 
	{
		sLog.Format("%s", "MySQL Connect Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return nRet;
	}
	mysql_set_character_set(conn, "euckr");

	sSQL.Format("select concat(group_concat(concat(sen_id, ':', z_response, ',', n_response, ',', e_response, ',', z_sensitivity, ',', n_sensitivity, ',', e_sensitivity) separator ';') , ';') from obs_sensor where net ='%s' and obs_id = '%s'", sNet, sSta);
	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s: %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		nRet = -1;
		return nRet;
	}
	// 출력
	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);
	if (row == false)			
	{
		nRet = -2;
	}
	else
	{
		sQueryResult = CA2T(row[0]);
	}

	TCHAR pQueryResult[2048];
	char* pTmpBuffer = NULL;
	int sLen = 0;

	if(sQueryResult.GetLength() > 0)
	{
		//memset(pQueryResult, 0x00, sizeof(pQueryResult));
		//memcpy(pQueryResult, sQueryResult, sizeof(pQueryResult));

		sLen = WideCharToMultiByte(CP_ACP, 0, CT2W(sQueryResult), -1, NULL, 0, NULL, NULL);

		if(sLen > 0)
		{
			pTmpBuffer = new char[sLen + 1];
			memset(pTmpBuffer, 0x00, sizeof(sLen + 1));
			WideCharToMultiByte(CP_ACP, 0, CT2W(sQueryResult), -1, pTmpBuffer, sLen, NULL, NULL);
		}
	}	

	// 안전성평가용 mSEED 구간자료 경로에 stationinfo.txt 생성하여 쿼리 결과 저장
	DWORD dwWritten;
	CString strEventBaseDir = "";
	strEventBaseDir = m_sMseedBaseDir + "\\" + m_sEventID;

	if( sLen > 0 )
	{
		if( GetFileAttributes(strEventBaseDir) != -1)
		{
			CString strStationInfoFileName = "";
			strStationInfoFileName = strEventBaseDir + "\\" + STATIONINFO_TXT;
			HANDLE hFile = CreateFile(strStationInfoFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile != NULL)
			{
				//WriteFile(hFile, sQueryResult.GetBuffer(), sQueryResult.GetLength(), &dwWritten, NULL);
				WriteFile(hFile, pTmpBuffer, sLen, &dwWritten, NULL);
				nRet = 1;
			}
			CloseHandle(hFile);
		}
		else
		{
			nRet = -3;
		}
	}
	else
	{
		nRet = -3;
	}

	if(pTmpBuffer)
	{
		delete pTmpBuffer;
	}

	mysql_free_result(res);
	mysql_close(conn);

	return nRet;
}

void CBuildingEvalutionDlg::OnBnClickedButton2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_sMseedDir = m_sMseedDir + "20150323193246";
	m_sEventID = "20150323193246";
}

// 향후 다중사이트 처리 작업시 해당 관리자 정보 읽을때 고려해야 할 것임
void CBuildingEvalutionDlg::GetUserDeptInfo()
{
    int dataCount = 0;
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int num_fields;
	CString	sLog = "";
	int	nRow = 0;

    m_strUser_dept = "";
    m_strUser_duty = "";
    m_strUser_name = "";
    m_strUser_tel = "";	// user_mobile, user_email(sms_yn/email_yn)

	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 
	{
		sLog.Format("%s", "MySQL Connect Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	mysql_set_character_set(conn, "euckr");
	
    CString query = "";

	query.AppendFormat("select  user_dept, user_duty, user_name, user_tel \n");
	query.AppendFormat(" from tbl_user \n");
	query.AppendFormat(" where main_yn = 'Y' \n");

	if (mysql_query(conn, (CStringA)query)) {
		sLog.Format("%s: %s", "MySQL query Error", query);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}

	res = mysql_use_result(conn);
	num_fields = mysql_num_fields(res);
	row = mysql_fetch_row(res);
	if (row == false)		
	{
		sLog.Format("%s info is empty", m_sEventID);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	else
	{
		m_strUser_dept.Format("%s", CA2T(row[0]));
		m_strUser_duty.Format("%s", CA2T(row[1]));
		m_strUser_name.Format("%s", CA2T(row[2]));
		m_strUser_tel.Format("%s", CA2T(row[3]));
	}	
	mysql_free_result(res);
	mysql_close(conn);
}

void CBuildingEvalutionDlg::MakeReportFile05_v2()
{
    int dataCount = 0;
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int num_fields;
	CString	sTableName = "";
	CString sEventType = "";
	CString	sData[16];	
	CString sFileName;
	CStdioFile fgloset;
	CString	sLog = "";
	CString sEventCol = "";
	int	nRow = 0;

	
	if(m_sEventID.GetLength() <= 10)	
	{
		sTableName = "kevtinfo_rpt2";	// 지진
		sEventType = "K";
		sEventCol = "eq_no";
	}
	else								
	{
		sTableName = "hevtinfo_rpt2";	// 자체
		sEventType = "H";
		sEventCol = "event_id";
	}

	if (m_sExecProcKind == "KV")
	{
		conn = mysql_init(NULL);
		if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 
		{
			sLog.Format("%s", "MySQL Connect Error");
			WriteReportLog(sLog);
			WriteLog(ERROR_FILENAME, sLog);
			mysql_close(conn);
			return;
		}
		mysql_set_character_set(conn, "euckr");
	
		CString query = "";

		//query.Append("select  a.eq_no, a.net, a.obs_id \n" );
		query.AppendFormat("select  %s, net, obs_id \n", sEventCol);
		query.Append("     , acc_val \n");
		query.Append("     , sp_val \n");
		query.Append("     , dis_val \n");
		query.Append("     , cav_val \n");
		query.Append("     , stm_val \n");		
		query.Append("     , result \n");
		query.Append("     , user_dept \n");
		query.Append("     , user_duty \n");
		query.Append("     , user_name \n");
		query.Append("     , user_tel \n");	// user_mobile, user_email(sms_yn/email_yn)
		query.AppendFormat(" from %s \n", sTableName);
		query.AppendFormat(" where %s = '%s' and net = '%s' and obs_id = '%s' \n", sEventCol, m_sEventID, m_strNet, m_strSta);

		bool bNotConn = false;

		if (mysql_query(conn, (CStringA)query)) {
			sLog.Format("%s: %s", "MySQL query Error", query);
			WriteReportLog(sLog);
			WriteLog(ERROR_FILENAME, sLog);
			mysql_close(conn);
			bNotConn = true;
			//return;
		}
		else
		{
			res = mysql_use_result(conn);
			num_fields = mysql_num_fields(res);
			row = mysql_fetch_row(res);
			if (row == false)
			{
				sLog.Format("%s info is empty", m_sEventID);
				WriteReportLog(sLog);
				WriteLog(ERROR_FILENAME, sLog);
				mysql_close(conn);
				bNotConn = true;
				//return;
			}
			else
			{
				for(int i = 0; i < num_fields; i++)
				{
					if(row[i] == NULL)
					{
						sData[i].Format("");
					}
					else
					{
						sData[i].Format("%s", CA2T(row[i]));
					}
				}
				nRow++;

				mysql_free_result(res);
				mysql_close(conn);
			}
		}

		if( bNotConn == true )
		{
			nRow = 1;
			sData[0] = m_sEventID;
			sData[1] = m_strNet;
			sData[2] = m_strSta;
			sData[3] = m_MakeEvaBD.m_strAcc_Val;
			sData[4] = m_MakeEvaBD.m_strSp_Val;
			sData[5] = m_MakeEvaBD.m_strDis_Val;
			sData[6] = m_MakeEvaBD.m_strCav_Val;
			sData[7] = m_MakeEvaBD.m_strStm_Val;
			sData[8] = m_MakeEvaBD.m_strTot_Result;
			sData[9] = "";
			sData[10] = "";
			sData[11] = "";
			sData[12] = "";
		}
	}
	else
	{
		nRow = 1;
		sData[0] = m_sEventID;
		sData[1] = m_strNet;
		sData[2] = m_strSta;
		sData[3] = m_MakeEvaBD.m_strAcc_Val;
		sData[4] = m_MakeEvaBD.m_strSp_Val;
		sData[5] = m_MakeEvaBD.m_strDis_Val;
		sData[6] = m_MakeEvaBD.m_strCav_Val;
		sData[7] = m_MakeEvaBD.m_strStm_Val;
		sData[8] = m_MakeEvaBD.m_strTot_Result;
		sData[9] = "";
		sData[10] = "";
		sData[11] = "";
		sData[12] = "";
	}

	//--------------------------------------파일에 쓰기
	CTime t = CTime::GetCurrentTime();
	CString sTime;
	sTime.Format("%04d%02d%02d%02d%02d%02d",t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
	CString	reportData;

	if (m_sExecProcKind == "KV")
	{
		sFileName.Format("%s\\RPT05_%s_%s_%s.txt", m_strReportDefDir, sData[1], sData[2], sTime);
	}
	else
	{		
		//sFileName.Format("%s\\RESULT\\RPT05_%s_%s_%s.txt", m_sMseedBaseDir, sData[1], sData[2], sTime);
		sFileName.Format("%s\\RESULT\\RPT05_%s_%s_%s.txt", m_sMseedBaseDir, sData[1], sData[2], m_sEventID);	// 2019.11.19 1,2세부 담당자 합의하에 시간 -> eventID로 변경
	}
	
	fgloset.Open(sFileName, CFile::modeCreate | CFile::modeWrite | CFile::typeText);
	
	// 기본적으로 nRow=1이어야함
	for(int i = 0 ; i < nRow; i++)
	{
		reportData.AppendFormat("ditc : %s\n", sEventType);
        reportData.AppendFormat("event_id : %s\n", sData[0]);
        reportData.AppendFormat("net : %s\n", sData[1]);
        reportData.AppendFormat("obs_id : %s\n", sData[2]);
        reportData.AppendFormat("acc_val : %s\n", sData[3]);
		reportData.AppendFormat("sp_val : %s\n", sData[4]);
        reportData.AppendFormat("dis_val : %s\n", sData[5]);
        reportData.AppendFormat("cav_val : %s\n", sData[6]);
        reportData.AppendFormat("stm_val : %s\n", sData[7]);
        reportData.AppendFormat("result : %s\n", sData[8]);
        reportData.AppendFormat("user_dept : %s\n", sData[9]);
        reportData.AppendFormat("user_duty : %s\n", sData[10]);
        reportData.AppendFormat("user_name : %s\n", sData[11]);
        reportData.AppendFormat("user_tel : %s\n", sData[12]);
		fgloset.WriteString(reportData);
	} 

    fgloset.Close();
}

void CBuildingEvalutionDlg::iniIniFile()
{
	TCHAR	szIniBuffer[1100];
	DWORD	dRet;
	TCHAR	szError[256];
	CString	sLog;

	m_strDBIP = _T("");	// DB정보
	dRet = GetPrivateProfileString("mySQLDB", "DB_IP", "1",szIniBuffer, (DWORD)sizeof(szIniBuffer)-1, CONFIG_INI);
	m_strDBIP = szIniBuffer;
	m_iDBPORT = GetPrivateProfileInt("mySQLDB", "DB_PORT", (int)3306, CONFIG_INI);
	m_strDBID = _T("");	
	dRet = GetPrivateProfileString("mySQLDB", "DB_ID", "accuser",szIniBuffer, (DWORD)sizeof(szIniBuffer)-1, CONFIG_INI);
	m_strDBID = szIniBuffer;
	m_strDBPW = _T("");	
	dRet = GetPrivateProfileString("mySQLDB", "DB_PW", "accuser123",szIniBuffer, (DWORD)sizeof(szIniBuffer)-1, CONFIG_INI);
	m_strDBPW = szIniBuffer;
	m_strDBDB = _T("");	
	dRet = GetPrivateProfileString("mySQLDB", "DB_DB", "accdb",szIniBuffer, (DWORD)sizeof(szIniBuffer)-1, CONFIG_INI);
	m_strDBDB = szIniBuffer;

	//---------------------------DB
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	CString sSQL= "";

	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 	
	{
		sLog.Format("%s", "MySQL Connect Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	mysql_set_character_set(conn, "euckr");

	// 안전성평가루트\이벤트명
	sSQL = "select cd_val from tbl_setting where cd_id = 'DATA_DEF_DIR' and ditc = 'B' and use_yn='Y'";
	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s: %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	// 출력
	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);
	if (row == false)
	{
		m_sMseedBaseDir = "C:\\Temp";
		m_sMseedDir  =  "C:\\Temp" + m_sEventID;	// 해당데이터가 없음
	}
	else
	{
		m_sMseedBaseDir.Format("%s", CA2T(row[0]));
		m_sMseedDir.Format("%s", CA2T(row[0]));
	}
	m_sMseedDir = m_sMseedDir + "\\" + m_sEventID;
	mysql_free_result(res);


	// log level
	sSQL = "select cd_val from tbl_setting where cd_id = 'LOGLEVEL' and ditc = 'L' and use_yn='Y'";
	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s: %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	// 출력
	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);
	if (row == false)			m_iLogLevel  =  (int)0;	// 해당데이터가 없음
	else						m_iLogLevel = atoi(row[0]);
	mysql_free_result(res);
	
	// 로그루트
	m_strLogDataDefDir = _T("");	
	sSQL = "select cd_val from tbl_setting where cd_id = 'LOG_DEF_DIR' and ditc = 'L' and use_yn='Y'";
	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s: %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	// 출력
	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);
	if (row == false)			m_strLogDataDefDir  =  "C:\\Log";	// 해당데이터가 없음
	else						m_strLogDataDefDir.Format("%s", CA2T(row[0]));
	mysql_free_result(res);

	// 리포트저장위치
	m_strReportDefDir = _T("");		
	sSQL = "select cd_val from tbl_setting where cd_id = 'RPT_DEF_DIR' and ditc = 'R' and use_yn='Y'";
	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s: %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	// 출력
	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);
	if (row == false)			m_strReportDefDir  =  "C:\\Report";	// 해당데이터가 없음
	else						m_strReportDefDir.Format("%s", CA2T(row[0]));
	mysql_free_result(res);
	
	if( _access( (CStringA)m_strReportDefDir, 0x00) != 0 )	// 없으면 만든다
	{
		if( _mkdir( (CStringA)m_strReportDefDir) != 0 )
		{
			wsprintf(szError, "Check Report File Default Directory[%s]", m_strReportDefDir);
			WriteLog(ERROR_FILENAME, szError);
			return ;
		}	
	}

	// 안전성평가 고유진동수비값 CString steplist[4][2] = {{"0","안전"},{"1","점검필요"},{"2","심한손상"},{"3","대피"}};
	m_strBackMajorNaturalFrequency = _T("");
	m_strBackMinorNaturalFrequency = _T("");
	sSQL = ""; 
	sSQL.Append("select fre_long_val  , fre_short_val, evt, regdate, result from ");
	sSQL.Append("    (select fre_long_val  , fre_short_val, event_id as evt, regdate, result from hevtinfo_rpt ");
	sSQL.Append("    union all ");
	sSQL.Append("    select fre_long_val  , fre_short_val, eq_no as evt, regdate, result from kevtinfo_rpt ) as tbl ");
	sSQL.Append("where result = '안전' or result = '점검필요' or result = '심한손상' or result = '대피' ");
	sSQL.Append("group by regdate desc limit 1");
	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s: %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return;
	}
	// 출력
	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);
	if (row == false)			
	{
			m_strBackMajorNaturalFrequency  =  "";	// 해당데이터가 없음
			m_strBackMinorNaturalFrequency  =  "";
	}
	else	
	{
		m_strBackMajorNaturalFrequency.Format("%s", CA2T(row[0]));
		m_strBackMinorNaturalFrequency.Format("%s", CA2T(row[1]));
	}
						
	mysql_free_result(res);
	mysql_close(conn);
}

void CBuildingEvalutionDlg::WriteLog(CString pFile, CString sLog)
{
#ifdef _DEBUG
  _tsetlocale(LC_ALL, _T("korean"));
#endif
	//TRACE("\n");
	TRACE(sLog + "\n");
	if( m_iLogLevel == 0 ) {
		return;
	}

	CFile	stream;
	CString	szlogBuff;
	char	szDateUTC[101];
	CTime t = CTime::GetCurrentTime();

	sprintf(szDateUTC, "%04d%02d%02d",  t.GetYear(), t.GetMonth(), t.GetDay());
	
	MakeQscdDir(m_strLogDataDefDir,  t.GetYear(), t.GetMonth(), t.GetDay());

	szlogBuff = genLogFileFullName(pFile,  szDateUTC);
	if( szlogBuff  == "" )
	{
		return ;
	}

	if(stream.Open(szlogBuff, CFile::modeWrite | CFile::modeCreate |CFile::modeNoTruncate)) 
	{
		stream.SeekToEnd();
 #ifdef _UNICODE // 유니코드 문자집합 설정이라면 
		USHORT uBOM = 0xfeff; 
		stream.Write(&uBOM,sizeof(uBOM)); // 파일의 첫부분에 BOM저장
#endif
		
		szlogBuff.Format("%04d-%02d-%02d %02d:%02d:%02d %s\r\n", 
				t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond(), sLog);

		stream.Write(szlogBuff, sizeof(TCHAR)*szlogBuff.GetLength());
		stream.Close();
	}
}

CString CBuildingEvalutionDlg::genLogFileFullName(CString spFileName, char* cpDate)
{
	CString cpFileFullName = "";
	TCHAR szError[256];
	
	if( strlen(cpDate) < 8 )
	{
		wsprintf(szError, "Check Date value _size error[%s]", cpDate);
		WriteLog(spFileName, szError);
		return "";
	}
		
	// ex : /2012/03/08/fileName
	cpFileFullName.Format("%s/%c%c%c%c/%c%c/%c%c/%s"
					, m_strLogDataDefDir
					, cpDate[0], cpDate[1], cpDate[2], cpDate[3]
					, cpDate[4], cpDate[5]
					, cpDate[6], cpDate[7]
					, spFileName);

	
    return cpFileFullName;
}



int CBuildingEvalutionDlg::MakeQscdDir(CString szInitDir, int szYYYY, int szMM, int szDD)
{
	//char szDirName[1024+100];
	CString szDirName;

	if( _access( (CStringA)szInitDir, 0x00) != 0 )
	{
		if( _mkdir( (CStringA)szInitDir) != 0 )
		{
			return 0;
		}	
	}

	szDirName.Format("%s/%04d", szInitDir, szYYYY);
	if( _access((CStringA)szDirName, 0x00) != 0 )
	{
		if( _mkdir((CStringA)szDirName) != 0 )
		{
			return 0;
		}	
	}

	szDirName.Format("%s/%04d/%02d", szInitDir, szYYYY, szMM);
	if( _access((CStringA)szDirName, 0x00) != 0 )
	{
		if( _mkdir((CStringA)szDirName) != 0 )
		{
			return 0;
		}	
	}

	szDirName.Format("%s/%04d/%02d/%02d", szInitDir, szYYYY, szMM, szDD);
	if( _access((CStringA)szDirName, 0x00) != 0 )
	{
		if( _mkdir((CStringA)szDirName) != 0 )
		{
			return 0;
		}	
	}
	return 1;
}

void CBuildingEvalutionDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}

// 2019.06.18 안전성 평가 종합 결과 저장
BOOL CBuildingEvalutionDlg::Insert_BuildingEvalutionResult(int iResType, CString sError, CString sTime)
{
	int	iQuery = 0;					// 0:insert 1:update
	TCHAR	szIniBuffer[256];		// ini결과데이터
	CString	sLog = "";
	//------------------------------------해당 EventID건이 있는지 체크(Update/Insert결정위함)
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int num_fields;
	int num_rows;
	CString	sTableName = "";
	CString sSQL= "";
	CString	sEventCol = "";
	if(m_sEventID.GetLength() <= 10)	
	{
		sTableName = "kevtinfo_rpt2";	// 지진
		sEventCol = "eq_no";
	}
	else		
	{
		sTableName = "hevtinfo_rpt2";	// 자체
		sEventCol = "event_id";
	}
	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 
	{
		sLog.Format("%s", "MySQL Connect Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return FALSE;
	}
	mysql_set_character_set(conn, "euckr");
	sSQL.Format("select %s from %s where %s = '%s' and net = '%s' and obs_id = '%s'" , sEventCol, sTableName, sEventCol, m_sEventID, m_strNet, m_strSta);
	
	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s : %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return FALSE;
	}

	res = mysql_use_result(conn);
	num_fields = mysql_num_fields(res);
	row = mysql_fetch_row(res);
	if (row == false)		iQuery = 0;	//INSERT
	else					iQuery = 1;	//UPDATE
	mysql_free_result(res);

	CString strDate = "";

	if( sTime.GetLength() <= 0 )
	{
		strDate = GetDBSysTime();
		if( strDate.GetLength() <= 0 )
		{
			CTime t = CTime::GetCurrentTime();
			strDate.Format("%04d-%02d-%02d %02d:%02d:%02d",t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
		}
	}
	else
	{
		strDate = sTime;
	}

	// detail 테이블에 먼저 처리되어 등록된 동일한 이벤트ID, 동일한 net, obs_id 로 등록된 기준별 result 값을 체크하여,
	// acc_val, dis_val, cav_val, stm_val, acc_spec_val 값 설정
	CString strAcc_Val = "";
	CString strDis_Val = "";
	CString strCav_Val = "";
	CString strStm_Val = "";
	CString strAcc_Spec_Val = "";

	sSQL = "";

	int nTotalRow = 0;	// detail 테이블에 등록된 동일한 이벤트ID, net, obs_id 평가 갯수

	if( m_strDitc == "S" || m_strDitc == "B" )	// 교량(현수교), 교량(사장교)
	{
		// 가속도 데이터 확인
		sSQL.Format("select result from %s_detail where %s = '%s' and sen_id like '%s_%s%%' and chan_id like 'H%%' and kind_val = 'A'", sTableName, sEventCol, m_sEventID, m_strNet, m_strSta);

		if (mysql_query(conn, (CStringA)sSQL)) {
			sLog.Format("%s : %s", "MySQL query Error", sSQL);
			WriteLog(ERROR_FILENAME, sLog);
			mysql_close(conn);
			return FALSE;
		}

		res = mysql_store_result(conn);
		num_fields = mysql_num_fields(res);
		num_rows = mysql_num_rows(res);

		int nFailCnt = 0;				// 점검 필요 수
		int nSystemFailCnt = 0;		// 시스템 점검 필요 수

		if( num_rows > 0 )
		{
			while((row = mysql_fetch_row(res)) != NULL)
			{
				CString strRet = row[0];

				if( strRet == "점검필요" )
				{
					nFailCnt++;
				}
				else if( strRet == "시스템 점검 필요" || strRet == "" )
				{
					nSystemFailCnt++;
				}
			}

			// 점검 필요 를 시스템 점검 필요 보다 우선으로 함
			if( nFailCnt > 0 )		// 점검 필요
			{
				strAcc_Val = "점검필요";
			}
			else if( nSystemFailCnt > 0 )	// 시스템 점검 필요
			{
				//strAcc_Val = "시스템 점검 필요";
				strAcc_Val = "점검필요";
			}
			else	// 안전
			{
				strAcc_Val = "안전";
			}
		}
		else
		{
			//strAcc_Val = "실패(데이터 없음)";
			strAcc_Val = "점검필요";
		}

		nTotalRow += num_rows;

		mysql_free_result(res);

		// 변위 데이터 확인
		sSQL.Format("select result from %s_detail where %s = '%s' and sen_id like '%s_%s%%' and chan_id like 'H%%' and kind_val = 'D'", sTableName, sEventCol, m_sEventID, m_strNet, m_strSta);

		if (mysql_query(conn, (CStringA)sSQL)) {
			sLog.Format("%s : %s", "MySQL query Error", sSQL);
			WriteLog(ERROR_FILENAME, sLog);
			mysql_close(conn);
			return FALSE;
		}

		res = mysql_store_result(conn);
		num_fields = mysql_num_fields(res);
		num_rows = mysql_num_rows(res);

		nFailCnt = 0;				// 점검 필요 수
		nSystemFailCnt = 0;		// 시스템 점검 필요 수

		if( num_rows > 0 )
		{
			while((row = mysql_fetch_row(res)) != NULL)
			{
				CString strRet = row[0];

				if( strRet == "점검필요" )
				{
					nFailCnt++;
				}
				else if( strRet == "시스템 점검 필요" || strRet == "" )
				{
					nSystemFailCnt++;
				}
			}

			// 점검 필요 를 시스템 점검 필요 보다 우선으로 함
			if( nFailCnt > 0 )		// 점검 필요
			{
				strDis_Val = "점검필요";
			}
			else if( nSystemFailCnt > 0 )	// 시스템 점검 필요
			{
				//strDis_Val = "시스템 점검 필요";
				strDis_Val = "점검필요";
			}
			else	// 안전
			{
				strDis_Val = "안전";
			}
		}
		else
		{
			//strDis_Val = "실패(데이터 없음)";
			strDis_Val = "점검필요";
		}

		nTotalRow += num_rows;

		mysql_free_result(res);

		// 가속도 응답스펙트럼 데이터 확인
		sSQL.Format("select result from %s_detail where %s = '%s' and sen_id like '%s_%s%%' and chan_id like 'H%%' and kind_val = 'SP'", sTableName, sEventCol, m_sEventID, m_strNet, m_strSta);

		if (mysql_query(conn, (CStringA)sSQL)) {
			sLog.Format("%s : %s", "MySQL query Error", sSQL);
			WriteLog(ERROR_FILENAME, sLog);
			mysql_close(conn);
			return FALSE;
		}

		res = mysql_store_result(conn);
		num_fields = mysql_num_fields(res);
		num_rows = mysql_num_rows(res);

		nFailCnt = 0;				// 점검 필요 수
		nSystemFailCnt = 0;		// 시스템 점검 필요 수

		if( num_rows > 0 )
		{
			while((row = mysql_fetch_row(res)) != NULL)
			{
				CString strRet = row[0];

				if( strRet == "점검필요" )
				{
					nFailCnt++;
				}
				else if( strRet == "시스템 점검 필요" || strRet == "" )
				{
					nSystemFailCnt++;
				}
			}

			// 점검 필요 를 시스템 점검 필요 보다 우선으로 함
			if( nFailCnt > 0 )		// 점검 필요
			{
				strAcc_Spec_Val = "점검필요";
			}
			else if( nSystemFailCnt > 0 )	// 시스템 점검 필요
			{
				//strAcc_Spec_Val = "시스템 점검 필요";
				strAcc_Spec_Val = "점검필요";
			}
			else	// 안전
			{
				strAcc_Spec_Val = "안전";
			}
		}
		else
		{
			//strAcc_Spec_Val = "실패(데이터 없음)";
			strAcc_Spec_Val = "점검필요";
		}

		//nTotalRow += num_rows;	// 가속도 응답스펙트럼은 가속도 데이터 검증이 가능하면 시행하므로, 평가 성분 총 갯수에서 배제

		mysql_free_result(res);

		if( nTotalRow < m_MakeEvaBD.m_nMinAvailChanCnt )	// 최소 평가 성분 갯수보다 작으면, 전체 점검필요 로 강제 설정)
		{
			strAcc_Val = "점검필요";
			strDis_Val = "점검필요";
			strAcc_Spec_Val = "점검필요";
		}
	}
	else if( m_strDitc == "K" )	// 콘크리트댐
	{
		// CAV 데이터 확인
		sSQL.Format("select result from %s_detail where %s = '%s' and sen_id like '%s_%s%%' and chan_id like 'H%%' and kind_val = 'C'", sTableName, sEventCol, m_sEventID, m_strNet, m_strSta);

		if (mysql_query(conn, (CStringA)sSQL)) {
			sLog.Format("%s : %s", "MySQL query Error", sSQL);
			WriteLog(ERROR_FILENAME, sLog);
			mysql_close(conn);
			return FALSE;
		}

		res = mysql_store_result(conn);
		num_fields = mysql_num_fields(res);
		num_rows = mysql_num_rows(res);

		int nFailCnt = 0;				// 점검 필요 수
		int nSystemFailCnt = 0;		// 시스템 점검 필요 수

		if( num_rows > 0 )
		{
			while((row = mysql_fetch_row(res)) != NULL)
			{
				CString strRet = row[0];

				if( strRet == "점검필요" )
				{
					nFailCnt++;
				}
				else if( strRet == "시스템 점검 필요" || strRet == "" )
				{
					nSystemFailCnt++;
				}
			}

			// 점검 필요 를 시스템 점검 필요 보다 우선으로 함
			if( nFailCnt > 0 )		// 점검 필요
			{
				strCav_Val = "점검필요";
			}
			else if( nSystemFailCnt > 0 )	// 시스템 점검 필요
			{
				//strCav_Val = "시스템 점검 필요";
				strCav_Val = "점검필요";
			}
			else	// 안전
			{
				strCav_Val = "안전";
			}
		}
		else
		{
			//strCav_Val = "실패(데이터 없음)";
			strCav_Val = "점검필요";
		}

		nTotalRow += num_rows;

		mysql_free_result(res);

		// Acc 데이터 확인
		sSQL.Format("select result from %s_detail where %s = '%s' and sen_id like '%s_%s%%' and chan_id like 'H%%' and kind_val = 'A'", sTableName, sEventCol, m_sEventID, m_strNet, m_strSta);

		if (mysql_query(conn, (CStringA)sSQL)) {
			sLog.Format("%s : %s", "MySQL query Error", sSQL);
			WriteLog(ERROR_FILENAME, sLog);
			mysql_close(conn);
			return FALSE;
		}

		res = mysql_store_result(conn);
		num_fields = mysql_num_fields(res);
		num_rows = mysql_num_rows(res);

		nFailCnt = 0;				// 점검 필요 수
		nSystemFailCnt = 0;		// 시스템 점검 필요 수

		if( num_rows > 0 )
		{
			while((row = mysql_fetch_row(res)) != NULL)
			{
				CString strRet = row[0];

				if( strRet == "점검필요" )
				{
					nFailCnt++;
				}
				else if( strRet == "시스템 점검 필요" || strRet == "" )
				{
					nSystemFailCnt++;
				}
			}

			// 점검 필요 를 시스템 점검 필요 보다 우선으로 함
			if( nFailCnt > 0 )		// 점검 필요
			{
				strAcc_Val = "점검필요";
			}
			else if( nSystemFailCnt > 0 )	// 시스템 점검 필요
			{
				//strAcc_Val = "시스템 점검 필요";
				strAcc_Val = "점검필요";
			}
			else	// 안전
			{
				strAcc_Val = "안전";
			}
		}
		else
		{
			//strAcc_Val = "실패(데이터 없음)";
			strAcc_Val = "점검필요";
		}

		nTotalRow += num_rows;

		mysql_free_result(res);

		if( nTotalRow < m_MakeEvaBD.m_nMinAvailChanCnt )	// 최소 평가 성분 갯수보다 작으면, 전체 점검필요 로 강제 설정)
		{
			strAcc_Val = "점검필요";
			strCav_Val = "점검필요";
		}
	}
	else if( m_strDitc == "D" )	// 필댐 및 저수지
	{
		// 손상지수 데이터 확인
		sSQL.Format("select result from %s_detail where %s = '%s' and sen_id like '%s_%s%%' and chan_id like 'H%%' and kind_val = 'S'", sTableName, sEventCol, m_sEventID, m_strNet, m_strSta);

		if (mysql_query(conn, (CStringA)sSQL)) {
			sLog.Format("%s : %s", "MySQL query Error", sSQL);
			WriteLog(ERROR_FILENAME, sLog);
			mysql_close(conn);
			return FALSE;
		}

		res = mysql_store_result(conn);
		num_fields = mysql_num_fields(res);
		num_rows = mysql_num_rows(res);

		int nFailCnt = 0;				// 점검 필요 수
		int nSystemFailCnt = 0;		// 시스템 점검 필요 수

		if( num_rows > 0 )
		{
			while((row = mysql_fetch_row(res)) != NULL)
			{
				CString strRet = row[0];

				if( strRet == "점검 필요" )
				{
					nFailCnt++;
				}
				else if( strRet == "시스템 점검 필요" || strRet == "" )
				{
					nSystemFailCnt++;
				}
			}

			// 점검 필요 를 시스템 점검 필요 보다 우선으로 함
			if( nFailCnt > 0 )		// 점검 필요
			{
				strStm_Val = "점검필요";
			}
			else if( nSystemFailCnt > 0 )	// 시스템 점검 필요
			{
				//strStm_Val = "시스템 점검 필요";
				strStm_Val = "점검필요";
			}
			else	// 안전
			{
				strStm_Val = "안전";
			}
		}
		else
		{
			//strStm_Val = "실패(데이터 없음)";
			strStm_Val = "점검필요";
		}

		nTotalRow += num_rows;

		mysql_free_result(res);

		if( nTotalRow < m_MakeEvaBD.m_nMinAvailChanCnt )	// 최소 평가 성분 갯수보다 작으면, 전체 점검필요 로 강제 설정)
		{
			strStm_Val = "점검필요";
		}
	}

	// 동일하지 않은 이벤트ID 이고, 동일한 net, obs_id 로 등록된 result 값을 result_bef 로 설정
	CString strResult_Bef = "";

	sSQL.Format("select result from %s where %s != '%s' and net = '%s' and obs_id = '%s' order by regdate desc" , sTableName, sEventCol, m_sEventID, m_strNet, m_strSta);

	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s : %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return FALSE;
	}

	res = mysql_use_result(conn);
	num_fields = mysql_num_fields(res);
	num_rows = mysql_num_rows(res);

	if( num_rows > 0 )
	{
		row = mysql_fetch_row(res);
		if (row == false)		strResult_Bef = "";
		else					strResult_Bef = row[0];
	}
	mysql_free_result(res);

	//------------------------------------DB UPDATE
    CString strQuery;
    if (iQuery == Insert)
    {
        strQuery.AppendFormat("INSERT INTO %s SET ", sTableName );
        strQuery.AppendFormat(" %s = '%s' ", sEventCol, m_sEventID);
        strQuery.AppendFormat(", net = '%s' ", m_strNet);
        strQuery.AppendFormat(", obs_id = '%s' ", m_strSta);
        strQuery.AppendFormat(", send_date = '%s' ", strDate);
        strQuery.AppendFormat(", regdate = '%s' ", strDate);
    }
    else if (iQuery == Update)
    {
        strQuery.AppendFormat("UPDATE %s SET ", sTableName );
		strQuery.AppendFormat(" updatedate = '%s' ", strDate);
    }

	// Acc_val, Sp_val, Dis_val, Cav_val, Stm_val 설정
	strQuery.AppendFormat(", acc_val = '%s' ", strAcc_Val);
	strQuery.AppendFormat(", sp_val  = '%s' ", strAcc_Spec_Val);
	strQuery.AppendFormat(", dis_val = '%s' ", strDis_Val);
	strQuery.AppendFormat(", cav_val = '%s' ", strCav_Val);
	strQuery.AppendFormat(", stm_val  = '%s' ", strStm_Val);
	
	// 관리자 정보 설정
	strQuery.AppendFormat(", user_dept = '%s' ", m_strUser_dept);
	strQuery.AppendFormat(", user_duty = '%s' ", m_strUser_duty);
	strQuery.AppendFormat(", user_name = '%s' ", m_strUser_name);
	strQuery.AppendFormat(", user_tel  = '%s' ", m_strUser_tel);
	
	// result_bef 설정
	strQuery.AppendFormat(", result_bef  = '%s' ", strResult_Bef);

	// 안전성 평가 종합 결과는 result.txt 의 total= 항목에서 가져와서 설정
	if( iResType == res_ok )
	{
		char sTotalResult[64];
		memset(sTotalResult, 0x00, sizeof(sTotalResult));

		char sResultFilePath[1024];
		memset(sResultFilePath, 0x00, sizeof(sResultFilePath));
		sprintf(sResultFilePath, "%s\\%s\\%s_%s_%s", m_sMseedFullPath, PATH_RESULT, m_strNet, m_strSta, BUILDEVA_RESULT);

		long nLen = 0;

		FILE *Result_file;
		Result_file = fopen(sResultFilePath, "r");
		if( Result_file == NULL )
		{
			sLog = "";
			sLog.Format("result file read failed!!");
			WriteReportLog(sLog);

			mysql_close(conn);
			return FALSE;
		}

		nLen = _filelength(fileno(Result_file));
		if( nLen <= 0 )
		{
			sLog = "";
			sLog.Format("result file data not found!! size = '%d'", nLen);
			WriteReportLog(sLog);

			fclose(Result_file);

			mysql_close(conn);
			return FALSE;
		}

		char sReadBuffer[4096];
		memset(sReadBuffer, 0x00, sizeof(sReadBuffer));

		fread(sReadBuffer, 1, nLen, Result_file);

		if( strlen(sReadBuffer) > 0 )
		{
			char* pTotalPos = strstr(sReadBuffer, "total=");
			char* pEndPos = NULL;

			if( pTotalPos != NULL )
			{
				pEndPos = strstr(pTotalPos, "\n");

				int nCopyDataLen = pEndPos - pTotalPos - strlen("total=");

				if( pEndPos != NULL && nCopyDataLen > 0 )
				{
					strncpy(sTotalResult, pTotalPos + strlen("total="), nCopyDataLen);
					sTotalResult[nCopyDataLen] = '\0';
				}
			}
		}

		if( strlen(sTotalResult) > 0 )
		{
			if( strAcc_Val == "점검필요" || strDis_Val == "점검필요" || strCav_Val == "점검필요" || strStm_Val == "점검필요" )
			{
				sprintf(sTotalResult, "%s", "점검필요");
			}

			strQuery.AppendFormat(", result  = '%s' ", sTotalResult);
		}
	}
	else
	{
		if( sError.GetLength() > 0 )
		{
			strQuery.AppendFormat(", result  = '%s' ", sError);
		}
		else
		{
			strQuery.Append(", result  = '계산실패(계측자료 오류)' ");
		}
	}

    if (iQuery == Update)
    {
        strQuery.AppendFormat(" WHERE net = '%s' ", m_strNet);
        strQuery.AppendFormat(" and obs_id = '%s' ", m_strSta);
        strQuery.AppendFormat(" and %s = '%s' ", sEventCol, m_sEventID);
    }

    if (mysql_query(conn, (CStringA)strQuery)) {
		sLog.Format("%s", "MySQL query Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		WriteLog(ERROR_FILENAME, strQuery);
		return FALSE;
	}                    
	mysql_close(conn);
    return TRUE;
}

// 2019.06.18 안전성 평가 상세 결과 저장
BOOL CBuildingEvalutionDlg::Insert_BuildingEvalutionResult_Detail(int iResType, char* sNet, char* sSta, CString sError, CString sTime)
{
	int	iQuery = 0;					// 0:insert 1:update
	TCHAR	szIniBuffer[256];		// ini결과데이터
	CString	sLog = "";
	//------------------------------------해당 EventID건이 있는지 체크(Update/Insert결정위함)
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int num_fields;
	CString	sTableName = "";
	CString	sSQL= "";
	CString	sEventCol = "";
	if(m_sEventID.GetLength() <= 10)	
	{
		sTableName = "kevtinfo_rpt2_detail";	// 지진
		sEventCol = "eq_no";
	}
	else		
	{
		sTableName = "hevtinfo_rpt2_detail";	// 자체
		sEventCol = "event_id";
	}
	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 
	{
		sLog.Format("%s", "MySQL Connect Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return FALSE;
	}
	mysql_set_character_set(conn, "euckr");

	CString strDate = "";
	
	if( sTime.GetLength() <= 0 )
	{
		strDate = GetDBSysTime();
		if( strDate.GetLength() <= 0 )
		{
			CTime t = CTime::GetCurrentTime();
			strDate.Format("%04d-%02d-%02d %02d:%02d:%02d",t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
		}
	}
	else
	{
		strDate = sTime;
	}

	// 2019.06.18 Calc_Result.txt 파일 읽어서 한줄씩 select 하여 처리되도록 추가
	char sCalcResultFilePath[1024];
	memset(sCalcResultFilePath, 0x00, sizeof(sCalcResultFilePath));
	sprintf(sCalcResultFilePath, "%s\\%s\\%s_%s_%s", m_sMseedFullPath, PATH_RESULT, sNet, sSta, CALC_RESULT);

	long nLen = 0;

	FILE *calcResult_file;
	calcResult_file = fopen(sCalcResultFilePath, "r");
	if( calcResult_file == NULL )
	{
		sLog = "";
		sLog.Format("Calculation result file read failed!!");
		WriteReportLog(sLog);

		mysql_close(conn);
		return FALSE;
	}

	nLen = _filelength(fileno(calcResult_file));
	if( nLen <= 0 )
	{
		sLog = "";
		sLog.Format("Calculation result file data not found!! size = '%d'", nLen);
		WriteReportLog(sLog);

		fclose(calcResult_file);

		mysql_close(conn);
		return FALSE;
	}

	char sReadBuffer[4096];
	memset(sReadBuffer, 0x00, sizeof(sReadBuffer));

	fread(sReadBuffer, 1, nLen, calcResult_file);
	char* pStartPos = NULL;
	int nReadBufferLen = 0;

	char sWriteBuffer[1024];
	char sValueKind[4];
	char sCalcValue[16];

	int nResultFaultCnt = 0;	// 점검 필요로 판단된 갯수
	// 2019.05.22 특수교량의 total 값에 사용하며, 전체 대상 센서 갯수 대비 점검 필요 갯수로 비교하거나, 점검 필요 갯수만으로 판단 - 미정 상태
	int nResultAvailCnt = 0;	// 2019.06.14 안전 또는 점검 필요로 판단된 총 갯수

	CAry* aryThStation = NULL;
	aryThStation = m_MakeEvaBD.GetAryThStation();

	if( aryThStation == NULL )
	{
		mysql_close(conn);
		return FALSE;
	}

	CString strQuery = "";

	if( m_strDitc == "S" || m_strDitc == "B" )	// 특수교량 은 계산된 값과 각 계측소 센서위치별 임계치와 비교
	{
		BOOL bChkAcc = FALSE;	// 가속도 데이터 체크 상태
		BOOL bChkSP = FALSE;	// 가속도 응답스펙트럼 데이터 체크 상태
		char sPrevChkSen_Id[16];	// 앞서 처리한 SensorID
		memset(sPrevChkSen_Id, 0x00, sizeof(sPrevChkSen_Id));

		// 임계치 저장 배열만큼 반복
		for(int j = 0; j < m_MakeEvaBD.GetThStationCount(); j++)
		{
			strQuery = "";

			if(strlen(sReadBuffer) <= 0) break;
			memset(sValueKind, 0x00, sizeof(sValueKind));
			nReadBufferLen = strlen(sReadBuffer);

			pStartPos = sReadBuffer;

			ThSta_t* pThSta = (ThSta_t*)aryThStation->GetAt(j);
			if( pThSta == NULL ) continue;

			// SensorID 값과 동일한 축 성분 데이터를 sWriteBuffer 에서 찾기
			char* pBuffer = strstr(pStartPos, pThSta->sSen_Id);
			if( pBuffer == NULL ) continue;

			char* pDataEndPoint = strstr(pBuffer, "\n");
			if( pDataEndPoint == NULL ) break;

			if ( strcmp(sPrevChkSen_Id, pThSta->sSen_Id) != 0 )
			{
				memset(sPrevChkSen_Id, 0x00, sizeof(sPrevChkSen_Id));
				memcpy(sPrevChkSen_Id, pThSta->sSen_Id, sizeof(sPrevChkSen_Id));
				bChkAcc = FALSE;
				bChkSP = FALSE;
			}

			// 필수축 성분의 계산된 값과 해당하는 성분의 임계치를 비교 (비교 대상 임계값이 없으면 '측정 불가')
			char* pValueKind = strstr(pBuffer, "\t");
			if( pValueKind == NULL ) continue;
			memset(sValueKind, 0x00, sizeof(sValueKind));
			strncpy(sValueKind, pValueKind + 1, 2);

			char* pCalcValue = strstr(pValueKind + 1, "\t");
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

			char sSen_Id[16];
			char sChan_Id[4];
			memset(sSen_Id, 0x00, sizeof(sSen_Id));
			memset(sChan_Id, 0x00, sizeof(sChan_Id));
			strncpy(sSen_Id, pThSta->sSen_Id, 6);			// ex) KC_WDA_HBX -> KC_WDA
			strncpy(sChan_Id, pThSta->sSen_Id + 7, 3);		// ex) KC_WDA_HBX -> HBX
			sSen_Id[6] = '\0';
			sChan_Id[3] = '\0';

			for( int k = 0; k < strlen(sValueKind); k++ )
			{
				if( ( (sValueKind[k] >= 0x41 && sValueKind[k] <= 0x5A) || (sValueKind[k] >= 0x61 && sValueKind[k] <= 0x7A) ) == false )
				{
					sValueKind[k] = 0x00;
				}
			}

			//int nChkAvailable = Available_BuildingEvalutionResult_Detail(m_sEventID, sSen_Id, sChan_Id, pThSta->sType);
			//double dblChkLastCalc = Last_BuildingEvalutionResult_Detail(m_sEventID, sSen_Id, sChan_Id, pThSta->sType);
			int nChkAvailable = Available_BuildingEvalutionResult_Detail(m_sEventID, sSen_Id, sChan_Id, sValueKind);
			double dblChkLastCalc = Last_BuildingEvalutionResult_Detail(m_sEventID, sSen_Id, sChan_Id, sValueKind);

			if( nChkAvailable == Insert )
			{
				strQuery.AppendFormat("INSERT INTO %s SET ", sTableName);
				strQuery.AppendFormat("%s = '%s' ", sEventCol, m_sEventID);
				strQuery.AppendFormat(", sen_id = '%s' ", sSen_Id);
				strQuery.AppendFormat(", chan_id = '%s' ", sChan_Id);
				//strQuery.AppendFormat(", kind_val = '%s' ", pThSta->sType);
				strQuery.AppendFormat(", kind_val = '%s' ", sValueKind);
				strQuery.AppendFormat(", calc_val = '%f' ", dblCalcValue);

				if( dblChkLastCalc != -999.0 )
				{
					strQuery.AppendFormat(", calc_val_bef = '%f' ", dblChkLastCalc);
				}

				strQuery.AppendFormat(", regdate = '%s' ", strDate);
			}
			else if( nChkAvailable == Update )
			{
				strQuery.AppendFormat("UPDATE %s SET ", sTableName);
				strQuery.AppendFormat("calc_val = '%f' ", dblCalcValue);

				if( dblChkLastCalc != -999.0 )
				{
					strQuery.AppendFormat(", calc_val_bef = '%f' ", dblChkLastCalc);
				}

				strQuery.AppendFormat(", updatedate = '%s' ", strDate);
			}
			else
			{
				continue;
			}

			if( pThStaDetail == NULL )
			{
//#ifdef _DEBUG
				if ( sValueKind[0] == 'S' && sValueKind[1] == 'P' )
				{
					int nCalcValue = (int)dblCalcValue;
					if( nCalcValue != 1 )
					{
						sprintf(sResultMsg, "%s", "점검필요");
						nResultFaultCnt++;
						nResultAvailCnt++;
					}
					else
					{
						sprintf(sResultMsg, "%s", "안전");
						nResultAvailCnt++;
					}
				}
				else
				{
					double dblThValue = 0.0;
					
					if( sChan_Id[2] == 'Z' ) dblThValue = 52;
					else						dblThValue = 67;

					if( dblCalcValue > dblThValue )	// 임계치 초과
					{
						sprintf(sResultMsg, "%s", "점검필요");
						nResultFaultCnt++;
						nResultAvailCnt++;
					}
					else
					{
						sprintf(sResultMsg, "%s", "안전");
						nResultAvailCnt++;
					}
				}

				strQuery.AppendFormat(", result = '%s' ", sResultMsg);
				strcat(sResultMsg, "\n");
//#else
//				sprintf(sResultMsg, "%s", "평가 불가");
//
//				strQuery.AppendFormat(", result = '%s' ", sResultMsg);
//				
//				strcat(sResultMsg, "\n");
//#endif	// DEBUG
			}
			else
			{
				if ( sValueKind[0] == 'S' && sValueKind[1] == 'P' )
				{
					int nThValue = (int)dblCalcValue;

					if( nThValue == 0 )
					{
						sprintf(sResultMsg, "%s", "점검필요");
						nResultFaultCnt++;
						nResultAvailCnt++;
					}
					else
					{
						sprintf(sResultMsg, "%s", "안전");
						nResultAvailCnt++;
					}
				}
				else
				{
					double dblThValue = atof(pThStaDetail->sMaxVal);

					if( dblCalcValue > dblThValue )	// 임계치 초과
					{
						sprintf(sResultMsg, "%s", "점검필요");
						nResultFaultCnt++;
						nResultAvailCnt++;
					}
					else
					{
						sprintf(sResultMsg, "%s", "안전");
						nResultAvailCnt++;
					}
				}

				strQuery.AppendFormat(", result = '%s' ", sResultMsg);
				strcat(sResultMsg, "\n");
			}

			// 관리자 정보 설정
			strQuery.AppendFormat(", user_dept = '%s' ", m_strUser_dept);
			strQuery.AppendFormat(", user_duty = '%s' ", m_strUser_duty);
			strQuery.AppendFormat(", user_name = '%s' ", m_strUser_name);
			strQuery.AppendFormat(", user_tel  = '%s' ", m_strUser_tel);

			if( nChkAvailable == Update )
			{
				for( int k = 0; k < strlen(sValueKind); k++ )
				{
					if( ( (sValueKind[k] >= 0x41 && sValueKind[k] <= 0x5A) || (sValueKind[k] >= 0x61 && sValueKind[k] <= 0x7A) ) == false )
					{
						sValueKind[k] = 0x00;
					}
				}

				strQuery.AppendFormat("where %s = '%s' ", sEventCol, m_sEventID);
				strQuery.AppendFormat("and sen_id = '%s' ", sSen_Id);
				strQuery.AppendFormat("and chan_id = '%s' ", sChan_Id);
				//strQuery.AppendFormat("and kind_val = '%s' ", pThSta->sType);
				strQuery.AppendFormat("and kind_val = '%s' ", sValueKind);
			}

			if (mysql_query(conn, (CStringA)strQuery)) {
				sLog.Format("%s", "MySQL query Error");
				WriteLog(ERROR_FILENAME, sLog);
				//mysql_close(conn);
				WriteLog(ERROR_FILENAME, strQuery);

				continue;
				//return FALSE;
			}

			// 가속도 또는 가속도 응답스펙트럼은 같은 성분의 데이터가 하나 더 존재하므로('A', 'SP') 한번 더 체크하도록 루프 변수 -1 처리
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

			// 처리된 필수축 성분의 데이터를 sWriteBuffer 변수에서 제거
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
	else if( m_strDitc == "K" )	// 콘크리트댐
	{
		// 임계치 저장 배열만큼 반복
		for(int j = 0; j < m_MakeEvaBD.GetThStationCount(); j++)
		{
			strQuery = "";

			ThSta_t* pThSta = (ThSta_t*)aryThStation->GetAt(j);
			if( pThSta == NULL ) continue;

			char sSenInfo[16];
			char* pReSearchStartPos = NULL;

			while (strlen(sReadBuffer) > 0)
			{
				if(strlen(sReadBuffer) <= 0) break;
				memset(sSenInfo, 0x00, sizeof(sSenInfo));
				memset(sValueKind, 0x00, sizeof(sValueKind));
				nReadBufferLen = strlen(sReadBuffer);

				pStartPos = sReadBuffer;

				// SensorID 값과 동일한 축 성분 데이터를 sWriteBuffer 에서 찾기
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

				// 필수축 성분의 계산된 값과 해당하는 성분의 임계치를 비교 (비교 대상 임계값이 없으면 '측정 불가')
				char* pValueKind = strstr(pBuffer, "\t");
				if( pValueKind == NULL ) continue;
				memset(sValueKind, 0x00, sizeof(sValueKind));
				strncpy(sValueKind, pValueKind + 1, 1);

				// 관리기준치의 Type 과 동일한 데이터를 먼저 처리
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
					int nSenIdSize = sizeof(sSenInfo) > pValueKind - pBuffer ? pValueKind - pBuffer : sizeof(sSenInfo);
					strncpy(sSenInfo, pBuffer, nSenIdSize);
				}

				int nCopySize = sizeof(sCalcValue) > pEndCalcValuePos - pCalcValue ? pEndCalcValuePos - pCalcValue : sizeof(sCalcValue);
				memset(sCalcValue, 0x00, sizeof(sCalcValue));
				strncpy(sCalcValue, pCalcValue, nCopySize);	

				double dblCalcValue = atof(sCalcValue);
				ThSta_Detail_t* pThStaDetail = (ThSta_Detail_t*)pThSta->pAryThSta_Detail->GetAt(0);

				char sResultMsg[16];
				memset(sResultMsg, 0x00, sizeof(sResultMsg));

				char sSen_Id[16];
				char sChan_Id[4];
				memset(sSen_Id, 0x00, sizeof(sSen_Id));
				memset(sChan_Id, 0x00, sizeof(sChan_Id));
				strncpy(sSen_Id, pThSta->sSen_Id, 6);			// ex) KC_WDA_HBX -> KC_WDA
				strncpy(sChan_Id, pThSta->sSen_Id + 7, 3);		// ex) KC_WDA_HBX -> HBX
				sSen_Id[6] = '\0';
				sChan_Id[3] = '\0';

				int nChkAvailable = Available_BuildingEvalutionResult_Detail(m_sEventID, sSen_Id, sChan_Id, sValueKind);
				double dblChkLastCalc = Last_BuildingEvalutionResult_Detail(m_sEventID, sSen_Id, sChan_Id, sValueKind);

				if( nChkAvailable == Insert )
				{
					strQuery.AppendFormat("INSERT INTO %s SET ", sTableName);
					strQuery.AppendFormat("%s = '%s' ", sEventCol, m_sEventID);
					strQuery.AppendFormat(", sen_id = '%s' ", sSen_Id);
					strQuery.AppendFormat(", chan_id = '%s' ", sChan_Id);
					strQuery.AppendFormat(", kind_val = '%s' ", sValueKind);
					strQuery.AppendFormat(", calc_val = '%f' ", dblCalcValue);

					if( dblChkLastCalc != -999.0 )
					{
						strQuery.AppendFormat(", calc_val_bef = '%f' ", dblChkLastCalc);
					}

					strQuery.AppendFormat(", regdate = '%s' ", strDate);
				}
				else if( nChkAvailable == Update )
				{
					strQuery.AppendFormat("UPDATE %s SET ", sTableName);
					strQuery.AppendFormat("calc_val = '%f' ", dblCalcValue);

					if( dblChkLastCalc != -999.0 )
					{
						strQuery.AppendFormat(", calc_val_bef = '%f' ", dblChkLastCalc);
					}

					strQuery.AppendFormat(", updatedate = '%s' ", strDate);
				}
				else
				{
					continue;
				}

				if( pThStaDetail == NULL )
				{
					sprintf(sResultMsg, "%s", "평가 불가");

					strQuery.AppendFormat(", result = '%s' ", sResultMsg);
					strcat(sResultMsg, "\n");
				}
				else
				{
					double dblThValue = atof(pThStaDetail->sMaxVal);

					if( dblCalcValue > dblThValue )	// 임계치 초과
					{
						sprintf(sResultMsg, "%s", "점검필요");
						nResultFaultCnt++;
						nResultAvailCnt++;
					}
					else
					{
						sprintf(sResultMsg, "%s", "안전");
						nResultAvailCnt++;
					}

					strQuery.AppendFormat(", result = '%s' ", sResultMsg);
					strcat(sResultMsg, "\n");
				}

				// 관리자 정보 설정
				strQuery.AppendFormat(", user_dept = '%s' ", m_strUser_dept);
				strQuery.AppendFormat(", user_duty = '%s' ", m_strUser_duty);
				strQuery.AppendFormat(", user_name = '%s' ", m_strUser_name);
				strQuery.AppendFormat(", user_tel  = '%s' ", m_strUser_tel);

				if( nChkAvailable == Update )
				{
					strQuery.AppendFormat("where %s = '%s' ", sEventCol, m_sEventID);
					strQuery.AppendFormat("and sen_id = '%s' ", sSen_Id);
					strQuery.AppendFormat("and chan_id = '%s' ", sChan_Id);
					strQuery.AppendFormat("and kind_val = '%s' ", sValueKind);
				}

				if (mysql_query(conn, (CStringA)strQuery)) {
					sLog.Format("%s", "MySQL query Error");
					WriteLog(ERROR_FILENAME, sLog);
					//mysql_close(conn);
					WriteLog(ERROR_FILENAME, strQuery);

					continue;
					//return FALSE;
				}

				// 처리된 필수축 성분의 데이터를 sWriteBuffer 변수에서 제거
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

					//if( pStartPos )
					//{
					//	if( nReadBufferLen - (pDataEndPoint - pStartPos) <= 0 )	break;

					//	memcpy(sTmpFirstBuffer, pDataEndPoint + 1, nReadBufferLen - (pDataEndPoint - pStartPos));
					//}
					////else
					////{
					////	if( pDataEndPoint == (pStartPos + nReadBufferLen) )
					////	{
					////		memcpy(sTmpFirstBuffer, pStartPos, pBuffer - pStartPos);
					////	}
					////	else
					////	{
					////		char sTmpSecondBuffer[1024];
					////		memset(sTmpSecondBuffer, 0x00, sizeof(sTmpSecondBuffer));
					////		memcpy(sTmpSecondBuffer, pDataEndPoint + 1, nReadBufferLen - (pDataEndPoint - pStartPos));

					////		memcpy(sTmpFirstBuffer, pStartPos, pBuffer - pStartPos);
					////		strcat(sTmpFirstBuffer, sTmpSecondBuffer);
					////	}
					////}

					memset(sReadBuffer, 0x00, sizeof(sReadBuffer));
					memcpy(sReadBuffer, sTmpFirstBuffer, sizeof(sReadBuffer));
					nReadBufferLen = strlen(sReadBuffer);

					pReSearchStartPos = NULL;
				}
			}
		}
	}
	else if( m_strDitc == "D" )	// 필댐 및 저수지 는 경험식에 의해 산출된 손상지수를 기준으로 계산된 손상지수와 비교하여 모든 필댐 및 저수지 시설물에 대해 동일하게 안전성 평가
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

			char sSenInfo[16];
			memset(sSenInfo, 0x00, sizeof(sSenInfo));

			BOOL bDataSetYN = FALSE;
			int nChkAvailable = -1;

			char sSen_Id[16];
			char sChan_Id[4];
			memset(sSen_Id, 0x00, sizeof(sSen_Id));
			memset(sChan_Id, 0x00, sizeof(sChan_Id));

			strQuery = "";

			if( pValueKind != NULL && pDataEndPos != NULL && pValueKind > pStartPos  )
			{
				int nCopySize = sizeof(sSenInfo) > pValueKind - pStartPos ? pValueKind - pStartPos : sizeof(sSenInfo);
				strncpy(sSenInfo, pStartPos, nCopySize);

				memset(sValueKind, 0x00, sizeof(sValueKind));
				strncpy(sValueKind, pValueKind + 1, 1);

				if( sValueKind[0] == 'S' )
				{
					char* pSettlementPos = strstr(pValueKind + 1, "\t");
					if( pSettlementPos != NULL )
					{
						char sSettlement[16];
						memset(sSettlement, 0x00, sizeof(sSettlement));					

						nCopySize = sizeof(sSettlement) > pDataEndPos - pSettlementPos ? pDataEndPos - pSettlementPos : sizeof(sSettlement);
						strncpy(sSettlement, pSettlementPos + 1, nCopySize);

						double dblSettlement = atof(sSettlement);
						//dblSettlement = dblSettlement * 0.010;

						strncpy(sSen_Id, sSenInfo, 6);			// ex) KC_WDA_HBX -> KC_WDA
						strncpy(sChan_Id, sSenInfo + 7, 3);		// ex) KC_WDA_HBX -> HBX
						sSen_Id[6] = '\0';
						sChan_Id[3] = '\0';

						nChkAvailable = Available_BuildingEvalutionResult_Detail(m_sEventID, sSen_Id, sChan_Id, sValueKind);
						double dblChkLastCalc = Last_BuildingEvalutionResult_Detail(m_sEventID, sSen_Id, sChan_Id, sValueKind);

						if( nChkAvailable == Insert )
						{
							strQuery.AppendFormat("INSERT INTO %s SET ", sTableName);
							strQuery.AppendFormat("%s = '%s' ", sEventCol, m_sEventID);
							strQuery.AppendFormat(", sen_id = '%s' ", sSen_Id);
							strQuery.AppendFormat(", chan_id = '%s' ", sChan_Id);
							strQuery.AppendFormat(", kind_val = '%s' ", sValueKind);
							strQuery.AppendFormat(", calc_val = '%f' ", dblSettlement);

							if( dblChkLastCalc != -999.0 )
							{
								strQuery.AppendFormat(", calc_val_bef = '%f' ", dblChkLastCalc);
							}

							strQuery.AppendFormat(", regdate = '%s' ", strDate);
						}
						else if( nChkAvailable == Update )
						{
							strQuery.AppendFormat("UPDATE %s SET ", sTableName);
							strQuery.AppendFormat("calc_val = '%f' ", dblSettlement);

							if( dblChkLastCalc != -999.0 )
							{
								strQuery.AppendFormat(", calc_val_bef = '%f' ", dblChkLastCalc);
							}

							strQuery.AppendFormat(", updatedate = '%s' ", strDate);
						}

						if( nChkAvailable == Insert || nChkAvailable == Update )
						{
							if( dblSettlement >= 0)
							{
								// 2019.11.21 손상지수 0.4 에 상응하는 gal 값은 143 gal 이므로 차후 gal 값으로 비교하는 것으로 변경시 0.400 => 143 으로 변경하여 사용
								if( dblSettlement > 0.400 )	// 점검 필요
								{
									CString strResult = "점검필요";
									sprintf(sResultMsg, "%s\t%s\n", sSen_Id, strResult);
									nResultFaultCnt++;
									nResultAvailCnt++;

									strQuery.AppendFormat(", result = '%s' ", strResult);
								}
								else if( dblSettlement <= 0.400 )
								{
									CString strResult = "안전";
									sprintf(sResultMsg, "%s\t%s\n", sSen_Id, strResult);
									nResultAvailCnt++;

									strQuery.AppendFormat(", result = '%s' ", strResult);
								}						

								bDataSetYN = TRUE;
							}
						}
					}
				}			
			}

			if( bDataSetYN == FALSE )
			{
				CString strResult = "평가 불가";

				if( strlen(sSenInfo) <= 0)
				{
					sprintf(sResultMsg, "%s\t%s\n", "Unknown", strResult);
				}
				else
				{
					sprintf(sResultMsg, "%s\t%s\n", sSenInfo, strResult);
				}

				strQuery.AppendFormat(", result = '%s' ", strResult);
			}

			// 관리자 정보 설정
			strQuery.AppendFormat(", user_dept = '%s' ", m_strUser_dept);
			strQuery.AppendFormat(", user_duty = '%s' ", m_strUser_duty);
			strQuery.AppendFormat(", user_name = '%s' ", m_strUser_name);
			strQuery.AppendFormat(", user_tel  = '%s' ", m_strUser_tel);

			if( nChkAvailable == Update )
			{
				strQuery.AppendFormat("where %s = '%s' ", sEventCol, m_sEventID);
				strQuery.AppendFormat("and sen_id = '%s' ", sSen_Id);
				strQuery.AppendFormat("and chan_id = '%s' ", sChan_Id);
				strQuery.AppendFormat("and kind_val = '%s' ", sValueKind);
			}

			if (mysql_query(conn, (CStringA)strQuery)) {
				sLog.Format("%s", "MySQL query Error");
				WriteLog(ERROR_FILENAME, sLog);
				//mysql_close(conn);
				WriteLog(ERROR_FILENAME, strQuery);

				continue;
				//return FALSE;
			}

			// 처리된 필수축 성분의 데이터를 sWriteBuffer 변수에서 제거
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

	mysql_close(conn);
	return TRUE;
}

//************************************
// Method:    Available_BuildingEvalutionResult_Detail
// FullName:  CBuildingEvalutionDlg::Available_BuildingEvalutionResult_Detail
// Access:    protected 
// Returns:   int		-1 : ERROR
// Returns:			 0 : INSERT
// Returns:			 1 : UPDATE
// Qualifier:
// Parameter: CString strEventID
// Parameter: char * sSen_Id
// Parameter: char * sChan_Id
// Parameter: char * sDitc
//************************************
int CBuildingEvalutionDlg::Available_BuildingEvalutionResult_Detail(CString strEventID, char* sSen_Id, char* sChan_Id, char* sDitc)
{
	int nRet = -1;
	CString sLog = "";
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int num_fields;
	CString	sTableName = "";
	CString sSQL= "";
	CString	sEventCol = "";
	if(m_sEventID.GetLength() <= 10)	
	{
		sTableName = "kevtinfo_rpt2_detail";	// 지진
		sEventCol = "eq_no";
	}
	else		
	{
		sTableName = "hevtinfo_rpt2_detail";	// 자체
		sEventCol = "event_id";
	}
	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 
	{
		sLog.Format("%s", "MySQL Connect Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return nRet;
	}
	mysql_set_character_set(conn, "euckr");
	sSQL.Format("select %s from %s where %s='%s' and sen_id = '%s' and chan_id = '%s' and kind_val = '%s'" , sEventCol, sTableName, sEventCol, strEventID, sSen_Id, sChan_Id, sDitc);

	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s : %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return nRet;
	}

	res = mysql_use_result(conn);
	num_fields = mysql_num_fields(res);
	row = mysql_fetch_row(res);
	if (row == false)		nRet = Insert;
	else					nRet = Update;
	mysql_free_result(res);

	return nRet;
}

double CBuildingEvalutionDlg::Last_BuildingEvalutionResult_Detail(CString strEventID, char* sSen_Id, char* sChan_Id, char* sDitc)
{
	double dblRet = -999.0;
	CString sLog = "";
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int num_fields;
	int num_rows;
	CString	sTableName = "";
	CString sSQL= "";
	CString	sEventCol = "";
	if(m_sEventID.GetLength() <= 10)	
	{
		sTableName = "kevtinfo_rpt2_detail";	// 지진
		sEventCol = "eq_no";
	}
	else		
	{
		sTableName = "hevtinfo_rpt2_detail";	// 자체
		sEventCol = "event_id";
	}
	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, (CStringA)m_strDBIP, (CStringA)m_strDBID, (CStringA)m_strDBPW, (CStringA)m_strDBDB, m_iDBPORT, NULL, 0)) 
	{
		sLog.Format("%s", "MySQL Connect Error");
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return dblRet;
	}
	mysql_set_character_set(conn, "euckr");
	sSQL.Format("select calc_val_bef from %s where %s != '%s' and sen_id = '%s' and chan_id = '%s' and kind_val = '%s' order by regdate desc" , sTableName, sEventCol, strEventID, sSen_Id, sChan_Id, sDitc);

	if (mysql_query(conn, (CStringA)sSQL)) {
		sLog.Format("%s : %s", "MySQL query Error", sSQL);
		WriteLog(ERROR_FILENAME, sLog);
		mysql_close(conn);
		return dblRet;
	}

	res = mysql_use_result(conn);
	num_fields = mysql_num_fields(res);
	num_rows = mysql_num_rows(res);
	row = mysql_fetch_row(res);

	if( num_rows > 0 )
	{
		if (row == false)		
			dblRet = -999.0;			// -999면 해당 값 사용하지 않음
		else					
			dblRet = atof(row[0]);
	}
	
	mysql_free_result(res);

	return dblRet;
}

void CBuildingEvalutionDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}


void CBuildingEvalutionDlg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	Wait(1000);		// 1초 대기후 종료
	CDialogEx::OnCancel();
}

void CBuildingEvalutionDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( TMR_BUILDINGEVALUTION_START == nIDEvent )
	{
		// SetWindowText(m_pApp->m_strCaptionTitle);
		KillTimer(TMR_BUILDINGEVALUTION_START);

		StartBuildingEvalution();
	}
	else if( TMR_BUILDINGEVALUTION_END == nIDEvent )
	{
		KillTimer(TMR_BUILDINGEVALUTION_END);
//#ifndef _DEBUG
		OnCancel();
//#endif	// DEBUG
	}

	CDialogEx::OnTimer(nIDEvent);
}

BOOL CBuildingEvalutionDlg::DeleteDir(CString dir)
{
	return FALSE;

    if( dir == _T("") )
    {
        return FALSE;
    }
 
    BOOL		bRval			= FALSE;
    int			nRval			= 0;
    CString		szNextDirPath	= _T("");
    CString		szRoot			= _T("");
    CFileFind	find;
 
    // Directory가 존재 하는지 확인 검사
    bRval = find.FindFile(dir);
 
    if( bRval == FALSE )
    {
        return bRval;
    }
 
    while( bRval )
    {
        bRval = find.FindNextFile();
 
        // . or .. 인 경우 무시한다.
        if( find.IsDots() == TRUE )
        {
            continue;
        }
 
        // Directory 일 경우
        if( find.IsDirectory() )
        {
            szNextDirPath.Format(_T("%s\\*.*"), find.GetFilePath());
 
            // Recursion function 호출
            DeleteDir(szNextDirPath);
        }
 
        // file일 경우
        else
        {
            //파일 삭제
            ::DeleteFile(find.GetFilePath());
        }
    }
 
    szRoot = find.GetRoot();
    find.Close();
 
    Sleep(1);
    bRval = RemoveDirectory(szRoot);
 
    return bRval;
}

CString CBuildingEvalutionDlg::GetPhysicalFactor(CString sResponse, CString sSensitivity)
{
	CString strRet = "";
	float fPhysicalFactor = 0.0;
	float fSensitivity = _ttof(sSensitivity);
	float fResponse = _ttof(sResponse);

	if(fSensitivity != 0.0 && fResponse != 0.0)
	{
		fPhysicalFactor = ((fSensitivity * pow(10.0, -6)) / fResponse / 9.81) * 981;
		strRet.Format("%f", fPhysicalFactor);
	}
	else
	{
		strRet.Format("0.000000");
	}

	return strRet;
}

void CBuildingEvalutionDlg::WriteReportLog(CString strLog)
{
	if( pIniFunc->m_iLogLevel == 0 )
	{
		return;
	}

	CString strNote = "";
	strNote.Format("(%s) %s", m_sEventID, strLog);

	char*	szlogPtr;
	char	szReportLogFile[1024];

	strcpy(szReportLogFile, "BuildingEvalution_v2.log");

	szlogPtr = strNote.GetBuffer(strNote.GetLength());
	pLogFunc->WriteLog(szReportLogFile, szlogPtr);

	strNote.ReleaseBuffer();
	strLog.ReleaseBuffer();
}

int CBuildingEvalutionDlg::WideCharToMultiByte_Len(CString sInString)
{
	int nRet = 0;
	nRet = WideCharToMultiByte(CP_ACP, 0, CT2W(sInString), -1, NULL, 0, NULL, NULL);

	if(nRet > 0)
	{
		nRet++;
	}

	return nRet;
}

void CBuildingEvalutionDlg::WideCharToMultiByte_Str(CString sInString, char* sRetBuffer, int nRetBufferLen)
{
	WideCharToMultiByte(CP_ACP, 0, CT2W(sInString), -1, sRetBuffer, nRetBufferLen, NULL, NULL);
	return;
}

void CBuildingEvalutionDlg::ShowTransLog(CString strLogMsg, int iWriteFile /* = 0*/ )
{
	CString	strAddLog;
	COleDateTime dateTime;

	dateTime = COleDateTime::GetCurrentTime();

	strAddLog.Format("%04d-%02d-%02d %02d:%02d:%02d : %s"
		, dateTime.GetYear()
		, dateTime.GetMonth()
		, dateTime.GetDay()
		, dateTime.GetHour()
		, dateTime.GetMinute()
		, dateTime.GetSecond()
		, strLogMsg);

	TRACE("\n");
	TRACE(strAddLog);

	switch( iWriteFile )
	{
	case 2 :
		WriteReportLog(strLogMsg);
		return;

	case 1 :
		WriteReportLog(strAddLog);

	case 0 :
		break;
	}


	m_listTransLog.InsertString(0, strAddLog);

	CDC *pDC = m_listTransLog.GetDC();
	if( pDC )
	{
		//취득한 리스트박스의 디바이스컨텍스트를 이용하여 추가하려는 스트링의 픽셀 폭을 취득
		//DC에 선택되어져 있는 폰트의 종류, 사이즈, 속성에 따라 픽셀폭이 달라짐
		int nTextWidth = pDC->GetTextExtent(strAddLog).cx;

		//추가하려는 문자의 픽셀폭이 현재 리스트박스에 들어 있는 다른 스트링의 최대폭보다 크다면
		if( nTextWidth > m_nListAEMaxWidth )
		{
			// 리스트박스의 최대 픽셀폭 저장 값을 갱신
			m_nListAEMaxWidth = nTextWidth;
			// 리스트박스의 스크롤 폭을 갱신
			m_listTransLog.SetHorizontalExtent(m_nListAEMaxWidth);
		}
	}
	m_listTransLog.ReleaseDC(pDC);

	//AfxGetApp()->m_pMainWnd->Invalidate();
	//m_pMain->Invalidate(FALSE);

	int	iCnt = m_listTransLog.GetCount();

	if( DEF_MAX_LOG_COUNT+100 >= iCnt )
	{
		return;
	}

	for( int i=iCnt-1; i>=DEF_MAX_LOG_COUNT; i-- )
	{
		m_listTransLog.DeleteString(i);
	}
}

afx_msg LRESULT CBuildingEvalutionDlg::OnUmShowtranslog(WPARAM wParam, LPARAM lParam)
{
	char *pMsg = (char*)wParam;
	//sprintf(sMsg, "%s", (char*)wParam);

	if( pMsg == NULL ) return 0;
	
	ShowTransLog(pMsg);

	delete [] pMsg;

	return 0;
}
