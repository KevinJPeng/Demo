#include "StdAfx.h"
#include "YunTaskStreamCtr.h"
#include "..\..\common\Ping.h"
#include "..\..\common\tinystr.h"
#include "..\..\common\tinyxml.h"
#include <Iphlpapi.h>
#include "..\..\common\Timer.h"
#include "TaskThread.h"
#include "CommFunc.h"

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "..\\..\\lib\\tinyxml.lib")

IXMLRW g_iniCfgFile;
CIniFile g_iniDebugData;
CRITICAL_SECTION g_Section;

CYunTaskStreamCtr::CYunTaskStreamCtr(CTaskThread* pTaskThread)
{
	InitializeCriticalSection(&g_Section);
	m_pTaskThread			 = pTaskThread;
	m_objTaskMgr			 = new CTaskMgr(m_pTaskThread, this);
	m_pLockRequest			 = new CLock;
	m_pLockInfo				 = new CLock;
	m_pLockRoundCount        = new CLock;
	m_pCheckIdle			 = new CLock;
	m_pCheckLockScreen		 = new CLock;
	m_strUserName            = _T("");
	m_strVersinId            = _T("");
	m_strUseAccount          = _T("");
	m_strShopTrafServer      = _T("");
	m_strKeyWordServer       = _T("");
	m_strRefreshServer       = _T("");
	m_strMainTaskServer		 = _T("");
	m_strTaskRequestFlag     = _T("");
	m_strGetPublicIpUrl		 = _T("");
	m_iOldTime				 = 0;
	m_iTime					 = 0;
	m_iTaskKeyWordTimeOut    = 0;
	m_iTaskGeneralTimeOut    = 0;
	// 默认获取关键字排名任务
	m_iTypeTask              = eType_QuickPhoto;
	m_bIsSubmitResult		 = TRUE;
	m_strDebugTaskData		 = _T("");
	m_bIsRequestMainObjTask  = FALSE;
	m_hSystemInputMuext      = NULL;
	m_hCheckThread 			 = NULL;
	m_bCheckStop             = FALSE;
	m_iSystemInputIdle       = 0;
	m_bIsObjTask			 = FALSE;
	m_bIsWin10Version        =FALSE;
	m_dwLastInputTime		 = GetTickCount();
	InitCfg();
	InitOtherVar();
}

CYunTaskStreamCtr::~CYunTaskStreamCtr()
{
	DeleteCriticalSection(&g_Section);
	m_objTaskMgr->TerminateAllTasks();
	if (m_pLockRequest != NULL)
	{
		delete m_pLockRequest;
		m_pLockRequest = NULL;
	}
	if (m_pLockInfo != NULL)
	{
		delete m_pLockInfo;
		m_pLockInfo = NULL;
	}
	if (m_pLockRoundCount != NULL)
	{
		delete m_pLockRoundCount;
		m_pLockRoundCount = NULL;
	}
	if (m_pCheckIdle != NULL)
	{
		delete m_pCheckIdle;
		m_pCheckIdle = NULL;
	}
	if (m_pCheckLockScreen != NULL)
	{
		delete m_pCheckLockScreen;
		m_pCheckLockScreen = NULL;
	}
	
	if (m_objTaskMgr != NULL)
	{
		delete m_objTaskMgr;
		m_objTaskMgr = NULL;
	}
}

void CYunTaskStreamCtr::SetUserInfo(CString strUserName, int iVersionId)
{
	m_pLockInfo->Lock();
	m_strUserName = strUserName;
	m_strVersinId.Format(_T("%d"), iVersionId);
	m_pLockInfo->Unlock();
}

bool CYunTaskStreamCtr::InitCfg()
{	
	CString strCfgFile = _T("");
	CString strCfg = _T("");
	int iVal = 0;
	CString strKeyWordServer = _T("");
	CString strRefreshServer = _T("");
	CString strShopTrafServer = _T("");
	CString strMainTaskServer = _T("");
	TCHAR* pContent = NULL;

	strCfgFile.Format(_T("%s\\data2\\YunTask.dat"), g_pGlobalData->dir.GetInstallDir());
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("读服务器地址的配置文件路径: %s! "), strCfgFile);

	g_iniCfgFile.init(strCfgFile);

	//请求任务同时运行最大任务数
	g_iniCfgFile.ReadInt(_T("YunTask"),_T("AutoTask"), _T("MaxConcurrency"), iVal, 2);

	//相关云任务服务器地址
	g_iniCfgFile.ReadString(_T("YunTask"),_T("AutoTask"), _T("KeyWordServer"), strKeyWordServer);
	g_iniCfgFile.ReadString(_T("YunTask"), _T("AutoTask"), _T("RefreshServer"), strRefreshServer);
	g_iniCfgFile.ReadString(_T("YunTask"), _T("AutoTask"), _T("ShopTrafServer"), strShopTrafServer);
	g_iniCfgFile.ReadString(_T("YunTask"), _T("AutoTask"), _T("MainTaskServer"), strMainTaskServer);
	g_iniCfgFile.ReadString(_T("YunTask"), _T("AutoTask"), _T("PublicIPServer"), m_strGetPublicIpUrl, _T("http://1212.ip138.com/ic.asp"));

	//任务超时时间配置获取
	g_iniCfgFile.ReadInt(_T("YunTask"), _T("AutoTask"), _T("keyWordTaskTimeOut"), m_iTaskKeyWordTimeOut, 20);
	g_iniCfgFile.ReadInt(_T("YunTask"), _T("AutoTask"), _T("GeneralTaskTimeOut"), m_iTaskGeneralTimeOut, 30);
	//m_iTaskKeyWordTimeOut = 100;
	//m_iTaskGeneralTimeOut = 100;
	//modified by qy -- 20180613(读取配置文件中的ipAPI)
	int nIPQueryCount = 0;
	g_iniCfgFile.ReadInt(_T("AreaSetting"), _T("ipQueryCount"), _T("count"), nIPQueryCount);
	if (nIPQueryCount > 0)
	{
		m_ipQueryAPIArray.RemoveAll();
		CString strAPISection = _T("");
		for (int i = 0; i < nIPQueryCount; i++)
		{
			strAPISection.Empty();
			strAPISection.Format(_T("ipQueryAPI_%d"), i + 1);

			CString strAPI			= _T("");
			CString strPreField		= _T("");
			CString strNextField	= _T("");
			BOOL bUnicode			= FALSE;
			int iUnicodeFlag		= 0;
			int iCodeType			= 0;
			g_iniCfgFile.ReadString(_T("AreaSetting"), strAPISection, _T("ipAPI"), strAPI, _T(""));
			g_iniCfgFile.ReadString(_T("AreaSetting"), strAPISection, _T("preField"), strPreField, _T(""));
			g_iniCfgFile.ReadString(_T("AreaSetting"), strAPISection, _T("nextField"), strNextField, _T(""));
			g_iniCfgFile.ReadInt(_T("AreaSetting"), strAPISection, _T("isUnicode"), iUnicodeFlag);
			g_iniCfgFile.ReadInt(_T("AreaSetting"), strAPISection, _T("CodeType"), iCodeType);
			bUnicode = (iUnicodeFlag == 0) ? FALSE : TRUE;

			if (strAPI.IsEmpty() || strPreField.IsEmpty() || strNextField.IsEmpty())
				continue;

			SIPQueryAPI ipQueryApiTemp;
			ipQueryApiTemp.strIPAPI		= strAPI;
			ipQueryApiTemp.strPreField	= strPreField;
			ipQueryApiTemp.strNextField = strNextField;
			ipQueryApiTemp.bUnicode		= bUnicode;
			ipQueryApiTemp.iCodeType	= iCodeType;
			m_ipQueryAPIArray.Add(ipQueryApiTemp);
		}
	}

	CString strAreaList;
	strAreaList.Empty();
	g_iniCfgFile.ReadString(_T("AreaSetting"), _T("AreaInfo"), _T("AreaList"), strAreaList);
	m_strAreaArray.clear();
	SplitCString(strAreaList, _T(","), m_strAreaArray);

	m_strArea.Empty();
	m_strAreaWriteTime.Empty();
	g_iniCfgFile.ReadString(_T("AreaSetting"), _T("AreaInfo"), _T("AreaName"), m_strArea);
	g_iniCfgFile.ReadString(_T("AreaSetting"), _T("AreaInfo"), _T("WriteTime"), m_strAreaWriteTime);


	//1.每轮任务的间隔时间,优先从注册表中读取，再从配置文件中读取，默认120s
	CReg reg;
	pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("TaskIntervalTime"));
	if (NULL == pContent)
	{
		g_iniCfgFile.ReadInt(_T("YunTask"), _T("AutoTask"), _T("delay_for_each_task"), m_iTime, 120);
	}
	else
	{
		m_iTime = _ttoi(pContent);
	}
	m_iOldTime = m_iTime;

	//2.任务请求标记从注册表中读取，默认为空，即请求所有任务
	pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("YunTaskTypeFlag"));
	m_strTaskRequestFlag = CString(pContent);

	//3.RunObjTask标记从配置文件中读取，有此标记则会忽略所有规则限制，一直请求抢占焦点任务
	pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("RunObjTask"));
	if (NULL == pContent)
	{
		m_bIsRequestMainObjTask = FALSE;
	}
	else
	{
		m_bIsRequestMainObjTask = TRUE;
	}

	//4.云推广请求系统空闲时间限制配置优先从注册表中读取，再配置文件中读取，默认3600s
	pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("InputIdleTime"));
	if (NULL == pContent)
	{
		g_iniCfgFile.ReadInt(_T("YunTask"), _T("AutoTask"), _T("InputIdleTime"), m_iSystemInputIdle, 3600);
	}
	else
	{
		m_iSystemInputIdle = _ttoi(pContent);
	}
	
	m_objTaskMgr->Init(iVal);
	m_strShopTrafServer = strShopTrafServer;
	m_strKeyWordServer = strKeyWordServer;
	m_strRefreshServer = strRefreshServer;
	m_strMainTaskServer = strMainTaskServer;


	return true;
}

