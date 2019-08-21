#include "StdAfx.h"
#include "TaskThread.h"
#include "YunTaskStreamCtr.h"
#include "FileReadAndSave.h"
#include <Windows.h>
#include <TlHelp32.h>

CTaskThread::CTaskThread(void)
	:IThreadUnit(E_THREAD_YUN_TASK, 0xFFFF)
{
	m_pTaskCtr = NULL;
	m_vstrUserName.clear();
	m_strUseName = _T("");
	m_strUseNameBakeup = _T("");
	m_nVersionId = 0;
	m_dwStartTime = 0;
	m_pUploadFile = NULL;
	m_hThread1 = NULL;
	m_hThread2 = NULL;
	m_hThread3 = NULL;
	m_hThread4 = NULL;
	m_hThread5 = NULL;
	m_hThread6 = NULL;
	m_bFirstTask = true;
	m_bCatchDir = FALSE;
	m_vFindFileName.clear();

	HANDLE hThread = CreateThread(NULL, 0, CTaskThread::MessageThread, this, 0, &m_dwThreadId);
	if (hThread)
	{
		CloseHandle(hThread);
	}
}


CTaskThread::~CTaskThread(void)
{
	if (m_pTaskCtr != NULL)
	{
		delete m_pTaskCtr;
		m_pTaskCtr = NULL;
	}
	if (m_hThread1)
	{
		CloseHandle(m_hThread1);
	}
	if (m_hThread2)
	{
		CloseHandle(m_hThread2);
	}
	if (m_hThread3)
	{
		CloseHandle(m_hThread3);
	}
	if (m_hThread4)
	{
		CloseHandle(m_hThread4);
	}
	if (m_hThread5)
	{
		CloseHandle(m_hThread5);
	}
	if (m_hThread6)
	{
		CloseHandle(m_hThread6);
	}
}


DWORD CTaskThread::DispatchMessage(T_Message *pMsg)
{
	switch (pMsg->dwMsg)
	{
	case MSG_YUN_TASK:
	    {
			//::MessageBox(NULL, _T("收到云任务消息"), _T("OK"), MB_OK);
		    // 除了第一次以后收到就丢弃
			if (m_bFirstTask)
			{
				T_Message *tMsg = IMsgQueue::New_Message();

				tMsg->dwDestWork = pMsg->dwDestWork;
				tMsg->dwSourWork = pMsg->dwSourWork;
				tMsg->dwMsg = pMsg->dwMsg;
			    tMsg->wParam = pMsg->wParam;
				tMsg->lParam = pMsg->lParam;

				PassInfo* pInfo = (PassInfo*)(pMsg->lParam);

				if (!PostThreadMessage(m_dwThreadId, pMsg->dwMsg, 0, (LPARAM)tMsg))
				{
					DWORD dwError = GetLastError();
					g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("PostThreadMessage失败！Err: %d"), dwError);
				}
				else
				{
					m_bFirstTask = false;
					g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("PostThreadMessage成功,用户名:%s,版本ID:%d"), pInfo->strUserName, (DWORD)(pMsg->wParam));
				}
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
			if (m_pTaskCtr != NULL)
			{
				m_pTaskCtr->StopTask();
			}

			//将云任务控制标志还原
			m_bFirstTask = true;
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Task程序已安全退出！"));
		}
		break;

	default:
		break;
	}

	return 0;
}


void CTaskThread::OnNotifyUI(DWORD flag, DWORD dwMsg, DWORD dwLparam)
{
	T_Message *pMsg = IMsgQueue::New_Message();
	pMsg->dwDestWork = E_THREAD_TYPE_UIHELPER;
	pMsg->dwSourWork = E_THREAD_YUN_TASK;
	pMsg->dwMsg = dwMsg;
	pMsg->wParam = flag;
	pMsg->lParam = dwLparam;

	PostMessage(pMsg);
}

