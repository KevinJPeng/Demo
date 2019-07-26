// BCTool.cpp : ����Ӧ�ó������ڵ㡣
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
	ILog4zManager::getRef().setLoggerLimitsize(g_utilityVar.loggerId, 3);	//���Ƶ�����־�ļ��Ĵ�С����λM
	ILog4zManager::getRef().setLoggerLevel(g_utilityVar.loggerId, LOG_LEVEL_TRACE);
	ILog4zManager::getRef().start();

	LOG_INFO(g_utilityVar.loggerId, "*** *** " << "��������" << " *** ***");
	HANDLE hMutex = CreateMutex(NULL, FALSE, _T("_BCTOOL_CLIENT_GUI_"));
	if (hMutex && ERROR_ALREADY_EXISTS == GetLastError())
	{
		//��ʾ������
// 		HWND hwnd = ::FindWindow(NULL, _T("_BCTOOL_GUI_"));
// 		if (hwnd != NULL)
// 		{
// 			LOG_INFO(g_utilityVar.loggerId, "��⵽����ʵ��������");
// 			::SendMessage(hwnd, USERMSG_SINGLE_RUN, 0, 0);
// 		}
		return 0;
	}

//	g_utilityVar.sqlIte.InitDb();
	CPaintManagerUI::SetInstance(hInstance);
	HRESULT Hr = ::CoInitialize(NULL);
	//���ó���Ĭ��ͼ��
	CIndexWnd mainWnd;
	mainWnd.Create(NULL, _T("_BCTOOL_GUI_"), WS_OVERLAPPEDWINDOW, WS_EX_WINDOWEDGE);
	mainWnd.SetIcon(IDI_BCTOOL_NEW);
	mainWnd.CenterWindow();
	mainWnd.ShowWindow(true);
	CPaintManagerUI::MessageLoop();
	//::CoUninitialize();
	ILog4zManager::getRef().stop();

	LOG_INFO(g_utilityVar.loggerId, "*** *** " << "�����˳�" << " *** ***");

	return 0;
}



