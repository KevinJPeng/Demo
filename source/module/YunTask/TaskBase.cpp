#include "StdAfx.h"
#include "TaskBase.h"
#include "Tlhelp32.h"

CTaskBase::CTaskBase(void)
{

}

CTaskBase::~CTaskBase()
{

}

//判断进程是否存在
bool CTaskBase::ProcessExist(TCHAR *pstrProcName, HANDLE *phProcess)
{
	ASSERT(pstrProcName != NULL);

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		if (_tcsicmp(pstrProcName, pe32.szExeFile) == 0)
		{
			//需要传出进程句柄
			if (phProcess)
			{
				*phProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			}

			return true;
		}

		bMore = ::Process32Next(hProcessSnap, &pe32);
	}

	return false;
}

//启动进程
bool CTaskBase::StartProcess(TCHAR *pstrProcName, TCHAR *pstrPort, HANDLE *phProcess)
{
	ASSERT(pstrProcName != NULL);

	TCHAR strDir[MAX_PATH] = {_T('\0')};
	TCHAR strExePath[MAX_PATH] = {0};

	_tcscpy_s(strDir, pstrProcName);

	TCHAR *pch = _tcsrchr(strDir, _T('\\'));
	if (pch == NULL/* || !PathFileExists(pstrProcName)*/)
	{
		return false;
	}

	*pch = 0;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si,sizeof(si));
	si.cb =sizeof(si);
	ZeroMemory(&pi,sizeof(pi));
	si.dwFlags = STARTF_FORCEOFFFEEDBACK;

	if(CreateProcess( pstrProcName, pstrPort, NULL,
		NULL,FALSE,0,NULL,strDir,&si,&pi ) == TRUE)
	{
		if (NULL != phProcess)
			*phProcess = pi.hProcess;

		return true;
	}

	return false;
}

//结束指定进程
bool CTaskBase::StopProcess(TCHAR *pstrProcName)
{
	HANDLE hProcess = NULL;

	if (ProcessExist(pstrProcName, &hProcess))
	{
		if (!hProcess)
			TerminateProcess(hProcess, 1);
	}

	return true;
}

//结束指定进程
bool CTaskBase::StopProcess(HANDLE hProcess)
{
	if (!hProcess && TerminateProcess(hProcess, 1))
	{
		return true;
	}

	return false;
}

//获取一个当前可用的端口号，用于启动主控
int CTaskBase::GetPort(int nDefault)
{
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR )
		return -1;

	// 创建socket
	SOCKET t_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if (nDefault < 1024)
		nDefault = 28016;

	int t_Port = nDefault;
	int r_Port = 0;
	sockaddr_in t_service;

	while(true)
	{
		int i = t_Port + r_Port;
		t_service.sin_family = AF_INET;
		t_service.sin_addr.s_addr = inet_addr( "127.0.0.1" );
		t_service.sin_port = htons( i );	//web版端口
		if ( ::bind( t_socket, (SOCKADDR*) &t_service, sizeof(t_service) ) != SOCKET_ERROR )
		{
			t_Port = i;
			break;
		}

		r_Port = rand() % 1000;

		Sleep(30);
	}

	closesocket(t_socket);
	return t_Port;
}

bool CTaskBase::IsOwnerMCProcess()
{
	HANDLE hMcMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T("szwWebMC"));

	if (hMcMutex != NULL)
	{
		CloseHandle(hMcMutex);
		hMcMutex = NULL;

		return TRUE;
	}
	return FALSE;
}
