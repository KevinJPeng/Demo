//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ProcessData.h"
#include "ServerSocket.h"
#include "Stringutils.h"
//#include "unzipwebtaskdata.h"
//#include "FileReadAndSave.h"
//#include "ZipEncode.h"
//#include "HttpUtils.h"
//#include "enumdef.h"
//#include "CommFunc.h"
//#include <fstream>
//#include <ShlObj.h>
//#include <atlimage.h>
//#include "SearchWord.h"
#include "Reg.h"
#include "WebDlg.h"
#include "Httputils.h"
#include "GetNetTime.h"


HHOOK g_hCBT = NULL;	// 保存CBT钩子句柄

//////////////////////////////////////////////////////////////////////////////////////
//
// 函数功能描述：构造函数，
// 输入：
// 输出：
// 返回值：
// 其它说明：
//
//////////////////////////////////////////////////////////////////////////////////////
CProcessData::CProcessData()
{
    m_pServerSocket = NULL;	
	bAlreadySend = FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////
//
// 函数功能描述：析构函数
// 输入：
// 输出：
// 返回值：
// 其它说明：
//
//////////////////////////////////////////////////////////////////////////////////////
CProcessData::~CProcessData()
{
	
}

//////////////////////////////////////////////////////////////////////////////////////
//
// 函数功能描述：将向socket发送信息的对象指针传送并保存
// 输入：
// 输出：
// 返回值：
// 其它说明：
//
//////////////////////////////////////////////////////////////////////////////////////
DWORD CProcessData::SetPtrOfServer(CServerSocket *pServerSocket)
{
    if (NULL != pServerSocket)
    {
        m_pServerSocket = pServerSocket;
    }
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
//
// 函数功能描述：将Socket传入的数据进行分流处理
// 输入：socket的缓存数据头指针,缓存数据长度
// 输出：
// 返回值：成功分流返回0，否则返回1
// 其它说明：
//
///////////////////////////////////////////////////////////////////////////////
DWORD CProcessData::CovertBufData(char *Orgbuf,DWORD dwTotalLen)
{

	CovertBufDataWeb(Orgbuf,dwTotalLen);

	return 1;
}


///////////////////////////////////////////////////////////////////////////////
//
// 函数功能描述：将Socket传入的数据进行分流处理
// 输入：socket的缓存数据头指针,缓存数据长度
// 输出：
// 返回值：成功分流返回0，否则返回1
// 其它说明：
//
///////////////////////////////////////////////////////////////////////////////
DWORD CProcessData::CovertBufDataWeb(char *Orgbuf,DWORD dwTotalLen)
{
	CStdString strData;
	CStdString strDataType;

	DWORD dwIndex = 0;
	DWORD dwSize = dwTotalLen - 4;

	/*size_t rsize = mbstowcs(0,Orgbuf+4,0);*/
	int iSize = MultiByteToWideChar(CP_ACP, 0, Orgbuf + 4, -1, NULL, 0);
	TCHAR *dest = NULL;
	
	_NEWA(dest,wchar_t,iSize+1);
	/*mbstowcs(dest,Orgbuf+4,rsize+1);*/
	MultiByteToWideChar(CP_ACP, 0, Orgbuf + 4, -1, dest, iSize+1);
	strData = CStdString(dest);
	_DELETEA(dest);

	/*strData = _T("SoftPost(;0)C3eU9AF7sGaUK05HyYF3hXC7wrUQTO9U61Nl1Cs9bUwZsMvWMICgCRjCVKLyTBkgkJeBiwGmwgDOAolYAE0H0UZA2gRoFwSaMRgDZWDiu/x2+QXWw3iGQLXGQGwIl4eIQEjcZpoymCPp2ZCalRsVvCgaZmoKQyJDMQOExCYGkQEABX8zdyYBAMA=");*/
	g_log.Trace(LOGL_TOP,LOGT_PROMPT,__TFILE__, __LINE__,_T("数据为：%s"), strData.Buffer());
	dwIndex = strData.Find(SIMPLE_LIST_SPLIT);

	//判断是否重启
	if (bAlreadySend)
	{	
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("MC.exe运行时检测到第二次收到数据，将产生异常！"));
		//::MessageBox(NULL, _T("MC.exe运行时检测到第二次收到数据，将产生异常！"), _T("MC运行"), MB_OK | MB_ICONERROR);
		return 1;
	}
	
	bAlreadySend = TRUE;

	if (dwIndex > 0)
	{
		strDataType = strData.Mid(0,dwIndex);
	}

	if (0 == strDataType.CompareNoCase(_T("QuickPhoto")))  //抓取各搜索引擎快照
	{
		strData = strData.Right(strData.length()-dwIndex-4);
		GetSearchPhoto(strData);
	}
	return 1;
}

//初始化调试Url
void CProcessData::InitDebugUrl()
{
	TCHAR szIniFile[MAX_PATH] = { 0 };
	TCHAR szInfo[MAX_PATH] = { 0 };

	GetModuleFileName(NULL, szIniFile, MAX_PATH);
	PathAppend(szIniFile, _T("..\\..\\"));
	PathAppend(szIniFile, _T("data2\\debugyundata.ini"));

	GetPrivateProfileString(_T("QuickPhoto"), _T("debugurl"), NULL, szInfo, MAX_PATH, szIniFile);

	g_strDebugUrl = szInfo;

	if (g_strDebugUrl.GetLength() > 1)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到有配置DebugUrl:%s,进入调试模式"), g_strDebugUrl.GetBuffer());
		g_bIsDebug = TRUE;
	}
}

