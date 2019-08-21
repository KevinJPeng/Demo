#include "StdAfx.h"
#include "TaskMgr.h"
#include "TaskGeneral.h"
#include "FileReadAndSave.h"


CTaskGeneral::CTaskGeneral(CTaskThread *pTaskThread, CYunTaskStreamCtr *pYunTaskStreamCtr)
{	
	m_hProcess          = NULL;

	m_pTaskThread       = pTaskThread;

	m_pYunTaskStreamCtr = pYunTaskStreamCtr;

	m_dwExecTime = 0;

	m_bIsTaskTimeOut = FALSE;

	m_bIsTaskComplete = FALSE;

	m_bStopWebc = false;

	m_bIsPrintMCLog = false;

	m_hRunning = ::CreateEvent(NULL, TRUE, TRUE, NULL);

	m_strLocalAccount = _T("");
}

CTaskGeneral::~CTaskGeneral()
{	
	if (m_hRunning != NULL)
	{
		CloseHandle(m_hRunning);
		m_hRunning = NULL;
	}

	m_vecCacheData.clear();
}

//设置任务数据
void CTaskGeneral::SetData(const T_TASK_DATA &tData)
{
	m_tTaskData = tData;
}

//返回任务数据
T_TASK_DATA CTaskGeneral::GetData(void)
{
	return m_tTaskData;
}

//初始化对象操作
bool CTaskGeneral::Init(void)
{
	CString strInitPath = _T("");
	m_vecCacheData.clear();
	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());
	GetHttpLoginInfo(strInitPath);

	//初始化http登录信息
	g_http1.GetHttpLoginInfo(m_httpLoginInfo);

	//提交本地缓存数据到服务器
	if (g_bDelFile)
	{
		SendLocalResultToServer();
		g_bDelFile = false;
	}
	return true;
}

//通知任务结束的回调函数
void CTaskGeneral::SetTaskMgr(CTaskMgr *pMgr)
{
	m_pMgr = pMgr;
}

//检测任务是否可以立即执行
bool CTaskGeneral::CanExecNow()
{

	if (ProcessExist(MC_PROCESS_NAME))
	{	
		if (IsOwnerMCProcess())
		{	
			if (!m_bIsPrintMCLog)
			{
				g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("执行普通任务时检测到主控已经存在,等待主控退出..."));

				//设置已经打印主控已经存在日志标记，控制只打印一次
				m_bIsPrintMCLog = true;
			}
			return false;
		}
	}

	return true;
}


//执行任务
DWORD CTaskGeneral::Exec(void)
{
	CString strCmd = _T("");
	CString strPort = _T("");
	int nPort = GetPort();

	//if (GetCurrTaskType() == eType_MainTask && m_pYunTaskStreamCtr->IsObjTaskRunning())
	//{
	//	strPort.Format(_T(" %d"), nPort);
	//}
	//else
	//{
	strPort.Format(_T(" %d -y"), nPort);
	/*}*/

	strCmd.Format(_T("%s"), g_pGlobalData->dir.GetMcProcPath());
	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("开始执行任务，subject:%s cmd:%s strPort:%s"), m_tTaskData.strSubject.GetString(), strCmd.GetString(), strPort.GetString());

	if (StartProcess(strCmd.GetBuffer(), strPort.GetBuffer(), &m_hProcess))
	{
		if (-1 == m_comm.Init(nPort, this))
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("启动任务失败，subject:%s cmd:%s"), m_tTaskData.strSubject, strCmd);
			Stop();

			return 0;
		}
		else
		{
			//起了主控，正在执行普通任务
			g_YunDataLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("主控启动执行普通任务"));
		}

		m_dwExecTime = GetTickCount();
		int iSendLength = m_comm.SendData(m_tTaskData.strData.GetBuffer());
		if (iSendLength < 0)
		{
			g_YunDataLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("向主控发送普通任务数据失败! errorcode:%d"), GetLastError());
		}

		return 1;
	}
	else
	{
		g_YunDataLog.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("主控未启动 ret：%d"), GetLastError());
		return 0;
	}

	return 0;
}


//停止任务
void CTaskGeneral::Stop()
{
	m_comm.Stop();
	if (m_hProcess != NULL)
	{
		TerminateProcess(m_hProcess, 1);
		m_bStopWebc = true;
		g_YunDataLog.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("停止任务, err:%d，subject:%s "), GetLastError(), m_tTaskData.strSubject.GetString());
	}
}

