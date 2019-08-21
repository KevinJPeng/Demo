#include "StdAfx.h"
#include "ScheduleThread.h"
#include "AutoGetKeywordPages.h"
#include "FileReadAndSave.h"

extern HANDLE hProcess;
HANDLE g_hEvent;
bool g_bWebcDisconnected;  //起了主控异常断开重试一次的标记
bool g_bCaptureKeywordFailed;   //抓取关键词失败重试一次的标记    

CScheduleThread::CScheduleThread(void)
	:IThreadUnit(E_THREAD_YUN_TASK, 0xFFFF)
{
	m_pAutoGetKeywordPages = NULL;
	m_vstrUserName.clear();
	m_strUseName = _T("");
	m_strUseNameBakeup = _T("");
	m_nVersionId = 0;
	m_nRefreshFlag = 0;
	m_dwStartTime = 0;
	g_hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	g_bWebcDisconnected = false;
	g_bCaptureKeywordFailed = false;
	m_pUploadFile = NULL;
	m_hThread1 = NULL;
	m_hThread2 = NULL;

	HANDLE hThread = CreateThread(NULL, 0, CScheduleThread::MessageThread, this, 0, &m_dwThreadId);
	if (hThread)
	{
		CloseHandle(hThread);
	}
}


CScheduleThread::~CScheduleThread(void)
{
	if (m_pAutoGetKeywordPages != NULL)
	{
		delete m_pAutoGetKeywordPages;
		m_pAutoGetKeywordPages = NULL;
	}
	if (m_hThread1)
	{
		CloseHandle(m_hThread1);
	}
	if (m_hThread2)
	{
		CloseHandle(m_hThread2);
	}
}


DWORD CScheduleThread::DispatchMessage(T_Message *pMsg)
{
	switch (pMsg->dwMsg)
	{
	case MSG_CHECK_REFRESH_KEYWORD_STATA:
		{
			T_Message *tMsg = IMsgQueue::New_Message();
			tMsg->lParam = pMsg->lParam;

			DWORD  dwExitCode; 
			GetExitCodeProcess(hProcess, &dwExitCode);
			if(dwExitCode == STILL_ACTIVE) 
			{ 
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检测到主控正在刷关键词，发给送给ui！"));
				OnNotifyUI(WAPAM_AUTO_REFRESH_KEYWORD_RUNNING, MSG_CHECK_REFRESH_KEYWORD_STATA, tMsg->lParam);
			} 
			else 
			{ 
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检测到主控没有刷关键词，发给送给ui！"));
				OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH, MSG_CHECK_REFRESH_KEYWORD_STATA, tMsg->lParam);
			} 

			IMsgQueue::Free_Message(tMsg);
		}
		break;

	case MSG_AUTO_REFRESH_KEYWORD:
		{   
			T_Message *tMsg = IMsgQueue::New_Message();
			
			tMsg->dwMsg = pMsg->dwMsg;
			tMsg->wParam = pMsg->wParam;
			tMsg->lParam = pMsg->lParam;

			PassInfo *pInfo = (PassInfo*)pMsg->lParam;
			//对同一用户在3秒内重复请求做处理
			m_strUseName = pInfo->strUserName;
			DWORD dwTime = GetTickCount() - m_dwStartTime;
			if (m_strUseNameBakeup == m_strUseName && dwTime <= 3000)
			{
				int nRefreshFlag = 0;
				memcpy(&nRefreshFlag, (char *)&pMsg->wParam + 2, 2);

				//如果是点击刷新按钮要被丢弃也要给ui发送完成消息
				if (nRefreshFlag == 1)
				{
					OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH, MSG_AUTO_REFRESH_KEYWORD);
				}

				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("该用户已存在且在三秒内请求刷排名，丢弃！"));
				return 0;
			}
			else
			{
				m_strUseNameBakeup = m_strUseName;
				m_dwStartTime = GetTickCount();
			}

			if (!PostThreadMessage(m_dwThreadId, pMsg->dwMsg, 0, (LPARAM)tMsg))
			{
				DWORD dwError = GetLastError();
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("PostThreadMessage失败！Err: %d"), dwError);
			}		
		}
		break;

	case MSG_CONNECT_SERV_WITH_UDP:
	case MSG_SUBMIT_LOG:
		{
			T_Message *tMsg = IMsgQueue::New_Message();

			tMsg->dwMsg = pMsg->dwMsg;
			tMsg->wParam = pMsg->wParam;
			tMsg->lParam = pMsg->lParam;

			if (!PostThreadMessage(m_dwThreadId, pMsg->dwMsg, 0, (LPARAM)tMsg))
			{
				DWORD dwError = GetLastError();
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("PostThreadMessage失败！Err: %d"), dwError);
			}	
		}
		break;


	case MSG_SALF_EXIT:
		{
			if (m_pAutoGetKeywordPages != NULL)
			{
				m_pAutoGetKeywordPages->StopTask();
			}
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Schedule程序已安全退出！"));
		}
		break;

	default:
		break;
	}

	return 0;
}


