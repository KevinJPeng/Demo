#include "StdAfx.h"
#include "TaskMgr.h"
#include "TaskQuickPhoto.h"
#include "FileReadAndSave.h"
#include "json/json.h"



// CInternetHttp g_http;

CTaskQuickPhoto::CTaskQuickPhoto(CTaskThread *pTaskThread, CYunTaskStreamCtr *pYunTaskStreamCtr)
{
	m_hProcess = NULL;
	m_pTaskThread = pTaskThread;
	m_pYunTaskStreamCtr = pYunTaskStreamCtr;
	m_dwExecTime = 0;
	m_bIsTaskTimeOut = FALSE;
	m_bIsMcException = FALSE;
	m_bIsPrintMCLog =  false;
	m_hRunning = ::CreateEvent(NULL, TRUE, TRUE, NULL);
}

CTaskQuickPhoto::~CTaskQuickPhoto(void)
{
	if (m_hRunning != NULL)
	{
		CloseHandle(m_hRunning);
		m_hRunning = NULL;
	}
}

//设置任务数据
void CTaskQuickPhoto::SetData(const T_TASK_DATA &tData)
{
	m_tTaskData = tData;
}

//返回任务数据
T_TASK_DATA CTaskQuickPhoto::GetData(void)
{
	return m_tTaskData;
}


//初始化对象操作
bool CTaskQuickPhoto::Init(void)
{	
	CString strInitPath = _T("");
	m_bStopWebc = false;
	m_vPageName.clear();
	m_mapPageName.clear();
	m_vCacheData.clear();
	m_vCacheUploadFailData.clear();

	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());
//	GetFtpLoginInfo(strInitPath);
	GetHttpLoginInfo(strInitPath);

	//初始化http登录信息
	g_http.GetHttpLoginInfo(m_httpLoginInfo);

	return true;
}

// void CTaskQuickPhoto::GetFtpLoginInfo(const CString &strIniPath)
// {
// 	TCHAR szIp[100];
// 	TCHAR szUserName[100];
// 	TCHAR szPwd[100];
// 	CString szInfo = _T("");
// 	int   iPort;
// 	IKeyRW key;
// 
// 	key.InitDll(KTYPE_CFG);
// 
// 	memset(szIp, 0, sizeof(szIp));
// 	memset(szUserName, 0, sizeof(szUserName));
// 	memset(szPwd, 0, sizeof(szPwd));
// 
// 	IXMLRW xml;
// 	xml.init(strIniPath);
// 	xml.ReadString(_T("MC"), _T("UPDATE"), _T("html"), szInfo);
// 
// 
// 	//httpupload为1，用http上传；httpupload为0，用ftp上传
// 	int iFtpUpLoad = -1;
// 	int iUseFtpZipUpload = -1;
// 	int iUploadZipFromBreak = -1;
// 	xml.ReadInt(_T("MC"), _T("SERVICE"), _T("HttpUpload"), iFtpUpLoad, TRUE);
// 	xml.ReadInt(_T("MC"), _T("SERVICE"), _T("UseFtpZipUpload"), iUseFtpZipUpload, TRUE);
// 	xml.ReadInt(_T("MC"), _T("SERVICE"), _T("UploadZipFromBreak"), iUploadZipFromBreak, TRUE);
// 
// 	m_isFtpUpload = iFtpUpLoad == TRUE ? FALSE : TRUE;
// 	m_bUseFtpZipUpload = iUseFtpZipUpload == TRUE ? TRUE : FALSE;
// 	m_bUploadZipFromBreak = iUploadZipFromBreak == TRUE ? TRUE : FALSE;
// 
// 	//m_isFtpUpload = GetPrivateProfileInt(_T("SERVICE"), _T("HttpUpload"), TRUE, strIniPath) == TRUE ? FALSE : TRUE;
// 	//m_bUseFtpZipUpload = GetPrivateProfileInt(_T("SERVICE"), _T("UseFtpZipUpload"), TRUE, strIniPath) == TRUE ? TRUE : FALSE;
// 	//m_bUploadZipFromBreak = GetPrivateProfileInt(_T("SERVICE"), _T("UploadZipFromBreak"), TRUE, strIniPath) == TRUE ? TRUE : FALSE;
// 
// 	CString strDecrypt;
// 	key.DecryptData(szInfo, DECRYPTCFG, strDecrypt);
// 
// 	if (strDecrypt.GetLength() > 0)
// 	{
// 		GetFtpDesCode(strDecrypt.GetBuffer(0), szIp, szUserName, szPwd, iPort);
// 		strDecrypt.ReleaseBuffer();
// 	}
// 	
// 	m_ftpLoginInfo.strServer = szIp;
// 	m_ftpLoginInfo.strUser = szUserName;
// 	m_ftpLoginInfo.strPwd = szPwd;
// 	m_ftpLoginInfo.nPort = iPort;
// 	m_ftpLoginInfo.bPassive = TRUE;
// }

