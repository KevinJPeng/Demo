// MasterZ.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "MasterZ.h"
#include "MainWnd.h"

CLogTrace g_log(_T("runlog.log"), NULL);

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{

 	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("命令行参数为%s"),lpCmdLine);

	//begin add by zhumingxing 20150316 增加客户端的类型控制
	DWORD dwClientType = GetClientType();
	//end add
	/*::MessageBox(NULL, _T("test"), _T("tt"), MB_OK);*/
	//增加单实例运行_BIZ_CLIENT_GUI_2.0
	HANDLE hMutex = CreateMutex(NULL, FALSE, _T("_MASTERZ_CLIENT_GUI_"));
	if (hMutex && ERROR_ALREADY_EXISTS == GetLastError())
	{	
		DWORD dwHideType = GetHideType();

		if (dwHideType == HIDE_VERSION)
		{
			//结束客户端，重新启动
			ULONG nProcessID;
			HANDLE hProcessHdl;
			HWND hwnd = ::FindWindow(NULL, _T("舟大师"));
			::GetWindowThreadProcessId(hwnd, &nProcessID);
			hProcessHdl = ::OpenProcess(PROCESS_TERMINATE, FALSE, nProcessID);
			::TerminateProcess(hProcessHdl, 4);
			goto beginLab;
		}

		if (!_tcscmp(lpCmdLine,_T("/Auto")) || !_tcscmp(lpCmdLine,_T("/Update")) || dwClientType == 1)
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检查到有实例正在运行！"));
			return FALSE;
		}

		//显示客户端主界面
		HWND hwnd = ::FindWindow(NULL,_T("舟大师"));
		if (hwnd != NULL)
		{
			::SendMessage(hwnd,WM_TRAY_MSG,0,WM_LBUTTONDBLCLK);
		}
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检查到有实例正在运行！"));
		return FALSE;
	}

beginLab:
	
	SetHideType();
	//生成dump文件dll发布时需要
	LoadLibrary(_T("BugReport.dll"));

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("起主程序界面！"));
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourceZip(_T("skin.zip"));

	CMainWnd mainWnd;
	mainWnd.SetClientType(dwClientType);

	mainWnd.Create(NULL, _T("舟大师"), WS_OVERLAPPEDWINDOW, WS_EX_WINDOWEDGE);
	mainWnd.ShowWindow(false);
	if (dwClientType == MAIN_LINE_VERSION)
	{
		mainWnd.SetIcon(IDI_MasterZ);
		mainWnd.CenterWindow();
	}

	//静默安装
	if (!_tcscmp(lpCmdLine, _T("/Silent")))
	{	
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检查到为静默安装启动运行！"));
		mainWnd.setTrayTips(_T("已经升级到最新版本!"), _T("舟大师"), _T("舟大师"));
		CPaintManagerUI::MessageLoop();
		return 0;
	}
	//开机启动
	if (!_tcscmp(lpCmdLine,_T("/Auto")))
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检查到为开机启动运行！"));
		CPaintManagerUI::MessageLoop();
		return 0;
	}
	//升级后台启动
	if (!_tcscmp(lpCmdLine,_T("/Update")))
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检查到为升级后启动运行！"));

		CPaintManagerUI::MessageLoop();
		return 0;
	}

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检查到为用户点击运行！"));

	if (dwClientType == MAIN_LINE_VERSION)
	{
		mainWnd.ShowWindow(true);
	}
	CPaintManagerUI::MessageLoop();
	return 0;
}
