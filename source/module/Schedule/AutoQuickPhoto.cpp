#include "StdAfx.h"
#include "ScheduleMgr.h"
#include "AutoQuickPhoto.h"
#include "FileReadAndSave.h"


extern bool g_bWebcDisconnected;
extern bool g_bCaptureKeywordFailed;
extern HANDLE g_hEvent;

CInternetHttp g_http;

HANDLE hProcess = NULL;     //执行任务的进程句柄
CAutoQuickPhoto::CAutoQuickPhoto(CScheduleThread *pScheduleThread, CAutoGetKeywordPages *pAutoGetKeywordPages)
{
	//m_hProcess = NULL;
	m_pScheduleThread = pScheduleThread;
	m_pAutoGetKeywordPages = pAutoGetKeywordPages;

	m_nFirstPageOfBaiduSum = 0;
	m_nFirstPageSum = 0;
	m_nFirstThreePagesSum = 0;
	m_nKeywordSum = 0;
	m_nJudgeWebcDisconnected = 1;
}


CAutoQuickPhoto::~CAutoQuickPhoto(void)
{
	
}

//设置任务数据
void CAutoQuickPhoto::SetData(const T_TASK_DATA &tData)
{
	m_tTaskData = tData;
}

//返回任务数据
T_TASK_DATA CAutoQuickPhoto::GetData(void)
{
	return m_tTaskData;
}


//初始化对象操作
bool CAutoQuickPhoto::Init(void)
{
	CString strInitPath = _T("");
	m_strCacheDataInSuccess.Empty();
	m_strCacheDataInFail.Empty();
	m_bStopWebc = false;
	m_vPageName.clear();
	m_mapPageName.clear();
	m_vCacheData.clear();
	m_vCacheUploadFailData.clear();

	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());
	GetFtpLoginInfo(strInitPath);
	GetHttpLoginInfo(strInitPath);

	//初始化http登录信息
	g_http.GetHttpLoginInfo(m_httpLoginInfo);

	return true;
}