void CScheduleThread::OnNotifyUI(DWORD flag, DWORD dwMsg, DWORD dwLparam)
{
	T_Message *pMsg = IMsgQueue::New_Message();
	pMsg->dwDestWork = E_THREAD_TYPE_UIHELPER;
	pMsg->dwSourWork = E_THREAD_YUN_TASK;
	pMsg->dwMsg = dwMsg;
	pMsg->wParam = flag;
	pMsg->lParam = dwLparam;

	PostMessage(pMsg);
}


//处理刷关键词排名
void CScheduleThread::HandleRefreshKeyword(T_Message *pMsg)
{
	m_nRefreshFlag = 0;

	if (pMsg != NULL)
	{
		PassInfo *pInfo = (PassInfo*)pMsg->lParam;
		m_strUseName = pInfo->strUserName;

		//添加立即刷新关键词排名按钮，wparam前两个字节为版本号，后两个字节为标志
		m_nVersionId = 0;
		m_nRefreshFlag = 0;
		memcpy(&m_nVersionId, &pMsg->wParam, 2);
		memcpy(&m_nRefreshFlag, (char *)&pMsg->wParam + 2, 2);	

		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到的版本号为：%d，是否是点击立即刷新按钮标志：%d"), m_nVersionId, m_nRefreshFlag);
	}
	
	//判断是否是同一用户
	//vector <CString>::iterator iter = m_vstrUserName.begin();
	//for (; iter != m_vstrUserName.end(); ++iter)
	//{
	//	if (m_strUseName == *iter)
	//	{
	//		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("该用户已存在！"));
	//		return;
	//	}	
	//}
	//m_vstrUserName.clear();
	//m_vstrUserName.push_back(m_strUseName);

	//防止切换账号起两个主控问题
	while (WaitForSingleObject(g_hEvent, INFINITE) != WAIT_OBJECT_0)
	{
		Sleep(100);
	}
	ResetEvent(g_hEvent);

	if (m_pAutoGetKeywordPages != NULL)
	{
		m_pAutoGetKeywordPages->StopTask();
	}
	else
	{
		m_pAutoGetKeywordPages = new CAutoGetKeywordPages(this);
	}
// 	vector <CString>::iterator iter = m_vstrUserName.begin();
// 	for (; iter != m_vstrUserName.end(); ++iter)
// 	{
// 		if (m_strUseName == *iter)
// 		{
// 			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("该用户已存在！"));
// 			return;
// 		}	
// 	}
// 	m_vstrUserName.clear();
// 	m_vstrUserName.push_back(m_strUseName);
// 	if (m_pAutoGetKeywordPages != NULL)
// 	{
// 		while (!m_pAutoGetKeywordPages->m_bAlreadySendDataToProc)
// 		{
// 			Sleep(100);
// 		}
// 		delete m_pAutoGetKeywordPages;
// 		m_pAutoGetKeywordPages = NULL;
// 		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("当前任务强制关闭！"));
// 	}


// 	if (m_pAutoGetKeywordPages == NULL)
// 	{
// 		m_pAutoGetKeywordPages = new CAutoGetKeywordPages(this);
// 	}

//	m_pAutoGetKeywordPages = new CAutoGetKeywordPages(this);
	//m_pAutoGetKeywordPages->StartExec(m_strUseName, m_nVersionId);
	HANDLE hThread = CreateThread(NULL, 0, CScheduleThread::StartExecThread, this, 0, NULL);
	if (hThread)
	{
		CloseHandle(hThread);
	}

	if (pMsg != NULL)
	{
		IMsgQueue::Free_Message(pMsg);
	}
}