void CTaskQuickPhoto::GetHttpLoginInfo(const CString &strIniPath)
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

	if (strDecrypt.GetLength() > 0)
	{
		GetHttpDesCode(strDecrypt.GetBuffer(0), szIp, szUploadUrl, szUserName, szPwd);
		strDecrypt.ReleaseBuffer();
	}
	m_httpLoginInfo.strServer = szIp;
	m_httpLoginInfo.strUploadUrl = szUploadUrl;
	m_httpLoginInfo.strUser = szUserName;
	m_httpLoginInfo.strPwd = szPwd;


	memset(szIp, 0, sizeof(szIp));
	memset(szUploadUrl, 0, sizeof(szUploadUrl));
	memset(szUserName, 0, sizeof(szUserName));
	memset(szPwd, 0, sizeof(szPwd));
	xml.ReadString(_T("MC"), _T("UPDATE"), _T("http2"), szInfo);
	key.DecryptData(szInfo, DECRYPTCFG, strDecrypt);

	if (strDecrypt.GetLength() > 0)
	{
		GetHttpDesCode(strDecrypt.GetBuffer(0), szIp, szUploadUrl, szUserName, szPwd);
		strDecrypt.ReleaseBuffer();
	}
	m_httpLoginInfo2.strServer = szIp;
	m_httpLoginInfo2.strUploadUrl = szUploadUrl;
	m_httpLoginInfo2.strUser = szUserName;
	m_httpLoginInfo2.strPwd = szPwd;


	memset(szIp, 0, sizeof(szIp));
	memset(szUploadUrl, 0, sizeof(szUploadUrl));
	memset(szUserName, 0, sizeof(szUserName));
	memset(szPwd, 0, sizeof(szPwd));
	xml.ReadString(_T("MC"), _T("UPDATE"), _T("OSSInfo"), szInfo);
	key.DecryptData(szInfo, DECRYPTCFG, strDecrypt);

	if (strDecrypt.GetLength() > 0)
	{
		GetHttpDesCode(strDecrypt.GetBuffer(0), szIp, szUploadUrl, szUserName, szPwd);
		strDecrypt.ReleaseBuffer();
	}
	m_ossInfo.sOssEndpoint = szIp;
	m_ossInfo.sAccessKeyId = szUploadUrl;
	m_ossInfo.sAccessKeySecret = szUserName;
	m_ossInfo.sBucketName = szPwd;
	


	//httpupload为1，用http上传；httpupload为0，用ftp上传
	int ihttpUpLoad = -1;
	int iSdkUpload = -1;
	xml.ReadInt(_T("MC"), _T("SERVICE"), _T("HttpUpload"), ihttpUpLoad, TRUE);
	xml.ReadInt(_T("MC"), _T("SERVICE"), _T("OssSdkUpload"), iSdkUpload, TRUE);
	m_bUsehttpZipUpload = ihttpUpLoad == TRUE ? TRUE : FALSE;
	m_bUseOssSDKUpload = iSdkUpload == TRUE ? TRUE : FALSE;
}

void CTaskQuickPhoto::GetFtpDesCode(TCHAR *code, TCHAR *ip, TCHAR *account, TCHAR *psd, int &port)
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


void CTaskQuickPhoto::GetHttpDesCode(TCHAR *code, TCHAR *pHome, TCHAR *pFileUpload, TCHAR *account, TCHAR *psd)
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
bool CTaskQuickPhoto::CanExecNow()
{
	//否则检查主控是否正在运行
	if (ProcessExist(MC_PROCESS_NAME))
	{	
		if (IsOwnerMCProcess())
		{	
			if (!m_bIsPrintMCLog)
			{
				g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("执行刷排名任务时检测到主控已经存在,等待主控退出..."));

				//设置已经打印主控已经存在日志标记，控制只打印一次
				m_bIsPrintMCLog = true;
			}
			return false;
		}
	}

	return true;
}

//执行任务
DWORD CTaskQuickPhoto::Exec(void)
{

#if 1
	DWORD dwReValue = 1;
	CString strCmd = _T("");
	CString strPort = _T("");
	int nPort = GetPort();
	strPort.Format(_T(" %d -y"), nPort);

	strCmd.Format(_T("%s"), g_pGlobalData->dir.GetMcProcPath());
	strCmd.Replace(_T("MC.exe"), _T("UpdateRank.exe"));
	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("开始执行任务，subject:%s cmd:%s strPort:%s"), m_tTaskData.strSubject.GetString(), strCmd.GetString(), strPort.GetString());

	for (int i = 0; i < 3; i++)
	{
		if (StartProcess(strCmd.GetBuffer(), strPort.GetBuffer(), &m_hProcess))
		{
			if (-1 == m_comm.Init(nPort, this))
			{
				g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("启动任务失败，subject:%s cmd:%s"), m_tTaskData.strSubject, strCmd);
			}
			else
			{
				//起了主控，正在刷关键词排名
				g_YunDataLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("主控启动刷关键词!"));
				m_dwExecTime = GetTickCount();
				int iSendLength = m_comm.SendData(m_tTaskData.strData.GetBuffer());
				if (iSendLength < 0)
				{
					g_YunDataLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("向主控发送关键词排名任务数据失败! errorcode:%d"), GetLastError());
				}
				else
				{
					dwReValue = 1;
					break;
				}
			}
			Stop();
			dwReValue = 0;
			Sleep(1000);
		}
		else
		{
			g_YunDataLog.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("主控未启动 ret：%d"), GetLastError());
			dwReValue = 0;
			Sleep(1000);
		}
	}

	return dwReValue;
#else
	DWORD dwReValue = 1;
	CString strCmd = _T("");
	CString strPort = _T("");
	int nPort = GetPort();
	strPort.Format(_T(" %d -y"), nPort);

	strCmd.Format(_T("%s"), g_pGlobalData->dir.GetMcProcPath());
	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("开始执行任务，subject:%s cmd:%s strPort:%s"), m_tTaskData.strSubject.GetString(), strCmd.GetString(), strPort.GetString());

	for (int i = 0; i < 10; i++)
	{
		if (StartProcess(strCmd.GetBuffer(), strPort.GetBuffer(), &m_hProcess))
		{
			if (-1 == m_comm.Init(nPort, this))
			{
				g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("启动任务失败，subject:%s cmd:%s"), m_tTaskData.strSubject, strCmd);
				//	m_pTaskThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
				Stop();

				dwReValue = 0;
				break;
			}
			else
			{
				//起了主控，正在刷关键词排名
				g_YunDataLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("主控启动刷关键词!"));
				// m_pTaskThread->OnNotifyUI(WAPAM_AUTO_REFRESH_KEYWORD_RUNNING);
			}

			m_dwExecTime = GetTickCount();
			int iSendLength = m_comm.SendData(m_tTaskData.strData.GetBuffer());
			if (iSendLength < 0)
			{
				g_YunDataLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("向主控发送关键词排名任务数据失败! errorcode:%d"), GetLastError());
			}

			//主控的抓排名模块有一定几率加载失败，故增加以下代码进行检测
			if (CheckUpdateRankModule(10 * 1000))
			{
				dwReValue = 1;
				break;
			}
			else
			{
				Stop();
				dwReValue = 0;
				g_YunDataLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("UpdateRank.dll模块启动失败! Retry:%d"), i);
			}

			//return 1;
		}
		else
		{
			g_YunDataLog.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("主控未启动 ret：%d"), GetLastError());
			//m_pTaskThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
			//return 0;
			dwReValue = 0;
			break;
		}
	}

	return dwReValue;