void CYunTaskStreamCtr::InitOtherVar()
{	
	//初始化调试数据配置文件
	CStdString strDebugFile = _T("");
	strDebugFile.Format(_T("%s\\data2\\debugyundata.ini"), g_pGlobalData->dir.GetInstallDir());
	g_iniDebugData.SetFilePath(strDebugFile);

	m_bIsWin10Version = IsWin10SystemVersion();

	if (NULL == m_hSystemInputMuext)
	{
		m_hSystemInputMuext = CreateMutex(NULL, FALSE, _T("Engine_SystemInPutMutex"));

		if (NULL == m_hSystemInputMuext)
		{
			g_log.Trace(LOGL_HIG, LOGT_ERROR, __TFILE__, __LINE__, _T("创建系统输入检测互斥量失败!"));
		}
	}

	//检测线程，检测系统的空闲时间
	m_hCheckThread = CreateThread(NULL, 0, &CYunTaskStreamCtr::ThreadProcCheckState, this, 0, NULL);
}

// 初始化URL请求地址
CString CYunTaskStreamCtr::GetURLAddress(int iType)
{
	// 加锁防止账号切换同步问题
	m_pLockInfo->Lock();
	CString strUseAccount = _T("");
	CString strVesionId = _T("");

	strUseAccount = DecodeString(m_strUserName);
	m_strUseAccount = strUseAccount;         //保存解密后未url编码前的当前客户端用户名
	strVesionId = m_strVersinId;
	m_pLockInfo->Unlock();

	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("接收到的账号：%s 版本号：%s"), strUseAccount, strVesionId);

	/*strUseAccount = URLEncode(strUseAccount);*/
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("接收到的账号Rul编码后：%s 版本号：%s"), strUseAccount, strVesionId);
	//获取客户地址
	CString strArea = _T("");
	strArea = GetArea();

	CString strURL = _T("");
	CString strTEP = _T("");
	CTime timep;
	timep = CTime::GetCurrentTime();
	CString strTime = _T("");
	strTime.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), timep.GetYear(), timep.GetMonth(), timep.GetDay(), timep.GetHour(), timep.GetMinute(), timep.GetSecond());

	switch (iType)
	{
	case eType_QuickPhoto:
		// 关键字排名任务请求地址
		strTEP.Format(_T("<@>&%s&%s&%s&%s&%s&<@>"), strTime, strUseAccount, strVesionId, GetPhysicalAddress(),strArea);
		EncryptRequData(strTEP);
		strURL.Format(_T("%s/api/KeywordCacheService/KeyWordRequst?key=%s"), m_strKeyWordServer,strTEP);
		break;
	case eType_ShopTraffic:
		// 刷新商铺信息任务地址
		strTEP.Format(_T("<@@@@>&%s&%s&%s&%s&<@@@@>"), strTime, strUseAccount, strVesionId, GetPhysicalAddress());
		EncryptRequData(strTEP);
		strURL.Format(_T("%s/api/cloudshop/GetVisitTask?key=%s"), m_strShopTrafServer,strTEP);
		break;
	case eType_Inforefr:
		// 信息刷新地址
		strTEP.Format(_T("<@@@>&%s&%s&%s&%s&<@@@>"), strTime, strUseAccount, strVesionId, GetPhysicalAddress());
		EncryptRequData(strTEP);
		strURL.Format(_T("%s/api/RefreshService/RefreshRequst?key=%s"), m_strRefreshServer,strTEP);
		break;
	case eType_MainTask:
		{	
			strTEP.Format(_T("<@@>&%s&%s&%s&%s&<@@>"),strTime, strUseAccount, strVesionId, GetPhysicalAddress());
			EncryptRequData(strTEP);
 			if (m_bIsRequestMainObjTask)
			{	
				m_bIsObjTask = TRUE;
				strURL.Format(_T("%s/api/Publish/GetAffectedPublishData?key=%s"), m_strMainTaskServer,strTEP);
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("(配置了RunObj标记)本次推广任务请求的是抢占焦点网站任务!"));
			}
			else
			{	
				//add by zhumingxing 20160104 根据用户屏幕锁定状态来确定获取何种云推广任务
				if (m_bIsWin10Version)
				{
					m_bIsObjTask = FALSE;
					strURL.Format(_T("%s/api/Publish/GetUnAffectedPublishData?key=%s"), m_strMainTaskServer, strTEP);
					g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("win10系统固定请求非抢占焦点网站任务!"));
				}
				else if (IsLockScreen())
				{	
					m_bIsObjTask = FALSE;
					strURL.Format(_T("%s/api/Publish/GetUnAffectedPublishData?key=%s"), m_strMainTaskServer,strTEP);
					g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("出现锁屏或屏幕保护本次推广任务请求的是非抢占焦点网站任务!"));	
				}
				else
				{	
					m_bIsObjTask = TRUE;
					strURL.Format(_T("%s/api/Publish/GetAffectedPublishData?key=%s"), m_strMainTaskServer,strTEP);
					g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("未出现锁屏或屏幕保护本次推广任务请求的是抢占焦点网站任务!"));
				}
				//end
			}			
		}
		break;

	default:
		break;
	}
	return strURL;
}


// 根据iType创建不同任务请求线程,0关键字排名请求线程,1一般任务请求线程
bool CYunTaskStreamCtr::StartExecYunTaskRequest()
{	
	m_pLockInfo->Lock();
	bool bReturn = false;

	//请求云任务数据
	HANDLE hGeneralThread = CreateThread(NULL, 0, &CYunTaskStreamCtr::ThreadProcReqYunTask, this, 0, NULL);
	if (hGeneralThread)
	{
		CloseHandle(hGeneralThread);
		bReturn = true;
	}
	m_pLockInfo->Unlock();

	return bReturn;
}

//准备自动任务,完成后发送通告执行任务,bType用于标识T_TASK_DATA中的bGeneralTask成员的值
bool CYunTaskStreamCtr::PrepareTask(T_DATA_FROM_SERVER &tData, bool bType)
{
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("enter PrepareTask"));

	int nTaskCount = 0;
	//将任务压入任务管理器
	std::vector<T_TASK_DATA>::iterator itTask = tData.vTask.begin();
	for (; itTask != tData.vTask.end(); ++itTask)
	{
		(*itTask).bGeneralTask = bType;
		m_objTaskMgr->PushTask(*itTask);
		nTaskCount++;
	}

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到%d个任务！当前缓冲%d个，正在执行%d个！"), nTaskCount,
		m_objTaskMgr->GetCachingTaskCount(), m_objTaskMgr->GetExecingTaskCount());

	tData.vTask.clear();

	//发送通告启动自动任务的轮询线程
	if (nTaskCount > 0)
	{
		m_objTaskMgr->OnNotifyRollingTask();
	}

	return true;
}