//获取任务的标识信息
CString CTaskGeneral::GetTaskInfo(void)
{
	CString strInfo = _T("");
	strInfo.Format(_T("uid=%s,type=%s,subject=%s,timemode=%s,time=%s"), \
		m_tTaskData.strUID, m_tTaskData.strType, m_tTaskData.strSubject, \
		m_tTaskData.strTimeMode, m_tTaskData.strTime);

	return strInfo;
}


void CTaskGeneral::GetHttpLoginInfo(const CString &strIniPath)
{
	TCHAR szIp[100];
	TCHAR szUploadUrl[100];
	TCHAR szUserName[100];
	TCHAR szPwd[100];
	CString szInfo = _T("");
	IKeyRW key;
	key.InitDll(KTYPE_CFG);
	memset(szIp, 0, sizeof(szIp));
	memset(szUploadUrl, 0, sizeof(szUploadUrl));
	memset(szUserName, 0, sizeof(szUserName));
	memset(szPwd, 0, sizeof(szPwd));

	IXMLRW xml;
	xml.init(strIniPath);
	xml.ReadString(_T("MC"), _T("UPDATE"), _T("http"), szInfo);

	CString strDecrypt;
	key.DecryptData(szInfo, DECRYPTCFG, strDecrypt);
	/*desDecrypt.Decrypt(strDecrypt, strEncrypt);*/

	if (strDecrypt.GetLength() > 0)
	{
		GetHttpDesCode(strDecrypt.GetBuffer(0), szIp, szUploadUrl, szUserName, szPwd);
		strDecrypt.ReleaseBuffer();
	}

	m_httpLoginInfo.strServer = szIp;
	m_httpLoginInfo.strUploadUrl = szUploadUrl;
	m_httpLoginInfo.strUser = szUserName;
	m_httpLoginInfo.strPwd = szPwd;
}

void CTaskGeneral::GetHttpDesCode(TCHAR *code, TCHAR *pHome, TCHAR *pFileUpload, TCHAR *account, TCHAR *psd)
{
	int i = 0;
	TCHAR *p[4];
	TCHAR *buf = code;

	while ((p[i] = _tcstok(buf, _T("|"))) != NULL)
	{
		if (i == 0)
		{
			_sntprintf(pHome, _TRUNCATE, _T("%s"), p[i]);
		}
		if (i == 1)
		{
			_sntprintf(pFileUpload, _TRUNCATE, _T("%s"), p[i]);
		}
		if (i == 2)
		{
			_sntprintf(account, _TRUNCATE, _T("%s"), p[i]);
		}
		if (i == 3)
		{
			_sntprintf(psd, _TRUNCATE, _T("%s"), p[i]);
		}
		i++;
		if (i == 4)
			break;
		buf = NULL;
	}
}

// 缓存主控发送过来的任务执行结果
void CTaskGeneral::CacheData(CString strData)
{
	m_vecCacheData.push_back(strData);
}

// 等待接收线程结束
void CTaskGeneral::WaitExit()
{
	m_comm.WaitReceiveExit();
}

// 释放内存通告
void CTaskGeneral::ReleaseTask()
{
	m_pMgr->NotifyReleaseTask(this);
}

// 获取所有缓存数据
CString CTaskGeneral::GetAllCacheDate()
{
	CString strAllData = _T("");
	for (int i = 0; i < m_vecCacheData.size(); i++)
	{
		strAllData.Append(m_vecCacheData[i]);
	}
	return strAllData;
}

// 接受结果回调函数
void CTaskGeneral::OnReceive(CString strData)
{
	if (strData.Find(_T("BackResult(;0)AllTaskComplete")) == 0 || 0 == strData.CompareNoCase(_T("DISCONNECTED")))
	{
		if (strData.Find(_T("BackResult(;0)AllTaskComplete")) == 0)
		{	
			//任务正常结束
			m_bIsTaskComplete = TRUE;
			g_YunDataLog.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("任务执行完成,收到主控发过来的完成标记:BackResult(;0)AllTaskComplete!"));
		}
		else
		{
			if (!m_bStopWebc)
			{
				//主控异常
				m_bIsTaskComplete = FALSE;
				g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("主控异常!"));
			}

		} 
		m_pMgr->TaskFinished(this);
		
		return;
	}
	else
	{	
		//本地账号密码保存字符串
		if (strData.Find(_T("SaveAccount(;0)")) == 0)
		{	
			m_strLocalAccount = strData;
			//保存主控返回的数据到本地(账号密码信息)
			SaveResultDataToFile(strData);
			m_strLocalAccount.Replace(_T("SaveAccount(;0)"), _T(""));

			g_YunDataLog.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到需要本地保存的账号密码信息:%s"), m_strLocalAccount);
		}
		else
		{
			CacheData(strData);
			//保存主控返回的数据到本地
			SaveResultDataToFile(strData);
			g_YunDataLog.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到第%d条主控返回的结果!"), m_vecCacheData.size());
		}
	}
}