void CAutoQuickPhoto::GetFtpLoginInfo(const CString &strIniPath)
{
	TCHAR szIp[100];
	TCHAR szUserName[100];
	TCHAR szPwd[100];
	TCHAR szInfo[MAX_PATH] = {0};
	int   iPort;
	CDES  desDecrypt;

	memset(szIp, 0, sizeof(szIp));
	memset(szUserName, 0, sizeof(szUserName));
	memset(szPwd, 0, sizeof(szPwd));

	GetPrivateProfileString(_T("UPDATE"), _T("html"), NULL, szInfo, MAX_PATH, strIniPath);

	//httpupload为1，用http上传；httpupload为0，用ftp上传
	m_isFtpUpload = GetPrivateProfileInt(_T("SERVICE"), _T("HttpUpload"), TRUE, strIniPath) == TRUE ? FALSE : TRUE;
	m_bUseFtpZipUpload = GetPrivateProfileInt(_T("SERVICE"), _T("UseFtpZipUpload"), TRUE, strIniPath) == TRUE ? TRUE : FALSE;
	m_bUploadZipFromBreak = GetPrivateProfileInt(_T("SERVICE"), _T("UploadZipFromBreak"), TRUE, strIniPath) == TRUE ? TRUE : FALSE;

	
	CString strEncrypt = szInfo;
	CString strDecrypt;
	desDecrypt.Decrypt(strDecrypt, strEncrypt);

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

void CAutoQuickPhoto::GetHttpLoginInfo(const CString &strIniPath)
{
	TCHAR szIp[100];
	TCHAR szUploadUrl[100];
	TCHAR szUserName[100];
	TCHAR szPwd[100];
	TCHAR szInfo[MAX_PATH] = {0};
	CDES  desDecrypt;

	memset(szIp, 0, sizeof(szIp));
	memset(szUploadUrl, 0, sizeof(szUploadUrl));
	memset(szUserName, 0, sizeof(szUserName));
	memset(szPwd, 0, sizeof(szPwd));

	GetPrivateProfileString(_T("UPDATE"), _T("http"), NULL, szInfo, MAX_PATH, strIniPath);

	CString strEncrypt = szInfo;
	CString strDecrypt;
	desDecrypt.Decrypt(strDecrypt, strEncrypt);

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

void CAutoQuickPhoto::GetFtpDesCode(TCHAR *code, TCHAR *ip, TCHAR *account, TCHAR *psd, int &port)
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


void CAutoQuickPhoto::GetHttpDesCode(TCHAR *code, TCHAR *pHome, TCHAR *pFileUpload, TCHAR *account, TCHAR *psd)
{
	int i = 0;
	TCHAR *p[4];
	TCHAR *buf = code;

	while((p[i]=_tcstok(buf, _T("|"))) != NULL) 
	{
		if(i == 0)
		{
			_sntprintf(pHome, _TRUNCATE, _T("%s"),p[i]);
		}
		if(i == 1)
		{
			_sntprintf(pFileUpload, _TRUNCATE, _T("%s"), p[i]);
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

//检测任务是否可以立即执行
bool CAutoQuickPhoto::CanExecNow(DWORD dwEnterTickCount)
{
	//检查执行时间
	if (m_tTaskData.strTimeMode == _T("0"))
	{//延迟执行
		int nDelayedTime = (GetTickCount() - dwEnterTickCount) / 60000;    //已延时时间，以分为单位

		int nRequireDelayTime = _ttoi(m_tTaskData.strTime.GetBuffer());
		if (nDelayedTime < nRequireDelayTime)
			return false;
	}
	else
	{//定时执行
		CTime time;
		time = CTime::GetCurrentTime();

		CString strCurTime = _T("");
		strCurTime.Format(_T("%2d:%02d:%02d"), time.GetHour(), time.GetMinute(), time.GetSecond());
		if (strCurTime < m_tTaskData.strTime)
			return false;
	}

	//要求独立执行的，检查当前主控是否在运行
	if (m_tTaskData.strExclusive == _T("1"))
	{
		//否则检查主控是否正在运行
		if (ProcessExist(MC_PROCESS_NAME))
		{
			return false;
		}
	}

	return true;
}

//执行任务
DWORD CAutoQuickPhoto::Exec(void)
{
	CString strCmd = _T("");
	CString strPort = _T("");
	int nPort = GetPort();
	strPort.Format(_T(" %d"), nPort); 

	strCmd.Format(_T("%s"), g_pGlobalData->dir.GetMcProcPath());
	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("开始执行任务，subject:%s cmd:%s strPort:%s"), m_tTaskData.strSubject.GetString(), strCmd.GetString(), strPort.GetString());

	if (StartProcess(strCmd.GetBuffer(), strPort.GetBuffer(), &hProcess))
	{
		if (-1 == m_comm.Init(nPort, this))
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("启动任务失败，subject:%s cmd:%s"), m_tTaskData.strSubject, strCmd);
			m_pScheduleThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
			Stop();

			return 0;
		}
		else
		{
			//起了主控，正在刷关键词排名
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("主控启动刷关键词，发给送给ui！"));
			m_pScheduleThread->OnNotifyUI(WAPAM_AUTO_REFRESH_KEYWORD_RUNNING);
		}


		m_comm.SendData(m_tTaskData.strData.GetBuffer());
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("发给主控的信息 data:%s"), m_tTaskData.strData);
		return 1;
	}
	else
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("主控未启动 ret：%d"), GetLastError());
		m_pScheduleThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
		return 0;
	}

	return 0;
}

//停止任务
void CAutoQuickPhoto::Stop(void)
{
	if (hProcess != NULL)
	{
		TerminateProcess(hProcess, 1);
		//WaitForSingleObject(hProcess, INFINITE);
		m_bStopWebc = true;
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("停止任务, err:%d，subject:%s "), GetLastError(), m_tTaskData.strSubject.GetString());
	}
	//Sleep(200);  //因为TerminateProcess是异步的，防止comm中m_pScheduleBase->OnReceive(strData)时m_pScheduleBase被同时置为null
	m_comm.Stop();
}

