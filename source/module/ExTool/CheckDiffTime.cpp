#include "stdafx.h"
#include "CheckDiffTime.h"

#include <WinSock2.h>
#include <WinInet.h>

const DWORD dwSpanTime = 10*60;  //网络时间和本地时间的间隔，为10分钟


CCheckDiffTime::CCheckDiffTime(void)
{
	m_bCheckFlag = false;
	m_bRepairFlag = false;
	m_bNetConnect = false;
	m_bIsGetNetTime = false;
}
CCheckDiffTime::~CCheckDiffTime(void)
{

}

BOOL CCheckDiffTime::CheckPro(void)
{
	//判断网络连接状态
	if (!IsInternetConnect())
	{
		m_bCheckFlag = false;
		m_bNetConnect = false;
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("电脑网络连接失败！"));

		return FALSE;
	}
	m_bNetConnect = true;

	CString strServer(_T("time.ien.it"));
	CTime IntnetTime(1990, 1, 1, 0, 0, 0);

	//获取网络时间
	if(GetInternetTime(&IntnetTime, strServer))
	{
		m_bIsGetNetTime = true;

		CTimeSpan timeSpan;
		CTime localTime = CTime::GetCurrentTime();

		if (IntnetTime - localTime > 0)
		{
			timeSpan = IntnetTime - localTime;
		}
		else
		{
			timeSpan = localTime - IntnetTime;
		}
		
		if (timeSpan.GetTotalSeconds() > dwSpanTime)
		{
			m_bCheckFlag = false;
			return FALSE;
		}
	}
	else
	{
		m_bIsGetNetTime = false;
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("检测时获取网络时间失败！"));
		return FALSE;
	}

	m_bCheckFlag = true;

	return TRUE;
}

BOOL CCheckDiffTime::RepairPro(void)
{
	CString strServer(_T("time.ien.it"));
	CTime tm(1990, 1, 1, 0, 0, 0);

	if(GetInternetTime(&tm, strServer))
	{
		m_bIsGetNetTime = true;

		if (SyncSystemClock(tm))
		{
			m_bRepairFlag = true;
		}
	}
	else
	{
		m_bIsGetNetTime = false;
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("修复时获取网络时间失败！"));
		return FALSE;
	}

	return TRUE;
}


void CCheckDiffTime::GetBackCheckStruct(T_PROBLEM_DATA &tData)
{
	tData.strIndex = _T("3");
	tData.bCheckFlag = m_bCheckFlag;

	if (m_bNetConnect) //网络是否连接
	{
		if (!m_bIsGetNetTime)
		{
			tData.strSuggestion = ProblemCheckDiffTimeFault;
			return;
		}

		if (!tData.bCheckFlag)
		{
			tData.strSuggestion = ProblemCheckDiffTimeSuggest;
			tData.bIsRepair = true;
		}	
	}
	else
	{
		tData.strSuggestion = ProblemCheckNetConnect;
	}
}

void CCheckDiffTime::GetBackRepairStruct(T_PROBLEM_DATA &tData)
{
	tData.strIndex = _T("3");

	if (!m_bIsGetNetTime)
	{
		tData.strSuggestion = ProblemCheckDiffTimeFault;
	}

	if (m_bRepairFlag)
	{
		tData.strSuggestion = RepairCheckDiffTimeSuggest;
		tData.bRepairFlag = true;
	}
}


