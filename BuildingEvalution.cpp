
// BuildingEvalution.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
//

//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "stdafx.h"
#include "BuildingEvalution.h"
#include "BuildingEvalutionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBuildingEvalutionApp

BEGIN_MESSAGE_MAP(CBuildingEvalutionApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CBuildingEvalutionApp ����

CBuildingEvalutionApp::CBuildingEvalutionApp()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// �ٽ� ���� ������ ����
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
}


// ������ CBuildingEvalutionApp ��ü�Դϴ�.

CBuildingEvalutionApp theApp;


// CBuildingEvalutionApp �ʱ�ȭ

BOOL CBuildingEvalutionApp::InitInstance()
{
	//CString	strMsg2;
	//strMsg2.Format(_T("[%s] [%s] [%s]"), __argv[1], __argv[2], __argv[3]);
	//AfxMessageBox(strMsg2);


	// ���� ���α׷� �Ŵ��佺Ʈ�� ComCtl32.dll ���� 6 �̻��� ����Ͽ� ���־� ��Ÿ����
	// ����ϵ��� �����ϴ� ���, Windows XP �󿡼� �ݵ�� InitCommonControlsEx()�� �ʿ��մϴ�.
	// InitCommonControlsEx()�� ������� ������ â�� ���� �� �����ϴ�.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ���� ���α׷����� ����� ��� ���� ��Ʈ�� Ŭ������ �����ϵ���
	// �� �׸��� �����Ͻʽÿ�.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// ��ȭ ���ڿ� �� Ʈ�� �� �Ǵ�
	// �� ��� �� ��Ʈ���� ���ԵǾ� �ִ� ��� �� �����ڸ� ����ϴ�.
	CShellManager *pShellManager = new CShellManager;

	// ǥ�� �ʱ�ȭ
	// �̵� ����� ������� �ʰ� ���� ���� ������ ũ�⸦ ���̷���
	// �Ʒ����� �ʿ� ���� Ư�� �ʱ�ȭ
	// ��ƾ�� �����ؾ� �մϴ�.
	// �ش� ������ ����� ������Ʈ�� Ű�� �����Ͻʽÿ�.
	// TODO: �� ���ڿ��� ȸ�� �Ǵ� ������ �̸��� ����
	// ������ �������� �����ؾ� �մϴ�.
	// SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));

	CBuildingEvalutionDlg dlg;
	m_pMainWnd = &dlg;
	//                [0]     [1]   [2] [3]
	// command line : aaa.exe evtId Net ObsID

//#ifdef	DEF_MULTI_SITE_SUPPORT
//
//	char	szNet[10];
//	char	szObsID[10];
//
//	strcpy(szNet, __argv[2]);
//	strcpy(szObsID, __argv[3]);
//
//	dlg.setNetSta(szNet, szObsID);
//
//	dlg.m_sEventID = __argv[1];
//#endif
//	
//	dlg.m_sEventID = __argv[1];

	//CString	strMsg;
	//strMsg.Format(_T("[%s] [%s] [%s]"), dlg.m_sEventID, szNet, szObsID);
	//AfxMessageBox(strMsg);

	if(dlg.m_sEventID.GetLength() <= 0 || dlg.GetNetCodeLength() <= 0 || dlg.GetStaCodeLength() <= 0 || dlg.m_strDitc.GetLength() <= 0 || dlg.m_sMseedBaseDir.GetLength() <= 0)
	{
		AfxMessageBox("���� ���ڰ��� �����մϴ�.");
		m_pMainWnd = NULL;
	}
	else
	{
		if( dlg.m_sExecProcKind == "KV" )
		{
			dlg.iniIniFile();
		}
	
		INT_PTR nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: ���⿡ [Ȯ��]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
			//  �ڵ带 ��ġ�մϴ�.
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: ���⿡ [���]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
			//  �ڵ带 ��ġ�մϴ�.
		}
	}

	// ������ ���� �� �����ڸ� �����մϴ�.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}
	
	// ��ȭ ���ڰ� �������Ƿ� ���� ���α׷��� �޽��� ������ �������� �ʰ�  ���� ���α׷��� ���� �� �ֵ��� FALSE��
	// ��ȯ�մϴ�.
	return FALSE;
}

