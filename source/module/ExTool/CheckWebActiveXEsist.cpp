#include "stdafx.h"
#include "CheckWebActiveXEsist.h"
#include <WinInet.h>


CCheckWebActiveXExist::CCheckWebActiveXExist(void)
{
	m_iCheckcnt = 0;
	m_bCheckFlag = false;
	m_bRepairSucc = false;
	m_vRegDll.clear();
}

CCheckWebActiveXExist::~CCheckWebActiveXExist(void)
{

};


BOOL CCheckWebActiveXExist::CheckPro(void)
{
	CString strWebActiveXPluginPath = _T("");
	CString strNpSumPluginPath = _T("");

	strWebActiveXPluginPath.Format(_T("%s\\plugins\\ieplugin.dll"), g_pGlobalData->dir.GetInstallDir());
	strNpSumPluginPath.Format(_T("%s\\plugins\\npszwplugin.dll"), g_pGlobalData->dir.GetInstallDir());

	if (!PathFileExists(strWebActiveXPluginPath) || !PathFileExists(strNpSumPluginPath))
	{
		return FALSE;
	}

	m_bCheckFlag = true;
	return TRUE;
}

BOOL CCheckWebActiveXExist::RepairPro(void)
{	
	m_vRegDll.clear();
	if (!IsInternetConnect())
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("电脑网络连接失败!"));
		return false;
	}

	if (!DownLoadDatFile())
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("下载dat失败!"));
		return false;
	}

	if (!DownloadDllFile())
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("下载web控件失败!"));
		return false;
	}

	if (!RegistDll())
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("注册dll失败!"));
		return false;
	}

	m_bRepairSucc = true;
	return TRUE;
}

//由于目前在后台处理了插件丢失情况，所以此处默认都是返回成功即可
void CCheckWebActiveXExist::GetBackCheckStruct(T_PROBLEM_DATA &tData)
{
	tData.strIndex = _T("2");
	
	//if (!m_bCheckFlag)
	//{
	//	tData.bIsRepair = true;
	//	tData.strSuggestion = ProblemCheckWebActiveX;
	//}
	//else
	//{
		tData.bCheckFlag = true;
	/*}*/
}

void CCheckWebActiveXExist::GetBackRepairStruct(T_PROBLEM_DATA &tData)
{
	tData.strIndex = _T("2");

	if (m_bRepairSucc)
	{
		tData.bRepairFlag = true;
	}
}

bool CCheckWebActiveXExist::IsInternetConnect()
{	
	DWORD dwFlags = 0;
	if (!InternetGetConnectedState(&dwFlags, 0))
	{
		return false;
	}
	return true;
}


bool CCheckWebActiveXExist::DownLoadDatFile()
{
	CFTP ftp;
	vector<CString> vDllPath;
	CString strInitPath = _T("");
	CString strLocalUpdatePath = _T("");
	CString strLocalMCPath = _T("");

	if (m_iCheckcnt < 1440)
	{
		m_iCheckcnt++;
		return true;
	}

	m_iCheckcnt = 0;
	
	strLocalMCPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());
	strLocalUpdatePath.Format(_T("%s\\data2\\UpdateOL.dat"), g_pGlobalData->dir.GetInstallDir());

	IXMLRW xml;
	xml.init(strLocalMCPath);
	if (XRET_SUCCESS != xml.ReadString(_T("MC"), _T("Innertest"), _T("host"), strInitPath))
	{
		CString strMcPath = _T("/客户端安全升级/DownloadFiles/UpdateFilesQY2.0/MasterZ/data2/mc.dat");
		vDllPath.push_back(strMcPath);
	}

	xml.init(strLocalUpdatePath);
	if (XRET_SUCCESS != xml.ReadString(_T("UpdateOL"), _T("server"), _T("port"), strInitPath))
	{
		CString strUpdatOLPath = _T("/客户端安全升级/DownloadFiles/UpdateFilesQY2.0/MasterZ/data2/UpdateOL.dat");
		vDllPath.push_back(strUpdatOLPath);
	}

	if (vDllPath.size() <= 0)
	{
		return true;
	}

	if (m_ftpLoginInfo.strServer.GetLength() < 7)
		GetFtpLoginInfo(strLocalMCPath);

	if (!ftp.Connect(m_ftpLoginInfo))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接ftp服务器失败！错误信息为：%s"), ftp.GetErrorMessage());
		return false;
	}

	for (int i = 0; i != vDllPath.size(); ++i)
	{
		int iPos = 0;
		CString strLocalPath = _T("");
		CString strPeName = _T("");
		CString strPePath = vDllPath[i];

		//获取下载的文件名
		iPos = strPePath.ReverseFind(_T('/'));
		if (iPos != -1)
		{
			strPeName = strPePath.Right(strPePath.GetLength() - iPos - 1);
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("未获取到下载下来的pe文件的名称！路径为：%s"), strPePath);
			return false;
		}
		strLocalPath.Format(_T("%s\\data2\\%s"), g_pGlobalData->dir.GetInstallDir(), strPeName);

		//下载保存pe文件
		if (!ftp.GetFile(strPePath, strLocalPath))
		{
			return false;
		}
	}

	return true;
}