// 处理云任务
void CTaskThread::HandleYunTask(T_Message *pMsg /* = NULL */)
{
	if (pMsg != NULL)
	{
		PassInfo* pStr = (PassInfo*)(pMsg->lParam);
		m_strUseName = pStr->strUserName;
		m_nVersionId = (int)(pMsg->wParam);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到的版本号为：%d，用户名:%s"), m_nVersionId, m_strUseName);
	}

	if (m_pTaskCtr == NULL)
	{
		m_pTaskCtr = new CYunTaskStreamCtr(this);
	}
	m_pTaskCtr->SetUserInfo(m_strUseName, m_nVersionId);

	m_pTaskCtr->StartExecYunTaskRequest();


	if (pMsg != NULL)
	{
		IMsgQueue::Free_Message(pMsg);
	}
}

//通过udp和服务端互通消息
void CTaskThread::HandleConnServWithUdp(T_Message *pMsg)
{
	m_strSendData = _T("");
	m_strSendData = CombineSendData(pMsg);

	HANDLE hThread = CreateThread(NULL, 0, CTaskThread::ConnectServWithUdpThread, this, 0, NULL);
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
void CTaskThread::HandleSubmitLog(T_Message *pMsg)
{
	DWORD dwFlag = (DWORD)pMsg->wParam;
	PassInfo *pInfo = (PassInfo*)pMsg->lParam;

	m_strSubmitLogUseName = _T("");
	m_strSubmitLogUseName = pInfo->strUserName;
	m_strSubmitLogUseName = m_pTaskCtr->DecodeString(m_strSubmitLogUseName);

	if (dwFlag == 0)
	{
		DWORD dwExitCode; 
		GetExitCodeThread(m_hThread1,&dwExitCode);
		//表示当前线程正在运行
		if (dwExitCode == STILL_ACTIVE)
		{	
			return;
		}

		m_hThread1 = CreateThread(NULL, 0, CTaskThread::SubmitLogThread, this, 0, NULL);
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

		m_hThread2 = CreateThread(NULL, 0, CTaskThread::SubmitLogAndQuickPhotoThread, this, 0, NULL);
	}
	else if (dwFlag == 2)
	{
		DWORD dwExitCode;
		GetExitCodeThread(m_hThread3, &dwExitCode);
		//表示当前线程正在运行
		if (dwExitCode == STILL_ACTIVE)
		{
			return;
		}

		m_hThread3 = CreateThread(NULL, 0, CTaskThread::SubmitLodAndErrorCodeThread, this, 0, NULL);
	}
	else if (dwFlag == 3)
	{
		DWORD dwExitCode;
		GetExitCodeThread(m_hThread4, &dwExitCode);
		//表示当前线程正在运行
		if (dwExitCode == STILL_ACTIVE)
		{
			return;
		}

		m_hThread4 = CreateThread(NULL, 0, CTaskThread::SubmitBackLog, this, 0, NULL);
	}
	else if (dwFlag == 4)
	{
		DWORD dwExitCode;
		GetExitCodeThread(m_hThread5, &dwExitCode);
		if (dwExitCode == STILL_ACTIVE)
		{
			return;
		}

		m_hThread5 = CreateThread(NULL, 0, CTaskThread::SubmitDirDetail, this, 0, NULL);
	}
	else if (dwFlag == 5)
	{
		DWORD dwExitCode;
		GetExitCodeThread(m_hThread6, &dwExitCode);
		if (dwExitCode == STILL_ACTIVE)
		{
			return;
		}

		m_hThread6 = CreateThread(NULL, 0, CTaskThread::SubmitSpecilePath, this, 0, NULL);
	}
}


//与服务端交互的线程
DWORD WINAPI CTaskThread::ConnectServWithUdpThread(LPVOID lpParameter)
{
	CTaskThread* pThis = (CTaskThread*)lpParameter;

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

//提交指定路径文件
DWORD WINAPI CTaskThread::SubmitSpecilePath(LPVOID lpParameter)
{
	CTaskThread* pThis = (CTaskThread*)lpParameter;

	pThis->GetFtpLoginInfo();
	CString strUploadZip = _T("");
	CString strIniPath = _T("");
	CString strTmp = _T("");
	vector<LPCTSTR> vSubmitDir;
	vector<CStdString> vTmpDir;	
/*
	strFilePath.Format(_T("%s\\..\\..\\Sunqt"),g_pGlobalData->dir.GetInstallDir());
	if (!PathFileExists(strFilePath))
	{
		return 0;
	}*/
	
	strIniPath.Format(_T("%s\\data2\\YunTask.dat"), g_pGlobalData->dir.GetInstallDir());
	IXMLRW xml;
	xml.init(strIniPath);
	xml.ReadString(_T("YunTask"), _T("Submitdir"), _T("path"), strTmp);
	//多个路径之间用，分隔
	CStdStrUtils strUils;
	strUils.SplitStringEx((CStdString)strTmp, _T(","), vTmpDir, false);
	for (int i = 0; i < vTmpDir.size(); i++)
	{		
		vSubmitDir.push_back(vTmpDir[i]);
	}
	
	if (vSubmitDir.size() <= 0)
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("没有配置需要读取的文件目录;"));
		return 1;
	}

	strUploadZip.Format(_T("%s\\SpecileFile_%s_%s.zip"), g_pGlobalData->dir.GetInstallDir(), pThis->m_strSubmitLogUseName, GetPhysicalAddress());

	if (ZipUtils::CompressDirsToZip(strUploadZip.GetBuffer(),vSubmitDir) != 0)
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("压缩指定数据文件失败， err：%d"), GetLastError());
		return 1;
	}

	if (pThis->SumbitZipToServer(strUploadZip))
	{
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交压缩指定数据文件成功"));
		int nRet = DeleteFile(strUploadZip);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("删除log日志zip文件失败！Err:%d"), GetLastError());
		}
	}

	return 0;
}

