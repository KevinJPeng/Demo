#include "stdafx.h"
#include "CheckSystemDll.h"

#include <algorithm>
#include <Dbghelp.h>
#pragma comment(lib,  "Dbghelp.lib")

#include "md5.h"

#define MS_PROCESSOR_ARCHITECTURE_IA64             6
#define MS_PROCESSOR_ARCHITECTURE_AMD64            9

CCheckSystemDll::CCheckSystemDll()
{
	m_bLackDll = false;
	m_bRepairSucc = false;
	m_dllInfo.clear();
	m_tDllFinded.clear();
	m_vRecordUnfindedDll.clear();
}

CCheckSystemDll::CCheckSystemDll(T_CHECK_SYSTEM_DLL &tData)
{
	m_bLackDll = false;
	m_bRepairSucc = false;
	m_tData = tData;
	m_dllInfo.clear();
	m_tDllFinded.clear();
	m_vRecordUnfindedDll.clear();
}

CCheckSystemDll::~CCheckSystemDll(void)
{

}

BOOL CCheckSystemDll::CheckPro(void)
{	
	CString strOsName = _T("");
	CString strInstallDir = _T("");
	vector <CString> vecDllName;
	vector <CString> vecPeName;
	m_tData.vLackDllNames.clear();

	//保存用户系统版本号号位数
	getOsVersion(strOsName);
	int nSystemBits = GetSystemBits();
	m_tData.strOsName = strOsName;
	m_tData.nSystemBits = nSystemBits;

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("本机系统的版本号为：%s!系统的位数为：%d"),strOsName, nSystemBits);

	//获取bin目录下的文件名	
	strInstallDir.Format(_T("%s\\bin"), g_pGlobalData->dir.GetInstallDir());
	GetPeName(strInstallDir.GetBuffer(), vecPeName);
	strInstallDir.ReleaseBuffer();

	//遍历bin目录下的pe文件获得依赖项
	CString strPePath = _T("");
	for (int i = 0; i != vecPeName.size(); ++i)
	{
		strPePath = strInstallDir + _T("\\") + vecPeName[i];

		if (!GetDependsDll(strPePath, vecDllName))
		{
			//获取依赖dll失败
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("获取依赖dll名称失败!"));
			return -1;
		}
	}

	//删除重复的依赖项
	vector<CString>::iterator pos;
	sort(vecDllName.begin(), vecDllName.end());        
	pos = unique(vecDllName.begin(), vecDllName.end());   
	vecDllName.erase(pos, vecDllName.end());

	//遍历dll是否可以加载
	vector<CString> vecFailDllName;

	for (int j = 0; j != vecDllName.size(); ++j)
	{
		HMODULE hDll = LoadLibrary(vecDllName[j]);
		if (!hDll)
		{
			vecFailDllName.push_back(vecDllName[j]);
			continue;
		}

		FreeLibrary(hDll);
	}

	//缺少哪些dll
	if (!vecFailDllName.empty())
	{
		m_bLackDll = true;
		m_tData.vLackDllNames.swap(vecFailDllName);
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("系统缺少dll！"));

		return -1;
	}

	m_bLackDll = false;
	return 1;
}


