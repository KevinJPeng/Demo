#include "stdafx.h"
#include "CommFunc.h"
#include <shlobj.h>
#include <tlhelp32.h>
//#include <afxstr.h>
//#include <Windows.h>

#include <Iphlpapi.h>

#include <WinVer.h>


#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Iphlpapi.lib")

#pragma warning(disable: 4996)

TCHAR *GetCurPath(void)
{
	static TCHAR s_szCurPath[MAX_PATH] = {0};

	GetModuleFileName(NULL, s_szCurPath, MAX_PATH);

	TCHAR *p = _tcsrchr(s_szCurPath, '\\');
	*p = 0;

	return s_szCurPath;
}

TCHAR *GetProgPath(void)
{
	static TCHAR s_szProgPath[MAX_PATH] = {0};

	SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 1, s_szProgPath);

	return s_szProgPath;
}

TCHAR *GetInstallPath(void)
{
	static TCHAR s_szCfgPath[MAX_PATH] = {0};

	GetModuleFileName(NULL, s_szCfgPath, MAX_PATH);

	TCHAR *p = _tcsrchr(s_szCfgPath, '\\');
	if (p != NULL)
	{
		*p = 0;
	}

	p = _tcsrchr(s_szCfgPath, '\\');
	if (p != NULL)
	{
		*p = 0;
	}

	return s_szCfgPath;
}

// 多字节转宽字节，长度不足时返回需要的长度
LPWSTR MByteToWChar( LPSTR lpcszStr, LPWSTR lpwszStr, DWORD *dwSize, UINT codePage)
{
	DWORD dwMinSize;

 	dwMinSize = MultiByteToWideChar(codePage,NULL,lpcszStr,-1,NULL,0);
 	if(*dwSize < dwMinSize || lpwszStr == NULL)
 	{
		*dwSize = dwMinSize;
 		return NULL;
 	}
 
 	MultiByteToWideChar(codePage,NULL,lpcszStr,-1,lpwszStr,*dwSize);

	*dwSize = dwMinSize;
	return lpwszStr;
}

LPSTR WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD *dwSize, UINT codePage)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(codePage,NULL,lpcwszStr,-1,NULL,0,NULL,FALSE);
	if(*dwSize < dwMinSize)
	{
		*dwSize = dwMinSize;
		return NULL;
	}
	WideCharToMultiByte(codePage,NULL,lpcwszStr,-1,lpszStr,*dwSize,NULL,FALSE);
	*dwSize = dwMinSize;
	return lpszStr;
}

bool WaitProcEnd(TCHAR *pProgName, DWORD dwTimeOut, bool bForceEnd)
{
	DWORD dwStartTime = GetTickCount();

	while(true)
	{
		bool bRun = false;
		PROCESSENTRY32 process; 

		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (((int)hSnapShot) != -1)  
		{ 
			process.dwSize = sizeof(PROCESSENTRY32);  

			BOOL bFlag = Process32First(hSnapShot, &process);
			while (bFlag)
			{       
				if (!_tcsicmp(pProgName, process.szExeFile))
				{
					bRun = true;
					break;
				}

				bFlag = Process32Next(hSnapShot, &process);
			}

			CloseHandle(hSnapShot); 

			if (!bRun) return true;
		}
		else
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("CreateToolhelp32Snapshot failed, err: %d"), GetLastError());

		//等待超时
		if (GetTickCount() - dwStartTime > dwTimeOut)
		{
			if (bForceEnd)
			{
				HANDLE handLe =  OpenProcess(PROCESS_TERMINATE, FALSE, process.th32ProcessID);
				BOOL bResult = TerminateProcess(handLe,0);
				return bResult;
			}

			return false;
		}

		Sleep(100);
	}

	return false;
}
// 比较两个BSTR类型的字符串
// int CompareBSTR(_bstr_t bstr1, _bstr_t bstr2)
// {
// 	if (bstr1 == bstr2)
// 		return 0;
// 	else if (bstr1 > bstr2)
// 		return 1;
// 	else
// 		return -1;
// }
// 
// 比较两个BSTR类型的字符串（忽略大小写）
// int CompareBSTRNoCase(_bstr_t bstr1, _bstr_t bstr2)
// {
// 	CComBSTR comBstr1(bstr1.GetBSTR());
// 	CComBSTR comBstr2(bstr2.GetBSTR());
// 
// 	comBstr1.ToLower();
// 	comBstr2.ToLower();
// 
// 	if (comBstr1 == comBstr2)
// 		return 0;
// 	else if (comBstr1 > comBstr2)
// 		return 1;
// 	else
// 		return -1;
// 
// }
// 

