// MagicBox.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "MagicBox.h"
#include "IndexWnd.h"
#include "UserDef.h"


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	//https://github.com/zsummer/log4z
	ILog4zManager::getRef().enableLogger(LOG4Z_MAIN_LOGGER_ID, false);
	g_logger = ILog4zManager::getRef().createLogger("ShellRun");
	ILog4zManager::getRef().setLoggerPath(g_logger, "./log");
	ILog4zManager::getRef().setLoggerLimitsize(g_logger, 3);	//���Ƶ�����־�ļ��Ĵ�С����λM
	//ILog4zManager::getRef().prePushLog(g_logger, LOG_LEVEL_TRACE);
	ILog4zManager::getRef().setLoggerLevel(g_logger, LOG_LEVEL_FATAL);
	ILog4zManager::getRef().start();
	
	LOG_INFO(g_logger, "*** *** " << "��������" << " *** ***");
	HANDLE hMutex = CreateMutex(NULL, FALSE, _T("_MagicBox_CLIENT_GUI_"));
	if (hMutex && ERROR_ALREADY_EXISTS == GetLastError())
	{
		//��ʾ������
		HWND hwnd = ::FindWindow(NULL, _T("_MagicBox_GUI_"));
		if (hwnd != NULL)
		{
			LOG_INFO(g_logger, "��⵽����ʵ��������");
			::SendMessage(hwnd, USERMSG_SINGLE_RUN, 0, 0);
		}
		return 0;
	}

	CPaintManagerUI::SetInstance(hInstance);
	HRESULT Hr = ::CoInitialize(NULL);
	//���ó���Ĭ��ͼ��
	CIndexWnd mainWnd;
	mainWnd.Create(NULL, _T("_MagicBox_GUI_"), WS_OVERLAPPEDWINDOW, WS_EX_WINDOWEDGE);
	mainWnd.SetIcon(IDI_ICON_MAGICBOXEX);
	mainWnd.CenterWindow();
	mainWnd.ShowWindow(true);
	CPaintManagerUI::MessageLoop();
	::CoUninitialize();
	ILog4zManager::getRef().stop();

	LOG_INFO(g_logger, "*** *** " << "�����˳�" << " *** ***");

	return 0;
}



