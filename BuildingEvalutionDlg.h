
// BuildingEvalutionDlg.h : ��� ����
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

// CBuildingEvalutionDlg ��ȭ ����
class CBuildingEvalutionDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CBuildingEvalutionDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.
	~CBuildingEvalutionDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_BUILDINGEVALUTION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

public:
#ifdef	DEF_MULTI_SITE_SUPPORT
	CString		strCmdLine;
	void		setNetSta(char* strNet, char* strObsID);
#else
	void		setNetSta();		// �ش� �̺�Ʈ ���̵��� net, obs_id��ȸ	
#endif
		
	void		setDitc(CString sNet, CString sSta, CString &sDitc);
	int			MakeStationInfoTxt(CString sNet, CString sSta);
	int			MakeThresholdsInfoTxt(CString sNet, CString sSta);

	CString		m_sEventID;
	void		iniIniFile();		// ���� db���� �д°����� ó�� ����
	BOOL		DeleteDir(CString dir);
	CString	GetPhysicalFactor(CString sResponse, CString sSensitivity);

	CString	m_sMseedBaseDir;		// miniSEED �����ڷ��� �⺻ ���丮, ���� �̺�Ʈ�� ���� ��Ƽ�������� ���, �ش� ��� ������ �̺�Ʈ�� ��θ� �ٿ��� ���
	CString	m_sExecProcKind;		// ���� ����Ʈ�������� ���и� (�����̸� 'KV')
	CMakeEvalutionBaseData	m_MakeEvaBD;

	// 2019.03.25 �ü��� ����
	CString	m_strDitc;			// �������� ��� �ü��� ����(D : �ʴ� �� ������, B : ����(������), S : ����(���屳), K : ��ũ��Ʈ��)

	CIniFunc*			pIniFunc;
	CLogFunc*		pLogFunc;

	// 2019.06.19
	char			m_sMseedFullPath[2048];
	void			WriteReportLog(CString strLog);

	int				WideCharToMultiByte_Len(CString sInString);
	void			WideCharToMultiByte_Str(CString sInString, char* sRetBuffer, int nRetBufferLen);

	int				GetNetCodeLength() { return m_strNet.GetLength(); };
	int				GetStaCodeLength() { return m_strSta.GetLength(); };

	// 2019.07.30 detail ���̺� ���� ó���Ǿ� ��ϵ� ������ �̺�ƮID, ������ net, obs_id �� ��ϵ� ���غ� result ���� üũ�Ͽ�, acc_val, dis_val, cav_val, stm_val �� ����
	CString		m_strAcc_Val;
	CString		m_strDis_Val;
	CString		m_strCav_Val;
	CString		m_strStm_Val;
	CString		m_strTot_Result;

	// 2019.09.03 ���������� ��ȯ ���� ������ ���� ����
	CString		m_strRemoveAnalDataYN;

// �����Դϴ�.
//protected:
	HICON m_hIcon;

	CString		m_sMseedDir;		// miniSeed���丮

	void		MakeReportFile05_v2();
	BOOL		Insert_BuildingEvalutionResult(int iResType, CString sError = "", CString sTime = "");
	BOOL		Insert_BuildingEvalutionResult_Detail(int iResType, char* sNet, char* sSta, CString sError = "", CString sTime = "");
	int			Available_BuildingEvalutionResult_Detail(CString strEventID, char* sSen_Id, char* sChan_Id, char* sDitc);	// 2019.06.19 ������ �̺�Ʈ�� ���� ���� ����, ä�� ������ ��� ���� Ȯ��
	double	Last_BuildingEvalutionResult_Detail(CString strEventID, char* sSen_Id, char* sChan_Id, char* sDitc);	// 2019.06.19 �������� ���� �̺�Ʈ�� ���� ���� ����, ä���� ���� ���� ������ Ȯ��
	BOOL		ReadConfig();

	CString	GetDBSysTime();		// 2019.06.19 DB �ý��� �ð� ��������

	void		Wait(DWORD dwMillisecond);

	//  log ����
	int			m_iLogLevel;
	CString	m_strLogDataDefDir;
	void		WriteLog(CString pFile, CString sLog);
	CString	genLogFileFullName(CString, char* cpDate);
	int			MakeQscdDir(CString szInitDir, int szYYYY, int szMM, int szDD);

	// DB�������(ini���� �о����)
	CString		m_strDBIP;
	int				m_iDBPORT;
	CString		m_strDBID;
	CString		m_strDBPW;
	CString		m_strDBDB;
	CString		m_strNet;			// ex SL
	CString		m_strSta;			// ex KM
	CString		m_strReportDefDir;	// ����Ʈ ���� ��ġ

	void		GetUserDeptInfo();
    CString		m_strUser_dept;
    CString		m_strUser_duty;
    CString		m_strUser_name;
    CString		m_strUser_tel;	// user_mobile, user_email(sms_yn/email_yn)


	// ���� ��� ���� ������ ����
	CString		m_strBackMajorNaturalFrequency;
	CString		m_strBackMinorNaturalFrequency;

	BOOL m_visible;  //�÷��׷� ����� ������ ����

	void StartBuildingEvalution();

	DWORD		m_dwCreateBldEvaThreadID;
	HANDLE		m_hCreateBldEvaThread;

	CListBox		m_listTransLog;
	int				m_nListAEMaxWidth;
	void			ShowTransLog(CString strLogMsg, int iWriteFile = 0);
	
	// ������ �޽��� �� �Լ�
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