// 请求下个云任务
void CTaskGeneral::RequestNextTask()
{
	if (IsSendNitify())
	{
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("请求下个云任务"));
		// 发送通告请求下个任务 在这eType_MainTask是阀值是一轮任务最后一个任务
		if (eType_MainTask == g_TypeMap[m_tTaskData.strType])
		{
			m_pYunTaskStreamCtr->NotifyRequestTask(eType_QuickPhoto);
		}
		else
		{
			m_pYunTaskStreamCtr->NotifyRequestTask(g_TypeMap[m_tTaskData.strType] + 1);
		}
	}
}

// 返回数据到服务器
bool CTaskGeneral::SendResultToServer()
{	
	//调试数据无需提交和上传结果
	if (!m_pYunTaskStreamCtr->IsNeedSumitResult())
	{
		//本地调试数据不需要提交
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("本次任务为本地调试,不需要提交数据到服务器! 任务信息:%s"), GetTaskInfo());
		return TRUE;
	}
	CString strURL = _T("");
	CString strTEP = _T("");
	CTime timep;
	timep = GetCurrentTime();
	CString strTime = _T("");
	strTime.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), timep.GetYear(), timep.GetMonth(), timep.GetDay(), timep.GetHour(), timep.GetMinute(), timep.GetSecond());


	CString strFlag = _T("");
	CString strResultData = GetAllCacheDate();

	//如果主控没有正常返回完成，且数据为空，则要进行请求限制
	if (strResultData.IsEmpty() && !m_bIsTaskComplete)
	{
		return false;
	}
	else if (strResultData.IsEmpty() && m_bIsTaskComplete)
	{
		return true;
	}

	CString strPostData = _T("");

	if (g_TypeMap[m_tTaskData.strType] == eType_Inforefr)
	{
		//要传输的数据格式：Result=用户ID||用户产品版本号||执行结果||MAC地址||任务ID
		strPostData.Format(_T("%s||%s||%s||%s||%s"), m_tTaskData.strUID, m_tTaskData.strUIPdtVer, \
			strResultData, GetPhysicalAddress(), m_tTaskData.strExtraData);
		strFlag = _T("<@@@>");
	}
	else if (g_TypeMap[m_tTaskData.strType] == eType_ShopTraffic)
	{
		g_log.Trace(LOGL_LOW, LOGT_WARNING, __TFILE__, __LINE__, _T("商铺流量任务结束不需提交任务数据!"));
		return true;
	}
	else if (g_TypeMap[m_tTaskData.strType] == eType_MainTask)
	{
		//要传输的数据格式：Result=执行结果(|)MAC地址(|)完成标记(|)任务ID
		strPostData.Format(_T("%s(|)%s(|)%d(|)%s"), strResultData, GetPhysicalAddress(), \
			m_bIsTaskComplete, m_tTaskData.strExtraData);
		strFlag = _T("<@@>");
	}

	CFileReadAndSave encryptCode;
	CStdString strPostDataZip = _T("");
	CString strPostUrlEncodeData = _T("");
	encryptCode.ZipEncodeStdString(strPostData.GetBuffer(), strPostDataZip);

	strPostUrlEncodeData.Format(_T("%s&%s&%s&%s"), strFlag, strTime, strPostDataZip.c_str(),strFlag);
	m_pYunTaskStreamCtr->EncryptRequData(strPostUrlEncodeData);
	/*strPostUrlEncodeData = m_pYunTaskStreamCtr->URLEncode(strPostDataZip.GetBuffer());*/
	strPostUrlEncodeData = _T("result=") + strPostUrlEncodeData;
	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("要传输的urlEncode数据为：%s"), strPostUrlEncodeData.GetString());

	int nRetry = 0;
