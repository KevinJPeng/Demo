#include "stdafx.h"
#include "ClientInterface.h"
#include <WinInet.h>
#include "../common/Ping.h"
#include "..\threadmodel\MsgDef.h"
#include "Reg.h"
#include <TlHelp32.h>
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")

bool GetURL(CStdString& strURL)
{	
	CString strLogURl;
	IXMLRW xml;
	CStdString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\MC.dat"), GetInstallPath());
	//CIniFile iniFile(strCfgFile);
	xml.init(strCfgFile.GetBuffer());

	xml.ReadString(_T("MC"), _T("LOGININFO"), _T("url"), strLogURl);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取到配置文件的API线上地址为%s！"),strURL.c_str());

	if (strLogURl.IsEmpty())
	{
		strURL = URL_DEFAULT_LOGINAPI;
		g_log.Trace(LOGL_TOP,LOGT_WARNING, __TFILE__,__LINE__, _T("API配置为空,使用默认API上线地址：http://api.sumszw.com"));
	}
	else
	{
		strURL = strLogURl;
	}
	return true;
}

DWORD GetConnectServer()
{	
	int iTime = 0;
	IXMLRW xml;
	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\mc.dat"), GetInstallPath());
	xml.init(strCfgFile);

	xml.ReadInt(_T("MC"), _T("CONNECTSERVER"), _T("time"), iTime,10);
	
	return iTime;
}

//明文加密
DWORD GetEncodeDate(const CString& strUserName,const CString&strPassWord,CString& strEnCodeData)
{	
	strEnCodeData.Empty();
	CString strTempUser = strUserName;
	CString strTempPassWord = strPassWord;
	CString strContent = _T("");

	time_t timep;
	time(&timep);

	strContent.Format(_T("%s|%d|%s"),strTempUser,(DWORD)timep,strTempPassWord);

	//进行一次异或操作
	TCHAR ch = _T('①');
	for (int i =0; i < strContent.GetLength(); ++i)
	{	
		DWORD Temp = strContent[i];
		Temp = Temp^ch;
		strContent.SetAt(i,Temp);
	}

	strEnCodeData = stringcoding::StringBase64Encode(strContent);
	//add by zhumingxing 20141106URLENCODE
	strEnCodeData = URLEncode(strEnCodeData);

	CString strDest = _T("");
	strDest.Format(_T("EncryptedStr=%s&TimeStamp=%d"),strEnCodeData,(DWORD)timep);
	strEnCodeData = strDest;

	return 0;
}

//密文加密
CString GetEncodeDate( CString& strEncodeUserName, CString&strEncodePassWord)
{	
	CString strEncodeData = _T("");
	//解密用户名
	CStdString strDecodeUserName = DecodeString(strEncodeUserName);
	//解密密码
	CStdString strDecodePassWord = DecodeString(strEncodePassWord);

	GetEncodeDate((CString)strDecodeUserName,(CString)(strDecodePassWord),strEncodeData);
	return strEncodeData;
}

CStdString EncodeString( CString &strSource)
{
	//进行一次异或操作
	TCHAR ch = _T('①');
	for (int i =0; i < strSource.GetLength(); ++i)
	{	
		DWORD Temp = strSource[i];
		Temp = Temp^ch;
		strSource.SetAt(i,Temp);
	}

	return(stringcoding::StringBase64Encode(strSource));
}

CStdString DecodeString( CString& strDest)
{	

	CString strDecodeData = stringcoding::StringBase64Decode(strDest);

	//进行一次异或操作
	TCHAR ch = _T('①');
	for (int i =0; i < strDecodeData.GetLength(); ++i)
	{	
		DWORD Temp = strDecodeData[i];
		Temp = Temp^ch;
		strDecodeData.SetAt(i,Temp);
	}
	return strDecodeData;
}

