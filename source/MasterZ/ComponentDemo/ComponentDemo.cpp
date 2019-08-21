// ComponentDemo.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "ComponentDemo.h"

#include "Demo.h"
#include "..\..\threadmodel\IThreadUnit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

T_GLOBAL_DATA *g_pGlobalData = NULL;
CLogTrace g_log(_T("demo.log"), NULL);

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

// CComponentDemoApp

BEGIN_MESSAGE_MAP(CComponentDemoApp, CWinApp)
END_MESSAGE_MAP()


// CComponentDemoApp 构造

CComponentDemoApp::CComponentDemoApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CComponentDemoApp 对象

CComponentDemoApp theApp;


// CComponentDemoApp 初始化

BOOL CComponentDemoApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

extern "C" __declspec(dllexport) IThreadUnit *CreateIThread(T_GLOBAL_DATA *pGlobalData)
{
	g_pGlobalData = pGlobalData;
	IThreadUnit *pThread = new CDemo();
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("创建ComponentDemo对象！安装目录：%s"), g_pGlobalData->dir.GetInstallDir());

	return pThread;
}