LONG GetGlobalIndexNum(void)
{
	static LONG lngBase = 0;

	return InterlockedIncrement(&lngBase);
}

int GetRandomNum()
{
	return rand() % 256;
}

void GetRandomMac(TCHAR *szMac, int nBufferSize)
{
	_stprintf_s(szMac, nBufferSize / sizeof(TCHAR), _T("%02X-%02X-%02X-%02X-%02X-%02X"), 0xFF, 0xFF, GetRandomNum(), GetRandomNum(), GetRandomNum(), GetRandomNum());
	return;
}

/*
add by zhoulin
@brief  通过网卡驱动IO取得mac地址
@param szMac Mac字符串空间, nBuffSize 空间大小
@return int状态码 成功返回0，失败返回-1
*/
int GetMac(TCHAR *szMac, int nBuffSize)
{
	int nError = -1;
	if (szMac == NULL || nBuffSize < 18){
		return nError;
	}

	HKEY hKey = NULL;
	HKEY hKey2 = NULL;
	TCHAR szKey[MAX_PATH], szBuffer[MAX_PATH];
	TCHAR szServiceName[MAX_PATH];
	TCHAR szFileName[MAX_PATH] = { 0 };
	DWORD dwRet = 0;
	DWORD dwType = 0;
	DWORD cbData = 0;
	DWORD cName = _countof(szBuffer);
	if (RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\"), &hKey) != ERROR_SUCCESS){
		return nError;
	}

	for (int i = 0; RegEnumKeyEx(hKey, i, szBuffer, &cName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS; ++i, cName = _countof(szBuffer))
	{
		_tcscpy_s(szKey, MAX_PATH, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\"));
		_tcscat_s(szKey, MAX_PATH, szBuffer);
		if (RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hKey2) != ERROR_SUCCESS)
		{
			continue;
		}

		dwType = REG_SZ;
		cbData = MAX_PATH*sizeof(TCHAR);
		if (RegQueryValueEx(hKey2, _T("ServiceName"), NULL, &dwType, (LPBYTE)szServiceName, &cbData) == ERROR_SUCCESS)
		{
			RegCloseKey(hKey2);

			_tcscpy_s(szFileName, MAX_PATH, _T("\\\\.\\"));
			_tcscat_s(szFileName, MAX_PATH, szServiceName);
			HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwInBuff = OID_802_3_PERMANENT_ADDRESS;
				BYTE outBuff[MAX_PATH];
				dwRet = DeviceIoControl(hFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &dwInBuff, sizeof(dwInBuff), outBuff, sizeof(outBuff), &cbData, NULL);

				//无论成功失败关闭文件句柄
				CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;

				if (dwRet)
				{
					_stprintf_s(szMac, nBuffSize / sizeof(TCHAR), _T("%02X-%02X-%02X-%02X-%02X-%02X"), outBuff[0], outBuff[1], outBuff[2], outBuff[3], outBuff[4], outBuff[5]);
					nError = 0;
					break;
				}
			}
		}
		else
		{
			RegCloseKey(hKey2);
		}

	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
	}

	return nError;
}

BOOL GetMacFromRegedit(HKEY hKey, const TCHAR *const pszKeyPath, const TCHAR *const pszKeyName, TCHAR* szMac)
{
	HKEY hSubKey;
	DWORD dwType = 0;
	BOOL bRet = FALSE;
	DWORD dwLen = 0;
	long lRes = RegOpenKeyEx(hKey, pszKeyPath, 0, KEY_ALL_ACCESS, &hSubKey);
	if (lRes == ERROR_SUCCESS)
	{
		DWORD cbData = MAX_PATH * sizeof(TCHAR);
		if (RegQueryValueEx(hSubKey, pszKeyName, NULL, &dwType, (LPBYTE)szMac, &cbData) == ERROR_SUCCESS)
		{
			bRet = TRUE;
		}
	}
	RegCloseKey(hSubKey);

	return bRet;
}

