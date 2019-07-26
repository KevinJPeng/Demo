// BCTool.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "BCTool.h"
#include "indexWnd.h"
//#include "UserDef.h"


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	//https://github.com/zsummer/log4z
	ILog4zManager::getRef().enableLogger(LOG4Z_MAIN_LOGGER_ID, false);
	g_utilityVar.loggerId = ILog4zManager::getRef().createLogger("BCToolRun");
	ILog4zManager::getRef().setLoggerPath(g_utilityVar.loggerId, "./log");
	ILog4zManager::getRef().setLoggerLimitsize(g_utilityVar.loggerId, 3);	//限制单个日志文件的大小，单位M
	ILog4zManager::getRef().setLoggerLevel(g_utilityVar.loggerId, LOG_LEVEL_TRACE);
	ILog4zManager::getRef().start();

	LOG_INFO(g_utilityVar.loggerId, "*** *** " << "程序启动" << " *** ***");
	HANDLE hMutex = CreateMutex(NULL, FALSE, _T("_BCTOOL_CLIENT_GUI_"));
	if (hMutex && ERROR_ALREADY_EXISTS == GetLastError())
	{
		//显示主界面
// 		HWND hwnd = ::FindWindow(NULL, _T("_BCTOOL_GUI_"));
// 		if (hwnd != NULL)
// 		{
// 			LOG_INFO(g_utilityVar.loggerId, "检测到已有实例在运行");
// 			::SendMessage(hwnd, USERMSG_SINGLE_RUN, 0, 0);
// 		}
		return 0;
	}

//	g_utilityVar.sqlIte.InitDb();
	CPaintManagerUI::SetInstance(hInstance);
	HRESULT Hr = ::CoInitialize(NULL);
	//设置程序默认图标
	CIndexWnd mainWnd;
	mainWnd.Create(NULL, _T("_BCTOOL_GUI_"), WS_OVERLAPPEDWINDOW, WS_EX_WINDOWEDGE);
	mainWnd.SetIcon(IDI_BCTOOL_NEW);
	mainWnd.CenterWindow();
	mainWnd.ShowWindow(true);
	CPaintManagerUI::MessageLoop();
	//::CoUninitialize();
	ILog4zManager::getRef().stop();

	LOG_INFO(g_utilityVar.loggerId, "*** *** " << "程序退出" << " *** ***");

	return 0;
}