//CBT钩子消息处理过程
LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CBT_CREATEWND *pWndParam = NULL;
	CREATESTRUCT *pCreateStruct = NULL;
	CString strText;

	switch (nCode)
	{
	case HCBT_CREATEWND:
		pWndParam = (CBT_CREATEWND*)lParam;
		pCreateStruct = pWndParam->lpcs;
		strText = pCreateStruct->lpszName;

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("捕获创建窗口消息，标题：%s"), strText);

		//监视类型为WS_POPUP且标题中含有过滤字符串的窗口创建
		if ((pCreateStruct->style & WS_POPUP) &&
			(-1 != strText.Find(_T("安全警告")) ||
			-1 != strText.Find(_T("安全警报")) ||
			0 == strText.CompareNoCase(_T("Internet Explorer")) ||
			0 == strText.CompareNoCase(_T("Windows Internet Explorer")) ||
			strText == _T("网页错误") ||
			strText.Find(_T("来自网页的消息")) != -1 ||
			strText.Find(_T("MC.exe")) != -1
			))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("捕获到安全弹框提示，已屏蔽，标题：%s"), strText);
			return 1;

		}
		break;

	}

	// 继续传递消息
	return CallNextHookEx(g_hCBT, nCode, wParam, lParam);
}

BOOL WINAPI SetHook(BOOL isInstall)
{
	// 需要安装，且钩子不存在
	if (isInstall && !g_hCBT)
	{
		// 设置全局钩子
		g_hCBT = SetWindowsHookEx(WH_CBT, (HOOKPROC)CBTProc, 0, GetCurrentThreadId());
		if (g_hCBT == NULL)
		{
			return FALSE;
		}

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("勾子设置成功"));
	}

	// 需要卸载，且钩子存在
	if (!isInstall && g_hCBT)
	{
		// 卸载钩子
		BOOL ret = UnhookWindowsHookEx(g_hCBT);
		g_hCBT = NULL;
		return ret;
	}

	return TRUE;
}