//向服务器提交执行结果
bool CAutoQuickPhoto::SendResultToServer(void)
{
	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("SendResultToServer：type:%s subject:%s"), m_tTaskData.strType.GetString(), m_tTaskData.strSubject.GetString());

	int nKeywordSum = 0;
	int nFirstPageOfBaiduSum = 0;
	int nFirstPageSum = 0; 
	int nFirstThreePagesSum = 0;
	//判断是否需要上传快照
	if (!WhehterSendToServerIsRequire())
	{
		return true;
	}

	//上传快照
	if (!UploadFileToServer())
	{
		return false;
	}

	CString strCacheData = _T("");
	if (m_nJudgeWebcDisconnected == 1)
	{
		strCacheData = FilterData(_T("FAIL"));
	}
	else
	{
		strCacheData = FilterData(_T("SUCCESS"));
	}

	//要传输的数据格式：Result=用户ID||用户产品版本号||执行结果||MAC地址||统计信息||附加数据
	CString strStaticInfo = _T("");
	CString strPostData = _T("");

	strStaticInfo = GetStatisticalInfo(nKeywordSum, nFirstPageOfBaiduSum, nFirstPageSum, nFirstThreePagesSum);
	
	strPostData.Format(_T("%s||%s||%s||%s||%s||%s"), m_tTaskData.strUID, m_tTaskData.strUIPdtVer, \
		strCacheData, GetPhysicalAddress(), strStaticInfo, m_tTaskData.strExtraData);

	//g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("要传输的数据为：result=%s"), strPostData.GetString());

	//压缩文本
	CFileReadAndSave encryptCode;
	CStdString strPostDataZip = _T("");
	CString strPostUrlEncodeData = _T("");
	encryptCode.ZipEncodeStdString(strPostData.GetBuffer(), strPostDataZip);
	CString strSendDataZip = strPostDataZip;
	//g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("要传输的压缩数据为：result=%s"), strSendDataZip.GetString());

	strPostUrlEncodeData = m_pAutoGetKeywordPages->URLEncode(strPostDataZip.GetBuffer());
	strPostUrlEncodeData = _T("result=") + strPostUrlEncodeData;
	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("要传输的urlEncode数据为：%s"), strPostUrlEncodeData.GetString());

	int nRetry = 0;
Retry:
	CString strResponseText = _T("");
	int iRet = g_http.HttpPost(m_tTaskData.strPostAddr, strPostUrlEncodeData, strResponseText);
	if (0 != iRet || 0 != strResponseText.CompareNoCase(_T("ok")))
	{
		++nRetry;
		//提交超时重试两次
		if (2 == iRet && nRetry != 3)
		{
			g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("提交任务结果超时，重试%d次！"), nRetry);
			goto Retry;
		}

		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("提交任务结果失败！ret:%d, lasterr:%d, errmsg:%s"), iRet, GetLastError(), strResponseText);
		return false;
	}

	//清空缓存结果
	//WritePrivateProfileString(_T("Schedule"), m_tTaskData.strSubject + m_tTaskData.strUID, _T(""),  m_tszDataCacheFile);
	/*if (m_nJudgeWebcDisconnected == 1)
	{
	WritePrivateProfileInt(nKeywordSum, _T("SUMKEYWORD"));
	WritePrivateProfileInt(nFirstPageOfBaiduSum, _T("SUMFIRSTPAGEOFBAIDU"));
	WritePrivateProfileInt(nFirstPageSum, _T("SUMFIRSTPAGE"));
	WritePrivateProfileInt(nFirstThreePagesSum, _T("SUMFIRSTTHREEPAGES"));
	}
	else
	{
	WritePrivateProfileString(_T("SUMKEYWORD"), m_tTaskData.strSubject + m_tTaskData.strUID, _T(""),  m_tszDataCacheFile);
	WritePrivateProfileString(_T("SUMFIRSTPAGEOFBAIDU"), m_tTaskData.strSubject + m_tTaskData.strUID, _T(""),  m_tszDataCacheFile);
	WritePrivateProfileString(_T("SUMFIRSTPAGE"), m_tTaskData.strSubject + m_tTaskData.strUID, _T(""),  m_tszDataCacheFile);
	WritePrivateProfileString(_T("SUMFIRSTTHREEPAGES"), m_tTaskData.strSubject + m_tTaskData.strUID, _T(""),  m_tszDataCacheFile);
	}*/
	m_strCacheDataInSuccess = _T("");
	m_strCacheDataInFail = _T("");

	if (m_nJudgeWebcDisconnected == 1)
	{
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("提交任务结果成功,还有部分关键词未刷！：%s"), GetTaskInfo());
	}
	else
	{
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("提交任务结果成功，所有关键词已刷完！：%s"), GetTaskInfo());
	}
	
	return true;

}


