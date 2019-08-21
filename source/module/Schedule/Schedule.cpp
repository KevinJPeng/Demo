// Schedule.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "Schedule.h"

#include "..\..\threadmodel\IThreadUnit.h"
#include "ScheduleThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLogTrace g_log(_T("Schedule.log"), NULL);
T_GLOBAL_DATA *g_pGlobalData = NULL;

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CScheduleApp

BEGIN_MESSAGE_MAP(CScheduleApp, CWinApp)
END_MESSAGE_MAP()


// CScheduleApp 构造

CScheduleApp::CScheduleApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CScheduleApp 对象

CScheduleApp theApp;


// CScheduleApp 初始化

BOOL CScheduleApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


extern "C" _declspec(dllexport) IThreadUnit *GetIThreadObject(T_GLOBAL_DATA *pGlobalData)
{
	g_pGlobalData = pGlobalData;
	IThreadUnit *pThread = new CScheduleThread();
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("创建WebSearch对象！安装目录：%s"), g_pGlobalData->dir.GetMcProcPath());

	return pThread;
}