Retry:
	CString strResponseText = _T("");
	int iRet = g_http1.HttpPost(m_tTaskData.strPostAddr, strPostUrlEncodeData, strResponseText);
	if (0 != iRet || 0 != strResponseText.CompareNoCase(_T("ok")))
	{
		++nRetry;
		//提交失败重试一次
		if (nRetry < 2)
		{
			g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交任务结果失败，重试%d次！"), nRetry);
			goto Retry;
		}
	}

	if (nRetry < 2)
	{
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交任务结果成功，任务信息：%s"), GetTaskInfo());
		DelResultDataFile();
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("提交任务结果失败！ret:%d, lasterr:%d, errmsg:%s"), iRet, GetLastError(), strResponseText);
	}

	//此处还需要对云任务执行的注册成功的账号及密码信息进行保存
	if (m_strLocalAccount.IsEmpty())
	{
		return true;
	}

	//对本地账号密码信息进行保存
	encryptCode.ZipEncodeStdString(m_strLocalAccount.GetBuffer(), strPostDataZip);
	strPostUrlEncodeData = m_pYunTaskStreamCtr->URLEncode(strPostDataZip.GetBuffer());
	strPostUrlEncodeData = _T("result=") + strPostUrlEncodeData;

	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());
	g_iniCfgFile.init(strCfgFile);

	//提交本地账号密码信息
	CString strApiAdress = _T("");
	g_iniCfgFile.ReadString(_T("MC"), _T("DASHIZHENCI"), _T("apiAddress"), strApiAdress, _T(""));
	
	if (strApiAdress.IsEmpty())
	{
		strApiAdress = _T("http://zdsapi.sumszw.com");
	}
	strApiAdress += _T("/api/Member/SaveWebsiteAccount");

	iRet = g_http2.HttpPost(strApiAdress, strPostUrlEncodeData, strResponseText);

	if (0 != iRet || 0 != strResponseText.CompareNoCase(_T("ok")))
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("提交本地账号密码失败!"));
	}
	else
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("提交本地账号密码成功!"));
		DelResultDataFile();
	}

	//记录提交结果信息到本地
	g_logYunSuccAccount.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("注册成功信息：%s，提交上层返回码:%s"),
		m_strLocalAccount.GetBuffer(), strResponseText.GetBuffer());

	return true;
}

// 判断是否需要发送通告请求任务
bool CTaskGeneral::IsSendNitify()
{
	if (m_tTaskData.bGeneralTask)
	{
		bool bIn = m_pMgr->IsInTaskList(g_TypeMap[m_tTaskData.strType]);
		if (bIn)
			return false;
		else
		    return true;
	}
	else
	{
		return false;
	}
}

// 判断任务超时
bool CTaskGeneral::IsTimeOut()
{
	DWORD dwCheckTime = GetTickCount();
	if ((dwCheckTime - m_dwExecTime) > m_pYunTaskStreamCtr->GetOutTime(1) * 60 * 1000)
	{	
		if (IsRunningSendResult())
		{	
			m_bIsTaskTimeOut = TRUE;
			return true;
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("程序正在进行提交结果操作，超时忽略!"));
		}
		
	}
	return false;
}

void CTaskGeneral::WaitForRunningEvent()
{
	::WaitForSingleObject(m_hRunning, INFINITE);
	::ResetEvent(m_hRunning);
}

void CTaskGeneral::SetEvent()
{
	::SetEvent(m_hRunning);
}

BOOL CTaskGeneral::IsRunningSendResult()
{
	if (::WaitForSingleObject(m_hRunning, 0) == WAIT_OBJECT_0)
	{	
		return TRUE;
	}

	return FALSE;
}

bool CTaskGeneral::GetTimeOutFlag()
{
	return m_bIsTaskTimeOut;
}

int CTaskGeneral::GetCurrTaskType()
{
	return(g_TypeMap[m_tTaskData.strType]);
}

BOOL CTaskGeneral::ReadStringToUnicode(CString &str)
{
	char *szBuf = new char[str.GetLength() + 1]; //数量要加1
	for (int i = 0; i < str.GetLength(); i++)
	{
		szBuf[i] = (CHAR)str.GetAt(i);
	}
	szBuf[str.GetLength()] = '\0';   //这里，必须要加上，否则会在结尾片显示一个"铪"字。
	// USES_CONVERSION;
	//char * sz=W2A(str.GetBuffer());  //这些方法我都试过，不行的。
	BOOL bok = CharToUnicode(szBuf, &str);
	delete[]szBuf;
	return bok;
}
int CTaskGeneral::CharToUnicode(char *pchIn, CString *pstrOut)
{
	int nLen;
	WCHAR *ptch;

	if (pchIn == NULL)
	{
		return 0;
	}
	nLen = MultiByteToWideChar(CP_ACP, 0, pchIn, -1, NULL, 0);//取得所需缓存的多少
	ptch = new WCHAR[nLen];//申请缓存空间
	MultiByteToWideChar(CP_ACP, 0, pchIn, -1, ptch, nLen);//转码
	pstrOut->Format(_T("%s"), ptch);
	delete[] ptch;

	return nLen;
}