bool CAutoQuickPhoto::WhehterSendToServerIsRequire(void)
{
	CString strRequestUrl = _T("");
	CString strSever = _T("");

	
// 	if (m_tTaskData.strUID != m_tTaskData.strCUID)
// 	{
	CString strResponseText = _T("");
	CString strPostData = _T("");

	strSever = m_pAutoGetKeywordPages->m_strFastServer;
	//判断任务用户id和当前客户端用户id是否相等
	if (m_tTaskData.strUID != m_tTaskData.strCUID)
	{
		strRequestUrl.Format(_T("%s/Timing/IsSubmitData?uName=%s&versionId=%s&isEqualUser=false"),strSever, m_tTaskData.strUID, m_tTaskData.strUIPdtVer);
	}
	else
	{
		strRequestUrl.Format(_T("%s/Timing/IsSubmitData?uName=%s&versionId=%s&isEqualUser=true"),strSever, m_tTaskData.strUID, m_tTaskData.strUIPdtVer);
	}

	int nRetry = 0;

Retry:
	int iRet = g_http.HttpPost(strRequestUrl, strPostData, strResponseText);
	if (0 == iRet)
	{
		if (0 == strResponseText.CompareNoCase(_T("0")))
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("服务器返回需要提交刷关键词排名任务结果！"));
			return true;
		}
		else
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("服务器返回不需要提交刷关键词排名任务结果！"));
			return false;
		}
	}
	else
	{
		//查询失败重试两次
		++nRetry;
		if (nRetry != 3)
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("查询是否需要提交结果失败！重试%d次！"), nRetry);
			goto Retry;
		}
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("查询是否需要提交结果失败！ret:%d, lasterr:%d, errmsg:%s"), iRet, GetLastError(), strResponseText);
		return false;
	}
	//}

	//return true;
}

bool CAutoQuickPhoto::UploadFileToServer(void)
{
	//抓到有排名才提交快照
	if (!m_vPageName.empty())
	{
		bool bIsCompressSucc = false;
		CString strUploadZipPath = _T("");
		CString strUploadFileDir = _T("");
		CString strUploadFilePath = _T("");
		GetUploadFilePath(strUploadZipPath, _T("Zip"));
		GetUploadFilePath(strUploadFileDir, _T("File"));

		//压缩快照文件
		vector<LPCTSTR> vPageName;
		vector<CString>::iterator iter = m_vPageName.begin();
		for (; iter != m_vPageName.end(); ++iter)
		{
			vPageName.push_back(*iter);
		}

		DWORD dwCompressErr = 0;
		dwCompressErr = ZipUtils::CompressFilesToZip(strUploadFileDir.GetBuffer(), strUploadZipPath.GetBuffer(), vPageName);

		if (dwCompressErr != 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("压缩文件失败切换为单个文件上传方式， CompressErr：%d, err: %d"), dwCompressErr, GetLastError());

			goto NotUploadZip;
		}

		//压缩zip包成功标记
		bIsCompressSucc = true;

		//判断用不用ftp上传zip包
		if (!m_bUseFtpZipUpload)
		{
			goto NotUploadZip;
		}
		//上传快照文件压缩包Zip
		DWORD dwTime = GetTickCount();
		if (!SaveZipToServer(strUploadZipPath))
		{
NotUploadZip:
			if (bIsCompressSucc)
			{
				DeleteFile(strUploadZipPath);
			}

			//上传快照文件采用非压缩的方式单个上传
			for (int i = 0; i != m_vPageName.size(); ++i)
			{
				strUploadFilePath = strUploadFileDir + _T("\\") + m_vPageName[i];
				if (!SaveHtmlToServer(strUploadFilePath))
				{
					g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("提交单个快照时该快照失败，快照路径：%s"), strUploadFilePath);
					CString strCache = m_mapPageName[m_vPageName[i]];
					m_vCacheUploadFailData.push_back(strCache);
				}
			}

			//当提交的所有有排名的快照文件都失败时，不提交快照数据
			if (m_vPageName.size() == m_vCacheUploadFailData.size())
			{
				g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("提交每个快照文件都失败，不提交快照数据！"));
				return false;
			}

			//单个提交不执行ftp服务器解压
			return true;
		}
		dwTime = GetTickCount()-dwTime;
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("本次上传快照时间,%.2f,秒"), dwTime/1000.0);

		DWORD dwRetryDezip = 0;
		CString strPostData = _T("");
		CString strResponseTxt = _T("");
		CString strPostAddr = _T("");
		CString strFileName = _T("");
		strFileName.Format(_T("%s_keyword_%s\.zip"), m_tTaskData.strCUID, GetPhysicalAddress());
		strPostAddr.Format(_T("%s/File/ExtractZipFile?filename=%s"), m_httpLoginInfo.strServer, strFileName);
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("请求ftp服务器解压缩zip包地址：%s"), strPostAddr);

		
RetryDezip:
		int iRet = g_http.HttpPost(strPostAddr, strPostData, strResponseTxt);
		if (0 != iRet || 0 != strResponseTxt.CompareNoCase(_T("1")))
		{
			//解压重试一次
			if (dwRetryDezip < 1)
			{
				++dwRetryDezip;
				Sleep(2000);
				g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("请求ftp服务器解压缩zip包失败，重试一次！：ret:%d, lasterr:%d, errmsg:%s"), iRet, GetLastError(), strResponseTxt);
				goto RetryDezip;
			}
			
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("服务端解压缩快照文件失败！单个上传！ret:%d, lasterr:%d, errmsg:%s"), iRet, GetLastError(), strResponseTxt);
			goto NotUploadZip;
		}

		if (bIsCompressSucc)
		{
			DeleteFile(strUploadZipPath);
		}
	}

	return true;
}