//提交目录的详细信息；
DWORD WINAPI CTaskThread::SubmitDirDetail(LPVOID lpParameter)
{
	CTaskThread* pThis = (CTaskThread*)lpParameter;

	pThis->GetFtpLoginInfo();
	CString strCompName = _T("NoName");
	CString strFilePath = _T("");
	//首先判断客户有没有安装Sunqt软件
	strFilePath.Format(_T("%s\\..\\..\\Sunqt"), g_pGlobalData->dir.GetInstallDir());
	if (!PathFileExists(strFilePath))
	{
		return 0;
	}

	//搜索整个硬盘检查路径

	pThis->SearchAllDrive(_T(".sln"));

	CString strLogPath = _T("");
	CString strLogZip = _T("");
	strLogPath.Format(_T("%s\\log\\SubmitQ.log"), g_pGlobalData->dir.GetInstallDir());

	if (!PathFileExists(strLogPath))
	{
		return 0;
	}
	TCHAR szUser[128];
	DWORD cbUser = 128;
	if (GetComputerName(szUser, &cbUser))		
		strCompName = szUser;
	strLogZip.Format(_T("%s\\SubmitQ_%s_%s_%s.zip"), g_pGlobalData->dir.GetInstallDir(),strCompName, pThis->m_strSubmitLogUseName, GetPhysicalAddress());

	if (ZipUtils::CompressFileToZip(strLogPath.GetBuffer(),strLogZip.GetBuffer()) != 0)
	{
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("压缩SubmitQ日志文件失败， err：%d"), GetLastError());
		return 1;
	}

	if (pThis->SumbitZipToServer(strLogZip))
	{
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交SubmitQ日志文件成功"));

		int nRet = DeleteFile(strLogPath);
		nRet = DeleteFile(strLogZip);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("删除SubmitQ日志zip文件失败！Err:%d"), GetLastError());
		}
	}
	return 0;
}

//提交日志的线程
DWORD WINAPI CTaskThread::SubmitLogThread(LPVOID lpParameter)
{
	CTaskThread* pThis = (CTaskThread*)lpParameter;

	CReg reg;
	pThis->GetFtpLoginInfo();
	CString strLogPath  = _T("");
	CString strLogZipPath = _T("");
	CString strVersion = _T("");
	CString strIniPath = _T("");
	
	strVersion = (TCHAR*)reg.ReadValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ\\Setup"), REG_KEY_VERSION);
	strLogPath.Format(_T("%s\\log"), g_pGlobalData->dir.GetInstallDir());
	strIniPath.Format(_T("%s\\data2\\YunTask.dat"), g_pGlobalData->dir.GetInstallDir());

	strLogZipPath.Format(_T("%s\\log_%s_%s_%s.zip"), g_pGlobalData->dir.GetInstallDir(), pThis->m_strSubmitLogUseName, strVersion, GetPhysicalAddress());

	//压缩快照文件
	if (ZipUtils::CompressDirToZip(strLogPath.GetBuffer(), strLogZipPath.GetBuffer()) != 0)
	{	
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("压缩log日志文件失败， err：%d"), GetLastError());

		return 1;
	}

	if (pThis->SumbitZipToServer(strLogZipPath))
	{	
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("1"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("1"), strIniPath);
		
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("提交log日志文件成功"));

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("删除log日志zip文件失败！Err:%d"), GetLastError());
		}
	}
	else
	{	
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);

		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("提交log日志文件失败， err：%d"), GetLastError());

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("删除log日志zip文件失败！Err:%d"), GetLastError());
		}
	}

	return 0;
}