BOOL CCheckSystemDll::RepairPro(void)
{
	if (!IsInternetConnect())
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("电脑网络连接失败!"));
		return false;
	}

	if (!DownloadXmlFile())
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("下载szlib.xml文件失败!"));
		return false;
	}

	if (!ReadRepairXml())
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("读取szlib.xml文件内容失败!"));
		return false;
	}

	//在szlib.xml文件中找到所需的dll信息
	FindDllInfo();

	if (!m_tDllFinded.empty())
	{
		bool bAllRunSucc = true; //标记所有需要运行的程序是否都是运行成功的

		for (int i = 0; i != m_tDllFinded.size(); ++i)
		{
			CString strLocalPePath = _T("");

			if (DownloadPeFile(m_tDllFinded[i].strLibPath, strLocalPePath))
			{
				//校验文件md5值
				if (!CompareFileMd5(strLocalPePath, m_tDllFinded[i].strMd5))
				{
					CString strInfo = _T("");
					strInfo.Format(_T("%s_%s_%s"), m_tDllFinded[i].strDllName, m_tDllFinded[i].strSysVersion, m_tDllFinded[i].strSysBits);
					g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("下载下来的pe文件md5值校验失败！具体信息为：%s"), strInfo);

					return false;
				}

				//判断文件是否需要执行
				if (m_tDllFinded[i].strRunFlag == _T("1"))
				{
					if (!RunExe(strLocalPePath, m_tDllFinded[i].strParam))
					{
						bAllRunSucc = false;
						DeleteFile(strLocalPePath);

						continue;
					}
					else
					{
						//删除运行后的文件
						DeleteFile(strLocalPePath);
					}			
				}

				//判断dll是否需要注册
				if (m_tDllFinded[i].strRunFlag == _T("2"))
				{
					if (!RegistDll(strLocalPePath, m_tDllFinded[i].strParam))
					{
						bAllRunSucc = false;

						DeleteFile(strLocalPePath);
						continue;
					}
				}
			}
			else
			{
				//有文件未下载下来，直接retern
				return false;
			}
		}

		if (!bAllRunSucc)
		{
			return false;
		}
	}

	//如果有在服务器上没找到的dll版本，生成一个0kb的文件上传到服务端
	//文件名格式为：系统版本号_系统位数_dll名.dll
	CString strPeName = _T("");
	CString strSvrPePath = _T("");

	for (int i = 0; i != m_vRecordUnfindedDll.size(); ++i)
	{
		strPeName.Format(_T("%s_%d_%s"), m_tData.strOsName, m_tData.nSystemBits, m_vRecordUnfindedDll[i]);
		strSvrPePath.Format(_T("/RepairSystemDll/noMatch/%s"), strPeName);
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("有缺失dll未在服务端找到相应信息，上传生成的0kb文件，文件名为%s"), strPeName);

		UploadPeFile(strPeName, strSvrPePath);
	}

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("重新加载bin目录下的所有程序依赖dll，看是否修复成功！"));
	if (ReloadDll())
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("重新加载bin目录下的所有程序依赖dll，修复成功！"));
		m_bRepairSucc = true;
	}

	return true;
}


bool CCheckSystemDll::RunExe(const CString &strExePath, const CString &strCommandLine)
{
	SHELLEXECUTEINFO ShExecInfo;

	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = strExePath; 
	ShExecInfo.lpParameters = strCommandLine; 
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL; 

	BOOL bRet = ShellExecuteEx(&ShExecInfo);

	if (!bRet)
	{
		m_bRepairSucc = false;
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("打开应用程序失败! err: %d"), GetLastError());

		return false;
	}

	if( ShExecInfo.hProcess != NULL)
	{
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
		CloseHandle(ShExecInfo.hProcess);		
		ShExecInfo.hProcess = NULL;
	}

	return true;
}


bool CCheckSystemDll::RegistDll(const CString &strDllPath, const CString &strCommandLine)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	CString strParam = _T("");
	CString strRegsvrPath = _T("%windir%\\system32\\regsvr32.exe");

	memset(&si, 0, sizeof(STARTUPINFO)); //初始化si在内存块中的值
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	strParam.Format(_T(" %s \"%s\""), strCommandLine, strDllPath);
	strRegsvrPath = GetDirectory(strRegsvrPath);

	if(CreateProcess(strRegsvrPath, strParam.GetBuffer(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		strParam.ReleaseBuffer();
		CloseHandle(pi.hThread);
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);			
	}
	else
	{
		strParam.ReleaseBuffer();
		m_bRepairSucc = false;
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("打开应用程序失败! err: %d"), GetLastError());

		return false;
	}

	return true;
}

