// DataExchange.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "DataExchange.h"
#include "GetData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CDataExchangeApp

BEGIN_MESSAGE_MAP(CDataExchangeApp, CWinApp)
END_MESSAGE_MAP()


// CDataExchangeApp construction

CDataExchangeApp::CDataExchangeApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDataExchangeApp object

CDataExchangeApp theApp;


// CDataExchangeApp initialization

BOOL CDataExchangeApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


extern "C" __declspec(dllexport) IThreadUnit *GetIThreadObject(T_GLOBAL_DATA *pGlobalData)
{
	g_pGlobalData = pGlobalData;
	IThreadUnit *pThread = new CGetData();
	//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("创建DataExchange对象！安装目录：%s"), g_pGlobalData->dir.GetInstallDir());

	return pThread;
}