#endif
}

//停止任务
void CTaskQuickPhoto::Stop()
{
	m_comm.Stop();
	if (m_hProcess != NULL)
	{
		TerminateProcess(m_hProcess, 1);
		m_bStopWebc = true;
		g_YunDataLog.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("停止任务, err:%d，subject:%s "), GetLastError(), m_tTaskData.strSubject.GetString());
	}
}

//向服务器提交执行结果
bool CTaskQuickPhoto::SendResultToServer(void)
{	
	//调试数据无需提交和上传结果
	if (!m_pYunTaskStreamCtr->IsNeedSumitResult())
	{	
		//本地调试数据不需要提交
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("本次关键词任务为本地调试,不需要提交数据到服务器!"));
		return TRUE;
	}

	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("SendResultToServer:type:%s subject:%s"), m_tTaskData.strType.GetString(), m_tTaskData.strSubject.GetString());

	//上传快照
	bool bAllUploadFail = false;
	if (!UploadFileToServer())
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("所有方式上传快照失败！"));
		bAllUploadFail = true;
		//return false;
	}

	CString strURL = _T("");
	CString strTEP = _T("");
	CTime timep;
	timep = GetCurrentTime();
	CString strTime = _T("");
	strTime.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), timep.GetYear(), timep.GetMonth(), timep.GetDay(), timep.GetHour(), timep.GetMinute(), timep.GetSecond());

	CString strCacheData = _T("");
	strCacheData = FilterData();

	if (strCacheData.CompareNoCase(_T("")) == 0 && m_bIsMcException)
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("主控异常且提交数据为空!"));
		return false;
	}
	else if (strCacheData.CompareNoCase(_T("")) == 0 && !m_bIsMcException)
	{

		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("主控正常返回且提交数据为空!"));
		return true;
	}

	// 去除最后一个(;1)
	CStdString strTemp = strCacheData;
	if (true == bAllUploadFail)
	{
		CStdStrUtils strUils;
		vector<CStdString> vSourRes;
		vector<CStdString> vDesRes;
		vSourRes.clear();
		vDesRes.clear();
		strUils.SplitString(CStdString(strTemp.GetBuffer()), _T("(;1)"), vSourRes, false);
		//strUils.SplitString(CStdString(strTemp.GetBuffer()), _T("(;1)"), vSourRes, true);
		int ilen = vSourRes.size();
		for (int i = 0; i < ilen; i++)
		{
			vector<CStdString> vSourSingle;
			CStdString sDesSingle = _T("");
			vSourSingle.clear();
			strUils.SplitString(CStdString(vSourRes[i].GetBuffer()), _T("(;0)"), vSourSingle, true);
			int iSize = vSourSingle.size();
			if (iSize > 5 && _ttoi(vSourSingle[5]) > 0)
			{
				vSourSingle[5] = _T("-3");
				for (int i = 0; i < iSize; i++)
				{
					if (sDesSingle.IsEmpty())
					{
						sDesSingle += vSourSingle[i];
					} 
					else
					{
						sDesSingle += _T("(;0)");
						sDesSingle += vSourSingle[i];
					}
				}
			}
			else
			{
				sDesSingle = vSourRes[i];
			}
			vDesRes.push_back(sDesSingle);
		}

		int ivlen = vDesRes.size();
		if (ivlen > 0)
		{
			strTemp = _T("");
		}
		for (int i = 0; i < ivlen; i++)
		{
			strTemp += vDesRes[i];
			strTemp += _T("(;1)");
		}
	}

	strCacheData = strTemp.Left(strTemp.length() - 4);
	CString strPostData = _T("");

	//要传输的数据格式：执行结果||MAC地址||城市||0  (0表示云刷)
	strPostData.Format(_T("%s||%s||%s||0"), \
		strCacheData, GetPhysicalAddress(), m_pYunTaskStreamCtr->GetArea());

	CFileReadAndSave encryptCode;
	CStdString strPostDataZip = _T("");
	CString strPostUrlEncodeData = _T("");
	encryptCode.ZipEncodeStdString(strPostData.GetBuffer(), strPostDataZip);

	strPostUrlEncodeData.Format(_T("<@>&%s&%s&<@>"), strTime, strPostDataZip.c_str());
	m_pYunTaskStreamCtr->EncryptRequData(strPostUrlEncodeData);
	/*strPostUrlEncodeData = m_pYunTaskStreamCtr->URLEncode(strPostDataZip.GetBuffer());*/
	strPostUrlEncodeData = _T("result=") + strPostUrlEncodeData;

	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("要传输的urlEncode数据为：%s"), strPostUrlEncodeData.GetString());

	int nRetry = 0;
Retry:
	CString strResponseText = _T("");
	//CString surl = _T("http://kwdapi.sumszw.cn/api/KeywordCacheService/SaveKeyWordDetail");
	int iRet = g_http.HttpPost(m_tTaskData.strPostAddr, strPostUrlEncodeData, strResponseText);
	if (0 != iRet || 0 != strResponseText.CompareNoCase(_T("ok")))
	{
		++nRetry;
		//提交失败重试一次
		if (nRetry < 2)
		{
			g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("提交任务结果失败，重试%d次！"), nRetry);
			goto Retry;
		}
	}

	if (nRetry < 2)
	{
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交任务结果成功，信息：%s"), GetTaskInfo());
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("提交任务结果失败！ret:%d, lasterr:%d, errmsg:%s"), iRet, GetLastError(), strResponseText);
	}	

	return true;
}