void DecodeString(const CString& strParam,tuserInfotodb&tuserdb)
{	
	CString strTemp = strParam;
	int ipos = strParam.Find(_T("&TimeStamp="));
	if (ipos == -1)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("登录模块传回的用户信息%s有误缺少&TimeStamp=！"),strParam);
		return;
	}

	strTemp = strTemp.Left(ipos);
	strTemp.Replace(_T("EncryptedStr="),_T(""));

	//add by zhumingxing 20141106
	CString strEncode = UrlDecode(strTemp);
	strTemp = DecodeString(strEncode);

	if (strTemp.Find(_T("|")) == -1)
	{	
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("登录模块传回的用户信息%s有误！"),strTemp);
		return ;
	}
	int ipos1 = strTemp.Find(_T("|"));
	int ipos2 = strTemp.Find(_T("|"),ipos1+1);

	tuserdb.strUserName = strTemp.Left(ipos1);
	tuserdb.strPassWord = strTemp.Mid(ipos2+1);

}

CString FormatDate(char* pDate)
{
	CString str(pDate);
	int ipos = str.Find(_T("-"));
	CString strDateFormat = str.Mid(ipos+1);

	return strDateFormat;
}

CString GetDay(char* pDate)
{	
	CString strFormat = _T("");
	CString strTempDate = pDate;
	int ipos = strTempDate.ReverseFind(_T('-'));
	CString strDay  = strTempDate.Mid(ipos+1);
	WORD wDays = _ttoi(strDay);
	strFormat.Format(_T("%d"),wDays);

	return strFormat;
}

void GetDateSeven(std::vector<CString>& vecDate)
{
	vecDate.clear();
	time_t          t;
	struct tm      *m;

	for (int iindex =1; iindex <= 7; ++iindex)
	{
		t = time(NULL) - 24 * 3600*iindex;
		m = localtime(&t);
		int mon = m->tm_mon + 1;
		int day = m->tm_mday;

		CString strDate;
		strDate.Format(_T("%02d-%02d"),mon,day);
		vecDate.push_back(strDate);
	}

}

char* CStringToMutilChar(CString& str,int& chLength,WORD wPage)
{
	char* pszMultiByte; 
	int iSize = WideCharToMultiByte(wPage, 0, str, -1, NULL, 0, NULL, NULL); 
	pszMultiByte = (char*)malloc((iSize+1)/**sizeof(char)*/); 
	memset(pszMultiByte,0,iSize+1);
	WideCharToMultiByte(wPage, 0, str, -1, pszMultiByte, iSize, NULL, NULL); 
	chLength = iSize;
	return pszMultiByte;
}

bool WriteUserInfo(CString& strUserName,CString& strPassWord,int iAutoFlag,int iPassWordFlag,int iProductid)
{	
	UserInfo tuser;
	int ilength = -1;

	strUserName = (CString)EncodeString(strUserName);
	strPassWord = (CString)EncodeString(strPassWord);

	char* pUserName = CStringToMutilChar(strUserName,ilength);
	memcpy(tuser.szUName,pUserName,ilength);
	char* PassWord = CStringToMutilChar(strPassWord,ilength);
	memcpy(tuser.szPwd,PassWord,ilength);
	tuser.iAutoLogin = iAutoFlag;
	tuser.iSavePwd = iPassWordFlag;
	tuser.iProduceId = iProductid;

	g_globalData.sqlIte.DeleteUserInfo();
	g_globalData.sqlIte.SaveUserInfo(tuser);

	free(pUserName);
	pUserName = NULL;
	free(PassWord);
	PassWord = NULL;

	return true;
}

bool ReadUserInfo(CString& strUserName,CString& strPassWord,int&iAutoFlag,int&iPassWordFlag)
{

	UserInfo tuser;
	g_globalData.sqlIte.GetUserInfo(tuser);
	iAutoFlag =  tuser.iAutoLogin;
	iPassWordFlag = tuser.iSavePwd;

	strUserName = (CString)DecodeString((CString)(tuser.szUName));
	strPassWord = (CString)DecodeString((CString)(tuser.szPwd));

	return true;
}

CString GetUserName()
{
	UserInfo tuser;
	g_globalData.sqlIte.GetUserInfo(tuser);

	CString strUserName = (CString)DecodeString((CString)(tuser.szUName));
	return strUserName;
}