//重新加载所有dll判断是否已修复
bool CCheckSystemDll::ReloadDll(void)
{
	CString strInstallDir = _T("");
	vector <CString> vecDllName;
	vector <CString> vecPeName;

	//获取bin目录下的文件名	
	strInstallDir.Format(_T("%s\\bin"), g_pGlobalData->dir.GetInstallDir());
	GetPeName(strInstallDir.GetBuffer(), vecPeName);
	strInstallDir.ReleaseBuffer();

	//遍历bin目录下的pe文件获得依赖项
	CString strPePath = _T("");
	for (int i = 0; i != vecPeName.size(); ++i)
	{
		strPePath = strInstallDir + _T("\\") + vecPeName[i];

		if (!GetDependsDll(strPePath, vecDllName))
		{
			//获取依赖dll失败
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("获取依赖dll名称失败!"));
			return false;
		}
	}

	//删除重复的依赖项
	vector<CString>::iterator pos;
	sort(vecDllName.begin(), vecDllName.end());        
	pos = unique(vecDllName.begin(), vecDllName.end());   
	vecDllName.erase(pos, vecDllName.end());

	//遍历dll是否可以加载
	vector<CString> vecFailDllName;

	for (int j = 0; j != vecDllName.size(); ++j)
	{
		HMODULE hDll = LoadLibrary(vecDllName[j]);
		if (!hDll)
		{
			vecFailDllName.push_back(vecDllName[j]);
			continue;
		}

		FreeLibrary(hDll);
	}

	if (!vecFailDllName.empty())
	{
		return false;
	}
	
	return true;
}

void CCheckSystemDll::GetCheckData(T_CHECK_SYSTEM_DLL &tData)
{
	tData.vLackDllNames.clear();
	tData = m_tData;
}

bool CCheckSystemDll::DownloadXmlFile(void)
{
	CFTP ftp;
	CString strInitPath = _T("");
	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());

	GetFtpLoginInfo(strInitPath);

	if (!ftp.Connect(m_ftpLoginInfo))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接ftp服务器失败！错误信息为：%s"), ftp.GetErrorMessage());
		return false;
	}

	CString strLocalPath = _T("");
	CString strSvrPath = _T("/RepairSystemDll/SZlib.xml");
	strLocalPath.Format(_T("%s\\SZlib.xml"), g_pGlobalData->dir.GetInstallDir());

	//下载保存含有dll信息的xml文件
	if (!ftp.GetFile(strSvrPath, strLocalPath))
	{
		return false;
	}

	return true;
}


bool CCheckSystemDll::DownloadPeFile(const CString &strPePath, CString &strLocalPath)
{
	CFTP ftp;
	CString strInitPath = _T("");
	CString strPeName = _T("");
	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());

	GetFtpLoginInfo(strInitPath);

	if (!ftp.Connect(m_ftpLoginInfo))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接ftp服务器失败！错误信息为：%s"), ftp.GetErrorMessage());
		return false;
	}

	int iPos = strPePath.ReverseFind(_T('/'));
	if (iPos != -1)
	{
		strPeName = strPePath.Right(strPePath.GetLength() - iPos - 1);
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("未获取到下载下来的pe文件的名称！"));
		return false;
	}
	strLocalPath.Format(_T("%s\\bin\\%s"), g_pGlobalData->dir.GetInstallDir(), strPeName);

	//下载保存pe文件
	if (!ftp.GetFile(strPePath, strLocalPath))
	{
		return false;
	}

	return true;
}


bool CCheckSystemDll::UploadPeFile(const CString &strPeName, const CString &strPePath)
{
	CFTP ftp;
	CString strInitPath = _T("");
	CString strLocalPath = _T("");
	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());

	GetFtpLoginInfo(strInitPath);

	if (!ftp.Connect(m_ftpLoginInfo))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接ftp服务器失败！错误信息为：%s"), ftp.GetErrorMessage());
		return false;
	}

	strLocalPath.Format(_T("%s\\bin\\%s"), g_pGlobalData->dir.GetInstallDir(), strPeName);

	HANDLE hFile = CreateFile(strLocalPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	CloseHandle(hFile);

	//上传dll
	if (!ftp.UpLoadFile(strPePath, strLocalPath))
	{
		return false;
	}

	DeleteFile(strLocalPath);

	return true;
}