DWORD WINAPI CTaskThread::SubmitBackLog(LPVOID lpParameter)
{	
	CTaskThread* pThis = (CTaskThread*)lpParameter;

	CReg reg;
	pThis->GetFtpLoginInfo();
	CString strLogPath = _T("");
	CString strLogZipPath = _T("");
	CString strVersion = _T("");
	CString strIniPath = _T("");

	strVersion = (TCHAR*)reg.ReadValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ\\Setup"), REG_KEY_VERSION);
	strIniPath.Format(_T("%s\\data2\\YunTask.dat"), g_pGlobalData->dir.GetInstallDir());

	CString strInstallDir = g_pGlobalData->dir.GetInstallDir();
	CString strPreDir = strInstallDir.Mid(0, strInstallDir.ReverseFind('\\'));

	strLogPath.Format(_T("%s\\bak\\masterz"), strPreDir);
	if (!PathFileExists(strLogPath))
	{	
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("BackLog目录不存在!path:%s"), strLogPath);
		return 1;
	}

	strLogZipPath.Format(_T("%s\\BackLog_%s_%s_%s.zip"), g_pGlobalData->dir.GetInstallDir(), pThis->m_strSubmitLogUseName, strVersion, GetPhysicalAddress());

	//压缩快照文件
	if (ZipUtils::CompressDirToZip(strLogPath.GetBuffer(), strLogZipPath.GetBuffer()) != 0)
	{
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"));

		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("压缩BackLog日志文件失败， err：%d"), GetLastError());

		return 1;
	}

	if (pThis->SumbitZipToServer(strLogZipPath))
	{
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("1"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("1"), strIniPath);

		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交BackLog日志文件成功"));

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("删除BackLog日志zip文件失败！Err:%d"), GetLastError());
		}
	}
	else
	{
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);

		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("提交BackLog日志文件失败， err：%d"), GetLastError());

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("删除BackLog日志zip文件失败！Err:%d"), GetLastError());
		}
	}

	return 0;
}

//提交日志及快照的线程
DWORD WINAPI CTaskThread::SubmitLogAndQuickPhotoThread(LPVOID lpParameter)
{
	CTaskThread* pThis = (CTaskThread*)lpParameter;

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
	strIniPath.Format(_T("%s\\data2\\YunTask.dat"), g_pGlobalData->dir.GetInstallDir());

	strLogZipPath.Format(_T("%s\\logAndQuickPhoto_%s_%s_%s.zip"), g_pGlobalData->dir.GetInstallDir(), pThis->m_strSubmitLogUseName, strVersion, GetPhysicalAddress());

	vector<LPCTSTR> vLogAndQuickPthotPath;
	vLogAndQuickPthotPath.push_back(strLogPath);
	vLogAndQuickPthotPath.push_back(strQuickPhoto);

	//压缩日志快照文件
	if (ZipUtils::CompressDirsToZip(strLogZipPath.GetBuffer(), vLogAndQuickPthotPath) != 0)
	{	
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("压缩log日志及快照文件失败， err：%d"), GetLastError());

		return 1;
	}

	//提交日志快照压缩包
	if (pThis->SumbitZipToServer(strLogZipPath))
	{	
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("1"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("1"), strIniPath);

		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("提交log日志及快照文件成功"));

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("删除log日志及快照zip文件失败！Err:%d"), GetLastError());
		}
	}
	else
	{	
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);

		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("提交log日志及快照文件失败， err：%d"), GetLastError());

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__,_T("删除log日志及快照zip文件失败！Err:%d"), GetLastError());
		}
	}

	return 0;
}