//写整型数据到配置文件进去
//void CAutoQuickPhoto::WritePrivateProfileInt(int nInfo, CString strSection)
//{
//	CString strTemp = _T("");
//	strTemp.Format(_T("%d"), nInfo);
//	WritePrivateProfileString(strSection, m_tTaskData.strSubject + m_tTaskData.strUID, strTemp,  m_tszDataCacheFile);
//}

//获取统计信息
CString CAutoQuickPhoto::GetStatisticalInfo(int &nKeywordAcount, int &nFirstPageOfBaiduAcount, int &nFirstPageAcount, int &nFirstThreePagesAcount)
{
	CString strStatisticalInfo = _T("");
	CString strExtraData = _T("");
	
	int nKeywordSumTemp = 0;
	int nFirstPageOfBaiduSumTemp = 0;
	int nFirstPageSumTemp = 0; 
	int nFirstThreePagesSumTemp = 0;
	CString strKeywordSumTemp = _T("");
	CString strFirstPageOfBaiduSumTemp = _T("");
	CString strFirstPageSumTemp = _T(""); 
	CString strFirstThreePagesSumTemp = _T("");

	strExtraData = m_tTaskData.strExtraData;
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("上层传过来的统计信息：首页排名关键词总量(;0)百度首页排名关键词总量(;0)前三页排名关键词总量(;0)完成抓取的词总量，data: %s"), strExtraData);

	CStdStrUtils utl;
	std::vector<CStdString> vRes;
	vRes.clear();
	utl.SplitString(CStdString(strExtraData.GetBuffer()), _T("(;0)"), vRes);

	vector <CStdString>::size_type res = vRes.size();
	if (res > 0)
	{
		strFirstPageSumTemp = vRes[0];
		strFirstPageOfBaiduSumTemp = vRes[1];
		strFirstThreePagesSumTemp = vRes[2];
		strKeywordSumTemp = vRes[3];
		nFirstPageSumTemp = _ttoi(strFirstPageSumTemp);
		nFirstPageOfBaiduSumTemp = _ttoi(strFirstPageOfBaiduSumTemp);
		nFirstThreePagesSumTemp = _ttoi(strFirstThreePagesSumTemp);
		nKeywordSumTemp = _ttoi(strKeywordSumTemp);

	}
	else
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("数据解析格式不正确，data: %s"), strExtraData);
		return _T("");
	}
	/*nKeywordSumTemp = GetPrivateProfileInt(_T("SUMKEYWORD"), m_tTaskData.strSubject + m_tTaskData.strUID, 0, m_tszDataCacheFile);
	nFirstPageOfBaiduSumTemp = GetPrivateProfileInt(_T("SUMFIRSTPAGEOFBAIDU"), m_tTaskData.strSubject + m_tTaskData.strUID, 0, m_tszDataCacheFile); 
	nFirstPageSumTemp = GetPrivateProfileInt(_T("SUMFIRSTPAGE"), m_tTaskData.strSubject + m_tTaskData.strUID, 0, m_tszDataCacheFile);
	nFirstThreePagesSumTemp = GetPrivateProfileInt(_T("SUMFIRSTTHREEPAGES"), m_tTaskData.strSubject + m_tTaskData.strUID, 0, m_tszDataCacheFile);*/

	nKeywordAcount = m_nKeywordSum + nKeywordSumTemp;
	nFirstPageOfBaiduAcount = m_nFirstPageOfBaiduSum + nFirstPageOfBaiduSumTemp;
	nFirstPageAcount = m_nFirstPageSum + nFirstPageSumTemp;
	nFirstThreePagesAcount = m_nFirstThreePagesSum + nFirstThreePagesSumTemp;
	
	//统计信息格式为：客户端用户名(;0)客户端版本号(;0)抓取结果状态(;0)完成抓取的词总量(;0)百度首页排名总量 (;0)首页排名总量 (;0)前三页排名总量 
	strStatisticalInfo.Format(_T("%s(;0)%s(;0)%d(;0)%d(;0)%d(;0)%d(;0)%d"), \
		m_tTaskData.strCUID.GetString(), m_tTaskData.strPdtVer.GetString(), m_nJudgeWebcDisconnected, \
		nKeywordAcount, nFirstPageOfBaiduAcount, nFirstPageAcount, nFirstThreePagesAcount);

	return strStatisticalInfo;
}


