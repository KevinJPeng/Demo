// UpdateRank.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "UpdateRank.h"
#include "CWebBrowser2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ψһ��Ӧ�ó������

CWinApp theApp;

using namespace std;


//��ͨ��ʽ����̨�ɽ���
BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
		if (NULL != g_server)
		{
			g_server->UnInit();
		}
		printf("Ctrl-Close event\n\n");
		return(FALSE);

		// Pass other signals to the next handler. 
	case CTRL_BREAK_EVENT:

		printf("Ctrl-Break event\n\n");
		return FALSE;

	case CTRL_LOGOFF_EVENT:

		printf("Ctrl-Logoff event\n\n");
		return FALSE;

	case CTRL_SHUTDOWN_EVENT:

		printf("Ctrl-Shutdown event\n\n");
		return FALSE;

	default:
		return FALSE;
	}
}
BOOL GetPath(TCHAR* szPath)
{
	if (szPath == NULL)
		return FALSE;

	if (GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		PathAppend(szPath, _T("..\\..\\"));
		return TRUE;
	}
	return FALSE;
}
//��ʼ��Ŀ¼�����������򴴽�
void InitDir(void)
{
	TCHAR tszInstPath[MAX_PATH] = { 0 };
	GetPath(tszInstPath);

	const int nFolderNum = 4;
	TCHAR tszDir[nFolderNum][MAX_PATH] = { _T("data"), _T("log"), _T("6") };

	for (int i = 0; i < nFolderNum; i++)
	{
		TCHAR tszTarDir[MAX_PATH] = { 0 };
		_stprintf(tszTarDir, _T("%s%s"), tszInstPath, tszDir[i]);

		SHCreateDirectoryEx(NULL, tszTarDir, NULL);
	}
}

//DWORD ServiceRun(int iArgCount, wchar_t** arg)
DWORD ServiceRun(int argc, char* argv[])
{
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("Eneter MCServiceRun"));
	int nPort = 27015; //������ܵ���27015��WEB��ܵ���27016
	BOOL bIsYun = FALSE; //�Ƿ��������ƹ�
	TCHAR szPath[MAX_PATH] = { 0 };

	if (GetPath(szPath))
	{
		PathAppend(szPath, _T("data2\\mc.dat"));
	}

	InitDir();

	IXMLRW xmlcfg;
	xmlcfg.init(szPath);
// 	g_DatMc = new IXMLRW;
// 	g_DatMc->init(szPath);

#ifdef _DEBUG
	MessageBox(NULL, L"webMcProc entry", L"DEBUG", MB_OK);
#else
	int nDbgFlag = 0;
	xmlcfg.ReadInt(_T("MC"), _T("CONTROL"), _T("DEBUG"), nDbgFlag); //���������ļ������˿�

	if (nDbgFlag == 1)
		MessageBox(NULL, L"webMcProc entry", L"DEBUG", MB_OK);
#endif

	xmlcfg.ReadInt(_T("MC"), _T("SERVICE"), _T("PORT"), nPort); //���������ļ������˿�
	if (nPort == 0)nPort = 27019;

	if (argc >= 2)
	{
		nPort = atoi(argv[1]);
	}
	printf("�˿� %d \n", nPort);

	if (argc >= 3)
	{
		CStringA strPara = argv[2];
		if (strPara.Compare("-y") == 0)
		{
			bIsYun = TRUE;
		}
	}

	// 	g_GlobalDataBlock = new CGlobalDataBlock();	//����ȫ��������
	// 
	// 	WriteInitGlobalData();

	g_server = new CServerSocket;

	if (g_server->Init(nPort) == -1)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T(" MCServiceRun ����ʧ��!�˿ں� %d", nPort));
		printf("����ʧ��\n");
		return 0;
	}

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("Enter MCServiceRun WaitRecData"));
	g_server->WaitRecData(FALSE);
	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T(" OUT WaitRecData"));

	g_server->UnInit();


	if (g_server)
		delete g_server;

	return(0);
}

int main(int argc, char* argv[])
{
	int nRetCode = 0;
	//Sleep(30000);

	HMODULE hModule = ::GetModuleHandle(NULL);
	if (hModule != NULL)
	{
		// ��ʼ�� MFC ����ʧ��ʱ��ʾ����
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO:  ���Ĵ�������Է���������Ҫ
			_tprintf(_T("����:  MFC ��ʼ��ʧ��\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO:  �ڴ˴�ΪӦ�ó������Ϊ��д���롣
			AfxEnableControlContainer();
			//CWinApp::InitInstance();

			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("Enter CUpdateRankApp::InitInstance"));
			HANDLE hMutex = CreateMutex(NULL, FALSE, _T("szwUpdateRankSpider"));
			setlocale(LC_ALL, "chs");   //��������ѡ��

#ifndef _DEBUG 
			HINSTANCE histDbg = LoadLibrary(_T("BugReport.dll"));
			if (histDbg == NULL)
			{
				::MessageBox(NULL, _T("װ��BugReport����"), _T("�����°�װ�ͻ���"), MB_OK | MB_ICONERROR);
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("load bugreport.dll failed,errcode:%d"), GetLastError());
				return 0;
			}
#endif	
			SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
			//ServiceRun(__argc, __targv);
			ServiceRun(argc, argv);
			CloseHandle(hMutex);
			hMutex = NULL;
#ifndef _DEBUG
			FreeLibrary(histDbg);
#endif
			return FALSE;
		}
	}
	else
	{
		// TODO:  ���Ĵ�������Է���������Ҫ
		_tprintf(_T("����:  GetModuleHandle ʧ��\n"));
		nRetCode = 1;
	}
	return nRetCode;
}