//等待接收线程结束
void CTaskQuickPhoto::WaitExit()
{
	m_comm.WaitReceiveExit();
}

//释放内存通告
void CTaskQuickPhoto::ReleaseTask()
{
	m_pMgr->NotifyReleaseTask(this);
}


//遍历快照目录下的所有公司名目录并把目录名存进vector中
void CTaskQuickPhoto::FindAddSaveCompanyNameDir(vector<LPCTSTR> &vComNameDir)
{
	vComNameDir.clear();
	TCHAR szPath[MAX_PATH] = { 0 };

	PathAppend(szPath, g_pGlobalData->dir.GetInstallDir());
	PathAppend(szPath, _T("\\image\\keyword\\"));
	CString strPath = szPath;
	_tcscat(szPath, _T("*.*"));

	WIN32_FIND_DATA fd;
	HANDLE hFile = ::FindFirstFile(szPath, &fd);

	while (hFile != INVALID_HANDLE_VALUE)
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // 目录
		{
			// 排除.和..目录
			if (fd.cFileName[0] != _T('.'))
			{
				CString strFullPath = strPath + fd.cFileName;
				TCHAR *pDir = new TCHAR[260];
				memset(pDir, 0, 260);
				memcpy(pDir, strFullPath.GetBuffer(), strFullPath.GetLength() * sizeof(TCHAR));
				vComNameDir.push_back(pDir);
			}
		}
		if (!::FindNextFile(hFile, &fd))
			break;
	}
	FindClose(hFile);
}


//删除快照文件下的所有子目录和文件
bool CTaskQuickPhoto::DeleteAllFiles(LPCTSTR szDirPath)
{
	SHFILEOPSTRUCT FileOp;
	ZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCT));

	FileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
	FileOp.hNameMappings = NULL;
	FileOp.hwnd = NULL;
	FileOp.lpszProgressTitle = NULL;
	FileOp.pFrom = szDirPath;
	FileOp.pTo = NULL;
	FileOp.wFunc = FO_DELETE;

	return SHFileOperation(&FileOp) == 0;
}

// FTP上传
// bool CTaskQuickPhoto::FtpUpload()
// {
// 	bool bSuccess = false;
// 	bool bRequst  = false;
// 	CString strUploadZipPath = _T("");
// 
// 	GetUploadFilePath(strUploadZipPath, _T("Zip"));
// 	// 压缩快照
// 	std::vector<LPCTSTR> vDir;
// 	FindAddSaveCompanyNameDir(vDir);
// 	DWORD dwCompressErr = 0;
// 
// 	for (int iCount = 0; iCount < 3; iCount++)
// 	{
// 		dwCompressErr = ZipUtils::CompressDirsToZip(strUploadZipPath.GetBuffer(), vDir);
// 
// 		if (dwCompressErr != 0)
// 		{
// 			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("压缩文件失败重试，次数:%d,err: %d"), iCount+1, GetLastError());
// 
// 			Sleep(3000);
// 			continue;
// 		}
// 		else
// 		{
// 			bSuccess = true;
// 			break;
// 		}
// 	}
// 
// 	if (!bSuccess)
// 	{
// 		goto ENDFTP;
// 	}
// 	
// 	// 上传ZIP文件包
// 	bSuccess = SaveZipToServer(strUploadZipPath);
// 	// 上传成功请求ftp服务器解压缩zip包
// 	//if (bSuccess)
// 	//{
// 	//	CString strPostData = _T("");
// 	//	CString strResponseTxt = _T("");
// 	//	CString strPostAddr = _T("");
// 	//	CString strFileName = PathFindFileName(strUploadZipPath);
// 	//	strPostAddr.Format(_T("%s/File/ExtractZipFile?filename=%s"), m_httpLoginInfo.strServer, strFileName);
// 	//	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("请求ftp服务器解压缩zip包地址：%s"), strPostAddr);
// 	//	// 请求2次如果失败用HTTP上传
// 	//	for (int iCount = 0; iCount < 2; ++iCount)
// 	//	{
// 	//		int iRet = g_http.HttpPost(strPostAddr, strPostData, strResponseTxt);
// 	//		if (0 != iRet || 0 != strResponseTxt.CompareNoCase(_T("1")))
// 	//		{
// 	//			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("请求ftp服务器解压缩zip包失败，重试一次！：ret:%d, lasterr:%d, errmsg:%s"), iRet, GetLastError(), strResponseTxt);
// 	//			Sleep(2000);
// 	//		}
// 	//		else
// 	//		{
// 	//			// 成功
// 	//			bRequst = true;
// 	//			break;
// 	//		}
// 	//	}
// 	//	bSuccess = bRequst;
// 	//}
// 	DeleteFile(strUploadZipPath);
// 	// 如果请求成功需清除快照下面的文件夹???失败了不清除吗？
// 	if (bSuccess)
// 	{
// 		for (int i = 0; i < vDir.size(); i++)
// 		{
// 			DeleteAllFiles(vDir[i]);
// 		}
// 	}
// 
// 	// 释放VECTOR内存
// ENDFTP:
// 	for (int j = 0; j < vDir.size(); j++)
// 	{
// 		delete[] vDir[j];
// 		vDir[j] = NULL;
// 	}
// 	vDir.clear();
// 
// 	return bSuccess;
// }