//获取用户对应产品下的普通任务
void CYunTaskStreamCtr::RequestYunTask(int iType)
{
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到普通请求任务,任务类型：%d"), iType);
	EnterCriticalSection(&g_Section);

	//重置任务提交标记
	m_bIsSubmitResult = TRUE;

	// 请求其他任务失败就继续请求下一个任务,直到取了一轮任务（3次）
	static int iRequestCnt = 0;
	int iCount = 0;
	int iRet = -1;
	do 
	{
		if (iCount == 4)
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("请求普通任务任务连续失败!"));
			break;
		}	
		//add by zhumingxing 20151105,新增对请求的控制
		if (CanRequestTask(iType))
		{
			//类型任务先获取调试数据，如果调试数据不存在或者是解析失败再去线上数据
			iRet = GetTaskDebugData(m_tTaskInfo, iType);

			if (iRet != ERRCODE_SUCCESS)
			{
				//g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("本地调试关键词任务数据解析失败，从服务器获取关键词任务数据!"));
				iRet = GetTaskData(GetURLAddress(iType), m_tTaskInfo);
			}
			else
			{
				//本地调试数据不需要提交
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("本次任务为本地调试!Type=%d"),iType);
				m_bIsSubmitResult = FALSE;
			}

			++iCount;
			if (iRet == ERRCODE_SUCCESS)  // 成功就返回
			{
				iRequestCnt = 0;
				m_iTime = m_iOldTime;
				PrepareTask(m_tTaskInfo, true);
				LeaveCriticalSection(&g_Section);
				return;
			}
			else // 失败就继续循环直到3次或者成功
			{
				if (iType == eType_MainTask)
				{
					break;
				}
				else
				{
					++iType;
				}
			}
		}
		else
		{
			++iCount;
			++iType;
		}

	}while (true);

	iRequestCnt++;
	LeaveCriticalSection(&g_Section);
	if (iRequestCnt>3)
	{
		m_iTime = m_iOldTime * 5;			//3次请求失败，则等待10分钟；
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("连续3次请求任务为空，调整等待时间为：%d"), m_iTime);
	}
	else if (iRequestCnt>8)
	{
		m_iTime = m_iOldTime * 10;			//3次请求失败，则等待20分钟；
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("连续3次请求任务为空，调整等待时间为：%d"), m_iTime);
	}
	//失败后,发送通告延时m_iTime分钟在取
	this->NotifyRequestTask(eType_QuickPhoto);

}

// 请求任务回调函数,iType用于请求何种任务
bool CYunTaskStreamCtr::NotifyRequestTask(int iType)
{
	m_pLockRequest->Lock();
	m_iTypeTask = iType;
	// 收到通告,创建单独一个线程防止阻塞
	HANDLE hNotifyThread = CreateThread(NULL, 0, &CYunTaskStreamCtr::ThreadProcNotify, this, 0, NULL);
	if (hNotifyThread)
	{
		CloseHandle(hNotifyThread);
	}
	m_pLockRequest->Unlock();

	return true;
}

// 通告线程函数
DWORD WINAPI CYunTaskStreamCtr::ThreadProcNotify(LPVOID lpParameter)
{
	CYunTaskStreamCtr* pThis = (CYunTaskStreamCtr*)lpParameter;
	// 在一般任务中eType_QuickPhoto是阀值,说明一轮任务的开始
	if (pThis->m_iTypeTask == eType_QuickPhoto)
	{
		int iTime = pThis->m_iTime;
		// 结束一轮任务后延时iTime s之后请求下轮任务
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, \
					_T("########################完成一轮任务,开启定时器,定时时间%ds########################"), iTime);
		pThis->AddTaskRoundCount();

		Sleep(iTime * 1000);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("定时请求到时：启动任务请求线程"));
	}

	pThis->StartExecYunTaskRequest();

	return 0;
}

// 请求普通任务线程函数
DWORD WINAPI CYunTaskStreamCtr::ThreadProcReqYunTask(LPVOID lpParameter)
{
	CYunTaskStreamCtr* pThis = (CYunTaskStreamCtr*)lpParameter;
	pThis->RequestYunTask(pThis->m_iTypeTask);
	return 0;
}