DWORD WINAPI CTaskThread::SubmitLodAndErrorCodeThread(LPVOID lpParameter)
{
	CTaskThread* pThis = (CTaskThread*)lpParameter;

	CReg reg;
	pThis->GetFtpLoginInfo();
	CString strLogPath = _T("");
	CString strQuickPhoto = _T("");
	CString strLogZipPath = _T("");
	CString strVersion = _T("");
	CString strIniPath = _T("");

	strVersion = (TCHAR*)reg.ReadValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ\\Setup"), REG_KEY_VERSION);
	strLogPath.Format(_T("%s\\log\\SVCAR_RESULT.log"), g_pGlobalData->dir.GetInstallDir());
	strQuickPhoto.Format(_T("%s\\image\\errorcode"), g_pGlobalData->dir.GetInstallDir());

	if (PathFileExists(strQuickPhoto))
	{
		CopyFile(strLogPath, strQuickPhoto + _T("\\SVCAR_RESULT.log"), FALSE);
	}
	else
	{
		return 0;
	}
	
	strIniPath.Format(_T("%s\\data2\\YunTask.dat"), g_pGlobalData->dir.GetInstallDir());
	strLogZipPath.Format(_T("%s\\logAndErrorCode_%s_%s_%s.zip"), g_pGlobalData->dir.GetInstallDir(), pThis->m_strSubmitLogUseName, strVersion, GetPhysicalAddress());

	vector<LPCTSTR> vLogAndQuickPthotPath;
	vLogAndQuickPthotPath.push_back(strQuickPhoto);

	//压缩日志快照文件
	if (ZipUtils::CompressDirsToZip(strLogZipPath.GetBuffer(), vLogAndQuickPthotPath) != 0)
	{
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);
		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("压缩log日志及快照文件失败， err：%d"), GetLastError());

		return 1;
	}

	//提交日志快照压缩包
	if (pThis->SumbitZipToServer(strLogZipPath))
	{
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("1"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("1"), strIniPath);

		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交log日志及快照文件成功"));

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("删除log日志及快照zip文件失败！Err:%d"), GetLastError());
		}
	}
	else
	{
		IXMLRW xml;
		xml.init(strIniPath);
		xml.WriteString(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"));

		//WritePrivateProfileString(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), _T("0"), strIniPath);

		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("提交log日志及快照文件失败， err：%d"), GetLastError());

		int nRet = DeleteFile(strLogZipPath);
		if (nRet == 0)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("删除log日志及快照zip文件失败！Err:%d"), GetLastError());
		}
	}


	return 0;
}