bool CTaskGeneral::WriteString(CStdioFile& file, CString _str)
{
	file.SeekToEnd();
	_str += _T("\n");
	file.WriteString(_str);
	return true;
}

bool CTaskGeneral::SaveResultDataToFile(CString _strData)
{
	CLocalLock localLock(&m_lock); //锁

	if (_T("MainTask") == m_tTaskData.strType)
	{
		//设置地域信息
		char* old_locale = _strdup(setlocale(LC_CTYPE, NULL));
		setlocale(LC_CTYPE, "chs");

		CString sFileName;
		sFileName.Format(_T("%s\\log\\resultData.dat"), g_pGlobalData->dir.GetInstallDir());

		CStdioFile file;
		DWORD dwAttrib = GetFileAttributes(sFileName);
		//判断文件是否已存在
		bool bFileIsExist = INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
		if (!bFileIsExist)
		{
			if (!file.Open(sFileName, CFile::modeCreate | CFile::modeWrite, NULL))
			{
				return false;
			}
			WriteString(file, m_tTaskData.strType);
			WriteString(file, m_tTaskData.strUID);
			WriteString(file, m_tTaskData.strUIPdtVer);
			WriteString(file, m_tTaskData.strExtraData);
			WriteString(file, m_tTaskData.strPostAddr);
		}
		else
		{
			if (!file.Open(sFileName, CFile::modeNoTruncate | CFile::modeWrite, NULL))
			{
				return false;
			}
		}

		if (!_strData.IsEmpty())
		{
			WriteString(file, _strData);
		}
		file.Close();

		//还原地域信息
		setlocale(LC_CTYPE, old_locale);
	}
	return true;
}

bool CTaskGeneral::SendLocalResultToServer()
{
	CTime timep;
	timep = GetCurrentTime();
	CString strTime = _T("");
	strTime.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), timep.GetYear(), timep.GetMonth(), timep.GetDay(), timep.GetHour(), timep.GetMinute(), timep.GetSecond());

	CString strFlag = _T("");
	CString strResultData;
	T_TASK_DATA tTaskData;
	CString strLocalAccount;
	if (!GetResultDataFromFile(strResultData, tTaskData, strLocalAccount))
	{
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("本地缓存数据不存在或数据无效"));
		return false;
	}
	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到存在本地缓存数据文件，提交数据到服务器"));

	CString strPostData = _T("");
	if (g_TypeMap[tTaskData.strType] == eType_Inforefr)
	{
		//要传输的数据格式：Result=用户ID||用户产品版本号||执行结果||MAC地址||任务ID
		strPostData.Format(_T("%s||%s||%s||%s||%s"), tTaskData.strUID, tTaskData.strUIPdtVer, \
			strResultData, GetPhysicalAddress(), tTaskData.strExtraData);
		strFlag = _T("<@@@>");
	}
	else if (g_TypeMap[tTaskData.strType] == eType_ShopTraffic)
	{
		g_log.Trace(LOGL_LOW, LOGT_WARNING, __TFILE__, __LINE__, _T("商铺流量任务结束不需提交任务数据!"));
		return true;
	}
	else if (g_TypeMap[tTaskData.strType] == eType_MainTask)
	{
		//要传输的数据格式：Result=执行结果(|)MAC地址(|)完成标记(|)任务ID
		strPostData.Format(_T("%s(|)%s(|)%d(|)%s"), strResultData, GetPhysicalAddress(), \
			m_bIsTaskComplete, tTaskData.strExtraData);
		strFlag = _T("<@@>");
	}

	CFileReadAndSave encryptCode;
	CStdString strPostDataZip = _T("");
	CString strPostUrlEncodeData = _T("");
	encryptCode.ZipEncodeStdString(strPostData.GetBuffer(), strPostDataZip);

	strPostUrlEncodeData.Format(_T("%s&%s&%s&%s"), strFlag, strTime, strPostDataZip.c_str(), strFlag);
	m_pYunTaskStreamCtr->EncryptRequData(strPostUrlEncodeData);
	/*strPostUrlEncodeData = m_pYunTaskStreamCtr->URLEncode(strPostDataZip.GetBuffer());*/
	strPostUrlEncodeData = _T("result=") + strPostUrlEncodeData;
	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("要传输的urlEncode数据为：%s"), strPostUrlEncodeData.GetString());

	bool bPostSuccess = false;
	int iRetry = 0;
	CString strResponseText = _T("");
	for (iRetry = 0; iRetry < 2; iRetry++)
	{
		strResponseText = _T("");
		int iRet = g_http1.HttpPost(tTaskData.strPostAddr, strPostUrlEncodeData, strResponseText);
		if (0 != iRet || 0 != strResponseText.CompareNoCase(_T("ok")))
		{
			//提交失败重试一次
			g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交任务结果失败，重试%d次！"), iRetry);
		}
		else
		{
			bPostSuccess = true;
			break;
		}
	}
	if (bPostSuccess)
	{
		CString strInfo = _T("");
		strInfo.Format(_T("uid=%s,type=%s"), tTaskData.strUID, tTaskData.strType);
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交任务结果成功，任务信息：%s"), strInfo);
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("提交任务结果失败！ret:%d, lasterr:%d, errmsg:%s"), iRetry, GetLastError(), strResponseText);
	}

	//此处还需要对云任务执行的注册成功的账号及密码信息进行保存
	if (strLocalAccount.IsEmpty())
	{
		return true;
	}

	//对本地账号密码信息进行保存
	encryptCode.ZipEncodeStdString(strLocalAccount.GetBuffer(), strPostDataZip);
	strPostUrlEncodeData = m_pYunTaskStreamCtr->URLEncode(strPostDataZip.GetBuffer());
	strPostUrlEncodeData = _T("result=") + strPostUrlEncodeData;

	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());
	g_iniCfgFile.init(strCfgFile);

	//提交本地账号密码信息
	CString strApiAdress = _T("");
	g_iniCfgFile.ReadString(_T("MC"), _T("DASHIZHENCI"), _T("apiAddress"), strApiAdress, _T(""));

	if (strApiAdress.IsEmpty())
	{
		strApiAdress = _T("http://zdsapi.sumszw.com");
	}
	strApiAdress += _T("/api/Member/SaveWebsiteAccount");

	int iRet = g_http2.HttpPost(strApiAdress, strPostUrlEncodeData, strResponseText);

	if (0 != iRet || 0 != strResponseText.CompareNoCase(_T("ok")))
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("提交本地账号密码失败!"));
	}
	else
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("提交本地账号密码成功!"));
	}

	//记录提交结果信息到本地
	g_logYunSuccAccount.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("注册成功信息：%s，提交上层返回码:%s"),
		strLocalAccount.GetBuffer(), strResponseText.GetBuffer());

	return true;
}