//从服务器请求自动任务数据
int CYunTaskStreamCtr::GetTaskData(CString strURL, T_DATA_FROM_SERVER &tData)
{	
	CString strResponseText = _T("");
	CHttpUtils http;


// 	strURL = _T("http://tui.sumszw.com/api/Publish/GetAffectedPublishData?key=4pGD4pC%2F4pC%2F4pGB4pGZ4pGN4pGP4pGO4pGH4pGS4pGP4pGK4pGS4pGP4pGN4pGf4pGO4pGL4pGF4pGP4pGM4pGF4pGO4pGO4pGZ4pGZ4pGP4pGZ4pGI4pGL4pGS4pC74pGL4pGS4pGM4pGK4pGS4pC84pC94pGS4pGP4pGL4pGS4pGN4pGI4pGZ4pGD4pC%2F4pC%2F4pGB");
	strResponseText = http.GetSerRespInfo(strURL.GetBuffer());
// 	strResponseText = _T("\
// 	<root>\
// 	<interact timemode = \"\" time = \"\"/>\
// 	<tasklist>\
// 	<task uid = \"\" vid = \"\" exclusive = \"\" timemode = \"\" time = \"\" type = \"QuickPhoto\" subject = \"抓取快照\" data = \"QuickPhoto(;0)C3ey9gUPZRcv9WhI5trFYgr5Bz7ky4edy9Aw4ob4U3TFaVl36eW6piFbglnTKzaV2iwtmVuataKVUrvB+sc5mIK4gcrEF4465uaL+kYF6xsLRgVlOAqT7Y0TpMzfL+cvz/Vak6N3uSzlXjzN/XnuPv3+ft/n9zx3jczF7NmtiTWxBraL7YFfu9nyqUu+Cl9iziPXJC4K+w/hfjfsD2c3/OWAXyeyG/2qZ7jRrzqGG123ED4bor+pdeSO3u13+B8K1b9SJ3waxztvk76csAUq5faZmPi3RD1qj1M/1D6d64/2NGb3tMCTqjmbgRTPQE7OOCp9HqTrNgNlC2vOUT4u7BAeCQxLN3q/6vDI+L/dPKROiMSWHOnSuZrTCqVRwuruHwepD2pLE7FO4ONabvfumrwV+6xfyczNFbVI6s5paT7pgYhHLr2uzRCxLiBtAFey52gldhD2Xo6LIxn5d6HiGMXcRr0I79HAqrufOkk9UauNYr8kjjgTW2Nav6XzqV3rr8p4ofgrp2qCO+M16K9L5yYk85j0aGc2jQtUWksz4b+XHZeMjImlU8jNnDmmRoiSA6zzv8gzg8t6lRpXZRKPOYrcjeaQnqrF3HjDOoXHm5I5ijPUsVZwiPIjsgMPrytRm+fFrbGM/K73bMiautJchewwQvHsUViMqoLjkuIG1BO1xfVqZOGVhXks1juAY40DmENTChK1A+xnz7Iuw1lvPYUT5li8CrgWfSx9JSyx52DfPWFvL0V3+fhyyyoecmQRHNmZuOT7rvuw14xKPDX0YcFZsplu5oasccCGemC1VjM5PMbCcZGYzNCqFExqVzt4xpnIyETkkV+Xyy363BB76G+kkpksemdE5kZdE8x++MxHcYc4i7IB9hLrZ3+knImfvR9EjOpUmvoa1UK3ILWKRWd1bYpVTz7VjKplPY9rVVbx+IOVykQ50rhUtRyn2ZldaTDKY3WF6oRow3jD9SDuFK3ioz1LA4Oh6ykzRlv9TH3ifPTmkR3CeqvS6mP0X6Z27YqJekVaIWyAKHQDJ16DYzCuhacjd8eoj43WSPrp2i/ciS0m0xHt+o+yKk9H6XmoLczmBjIcubgXbvcuJn2gK80Ijaye6OfLTP0yB6tqqfUY1ceIjFo9hLj6rI7QGOtIb+lY6aw6/H2g8vTbI7WB4CT1V0wlv5WujTwj7A68Gd0m7fR+0r3lPqpXeV+rqfY8ejZqCzHj3MSezU2u6kioWmh97WqyPbhv+vwpT5HrK7ybbXZZON6zkl4YtwXeT68IWkr1cSKjVg+hA5wU3QevwZzEauSFwdsD9WGPbIbz5J+PWUGovDVRr8ucmbsaHzumMNL/qpgRaw0l+g+ntM/Yjs6n5uHtyWZRsSWbixSnmUjQi3mRiXzk35sMev/0to1boaUyUtQGTg4FB9GB1nPa1UfpqagtnJWoI86C+GwV3w/dnr4cj/b9NGWGnvpHlNTow+nkIFYE//+2er1ziJba/NTotVgBra4PPhb73qgKtI7V99wQXhSop41mav6qu05YTEbbr4Quwvvjr2d+61R7LWpe4VO/n9eeTc9GrT5a/A6Bz3WJdjRwyPvrnPG6XI/GWg4rqJ0w4vDIJmpcf+k1nMH5RxuraXF+jPUfd2WMpHdi08KTLw+Lt2JiV0f6nxOwAjBBcVOc2FY86wnfW1Ntr+KdtTUEP0LPQW3hOFYYsYbAa7CC2CPNJsSuv9LE+J7v5hHqrzz57BCvfGbWF2nwe+Tezi9yKhqlyx+1xVbPDr7L6dph5RrV+2HITP1KS4jjKc9B21F3oCq8b7w2tpxTsJxjFN86urMbd1Ucq9sGFsbNqeFxJNGjoParNa3PaI9T3lBbyG3w6z38kqQZaPEadBusjJAUv927nqKeNuoz+tdRtSzFYcXZGc/NeCgy2zRp692MqrpAU56jd6SqMH7F5zHp3fmDFcGoKY/gb44Tq9HofRAo/wVESMpoZisAwA==\" extradata = \"\" posturl = \"\"/>\
// 	</tasklist>\
// 	</root>");
// 	strResponseText = _T("\
// 	<root>\
// 	<interact timemode = \"\" time = \"\"/>\
// 	<tasklist>\
// 	<task uid = \"\" vid = \"\" exclusive = \"\" timemode = \"\" time = \"\" type = \"QuickPhoto\" subject = \"抓取快照\" data = \"QuickPhoto(;0)C3ey9VsLrSIjtPgFNFoJHGlmfoEwaLxNDGfIMKcbqTqqIhpIUElxh7JQkYVCGcpQBQkhoSDBhIQQCysSWxCwMcDExoDEwsIICCrxzuZ8dmx3iA9ysizFsZ3Y/vzf3fvfeQzGoQxF97MCh+AYWHAYdhZf1K3uuQfla73V2Su91TsP9zqPz+DRqjnadBfcKpmtDXehrQLgQlsjgAv9L9/6eJW+07rkH91s5O2dxTfb3nVvLRccvO6I/bqZc+a7a6d/1Ois8b+iM9J61D8z7Rlz91TMPQepq+5epJ6CU3ACwtx03TTcJfN0i5BvHbxBd0LrYdPjfeG9IH2Ym+5wUK1vtp826By0Hj6tFwFIe+R6zn65IsusWWfL1zkuykmhQdXWTO5FHWqOFcbqfu9iXcG6JlHTNJMf9TXfrn9oHz8fZJ90tpy0qs8uTzo687y8C7lERUdyT3l6hrQefo0b99lPQg2mTW0vwqj76VW72tyvjSedVx2J6H/bvK+sylfM2M5xP+XsWdpqh7m/Lry79632xV6aJ80GrXo66Tn2w9wSGa+12qHqHPcrsA6tv37m59nnnWcLnzu37/6up9VbX61D7gk/3/8Vt84459kKZnTO8aI9qHeWKz0qzzOXpCcgkfMa1ccxDbMZXd1++4BxdThXDWqf1dHNMroTeXB0k2bXW+kto/r/eAL6uhUVQ86zmST1045y6G11srOve3/p0Vw05yXINbp61L3f1Unnu9YZDbKzs4tqjv3BtLprGuEmDG3V1PgisL+J61VJcOur8UzP3iaOXsLX6It4okf1aYSLduIllNc0k2FqdjZRagkvr5Oa3YxX3eiNi4TOmsZx1pk9zGbj04Uwddpars29MDW7l2g3VmK2ojO62bfs3o3cd3FGoCOnuab3u5is9mE54vs7U9h1jjq4tBmvyb3xu3Rmj45lWct2puYYj1JLjGeatOY4Z6ceje7C5Z49Y6eNca1O3TKzlH7/EpyZytDrUx252cUkdWIkPKtOP4P87GeS+CUyXp+DRXZ2NfGdqCzPT5Gf63zSm4e0FU9X3P8BMU2xLSYnAMA=\" extradata = \"\" posturl = \"\"/>\
// 	</tasklist>\
// 	</root>");

// 	strResponseText = _T("\
// 	<root>\
// 	<interact timemode = \"\" time = \"\"/>\
// 	<tasklist>\
// 	<task uid = \"\" vid = \"\" exclusive = \"\" timemode = \"\" time = \"\" type = \"QuickPhoto\" subject = \"抓取快照\" data = \"QuickPhoto(;0)C3dcCFoATnRxIXE3MUVjOXweMBZ/OOjk0KCc89izU4br5q4CiVkCxeLBEMQzBvJqwBDG02cAQRhPlwEEYfrqypbFwdgw2hguC7Lp3VSWlvZUseCLwaExnOkB03ICfwTDzILJwXTCaD24CTARQ7CIBdBtAFqoJtfUAADA\" extradata = \"\" posturl = \"\"/>\
// 	</tasklist>\
// 	</root>");

	//推广测试
// 	strResponseText = _T("\
// 	<root>\
// 	<tasklist>\
// 	<task uid = \"101761\" type = \"MainTask\" data = \"MainTask(;0)0(;0)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\" posturl = \"/Publish/SaveUnAffectedPublishData\" extradata = \"173625826\"/>\
// 	</tasklist>\
// 	</root>");

// 	strResponseText = _T("\
// 	<root>\
// 	<tasklist>\
// 	<task uid = \"101761\" type = \"MainTask\" data = \"MainTask(;0)0(;0)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\" posturl = \"/Publish/SaveUnAffectedPublishData\" extradata = \"173625826\"/>\
// 	</tasklist>\
// 	</root>");

// 	strResponseText = _T("\
// 	<root>\
// 	<tasklist>\
// 		<task uid = \"101761\" type = \"MainTask\" data = \"MainTask(;0)0(;0)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\" posturl = \"/Publish/SaveUnAffectedPublishData\" extradata = \"173625826\"/>\
// 			</tasklist>\
// 	</root>");

// 	strResponseText = _T("\
// 		<root>\
// 		<tasklist>\
// 		<task uid = \"zzds\" type = \"ShopTraffic\" data = \"ShopTraffic(;0)40000563(;0)zzds.16898.cc?optimizeImage=1(;0)3(;0)1\" / >\
// 		< / tasklist>\
// 		</root>");

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("上层传过来的数据:%s"), strResponseText);

	if (0 == strResponseText.CompareNoCase(_T("$ERR$")))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("发送任务请求失败! 地址：%s , LastErr:%d, ErrMsg:%s"), strURL.GetBuffer(), GetLastError(), strResponseText);
		return ERRCODE_FAIL;
	}
	else if (strResponseText.IsEmpty())
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("用户[%s]没有需要执行的自动任务(无数据返回)！地址：%s"), m_strUseAccount, strURL);
		return ERRCODE_NO_TASK;
	}

	int iMaxXMLen = 512 * 1024;       //512k
	TCHAR *ptmpBuf = NULL;
	char *pmsbXMLData = NULL;

	try
	{
		ptmpBuf = new TCHAR[iMaxXMLen];
		memset(ptmpBuf, 0, iMaxXMLen* sizeof(TCHAR));
		memcpy_s(ptmpBuf, iMaxXMLen* sizeof(TCHAR), strResponseText.GetBuffer(), strResponseText.GetLength() * sizeof(TCHAR));

		DWORD dwSize = strResponseText.GetLength() * 2;
		pmsbXMLData = new char[dwSize];
		memset(pmsbXMLData, 0, dwSize);
		WCharToMByte(ptmpBuf, pmsbXMLData, &dwSize);

		TiXmlDocument doc;
		doc.Parse(pmsbXMLData);

		delete[]pmsbXMLData;
		delete[]ptmpBuf;

		if (!ParseServerResponse(&doc, tData, m_strVersinId))
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("解析任务数据失败！URL:%s\r\n"), strURL);
			return ERRCODE_FAIL;
		}
	}
	catch (...)
	{
		if (!ptmpBuf)
			delete[]ptmpBuf;

		if (!pmsbXMLData)
			delete[]pmsbXMLData;

		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("解析任务数据异常！URL:%s\r\n"), strURL);
		return ERRCODE_FAIL;
	}

	return ERRCODE_SUCCESS;
}

