#include "stdafx.h"
#include "CheckSvrConnect.h"

//默认登录API
const TCHAR* const URL_DEFAULT_LOGINAPI = _T("http://api.sumszw.com");

const TCHAR* const QUICK_PHOTO_SVR = _T("快照");

const TCHAR* const API_SVR = _T("API");

const TCHAR* const BAIDU_SVR = _T("百度");

const TCHAR* const SEO_SVR = _T("SEO网站综合查询");


CCheckSvrConnect::CCheckSvrConnect(void)
{

}
CCheckSvrConnect::~CCheckSvrConnect(void)
{

}

BOOL CCheckSvrConnect::CheckPro(void)
{
	if (IsInternetConnect())
	{
		m_svrConnect.bNetConnect = true;
		m_svrConnect.bQuickPhotoSvr = CheckQuickPhotoSvr();
		m_svrConnect.bApiSvr = CheckApiSvr();
		m_svrConnect.bBaiDuSvr = CheckBaiDuSvr();
		m_svrConnect.bSeoSvr = CheckSeoSvr();
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("电脑网络连接失败！"));
		m_svrConnect.bNetConnect = false;
	}
	return TRUE;
}

BOOL CCheckSvrConnect::RepairPro(void)
{
	return TRUE;
}

void CCheckSvrConnect::GetBackCheckStruct(T_PROBLEM_DATA &tData)
{
	tData.strIndex = _T("2");
	if (m_svrConnect.bNetConnect)
	{
		if (m_svrConnect.bQuickPhotoSvr && m_svrConnect.bApiSvr && m_svrConnect.bBaiDuSvr && m_svrConnect.bSeoSvr)
		{
			tData.bCheckFlag = true;
		}
		else
		{//格式化建议信息
			map<CString, bool> mapSvr;
			vector<CString> vData;
			vData.clear();
			mapSvr.clear();
			tData.bCheckFlag = false;
			CString strData = _T("");

			mapSvr[QUICK_PHOTO_SVR] =  m_svrConnect.bQuickPhotoSvr;
			mapSvr[API_SVR] =  m_svrConnect.bApiSvr;
			mapSvr[BAIDU_SVR] =  m_svrConnect.bBaiDuSvr;
			mapSvr[SEO_SVR] =  m_svrConnect.bSeoSvr;

			for (map<CString, bool>::iterator iter = mapSvr.begin(); iter != mapSvr.end(); ++iter)
			{
				if (!(iter->second))
				{
					vData.push_back(iter->first);
				}
			}
			
			int nCount = 0;
			for (int i = 0; i != vData.size(); ++i)
			{
				++nCount;

				strData += vData[i];

				if (nCount == 1)
				{
					strData += _T("服务器");
				}

				strData += _T("\,");
			}

			CString strTemp = _T("");
			if (nCount != 1)
			{
				strTemp.Format(_T("%s等服务器,"), vData[0]);
				tData.strSuggestion.Format(ProblemCheckSvrConSuggest, strTemp);
			} 
			else
			{
				tData.strSuggestion.Format(ProblemCheckSvrConSuggest, strData);
			}	

			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接不上的服务器为：%s"), strData);
		}
	}
	else
	{
		tData.strSuggestion = ProblemCheckNetConnect;
		tData.bCheckFlag = false;
	}
}


void CCheckSvrConnect::GetBackRepairStruct(T_PROBLEM_DATA &tData)
{
	tData.strIndex = _T("2");
}

//检测快照服务器连接是否正常
bool CCheckSvrConnect::CheckQuickPhotoSvr(void)
{
	CFTP ftp;
	CString strInitPath = _T("");
	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());

	GetFtpLoginInfo(strInitPath);

	if (!ftp.Connect(m_ftpLoginInfo))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接ftp快照服务器失败！错误信息为：%s"), ftp.GetErrorMessage());
		return false;
	}

	return true;
}

//检测api服务器连接是否正常
bool CCheckSvrConnect::CheckApiSvr(void)
{
	CInternetHttp http;
	CString strPostAddr = _T("");
	CString strPostData = _T("");
	CString strResponseTxt = _T("");

	GetURL(strPostAddr);

	DWORD dwRet = 0;
	int iRet = http.HttpPost(strPostAddr, strPostData, strResponseTxt, dwRet);

	if (iRet != 0 && dwRet != 200)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接api服务器失败!ret:%d, lasterr:%d, 返回码:%d"), iRet, GetLastError(), dwRet);
		return false;
	}

	return true;
}

//检测百度服务器
bool CCheckSvrConnect::CheckBaiDuSvr(void)
{
	CInternetHttp http;
	CString strPostAddr = _T("https://www.baidu.com/");
	CString strPostData = _T("");
	CString strResponseTxt = _T("");

	DWORD dwRet = 0;
	int iRet = http.HttpPost(strPostAddr, strPostData, strResponseTxt, dwRet);

	if (iRet != 0 && dwRet != 200)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接百度服务器失败!ret:%d, lasterr:%d, 返回码:%d"), iRet, GetLastError(), dwRet);
		return false;
	}

	return true;
}

//检测SEO服务器
bool CCheckSvrConnect::CheckSeoSvr(void)
{
	CInternetHttp http;
	CString strPostAddr = _T("http://seo.chinaz.com/");
	CString strPostData = _T("");
	CString strResponseTxt = _T("");

	DWORD dwRet = 0;
	int iRet = http.HttpPost(strPostAddr, strPostData, strResponseTxt, dwRet);

	if (iRet != 0 && dwRet != 200)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接seo服务器失败!ret:%d, lasterr:%d, 返回码:%d"), iRet, GetLastError(), dwRet);
		return false;
	}

	return true;
}


void CCheckSvrConnect::GetFtpLoginInfo(const CString &strIniPath)
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

	xml.ReadString(_T("MC"), _T("update"), _T("html"), szInfo);

	if (szInfo == _T(""))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("读取mcconfig中的update_html登陆信息为空!"));
		return;
	}

	CString strDecrypt;
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


void CCheckSvrConnect::GetFtpDesCode(TCHAR *code, TCHAR *ip, TCHAR *account, TCHAR *psd, int &port)
{
	int i = 0;
	TCHAR *p[4];
	TCHAR *buf = code;
	while((p[i]=_tcstok(buf, _T(":"))) != NULL) 
	{
		//printf("str = [%s]\n",p[i]);
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


bool CCheckSvrConnect::GetURL(CString& strURL)
{
	CString strCfgFile = _T("");
	CString szVal = _T("");
	strCfgFile.Format(_T("%s\\data2\\mc.dat"), GetInstallPath());

	IXMLRW xml;
	xml.init(strCfgFile);

	xml.ReadString(_T("MC"), _T("LOGININFO"), _T("url"), szVal);

	strURL = szVal;
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取到配置文件的API线上地址为%s！"),strURL.GetBuffer());
	strURL.ReleaseBuffer();

	//如果为空，则取[SERVICE] Current 
	if (strURL.IsEmpty())
	{
		strURL = URL_DEFAULT_LOGINAPI;
		g_log.Trace(LOGL_TOP,LOGT_WARNING, __TFILE__,__LINE__, _T("API配置为空,使用默认API上线地址：http://api.sumszw.com"));
	}
	return true;
}

bool CCheckSvrConnect::IsInternetConnect()
{	
	DWORD dwFlags = 0;
	if (!InternetGetConnectedState(&dwFlags, 0))
	{
		return false;
	}
	return true;
}