bool CTaskGeneral::GetResultDataFromFile(CString& _strData, T_TASK_DATA& _tTaskData, CString& _strLocalAccount)
{
	CLocalLock localLock(&m_lock); //锁

	CString sFileName;
	sFileName.Format(_T("%s\\log\\resultData.dat"), g_pGlobalData->dir.GetInstallDir());
	CStdioFile file;
	DWORD dwAttrib = GetFileAttributes(sFileName);
	bool bFileIsExist = INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
	if (bFileIsExist)
	{
		CStdioFile file(sFileName, CFile::modeRead);
		CString contents;
		int i = 0;
		while (file.ReadString(contents))
		{
			ReadStringToUnicode(contents);
			if (i < 5)
			{
				if (0 == i)
				{
					_tTaskData.strType = contents;
				}
				else if (1 == i)
				{
					_tTaskData.strUID = contents;
				}
				else if (2 == i)
				{
					_tTaskData.strUIPdtVer = contents;
				}
				else if (3 == i)
				{
					_tTaskData.strExtraData = contents;
				}
				else if (4 == i)
				{
					_tTaskData.strPostAddr = contents;
				}
				i++;
			}
			else
			{
				if (contents.Find(_T("SaveAccount(;0)")) == 0)
				{
					_strLocalAccount = contents;
					_strLocalAccount.Replace(_T("SaveAccount(;0)"), _T(""));
				}
				else
				{
					if (!contents.IsEmpty())
					{
						_strData += contents;
					}
				}
			}
		}
		file.Close();

		::DeleteFile(sFileName);
		//初步判断文件数据是否有效
		if (i >= 5)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		_strData = _T("");
		return false;
	}
}
bool CTaskGeneral::DelResultDataFile()
{
	CString sFileName;
	sFileName.Format(_T("%s\\log\\resultData.dat"), g_pGlobalData->dir.GetInstallDir());
	::DeleteFile(sFileName);
	return true;
}