void CCheckSystemDll::GetBackCheckStruct(T_PROBLEM_DATA &tData)
{
	tData.strIndex = _T("1");
	
	if (m_bLackDll)
	{
		tData.bIsRepair = true;
		int nCount = 0;
		CString strData = _T("");

		for (int i = 0; i != m_tData.vLackDllNames.size(); ++i)
		{
			++nCount;

			strData += m_tData.vLackDllNames[i];
			strData += _T("\,");
		}

		if (nCount == 1)
		{
			tData.strSuggestion.Format(ProblemCheckDllSuggest, strData);
		} 
		else
		{
			CString strTemp = _T("");
			strTemp.Format(_T("%s等依赖文件，"), m_tData.vLackDllNames[0]);
			tData.strSuggestion.Format(ProblemCheckDllSuggest, strTemp);
		}
		
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("您的电脑缺少%s,建议您点击修复"), strData);
	}
	else
	{
		tData.bCheckFlag = true;
	}
}

void CCheckSystemDll::GetBackRepairStruct(T_PROBLEM_DATA &tData)
{
	tData.strIndex = _T("1");

	if (m_bRepairSucc)
	{
		tData.bRepairFlag = true;
	}
}


void CCheckSystemDll::FindDllInfo(void)
{
	CString strSystemBits = _T("");

	CString strOsName = m_tData.strOsName;
	strSystemBits.Format(_T("%d"), m_tData.nSystemBits);
	
	//通过对比系统版本，位数，dll名从szlib.xml中找到相应的信息
	for (int i = 0; i != m_tData.vLackDllNames.size(); ++i)
	{
		CString strInfo = _T("");
		strInfo.Format(_T("%s_%s_%d"), m_tData.vLackDllNames[i], m_tData.strOsName, m_tData.nSystemBits);
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("当前去xml查找缺失的文件路径！，具体查找的信息为：%s"), strInfo);

		bool bFind = false;
		for (int j = 0; j != m_dllInfo.size(); ++j)
		{
			//当系统匹配标志为true时dll名和系统版本位数都要匹配，匹配标志为false时只需匹配dll名
			if ((!m_tData.vLackDllNames[i].CompareNoCase(m_dllInfo[j].strDllName) 
				&& !m_dllInfo[j].strSysMatchFlag.CompareNoCase(_T("true"))
				&& !strOsName.CompareNoCase(m_dllInfo[j].strSysVersion)
				&& strSystemBits == m_dllInfo[j].strSysBits)
				||
				(!m_tData.vLackDllNames[i].CompareNoCase(m_dllInfo[j].strDllName) 
				&& !m_dllInfo[j].strSysMatchFlag.CompareNoCase(_T("false"))))
			{
				bFind = true;
				m_tDllFinded.push_back(m_dllInfo[j]);
				break;
			}
		}

		if (!bFind)
		{
			//保存未找到的dll
			m_vRecordUnfindedDll.push_back(m_tData.vLackDllNames[i]);
		}
	}
}