bool IsInternetConnect()
{	
	DWORD dwFlags = 0;
	if (!InternetGetConnectedState(&dwFlags, 0))
	{
		return false;
	}
	return true;
}

bool AnysisTipsInfo(const CString& strInfo, CString& strTitle, CString& strContent, CString& strTime)
{
	if (strInfo.IsEmpty())
	{	
		return false;
	}
	if (strInfo.Find(_T("##")) == -1)
	{
		return false;
	}
	int ipos1 = strInfo.Find(_T("##"));
	int ipos2 = strInfo.Find(_T("##"),ipos1+1);

	strTitle = strInfo.Left(ipos1);
	strContent = strInfo.Mid(ipos1+2,ipos2-ipos1-2);
	strTime = strInfo.Mid(ipos2+2);

	return true;
}

int GetRand(DWORD dwBase, DWORD dwMin)
{
	srand((unsigned)time(NULL)); //用于保证是随机数
	return (rand()%dwBase + dwMin); 
}

CString GeLocalTimeRand()
{	
	CString strTemp;
	time_t timep;
	time(&timep);

	strTemp.Format(_T("%d"),timep);	
	return strTemp;
}

void OperateRegistOfStart(WORD wtype)
{	
	CReg reg;
	CString strKeyPath = _T("");
	CString strCurUserkeyPath = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
	CString strNoReStartFlagPath = _T("Software\\szw\\MasterZ");
	////判断操作系统类型
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL); 
	LPFN_ISWOW64PROCESS fnIsWow64Process; 
	BOOL bIsWow64 = FALSE; 
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandleW(_T("kernel32")),"IsWow64Process"); 
	if (NULL != fnIsWow64Process) 
	{ 
		fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
	} 
	//64位操作系统
	if (bIsWow64)
	{	
		strKeyPath = _T("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户操作系统为64位系统"));
	}
	else
	{
		strKeyPath = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户操作系统为32位系统"));
	}
	CString strValue = _T("");
	strValue.Format(_T("%s\\bin\\MasterZ.exe /Auto"),g_globalData.dir.GetInstallDir());

	//开机启动项为了避免用户需要管理员操作注册表，将注册表位置写到HKEY_CURRENT_USER
	if (wtype == 0)
	{	
		if (reg.WriteValueOfKey(HKEY_CURRENT_USER,strCurUserkeyPath,_T("MasterZ"),strValue))
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("写用户开机启动注册表项成功，注册表路径为:%s"),strKeyPath);		
		}
		else
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("写用户开机启动注册表项失败，注册表路径为:%s"),strKeyPath);
		}

		reg.DeleteValue(HKEY_CURRENT_USER, strNoReStartFlagPath, _T("nostart"));
	}
	else
	{	
		if (reg.DeleteValue(HKEY_LOCAL_MACHINE, strKeyPath, _T("MasterZ")))
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("删除MACHINE开机启动注册表项成功，注册表路径为:%s"),strKeyPath);		
		}
		else
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("删除MACHINE开机启动注册表项失败，注册表路径为:%s"),strKeyPath);
		}
		if (reg.DeleteValue(HKEY_CURRENT_USER,strCurUserkeyPath,_T("MasterZ")))
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("删除User开机启动注册表项成功，注册表路径为:%s"),strKeyPath);		
		}
		else
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("删除User开机启动注册表项成功，注册表路径为:%s"),strKeyPath);	
		}

		//通过设置勾选的开机不启动，则启动程序也不用再去检测
		reg.WriteValueOfKey(HKEY_CURRENT_USER, strNoReStartFlagPath, _T("nostart"),_T("1"));
	}
}

