#include "StdAfx.h"
#include "AutoGetKeywordPages.h"
#include "..\..\common\Ping.h"
#include "..\..\common\tinystr.h"
#include "..\..\common\tinyxml.h"
#include <Iphlpapi.h>
#include "..\..\common\Timer.h"
#include "ScheduleThread.h"

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "..\\..\\lib\\tinyxml.lib")

CHttpUtils g_http;
// CInternetHttp g_InternetHttp;
CIniFile g_iniCfgFile;
CRITICAL_SECTION g_Section;
CTimer g_Timer;
extern HANDLE g_hEvent;


CAutoGetKeywordPages::CAutoGetKeywordPages(CScheduleThread *pScheduleThread)
{	
	InitializeCriticalSection(&g_Section);
	m_bAlreadySendDataToProc = false;
	m_pScheduleThread = pScheduleThread;
	m_objScheduleMgr = new CScheduleMgr(m_pScheduleThread, this);
	m_hThreadTaskPolling = NULL;
	m_strFastServer = _T("");
	m_strUserName = _T("");
	m_strVersinId = _T("");
	m_strUseAccount = _T("");
	m_nRefreshFlag = 0;
	init();
}


CAutoGetKeywordPages::~CAutoGetKeywordPages(void)
{	
	DeleteCriticalSection(&g_Section);
	m_objScheduleMgr->TerminateAllTasks();
}


bool CAutoGetKeywordPages::init(void)
{
	CStdString strCfgFile = _T("");
	CString strCfg = _T("");
	int iVal = 0;
	CStdString strSouthServer = _T("");
	CStdString strNorthServer = _T("");
	CStdString strFastServer = _T("");
	strCfgFile.Format(_T("%s\\data2\\Schedule.dat"), g_pGlobalData->dir.GetInstallDir());
	strCfg = strCfgFile;
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("读服务器地址的配置文件路径: %s! "), strCfg);
	g_iniCfgFile.SetFilePath(strCfgFile);

	//请求任务同时运行最大任务数
	g_iniCfgFile.ReadInteger(_T("AutoTask"), _T("MaxConcurrency"), iVal, 2);
	g_iniCfgFile.ReadString(_T("AutoTask"), _T("SouthServer"), strSouthServer);
	g_iniCfgFile.ReadString(_T("AutoTask"), _T("NorthServer"), strNorthServer);
	g_iniCfgFile.ReadString(_T("AutoTask"), _T("FastServer"), strFastServer);
	m_objScheduleMgr->Init(iVal);

	//选择服务器
	m_strFastServer = ChooseHttpServer(strSouthServer, strNorthServer, strFastServer);
	
	m_bPollingThreadStop = false;
	m_hTimer = NULL;

	return TRUE;
}


bool CAutoGetKeywordPages::StartExec(CString strUserName, int strVesionId, int nRefreshFlag)
{
	m_hThreadTaskPolling = NULL;
	m_strUserName = strUserName;
	m_strVersinId.Format(_T("%d"), strVesionId);
	m_nRefreshFlag = nRefreshFlag;

	DWORD dwError = CreateScheduleTimer(100);
	if (dwError)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("创建定时器失败! Err: %d"), dwError);
		return false;
	}

	return true;
}


int CAutoGetKeywordPages::CreateScheduleTimer(LONGLONG dwTime)
{
	if (m_hTimer != NULL)
	{
		g_Timer.DestroyTimer();
	}

	if (!g_Timer.InstallTimer(m_hTimer, dwTime, NULL))
	{
		goto end;
	}


	if (WaitForSingleObject(m_hTimer, INFINITE) != WAIT_OBJECT_0)
	{
		goto end;
	}
	else
	{
		//CloseHandle(m_hTimer);
		RequestScheduleForTime();
	}

	return 0;

end:
	CloseHandle(m_hTimer);
	m_hTimer = NULL;

	DWORD dwError = GetLastError();
	return dwError;
}