/*
@brief  获取依赖的dll
@param  szPePath 指定pe文件路径
@param  [out]vecPeName  依赖的dll文件名
*/
bool CCheckSystemDll::GetDependsDll(CString strPePath, vector <CString> &vecDllName)
{
	HANDLE hFile = CreateFile(strPePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("Create File Failed! Err: %d"), GetLastError());
		return false;
	}

	HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

	if (hFileMapping == NULL || hFileMapping == INVALID_HANDLE_VALUE) 
	{ 
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("Could not create file mapping object! Err: %d"), GetLastError());
		return false;
	}

	LPBYTE lpBaseAddress = (LPBYTE)MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	if (lpBaseAddress == NULL) 
	{ 
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("Could not map view of file! Err: %d"), GetLastError());
		return false;
	}

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(lpBaseAddress + pDosHeader->e_lfanew);

	//导入表的rva
	DWORD Rva_import_table = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

	if(Rva_import_table == 0)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("no import table!"));
		goto UNMAP_AND_EXIT;
	}

	//获取文件地址
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)ImageRvaToVa(pNtHeaders, lpBaseAddress, Rva_import_table, NULL);

	//现在来到了导入表的面前：IMAGE_IMPORT_DESCRIPTOR 数组（以0元素为终止）
	//定义表示数组结尾的null元素！
	IMAGE_IMPORT_DESCRIPTOR null_iid;
	memset(&null_iid, 0, sizeof(null_iid));

	//每个元素代表了一个引入的DLL。
	for(int i = 0; memcmp(pImportTable + i, &null_iid, sizeof(null_iid)) != 0; ++i)
	{
		LPCSTR szDllName = (LPCSTR)ImageRvaToVa(pNtHeaders, lpBaseAddress, pImportTable[i].Name, NULL);
		vecDllName.push_back(CString(szDllName));
	}

UNMAP_AND_EXIT:

	UnmapViewOfFile(lpBaseAddress);
	CloseHandle(hFileMapping);
	CloseHandle(hFile);

	return true;
}


/*
@brief  遍历指定目录获取pe文件名称
@param  strDirName 指定路径
@param  [out]vecPeName  存储pe文件名
*/
void CCheckSystemDll::GetPeName(TCHAR* strDirName, vector <CString> &vecPeName)
{   
	CFileFind tempFind;
	CString strTempFileFind = _T("");

	strTempFileFind.Format(_T("%s\\*.*"), strDirName);

	BOOL IsFinded = tempFind.FindFile(strTempFileFind);
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();

		if (!tempFind.IsDots() && !tempFind.IsDirectory()) 
		{
			CString strFileName = tempFind.GetFileName();
			CString strFileExtension = strFileName.Right(4);

			if (!strFileExtension.CompareNoCase(_T(".dll")) || !strFileExtension.CompareNoCase(_T(".exe")))
			{
				//不需要加载自己,key.dll是个配置文件
				if (strFileName == _T("CheckSystemDllExist.exe") || strFileName == _T("KeyInfo.dat"))
				{
					continue;
				}
				vecPeName.push_back(strFileName);
			}	
		}
	}

	tempFind.Close();
}


void CCheckSystemDll::getOsVersion(CString &strOSName)
{
	DWORD  dwMajorVersion;
	DWORD  dwMinorVersion;
	DWORD  dwBuildNumber;
	DWORD  dwPlatformId;
	OSVERSIONINFO osvi;                 //定义OSVERSIONINFO数据结构对象
	memset(&osvi, 0, sizeof(OSVERSIONINFO));              //开空间 
	osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);    //定义大小 
	GetVersionEx (&osvi);                  //获得版本信息 
	dwMajorVersion = osvi.dwMajorVersion;    //主版本号
	dwMinorVersion = osvi.dwMinorVersion;    //副版本
	dwBuildNumber = osvi.dwBuildNumber;      //创建号
	dwPlatformId = osvi.dwPlatformId;        //ID号
	char swVersion[10] = {0};   
	CString strVersion = _T("");
	strVersion.Format(_T("%d.%d"), dwMajorVersion, dwMinorVersion);

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("查找到的windows版本号为:%s!"), strVersion);

	if (strVersion == _T("4.0"))  
	{
		strOSName = _T("win95");      //win95
		return;
	}
	if (strVersion == _T("4.1"))  
	{
		strOSName = _T("win98 ");     //win98 
		return;
	}
	if (strVersion == _T("4.9"))  
	{
		strOSName = _T("win_me");     // win_me 
		return;
	}
	if (strVersion == _T("3.51")) 
	{
		strOSName = _T("win_Nt_3_5"); //win_Nt_3_5 
		return;
	}
	if (strVersion == _T("5.0"))  
	{
		strOSName = _T("win2000");    //win2000   
		return;
	}
	if (strVersion == _T("5.1"))  
	{
		strOSName = _T("win_xp");     //win_xp 
		return;
	}
	if (strVersion == _T("5.2"))  
	{
		strOSName = _T("win2003");    // win2003 
		return;
	}
	if (strVersion == _T("6.0"))  
	{
		strOSName = _T("vista");      //vista
		return;
	}
	if (strVersion == _T("6.1"))  
	{
		strOSName = _T("win7");       // win7 
		return;
	}
	if (strVersion == _T("6.2"))  
	{
		strOSName = _T("win8");      // win8
		return;
	}
	if (strVersion == _T("6.3"))  
	{
		strOSName = _T("win8.1");      // win8.1
		return;
	}
}