void CYunTaskStreamCtr::GetDebugIniData(int itype)
{	
	if (itype == eType_QuickPhoto)
	{
		g_iniDebugData.ReadString(_T("QuickPhoto"), _T("data"), m_strDebugTaskData);
	}
	else if (itype == eType_ShopTraffic)
	{
		g_iniDebugData.ReadString(_T("ShopTraffic"), _T("data"), m_strDebugTaskData);
	}
	else if (itype == eType_Inforefr)
	{
		g_iniDebugData.ReadString(_T("Inforefr"), _T("data"), m_strDebugTaskData);
	}
	else if (itype == eType_MainTask)
	{
		g_iniDebugData.ReadString(_T("MainTask"), _T("data"), m_strDebugTaskData);
	}
}

int CYunTaskStreamCtr::GetTaskDebugData(T_DATA_FROM_SERVER &tData, int iType)
{	
	//获取对应任务类型的调试数据
	GetDebugIniData(iType);

	CString strResponseText = CString(m_strDebugTaskData.GetBuffer());

	int iMaxXMLen = 512 * 1024;       //512k
	TCHAR *ptmpBuf = NULL;
	char *pmsbXMLData = NULL;

	try
	{
		ptmpBuf = new TCHAR[iMaxXMLen];
		memset(ptmpBuf, 0, iMaxXMLen* sizeof(TCHAR));
		memcpy_s(ptmpBuf, iMaxXMLen* sizeof(TCHAR), strResponseText.GetBuffer(), strResponseText.GetLength() * sizeof(TCHAR));

		DWORD dwSize = strResponseText.GetLength() * 2;
		pmsbXMLData = new char[dwSize];
		memset(pmsbXMLData, 0, dwSize);
		WCharToMByte(ptmpBuf, pmsbXMLData, &dwSize);

		TiXmlDocument doc;
		doc.Parse(pmsbXMLData);

		delete[]pmsbXMLData;
		delete[]ptmpBuf;

		if (!ParseServerResponse(&doc, tData, m_strVersinId))
		{
			return ERRCODE_FAIL;
		}
	}
	catch (...)
	{
		if (!ptmpBuf)
			delete[]ptmpBuf;

		if (!pmsbXMLData)
			delete[]pmsbXMLData;

		return ERRCODE_FAIL;
	}

	return ERRCODE_SUCCESS;

}

// 解析从服务端请求到的自动任务数据
bool CYunTaskStreamCtr::ParseServerResponse(TiXmlDocument *pDoc, T_DATA_FROM_SERVER &tTaskInfo, CString strVersionId)
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

	//获取任务列表
	TiXmlElement *pTaskList = pRoot->FirstChildElement("tasklist");
	if (!pTaskList)
	{
		//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("关键词数据为空"));
		return false;  //为空说明没有任务，解析失败
	}

	TiXmlElement *pTaskItem = pTaskList->FirstChildElement("task");
	if (!pTaskItem)
	{
		return false;  //为空说明没有任务，解析失败
	}

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

		// 上层传过来的数据有问题丢弃

		//补充信息
		if (g_TypeMap[tData.strType] == eType_QuickPhoto)
		{
			CString strTemp = tData.strData;
			if (strTemp.IsEmpty())
			{
				return false;
			}

			//此处做一个兼容性处理
			if (tData.strPostAddr.IsEmpty())
			{
				tData.strPostAddr = m_strKeyWordServer + _T("/api/KeywordCacheService/SaveKeyWordDetail");
			}
			else
			{
				tData.strPostAddr = m_strKeyWordServer + _T("/api") + tData.strPostAddr;
			}	
		}
		if (g_TypeMap[tData.strType] == eType_Inforefr)
		{
			CStdString strTemp = tData.strData;
			// 上层传过来的数据有问题丢弃 属性字段全为空
			if (strTemp.length() < 76)
			{
				g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("上层传过来的数据有问题属性全为空丢弃"));
				return false;
			}
			tData.strPostAddr = m_strRefreshServer + _T("/api") + tData.strPostAddr;
		}
		//add by zhumingxing 20151106(抢焦点与不抢焦点网站提交地址是否一致)
		if (g_TypeMap[tData.strType] == eType_MainTask)
		{
			CStdString strTemp = tData.strData;
			// 上层传过来的数据有问题丢弃 属性字段全为空
			if (strTemp.length() < 76)
			{
				g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("上层传过来的数据有问题属性全为空丢弃"));
				return false;
			}
			tData.strPostAddr = m_strMainTaskServer + _T("/api") + tData.strPostAddr;

		}
		//end add

		tData.strCUID = m_strUseAccount;
		tData.strPdtVer = strVersionId;

		tTaskInfo.vTask.push_back(tData);
	}

	return true;
}

BYTE CYunTaskStreamCtr::ToHex(const BYTE &x)
{
	return x > 9 ? x + 55 : x + 48;
}