void GetSettingInfo(tclientsetting& tsSetting)
{	
	CReg reg;
	CString strKeyPath = _T("");
	CString strCurUserkeyPath = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
	//判断操作系统类型
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL); 
	LPFN_ISWOW64PROCESS fnIsWow64Process; 
	BOOL bIsWow64 = FALSE; 
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandleW(_T("kernel32")),"IsWow64Process"); 
	if (NULL != fnIsWow64Process) 
	{ 
		fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
	} 
	//64位操作系统
	if (bIsWow64)
	{	
		strKeyPath = _T("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户操作系统为64位系统"));
	}
	else
	{
		strKeyPath = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户操作系统为32位系统"));
	}

	//此处为了兼容以前的安装包写开机启动项在HKEY_LOCAL_MACHINE位置
	TCHAR* pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_LOCAL_MACHINE,strKeyPath,_T("MasterZ"));
	TCHAR* pContentCurUser = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,strCurUserkeyPath,_T("MasterZ"));

	if (pContent == NULL && pContentCurUser == NULL)
	{	
		tsSetting.strRebootStart = _T("0");
	}
	else
	{
		tsSetting.strRebootStart = _T("1");
	}

	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\mc.dat"), GetInstallPath());
	IXMLRW iniFile;
	iniFile.init(strCfgFile);

	CString strContent = _T("");
	iniFile.ReadString(_T("MC"),_T("MasterZSETTING"), _T("MasterZplain"), strContent);
	tsSetting.strMasterZPlain = strContent;
}

void WriteConfig(tclientsetting& tsSetting)
{	
	//处理开机启动
	if (tsSetting.strRebootStart.Compare(_T("1")) == 0)
	{
		OperateRegistOfStart();
	}
	else
	{
		OperateRegistOfStart(1);
	}

	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\mc.dat"), GetInstallPath());

	IXMLRW iniFile;
	iniFile.init(strCfgFile);
	iniFile.WriteString(_T("MC"),_T("MasterZSETTING"),_T("MasterZplain"),tsSetting.strMasterZPlain);
}

BOOL CheckProcessRun(TCHAR* chProName)
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
			if (!_tcsicmp(chProName, process.szExeFile))
			{
				bRun = true;
				break;
			}

			bFlag = Process32Next(hSnapShot, &process);
		}

		CloseHandle(hSnapShot); 

	}
	else
	{	
		bRun = true;
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("CreateToolhelp32Snapshot failed, err: %d"), GetLastError());
	}

	return bRun;
}

void ChangeLoginfo(PassInfo& passinfo, CString strEncodeString, CString strUserName)
{	
	CStdString strUrl = _T("");
	GetURL(strUrl);

	passinfo.strParam = strEncodeString;
	passinfo.strUrl = strUrl;
	passinfo.strUserName = strUserName;
}

CString GetLocalTimeFormat()
{	
	CString strFormat = _T("");
	SYSTEMTIME time = {0};
	::GetLocalTime(&time);
	strFormat.Format(_T("%04d%02d%02d"),time.wYear,time.wMonth,time.wDay);

	return strFormat;
}

//Unicode CString URLEncode 
BYTE toHex(const BYTE &x)
{
	return x > 9 ? x + 55: x + 48;
}

CString  URLEncode(CString sIn)
{
	int ilength = -1;
	char* pUrl = CStringToMutilChar(sIn,ilength,CP_UTF8);
	CStringA strSrc(pUrl);

	CStringA sOut;
	const int nLen = strSrc.GetLength() + 1;

	register LPBYTE pOutTmp = NULL;
	LPBYTE pOutBuf = NULL;
	register LPBYTE pInTmp = NULL;
	LPBYTE pInBuf =(LPBYTE)strSrc.GetBuffer(nLen);
	BYTE b = 0;

	//alloc out buffer
	pOutBuf = (LPBYTE)sOut.GetBuffer(nLen*3 - 2);//new BYTE [nLen  * 3];

	if(pOutBuf)
	{
		pInTmp   = pInBuf;
		pOutTmp = pOutBuf;

		// do encoding
		while (*pInTmp)
		{
			if(isalnum(*pInTmp))
				*pOutTmp++ = *pInTmp;
			else
				if(isspace(*pInTmp))
					*pOutTmp++ = '+';
				else
				{
					*pOutTmp++ = '%';
					*pOutTmp++ = toHex(*pInTmp>>4);
					*pOutTmp++ = toHex(*pInTmp%16);
				}
				pInTmp++;
		}
		*pOutTmp = '\0';
		//sOut=pOutBuf;
		//delete [] pOutBuf;
		sOut.ReleaseBuffer();
	}
	strSrc.ReleaseBuffer();
	if (pUrl != NULL)
	{
		delete pUrl;
		pUrl = NULL;
	}
	return CString(sOut);
}

