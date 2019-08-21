// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <atlstr.h>
#include <WinSock2.h>
#include "..\\..\\common\\Detours\\detours.h"
#include "..\..\threadmodel\MsgDef.h"

#pragma comment(lib, "Ws2_32.lib")

const TCHAR *szIeCore = _T("Ie");
const TCHAR *szNotIeCore = _T("NotIe");

HHOOK g_Key = NULL;
HINSTANCE g_hModule;
TCHAR g_tszInstallDir[MAX_PATH];

CLogTrace g_log(_T("RunDetours.log"), NULL);
CDirectory g_dir;

//shell钩子消息处理过程
LRESULT CALLBACK SheelProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	//继续传递消息
	return CallNextHookEx(g_Key, nCode, wParam, lParam);
}


BOOL APIENTRY SetKeyHook(BOOL isInstall) 
{
	// 需要安装，且钩子不存在
	if (isInstall && !g_Key)
	{
		// 设置全局钩子
		g_Key = SetWindowsHookEx(WH_SHELL, (HOOKPROC)SheelProc, g_hModule, 0);

		if (g_Key == NULL)
		{
			return FALSE ;
		}
	}

	// 需要卸载，且钩子存在
	if (!isInstall && g_Key)
	{
		// 卸载钩子
		BOOL ret = UnhookWindowsHookEx(g_Key) ;
		g_Key = NULL ;
		return ret ;
	}

	return TRUE ;
}

//判断dll是否存在
bool IsDllExist(const TCHAR *szKernelStyle)
{
	CString strWebActiveXPluginPath = _T("");
	CString strNpSumPluginPath = _T("");

	strWebActiveXPluginPath.Format(_T("%s\\plugins\\ieplugin.dll"), g_dir.GetInstallDir());
	strNpSumPluginPath.Format(_T("%s\\plugins\\npszwplugin.dll"), g_dir.GetInstallDir());

	//用户打开的是ie内核浏览器且ie插件不存在
	if (_tcsicmp(szKernelStyle, szIeCore) == 0 && !PathFileExists(strWebActiveXPluginPath))
	{
		return false;
	}

	//用户打开的是非ie浏览器且非ie插件不存在
	if (_tcsicmp(szKernelStyle, szNotIeCore) == 0 && !PathFileExists(strNpSumPluginPath))
	{
		return false;
	}

	return true;
}

bool IsPostMsgToUi(const CString &str)
{
	if (((str.Find(_T("Referer: http://2.16898.cc/Member")) != -1 || str.Find(_T("Referer: http://2.16898.cc/enginespread")) != -1) && str.Find(_T("Host: 2.16898.cc")) != -1 && str.Find(_T("Content-Length: 0")) != -1)
		|| ((str.Find(_T("Referer: http://198.18.0.254:8009/Member")) != -1 || str.Find(_T("Referer: http://198.18.0.254:8009/enginespread")) != -1) && str.Find(_T("Host: 198.18.0.254:8009")) != -1 && str.Find(_T("Content-Length: 0")) != -1))
	{	
		int iPos = str.Find(_T("User-Agent"));
		if (-1 != iPos)
		{
			int iRet = str.Find(_T("\r\n"), iPos);
			if (iRet != -1)
			{
				CString strTemp = str.Mid(iPos, iRet - iPos);
				//判断是ie浏览器
				if (-1 != strTemp.Find(_T("MSIE")) || -1 != strTemp.Find(_T("Trident")))
				{
					if (!IsDllExist(szIeCore))
					{
						return true;
					}
				}
				else
				{
					//不是ie浏览器
					if (!IsDllExist(szNotIeCore))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

//在非ie浏览器和高版本ie浏览器中使用wsasend
static int (WSAAPI *Old_WSASend)(
	__in          SOCKET s,
	__in          LPWSABUF lpBuffers,
	__in          DWORD dwBufferCount,
	__out         LPDWORD lpNumberOfBytesSent,
	__in          DWORD dwFlags,
	__in          LPWSAOVERLAPPED lpOverlapped,
	__in          LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
	) = WSASend;

int WSAAPI New_WSASend(
	__in          SOCKET s,
	__in          LPWSABUF lpBuffers,
	__in          DWORD dwBufferCount,
	__out         LPDWORD lpNumberOfBytesSent,
	__in          DWORD dwFlags,
	__in          LPWSAOVERLAPPED lpOverlapped,
	__in          LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
	)
{
	CString  str = lpBuffers->buf;

	if (dwFlags == 0)
	{
		if (IsPostMsgToUi(str))
		{
			HWND hWnd = FindWindow(_T("舟大师"), NULL);
			if (hWnd != NULL)
			{
				PostMessage(hWnd, MSG_WEB_ACTIVEX_LOSE, 0, 0);
			}
		}
	}

	return Old_WSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}

//一般在低版本ie中使用的是send
 static int (WSAAPI *Old_send)(
 	__in          SOCKET s,
 	__in          const char* buf,
 	__in          int len,
 	__in          int flags
 	) = send;
 
 int WSAAPI New_send(
 	__in          SOCKET s,
 	__in          const char* buf,
 	__in          int len,
 	__in          int flags
 	)
 {
 	CString str = buf;
 	
	if (flags == 0)
	{
		if (IsPostMsgToUi(str))
		{
			HWND hWnd = FindWindow(_T("舟大师"), NULL);
			if (hWnd != NULL)
			{
				PostMessage(hWnd, MSG_WEB_ACTIVEX_LOSE, 0, 0);
			}
		}
	}

 	return Old_send(s, buf, len, flags);
 }


void SetHook()
{
	long lValue = 0;
	lValue = DetourTransactionBegin();  
	lValue = DetourUpdateThread(GetCurrentThread());  
	lValue = DetourAttach(&(PVOID&)Old_send, New_send);  
	lValue = DetourAttach(&(PVOID&)Old_WSASend, New_WSASend); 
	lValue = DetourTransactionCommit();  
}
void  UnHook()
{
	DetourTransactionBegin();  
	DetourUpdateThread(GetCurrentThread());  
	DetourDetach(&(PVOID&)Old_send, New_send);  
	DetourDetach(&(PVOID&)Old_WSASend, New_WSASend);   
	DetourTransactionCommit();  
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hModule = hModule;

			CString strFilePath = _T("");
			GetModuleFileName(NULL, strFilePath.GetBuffer(MAX_PATH), MAX_PATH);
			strFilePath.ReleaseBuffer(MAX_PATH);

			CString strExeName = PathFindFileName(strFilePath);
			if (strExeName.CompareNoCase(_T("MasterZ.exe")) == 0)
			{
				SetKeyHook(TRUE);
			}
			SetHook();
		}
		break;
	case DLL_THREAD_ATTACH:
		{

		}
		break;
	case DLL_THREAD_DETACH:
		{

		}
		break;
	case DLL_PROCESS_DETACH:
		{
			UnHook();
			SetKeyHook(FALSE);
		}
		break;
	}
	return TRUE;
}