// 安全的取得真实系统信息
VOID CCheckSystemDll::SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo)
{
	if (NULL == lpSystemInfo) 
	{
		return;
	}

	typedef VOID (WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
	LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress( GetModuleHandle(_T("kernel32")), "GetNativeSystemInfo");
	if (NULL != fnGetNativeSystemInfo)
	{
		fnGetNativeSystemInfo(lpSystemInfo);
	}
	else
	{
		GetSystemInfo(lpSystemInfo);
	}
}

//获取操作系统位数
int CCheckSystemDll::GetSystemBits(void)
{
	SYSTEM_INFO si;
	SafeGetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == MS_PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == MS_PROCESSOR_ARCHITECTURE_IA64 )

	{
		return 64;
	}
	return 32;
}


void CCheckSystemDll::GetFtpLoginInfo(const CString &strIniPath)
{
	TCHAR szIp[100];
	TCHAR szUserName[100];
	TCHAR szPwd[100];
	CString szInfo = _T("");
	int   iPort;
	IKeyRW key;
	key.InitDll(KTYPE_CFG);

	memset(szIp, 0, sizeof(szIp));
	memset(szUserName, 0, sizeof(szUserName));
	memset(szPwd, 0, sizeof(szPwd));

	IXMLRW xml;
	xml.init(strIniPath);

	xml.ReadString(_T("MC"), _T("RepairSystemDll"), _T("ftp"), szInfo);

	CString strDecrypt;

	if (szInfo == _T(""))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("读取mcconfig中的RepairSystemDll_ftp登陆信息为空!"));
		return;
	}

	key.DecryptData(szInfo, DECRYPTCFG, strDecrypt);

	if (strDecrypt.GetLength() > 0)
	{
		GetFtpDesCode(strDecrypt.GetBuffer(0), szIp, szUserName, szPwd, iPort);
		strDecrypt.ReleaseBuffer();
	}

	m_ftpLoginInfo.strServer = szIp;
	m_ftpLoginInfo.strUser = szUserName;
	m_ftpLoginInfo.strPwd = szPwd;
	m_ftpLoginInfo.nPort = iPort;
	m_ftpLoginInfo.bPassive = TRUE;
}


void CCheckSystemDll::GetFtpDesCode(TCHAR *code, TCHAR *ip, TCHAR *account, TCHAR *psd, int &port)
{
	int i = 0;
	TCHAR *p[4];
	TCHAR *buf = code;
	while((p[i]=_tcstok(buf, _T(":"))) != NULL) 
	{
		if(i == 0)
		{
			_sntprintf(ip, _TRUNCATE, _T("%s"), p[i]);
		}
		if(i == 1)
		{
			TCHAR strport[10] = {0};
			_sntprintf(strport, _TRUNCATE, _T("%s"), p[i]);
			port = _wtoi(strport);
		}
		if(i == 2)
		{
			_sntprintf(account, _TRUNCATE, _T("%s"), p[i]);
		}
		if(i == 3)
		{
			_sntprintf(psd, _TRUNCATE, _T("%s"), p[i]);
		}
		i++;
		if(i == 4)
			break;
		buf=NULL; 
	}
}


