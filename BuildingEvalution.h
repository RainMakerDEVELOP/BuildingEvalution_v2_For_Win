
// BuildingEvalution.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CBuildingEvalutionApp:
// �� Ŭ������ ������ ���ؼ��� BuildingEvalution.cpp�� �����Ͻʽÿ�.
//

class CBuildingEvalutionApp : public CWinApp
{
public:
	CBuildingEvalutionApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CBuildingEvalutionApp theApp;