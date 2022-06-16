
// BuildingEvalutionDlg.h : 헤더 파일
//
#include <direct.h>
#include <io.h>
#include <afxconv.h>

#include "locale.h"
#include "XLEzAutomation.h"
#include "../include/mySqlClass.h"
#include "../include/libmseed.h"
#include "../include/define.h"
#include "../Lib/Ary.h"
#include "MakeEvalutionBaseData.h"

#include "../include/iniFunc.h"
#include "../include/LogFunc.h"
//#include "../include/DataExtract.h"

//#include "../Lib/common.h"

 
#pragma once

// CBuildingEvalutionDlg 대화 상자
class CBuildingEvalutionDlg : public CDialogEx
{
// 생성입니다.
public:
	CBuildingEvalutionDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	~CBuildingEvalutionDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_BUILDINGEVALUTION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

public:
#ifdef	DEF_MULTI_SITE_SUPPORT
	CString		strCmdLine;
	void		setNetSta(char* strNet, char* strObsID);
#else
	void		setNetSta();		// 해당 이벤트 아이디의 net, obs_id조회	
#endif
		
	void		setDitc(CString sNet, CString sSta, CString &sDitc);
	int			MakeStationInfoTxt(CString sNet, CString sSta);
	int			MakeThresholdsInfoTxt(CString sNet, CString sSta);

	CString		m_sEventID;
	void		iniIniFile();		// 향후 db에서 읽는것으로 처리 수정
	BOOL		DeleteDir(CString dir);
	CString	GetPhysicalFactor(CString sResponse, CString sSensitivity);

	CString	m_sMseedBaseDir;		// miniSEED 구간자료의 기본 디렉토리, 다중 이벤트에 대해 멀티스레딩할 경우, 해당 경로 하위에 이벤트별 경로를 붙여서 사용
	CString	m_sExecProcKind;		// 가통 소프트웨어인지 구분명 (가통이면 'KV')
	CMakeEvalutionBaseData	m_MakeEvaBD;

	// 2019.03.25 시설물 종류
	CString	m_strDitc;			// 안전성평가 대상 시설물 종류(D : 필댐 및 저수지, B : 교량(현수교), S : 교량(사장교), K : 콘크리트댐)

	CIniFunc*			pIniFunc;
	CLogFunc*		pLogFunc;

	// 2019.06.19
	char			m_sMseedFullPath[2048];
	void			WriteReportLog(CString strLog);

	int				WideCharToMultiByte_Len(CString sInString);
	void			WideCharToMultiByte_Str(CString sInString, char* sRetBuffer, int nRetBufferLen);

	int				GetNetCodeLength() { return m_strNet.GetLength(); };
	int				GetStaCodeLength() { return m_strSta.GetLength(); };

	// 2019.07.30 detail 테이블에 먼저 처리되어 등록된 동일한 이벤트ID, 동일한 net, obs_id 로 등록된 기준별 result 값을 체크하여, acc_val, dis_val, cav_val, stm_val 값 설정
	CString		m_strAcc_Val;
	CString		m_strDis_Val;
	CString		m_strCav_Val;
	CString		m_strStm_Val;
	CString		m_strTot_Result;

	// 2019.09.03 구간데이터 변환 산출 데이터 삭제 여부
	CString		m_strRemoveAnalDataYN;

// 구현입니다.
//protected:
	HICON m_hIcon;

	CString		m_sMseedDir;		// miniSeed디렉토리

	void		MakeReportFile05_v2();
	BOOL		Insert_BuildingEvalutionResult(int iResType, CString sError = "", CString sTime = "");
	BOOL		Insert_BuildingEvalutionResult_Detail(int iResType, char* sNet, char* sSta, CString sError = "", CString sTime = "");
	int			Available_BuildingEvalutionResult_Detail(CString strEventID, char* sSen_Id, char* sChan_Id, char* sDitc);	// 2019.06.19 동일한 이벤트에 대한 동일 센서, 채널 데이터 등록 여부 확인
	double	Last_BuildingEvalutionResult_Detail(CString strEventID, char* sSen_Id, char* sChan_Id, char* sDitc);	// 2019.06.19 동일하지 않은 이벤트에 대한 동일 센서, 채널의 이전 최종 데이터 확인
	BOOL		ReadConfig();

	CString	GetDBSysTime();		// 2019.06.19 DB 시스템 시간 가져오기

	void		Wait(DWORD dwMillisecond);

	//  log 쓰기
	int			m_iLogLevel;
	CString	m_strLogDataDefDir;
	void		WriteLog(CString pFile, CString sLog);
	CString	genLogFileFullName(CString, char* cpDate);
	int			MakeQscdDir(CString szInitDir, int szYYYY, int szMM, int szDD);

	// DB사용위함(ini에서 읽어오기)
	CString		m_strDBIP;
	int				m_iDBPORT;
	CString		m_strDBID;
	CString		m_strDBPW;
	CString		m_strDBDB;
	CString		m_strNet;			// ex SL
	CString		m_strSta;			// ex KM
	CString		m_strReportDefDir;	// 리포트 저장 위치

	void		GetUserDeptInfo();
    CString		m_strUser_dept;
    CString		m_strUser_duty;
    CString		m_strUser_name;
    CString		m_strUser_tel;	// user_mobile, user_email(sms_yn/email_yn)


	// 이전 결과 정보 실행전 저장
	CString		m_strBackMajorNaturalFrequency;
	CString		m_strBackMinorNaturalFrequency;

	BOOL m_visible;  //플레그로 사용할 변수를 선언

	void StartBuildingEvalution();

	DWORD		m_dwCreateBldEvaThreadID;
	HANDLE		m_hCreateBldEvaThread;

	CListBox		m_listTransLog;
	int				m_nListAEMaxWidth;
	void			ShowTransLog(CString strLogMsg, int iWriteFile = 0);
	
	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnBnClickedOk();

	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnDestroy();
	virtual void OnCancel();	
protected:
	afx_msg LRESULT OnUmShowtranslog(WPARAM wParam, LPARAM lParam);
};