BOOL WriteMacToRegedit(HKEY hKey, const TCHAR *const pszKeyPath, const TCHAR *const pszKeyName, const TCHAR *const pszKeyValue)
{
	HKEY hKeySub;
	long lRes = RegCreateKey(hKey, pszKeyPath, &hKeySub);
	if (ERROR_SUCCESS == lRes)
	{
		lRes = RegSetValueEx(hKeySub, pszKeyName, 0, REG_SZ, (BYTE*)pszKeyValue, _tcslen(pszKeyValue) * 2);
	}

	RegCloseKey(hKeySub);
	return (lRes == ERROR_SUCCESS);
}

CString GetMacByCongigAll()
{
	BOOL bret;
	SECURITY_ATTRIBUTES sa;
	HANDLE hReadPipe, hWritePipe;
	const int MAX_COMMAND_SIZE = 10240;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;


	CString strData = _T("");
	TCHAR szFetCmd[] = _T("ipconfig /all");

	CString str4Search = _T("物理地址. . . . . . . . . . . . . : ");

	//创建管道
	if (CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		si.cb = sizeof(STARTUPINFO);
		GetStartupInfo(&si);

		si.hStdError = hWritePipe;
		si.hStdOutput = hWritePipe;
		si.wShowWindow = SW_HIDE; //隐藏命令行窗口
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

		//创建获取命令行进程
		bret = CreateProcess(NULL, szFetCmd, NULL, NULL, TRUE, 0, NULL,
			NULL, &si, &pi);

		char szBuffer[MAX_COMMAND_SIZE + 1]; //放置命令行输出缓冲区
		CString strBuffer = _T("");
		if (bret)
		{
			//WaitForSingleObject (pi.hProcess, INFINITE); 
			Sleep(500);
			unsigned long count;
			memset(szBuffer, 0x00, sizeof(szBuffer));
			bret = ReadFile(hReadPipe, szBuffer, MAX_COMMAND_SIZE, &count, 0);
			if (bret)
			{
				long ipos;

				strBuffer = szBuffer;
				ipos = strBuffer.Find(str4Search.GetString());


				if (ipos == -1)
				{
					str4Search = _T("Physical Address. . . . . . . . . : ");
					ipos = strBuffer.Find(str4Search.GetString());
				}
				//提取MAC地址串
				strBuffer = strBuffer.Mid(ipos + str4Search.GetLength());
				ipos = strBuffer.Find(_T("\n"));
				strBuffer = strBuffer.Mid(0, ipos);
				long ipos1 = strBuffer.Find(_T("\r"));
				if (ipos1 == -1)
				{
					strData = strBuffer.Mid(0, ipos);
				}
				else
				{
					strData = strBuffer.Mid(0, ipos1);
				}
			}
			else
			{
				strData = _T("");
			}

		}
		else
		{
			strData = _T("");
		}

		//关闭所有的句柄
		CloseHandle(hWritePipe);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hReadPipe);
	}

	return strData;
}

CString GetMacByAPI()
{
	PIP_ADAPTER_ADDRESSES pIpAdapterInfo = new IP_ADAPTER_ADDRESSES();
	unsigned long stSize = sizeof(IP_ADAPTER_ADDRESSES);
	CString strMacAddr = _T("");

	int nRel = GetAdaptersAddresses(AF_INET, 0, NULL, pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		delete pIpAdapterInfo;

		//重新申请内存空间用来存储所有网卡信息
		pIpAdapterInfo = (PIP_ADAPTER_ADDRESSES)new BYTE[stSize];
		nRel = GetAdaptersAddresses(AF_INET, 0, NULL, pIpAdapterInfo, &stSize);
	}

	if (ERROR_SUCCESS == nRel)
	{

		for (int i = 0; i < pIpAdapterInfo->PhysicalAddressLength; i++)
		{
			strMacAddr.AppendFormat(_T("%02X-"), pIpAdapterInfo->PhysicalAddress[i]);
		}

		strMacAddr.Delete(strMacAddr.GetLength() - 1);

	}

	if (pIpAdapterInfo)
		delete pIpAdapterInfo;

	if (strMacAddr.IsEmpty())
	{
		strMacAddr = GetMacByCongigAll();
	}

	return strMacAddr;
}