void CAutoGetKeywordPages::RequestScheduleForTime()
{
	StringUtils util;

	RequestSchedule();
	
	if (m_tScheduleInfo.tInteract.strTimeMode == _T("0"))
	{//延时请求
		int nDelayTime = _ttoi(m_tScheduleInfo.tInteract.strTime);   //以分钟为单位

		if (nDelayTime > 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("延时请求已设置，延时时间：%s分钟！"), m_tScheduleInfo.tInteract.strTime);
			CreateScheduleTimer(nDelayTime * 60 * 1000);
		}
	}
	else if (m_tScheduleInfo.tInteract.strTimeMode == _T("1"))
	{//定时请求
		CStdString strTmp = m_tScheduleInfo.tInteract.strTime;
		std::vector<CStdString> vRes;

		util.SplitString(strTmp, _T(":"), vRes);
		if (vRes.size() == 3)
		{
			int nTargetHour = _ttoi(vRes[0]);
			int nTargetMin = _ttoi(vRes[1]);
			int nTargetSec = _ttoi(vRes[2]);

			CTime time;
			time = CTime::GetCurrentTime();

			//转换成秒判断定时是否已过
			int nTargetTime = (nTargetHour * 3600 + nTargetMin * 60 + nTargetSec) - 
				(time.GetHour() * 3600 + time.GetMinute() * 60 + time.GetSecond());

			if (nTargetTime > 0)
			{
				g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("定时请求已设置，下次执行时间：%s！"), m_tScheduleInfo.tInteract.strTime);
				CreateScheduleTimer(nTargetTime * 1000);
				
			}
		}
	}
	else if (m_tScheduleInfo.tInteract.strTimeMode == _T("2"))
	{//不再请求
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("当前用户已经没有任务，停止自动请求！"));
	}
	else
	{//默认请求
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("定时请求未设置[模式：%s 时间：%s]，默认将在1小时后再次执行！"), m_tScheduleInfo.tInteract.strTimeMode, m_tScheduleInfo.tInteract.strTime);
		CreateScheduleTimer(60 * 60000);
	}

}

CStdString CAutoGetKeywordPages::ChooseHttpServer(CStdString strSer1, CStdString strSer2, CStdString strFastSer)
{	
	CString strFastServer = _T("");
	if (strFastSer == _T(""))
	{
		strFastSer = ChooseBestServer(strSer1, strSer2);
	}

	//如果读不到服务器用默认服务器
	if (strFastSer == _T(""))
	{
		strFastSer = _T("http://2.16898.cc");
	}

	strFastServer = strFastSer;
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("较快服务器: %s"), strFastServer);	

	return strFastSer;
}

CString CAutoGetKeywordPages::ChooseBestServer(CStdString strSer1, CStdString strSer2)
{
	//只ping一次，后续直接根据上次的结果返回第一个或第二个
	static int s_iFlag = -1;
	if (s_iFlag == 1)
	{
		return strSer1;
	}
	else if (s_iFlag == 2)
	{
		return strSer2;
	}

	CPing Ping;
	DWORD fastTime = 9999;
	DWORD retTime1 = 0, retTime2 = 0;
	char szTmp[MAX_PATH] = {0};

	CString strBestServer = _T("");

	DWORD dwSize = MAX_PATH;
	WCharToMByte(strSer1, szTmp, &dwSize);
	retTime1 = Ping.PingHost(szTmp);

	dwSize = MAX_PATH;
	WCharToMByte(strSer2, szTmp, &dwSize);
	retTime2 = Ping.PingHost(szTmp);

	if (retTime1 <= retTime2)
	{
		s_iFlag = 1;
		return strSer1;
	}
	else
	{
		s_iFlag = 2;
		return strSer2;
	}
}