BOOL CCheckDiffTime::GetInternetTime(CTime* pTime, CString strServer)
{
	//if (!AfxSocketInit())
	//{
	//	g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("初始化socket失败！"));
	//	return FALSE;
	//}

	//CSocket sockClient;

	//try
	//{
	//	sockClient.Create();			//创建socket
	//}
	//catch (CException* e)
	//{
	//}
	//
	//sockClient.Connect((LPCTSTR)strServer, 37); // strServer：时间服务器网址； 37：端口号

	int ilength = -1;
	WORD wVerisonRequested;
	WSADATA wsaData;
	int err;
	wVerisonRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVerisonRequested, &wsaData);
	if (err != 0)
	{
		return -1;
	}
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

	struct hostent *hbd;
	hbd = gethostbyname(CStringToMutilChar(strServer,ilength));

	struct in_addr addrIpTime;
	memcpy(&addrIpTime,hbd->h_addr_list[0],sizeof(in_addr));

	//inet_ntop(hbd->h_addrtype, *hbd->h_addr_list,ipbd, sizeof(ipbd));
	char* pHostIp = inet_ntoa(addrIpTime);
	SOCKADDR_IN addrServer;
	addrServer.sin_addr.S_un.S_addr = inet_addr(pHostIp);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(37);
	int mmm = connect(sockClient, (SOCKADDR *)&addrServer, sizeof(addrServer));


	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("请求的时间服务器地址为：%s！"), (LPCTSTR)strServer);

	DWORD dwTime = 0;				//用来存放服务器传来的标准时间数据
	unsigned char nTime[8];			//临时接收数据
	memset(nTime, 0, sizeof(nTime));

	//sockClient.Receive(nTime, sizeof(nTime));	//接收服务器发送来得4个字节的数据
	recv(sockClient, (char*)nTime, sizeof(nTime), 0);
	closesocket(sockClient);				//关闭socket
	WSACleanup();

	dwTime += nTime[0] << 24;		//整合数据	
	dwTime += nTime[1] << 16;
	dwTime += nTime[2] << 8;
	dwTime += nTime[3];		

	if (0 == dwTime)	
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("收到的网络时间通过转换后为0！"));
		return FALSE;
	}

	//服务器传来的数据是自从1900年以来的秒数
	//取得 1900~1970 的时间差(以秒数计算) ，放在dwSpan里面
	COleDateTime t00( 1900, 1, 1, 0, 0, 0 ); // 1900.1.1 00:00:00 
	COleDateTime t70( 1970, 1, 1, 0, 0, 0 ); // 1970.1.1 00:00:00 

	COleDateTimeSpan ts70to00 = t70 - t00; 
	DWORD dwSpan = (DWORD)ts70to00.GetTotalSeconds(); 
	ASSERT( dwSpan == 2208988800L ); 

	//把时间变为基于1970年的，便于用CTime处理
	dwTime -= dwSpan;		

	//构造当前时间的CTime对象
	CTime timeNow = (CTime)dwTime;
	*pTime = timeNow;		

	return TRUE;
}


BOOL CCheckDiffTime::SyncSystemClock(CTime tmServer)
{
	//如果是在XP下，则先提升进程的权限
	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof OSVERSIONINFO;
	GetVersionEx(&osv);
	if(osv.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		HANDLE hToken; 
		TOKEN_PRIVILEGES tkp; 

		// Get a token for this process. 

		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
		{
			return FALSE; 
		}

		// Get the LUID for the shutdown privilege. 

		LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &tkp.Privileges[0].Luid); 

		tkp.PrivilegeCount = 1;  // one privilege to set    
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

		// Get the shutdown privilege for this process. 

		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 

		if (GetLastError() != ERROR_SUCCESS) 
		{
			return FALSE;
		} 
	}

	//设置系统时间
	SYSTEMTIME systm;
	systm.wYear = tmServer.GetYear();
	systm.wMonth = tmServer.GetMonth();
	systm.wDay = tmServer.GetDay();
	systm.wHour = tmServer.GetHour();
	systm.wMinute = tmServer.GetMinute();
	systm.wSecond = tmServer.GetSecond();
	systm.wMilliseconds = 0;


	if(0 != ::SetLocalTime(&systm))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("设置本地时间成功！"));
		return TRUE;
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("设置本地时间失败！"));
		return FALSE;
	}
}


bool CCheckDiffTime::IsInternetConnect()
{	
	DWORD dwFlags = 0;
	if (!InternetGetConnectedState(&dwFlags, 0))
	{
		return false;
	}
	return true;
}

char* CCheckDiffTime::CStringToMutilChar( CString& str,int& chLength,WORD wPage/*=CP_ACP*/ )
{
	char* pszMultiByte; 
	int iSize = WideCharToMultiByte(wPage, 0, str, -1, NULL, 0, NULL, NULL); 
	pszMultiByte = (char*)malloc((iSize+1)/**sizeof(char)*/); 
	memset(pszMultiByte,0,iSize+1);
	WideCharToMultiByte(wPage, 0, str, -1, pszMultiByte, iSize, NULL, NULL); 
	chLength = iSize;
	return pszMultiByte;
}