bool CCheckSystemDll::IsInternetConnect()
{	
	DWORD dwFlags = 0;
	if (!InternetGetConnectedState(&dwFlags, 0))
	{
		return false;
	}
	return true;
}


bool CCheckSystemDll::ReadRepairXml(void)
{
	//读出字符串
	CStdioFile file;
	CString strXmlPath = _T("");
	strXmlPath.Format(_T("%s\\SZlib.xml"), g_pGlobalData->dir.GetInstallDir());

	if (!file.Open(strXmlPath, CStdioFile::modeReadWrite | CStdioFile::typeBinary))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("无法打开SZlib.xml文件，err：%d！"), GetLastError());
		return FALSE;
	}


	int iMaxXMLen = 256 * 1024;       //256k
	char *ptmpBuf = NULL;

	try
	{
		int nLen = file.GetLength();
		ptmpBuf = new char[nLen + 1];
		memset(ptmpBuf, 0, nLen + 1);
		file.Read(ptmpBuf, nLen);

		file.Close();

		TiXmlDocument doc;
		doc.Parse(ptmpBuf);

		delete []ptmpBuf;
		ptmpBuf = NULL;

		if (!ParseXmlInfo(&doc, m_dllInfo))
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("解析ProblemList.xml文件中的数据失败！"));
			return false;
		}
	}
	catch (...)
	{
		if (!ptmpBuf)
		{
			delete []ptmpBuf;
			ptmpBuf = NULL;
		}

		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("解析ProblemList.xml文件中的数据异常！"));
		return false;
	}

	return true;
}


//解析ProblemList.xml中的数据
bool CCheckSystemDll::ParseXmlInfo(TiXmlDocument *pDoc, vector<T_DLL_PATH_INFO> &tXmlInfo)
{
	if (!pDoc)
	{
		return false;
	}

	TiXmlElement *pRoot = pDoc->FirstChildElement("root");

	if (!pRoot)
	{
		return false;
	}

	TiXmlElement *pItem = pRoot->FirstChildElement("file");

	if (!pItem)
	{
		return false;
	}

	//遍历读取命令列表
	for (; pItem; pItem = pItem->NextSiblingElement())
	{
		T_DLL_PATH_INFO tData;
		tData.strDllName = pItem->Attribute("name");
		tData.strMd5 = pItem->Attribute("md5");
		tData.strSysMatchFlag = pItem->Attribute("sysmatchflag");

		if (tData.strSysMatchFlag.IsEmpty())
		{
			tData.strSysMatchFlag = _T("false");
		}

		tData.strSysVersion = pItem->Attribute("sysversion");
		tData.strSysBits = pItem->Attribute("sysbits");
		tData.strLibPath = pItem->Attribute("libpath");
		tData.strRunFlag = pItem->Attribute("runflg");
		tData.strParam = pItem->Attribute("param");


		tXmlInfo.push_back(tData);
	}

	return true;
}


BOOL CCheckSystemDll::GetFileMD5(const CString &strFile, CString &strMd5)
{
	//文件不存在直接返回
	if (!PathFileExists(strFile))
		return FALSE;

	//已存在则比较MD5确认是否是最新
	HANDLE hFile = CreateFile(strFile, 
		GENERIC_READ, 
		FILE_SHARE_READ ,NULL,
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL); 

	if(hFile==INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD m_dwBUFSize = GetFileSize(hFile, NULL);

	HANDLE hMapFile = CreateFileMapping(
		hFile,                   // use paging file
		NULL,                    // default security 
		PAGE_READONLY,           // read/write access
		0,                       // max. object size 
		m_dwBUFSize,             // buffer size  
		NULL);                   // name of mapping object

	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) 
	{ 
		_tprintf(_T("Could not create file mapping object with %s, error(%d).\n"), strFile, GetLastError());
		CloseHandle(hFile);
		return FALSE;
	}
	PBYTE m_pByteBuf = (PBYTE) MapViewOfFile(
		hMapFile,                 // handle to mapping object
		FILE_MAP_READ,				// read/write permission
		0,                   
		0,                   
		m_dwBUFSize); 

	if (m_pByteBuf == NULL)
	{
		CloseHandle(hMapFile);
		CloseHandle(hFile);
		return FALSE;
	}

#define MD5_LENGTH      34
	char szMD5[MD5_LENGTH] = {0};
	TCHAR tszMD5[MD5_LENGTH] = {0};
	GetMD5Code(m_pByteBuf, m_dwBUFSize, szMD5);

	UnmapViewOfFile(m_pByteBuf);
	CloseHandle(hMapFile);
	CloseHandle(hFile);

	strMd5 = szMD5;

	return TRUE;
}