//获取任务的标识信息
CString CAutoQuickPhoto::GetTaskInfo(void)
{
	CString strInfo = _T("");
	strInfo.Format(_T("uid=%s,type=%s,subject=%s,timemode=%s,time=%s"), \
		m_tTaskData.strUID, m_tTaskData.strType, m_tTaskData.strSubject, \
		m_tTaskData.strTimeMode, m_tTaskData.strTime);

    return strInfo;
}


//通知任务结束的回调函数
void CAutoQuickPhoto::SetTaskMgr(CScheduleMgr *pMgr)
{
	m_pMgr = pMgr;
}

//缓存收到的执行结果
void CAutoQuickPhoto::OnReceive(CString strData)
{
	//任务执行结束
	if (strData.Find(_T("BackResult(;0)AllTaskComplete")) == 0 || 0 == strData.CompareNoCase(_T("DISCONNECTED")))
	{
		//主控异常结束重启且只重启一次 
		if (0 == strData.CompareNoCase(_T("DISCONNECTED")) && !g_bWebcDisconnected && !m_bStopWebc)
		{
			g_bWebcDisconnected = true;
			m_pMgr->TaskFinished(this);    //先提交再重试

			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("主控异常断开，重试一次！"));
			
			m_pScheduleThread->HandleRefreshKeyword();
			return;
		}


		//只要找到结束标记就代表主控正常退出
		if (strData.CompareNoCase(_T("BackResult(;0)AllTaskComplete(;0)(;0)")) == 0)
		{
			m_nJudgeWebcDisconnected = 2;
		}
		else if (strData.CompareNoCase(_T("BackResult(;0)AllTaskComplete(;0)Failed(;0)")) == 0 && !g_bCaptureKeywordFailed)
		{
			g_bCaptureKeywordFailed = true;
			m_pMgr->TaskFinished(this);    //先提交再重试

			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("抓取关键词排名失败，重试一次！"));

			m_pScheduleThread->HandleRefreshKeyword();
			return;
		}

		//通知管理模块任务结束
		m_pMgr->TaskFinished(this);
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("任务刷完或主控断开，发给送给ui！"));
		m_pScheduleThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
	}
	else
	{
		//只要收到数据就通知ui正在运行主控
		m_pScheduleThread->OnNotifyUI(WAPAM_AUTO_REFRESH_KEYWORD_RUNNING);

		//缓存要提交的关键词数据
		//m_vCacheData.push_back(strData);

		//缓存快照数据和快照名称
		CacheDataAndQuickPhotoName(strData);

		//过滤关键词数据，提交状态为成功的时候把关键词排名为零的过滤掉，失败不用过滤
		//FilterData(strData);

		//收到记录数达到阀值则提交一次
		//if (++m_nReceivedRecord % m_nThresholdValToSubmit == 0)
		//{
			//SendResultToServer();
			//m_nReceivedRecord = 0;
		//}
	}
}


void CAutoQuickPhoto::CacheDataAndQuickPhotoName(CString strData)
{
	//去掉BackResult(;0)QuickPhoto(;0)
	strData = strData.Right(strData.GetLength() - 28);

	strData.Append(_T("(;1)"));    //解决只有一条数据的情况
	CString strWhichPage = _T("");
	CString strSearchEngine = _T("");
	CString strPageName = _T("");
	CStdString strInfo = _T("");

	CStdStrUtils utl;
	std::vector<CStdString> vRes;
	vRes.clear();
	utl.SplitString(CStdString(strData.GetBuffer()), _T("(;1)"), vRes);

	vector <CStdString>::size_type res = vRes.size();
	if (res > 0)
	{
		for (int i = 0; i != res; ++i)
		{
			CStdString strTemp = vRes[i];
			CString strCacheData = vRes[i];
			if (strCacheData == _T(""))
			{
				break;
			}

			//缓存要提交的关键词数据
			m_vCacheData.push_back(strCacheData);

			std::vector<CStdString> vRet;
			vRet.clear();
			utl.SplitString(strTemp, _T("(;0)"), vRet);
			vector <CStdString>::size_type ret = vRet.size();

			if (ret > 0)
			{
				//缓存快照名称
				strPageName = vRet[4];
				if (strPageName != _T(""))
				{
					m_mapPageName[strPageName] = strCacheData;
					m_vPageName.push_back(strPageName);
				}
			}
		}
	}
}