//URLDecode
CString Utf8ToStringT(LPSTR str)  
{  
	_ASSERT(str);  
	USES_CONVERSION;  
	WCHAR *buf;  
	int length = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);  
	buf = new WCHAR[length+1];  
	ZeroMemory(buf, (length+1) * sizeof(WCHAR));  
	MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, length);  

	if (str != NULL)
	{
		delete str;
		str =  NULL;
	}
	return (CString(W2T(buf)));  
}  

CString UrlDecode(LPCTSTR url)  
{  
	_ASSERT(url);  
	USES_CONVERSION;  
	LPSTR _url = T2A(const_cast<LPTSTR>(url));  

	int i = 0;  
	int length = (int)strlen(_url);  
	CHAR *buf = new CHAR[length+1];  
	ZeroMemory(buf, length+1);  
	LPSTR p = buf;  
	while(i < length)  
	{  
		if(i <= length -3 && _url[i] == '%' && IsHexNum(_url[i+1]) && IsHexNum(_url[i+2]))  
		{  
			sscanf(_url + i + 1, "%2x", p++);

			i += 3;  
		}  
		else  
		{  
			*(p++) = _url[i++];  
		}  
	}  

	return Utf8ToStringT(buf);  
}  

BOOL IsChartDataChange (const KEYWORD_PRODUCT_LIST& tOld,const KEYWORD_PRODUCT_LIST& tNew,DWORD wtype)
{	
	//比较关键词图表
	if (wtype == KEYWORD_CHART)
	{	
		//列表个数不一样，图表有变化加载
		if (tOld.keyWordList.size() != tNew.keyWordList.size())
		{	
			return TRUE;
		}
		else
		{	
			//大小相等，且都为0,无变化加载默认图表
			if (tOld.keyWordList.size() == 0)
			{
				return FALSE;
			}
			
			//大小相等，且都不为0则考虑最后一项数据是否相等
			if (tOld.keyWordList[tOld.keyWordList.size()-1].iAllSEHasRankingCount == tNew.keyWordList[tNew.keyWordList.size()-1].iAllSEHasRankingCount && 
				!strcmp(tOld.keyWordList[tOld.keyWordList.size()-1].szDate,tNew.keyWordList[tNew.keyWordList.size()-1].szDate))
			{
				return FALSE;
			}
			else
			{
				return TRUE;
			}	
		}
	}
	//比较曝光图表
	if (wtype == PRODUCT_CHART)
	{	
		//列表个数不一样，图表有变化加载
		if (tOld.productList.size() != tNew.productList.size())
		{	
			return TRUE;
		}
		else
		{	
			//大小相等，且都为0,无变化加载默认图表
			if (tOld.productList.size() == 0)
			{
				return FALSE;
			}

			//大小相等，且都不为0则考虑最后一项数据是否相等
			if (tOld.productList[tOld.productList.size()-1].iProductCoverCount == tNew.productList[tNew.productList.size()-1].iProductCoverCount && 
				!strcmp(tOld.productList[tOld.productList.size()-1].szDate,tNew.productList[tNew.productList.size()-1].szDate))
			{
				return FALSE;
			}
			else
			{
				return TRUE;
			}	
		}
	}
	return TRUE;
}