bool CTaskQuickPhoto::ALiYunAPIUpload()
{
	bool bReValue = false;
	CString strUploadFileDir = _T("");
	CString strUploadFilePath = _T("");
	GetUploadFilePath(strUploadFileDir, _T("File"));

	CString strResponseText = _T("");
	CHttpUtils http;
	CString strURL = m_httpLoginInfo2.strUploadUrl;
	strResponseText = http.GetSerRespInfo(strURL.GetBuffer());
	CAliOssApi aLiOssApi;
	char* pJsondata = NULL;

	pJsondata = ToMultiByte(strResponseText.GetBuffer());
	if (!parseOssAccessInfo(pJsondata, aLiOssApi.m_ossInfo))
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("JSon解析失败，JSon数据为：%s, 从本地缓存获取账号信息"), strResponseText);
		aLiOssApi.m_ossInfo.sOssEndpoint = this->m_ossInfo.sOssEndpoint;
		aLiOssApi.m_ossInfo.sAccessKeyId = this->m_ossInfo.sAccessKeyId;
		aLiOssApi.m_ossInfo.sAccessKeySecret = this->m_ossInfo.sAccessKeySecret;
		aLiOssApi.m_ossInfo.sBucketName = this->m_ossInfo.sBucketName;
	}


	if (aLiOssApi.m_ossInfo.sOssEndpoint.IsEmpty() || aLiOssApi.m_ossInfo.sBucketName.IsEmpty())
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("获取oss信息失败，sOssEndpoint 或 sBucketName为空"));
		if (NULL != pJsondata)
		{
			delete[] pJsondata;
			pJsondata = NULL;
		}
		return false;
	}
	//判断从服务器获取的信息是否与本地相同，若不相同，更新本地缓存
	if ((aLiOssApi.m_ossInfo.sOssEndpoint != this->m_ossInfo.sOssEndpoint)
		|| (aLiOssApi.m_ossInfo.sAccessKeyId != this->m_ossInfo.sAccessKeyId) 
		|| (aLiOssApi.m_ossInfo.sAccessKeySecret != this->m_ossInfo.sAccessKeySecret) 
		|| (aLiOssApi.m_ossInfo.sBucketName != this->m_ossInfo.sBucketName)
		)
	{
		CString sOssInfo = _T("");
		sOssInfo.Format(_T("%s|%s|%s|%s"), aLiOssApi.m_ossInfo.sOssEndpoint, aLiOssApi.m_ossInfo.sAccessKeyId, aLiOssApi.m_ossInfo.sAccessKeySecret, aLiOssApi.m_ossInfo.sBucketName);

		IKeyRW key;
		key.InitDll(KTYPE_CFG);
		CString strDecrypt;
		key.EncryptData(sOssInfo, DECRYPTCFG, strDecrypt);


		CString strInitPath = _T("");
		strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());
		IXMLRW xml;
		xml.init(strInitPath);
		xml.WriteString(_T("MC"), _T("UPDATE"), _T("OSSInfo"), strDecrypt);
	}

	if (NULL != pJsondata)
	{
		delete[] pJsondata;
		pJsondata = NULL;
	}

	strUploadFilePath = strUploadFileDir + _T("\\");
	if (aLiOssApi.put_object_from_file(strUploadFilePath, m_vPageName, m_vCacheUploadFailData, m_mapPageName))
	{
		bReValue = true;
	}
	aLiOssApi.UnInit();

	// 删除快照目录下的文件夹 清除内存
	std::vector<LPCTSTR> vDir;
	FindAddSaveCompanyNameDir(vDir);
	for (int i = 0; i < vDir.size(); i++)
	{
		DeleteAllFiles(vDir[i]);
	}
	for (int j = 0; j < vDir.size(); j++)
	{
		delete[] vDir[j];
		vDir[j] = NULL;
	}
	vDir.clear();

	//当提交的所有有排名的快照文件都失败时，不提交快照数据
	if (m_vPageName.size() == m_vCacheUploadFailData.size())
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("提交每个快照文件都失败，不提交快照数据！"));
		return false;
	}
	return bReValue;
}
bool CTaskQuickPhoto::parseOssAccessInfo(const char *pJson, OssAccessInfo& _ossAccessInfo)
{
	//{"code":0,"data":{"endpoint":"oss-cn-shenzhen.aliyuncs.com","accessKeyId":"LTAIdltXZA2s4vdW","accessKeySecret":"KBMKltf14o51QnWexRjmE9rdgmzwl0",
	//"bucketList":[{"typeId":1,"bucketName":"sumszw-snapshot"},{"typeId":2,"bucketName":"sumszw-image"}]}}

	//{ "code":0, "data" : {"Endpoint":"oss-cn-shenzhen.aliyuncs.com", "IntEndpoint" : "oss-cn-shenzhen-internal.aliyuncs.com", "IsOpenInt" : false, "AccessKeyId" : "LTAIdltXZA2s4vdW", "AccessKeySecret" : "KBMKltf14o51QnWexRjmE9rdgmzwl0", "BucketList" : [{"TypeId":1, "BucketName" : "kz-snapshot"}, { "TypeId":2, "BucketName" : "sumszw-image" }]} }


	int iCode = -1;
	CString strReValue(_T(""));
	if (pJson != NULL)
	{
		Json::Reader reader;
		Json::Value json_object;

		if (reader.parse(pJson, json_object))
		{
			iCode = json_object["code"].asInt();
			if (iCode == 0)
			{
				try
				{
					_ossAccessInfo.sOssEndpoint = json_object["data"]["endpoint"].asCString();
					_ossAccessInfo.sAccessKeyId = json_object["data"]["accessKeyId"].asCString();
					_ossAccessInfo.sAccessKeySecret = json_object["data"]["accessKeySecret"].asCString();

// 					_ossAccessInfo.sOssEndpoint = json_object["data"]["Endpoint"].asCString();
// 					_ossAccessInfo.sAccessKeyId = json_object["data"]["AccessKeyId"].asCString();
// 					_ossAccessInfo.sAccessKeySecret = json_object["data"]["AccessKeySecret"].asCString();

					int iSize = json_object["data"]["bucketList"].size();
					for (int i = 0; i < iSize; i++)
					{
						int iTypeId = json_object["data"]["bucketList"][i]["typeId"].asInt();
						if (1 == iTypeId)
						{
							_ossAccessInfo.sBucketName = json_object["data"]["bucketList"][i]["bucketName"].asCString();
							return true;
						}
					}
				}
				catch (CException* e)
				{
					return false;
				}
			}
		}
	}
	return false;
}