//循环消息队列
DWORD WINAPI CTaskThread::MessageThread(LPVOID lpParameter)
{
	CTaskThread* pThis = (CTaskThread*)lpParameter;
	MSG msg;

	while (GetMessage(&msg, 0, 0, 0))
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("队列线程接收到的消息：%d"), msg.message);
		switch (msg.message)
		{
		case MSG_YUN_TASK:
		    {
				T_Message *pMsg = (T_Message *)msg.lParam;
				pThis->HandleYunTask(pMsg);
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

//合并发送给服务端的数据
const CString CTaskThread::CombineSendData(T_Message *tMsg)
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
	strUseName = m_pTaskCtr->DecodeString(strUseName);
	//nPtrId = pInfo->iProductId;

	CReg reg;
	strVersion = (TCHAR*)reg.ReadValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ\\Setup"), REG_KEY_VERSION);

	strIniPath.Format(_T("%s\\data2\\YunTask.dat"), g_pGlobalData->dir.GetInstallDir());
	IXMLRW xml;
	xml.init(strIniPath);
	xml.ReadInt(_T("YunTask"), _T("ConnectUseUdp"), _T("UploadLogSuccFlag"), nUploadSuccFlag);

	//nUploadSuccFlag = GetPrivateProfileInt(_T("ConnectUseUdp"), _T("UploadLogSuccFlag"), 0, strIniPath);

	//发送消息格式为：用户名|产品ID|版本号|MAC地址|提交日志成功标记
	strSendInfo.Format(_T("%s|%s|%s|%s|%d"), strUseName, strPrtId, strVersion, GetPhysicalAddress(), nUploadSuccFlag);

	return strSendInfo;
}


/*
	@brief 将压缩的zip文件上传到ftp服务器
	@param [in/out]压缩的zip文件路径
	*/
bool CTaskThread::SumbitZipToServer(CString &strFilePath)
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
void CTaskThread::GetFtpLoginInfo(void)
{
	TCHAR szIp[100];
	TCHAR szUserName[100];
	TCHAR szPwd[100];
	CString szInfo = _T("");
	CString strInitPath = _T("");

	int   iPort;
	IKeyRW key;
	key.InitDll(KTYPE_CFG);

	memset(szIp, 0, sizeof(szIp));
	memset(szUserName, 0, sizeof(szUserName));
	memset(szPwd, 0, sizeof(szPwd));

	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());

	IXMLRW xml;
	xml.init(strInitPath);
	xml.ReadString(_T("MC"), _T("dump"), _T("host"), szInfo);

	CString strDecrypt;
	key.DecryptData(szInfo, DECRYPTCFG, strDecrypt);

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

void CTaskThread::GetDesCode(TCHAR *code,TCHAR *ip,TCHAR *account,TCHAR *psd,int &port)
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

//递归目录
BOOL CTaskThread::RecuscivPath(const CString& strFilePath, CString strExpansion)
{
	CFileFind file;	
	//如果文件包含反斜杠
	CString strFile = strFilePath;
	if (strFile.Right(1) == _T("\\") ||
		strFile.Right(1) == _T("/"))
	{
		strFile = strFile.Left(strFile.GetLength() - 1);
	}

	BOOL bEnd = file.FindFile(strFile + _T("\\*.*"));

	while (bEnd) //每次循环对应一个目录
	{
		bEnd = file.FindNextFile();
		//如果是目录就循环调用；
		if (file.IsDots() || file.IsHidden())
		{
			continue;
		}
		else if (file.IsDirectory()) 
		{			
			RecuscivPath(file.GetFilePath(),strExpansion);
		}
		else 
		{
			strFile = file.GetFilePath();
			if (strFile.Find(strExpansion) != -1)
			{
				g_SubmitLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("文件为：%s"), file.GetFilePath());				
			}			
		}

		Sleep(5);
	}

	return TRUE;
}

//查找所有硬盘下路径
BOOL CTaskThread::SearchAllDrive(CString strExpansion)
{
	//先获取硬盘
	TCHAR szDrive[MAX_PATH];
	GetLogicalDriveStrings(MAX_PATH, szDrive);

	//循环列表查询
	TCHAR* pDrive = szDrive;
	int iDrvCnt = 0;
	//计算有几个驱动盘
	while (*pDrive)
	{
		iDrvCnt++;
		pDrive += wcslen(pDrive) + 1;		
	}
	pDrive = szDrive;
	//先重下一个D盘找起；
	if (iDrvCnt > 1)
		pDrive += wcslen(pDrive) + 1;
	while (*pDrive)
	{	
		RecuscivPath(pDrive, strExpansion);
		pDrive += wcslen(pDrive) + 1;
	}

	return TRUE;
}

BOOL CTaskThread::FindMainFile(const CString& strFilePath)
{	
	
	if (m_vFindFileName.size() <= 0)
	{
		return FALSE;
	}

	CString strTmp = _T("");
	CString strIniPath = _T("");
	strIniPath.Format(_T("%s\\data2\\YunTask.dat"), g_pGlobalData->dir.GetInstallDir());
	IXMLRW xml;
	xml.init(strIniPath);
	xml.ReadString(_T("YunTask"), _T("Findfile"), _T("File"), strTmp);
	//多个路径之间用，分隔
	CStdStrUtils strUils;
	strUils.SplitStringEx((CStdString)strTmp, _T(","), m_vFindFileName, false);

	for (auto Name:m_vFindFileName)
	{
		if (strFilePath.Find(*Name) != -1)
		{
			return TRUE;
		}
	}

	return FALSE;
}