CString CAutoQuickPhoto::FilterData(CString strMod)
{
	CString strData = _T("");
	//去除上传失败的快照数据
	if (m_vCacheUploadFailData.size() > 0)
	{
		for (int i = 0; i != m_vCacheUploadFailData.size(); ++i)
		{
			vector<CString>::iterator iter = m_vCacheData.begin();
			for (; iter != m_vCacheData.end(); ++iter)
			{
				if (m_vCacheUploadFailData[i] == *iter)
				{
					m_vCacheData.erase(iter);
					break;
				}
			}
		}
	}

	//统计本地信息
	StatisticalInfoInLoacl();

	if (m_vCacheData.size() > 0)
	{
		strData = CombineQuickPhotoData(strMod);
	}
	else
	{
		return _T("");
	}	

	return strData;
}


//本地统计信息
void CAutoQuickPhoto::StatisticalInfoInLoacl(void)
{
	CString strData = _T("");
	CString strWhichPage = _T("");
	CString strSearchEngine = _T("");
	CString strPageName = _T("");
	CStdString strInfo = _T("");

	CStdStrUtils utl;
	std::vector<CStdString> vRes;
	for (int i = 0; i != m_vCacheData.size(); ++i)
	{
		strData = m_vCacheData[i];
		vRes.clear();
		utl.SplitString(CStdString(strData.GetBuffer()), _T("(;0)"), vRes);

		vector <CStdString>::size_type res = vRes.size();
		if (res > 0)
		{
			//排名在第几页
			strWhichPage = vRes[1];

			if (strWhichPage == _T("0"))
			{
				++m_nKeywordSum;
			}
			else if (strWhichPage == _T("1"))
			{
				++m_nFirstThreePagesSum;
				++m_nKeywordSum;

				strSearchEngine = vRes[5];

				//手机百度和手机搜狗不算在第一页
				if (strSearchEngine != _T("手机百度") && strSearchEngine != _T("手机搜狗"))
				{
					++m_nFirstPageSum;
				}

				if (strSearchEngine == _T("百度"))      //在百度排名第一页
				{
					++m_nFirstPageOfBaiduSum;
				}
			}
			else
			{
				++m_nFirstThreePagesSum;
				++m_nKeywordSum;
			}
		}
		else
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("统计信息时数据解析格式不正确，data: %s"), strData);
		}
	}	
}


//组合过滤的关键词
CString CAutoQuickPhoto::CombineQuickPhotoData(CString strMode)
{
	CString strCombineData = _T("");

	if (strMode == _T("SUCCESS"))
	{
		//删除排名为0数据
		CStdStrUtils utl;
		std::vector<CStdString> vRes;

		vector<CString>::iterator iter = m_vCacheData.begin();
		while (iter != m_vCacheData.end())
		{
			CString strData = *iter;
			vRes.clear();
			utl.SplitString(CStdString(strData.GetBuffer()), _T("(;0)"), vRes);

			vector <CStdString>::size_type res = vRes.size();
			if (res > 4)
			{
				CString strTemp = vRes[4];
				if (strTemp == _T(""))
				{
					iter = m_vCacheData.erase(iter);
				}
				else
				{
					++iter;
				}
			}
		}
	}
	
	//合并关键词排名
	CStdStrUtils utl;
	std::vector<CStdString> vRet1;
	std::vector<CStdString> vRet2;
	//////////////////////////////////////////////////////////////////////////
	//合并后关键词的格式：BackResult(;0)QuickPhoto(;0)A关键词BackResult(;0)QuickPhoto(;0)B关键词
	//A关键词格式：A单条关键词（;1）A单条关键词(;1)......
	//A单条关键词格式：A关键词(;0)收录页号(;0)收录条数(;0)百度或谷歌排名(;0)快照路径(;0)搜索引擎标志( 百度、谷歌、360搜索、soso搜索、搜狗)
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != m_vCacheData.size(); ++i)
	{
		vRet1.clear();
		vRet2.clear();
		CString strFirstData = m_vCacheData[i];
		if (i == (m_vCacheData.size() - 1))
		{
			strCombineData += m_vCacheData[i];
			break;
		}
		CString strSecondData = m_vCacheData[i + 1];
		utl.SplitString(CStdString(strFirstData.GetBuffer()), _T("(;0)"), vRet1);
		utl.SplitString(CStdString(strSecondData.GetBuffer()), _T("(;0)"), vRet2);
		
		CString strFirstTemp = vRet1[0];
		CString strSecondTemp = vRet2[0];

		if (strFirstTemp == strSecondTemp)
		{
			strCombineData = strCombineData + m_vCacheData[i] + _T("(;1)");
		}
		else
		{
			strCombineData = strCombineData + m_vCacheData[i] + _T("BackResult(;0)QuickPhoto(;0)");
		}	
	}

	if (strCombineData != _T(""))
	{
		strCombineData = _T("BackResult(;0)QuickPhoto(;0)") + strCombineData;
	}

	return strCombineData;
}