//通过udp和服务端互通消息
void CScheduleThread::HandleConnServWithUdp(T_Message *pMsg)
{
	m_strSendData = _T("");
	m_strSendData = CombineSendData(pMsg);

	HANDLE hThread = CreateThread(NULL, 0, CScheduleThread::ConnectServWithUdpThread, this, 0, NULL);
	if (hThread)
	{
		CloseHandle(hThread);
	}

	if (pMsg != NULL)
	{
		IMsgQueue::Free_Message(pMsg);
	}
}


//提交压缩的日志zip文件
void CScheduleThread::HandleSubmitLog(T_Message *pMsg)
{
	DWORD dwFlag = (DWORD)pMsg->wParam;
	PassInfo *pInfo = (PassInfo*)pMsg->lParam;
	m_strSubmitLogUseName = _T("");
	m_strSubmitLogUseName = pInfo->strUserName;
	m_strSubmitLogUseName = m_pAutoGetKeywordPages->DecodeString(m_strSubmitLogUseName);

	if (dwFlag == 0)
	{
		DWORD dwExitCode; 
		GetExitCodeThread(m_hThread1,&dwExitCode);
		//表示当前线程正在运行
		if (dwExitCode == STILL_ACTIVE)
		{	
			return;
		}

		m_hThread1 = CreateThread(NULL, 0, CScheduleThread::SubmitLogThread, this, 0, NULL);
	}
	else if (dwFlag == 1)
	{
		DWORD dwExitCode; 
		GetExitCodeThread(m_hThread2,&dwExitCode);
		//表示当前线程正在运行
		if (dwExitCode == STILL_ACTIVE)
		{	
			return;
		}

		m_hThread2 = CreateThread(NULL, 0, CScheduleThread::SubmitLogAndQuickPhotoThread, this, 0, NULL);
	}
}

//刷关键词排名的线程
DWORD WINAPI CScheduleThread::StartExecThread(LPVOID lpParameter)
{
	CScheduleThread* pThis = (CScheduleThread*)lpParameter;
	
	pThis->m_pAutoGetKeywordPages->StartExec(pThis->m_strUseName, pThis->m_nVersionId, pThis->m_nRefreshFlag);

	return 0;
}


//与服务端交互的线程
DWORD WINAPI CScheduleThread::ConnectServWithUdpThread(LPVOID lpParameter)
{
	CScheduleThread* pThis = (CScheduleThread*)lpParameter;

	pThis->m_strReciveData = _T("");
	pThis->m_pUploadFile = new CUploadFileUseUdp;
	if (!pThis->m_pUploadFile->Exec(pThis->m_strSendData))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("使用udp跟服务器交互失败!"));
		return -1;
	}

	pThis->m_strReciveData = pThis->m_pUploadFile->GetReciveData();
	DWORD dwReciveData = (DWORD)pThis->m_strReciveData.GetBuffer();

	if (pThis->m_pUploadFile != NULL)
	{
		delete pThis->m_pUploadFile;
		pThis->m_pUploadFile = NULL;
	}

	pThis->OnNotifyUI(dwReciveData, MSG_CONNECT_SERV_WITH_UDP);

	return 0;
}

//提交日志的线程
DWORD WINAPI CScheduleThread::SubmitLogThread(LPVOID lpParameter)
{
	CScheduleThread* pThis = (CScheduleThread*)lpParameter;

	CReg reg;
	pThis->GetFtpLoginInfo();
	CString strLogPath  = _T("");
	CString strLogZipPath = _T("");
	CString strVersion = _T("");
	CString strIniPath = _T("");
	
	strVersion = (TCHAR*)reg.ReadValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ\\Setup"), REG_KEY_VERSION);
	strLogPath.Format(_T("%s\\log"), g_pGlobalData->dir.GetInstallDir());
	strIniPath.Format(_T("%s\\data2\\Schedule.dat"), g_pGlobalData->dir.GetInstallDir());

	strLogZipPath.Format(_T("%s\\log_%s_%s_%s.zip"), g_pGlobalData->dir.GetInstallDir(), pThis->m_strSubmitLogUseName, strVersion, GetPhysicalAddress());

	//压缩快照文件
	if (ZipUtils::CompressDirToZip(strLogPath.GetBuffer(), strLogZipPath.GetBuffer()) != 0)
	{
		WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("压缩log日志文件失败， err：%d"), GetLastError());

		return 1;
	}

	if (pThis->SumbitZipToServer(strLogZipPath))
	{
		WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("1"), strIniPath);
		
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("提交log日志文件成功"));

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("删除log日志zip文件失败！Err:%d"), GetLastError());
		}
	}
	else
	{
		WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);

		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("提交log日志文件失败， err：%d"), GetLastError());

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("删除log日志zip文件失败！Err:%d"), GetLastError());
		}
	}

	return 0;
}