CString CTaskQuickPhoto::GetResponseErrorCode(const char *pJson)
{
	//{"code":0, "msg" : "操作成功"}0
	int intReValue = -1;
	CString strReValue(_T(""));
	if (pJson != NULL)
	{
		Json::Reader reader;
		Json::Value json_object;

		if (reader.parse(pJson, json_object))
		{
			try
			{
				intReValue = json_object["code"].asInt();
				strReValue.Format(_T("%d"), intReValue);
			}
			catch (CException* e)
			{
				strReValue = _T("-1");
			}
		}
	}
	return strReValue;
}
char * CTaskQuickPhoto::ToMultiByte(LPCWSTR pszSource)
{
	int nLanguage = CP_ACP;

	int nLength = WideCharToMultiByte(nLanguage, 0, pszSource, wcslen(pszSource), NULL, 0, NULL, FALSE);
	char *pBuffer = NULL;
	pBuffer = new char[nLength + 2];
	memset(pBuffer, 0, nLength + 2);
	if (!pBuffer)
	{
		return NULL;
	}
	WideCharToMultiByte(nLanguage, 0, pszSource, wcslen(pszSource), pBuffer, nLength, NULL, FALSE);
	pBuffer[nLength + 1] = 0;

	return pBuffer;
}

// HTTP上传快照
bool CTaskQuickPhoto::HttpUpload2()
{
	bool bSuccess = false;
	CString strUploadZipPath = _T("");

	GetUploadFilePath(strUploadZipPath, _T("Zip"));
	// 压缩快照
	std::vector<LPCTSTR> vDir;
	FindAddSaveCompanyNameDir(vDir);
	DWORD dwCompressErr = 0;
	for (int iCount = 0; iCount < 3; iCount++)
	{
		dwCompressErr = ZipUtils::CompressDirsToZip(strUploadZipPath.GetBuffer(), vDir);

		if (dwCompressErr != 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("压缩文件失败重试，次数:%d,err: %d"), iCount + 1, GetLastError());
			Sleep(3000);
			continue;
		}
		else
		{
			bSuccess = true;
			break;
		}
	}

	if (!bSuccess)
	{
		// 释放VECTOR内存
		for (int j = 0; j < vDir.size(); j++)
		{
			delete[] vDir[j];
			vDir[j] = NULL;
		}
		vDir.clear();
		return bSuccess;
	}

	if (PathFileExists(strUploadZipPath))
	{
		int iRetry = 3;
		do
		{
			CString strResponse(_T(""));
			g_http.UploadFile(m_httpLoginInfo.strUploadUrl, strUploadZipPath, strResponse);
			//g_http.UploadFile(_T("http://192.168.1.14:7001/upload/UploadFile"), strUploadZipPath, strResponse);
			if (strResponse.IsEmpty())
			{
				bSuccess = false;
				g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("http模式上传快照zip压缩档得到服务器返回的数据为空"));
			}
			else if (strResponse.Find(_T("code")))
			{
				char* pJsondata = NULL;
				pJsondata = ToMultiByte(strResponse.GetBuffer());
				strResponse = GetResponseErrorCode(pJsondata);
				if (pJsondata)
				{
					delete[] pJsondata;
					pJsondata = NULL;
				}
				if (_T("0") == strResponse)
				{
					bSuccess = true;
					break;
				}
				else
				{
					bSuccess = false;
					g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("http模式上传快照zip压缩档失败, 错误码为:%s, 快照路径：%s"), strResponse, strUploadZipPath);
				}
			}
			else
			{
				bSuccess = false;
				g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("http模式上传快照zip压缩档失败，错误数据为:%s"), strResponse);
			}
		} while ((--iRetry) > 0);
	}
	else
	{
		bSuccess = false;
	}
	// 删除ZIP文件包
	DeleteFile(strUploadZipPath);
	// 如果请求成功需清除快照下面的文件夹???失败了不清除吗？失败不清除，因为还有其它上传方式，数据需要保留
	if (bSuccess)
	{
		for (int i = 0; i < vDir.size(); i++)
		{
			DeleteAllFiles(vDir[i]);
		}
	}

	// 释放VECTOR内存
	for (int j = 0; j < vDir.size(); j++)
	{
		delete[] vDir[j];
		vDir[j] = NULL;
	}
	vDir.clear();

	return bSuccess;
}

// HTTP上传快照
// bool CTaskQuickPhoto::HttpUpload()
// {
// 	CString strUploadFileDir = _T("");
// 	CString strUploadFilePath = _T("");
// 	GetUploadFilePath(strUploadFileDir, _T("File"));
// 
// 	//上传快照文件采用非压缩的方式单个上传
// 	for (int i = 0; i != m_vPageName.size(); ++i)
// 	{
// 		strUploadFilePath = strUploadFileDir + _T("\\") + m_vPageName[i];
// 		if (!SaveHtmlToServer(strUploadFilePath))
// 		{
// 			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("提交单个快照时该快照失败，快照路径：%s"), strUploadFilePath);
// 			CString strCache = m_mapPageName[m_vPageName[i]];
// 			m_vCacheUploadFailData.push_back(strCache);
// 		}
// 	}
// 
// 	// 删除快照目录下的文件夹 清除内存
// 	std::vector<LPCTSTR> vDir;
// 	FindAddSaveCompanyNameDir(vDir);
// 	for (int i = 0; i < vDir.size(); i++)
// 	{
// 		DeleteAllFiles(vDir[i]);
// 	}
// 	for (int j = 0; j < vDir.size(); j++)
// 	{
// 		delete[] vDir[j];
// 		vDir[j] = NULL;
// 	}
// 	vDir.clear();
// 
// 	//当提交的所有有排名的快照文件都失败时，不提交快照数据
// 	if (m_vPageName.size() == m_vCacheUploadFailData.size())
// 	{
// 		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("提交每个快照文件都失败，不提交快照数据！"));
// 		return false;
// 	}
// 	return true;
// }