//获取用户对应产品下的自动任务
void CAutoGetKeywordPages::RequestSchedule(void)
{
	CString strUseAccount = _T("");
	CString strVesionId = _T("");

	strUseAccount = DecodeString(m_strUserName);
	m_strUseAccount = strUseAccount;         //保存解密后未url编码前的当前客户端用户名
	strVesionId = m_strVersinId;
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("接收到的账号：%s 版本号：%s"), strUseAccount, strVesionId);

	strUseAccount = URLEncode(strUseAccount);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("接收到的账号Rul编码后：%s 版本号：%s"), strUseAccount, strVesionId);


	//添加清除快照功能
	if (m_nRefreshFlag == 1)
	{
		if (!RemoveQuickPhoto(strUseAccount, strVesionId))
		{
			//清除失败就结束定时器
			m_tScheduleInfo.tInteract.strTimeMode = _T("2");

			m_pScheduleThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
			SetEvent(g_hEvent);

			return;
		}
		//else
		//{
			//清除快照成功也要返回给ui，好进行同步更新显示
			//m_pScheduleThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
		//}
	}


	try
	{	

		EnterCriticalSection(&g_Section);
		int iRet = GetScheduleData(strUseAccount, strVesionId, m_tScheduleInfo);
		if (ERRCODE_SUCCESS == iRet)
		{
			PrepareSchedule(m_tScheduleInfo);

			//***add by zhangdongshan -begin 点立即刷新确保起中控后再给ui发清除完成消息
			if (m_nRefreshFlag == 1)
			{
				Sleep(2000);
				//清除快照成功也要返回给ui，好进行同步更新显示
				m_pScheduleThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
			}
			//*** -end

		}
		else    //解析数据失败或者没有任务或者请求任务失败或者解析数据异常
		{
			SetEvent(g_hEvent);
			Sleep(3000);   //解决用户没有任务3秒内点刷新消息被丢弃，ui接收不到完成消息的问题
			m_pScheduleThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
		}
		LeaveCriticalSection(&g_Section);
	}
	catch (...)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("获取数据异常，账号：%s 版本号：%s"), strUseAccount, strVesionId);
		m_pScheduleThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
	}
	
}


bool CAutoGetKeywordPages::RemoveQuickPhoto(const CString &strUser, const CString &strVerId)
{
	CString strRequestURL = _T(""); 
	CString strResponseText = _T("");
	CString strRequestData = _T("");
	CString strFastServer = _T("");
	//CInternetHttp http;
	CHttpUtils http;
	strFastServer = m_strFastServer;

	strRequestURL.Format(_T("%s/Timing/RemoveTimingKeyWordsData?uName=%s&versionId=%s"), strFastServer, strUser, strVerId);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("用户[%s]清除关键词排名！地址：%s"), strUser, strRequestURL);

	strResponseText = http.GetSerRespInfo(strRequestURL.GetBuffer());
	//int iRet = http.HttpGet(strRequestURL, strRequestData, strResponseText);
	if (0 != strResponseText.CompareNoCase(_T("清除成功！")))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("发送清排名任务请求失败! 地址：%s , lasterr:%d, errmsg:%s"), strRequestURL, GetLastError(), strResponseText);
		return false;
	}

	return true;
}

//准备自动任务
bool CAutoGetKeywordPages::PrepareSchedule(T_DATA_FROM_SERVER &tData)
{
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("enter PrepareSchedule"));

	int nTaskCount = 0;

	/*while (WaitForSingleObject(g_hEvent, INFINITE) != WAIT_OBJECT_0)
	{
		Sleep(100);
	}
	ResetEvent(g_hEvent);*/
	//////////////////////////////////////////////////////////////////////////
	//防止切换账号起两个主控问题p
	//m_objScheduleMgr->TerminateAllTasks();
	//////////////////////////////////////////////////////////////////////////


	//将任务压入任务管理器
	std::vector<T_TASK_DATA>::iterator itTask = tData.vTask.begin();
	for (; itTask != tData.vTask.end(); ++itTask)
	{
		m_objScheduleMgr->PushTask(*itTask);
		nTaskCount++;
	}

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("收到%d个任务！当前缓冲%d个，正在执行%d个！"), nTaskCount, 
		m_objScheduleMgr->GetCachingTaskCount(), m_objScheduleMgr->GetExecingTaskCount());

	tData.vTask.clear();

	//启动自动任务的轮询线程
	if (nTaskCount > 0)
	{
		if (NULL == m_hThreadTaskPolling)
		{
			m_bPollingThreadStop = false;
			m_hThreadTaskPolling = CreateThread(NULL, 0, &CAutoGetKeywordPages::ThreadProcPollingTask, this, 0, NULL );

			//stoptask中还要用到该句柄
 			//CloseHandle(m_hThreadTaskPolling);
 			//m_hThreadTaskPolling = NULL;
		}
	}

	return true;
}

