
// Havana.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CHavanaApp:
// �� Ŭ������ ������ ���ؼ��� Havana.cpp�� �����Ͻʽÿ�.
//

class CHavanaApp : public CWinApp
{
public:
	CHavanaApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CHavanaApp theApp;