bool CTaskQuickPhoto::UploadFileToServer(void)
{
	//抓到有排名才提交快照
	if (!m_vPageName.empty())
	{
		bool bSuccess = false;
		// 用FTP上传
		DWORD dwTime = GetTickCount();
// 		if (m_bUseFtpZipUpload)
// 		{
// 			bSuccess = FtpUpload();
// 		}
// 		// FTP失败或没指定FTP用HTTP上传
// 		if (!bSuccess)
// 		{
// 			bSuccess = HttpUpload();
// 		}
		if (m_bUsehttpZipUpload)
		{
			bSuccess = HttpUpload2();
		}
		if (!bSuccess)
		{
			if (m_bUseOssSDKUpload)
			{
				g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("上传.zip压缩档失败，采用阿里云c-sdk上传"));
				bSuccess = ALiYunAPIUpload();
			}
		}

		dwTime = GetTickCount() - dwTime;
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("本次上传快照时间,%.2f,秒"), dwTime / 1000.0);

		return bSuccess;
	}
	// 没有快照不提交需要删除创建的文件夹
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("没有快照清除文件夹!"));

	std::vector<LPCTSTR> vDir;
	FindAddSaveCompanyNameDir(vDir);

	for (int i = 0; i < vDir.size(); i++)
	{
		DeleteAllFiles(vDir[i]);
	}
	for (int j = 0; j < vDir.size(); j++)
	{
		delete[] vDir[j];
		vDir[j] = NULL;
	}
	vDir.clear();

	return true;
}


//获取任务的标识信息
CString CTaskQuickPhoto::GetTaskInfo(void)
{
	CString strInfo = _T("");
	strInfo.Format(_T("uid=%s,type=%s,subject=%s,timemode=%s,time=%s"), \
		m_tTaskData.strUID, m_tTaskData.strType, m_tTaskData.strSubject, \
		m_tTaskData.strTimeMode, m_tTaskData.strTime);

    return strInfo;
}


//通知任务结束的回调函数
void CTaskQuickPhoto::SetTaskMgr(CTaskMgr *pMgr)
{
	m_pMgr = pMgr;
}

//缓存收到的执行结果
void CTaskQuickPhoto::OnReceive(CString strData)
{
	g_YunDataLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到主控传过来的数据:%s"), strData);

	//任务执行结束
	if (strData.Find(_T("BackResult(;0)AllTaskComplete")) == 0 || 0 == strData.CompareNoCase(_T("DISCONNECTED")))
	{
		// 主控异常断开
		if (0 == strData.CompareNoCase(_T("DISCONNECTED")) && !m_bStopWebc)
		{	
			m_bIsMcException = TRUE;
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("主控异常断开!"));
		}
		//通知管理模块任务结束
		m_pMgr->TaskFinished(this);
	}
	else
	{
		//缓存快照数据和快照名称
		CacheDataAndQuickPhotoName(strData);
	}
}


// 请求下个云任务
void CTaskQuickPhoto::RequestNextTask()
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