//提交日志及快照的线程
DWORD WINAPI CScheduleThread::SubmitLogAndQuickPhotoThread(LPVOID lpParameter)
{
	CScheduleThread* pThis = (CScheduleThread*)lpParameter;

	CReg reg;
	pThis->GetFtpLoginInfo();
	CString strLogPath  = _T("");
	CString strQuickPhoto = _T("");
	CString strLogZipPath = _T("");
	CString strVersion = _T("");
	CString strIniPath = _T("");

	strVersion = (TCHAR*)reg.ReadValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ\\Setup"), REG_KEY_VERSION);
	strLogPath.Format(_T("%s\\log"), g_pGlobalData->dir.GetInstallDir());
	strQuickPhoto.Format(_T("%s\\image\\result"), g_pGlobalData->dir.GetInstallDir());
	strIniPath.Format(_T("%s\\data2\\Schedule.dat"), g_pGlobalData->dir.GetInstallDir());

	strLogZipPath.Format(_T("%s\\logAndQuickPhoto_%s_%s_%s.zip"), g_pGlobalData->dir.GetInstallDir(), pThis->m_strSubmitLogUseName, strVersion, GetPhysicalAddress());

	vector<LPCTSTR> vLogAndQuickPthotPath;
	vLogAndQuickPthotPath.push_back(strLogPath);
	vLogAndQuickPthotPath.push_back(strQuickPhoto);

	//压缩日志快照文件
	if (ZipUtils::CompressDirsToZip(strLogZipPath.GetBuffer(), vLogAndQuickPthotPath) != 0)
	{
		WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("压缩log日志及快照文件失败， err：%d"), GetLastError());

		return 1;
	}

	//提交日志快照压缩包
	if (pThis->SumbitZipToServer(strLogZipPath))
	{
		WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("1"), strIniPath);

		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("提交log日志及快照文件成功"));

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("删除log日志及快照zip文件失败！Err:%d"), GetLastError());
		}
	}
	else
	{
		WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);

		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("提交log日志及快照文件失败， err：%d"), GetLastError());

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("删除log日志及快照zip文件失败！Err:%d"), GetLastError());
		}
	}

	return 0;
}

//循环消息队列
DWORD WINAPI CScheduleThread::MessageThread(LPVOID lpParameter)
{
	CScheduleThread* pThis = (CScheduleThread*)lpParameter;
	MSG msg;

	while (GetMessage(&msg, 0, 0, 0))
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("队列线程接收到的消息：%d"), msg.message);
		switch (msg.message)
		{
		case MSG_AUTO_REFRESH_KEYWORD:
			{
				T_Message *pMsg = (T_Message *)msg.lParam;

				g_bWebcDisconnected = false;
				g_bCaptureKeywordFailed = false;
				pThis->HandleRefreshKeyword(pMsg);
			}
			break;

		case MSG_CONNECT_SERV_WITH_UDP:
			{
				T_Message *pMsg = (T_Message *)msg.lParam;

				pThis->HandleConnServWithUdp(pMsg);
			}
			break;

		case MSG_SUBMIT_LOG:
			{
				T_Message *pMsg = (T_Message *)msg.lParam;

				pThis->HandleSubmitLog(pMsg);
			}
			break;

		case MSG_SALF_EXIT:
			{
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到退出消息！退出线程！"));
				goto end;
			}
			break;

		default:
			break;
		}		
	}

end:
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("退出消息队列线程！"));
	return 0;
}


/*
	@brief  取得mac地址
	@return mac地址
	*/
CString CScheduleThread::GetClientMacAddr(void)
{
	PIP_ADAPTER_ADDRESSES pIpAdapterInfo = new IP_ADAPTER_ADDRESSES();
	unsigned long stSize = sizeof(IP_ADAPTER_ADDRESSES);
	CString strMacAddr;

	int nRel = GetAdaptersAddresses(AF_INET, 0, NULL, pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		delete pIpAdapterInfo;

		//重新申请内存空间用来存储所有网卡信息
		pIpAdapterInfo = (PIP_ADAPTER_ADDRESSES)new BYTE[stSize];
		nRel = GetAdaptersAddresses(AF_INET, 0, NULL, pIpAdapterInfo, &stSize);
	}

	if (ERROR_SUCCESS == nRel)
	{
		
		for (int i = 0; i < pIpAdapterInfo->PhysicalAddressLength; i++)
		{
			strMacAddr.AppendFormat(_T("%02X-"), pIpAdapterInfo->PhysicalAddress[i]);
		}

		strMacAddr.Delete(strMacAddr.GetLength() - 1);
	
	}

	if (pIpAdapterInfo)
		delete pIpAdapterInfo;

	if (strMacAddr.IsEmpty())
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("取mac地址失败, err:%d"), GetLastError());
	}

	return strMacAddr;
}