// URL编码
CString CYunTaskStreamCtr::URLEncode(CString sIn)
{
	int ilength = -1;
	char* pUrl = CStringToUtf8Char(sIn, ilength);
	CStringA strSrc(pUrl);

	CStringA sOut;
	const int nLen = strSrc.GetLength() + 1;

	register LPBYTE pOutTmp = NULL;
	LPBYTE pOutBuf = NULL;
	register LPBYTE pInTmp = NULL;
	LPBYTE pInBuf = (LPBYTE)strSrc.GetBuffer(nLen);
	BYTE b = 0;

	//alloc out buffer
	pOutBuf = (LPBYTE)sOut.GetBuffer(nLen * 3 - 2);//new BYTE [nLen  * 3];

	if (pOutBuf)
	{
		pInTmp = pInBuf;
		pOutTmp = pOutBuf;

		// do encoding
		while (*pInTmp)
		{
			if (isalnum(*pInTmp))
				*pOutTmp++ = *pInTmp;
			else
			if (isspace(*pInTmp))
				*pOutTmp++ = '+';
			else
			{
				*pOutTmp++ = '%';
				*pOutTmp++ = ToHex(*pInTmp >> 4);
				*pOutTmp++ = ToHex(*pInTmp % 16);
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

char* CYunTaskStreamCtr::CStringToMutilChar(CString& str, int& chLength)
{
	char* pszMultiByte;
	int iSize = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	pszMultiByte = (char*)malloc((iSize + 1)/**sizeof(char)*/);
	memset(pszMultiByte, 0, iSize + 1);
	WideCharToMultiByte(CP_ACP, 0, str, -1, pszMultiByte, iSize, NULL, NULL);
	chLength = iSize;
	return pszMultiByte;
}

char* CYunTaskStreamCtr::CStringToUtf8Char(CString& str, int& chLength)
{
	char* pszMultiByte;
	int iSize = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	pszMultiByte = (char*)malloc((iSize + 1)/**sizeof(char)*/);
	memset(pszMultiByte, 0, iSize + 1);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, pszMultiByte, iSize, NULL, NULL);
	chLength = iSize;
	return pszMultiByte;
}


CString CYunTaskStreamCtr::DecodeString(CString& strDest)
{
	CString strDecodeData = stringcoding::StringBase64Decode(strDest);

	//进行一次异或操作
	TCHAR ch = _T('①');
	for (int i = 0; i < strDecodeData.GetLength(); ++i)
	{
		DWORD Temp = strDecodeData[i];
		Temp = Temp^ch;
		strDecodeData.SetAt(i, Temp);
	}
	return strDecodeData;
}

bool CYunTaskStreamCtr::StopTask()
{
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("enter StopTask！"));

	StopCheckThread();
	m_objTaskMgr->SetPollingFlag(true);
	m_objTaskMgr->WaitEnd();
	m_objTaskMgr->TerminateAllTasks();

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("out StopTask！"));
	return true;
}

int CYunTaskStreamCtr::GetOutTime(int iType)
{

	if (iType == 0)
	{
		return m_iTaskKeyWordTimeOut;
	}
	else
	{
		return m_iTaskGeneralTimeOut;
	}
	
}

BOOL CYunTaskStreamCtr::IsNeedSumitResult()
{
	return m_bIsSubmitResult;
}

BOOL CYunTaskStreamCtr::MathCanRequestType(int iType)
{	
	//默认所有任务全都启用
	if (m_strTaskRequestFlag.IsEmpty())
	{
		return TRUE;
	}
	//表示请求标志少配置了iType类型任务
	if (m_strTaskRequestFlag.GetLength() < iType)
	{
		return FALSE;
	}
	else
	{
		TCHAR CFlag = m_strTaskRequestFlag[iType - 1];

		if (CFlag == _T('1'))
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CYunTaskStreamCtr::CanRequestTask(int iType)
{	
	//首先通过配置文件中的任务请求标志，确定此任务是否能够请求数据
	if (!MathCanRequestType(iType))
	{	
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("YunTask配置文件中没有启动任务类型:%d"), iType);
		return FALSE;
	}

	//add by zhumingxing 20150404,云推广需要满足系统空闲条件才能请求
	if (iType == eType_MainTask && !m_bIsRequestMainObjTask)
	{	
		//不满足可以执行云推广任务的条件，系统空闲时间不够
		if (!IsInputIdleOk())
		{	
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("云推广任务未满足系统空闲时间规则,放弃请求!"));
			return FALSE;
		}
	}
	//end
	//itype类型任务如果未失败过，或者是只失败一次，或者是已经等待了对应的轮数，则可以请求任务
	if (iType == eType_QuickPhoto)
	{	
		if ((m_tRoundCount.m_dwKwTaskErrorCount < 2) 
			|| m_tRoundCount.m_dwKeyWordCurrWaitRound == m_tRoundCount.m_DwKeyWordErrorWaitRound)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}

	}
	else if (iType == eType_ShopTraffic)
	{
		if ((m_tRoundCount.m_dwShopTaskErrorCount < 2)
			|| m_tRoundCount.m_dwShopCurrWaitRound == m_tRoundCount.m_DwShopErrorWaitRound)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else if (iType == eType_Inforefr)
	{
		if ((m_tRoundCount.m_dwFreshTaskErrorCount < 2)
			|| m_tRoundCount.m_dwFreshCurrWaitRound == m_tRoundCount.m_dwFreshErrorWaitRound)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else if (iType == eType_MainTask)
	{
		if ((m_tRoundCount.m_dwMainTaskErrorCount < 2)
			|| m_tRoundCount.m_dwMainTaskCurrWaitRound == m_tRoundCount.m_dwMainTaskWaitRound)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	return FALSE;
}

void CYunTaskStreamCtr::AddTaskRoundCount()
{	
	m_pLockRoundCount->Lock();

	if ((m_tRoundCount.m_dwKeyWordCurrWaitRound < m_tRoundCount.m_DwKeyWordErrorWaitRound)
		&& m_tRoundCount.m_dwKwTaskErrorCount > 1)
	{
		++m_tRoundCount.m_dwKeyWordCurrWaitRound;
	}
	if ((m_tRoundCount.m_dwShopCurrWaitRound <  m_tRoundCount.m_DwShopErrorWaitRound)
		&& m_tRoundCount.m_dwShopTaskErrorCount > 1)
	{
		++m_tRoundCount.m_dwShopCurrWaitRound;
	}
	if ((m_tRoundCount.m_dwFreshCurrWaitRound <  m_tRoundCount.m_dwFreshErrorWaitRound)
		&& m_tRoundCount.m_dwFreshTaskErrorCount > 1)
	{
		++m_tRoundCount.m_dwFreshCurrWaitRound;
	}
	if ((m_tRoundCount.m_dwMainTaskCurrWaitRound < m_tRoundCount.m_dwMainTaskWaitRound)
		&& m_tRoundCount.m_dwMainTaskErrorCount > 1)
	{
		++m_tRoundCount.m_dwMainTaskCurrWaitRound;
	}

	m_pLockRoundCount->Unlock();
}
void CYunTaskStreamCtr::SetRoundCount(int Type)
{
	m_pLockRoundCount->Lock();

	if (Type == eType_QuickPhoto)
	{
		if (m_tRoundCount.m_dwKwTaskErrorCount > 0
			|| m_tRoundCount.m_dwKeyWordCurrWaitRound > 0
			|| m_tRoundCount.m_DwKeyWordErrorWaitRound > 1)
		{
			m_tRoundCount.m_dwKeyWordCurrWaitRound = 0;
			m_tRoundCount.m_dwKwTaskErrorCount = 0;
			m_tRoundCount.m_DwKeyWordErrorWaitRound = 1;
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("关键词任务等待轮数重新归零"));
		}
	}
	else if (Type == eType_ShopTraffic)
	{
		if (m_tRoundCount.m_dwShopTaskErrorCount > 0
			|| m_tRoundCount.m_dwShopCurrWaitRound > 0
			|| m_tRoundCount.m_DwShopErrorWaitRound > 1)
		{
			m_tRoundCount.m_dwShopTaskErrorCount = 0;
			m_tRoundCount.m_dwShopCurrWaitRound = 0;
			m_tRoundCount.m_DwShopErrorWaitRound = 1;
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("刷新商铺流量任务等待轮数重新归零"));
		}
	}
	else if (Type == eType_Inforefr)
	{
		if (m_tRoundCount.m_dwFreshTaskErrorCount > 0
			|| m_tRoundCount.m_dwFreshCurrWaitRound > 0
			|| m_tRoundCount.m_dwFreshErrorWaitRound > 1)
		{
			m_tRoundCount.m_dwFreshTaskErrorCount = 0;
			m_tRoundCount.m_dwFreshCurrWaitRound = 0;
			m_tRoundCount.m_dwFreshErrorWaitRound = 1;
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("网站刷新任务等待轮数重新归零"));
		}
	}
	else if (Type == eType_MainTask)
	{
		if (m_tRoundCount.m_dwMainTaskErrorCount > 0
			|| m_tRoundCount.m_dwMainTaskCurrWaitRound > 0
			|| m_tRoundCount.m_dwMainTaskWaitRound > 1)
		{
			m_tRoundCount.m_dwMainTaskErrorCount = 0;
			m_tRoundCount.m_dwMainTaskCurrWaitRound = 0;
			m_tRoundCount.m_dwMainTaskWaitRound = 1;
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("网站推广任务等待轮数重新归零"));
		}
	}

	m_pLockRoundCount->Unlock();
}

void CYunTaskStreamCtr::ResetRoundCount(int Type)
{	
	m_pLockRoundCount->Lock();

	if (Type == eType_QuickPhoto)
	{	
		if (m_tRoundCount.m_dwKwTaskErrorCount == 0)
		{
			++m_tRoundCount.m_dwKwTaskErrorCount;
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("刷新关键词排名任务首次失败,不需要等待，下轮继续请求!"));
			m_pLockRoundCount->Unlock();
			return;
		}
		++m_tRoundCount.m_dwKwTaskErrorCount;
		m_tRoundCount.m_dwKeyWordCurrWaitRound = 0;
		m_tRoundCount.m_DwKeyWordErrorWaitRound = m_tRoundCount.m_DwKeyWordErrorWaitRound * 5;
		//新增上限25轮 2018/05/04
		if (m_tRoundCount.m_DwKeyWordErrorWaitRound > 25)
		{
			m_tRoundCount.m_DwKeyWordErrorWaitRound = 25;
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("下次请求关键词任务需要等待轮数已达上限25轮,任务失败次数:%d!"),
				m_tRoundCount.m_dwKwTaskErrorCount);
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("由于本次关键词任务出错,下次请求关键词任务需要等待%d轮任务之后,任务失败次数:%d!"),
				m_tRoundCount.m_DwKeyWordErrorWaitRound, m_tRoundCount.m_dwKwTaskErrorCount);
		}
	}
	else if (Type == eType_ShopTraffic)
	{	
		if (m_tRoundCount.m_dwShopTaskErrorCount == 0)
		{
			++m_tRoundCount.m_dwShopTaskErrorCount;
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("刷新商铺流量任务首次失败,不需要等待，下轮继续请求!"));
			m_pLockRoundCount->Unlock();
			return;
		}
		++m_tRoundCount.m_dwShopTaskErrorCount;
		m_tRoundCount.m_dwShopCurrWaitRound = 0;
		m_tRoundCount.m_DwShopErrorWaitRound = m_tRoundCount.m_DwShopErrorWaitRound * 5;

		//新增上限25轮 2018/05/04
		if (m_tRoundCount.m_DwShopErrorWaitRound > 25)
		{
			m_tRoundCount.m_DwShopErrorWaitRound = 25;
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("下次请求商铺流量任务需要等待轮数已达上限25轮,任务失败次数:%d!"),
				m_tRoundCount.m_dwShopTaskErrorCount);
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("由于本次商铺流量任务出错,下次请求商铺流量任务需要等待%d轮任务之后,任务失败次数:%d!"),
				m_tRoundCount.m_DwShopErrorWaitRound, m_tRoundCount.m_dwShopTaskErrorCount);
		}

	}
	else if (Type == eType_Inforefr)
	{	
		if (m_tRoundCount.m_dwFreshTaskErrorCount == 0)
		{	
			++m_tRoundCount.m_dwFreshTaskErrorCount;
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("网站刷新任务首次失败,不需要等待，下轮继续请求!"));
			m_pLockRoundCount->Unlock();
			return;
		}
		++m_tRoundCount.m_dwFreshTaskErrorCount;
		m_tRoundCount.m_dwFreshCurrWaitRound = 0;
		m_tRoundCount.m_dwFreshErrorWaitRound = m_tRoundCount.m_dwFreshErrorWaitRound * 5;


		//新增上限25轮 2018/05/04
		if (m_tRoundCount.m_dwFreshErrorWaitRound > 25)
		{
			m_tRoundCount.m_dwFreshErrorWaitRound = 25;
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("下次请求信息刷新任务需要等待轮数已达上限25轮,任务失败次数:%d!"),
				m_tRoundCount.m_dwFreshTaskErrorCount);
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("由于本次信息刷新任务出错,下次请求信息刷新任务需要等待%d轮任务之后,任务失败次数:%d!"),
				m_tRoundCount.m_dwFreshErrorWaitRound, m_tRoundCount.m_dwFreshTaskErrorCount);
		}

	}
	else if (Type == eType_MainTask)
	{
		if (m_tRoundCount.m_dwMainTaskErrorCount == 0)
		{
			++m_tRoundCount.m_dwMainTaskErrorCount;
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("网站推广任务首次失败,不需要等待，下轮继续请求!"));
			m_pLockRoundCount->Unlock();
			return;
		}
		++m_tRoundCount.m_dwMainTaskErrorCount;
		m_tRoundCount.m_dwMainTaskCurrWaitRound = 0;
		m_tRoundCount.m_dwMainTaskWaitRound = m_tRoundCount.m_dwMainTaskWaitRound * 5;

		//新增上限25轮 2018/05/04
		if (m_tRoundCount.m_dwMainTaskWaitRound > 25)
		{
			m_tRoundCount.m_dwMainTaskWaitRound = 25;
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("下次请求网站推广任务需要等待轮数已达上限25轮,任务失败次数:%d!"),
				m_tRoundCount.m_dwMainTaskErrorCount);
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("由于本次网站推广任务出错,下次请求网站推广任务需要等待%d轮任务之后,任务失败次数:%d!"),
				m_tRoundCount.m_dwMainTaskWaitRound, m_tRoundCount.m_dwMainTaskErrorCount);
		}

	}

	m_pLockRoundCount->Unlock();
}