BOOL IsWeixinUserCareDataChange(const WEIXINTONG_DATA& told,const WEIXINTONG_DATA& tnew,DWORD wtype)
{

	//关注曲线比较
	if (wtype == WEIXIN_USERCARE_CHART)
	{

		if (told.subscribeChartList.size() != tnew.subscribeChartList.size() || told.iSuccessFlag != tnew.iSuccessFlag)
		{
			return TRUE;
		}
		else
		{

			if (told.subscribeChartList.size() == 0)
			{
				return FALSE;
			}
			//大小相等考虑最后一项数据是否相等
			if (told.subscribeChartList[told.subscribeChartList.size()-1].iAddNum == tnew.subscribeChartList[tnew.subscribeChartList.size()-1].iAddNum &&
				told.subscribeChartList[told.subscribeChartList.size()-1].iSubscribNum == tnew.subscribeChartList[tnew.subscribeChartList.size()-1].iSubscribNum &&
				told.subscribeChartList[told.subscribeChartList.size()-1].iUnSubscribNum == tnew.subscribeChartList[tnew.subscribeChartList.size()-1].iUnSubscribNum &&
				!strcmp(told.subscribeChartList[told.subscribeChartList.size()-1].szDate,tnew.subscribeChartList[tnew.subscribeChartList.size()-1].szDate))  

			{
				return FALSE;
			}
			else
			{
				return TRUE;
			}

		}
	}
	//请求曲线比较
	if (wtype == WEIXIN_USERREQUEST_CHART)
	{
		if (told.requestChartList.size() != tnew.requestChartList.size() || told.iSuccessFlag != tnew.iSuccessFlag)
		{
			return TRUE;
		}
		else
		{
			if (told.requestChartList.size() == 0)
			{
				return FALSE;
			}
			if (told.requestChartList[told.requestChartList.size() -1].iTextReply == tnew.requestChartList[tnew.requestChartList.size()-1].iTextReply && 
				told.requestChartList[told.requestChartList.size() -1].iMenuClick == tnew.requestChartList[tnew.requestChartList.size()-1].iMenuClick && 
				told.requestChartList[told.requestChartList.size() -1].iTotalRequest == tnew.requestChartList[tnew.requestChartList.size()-1].iTotalRequest && 
				!strcmp(told.requestChartList[told.requestChartList.size() -1].szDate,tnew.requestChartList[tnew.requestChartList.size()-1].szDate))
			{
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
	}


}

TCHAR* GetRegCurrentUserName()
{
	CReg reg;
	TCHAR* pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),_T("SupportProduct"));
	if (pContent == NULL)
	{	
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("注册表Software\\szw\\MasterZ\\SupportProduct版本支持为空！"));
		return NULL;
	}
	else
	{	
		TCHAR* pUserName = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),pContent);

		if (pUserName == NULL)
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("注册表Software\\szw\\MasterZ\\%s用户名为空!"),pContent);
			return NULL;
		}
		else
		{
			return pUserName;
		}
	}
}

BOOL UpdateRegUserInfo( CString strUserName,int ProDuctId )
{
	TCHAR* pUserName = GetRegCurrentUserName();
	CString strTempUserName = (CString)DecodeString(strUserName);
	int iCurrentRegProDuctID = GetRegCurentVersionID();

	//表示有变化---需要更新注册表
	if ((NULL ==  pUserName  || _tcscmp(pUserName,strTempUserName.GetBuffer())) || ProDuctId != iCurrentRegProDuctID)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检测到了需要更新注册表用户名!"));

		CString strSupportProduct = _T("");
		strSupportProduct.Format(_T("ZDS_%d"),ProDuctId);

		CReg reg;
		if (!reg.WriteValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),_T("SupportProduct"),strSupportProduct))
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("写注册表Software\\szw\\MasterZ\\SupportProduct失败!错误码:%d"),GetLastError());
			return FALSE;
		}
		if(!reg.WriteValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),strSupportProduct,strTempUserName))
		{	
			//此处可能写失败，故多重试一次
			Sleep(100);
			if (!reg.WriteValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), strSupportProduct, strTempUserName))
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("写注册表Software\\szw\\MasterZ\\%s失败!用户名为:%s:错误码:%d"), strSupportProduct, strTempUserName, GetLastError());
			}	
			return FALSE;
		}
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("注册表用户名更新完成!"));
	}
	return TRUE;
}

DWORD GetClientType()
{
	CStdString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\customize.dat"), GetInstallPath());
	CIniFile iniFile(strCfgFile);

	int iType = -1;
	iniFile.ReadInteger(_T("version"),_T("type"),iType);

	if (iType == 0)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("客户端类型Type值为0,当前客户端为主线版本!"));
	}
	else
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("客户端类型Type值为%d,当前客户端为定制版本!"),iType);
	}

	return iType;
}