//合并发送给服务端的数据
const CString CScheduleThread::CombineSendData(T_Message *tMsg)
{
	CString strUseName = _T("");
	//int nPtrId = 0;
	CString strPrtId = _T("ZhouDaShi");
	CString strVersion = _T("");
	CString strSendInfo = _T("");
	CString strIniPath = _T("");
	int nUploadSuccFlag = 0;

	PassInfo *pInfo = (PassInfo*)tMsg->lParam;
	strUseName = pInfo->strUserName;
	strUseName = m_pAutoGetKeywordPages->DecodeString(strUseName);
	//nPtrId = pInfo->iProductId;

	CReg reg;
	strVersion = (TCHAR*)reg.ReadValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ\\Setup"), REG_KEY_VERSION);

	strIniPath.Format(_T("%s\\data2\\Schedule.dat"), g_pGlobalData->dir.GetInstallDir());
	nUploadSuccFlag = GetPrivateProfileInt(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), 0, strIniPath);

	//发送消息格式为：用户名|产品ID|版本号|MAC地址|提交日志成功标记
	strSendInfo.Format(_T("%s|%s|%s|%s|%d"), strUseName, strPrtId, strVersion, GetPhysicalAddress(), nUploadSuccFlag);

	return strSendInfo;
}


/*
	@brief 将压缩的zip文件上传到ftp服务器
	@param [in/out]压缩的zip文件路径
	*/
bool CScheduleThread::SumbitZipToServer(CString &strFilePath)
{
	CFTP ftp;
	CString strRemotePath = _T("/log/");
	CString strTemp = PathFindFileName(strFilePath);
	strRemotePath.Append(strTemp);

	BOOL bRes = FALSE;

	if (PathFileExists(strFilePath))
	{
		if (!ftp.Connect(m_ftpLoginInfo))
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("准备上传log日志时连接ftp服务器失败！错误信息为：%s"), ftp.GetErrorMessage());
			return false;
		}

		bRes = ftp.UpLoadFileByte(strRemotePath, strFilePath);

		if (bRes)
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ftp上传log日志zip包成功, 路径为:%s"), strFilePath.GetString());
			return true;
		}
		else
		{	
			CString strLastErrorMsg = ftp.GetErrorMessage();
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("ftp上传log日志zip包失败, 路径为:%s,错误信息为:%s"), strFilePath.GetString(), strLastErrorMsg);
		}	
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("上传log日志zip包路径不存在, 路径为:%s"), strFilePath.GetString());
	}

	return false;
}


//获取登录ftp的信息
void CScheduleThread::GetFtpLoginInfo(void)
{
	TCHAR szIp[100];
	TCHAR szUserName[100];
	TCHAR szPwd[100];
	TCHAR szInfo[MAX_PATH] = {0};
	CString strInitPath = _T("");

	int   iPort;
	CDES  desDecrypt;

	memset(szIp, 0, sizeof(szIp));
	memset(szUserName, 0, sizeof(szUserName));
	memset(szPwd, 0, sizeof(szPwd));

	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());
	GetPrivateProfileString(_T("dump"), _T("host"), NULL, szInfo, MAX_PATH, strInitPath);

	CString strEncrypt = szInfo;
	CString strDecrypt;
	desDecrypt.Decrypt(strDecrypt, strEncrypt);

	if (strDecrypt.GetLength() > 0)
	{
		GetDesCode(strDecrypt.GetBuffer(0), szIp, szUserName, szPwd, iPort);
		strDecrypt.ReleaseBuffer();
	}

	m_ftpLoginInfo.strServer = szIp;
	m_ftpLoginInfo.strUser = szUserName;
	m_ftpLoginInfo.strPwd = szPwd;
	m_ftpLoginInfo.nPort = iPort;
	m_ftpLoginInfo.bPassive = TRUE;
}

void CScheduleThread::GetDesCode(TCHAR *code,TCHAR *ip,TCHAR *account,TCHAR *psd,int &port)
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