/*
	@brief 将html上传到服务器
	@param [in/out]html路径
	*/
bool CAutoQuickPhoto::SaveHtmlToServer(CString &strFilePath)
{
	CString strRemotePath = _T("/QuickPhoto/QY2.0/");
	CString strTemp = PathFindFileName(strFilePath);
	strRemotePath.Append(strTemp);

	BOOL bRes = FALSE;

	if (PathFileExists(strFilePath))
	{
		//默认http上传，失败后用ftp上传，具体情况由ini配置文件控制
		if (!m_isFtpUpload)
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("http上传模式上传快照"));
			bRes = g_http.LoginAndUploadFile(strFilePath);

			if (bRes)
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("http上传成功, 快照为:%s"), strFilePath.GetString());
				return true;
			}
			else
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("http上传失败, 快照为:%s"), strFilePath.GetString());
			}	
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp上传模式上传快照"));
		}
				

		if (!bRes)
		{
			CFTP ftpTemp;
			if (!ftpTemp.IsConnect() && !ftpTemp.Connect(m_ftpLoginInfo))
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp连接失败！:%s"), ftpTemp.GetErrorMessage());
				return false;
			}
		
			bRes = ftpTemp.UpLoadFile(strRemotePath, strFilePath);

			if (bRes)
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp上传快照成功,文件名为:%s"), strFilePath.GetString());
				return true;
			}
			else
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("ftp上传快照失败,文件名为:%s, 错误信息为:%s"), strFilePath.GetString(), ftpTemp.GetErrorMessage());
			}
		}

	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("上传快照文件路径不存在: %s"), strFilePath.GetString());
	}

	return false;
}


/*
	@brief 将压缩的zip文件上传到ftp服务器
	@param [in/out]压缩的zip文件路径
	*/
bool CAutoQuickPhoto::SaveZipToServer(CString &strFilePath)
{
	CFTP ftp;
	CString strRemotePath = _T("/QuickPhoto/Zip/");
	CString strTemp = PathFindFileName(strFilePath);
	strRemotePath.Append(strTemp);

	BOOL bRes = FALSE;

	if (PathFileExists(strFilePath))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp上传模式上传zip包"));

		if (!ftp.Connect(m_ftpLoginInfo))
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接ftp服务器失败！错误信息为：%s"), ftp.GetErrorMessage());
			return false;
		}
		
		//判断是否使用断点续传
		if (m_bUploadZipFromBreak)
		{
			bRes = ftp.UpLoadFileByte(strRemotePath, strFilePath);
		}
		else
		{
			bRes = ftp.UpLoadFile(strRemotePath, strFilePath);
		}
		

		if (bRes)
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp上传zip包成功, 路径为:%s"), strFilePath.GetString());
			return true;
		}
		else
		{	
			CString strLastErrorMsg = ftp.GetErrorMessage();
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("ftp上传zip包失败, 路径为:%s,错误信息为:%s"), strFilePath.GetString(), strLastErrorMsg);
		}	
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("上传zip包路径不存在, 路径为:%s"), strFilePath.GetString());
	}

	return false;
}


void CAutoQuickPhoto::GetUploadFilePath(CString &strUploadPath, CString strMode)
{
	CString strUploadZipDir = _T("");
	CString strMacAddr = _T("");
	CString strUseName = _T("");
	CString strZipName = _T("");
	
	TCHAR szUploadPath[MAX_PATH] = {0};

	PathAppend(szUploadPath, g_pGlobalData->dir.GetInstallDir());
	PathAppend(szUploadPath, _T("\\image\\"));

	if (strMode == _T("Zip"))
	{
		strMacAddr = GetPhysicalAddress();
		strUseName = m_tTaskData.strCUID;
		strZipName.Format(_T("%s_keyword_%s\.zip"), strUseName.GetString(), strMacAddr.GetString());
		PathAppend(szUploadPath, strZipName);
	}
	else if (strMode == _T("File"))
	{
		PathAppend(szUploadPath, _T("keyword"));
	}
	
	strUploadPath = szUploadPath;
}
