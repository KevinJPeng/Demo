
// EngineValidation.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CEngineValidationApp: 
// �йش����ʵ�֣������ EngineValidation.cpp
//

class CEngineValidationApp : public CWinApp
{
public:
	CEngineValidationApp();

// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CEngineValidationApp theApp;