BOOL CYunTaskStreamCtr::GetRunObjTaskFlag()
{
	return m_bIsRequestMainObjTask;
}

DWORD WINAPI CYunTaskStreamCtr::ThreadProcCheckState(LPVOID lpParameter)
{	
	//内核触发系统输入操作的最后时间
	DWORD dwEngineInputTime = 0;
	CYunTaskStreamCtr* pThis = (CYunTaskStreamCtr*)lpParameter;

	while (!pThis->m_bCheckStop)
	{	
		pThis->m_pCheckIdle->Lock();

		LASTINPUTINFO tInputTime = { sizeof(LASTINPUTINFO) };
		GetLastInputInfo(&tInputTime);

		//此时为内核触发系统输入事件，记录其时间
		if (WAIT_TIMEOUT == WaitForSingleObject(pThis->m_hSystemInputMuext, 0))
		{	
			dwEngineInputTime = tInputTime.dwTime;
		}
		else
		{
			ReleaseMutex(pThis->m_hSystemInputMuext);

			//如果两个时间相等表示内核触发输入事件后用户未继续触发
			if (dwEngineInputTime != tInputTime.dwTime)
			{
				pThis->m_dwLastInputTime = tInputTime.dwTime;
			}
		}

		pThis->m_pCheckIdle->Unlock();

		Sleep(50);
	}
	return 0;
}

BOOL CYunTaskStreamCtr::IsInputIdleOk()
{
	m_pCheckIdle->Lock();
	if (((GetTickCount() - m_dwLastInputTime)/1000) >= m_iSystemInputIdle)
	{	
		m_pCheckIdle->Unlock();
		return TRUE;
	}
	else
	{	
		m_pCheckIdle->Unlock();
		return FALSE;
	}
}

BOOL CYunTaskStreamCtr::IsLockScreen()
{	
	m_pCheckLockScreen->Lock();

	BOOL bRunning = FALSE;
	BOOL bLocked = FALSE;

	//检测屏保是否正在运行
	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &bRunning, 0);

	//检测是否运行屏幕锁定
	HDESK hDesk = OpenDesktop(_T("Default"), 0, FALSE, DESKTOP_SWITCHDESKTOP);
	if (hDesk)
	{
		bLocked = !SwitchDesktop(hDesk);
		CloseDesktop(hDesk);
	}

	if (!bRunning && !bLocked)
	{	
		m_pCheckLockScreen->Unlock();
		return FALSE;
	}

	m_pCheckLockScreen->Unlock();
	return TRUE;
}

BOOL CYunTaskStreamCtr::IsObjTaskRunning()
{
	return m_bIsObjTask;
}

void CYunTaskStreamCtr::StopCheckThread()
{
	m_bCheckStop = TRUE;

	if (NULL != m_hCheckThread)
	{
		WaitForSingleObject(m_hCheckThread, 100);
		CloseHandle(m_hCheckThread);
		m_hCheckThread = NULL;
	}
}