bool CProcessData::AllisNum(CString str)
{
    for (int i = 0; i < str.GetLength(); i++)
    {
        int tmp = (int)str[i];
        if (tmp >= 48 && tmp <= 57)
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    return true;
}
// 初始化延时时间
void CProcessData::InitDelayTime()
{
	TCHAR szFilePath[MAX_PATH] = { 0 };

	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	PathAppend(szFilePath, _T("..\\..\\"));
	PathAppend(szFilePath, _T("data2\\mc.dat"));
	IXMLRW xml;
	xml.init(szFilePath);
	xml.ReadInt(_T("MC"), _T("QuickPhoto"), _T("delay_for_each_keyword"), g_iDelayForKeyWord, -1);
	xml.ReadInt(_T("MC"), _T("QuickPhoto"), _T("delay_for_each_site"), g_iDalayForSite, -1);
	xml.ReadInt(_T("MC"), _T("QuickPhoto"), _T("count"), g_iThreadCount, -1);

	//xml.ReadString(_T("MC"), _T("QuickPhoto"), _T("http_oss"), g_shttpOss, _T(""));
	CString szInfo = _T("");
	CString strURL = _T("");
	IKeyRW key;
	xml.ReadString(_T("MC"), _T("QuickPhoto"), _T("time_server"), szInfo, _T(""));
	key.InitDll(KTYPE_CFG);
	key.DecryptData(szInfo, DECRYPTCFG, strURL);
	//从.net端获取时间
	CString strResponseText = _T("");
	CHttpUtils http;
	//strURL = _T("http://192.168.1.16:7001/home/GetKZName");
	strResponseText = http.GetSerRespInfo(strURL.GetBuffer());
	bool bFlag = false;
	int iPos = strResponseText.Find(_T("\""));
	if (-1 != iPos)
	{
		strResponseText = strResponseText.Mid(iPos +1);
		iPos = strResponseText.Find(_T("\""));
		if (-1 != iPos)
		{
			strResponseText = strResponseText.Mid(0, iPos);
			bFlag = AllisNum(strResponseText);
		}
	}
	if (_T("$ERR$") == strResponseText || strResponseText.IsEmpty() || !bFlag)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("从.Net端获取时间失败"));
		//从网络服务器获取时间
		CTime timeTemp;
		CGetNetTime	getNetTime;
		int iRet = -1;
		iRet = getNetTime.GetNetTime(timeTemp);		//获取网络时间
		if (0 != iRet)
		{
			//失败,使用本地时间
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("获取网络时间失败"));
			timeTemp = CTime::GetCurrentTime();
		}
		int iYear = _ttoi(timeTemp.Format("%Y").Right(2));//取年份的后2位
		int iMonth = timeTemp.GetMonth();
		CString sYMD;
		int iDay = timeTemp.GetDay();
		sYMD.Format(_T("%d%d"), iDay, (iYear + iMonth));
		strResponseText = sYMD;
	}
	g_sKWPath = strResponseText;

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("初始化数据:%d,%d,%d,%s"), g_iDalayForSite, g_iDelayForKeyWord, g_iThreadCount, strResponseText);
}
//初始化配置文件信息
void CProcessData::InitCfgInfo()
{
	StringUtils StringUtil;
	int iCnt;
	TCHAR szPath[MAX_PATH] = { 0 };

	GetModuleFileName(NULL, szPath, MAX_PATH);
	PathAppend(szPath, _T("..\\..\\"));
	PathAppend(szPath, _T("data2\\updaterank.dat"));
	IXMLRW xmlCfg;
	xmlCfg.init(szPath);

	xmlCfg.ReadInt(_T("updaterank"), _T("EngineCnt"), _T("count"), iCnt);
	//王牌网站对应
	CString strTmp;
	std::vector<CStdString> vTmp;
	std::vector<CStdString> vTmp2;
	std::map<CStdString, DWORD> mapTmp;
	xmlCfg.ReadString(_T("updaterank"), _T("EngineCnt"), _T("AceWeb"), strTmp);
	StringUtil.SplitString((CStdString)strTmp, _T(","), vTmp, false);
	for (auto i : vTmp)
	{
		StringUtil.SplitString(i, _T("|"), vTmp2, false);
		if (vTmp2.size() > 0)
			mapTmp[vTmp2[0]] = _ttol(vTmp2[1]);
	}

	CString strCssPath;
	xmlCfg.ReadString(_T("updaterank"), _T("EngineCnt"), _T("CssPath"), strCssPath);


	if (iCnt <= 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("读取网站标记配置文件失败或没被设置；"));
		return;
	}

	for (int i = 0; i < iCnt; i++)
	{
		TCHAR szName[50] = { 0 };
		CString strInfo = _T("");
		_Elem_MatchFlag elemCfgFlag;

		wsprintf(szName, _T("Engine%d"), i);

		xmlCfg.ReadString(_T("updaterank"), szName, _T("JumpUrl"), elemCfgFlag.strJumpUrl);
		//如果搜索引擎不存在，退出 开始读取配置信息
		if (elemCfgFlag.strJumpUrl.GetLength() <= 0)
		{
			continue;
		}
		xmlCfg.ReadString(_T("updaterank"), szName, _T("CollTags"), elemCfgFlag.strCollTags);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("ItemFlag1"), elemCfgFlag.strItemFlag1);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("ItemFlag2"), elemCfgFlag.strItemFlag2);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("FindUrlFlag1"), elemCfgFlag.strFindUrlFlag1);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("FindUrlFlag2"), elemCfgFlag.strFindUrlFlag2);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("FindUrlFlag3"), elemCfgFlag.strFindUrlFlag3);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("BtnId"), elemCfgFlag.strBtnId);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("GetUrlFlag1"), elemCfgFlag.strGetUrlFlag1);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("GetUrlFlag2"), elemCfgFlag.strGetUrlFlag2);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("JudgeHtmlTag"), elemCfgFlag.strJudgeHtmlTag);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("UniversalFst"), elemCfgFlag.strUniversalFst);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("UniversalSnd"), elemCfgFlag.strUniversalSnd);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("SaveHtmlName"), elemCfgFlag.strHtmlName);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("LocationAd"), elemCfgFlag.strLocationAd);

		xmlCfg.ReadString(_T("updaterank"), szName, _T("TargetMarkWeb"), strInfo);
		StringUtil.SplitString((CStdString)strInfo, _T(","), elemCfgFlag.vTargetMarkWeb, false);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("TargetKey"), strInfo);
		StringUtil.SplitString((CStdString)strInfo, _T(","), elemCfgFlag.vTargetKey, false);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("TargetStart"), strInfo);
		StringUtil.SplitString((CStdString)strInfo, _T(","), elemCfgFlag.vTargetStart, false);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("TargetEnd"), strInfo);
		StringUtil.SplitString((CStdString)strInfo, _T(","), elemCfgFlag.vTargetEnd, false);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("TargetPreLen"), strInfo);
		StringUtil.SplitString((CStdString)strInfo, _T(","), elemCfgFlag.vTargetPreLen, false);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("TargetLastLen"), strInfo);
		StringUtil.SplitString((CStdString)strInfo, _T(","), elemCfgFlag.vTargetLstLen, false);
		xmlCfg.ReadString(_T("updaterank"), szName, _T("ReplaceTagWebs"), strInfo);
		StringUtil.SplitString((CStdString)strInfo, _T(","), elemCfgFlag.vReplaceTagWebs, false);

		bool bFound = false;
		vector<locationStruct> vlocationTitle;
		vector<CString> vsSingleStruct;
		vlocationTitle.clear();
		vsSingleStruct.clear();
		xmlCfg.ReadString(_T("updaterank"), szName, _T("ItemLocationTitle"), strInfo);
		SplitCString(strInfo, _T(";"), vsSingleStruct, false);
		for (int io = 0; io < vsSingleStruct.size(); io++)
		{
			strInfo = vsSingleStruct[io];
			vector<CString> vstrTemp;
			SplitCString(strInfo, _T(","), vstrTemp, false);
			if (5 == vstrTemp.size())
			{
				locationStruct itemLable;
				bFound = true;
				itemLable.iLocationType = atol(CStrW2CStrA(vstrTemp[0]));
				itemLable.sTagType = vstrTemp[1];
				itemLable.lFuzzyIndex = atol(CStrW2CStrA(vstrTemp[2]));
				itemLable.sKey = vstrTemp[3];
				itemLable.sValue = vstrTemp[4];
				vlocationTitle.push_back(itemLable);
			}
		}
		if (bFound)
		{
			bFound = false;
			elemCfgFlag.vstrLocationTitle = vlocationTitle;
		}


		targetAddressSign	starget;
		vector<CString> vstrTemp;
		vstrTemp.clear();
		xmlCfg.ReadString(_T("updaterank"), szName, _T("ItemtargetTitle"), strInfo);
		SplitCString(strInfo, _T(","), vstrTemp, false);
		if (2 == vstrTemp.size())
		{
			starget.iUrlGetType = atol(CStrW2CStrA(vstrTemp[0]));
			starget.sHrefKey = vstrTemp[1];
			elemCfgFlag.strTargetTitle = starget;
		}

		vlocationTitle.clear();
		vsSingleStruct.clear();
		xmlCfg.ReadString(_T("updaterank"), szName, _T("ItemLocationLink"), strInfo);
		SplitCString(strInfo, _T(";"), vsSingleStruct, false);
		for (int io = 0; io < vsSingleStruct.size(); io++)
		{
			strInfo = vsSingleStruct[io];
			vector<CString> vstrTemp;
			SplitCString(strInfo, _T(","), vstrTemp, false);
			if (5 == vstrTemp.size())
			{
				locationStruct itemLable;
				bFound = true;
				itemLable.iLocationType = atol(CStrW2CStrA(vstrTemp[0]));
				itemLable.sTagType = vstrTemp[1];
				itemLable.lFuzzyIndex = atol(CStrW2CStrA(vstrTemp[2]));
				itemLable.sKey = vstrTemp[3];
				itemLable.sValue = vstrTemp[4];
				vlocationTitle.push_back(itemLable);
			}
		}
		if (bFound)
		{
			bFound = false;
			elemCfgFlag.vstrLocationLink = vlocationTitle;
		}

		xmlCfg.ReadString(_T("updaterank"), szName, _T("ItemtargetLink"), strInfo);
		vstrTemp.clear();
		SplitCString(strInfo, _T(","), vstrTemp, false);
		if (2 == vstrTemp.size())
		{
			starget.iUrlGetType = atol(CStrW2CStrA(vstrTemp[0]));
			starget.sHrefKey = vstrTemp[1];
			elemCfgFlag.strTargetLink = starget;
		}
		elemCfgFlag.strCssPath = strCssPath;
		elemCfgFlag.mapAceWebToID = mapTmp;

		xmlCfg.ReadString(_T("updaterank"), szName, _T("SearchPageNum"), strInfo);
		elemCfgFlag.iSearchPageNum = atol(CStrW2CStrA(strInfo));
		if (elemCfgFlag.iSearchPageNum <= 0)
		{
			elemCfgFlag.iSearchPageNum = 3;
		}

		xmlCfg.ReadString(_T("updaterank"), szName, _T("kwSearchMethod"), elemCfgFlag.sKWSearchMethod);
		if (i == 0)//百度
		{
			xmlCfg.ReadString(_T("updaterank"), szName, _T("HomePage2"), elemCfgFlag.sHomePage);
		}
		else
		{
			xmlCfg.ReadString(_T("updaterank"), szName, _T("HomePage"), elemCfgFlag.sHomePage);
		}

		TagNode tagNodeData;
		xmlCfg.ReadString(_T("updaterank"), szName, _T("SearchInput"), strInfo);
		vstrTemp.clear();
		SplitCString(strInfo, _T(","), vstrTemp, false);
		if (3 == vstrTemp.size())
		{
			tagNodeData.sTag = vstrTemp[0];
			tagNodeData.skey = vstrTemp[1];
			tagNodeData.sValue = vstrTemp[2];
			elemCfgFlag.tagKWEdit = tagNodeData;
		}

		tagNodeData.clear();
		xmlCfg.ReadString(_T("updaterank"), szName, _T("SearchBtn"), strInfo);
		vstrTemp.clear();
		SplitCString(strInfo, _T(","), vstrTemp, false);
		if (3 == vstrTemp.size())
		{
			tagNodeData.sTag = vstrTemp[0];
			tagNodeData.skey = vstrTemp[1];
			tagNodeData.sValue = vstrTemp[2];
			elemCfgFlag.tagSearchBtn = tagNodeData;
		}

		tagNodeData.clear();
		xmlCfg.ReadString(_T("updaterank"), szName, _T("SearchNext"), strInfo);
		vstrTemp.clear();
		SplitCString(strInfo, _T(","), vstrTemp, false);
		if (3 == vstrTemp.size())
		{
			tagNodeData.sTag = vstrTemp[0];
			tagNodeData.skey = vstrTemp[1];
			tagNodeData.sValue = vstrTemp[2];
			elemCfgFlag.tagNextPage = tagNodeData;
		}

		tagNodeData.clear();
		xmlCfg.ReadString(_T("updaterank"), szName, _T("MainBody"), strInfo);
		vstrTemp.clear();
		SplitCString(strInfo, _T(","), vstrTemp, false);
		if (3 == vstrTemp.size())
		{
			tagNodeData.sTag = vstrTemp[0];
			tagNodeData.skey = vstrTemp[1];
			tagNodeData.sValue = vstrTemp[2];
			elemCfgFlag.tagMainBody = tagNodeData;
		}

		


		g_mapElemCfg[i] = elemCfgFlag;
	}
	//初始化指纹数据2018/06/15
	xmlCfg.ReadString(_T("FingerRecogn"), _T("FingerInfo"), _T("CharData"), g_sFingerRecogn.sCharData);
	xmlCfg.ReadInt(_T("FingerRecogn"), _T("FingerInfo"), _T("MatchNum"), g_sFingerRecogn.iMatchNum, 2);
	SplitCString(g_sFingerRecogn.sCharData, _T("|"), g_sFingerRecogn.vsCharData, false);
	g_sFingerRecogn.iCount = g_sFingerRecogn.vsCharData.size();

	return;
}
DWORD WINAPI StartCapture(LPVOID lp)
{
	CoInitialize(NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	AfxEnableControlContainer();

	CWebDlg  photoDlg;
	photoDlg.DoModal();
	CoUninitialize();

	return 0;
}

//抓取各搜索引擎快照
void CProcessData::GetSearchPhoto( const CStdString &strData )
{
	//如果当前定义的非804，则不需要抓取排名
	CReg reg;	
	CString strInfoSys;
	TCHAR* pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Control Panel\\International"), _T("Locale"));
	strInfoSys = pContent;
	if (strInfoSys != _T("00000804"))
	{
		g_log.Trace(LOGL_HIG, LOGT_ERROR, __TFILE__, __LINE__,
			_T("检测到系统编码非804，不执行抓取排名, 系统Locale：%s"), pContent);
		Sleep(1000);
		ExitProcess(0);
	}

	InitializeCriticalSection(&critSection);
	InitializeCriticalSection(&critSendMsg);
	InitializeCriticalSection(&critSearchFlag);
	InitializeCriticalSection(&critCount);

	//解密加密字符串
	CStdString sSource = strData;
	CString sHead = sSource.Mid(0, 5);
	if (_T("1(;0)") == sHead)
	{
		sSource = sSource.Mid(5);
		g_bManualRefresh = true;
	}

	CString sHMutexName;
	if (g_bManualRefresh)
	{
		sHMutexName = _T("_UpdateRank_UserSide_");
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到用户手动刷新排名的数据；"));
	}
	else
	{
		sHMutexName = _T("_UpdateRank_Yun_");
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到云刷新排名的数据；"));
	}

	//创建命名互斥对象
	HANDLE hMutex = CreateMutex(NULL, FALSE, sHMutexName);
	if (!hMutex)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("%s互斥对象创建失败"), sHMutexName);
	}

	// 	strWord = _T("C3ey9lUIrRIj9ChBdxQHVDvdJTZTduuTJu7cQzEZQdwoIKNEaisFJKBVpSIqUYSYQCgLKh34AP0I2RnYusDCxBdgQ0LAyD2OLvcGdaoveQ/LSl3HSuqf7/w8z7mOXewa9IpcQq8Fd41+XqDXorvsfr7/Nni39nrvVXuYY09Ge3rFgq2Ytp4XC29dclh466LDwp/71Nvr8u+8jsd7fzyOO3PdO88OW+n2cv555+Xqyf7y/qPWnzZ/m7+PP8/rxfH38Du14p0GHaHPN/p74Kup88WuTmdsfv3cLh8Dr2dNmRTsI8oGHdNBV6uGdhjTqTG+eHoQvE/rR9J9Xxnmu/e4q457Hdqp4ZUjKT+u3Nwqy3hj9WrfQh0bUyOcX/86Pkt8tmatM9k/dFCZ382lB3w8/3uHpqSdYBEl1SIM74UpOSCzZaoeYYFNcky/nRQJplramVKq4eolg+tb6EsN3bThDcij4oCnm2ebWvkFvjBiZI3i9Sy1E3zifcyn06GhXQ9s4np89fFZPq4jWPBzkInjHbZbd9GVGmQWOhIeABZ4nSZb+H7MPKfjfqxOSompJ8XtMLWnNMezJ5TtTit+kHl+FxNjRv7A9yaq5HsZ1Y6vQqjLh7x+u2wNoZ2hpwV0qUzsIMOVqHEV2nAG9KNfNy06C3WTpPImj/Y1u9KCN0hW2dg8X9wn08lhVpJm5mWWL2u/1JJ06CkvobwiiYXJqqKXoJP5/MythcGJzqZ76J44rcwZWllAKLllrve2AzKN/zPYuPLAJzM682n0p41MhntjMg3pzrChtQX3xkRbmE0jZ9roTfCJumjzhVYW0ImyxKSYUBaN6oXOKyATTdEks1AzmXx4iuUJT8MVrKhmzZuDpsFpoZIyC3GPlp1h7VRPpiEtNgsTLJRFJiEtMgu9KBkFT3xEdN9dU1XCO0LkJZXJZ3Y0siZ6M3wVIy+tTDJquLqVTBZ5qWU6lBYqKfll8gmsalVyMsvIc3RlfdBKr2K65fudp+4P83Rba3oP36OJpzbCpqGm4eda/wlPYdO49sI7YeLNRrpsIV3wLyv5FE2eKwDA");
	//解密加密字符串
	CStdString strInfo = _T("");
	CFileReadAndSave::UnZipDecodeStdString(sSource, strInfo);
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到主控传过来的抓取快照数据；"));

	strInfo.append(_T("(;8)"));
	//初始化调试URL
	InitDebugUrl();

	InitDelayTime();
	ParseAllKeyWordData(strInfo);


	for (int j = 0; j<SEARCHFLAGCOUNT; j++)
	{
		bSearchFlag[j] = TRUE;
	}

	if (g_vAllKeyWords.size() > 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("快照总数：%d"), g_vAllKeyWords.size());
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("开始清除ie中搜索引擎缓存"));
		DeleteSearchCache();
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("清除ie中搜索引擎缓存结束"));

		SetHook(TRUE);

		//InitDelayTime();
		//初始化配置
		InitCfgInfo();


		int iCPUNums = 2;
		iCPUNums = GetCPUCoreNums();

		if (g_iThreadCount == -1)
		{
			if (iCPUNums <= 0) // 防止GetCPUCoreNums()返回-1的情况
			{
				g_iThreadCount = 2;
			}
			else
			{
				g_iThreadCount = iCPUNums + 1;
			}
		}

		if (g_bIsDebug)
		{
			//调试模式只要一个线程
			g_iThreadCount = 1;
		}

		if (g_iThreadCount > g_vAllKeyWords.size())  //这里任务线程多于任务个数
			g_iThreadCount = g_vAllKeyWords.size();

		//Sleep(10000);
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("CPU核心数:%d,开启线程数:%d"), iCPUNums, g_iThreadCount);
		//g_iThreadCount = 1;
		HANDLE *hThreads = new HANDLE[g_iThreadCount];
		for (int i = 0; i < g_iThreadCount; i++)
		{
			DWORD dwThreadId;
			hThreads[i] = CreateThread(NULL, NULL, StartCapture, 0, 0, &dwThreadId);
		}

		DWORD dwTime = GetTickCount();
		WaitForMultipleObjects(g_iThreadCount, hThreads, TRUE, INFINITE);
		dwTime = GetTickCount() - dwTime;

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("本次抓取快照花费时间,%.2f,秒"), dwTime / 1000.0);

		for (int i = 0; i < g_iThreadCount; i++)
			CloseHandle(hThreads[i]);

		delete[]hThreads;

		SetHook(FALSE);
	}


	DeleteCriticalSection(&critSearchFlag);
	DeleteCriticalSection(&critSendMsg);
	DeleteCriticalSection(&critSection);
	DeleteCriticalSection(&critCount);

	CStdString strBack;

	strBack.Format(_T("BackResult(;0)AllTaskComplete(;0)(;0)"));

	BYTE *pByData = (BYTE *)strBack.c_str();
	g_server->SendData(strBack.size() * 2, E_GET_EXCUTE_TASK_RESULT, (char*)pByData);

	g_log.Trace(LOGL_HIG, LOGT_PROMPT, __TFILE__, __LINE__,
		_T("客户手动抓取快照结束，已发送结束字符串:%s"), strBack.c_str());
}