bool CCheckWebActiveXExist::DownloadDllFile()
{
	CFTP ftp;
	vector<CString> vDllPath;
	CString strInitPath = _T("");
	CString strLocalWebDllPath = _T("");
	CString strLocalNpDllPath = _T("");
	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());

	strLocalWebDllPath.Format(_T("%s\\plugins\\ieplugin.dll"), g_pGlobalData->dir.GetInstallDir());
	strLocalNpDllPath.Format(_T("%s\\plugins\\npszwplugin.dll"), g_pGlobalData->dir.GetInstallDir());

	//判断缺少哪个dll
	if (!PathFileExists(strLocalWebDllPath))
	{
		CString strWebDllPath = _T("/客户端安全升级/DownloadFiles/UpdateFilesQY2.0/MasterZ/plugins/ieplugin.dll");
		vDllPath.push_back(strWebDllPath);
	}
	if (!PathFileExists(strLocalNpDllPath))
	{
		CString strNpPath = _T("/客户端安全升级/DownloadFiles/UpdateFilesQY2.0/MasterZ/plugins/npszwplugin.dll");
		vDllPath.push_back(strNpPath);
	}

	if (vDllPath.empty())
	{
		return true;
	}
	

	GetFtpLoginInfo(strInitPath);

	if (!ftp.Connect(m_ftpLoginInfo))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接ftp服务器失败！错误信息为：%s"), ftp.GetErrorMessage());
		return false;
	}

	for (int i = 0; i != vDllPath.size(); ++i)
	{
		int iPos = 0;
		CString strLocalPath = _T("");
		CString strPeName = _T("");
		CString strPePath = vDllPath[i];

		//获取下载的文件名
		iPos = strPePath.ReverseFind(_T('/'));
		if (iPos != -1)
		{
			strPeName = strPePath.Right(strPePath.GetLength() - iPos - 1);
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("未获取到下载下来的pe文件的名称！路径为：%s"), strPePath);
			return false;
		}
		strLocalPath.Format(_T("%s\\plugins\\%s"), g_pGlobalData->dir.GetInstallDir(), strPeName);

		//记录需要注册的dll
		m_vRegDll.push_back(strLocalPath);

		//下载保存pe文件
		if (!ftp.GetFile(strPePath, strLocalPath))
		{
			return false;
		}
	}

	return true;
}


void CCheckWebActiveXExist::GetFtpLoginInfo(const CString &strIniPath)
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

/*
	if (szInfo == _T(""))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("读取mcconfig中的RepairSystemDll_ftp登陆信息为空!"));
		return;
	}*/

	key.DecryptData(szInfo, DECRYPTCFG, strDecrypt);

	if (strDecrypt.GetLength() > 0)
	{
		GetFtpDesCode(strDecrypt.GetBuffer(0), szIp, szUserName, szPwd, iPort);
		strDecrypt.ReleaseBuffer();

		m_ftpLoginInfo.strServer = szIp;
		m_ftpLoginInfo.strUser = szUserName;
		m_ftpLoginInfo.strPwd = szPwd;
		m_ftpLoginInfo.nPort = iPort;
		m_ftpLoginInfo.bPassive = TRUE;
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("读取mcconfig中的RepairSystemDll_ftp登陆信息为空,使用默认FTP!"));

		m_ftpLoginInfo.strServer = _T("112.74.102.50");
		m_ftpLoginInfo.strUser = _T("upgradeSer");
		m_ftpLoginInfo.strPwd = _T("Us$@#^%&88");
		m_ftpLoginInfo.nPort = 21;
		m_ftpLoginInfo.bPassive = TRUE;
	}
}


void CCheckWebActiveXExist::GetFtpDesCode(TCHAR *code, TCHAR *ip, TCHAR *account, TCHAR *psd, int &port)
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

bool CCheckWebActiveXExist::RegistDll()
{	
	if (m_vRegDll.empty())
	{
		return true;
	}
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	CString strParam = _T("");
	CString strRegsvrPath = _T("%windir%\\system32\\regsvr32.exe");

	memset(&si, 0, sizeof(STARTUPINFO)); //初始化si在内存块中的值
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	for (int i = 0; i != m_vRegDll.size(); ++i)
	{
		strParam.Format(_T(" /s \"%s\""), m_vRegDll[i]);
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
			DeleteFile(m_vRegDll[i]);
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("注册dll失败! err: %d"), GetLastError());

			return false;
		}
	}	

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("注册dll成功! "));
	return true;
}


CString CCheckWebActiveXExist::GetDirectory(CString strShortDir)
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