BOOL CYunTaskStreamCtr::IsWin10SystemVersion()
{
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
	if (dwMajorVersion >= 10)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

DWORD CYunTaskStreamCtr::EncryptRequData(CString & strDest)
{
	CString strContent = _T("");

	strContent = strDest;

	if (strDest.GetLength()<= 0)
	{
		return 1;
	}
	//进行一次异或操作
	TCHAR ch = _T('⑿');
	for (int i = 0; i < strContent.GetLength(); ++i)
	{
		DWORD Temp = strContent[i];
		Temp = Temp^ch;
		strContent.SetAt(i, Temp);
	}

	strDest = stringcoding::StringBase64Encode(strContent);

	strDest = URLEncode(strDest);

	return 0;
}

CString CYunTaskStreamCtr::GetAreaByIp()
{
	CString strBuf;
	char buf[MAX_PATH] = { 0 };
	CString strTmp;
	CString str =_T("深圳");
	int iPos = 1;
	//下载URL文件
	URLDownloadToFile(0, m_strGetPublicIpUrl, _T("c:\\1.ini"), 0, NULL);

	FILE *fp = fopen("c:\\1.ini", "r");
	if (fp != NULL)
	{

		fseek(fp, 0, SEEK_SET);
		fread(buf, 1, 256, fp);
		fclose(fp);

		strBuf = buf;
		/*<html>
		<head>
			<meta http - equiv = "content-type" content = "text/html; charset=gb2312">
			<title> 您的IP地址 < / title>
			< / head>
			<body style = "margin:0px"><center>您的IP是：[113.81.234.168] 来自：广东省深圳市 电信< / center>< / body>< / html>*/

		//<@>&2017-09-18 11:09:45&sddsd&0&00-1E-33-E6-41-DB&内蒙古自治区呼和浩特&<@>
		int iStartPos = strBuf.Find(_T("省"));
		if (iStartPos == -1)
		{
			iStartPos = strBuf.Find(_T("自治区"));
			if (iStartPos == -1)
			{
				iStartPos = strBuf.Find(_T("]"));
				iPos = 5;//"] 来自："
			}
			else
			{
				iPos = 3;//"自治区"
			}
		}
		int iEndPos = strBuf.Find(_T("市"), iStartPos);
		if (iEndPos == -1)
		{
			for (int i = (iStartPos + 3); i < strBuf.GetLength(); i++)
			{
				TCHAR cChar = strBuf.GetAt(i);
				if (' ' == cChar)
				{
					iEndPos = i;
					break;
				}
				else if ('<' == cChar)
				{
					iEndPos = i - 2;//2表示"电信"、"移动"、"联通"等
					break;
				}
			}
// 			iEndPos = strBuf.Find(_T(" "), iStartPos);
// 			if ()
// 			{
// 			}
// 			iEndPos = strBuf.Find(_T("</center>"), iStartPos + 3);
		}

		if (iEndPos != -1)
		{
			str = strBuf.Mid(iStartPos + iPos, iEndPos - iStartPos - iPos);
		}
		else
		{
			//异常情况也返回给上层，便于问题查找
			str = strBuf.Mid(iStartPos + iPos);
		}

		DeleteFile(_T("c:\\1.ini"));
	}

	return str;
}

bool CYunTaskStreamCtr::SplitCString(const CString & input, const CString & delimiter, std::vector<CString >& results)
{
	int iPos = 0;
	int newPos = -1;
	int sizeS2 = (int)delimiter.GetLength();
	int isize = (int)input.GetLength();

	int offset = 0;
	CString  s;

	if (
		(isize == 0)
		||
		(sizeS2 == 0)
		)
	{
		return 0;
	}

	std::vector<int> positions;

	newPos = input.Find(delimiter, 0);

	if (newPos < 0)
	{
		if (!input.IsEmpty())
		{
			results.push_back(input);
		}
		return 0;
	}

	int numFound = 0;

	while (newPos >= iPos)
	{
		numFound++;
		positions.push_back(newPos);
		iPos = newPos;
		if (iPos + sizeS2 < isize)
		{
			newPos = input.Find(delimiter, iPos + sizeS2);
		}
		else
		{
			newPos = -1;
		}
	}

	if (numFound == 0)
	{
		return 0;
	}

	for (int i = 0; i <= (int)positions.size(); ++i)
	{
		s.Empty();
		if (i == 0)
		{
			s = input.Mid(i, positions[i]);
		}
		else
		{
			offset = positions[i - 1] + sizeS2;

			if (offset < isize)
			{
				if (i == positions.size())
				{
					s = input.Mid(offset);
				}
				else if (i > 0)
				{
					s = input.Mid(positions[i - 1] + sizeS2, positions[i] - positions[i - 1] - sizeS2);
				}
			}
		}

		if ((s.GetLength() > 0))
		{
			results.push_back(s);
		}
	}
	return true;
}

BOOL CYunTaskStreamCtr::IsInAreaList(CString &strArea)
{
	if (strArea.IsEmpty())
		return FALSE;

	strArea.Trim();

	int nAreaCount = 0;
	nAreaCount = m_strAreaArray.size();
	if (nAreaCount <= 0)
		return TRUE;	//列表没有数据，不处理查询结果

	BOOL bFind = FALSE;

	//查找传进来的区域是否在包含在区域列表的区域中
	for (int i = 0; i < nAreaCount; i++)
	{
		CString strTemp = m_strAreaArray[i];
		strTemp.Trim();
		if (strTemp.IsEmpty())
			continue;

		if (strTemp.Find(strArea) == -1)	//没有找到
			continue;

		bFind = TRUE;
		break;
	}

	if (bFind)
		return TRUE;

	//区域列表中的区域是否包含在传进来的区域中
	//根据观察，区域列表中的区域前2个字符具有唯一标识性，如“海南藏族自治州”中的“海南”,所以只取前2个字符查找
	for (int j = 0; j < nAreaCount; j++)
	{
		CString strTemp = m_strAreaArray[j];
		strTemp.Trim();
		if (strTemp.IsEmpty())
			continue;
		if (strTemp.GetLength() < 2)
			continue;

		CString strTemp2 = strTemp.Left(2); 
		ASSERT(!strTemp2.IsEmpty());
		if (strArea.Find(strTemp2) == -1)
			continue;

		bFind = TRUE;
		strArea = strTemp;
		break;
	}

	return bFind;
}

CString CYunTaskStreamCtr::GetArea()
{
	BOOL bNeedRefresh = IsNeedToUpdateArea();	//是否要重新获取区域
	if (!bNeedRefresh)
		return m_strArea;

	CString strArea;
	strArea.Empty();
	
	BOOL bRes = IPAreaQuery(strArea);
	if (bRes)
	{
		//写入配置文件中			
		CString strDate;
		CTime time = CTime::GetCurrentTime();
		strDate.Format(_T("%4d-%02d-%02d"), time.GetYear(), time.GetMonth(), time.GetDay());	//当前日期

		if (strDate.Compare(m_strAreaWriteTime) != 0)	//上一次写入配置文件的日期与当前日期不同，执行写入写入操作
		{
			if (XRET_SUCCESS == g_iniCfgFile.WriteString(_T("AreaSetting"), _T("AreaInfo"), _T("AreaName"), strArea))
			{
				if (XRET_SUCCESS == g_iniCfgFile.WriteString(_T("AreaSetting"), _T("AreaInfo"), _T("WriteTime"), strDate))
				{
					//写入成功后才改变
					m_strArea = strArea;
					m_strAreaWriteTime = strDate;
				}
			}
		}
	}

	return m_strArea;	//返回配置文件保存的区域
}

BOOL CYunTaskStreamCtr::IPAreaQuery(CString &strArea)
{
	int nAPICount = m_ipQueryAPIArray.GetCount();
	if (nAPICount <= 0)
		return FALSE;

	for (int i = 0; i < nAPICount; i++)
	{
		SIPQueryAPI ipQueryAPITemp = m_ipQueryAPIArray.GetAt(i);
		if (ipQueryAPITemp.strIPAPI.IsEmpty() || ipQueryAPITemp.strPreField.IsEmpty() || ipQueryAPITemp.strNextField.IsEmpty())
			continue;
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("请求IP区域信息URL:%s!"), ipQueryAPITemp.strIPAPI);

		CString strResponse;	//调用API回复数据
		strResponse.Empty();		

		int iRes = 0;
		iRes = m_internetHttp.HttpGet(ipQueryAPITemp.strIPAPI, NULL, strResponse, ipQueryAPITemp.iCodeType);	//接口请求数据
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("请求的数据为：%s!"), strResponse);
		if (iRes != 0)
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("请求IP区域信息数据失败!"));
			continue;
		}

		if (strResponse.IsEmpty())
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("请求IP区域信息数据为空!"));
			continue;
		}

		int nPos = -1;
		nPos = strResponse.Find(ipQueryAPITemp.strPreField);
		if (nPos < 0)
			continue;

		int nEndPos = -1;
		nEndPos = strResponse.Find(ipQueryAPITemp.strNextField, nPos);

		if (nEndPos != -1)
		{
			nPos += ipQueryAPITemp.strPreField.GetLength();
			CString strResult;	//获取city数据："\u6df1\u5733"
			strResult = strResponse.Mid(nPos, nEndPos - nPos);
			strResult.Replace(_T(":"), _T(""));
			strResult.Replace(_T("\""), _T(""));
			strResult.Trim();

			if (!ipQueryAPITemp.bUnicode)	//不是Unicode编码
			{
				if (strResult.IsEmpty())
				{
					continue;
				}
				else
				{
					strArea = strResult;
					if (IsInAreaList(strArea))	//得到的区域是否在区域列表中，如果在得到的区域与列表中的一致
					{
						g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("获取区域为：%s!"), strArea);
						return TRUE;
					}
				}
			}
			else
			{
				CString strOut;
				strOut.Empty();

				std::vector<CString> strArray;
				SplitCString(strResult, _T("\\u"), strArray);	//分割字符（例："\u6df1\u5733"，分割为6df1、5733）
				if (strArray.size() <= 0)
				{
					g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("city数据解析出错，数据为：%s!"), strResult);
					continue;
				}

				for (int i = 0; i < strArray.size(); i++)
				{
					CString strTemp = strArray[i];
					strTemp.Trim();
					if (strTemp.IsEmpty())
						continue;

					char *p = new char[strTemp.GetLength() + 1];
					for (int i = 0; i < strTemp.GetLength(); i++)
					{
						CString str = strTemp.Mid(i, 1);
						char *charSource = (char*)str.GetBuffer(0);
						p[i] = charSource[0];
					}
					p[strTemp.GetLength()] = '\0';

					WCHAR wstr[2];
					sscanf(p, "%4x", &wstr[0]);
					wstr[1] = 0;
					strOut += CString(wstr);

					if (p != NULL)
					{
						delete[] p;
						p = NULL;
					}
				}

				strArea = strOut;
				if (IsInAreaList(strArea))	//得到的区域是否在区域列表中，如果在得到的区域与列表中的一致
					return TRUE;
			}
		}
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("查找不到city信息,请求的数据为：%s!"), strResponse);
	}

	return FALSE;
}

BOOL CYunTaskStreamCtr::IsNeedToUpdateArea()
{
	if (m_strArea.IsEmpty() || m_strAreaWriteTime.IsEmpty())
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("配置ip信息为空，需重新获取!"));
		return TRUE;
	}

	CString strDate;
	CTime time = CTime::GetCurrentTime();
	strDate.Format(_T("%4d-%02d-%02d"), time.GetYear(), time.GetMonth(), time.GetDay());	//当前日期

	if (strDate.Compare(m_strAreaWriteTime) != 0)	//上一次写入配置文件的日期与当前日期不同，执行写入写入操作
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("当天ip信息没有写入，需重新获取!上次获取日期为：%s"), m_strAreaWriteTime);
		return TRUE;
	}

	g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("当天ip信息已经写入配置文件中，不需重新获取!"));
	return FALSE;
}