CString GetPhysicalAddress()
{
	CString strMac = _T("");
	TCHAR szMac[MAX_PATH] = { 0 };
	if (GetMac(szMac, MAX_PATH) == 0)
	{
		strMac = szMac;
	}
	else
	{
		strMac = GetMacByAPI();
	}
	if (strMac.Find(_T("-")) == -1)
	{
		if (GetMacFromRegedit(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("ClientDefaultMac"), szMac))
		{
			strMac = szMac;
		}
		if (strMac.Find(_T("-")) == -1)
		{
			GetRandomMac(szMac, MAX_PATH);
			strMac = szMac;
			WriteMacToRegedit(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("ClientDefaultMac"), szMac);
		}
	}
	return strMac;
}

// 安全的取得真实系统信息
void SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo)
{
	if (NULL == lpSystemInfo)    
		return;

	typedef VOID(WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);

	LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetNativeSystemInfo");

	if (NULL != fnGetNativeSystemInfo)
	{
		fnGetNativeSystemInfo(lpSystemInfo);
	}
	else
	{
		GetSystemInfo(lpSystemInfo);
	}

}

// 获取操作系统位数
void GetSystemBits(__out TCHAR* szBits)
{
	SYSTEM_INFO si;
	SafeGetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
	{
		_tcscpy(szBits, _T("64-bit"));
		return;
	}
	_tcscpy(szBits, _T("32-bit"));
	return;
}