BOOL GetUpdateDatCfg()
{
	IXMLRW iniFile;
	int iValue = 0;

	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\mc.dat"), GetInstallPath());

	if (iniFile.init(strCfgFile) != XRET_SUCCESS)
	{
		return FALSE;
	}

	iniFile.ReadInt(_T("MC"),_T("SYNCDAT"),_T("clientupdate"),iValue);
	if (iValue == 0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

void GetEncodeUpdateData(CString& strEnCodeData, DWORD dwClientType)
{	
	//获取当前时间
	CString strSrcData = _T("");
	CString strDesData = _T("");
	time_t timep;
	time(&timep);

	CReg reg;
	CString strClientVersion = (TCHAR*)reg.ReadValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ\\Setup"), REG_KEY_VERSION);
	CString strUserName = GetRegCurrentUserName();
	CString strMacAddress = GetPhysicalAddress();

	if (strUserName.IsEmpty())
	{
		strUserName = _T("NoUserName");
	}
	if (strMacAddress.IsEmpty())
	{
		strMacAddress = _T("00-00-00-00-00-00");
	}

	if (dwClientType != CUSTOMIZE_VERSION)
	{
		strSrcData.Format(_T("zds&%d&%s&%s&%s"), (DWORD)timep, strUserName, strMacAddress, strClientVersion);
	}
	else
	{
		strSrcData.Format(_T("zds_custom&%d&%s&%s&%s"), (DWORD)timep, strUserName, strMacAddress, strClientVersion);
	}
	
	g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("封装升级字符串源字符串为:%s!"),strSrcData);

	//进行一次异或操作
	TCHAR ch = _T('①');
	for (int i =0; i < strSrcData.GetLength(); ++i)
	{	
		DWORD Temp = strSrcData[i];
		Temp = Temp^ch;
		strSrcData.SetAt(i,Temp);
	}

	strDesData = stringcoding::StringBase64Encode(strSrcData);
	//add by zhumingxing 20141106URLENCODE
	strDesData = URLEncode(strDesData);

	strEnCodeData = strDesData;
}

int GetRegCurentVersionID()
{
	CReg reg;
	TCHAR* pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),_T("SupportProduct"));

	if (pContent == NULL)
	{	
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("注册表Software\\szw\\MasterZ\\SupportProduct版本支持为空!"));
		return -1;
	}
	else
	{		
		CString strVersion(pContent);
		if (strVersion.Find(_T("_")) == -1)
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("注册表中支持版本格式有误，未找到下划线标示符"));
			return -1;
		}

		strVersion = strVersion.Mid(strVersion.ReverseFind(_T('_')) + 1);
		return(_ttoi(strVersion));
	}
}

CString GetDirectory(CString strShortDir)
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
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("未找到环境变量 %s,目录未替换！"), strTotalFlag);
		}
		strRet.ReleaseBuffer();
	}


	//将替换后的目录加上目录变量之后的路径
	strShortDir.Replace(strTotalFlag, strRet);
	goto FindFlag;           //继续查找下一个通配符
}

DWORD GetHideType()
{
	/*IXMLRW iniFile;*/
	int iValue = 0;

	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\version.dat"), GetInstallPath());

	CIniFile iniFile;
	iniFile.SetFilePath((CStdString)strCfgFile);

	iniFile.ReadInteger(_T("Control"), _T("HideClient"), iValue);
/*
	iniFile.init(strCfgFile);
	iniFile.ReadInt(_T("MC"), _T("CONTROL"), _T("HideClient"), iValue);*/

	return iValue;
}

void SetHideType()
{	
	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\version.dat"), GetInstallPath());

	CIniFile iniFile;
	iniFile.SetFilePath((CStdString)strCfgFile);

	iniFile.WriteInteger(_T("Control"), _T("HideClient"), MAIN_LINE_VERSION);
	/*IXMLRW iniMcFile;
	iniMcFile.init(strCfgFile);

	iniMcFile.WriteInt(_T("MC"), _T("CONTROL"), _T("HideClient"), MAIN_LINE_VERSION);*/
}