void CTaskQuickPhoto::CacheDataAndQuickPhotoName(CString strData)
{
	if (strData.Compare(_T("")) == 0)
	{
		return;
	}
	CStdString strReciveData = strData;
	CString strPageName = _T("");

	CStdStrUtils utl;
	std::vector<CStdString> vRes;
	vRes.clear();
	utl.SplitString(strReciveData, _T("(;1)"), vRes);

	vector <CStdString>::size_type res = vRes.size();
	if (res > 0)
	{
		for (int i = 0; i != res; ++i)
		{
			CStdString strTemp = vRes[i];
			CString strCacheData = vRes[i];
			if (strCacheData == _T(""))
			{
				continue;
			}

			//缓存要提交的关键词数据
			m_vCacheData.push_back(strCacheData);

			std::vector<CStdString> vRet;
			vRet.clear();
			utl.SplitString(strTemp, _T("(;0)"), vRet);
			vector <CStdString>::size_type ret = vRet.size();

			if (ret < 4)
			{
				continue;
			}
			else
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

CString CTaskQuickPhoto::FilterData()
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

	if (m_vCacheData.size() > 0)
	{
		strData = CombineQuickPhotoData();
	}
	else
	{
		return _T("");
	}	

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("快照数据条数:%d"), m_vCacheData.size());

	return strData;
}


//组合过滤的关键词
CString CTaskQuickPhoto::CombineQuickPhotoData()
{
	CString strCombineData = _T("");

	for (int i = 0; i != m_vCacheData.size(); ++i)
	{
		strCombineData += m_vCacheData[i];
		strCombineData += _T("(;1)");
	}

	return strCombineData;
}


/*
	@brief 将html上传到服务器
	@param [in/out]html路径
	*/
// bool CTaskQuickPhoto::SaveHtmlToServer(CString &strFilePath)
// {
// 	CString strRemotePath = _T("/QuickPhoto/QY2.0/");
// 	CString strTemp = PathFindFileName(strFilePath);
// 	int iPos1 = strFilePath.ReverseFind(_T('/'));
// 	CString strTem = strFilePath.Mid(0, iPos1);
// 	int iPos2 = strTem.ReverseFind(_T('\\'));
// 	CString strPath = strTem.Mid(iPos2 + 1);
// 	strRemotePath.Append(strPath);
// 	strRemotePath.Append(_T("/"));
// 	strRemotePath.Append(strTemp);
// 
// 	BOOL bRes = FALSE;
// 
// 	if (PathFileExists(strFilePath))
// 	{
// 		//默认http上传，失败后用ftp上传，具体情况由ini配置文件控制
// 		if (!m_isFtpUpload)
// 		{
// 			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("http上传模式上传快照"));
// 			bRes = g_http.LoginAndUploadFile(strFilePath);
// 
// 			if (bRes)
// 			{
// 				//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("http上传成功, 快照为:%s"), strFilePath.GetString());
// 				return true;
// 			}
// 			else
// 			{
// 				//g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("http上传失败, 快照为:%s"), strFilePath.GetString());
// 			}	
// 		}
// 		else
// 		{
// 			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp上传模式上传快照"));
// 			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp文件路径:%s"), strRemotePath.GetString());
// 		}
// 				
// 
// 		if (!bRes)
// 		{
// 			CFTP ftpTemp;
// 			if (!ftpTemp.IsConnect() && !ftpTemp.Connect(m_ftpLoginInfo))
// 			{
// 				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp连接失败！:%s"), ftpTemp.GetErrorMessage());
// 				return false;
// 			}
// 		
// 			bRes = ftpTemp.UpLoadFile(strRemotePath, strFilePath);
// 
// 			if (bRes)
// 			{
// 				//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp上传快照成功,文件名为:%s"), strFilePath.GetString());
// 				return true;
// 			}
// 			else
// 			{
// 				//g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("ftp上传快照失败,文件名为:%s, 错误信息为:%s"), strFilePath.GetString(), ftpTemp.GetErrorMessage());
// 			}
// 		}
// 
// 	}
// 	else
// 	{
// 		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("上传快照文件路径不存在: %s"), strFilePath.GetString());
// 	}
// 
// 	return false;
// }


/*
	@brief 将压缩的zip文件上传到ftp服务器
	@param [in/out]压缩的zip文件路径
	*/
// bool CTaskQuickPhoto::SaveZipToServer(CString &strFilePath)
// {
// 	CFTP ftp;
// 	CString strRemotePath = _T("/QuickPhoto/Zip/");
// 	CString strTemp = PathFindFileName(strFilePath);
// 	strRemotePath.Append(strTemp);
// 
// 	BOOL bRes = FALSE;
// 
// 	if (PathFileExists(strFilePath))
// 	{
// 		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp上传模式上传zip包"));
// 
// 		if (!ftp.Connect(m_ftpLoginInfo))
// 		{
// 			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("连接ftp服务器失败！错误信息为：%s"), ftp.GetErrorMessage());
// 			return false;
// 		}
// 		
// 		//判断是否使用断点续传
// 		if (m_bUploadZipFromBreak)
// 		{
// 			bRes = ftp.UpLoadFileByte(strRemotePath, strFilePath);
// 		}
// 		else
// 		{
// 			bRes = ftp.UpLoadFile(strRemotePath, strFilePath);
// 		}
// 		
// 
// 		if (bRes)
// 		{
// 			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp上传zip包成功, 路径为:%s"), strFilePath.GetString());
// 			return true;
// 		}
// 		else
// 		{	
// 			CString strLastErrorMsg = ftp.GetErrorMessage();
// 			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("ftp上传zip包失败, 路径为:%s,错误信息为:%s"), strFilePath.GetString(), strLastErrorMsg);
// 		}	
// 	}
// 	else
// 	{
// 		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("上传zip包路径不存在, 路径为:%s"), strFilePath.GetString());
// 	}
// 
// 	return false;
// }


void CTaskQuickPhoto::GetUploadFilePath(CString &strUploadPath, CString strMode)
{
	CString strMacAddr = _T("");
	CString strZipName = _T("");
	
	TCHAR szUploadPath[MAX_PATH] = {0};

	PathAppend(szUploadPath, g_pGlobalData->dir.GetInstallDir());
	PathAppend(szUploadPath, _T("\\image\\"));

	if (strMode == _T("Zip"))
	{
		CTime time;
		time = CTime::GetCurrentTime();
		CString strCurTime = _T("");
		strCurTime.Format(_T("%02d-%02d-%02d"), time.GetHour(), time.GetMinute(), time.GetSecond());
		strMacAddr = GetPhysicalAddress();
		strZipName.Format(_T("%s_%s.zip"), strMacAddr.GetString(), strCurTime.GetString());
		PathAppend(szUploadPath, strZipName);
	}
	else if (strMode == _T("File"))
	{
		PathAppend(szUploadPath, _T("keyword"));
	}
	
	strUploadPath = szUploadPath;
}


// 判断是否需要发送通告请求任务
bool CTaskQuickPhoto::IsSendNitify()
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
bool CTaskQuickPhoto::IsTimeOut()
{
// 	static bool bchange = false;
// 	if (!bchange)
// 	{
// 		bchange = true;
// 		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("等到超时时间为%d分钟"), m_pYunTaskStreamCtr->GetOutTime(0));
// 	}

	DWORD dwCheckTime = GetTickCount();
	if ((dwCheckTime - m_dwExecTime) > m_pYunTaskStreamCtr->GetOutTime(0)* 60 *1000)
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

void CTaskQuickPhoto::WaitForRunningEvent()
{
	::WaitForSingleObject(m_hRunning, INFINITE);
	::ResetEvent(m_hRunning);
}

void CTaskQuickPhoto::SetEvent()
{
	::SetEvent(m_hRunning);
}

BOOL CTaskQuickPhoto::IsRunningSendResult()
{
	if (::WaitForSingleObject(m_hRunning, 0) == WAIT_OBJECT_0)
	{	
		return TRUE;
	}

	return FALSE;
}

bool CTaskQuickPhoto::GetTimeOutFlag()
{
	return m_bIsTaskTimeOut;
}

int CTaskQuickPhoto::GetCurrTaskType()
{
	return g_TypeMap[m_tTaskData.strType];
}

bool CTaskQuickPhoto::CheckUpdateRankModule(int nWaitTime)
{
	static int iInterval = 20;
	DWORD dwStartTime = GetTickCount();
	HANDLE hMcMutex;
	do
	{
		Sleep(iInterval);
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		hMcMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T("_UpdateRank_Yun_"));
		if (hMcMutex != NULL)
		{
			CloseHandle(hMcMutex);
			hMcMutex = NULL;
			return TRUE;
		}

	} while (GetTickCount() - dwStartTime < nWaitTime);

	return FALSE;
}