//向服务器请求自动任务数据
//请求到任务返回0，失败返回1，无任务数据返回2
int CAutoGetKeywordPages::GetScheduleData(CString strUseAccount, CString strVersionId, T_DATA_FROM_SERVER &tData)
{
	CString strRequestURL = _T(""); 
	CString strResponseText = _T("");
	CString strRequestData = _T("");
	CString strFastServer = _T("");
	//CInternetHttp http;
	CHttpUtils http;
	strFastServer = m_strFastServer;

	strRequestURL.Format(_T("%s/Timing/GetTimingData?uName=%s&versionId=%s&MAC=%s"), strFastServer, strUseAccount, strVersionId, GetPhysicalAddress());
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("用户[%s]请求任务！地址：%s"), strUseAccount, strRequestURL);

	//int iRet = http.HttpGet(strRequestURL, strRequestData, strResponseText);
	strResponseText = http.GetSerRespInfo(strRequestURL.GetBuffer());
	if (0 == strResponseText.CompareNoCase(_T("$ERR$")))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("发送任务请求失败! 地址：%s , lasterr:%d, errmsg:%s"), strRequestURL, GetLastError(), strResponseText);
		return ERRCODE_FAIL;
	}
	else if (strResponseText.IsEmpty())
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("用户[%s]没有需要执行的自动任务(无数据返回)！地址：%s"), strUseAccount, strRequestURL);
		return ERRCODE_NO_TASK;
	}

	int iMaxXMLen = 512 * 1024;       //512k
	TCHAR *ptmpBuf = NULL;
	char *pmsbXMLData = NULL;

	try
	{
		ptmpBuf = new TCHAR[iMaxXMLen];
		memset(ptmpBuf, 0, iMaxXMLen);
		memcpy_s(ptmpBuf, iMaxXMLen, strResponseText.GetBuffer(), strResponseText.GetLength() * sizeof(TCHAR));

		DWORD dwSize = strResponseText.GetLength() * 2;
		pmsbXMLData = new char[dwSize];
		memset(pmsbXMLData, 0, dwSize);
		WCharToMByte(ptmpBuf, pmsbXMLData, &dwSize);

		TiXmlDocument doc;
		doc.Parse(pmsbXMLData);

		delete []pmsbXMLData;
		delete []ptmpBuf;

		if (!ParseServerResponse(&doc, tData, strVersionId))
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("解析任务数据失败！URL:%s\r\nData:\r\n%s\r\n"), strRequestURL, strResponseText);
			return ERRCODE_FAIL;
		}
	}
	catch (...)
	{
		if (!ptmpBuf)
			delete []ptmpBuf;

		if (!pmsbXMLData)
			delete []pmsbXMLData;

		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("解析任务数据异常！URL:%s\r\nData:\r\n%s\r\n"), strRequestURL, strResponseText);
		return ERRCODE_FAIL;
	}

	return ERRCODE_SUCCESS;
}