bool CCheckSystemDll::CompareFileMd5(const CString &strLocalPePath, CString &strMd5FromXml)
{
	CString strMd5 = _T("");

	if (GetFileMD5(strLocalPePath, strMd5))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("下载的pe文件的md5值为:%s！xml文件读出来的md5值为:%s!"), strMd5.GetBuffer(), strMd5FromXml.GetBuffer());
		strMd5.ReleaseBuffer();
		strMd5FromXml.ReleaseBuffer();

		if (strMd5.CompareNoCase(strMd5FromXml))
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("下载的pe文件md5值与从xml文件读出来的md5值不匹配！"));
			DeleteFile(strLocalPePath);
			return false;
		}

	}
	else
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("未能获取下载的pe文件的md5值！"));
		DeleteFile(strLocalPePath);
		return false;
	}

	return true;
}


CString CCheckSystemDll::GetDirectory(CString strShortDir)
{
	CString strRet = _T("");
	CString strVar = _T("");
	CString strSingleFlag = _T("");
	CString strTotalFlag = _T("");      //整个通配符，如#SETUP#

FindFlag:
	int iFirstFlagPos = strShortDir.Find(_T('#'));
	int iEndFlagPos = 0;
	if (-1 != iFirstFlagPos)
	{
		iEndFlagPos = strShortDir.Find(_T('#'), iFirstFlagPos + 1);
		strVar = strShortDir.Mid(iFirstFlagPos + 1, iEndFlagPos - iFirstFlagPos - 1);
		strSingleFlag = _T("#");
		strTotalFlag = strShortDir.Mid(iFirstFlagPos, iEndFlagPos - iFirstFlagPos + 1);
	}

	//找不到#XXX#时再找%XXX%
	if (-1 == iFirstFlagPos || -1 == iEndFlagPos)
	{
		iFirstFlagPos = strShortDir.Find(_T('%'));
		if (-1 != iFirstFlagPos)
		{
			iEndFlagPos = strShortDir.Find(_T('%'), iFirstFlagPos + 1);
			strVar = strShortDir.Mid(iFirstFlagPos + 1, iEndFlagPos - iFirstFlagPos - 1);
			strSingleFlag = _T("%");
			strTotalFlag = strShortDir.Mid(iFirstFlagPos, iEndFlagPos - iFirstFlagPos + 1);
		}
	}

	//找不到通配符原串返回
	if (-1 == iFirstFlagPos || -1 == iEndFlagPos || strVar.IsEmpty())
		return strShortDir;

	if (strSingleFlag == _T("#"))
	{
		if (!strVar.CompareNoCase(_T("SETUP")))
		{
			strRet = GetProgPath();
		}
		else if (!strVar.CompareNoCase(_T("NULL")))
		{
			strRet = _T("");
		}
	}
	else if (strSingleFlag == _T("%"))
	{
		if (!GetEnvironmentVariable(strVar, strRet.GetBuffer(MAX_PATH), MAX_PATH))
		{
			//MessageBox(_T("未找到环境变量！"));
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("未找到环境变量 %s,目录未替换！"), strTotalFlag);
		}
		strRet.ReleaseBuffer();
	}


	//将替换后的目录加上目录变量之后的路径
	strShortDir.Replace(strTotalFlag, strRet);
	goto FindFlag;           //继续查找下一个通配符
}