// 通过Kernel32.dll版本信息跟GetVersionEx配合获取,因为只用GetVersionEx在操作系统程序设置成兼容模式下无法正确判断
void GetSystemNameByKernel32(__out TCHAR* szSystemName)
{
	_tcscpy(szSystemName, _T("Windows Unknown"));

	SYSTEM_INFO info;
	::GetSystemInfo(&info);
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	const TCHAR szFilename[] = _T("kernel32.dll");
	DWORD dwMajorVersion = 0, dwMinorVersion = 0;
	DWORD dwHandle = 0;

	DWORD dwVerInfoSize = GetFileVersionInfoSize(szFilename, &dwHandle);

	if (dwVerInfoSize)
	{
		LPVOID lpBuffer = LocalAlloc(LPTR, dwVerInfoSize);
		if (lpBuffer)
		{
			if (GetFileVersionInfo(szFilename, dwHandle, dwVerInfoSize, lpBuffer))
			{
				VS_FIXEDFILEINFO * lpFixedFileInfo = NULL;
				UINT nFixedFileInfoSize = 0;
				if (VerQueryValue(lpBuffer, TEXT("\\"), (LPVOID*)&lpFixedFileInfo, &nFixedFileInfoSize) && (nFixedFileInfoSize))
				{
					dwMajorVersion = HIWORD(lpFixedFileInfo->dwFileVersionMS);
					dwMinorVersion = LOWORD(lpFixedFileInfo->dwFileVersionMS);
				}
			}
			LocalFree(lpBuffer);
		}
	}

	if (::GetVersionEx((OSVERSIONINFO *)&os))
	{
		// 根据文件信息判断
		switch (dwMajorVersion) // 判断文件主版本
		{
		case 4:
			switch (dwMinorVersion)//判断文件次版本号   
			{
			case 0:
				if (os.dwPlatformId == VER_PLATFORM_WIN32_NT)
					_tcscpy(szSystemName, _T("Windows NT 4.0"));
				else if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
					_tcscpy(szSystemName, _T("Windows 95"));
				break;
			case 10:
				_tcscpy(szSystemName, _T("Windows 98"));
				break;
			case 90:
				_tcscpy(szSystemName, _T("Windows Me"));
				break;
			}
			break;

		case 5:
			switch (dwMinorVersion)
			{
			case 0:
				_tcscpy(szSystemName, _T("Windows 2000"));
				break;
			case 1:
				_tcscpy(szSystemName, _T("Windows XP"));
				break;
			case 2:
				if (os.wProductType == VER_NT_WORKSTATION
					&& info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				{
					_tcscpy(szSystemName, _T("Windows XP Professional x64 Edition"));
				}
				else if (::GetSystemMetrics(SM_SERVERR2) == 0)
					_tcscpy(szSystemName, _T("Windows Server 2003"));
				else if (::GetSystemMetrics(SM_SERVERR2) != 0)
					_tcscpy(szSystemName, _T("Windows Server 2003 R2"));
				break;
			}
			break;

		case 6:
			switch (dwMinorVersion)
			{
			case 0:
				if (os.wProductType == VER_NT_WORKSTATION)
					_tcscpy(szSystemName, _T("Windows Vista"));
				else
					_tcscpy(szSystemName, _T("Windows Server 2008"));
				break;
			case 1:
				if (os.wProductType == VER_NT_WORKSTATION)
					_tcscpy(szSystemName, _T("Windows 7"));
				else
					_tcscpy(szSystemName, _T("Windows Server 2008 R2"));
				break;
			case 2:
				if (os.wProductType == VER_NT_WORKSTATION)
					_tcscpy(szSystemName, _T("Windows 8"));
				else
					_tcscpy(szSystemName, _T("Windows Server 2012"));
				break;
			case 3:
				if (os.wProductType == VER_NT_WORKSTATION)
					_tcscpy(szSystemName, _T("Windows 8.1"));
				else
					_tcscpy(szSystemName, _T("Windows Server 2012 R2"));
				break;
			}
			break;

		case 10:
			switch (dwMinorVersion)
			{
			case 0:
				if (os.wProductType == VER_NT_WORKSTATION)
					_tcscpy(szSystemName, _T("Windows 10 Insider Preview"));
				else
					_tcscpy(szSystemName, _T("Windows Server Technical Preview"));
				break;
			}
			break;

		default:
			break;
		}
	}//if(GetVersionEx((OSVERSIONINFO *)&os))
}

// 获取系统版本
void GetSystemName(__out TCHAR* szSystemName)
{
	SYSTEM_INFO info;
	::GetSystemInfo(&info);
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	_tcscpy(szSystemName, _T("Windows Unknown"));

	if (::GetVersionEx((OSVERSIONINFO *)&os))
	{
		// 根据信息判断
		switch (os.dwMajorVersion) // 主版本
		{
		case 4:
			switch (os.dwMinorVersion)//判断次版本号   
			{
			case 0:
				if (os.dwPlatformId == VER_PLATFORM_WIN32_NT)
					_tcscpy(szSystemName, _T("Windows NT 4.0"));
				else if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
					_tcscpy(szSystemName, _T("Windows 95"));
				break;
			case 10:
				_tcscpy(szSystemName, _T("Windows 98"));
				break;
			case 90:
				_tcscpy(szSystemName, _T("Windows Me"));
				break;
			}
			break;

		case 5:
			switch (os.dwMinorVersion)
			{
			case 0:
				_tcscpy(szSystemName, _T("Windows 2000"));
				break;
			case 1:
				_tcscpy(szSystemName, _T("Windows XP"));
				break;
			case 2:
				if (os.wProductType == VER_NT_WORKSTATION
					&& info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				{
					_tcscpy(szSystemName, _T("Windows XP Professional x64 Edition"));
				}
				else if (::GetSystemMetrics(SM_SERVERR2) == 0)
					_tcscpy(szSystemName, _T("Windows Server 2003")); 
				else if (::GetSystemMetrics(SM_SERVERR2) != 0)
					_tcscpy(szSystemName, _T("Windows Server 2003 R2"));
				break;
			}
			break;

		case 6:
			switch (os.dwMinorVersion)
			{
			case 0:
				if (os.wProductType == VER_NT_WORKSTATION)
					_tcscpy(szSystemName, _T("Windows Vista"));
				else
					_tcscpy(szSystemName, _T("Windows Server 2008"));
				break;
			case 1:
				if (os.wProductType == VER_NT_WORKSTATION)
					_tcscpy(szSystemName, _T("Windows 7"));
				else
					_tcscpy(szSystemName, _T("Windows Server 2008 R2"));
				break;
			case 2:
				if (os.wProductType == VER_NT_WORKSTATION)
					_tcscpy(szSystemName, _T("Windows 8"));
				else
					_tcscpy(szSystemName, _T("Windows Server 2012"));
				break;
			case 3:
				if (os.wProductType == VER_NT_WORKSTATION)
					_tcscpy(szSystemName, _T("Windows 8.1"));
				else
					_tcscpy(szSystemName, _T("Windows Server 2012 R2"));
				break;
			}
			break;

		case 10:
			switch (os.dwMinorVersion)
			{
			case 0:
				if (os.wProductType == VER_NT_WORKSTATION)
					_tcscpy(szSystemName, _T("Windows 10 Insider Preview"));
				else
					_tcscpy(szSystemName, _T("Windows Server Technical Preview"));
				break;
			}
			break;

		default:
			break;
		}
	}//if(GetVersionEx((OSVERSIONINFO *)&os))
}

// 获取操作系统具体版本比如专业版还是旗舰版
void GetSystemMark(__out TCHAR* szSystemMark)
{
	_tcscpy(szSystemMark, _T("Unknown"));

	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	typedef BOOL(WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
	PGPI pGPI = (PGPI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");

	if (pGPI == NULL)
		return;

	if (::GetVersionEx((OSVERSIONINFO *)&os))
	{
		DWORD dwType = 0;
		pGPI(os.dwMajorVersion, os.dwMinorVersion, 0, 0, &dwType);

		switch (dwType)
		{
		case PRODUCT_ULTIMATE:
			_tcscpy(szSystemMark, _T("Ultimate Edition"));
			break;
		case PRODUCT_PROFESSIONAL:
			_tcscpy(szSystemMark, _T("Professional"));
			break;
		case PRODUCT_HOME_PREMIUM:
			_tcscpy(szSystemMark, _T("Home Premium Edition"));
			break;
		case PRODUCT_HOME_BASIC:
			_tcscpy(szSystemMark, _T("Home Basic Edition"));
			break;
		case PRODUCT_ENTERPRISE:
			_tcscpy(szSystemMark, _T("Enterprise Edition"));
			break;
		case PRODUCT_BUSINESS:
			_tcscpy(szSystemMark, _T("Business Edition"));
			break;
		case PRODUCT_STARTER:
			_tcscpy(szSystemMark, _T("Starter Edition"));
			break;
		case PRODUCT_CLUSTER_SERVER:
			_tcscpy(szSystemMark, _T("Cluster Server Edition"));
			break;
		case PRODUCT_DATACENTER_SERVER:
			_tcscpy(szSystemMark, _T("Datacenter Edition"));
			break;
		case PRODUCT_DATACENTER_SERVER_CORE:
			_tcscpy(szSystemMark, _T("Datacenter Edition core installation"));
			break;
		case PRODUCT_ENTERPRISE_SERVER:
			_tcscpy(szSystemMark, _T("Enterprise Edition"));
			break;
		case PRODUCT_ENTERPRISE_SERVER_CORE:
			_tcscpy(szSystemMark, _T("Enterprise Edition core installation"));
			break;
		case PRODUCT_ENTERPRISE_SERVER_IA64:
			_tcscpy(szSystemMark, _T("Enterprise Edition for Itanium-based Systems"));
			break;
		case PRODUCT_SMALLBUSINESS_SERVER:
			_tcscpy(szSystemMark, _T("Small Business Server"));
			break;
		case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
			_tcscpy(szSystemMark, _T("Small Business Server Premium Edition"));
			break;
		case PRODUCT_STANDARD_SERVER:
			_tcscpy(szSystemMark, _T("Standard Edition"));
			break;
		case PRODUCT_STANDARD_SERVER_CORE:
			_tcscpy(szSystemMark, _T("Standard Edition core installation"));
			break;
		case PRODUCT_WEB_SERVER:
			_tcscpy(szSystemMark, _T("Web Server Edition"));
			break;
		default:
			break;
		}
	}
}

// 获取操作系统安装的服务包
void GetSystemServicePack(__out TCHAR* szSystemSP)
{
	_tcscpy(szSystemSP, _T("No Service Pack Installed"));
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (::GetVersionEx((OSVERSIONINFO *)&os))
	{
		if (_tcscmp(os.szCSDVersion, _T("")) != 0)
		{
			_tcscpy(szSystemSP, os.szCSDVersion);
		}
	}
}


void GetSystemDetailedInfo(__out TCHAR* szSystemInfo)
{     
	TCHAR szOsName[100] = { 0 };
	TCHAR szSP[50] = { 0 };
	TCHAR szOsBits[20] = { 0 };
	TCHAR szOsMark[100] = { 0 };
	GetSystemNameByKernel32(szOsName);
	GetSystemMark(szOsMark);
	GetSystemServicePack(szSP);
	GetSystemBits(szOsBits);

	wsprintf(szSystemInfo, _T("%s,%s,%s,%s"), szOsName, szSP, szOsMark, szOsBits);
}