//分解从服务端请求到的自动任务数据
bool CAutoGetKeywordPages::ParseServerResponse(TiXmlDocument *pDoc, T_DATA_FROM_SERVER &tScheduleInfo,  CString strVersionId)
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

	TiXmlElement *pInteract = pRoot->FirstChildElement("interact");
	if (!pInteract)
	{
		//默认延时60分钟后再次请求
		tScheduleInfo.tInteract.strTimeMode = _T("0");
		tScheduleInfo.tInteract.strTime = _T("60");
	}
	else
	{
		tScheduleInfo.tInteract.strTimeMode = pInteract->Attribute("timemode");
		if (tScheduleInfo.tInteract.strTimeMode == _T("2"))
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("没有任务！"));
			return false;
		}
		tScheduleInfo.tInteract.strTime = pInteract->Attribute("time");
	}

	//获取任务列表
	TiXmlElement *pTaskList = pRoot->FirstChildElement("tasklist");
	if (!pTaskList)
	{
		return false;
	}

	TiXmlElement *pTaskItem = pTaskList->FirstChildElement("task");
	if (!pTaskItem)
	{
		return true;  //为空说明没有任务，解析成功
	}

	CString strFastServer = _T("");
	//遍历读取命令列表
	for (; pTaskItem; pTaskItem = pTaskItem->NextSiblingElement())
	{
		T_TASK_DATA tData;
		tData.strUID = pTaskItem->Attribute("uid");
		tData.strUIPdtVer = pTaskItem->Attribute("vid");
		tData.strExclusive = pTaskItem->Attribute("exclusive");
		tData.strTimeMode = pTaskItem->Attribute("timemode");
		tData.strTime = pTaskItem->Attribute("time");
		tData.strType = pTaskItem->Attribute("type");
		tData.strSubject = pTaskItem->Attribute("subject");
		tData.strData = pTaskItem->Attribute("data");
		tData.strPostAddr = pTaskItem->Attribute("posturl");
		tData.strExtraData = pTaskItem->Attribute("extradata");
		//tData.strCUID = pTaskItem->Attribute("cuid");

		//补充信息
		strFastServer = m_strFastServer;
		tData.strPostAddr = strFastServer + tData.strPostAddr;
		tData.strCUID = m_strUseAccount;
		tData.strPdtVer = strVersionId;

		tScheduleInfo.vTask.push_back(tData);
	}

	return true;
}


//轮循自动任务
void CAutoGetKeywordPages::PollingSchedule(void)
{
	m_objScheduleMgr->PollingTask();
}

DWORD WINAPI CAutoGetKeywordPages::ThreadProcPollingTask(LPVOID lpParameter)
{
	CAutoGetKeywordPages *pThis = (CAutoGetKeywordPages *)lpParameter;

	while (!pThis->m_bPollingThreadStop)
	{
		Sleep(1000);

		pThis->PollingSchedule();
	}

	return 0;
}

CString CAutoGetKeywordPages::DecodeString( CString& strDest)
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


//Unicode CString URLEncode 
BYTE toHex(const BYTE &x)
{
	return x > 9 ? x + 55: x + 48;
}

CString  CAutoGetKeywordPages::URLEncode(CString sIn)
{
	int ilength = -1;
	char* pUrl = CStringToUtf8Char(sIn,ilength);
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


char* CAutoGetKeywordPages::CStringToMutilChar(CString& str,int& chLength)
{
	char* pszMultiByte; 
	int iSize = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL); 
	pszMultiByte = (char*)malloc((iSize+1)/**sizeof(char)*/); 
	memset(pszMultiByte,0,iSize+1);
	WideCharToMultiByte(CP_ACP, 0, str, -1, pszMultiByte, iSize, NULL, NULL); 
	chLength = iSize;
	return pszMultiByte;
}


char* CAutoGetKeywordPages::CStringToUtf8Char(CString& str,int& chLength)
{
	char* pszMultiByte; 
	int iSize = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL); 
	pszMultiByte = (char*)malloc((iSize+1)/**sizeof(char)*/); 
	memset(pszMultiByte,0,iSize+1);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, pszMultiByte, iSize, NULL, NULL); 
	chLength = iSize;
	return pszMultiByte;
}



void CAutoGetKeywordPages::StopTask(void)
{
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("enter StopTask！"));

	m_bPollingThreadStop = true;
	WaitForSingleObject(m_hThreadTaskPolling, INFINITE);
	CloseHandle(m_hThreadTaskPolling);
	m_hThreadTaskPolling = NULL;
	CancelWaitableTimer(m_hTimer);
	m_objScheduleMgr->TerminateAllTasks();

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("out StopTask！"));

}