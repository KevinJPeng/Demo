#include "stdafx.h"
#include <Wininet.h>
#include "UpdateOperate.h"
#include "..\..\common\Ping.h"
#include "..\..\common\StdStrUtils.h"
#include "md5.h"
#include <Tlhelp32.h>

BOOL g_bIsCancelUpdate = FALSE;

CUpdateOperate::CUpdateOperate(void) : IThreadUnit(E_THREAD_UPDATE,0xFFFF)
{
	m_dwUpdateFailCount = 0;
}

CUpdateOperate::~CUpdateOperate()
{
	;
}

DWORD CUpdateOperate::DispatchMessage(T_Message *pMsg)
{	
	switch (pMsg->dwMsg)
	{	
		//接收到升级消息
	case MSG_PRODUCT_UPDATE:
		{	
			OnHandleUpdateMsg(pMsg);
		}	
		break;
		//升级成功校验消息
	case MSG_UPDATE_SUCCESS_CHECK:
		{	
			OnHandleSuccessCheck(pMsg);
		}
		break;
		//取消升级
	case MSG_CANCEL_UPDATE:
		{	
			OnHandleCancelUpdate(pMsg);
		}
		break;
		//add by zhumingxing 20141124 下载安装包
	case MSG_DOWNLOAD_INSTALLEXE:
		{	
			OnHandleDownLoadInstallExe(pMsg);
		}
		break;
	case MSG_DOWNLOAD_SILENT_EXE:
		{
			 OnDownLoadExeAndSlientInstall(pMsg);
		}
		break;
		//安全退出
	case MSG_SALF_EXIT:
		{	
			OnHandleSafeExit(pMsg);
		}
		break;
	default:
		break;
	}
	return 0;
}

DWORD  CUpdateOperate::OnHandleUpdateMsg(T_Message *pMsg)
{
	//重启客户端消息
	if (pMsg->wParam == WPARAM_REBOOT_CLIENT)
	{	
		//add by zhumingxing 20140807 区分手动升级还是后台强制重启
		m_wRbootType = pMsg->lParam;

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到了用户需要重启客户端消息！"));
		HanleRebootMsg();
	}
	//升级消息包括用户手动点击升级;后台自动升级;诊断升级三种类型
	else
	{	
		m_wUpdateFlag = pMsg->wParam;
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("当前升级类型为:%d!"),m_wUpdateFlag);

		DWORD dwExitCode; 
		GetExitCodeThread(m_thread,&dwExitCode);
		//表示当前升级线程正在运行
		if (dwExitCode == STILL_ACTIVE /*|| m_dwProgeress == LPARAM_DOWNLOAD_FINISH*/)
		{	
			//表示用户之前文件已经下载完毕一直未重新启动
			/*if (m_dwProgeress == LPARAM_DOWNLOAD_FINISH)
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("上次升级文件已经下载完毕，客户端未重启，升级消息忽略!"));
			}*/
			/*else
			{*/
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("后台下载线程正在运行,丢弃升级消息!"));
			/*}*/
	
			return 0;		
		}

		//初始化相关变量
		InitalParam();					

		//接收加密字符串
		m_strEncodedata = *((CString*)(pMsg->lParam));

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("接收到的升级字符串为:%s!"),m_strEncodedata);

		//开始升级线程
		m_thread = CreateThread(NULL, 0, &CUpdateOperate::ThreadProc, this, 0, NULL );
	}	
	return 1;
}

DWORD CUpdateOperate::OnHandleSuccessCheck(T_Message *pMsg)
{	
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到了校验升级是否成功消息!"));

	//清空相关成员变量
	InitalParam();

	if (CheckUpdateSuccess())
	{	
		m_dwUpdateFailCount = 0;
		WriteCheckUpdateFailCount();

		//删除filelist_new.dat文件
		TCHAR szXMLFile[MAX_PATH] = {0};
		_sntprintf(szXMLFile, _TRUNCATE, _T("%s\\bin\\filelist_new.dat"), g_pGlobalData->dir.GetInstallDir());
		::DeleteFile(szXMLFile);

		//删除temp临时目录
		DeleteDownLoadDir();

		PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_SUCCESS_CHECK,0,RET_SUCCESS);
	}
	else
	{	
		TCHAR szXMLFile[MAX_PATH] = {0};
		_sntprintf(szXMLFile, _TRUNCATE, _T("%s\\bin\\filelist_new.dat"), g_pGlobalData->dir.GetInstallDir());
		::DeleteFile(szXMLFile);

		ReadCheckUpdateFailCount();			//获取当前校验失败次数

		if (m_dwUpdateFailCount < 1)
		{	
			++m_dwUpdateFailCount;
			WriteCheckUpdateFailCount();
			PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_SUCCESS_CHECK,0,RET_ERROR);
		}
		else
		{	
			//弹出提示框，代表可能是系统权限问题
			PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_ERROR,WPARAM_POWER_ERROR,0);
		}		
	}

	return 1;
}

DWORD CUpdateOperate::OnHandleCancelUpdate(T_Message *pMsg)
{
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到了取消升级消息！"));

	//设置取消标志为TRUE
	m_dwProgeress = 0;
	m_bIsCancelUpdate = TRUE;
	g_bIsCancelUpdate = TRUE;

	WaitForSingleObject(m_thread,INFINITE);
	CloseHandle(m_thread);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("成功结束当前升级线程！"));
	return 1;
}

DWORD CUpdateOperate::OnHandleSafeExit(T_Message *pMsg)
{
	//避免造成阻塞
	try
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到安全退出消息！"));
		g_bIsCancelUpdate = TRUE;
		m_bIsCancelUpdate = TRUE;
		m_bIsCancelDownLoad = TRUE;
		m_bIsCancelSilentInstall = TRUE;

		DWORD dwExitCode; 
		GetExitCodeThread(m_thread,&dwExitCode);
		//表示当前升级线程正在运行
		if (dwExitCode == STILL_ACTIVE)
		{	
			WaitForSingleObject(m_thread,500);
			CloseHandle(m_thread);
		}

		GetExitCodeThread(m_theadDownLoad,&dwExitCode);
		if (dwExitCode == STILL_ACTIVE)
		{	
			WaitForSingleObject(m_theadDownLoad,200);
			CloseHandle(m_theadDownLoad);
		}

		GetExitCodeThread(m_threadSilent, &dwExitCode);
		if (dwExitCode == STILL_ACTIVE)
		{
			WaitForSingleObject(m_threadSilent, 200);
			CloseHandle(m_threadSilent);
		}
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("成功安全退出升级模块线程！"));
	}
	catch (...)
	{

	}
	return 1;
}

//下载安装包处理函数
DWORD CUpdateOperate::OnHandleDownLoadInstallExe(T_Message *pMsg)
{
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到下载安装包消息！"));
	m_tPostMessage = *((DELAY_MESSAGE*)pMsg->wParam);

	//如果当前下载线程正在运行，那么就结束掉当前下载线程执行新的任务
	DWORD dwExitCode; 
	GetExitCodeThread(m_theadDownLoad,&dwExitCode);
	if (dwExitCode == STILL_ACTIVE)
	{	
		m_bIsCancelDownLoad = TRUE;
		WaitForSingleObject(m_theadDownLoad,INFINITE);
		CloseHandle(m_theadDownLoad);
	}
	//重置取消标志
	m_bIsCancelDownLoad = FALSE;

	m_theadDownLoad = CreateThread(NULL, 0, &CUpdateOperate::DownLoadInstallExe, this, 0, NULL);
	return 1;
}

//下载安装包且静默安装
DWORD CUpdateOperate::OnDownLoadExeAndSlientInstall(T_Message *pMsg)
{
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到下载安装包且静默安装消息!"));

	CString strTuiSong = *((CString*)pMsg->wParam);

	if (!GetTuiSongFromStr(strTuiSong))
	{
		return 0;
	}

	DWORD dwExitCode;
	GetExitCodeThread(m_threadSilent, &dwExitCode);
	if (dwExitCode == STILL_ACTIVE)
	{
		m_bIsCancelSilentInstall = TRUE;
		WaitForSingleObject(m_threadSilent, INFINITE);
		CloseHandle(m_threadSilent);
	}
	//重置取消标志
	m_bIsCancelSilentInstall = FALSE;

	m_threadSilent = CreateThread(NULL, 0, &CUpdateOperate::DownLoadSilentInstallExe, this, 0, NULL);
	
	return 1;
}

//下载安装包线程接口
DWORD WINAPI CUpdateOperate::DownLoadInstallExe(LPVOID lpParameter)
{
	CUpdateOperate* pThis = (CUpdateOperate*)lpParameter;
	//文件下载完之后存放在安装目录下
	CString strInstallExePath =pThis->GetDirectory(_T("%temp%"));
	strInstallExePath += _T("\\setup.exe");
	CString strMd5 = _T("");

	//如果本地已经存在此文件那么就需要比较MD5值看是否需要重新下载
	if (PathFileExists(strInstallExePath))
	{
		pThis->GetFileMD5(strInstallExePath,strMd5);

		if (!strMd5.CompareNoCase(pThis->m_tPostMessage.strMd5))
		{	
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("本地已下载安装包文件与需要下载安装包文件MD5值一致，无需重复下载！"));

			//复制安装包路径给结构体返回
			pThis->m_tPostMessage.strDetailUrl = strInstallExePath;
			pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_DOWNLOAD_INSTALLEXE,WPARAM(&pThis->m_tPostMessage),RET_SUCCESS);
			return 1;
		}
	}
	if (!pThis->DownLoadFileByURL(pThis->m_tPostMessage.strDetailUrl))
	{
		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_DOWNLOAD_INSTALLEXE,WPARAM(&pThis->m_tPostMessage),RET_ERROR);
	}
	else
	{
		//复制安装包路径给结构体返回
		pThis->m_tPostMessage.strDetailUrl = strInstallExePath;
		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_DOWNLOAD_INSTALLEXE,WPARAM(&pThis->m_tPostMessage),RET_SUCCESS);
	}
	return 1;
}

//下载静默安装包接口
DWORD WINAPI CUpdateOperate::DownLoadSilentInstallExe(LPVOID lpParameter)
{
	CUpdateOperate* pThis = (CUpdateOperate*)lpParameter;

	CString strInstallExePath = pThis->GetDirectory(_T("%temp%"));
	strInstallExePath += _T("\\MasterZ_Slient_setup.exe");
	CString strMd5 = _T("");

	//如果本地已经存在此文件那么就需要比较MD5值看是否需要重新下载
	if (PathFileExists(strInstallExePath))
	{
		pThis->GetFileMD5(strInstallExePath, strMd5);

		if (!strMd5.CompareNoCase(pThis->m_tTuiSong.m_strMd5))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("本地已下载安装包文件与需要下载安装包文件MD5值一致，无需重复下载！"));

			//复制安装包路径给结构体返回
			pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_UPDATE, MSG_DOWNLOAD_SILENT_EXE, 0, RET_SUCCESS);
			return 1;
		}
	}

	//下载安装包，支持多个URL切换
	BOOL bIsDownLoadOk = FALSE;

	for (int iindex = 0; iindex < pThis->m_tTuiSong.m_vecDownLoadUrl.size(); ++iindex)
	{	
		if (pThis->m_bIsCancelSilentInstall)
		{
			return 0;
		}
		if (!pThis->DownLoadFileByURL(pThis->m_tTuiSong.m_vecDownLoadUrl[iindex], strInstallExePath))
		{	
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("下载安装包文件失败，URL:%s！"), pThis->m_tTuiSong.m_vecDownLoadUrl[iindex]);
			continue;
		}
		else
		{
			bIsDownLoadOk = TRUE;
			break;
		}
	}

	if (bIsDownLoadOk)
	{
		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_UPDATE, MSG_DOWNLOAD_SILENT_EXE, 0, RET_SUCCESS);
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("下载安装包文件失败，错误码:%d！"), GetLastError());
		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_UPDATE, MSG_DOWNLOAD_SILENT_EXE, 0, RET_ERROR);
	}

	return 1;
}

//直接提供给外部使用的接口进行更新处理
DWORD WINAPI CUpdateOperate::ThreadProc(LPVOID lpParameter)
{	
	CUpdateOperate* pThis = (CUpdateOperate*)lpParameter;

	//升级模块功能调整,先更新本地配置文件,然后再进行客户端文件升级流程
	if (!pThis->UpdateConfigToLocal())
	{	
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("更新本地配置文件失败!"));

		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_ERROR,WPARAM_UPDATE_CONFIG_ERROR,pThis->m_wUpdateFlag);

		return 0;
	}
	//读取本地UpdateOL.dat配置
	if (!pThis->ReadUpdateOLConfig())
	{	
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("读取本地升级配置文件失败！"));

		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_ERROR,WPARAM_WINNET_ERROR,pThis->m_wUpdateFlag);

		return 0;
	}

	//拼接url来向服务器确定是否有更新出现
	CReg reg;
	CHttpUtils http;
	CString strUpdateAddr = _T(""); 

	strUpdateAddr.Format(_T("%s?msg=%s"), pThis->m_tPtdSetting.strUpdateRequestPage.GetBuffer(),pThis->m_strEncodedata.GetBuffer());

	//选择服务器
	if (!pThis->ChooseHttpServer(strUpdateAddr))
	{	
		if (pThis->m_bIsCancelUpdate)
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到了用户点击取消升级消息！"));
		}
		else
		{	
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("升级失败，无可用服务器选择！"));

			pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_ERROR,WPARAM_WINNET_ERROR,pThis->m_wUpdateFlag);
		}
		return 0;
	}

	//构造URL向服务器发送是否有下载数据请求
	CString strURL = pThis->m_szHttpServer + strUpdateAddr;
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("请求服务器URL为:%s"),strURL);

	strURL = _T("http://112.74.102.50:2015/downloadfilelist/index?msg=4pCa4pCE4pCT4pGG4pGR4pGV4pGV4pGZ4pGR4pGS4pGY4pGR4pGQ4pGU4pGG4pCT4pCa4pCX4pCU4pCa4pCK4pGS4pGQ4pGR4pGZ4pGQ4pGU4pGS4pGV4pGR4pGX4pGG4pGU4pGQ4pGN4pGY4pCk4pGN4pGV4pCj4pGN4pGR4pGS4pGN4pGV4pCi4pGN4pCh4pCk4pGG4pGR4pGO4pGQ4pGS4pGO4pGQ4pGQ4pGW4pGR");
	pThis->strSerResponse = http.GetSerRespInfo(strURL.GetBuffer());

	if (pThis->m_bIsCancelUpdate)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到了取消升级消息"));
		return 0;
	}
	//等于0 表示已经是最新版本【最新版本需要向客户端发送版本已经为最新的消息】
	if (!pThis->strSerResponse.Compare(_T("0")))
	{	
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("服务器返回字符串为:0表示为最新版本"));

		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_LATEST_VERSION,0,pThis->m_wUpdateFlag);

		return 0;
	}
	//等于-1服务器正忙
	if (!pThis->strSerResponse.Compare(_T("-1")))
	{	
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("服务器返回字符串为:-1表示为服务器正忙，拒绝连接"));

		//modefy by zhumingxing 20140520 服务器正忙
		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_ERROR,WPARAM_SERVER_BUSY_UPDATE,pThis->m_wUpdateFlag);

		return 0;
	}
	//访问升级服务器失败
	if (!pThis->strSerResponse.Compare(_T("$ERR$")))
	{	
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("服务器连接错误，返回字符串为:$ERR$"));

		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_ERROR,WPARAM_WINNET_ERROR,pThis->m_wUpdateFlag);

		return 0;
	}

	//解析失败的情况可能是返回了错误码
	if (!pThis->GetNewUpdateInfo(pThis->strSerResponse, &(pThis->m_newUpdate)))	
	{	
		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_ERROR,WPARAM_WINNET_ERROR,pThis->m_wUpdateFlag);

		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("解析服务端更新列表失败！服务器返回字符串为:%s"),pThis->strSerResponse);
		return 0;
	}

	//此处进行判断是否需要进行升级操作，如果满足一定限制条件无需升级--------20150731
	if (!pThis->m_newUpdate.szVersion.Compare(pThis->GetUserRequestVersion()))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("服务器最新版本与本地版本一致，已经是最新版本!"));
		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_UPDATE, MSG_LATEST_VERSION, 0, pThis->m_wUpdateFlag);

		return 0;
	}
	if (pThis->GetUserRequestVersion().Compare(pThis->m_newUpdate.szCmpLessVersion) < 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("客户端版本符合filelist.xml中最小版本升级限制,不升级!less_limit:%s"), pThis->m_newUpdate.szCmpLessVersion);
		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_UPDATE, MSG_LATEST_VERSION, 0, pThis->m_wUpdateFlag);

		return 0;
	}
	if (pThis->IsMatchLimitEquelVersion(pThis->GetUserRequestVersion()))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("客户端版本符合filelist.xml中等于版本升级限制,不升级!"));
		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_UPDATE, MSG_LATEST_VERSION, 0, pThis->m_wUpdateFlag);

		return 0;
	}
	//end

	//获取需要下载的文件列表URL
	if (!pThis->GetDownLoadFileList(pThis->m_newUpdate,pThis->m_vecDownLoadUrl,pThis->m_vecTempPath))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("检测到了用户取消升级消息，线程退出！"));
		return 0;
	}

	//判断是否需要更新Deamon.exe服务文件
	if (pThis->IsNeedUpdateServers())
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到本次升级需要升级Deamon.exe服务文件,写入服务结束标记!"));

		if (!reg.WriteValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ"), _T("deamonId"), pThis->GetGuidCode()))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("写入服务结束标记失败!"));
		}
		Sleep(500);
	}
	
	if (pThis->m_vecDownLoadUrl.size() == 0)
	{	
		CString strTemPath;
		strTemPath.Format(_T("%s\\temp"), g_pGlobalData->dir.GetInstallDir());

		if (PathFileExists(strTemPath))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("校验升级失败后temp目录下已经下载了所有文件，无需重复下载!"));

			//如果是自动升级，那么需要返回已经下载文件完成消息给主界面，等待主界面最小化到托盘之后气泡提示升级
			pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_PRODUCT_UPDATE,pThis->m_wUpdateFlag,LPARAM_UPDATE_SUCCESS);
		}
		else
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("与本地文件比较,没有需要更新的文件，已经是最新版本！"));
			//这种情况需要将服务器上面最新的版本号写到注册里面去
			reg.WriteValueOfKey(REG_USER_ROOT, pThis->m_tPtdSetting.strRegKey, REG_KEY_VERSION, pThis->m_newUpdate.szVersion);

			pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_LATEST_VERSION,0,pThis->m_wUpdateFlag);	
		}

		return 0;
	}
	else
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检测到有更新！总共需要下载%d个文件!"),pThis->m_vecDownLoadUrl.size());
	}
	
	//add by qy --- 2018.6.13
	BOOL bNeedUpdateService = FALSE;
	BOOL bNeedUpdateSerchPro = FALSE;
	pThis->IsNeedUpdateClickPro(bNeedUpdateService, bNeedUpdateSerchPro);
	if (bNeedUpdateService)
	{
		CString strAppPath;
		strAppPath.Format(_T("%s\\bin\\KillService.bat"), GetInstallPath());
		ShellExecute(NULL, _T("open"), strAppPath, _T(""), NULL, SW_HIDE);
	}
	if (bNeedUpdateSerchPro)
	{
		pThis->WaitProcEnd(_T("AutomaticSearch.exe"), 0, true);		
	}	
	pThis->WaitProcEnd(_T("CtrlUserTask.exe"), 0, true);
	pThis->WaitProcEnd(_T("UpdateRank.exe"), 0, true);

	BOOL bIsCheckCancelUpdate = FALSE;
	if (pThis->StartDownLoadFile(bIsCheckCancelUpdate))
	{	
		//升级流程正常走完，文件下载成功，发送消息到主界面提示立即重启
		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_PRODUCT_UPDATE,pThis->m_wUpdateFlag,LPARAM_UPDATE_SUCCESS);	
		return 1;
	}
	else
	{
		if (bIsCheckCancelUpdate)
		{
			return 0;
		}

		pThis->PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_ERROR,WPARAM_WINNET_ERROR,pThis->m_wUpdateFlag);
		return 0;
	}
}


void CUpdateOperate::InitalParam()
{	
	m_bIsCancelUpdate = FALSE;
	g_bIsCancelUpdate = FALSE;
	m_dwProgeress = 0;
	m_strbatFilePath = _T("");
	m_thread = NULL;
	m_theadDownLoad = NULL;
	m_threadSilent = NULL;
	m_tPtdSetting.clear();
	m_newUpdate.clear();
	m_vec_ExcludeCheckFile.clear();
}

BOOL CUpdateOperate::ReadUpdateOLConfig(void)
{	
	TCHAR szCfgINI[MAX_PATH] = {0};
	IXMLRW iniFile;
	CStdString strVal = _T("");

	_sntprintf(szCfgINI, _TRUNCATE, _T("%s\\data2\\UpdateOL.dat"), g_pGlobalData->dir.GetInstallDir());
	iniFile.init(szCfgINI);

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("UpdateOL.dat:%s！"), szCfgINI);

	//产品名称 用作通知升级结束时的提示标题
	iniFile.ReadString(_T("UpdateOL"),_T("product"), _T("name"), m_tPtdSetting.strName);

	//产品注册路径 用于读取产品的版本等信息
	iniFile.ReadString(_T("UpdateOL"),_T("product"), _T("regkey"), m_tPtdSetting.strRegKey);
	iniFile.ReadInt(_T("UpdateOL"),_T("product"), _T("updatesuccflag"), m_tPtdSetting.iUpdateSuccFlag);
	iniFile.ReadString(_T("UpdateOL"),_T("product"), _T("UpdateRequestPage"), m_tPtdSetting.strUpdateRequestPage);
	iniFile.ReadString(_T("UpdateOL"),_T("product"), _T("UpdateDownLoadPage"), m_tPtdSetting.strUpdateDownLoadpage);
	iniFile.ReadString(_T("UpdateOL"),_T("product"), _T("clientTool"), m_tPtdSetting.strClientTool);
	iniFile.ReadString(_T("UpdateOL"),_T("product"), _T("MsgTo"), m_tPtdSetting.strDlgToRecvMsg);

	if (m_tPtdSetting.strName == _T("") || m_tPtdSetting.strRegKey == _T("") || 
		m_tPtdSetting.iUpdateSuccFlag == 0 || m_tPtdSetting.strUpdateRequestPage == _T(""))
	{	
		//add by zhumingxing 20140415 ,如果出现错误此处应该反馈给上层何种消息
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("产品配置信息无效，请检查UpdateOL.dat！"));
		return FALSE;
	}
	return TRUE;
}

BOOL CUpdateOperate::ChooseHttpServer(const CString& strAddr)
{	
	IXMLRW xmlmc;
	IXMLRW xmlUpdate;

	CString szVal = _T("");
	TCHAR szCfgINI[MAX_PATH] = {0};
	TCHAR szUpdateOLINI [MAX_PATH] = {0};

	_sntprintf(szCfgINI, _TRUNCATE, _T("%s\\data2\\mc.dat"),g_pGlobalData->dir.GetInstallDir());
	_sntprintf(szUpdateOLINI, _TRUNCATE, _T("%s\\data2\\UpdateOL.dat"),g_pGlobalData->dir.GetInstallDir());
	xmlmc.init(szCfgINI);
	xmlUpdate.init(szUpdateOLINI);

	//获取升级服务器地址
	xmlmc.ReadString(_T("MC"), _T("InnerTest"), _T("host"), szVal);

	if (!szVal.IsEmpty())
	{
		//内部测试服务器
		m_szHttpServer.Format(_T("http://%s"), szVal);          //可配IP或域名

		CString strURL = m_szHttpServer + strAddr;
		if (IsUrlAvailable(strURL))
		{
			return TRUE;
		}
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("服务器地址无法访问:%s"),strURL);
	}	
	else
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("mc.dat->[InnerTest]->host无配置！"));
	}

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("读取UpdateOL.dat升级地址配置信息！"));

	vector<CString>vecServerHost;
	vecServerHost.clear();

	CString strInfo = _T("");
	xmlUpdate.ReadString(_T("UpdateOL"), _T("server"), _T("count"), strInfo);

	DWORD dwHostCount = _ttoi(strInfo);

	if (dwHostCount > 0)
	{	
		//获取端口号
		xmlUpdate.ReadString(_T("UpdateOL"), _T("server"), _T("port"), strInfo);
		DWORD dwHostPort = _ttoi(strInfo);

		if (dwHostPort > 0)
		{
			for (int iindex = 1; iindex <= dwHostCount; ++iindex)
			{
				CString strFormat;
				strFormat.Format(_T("host%d"),iindex);
				xmlUpdate.ReadString(_T("UpdateOL"), _T("server"), strFormat, strInfo);

				if (!strInfo.IsEmpty())
				{
					vecServerHost.push_back(strInfo);
				}
			}
			return (SelAvailableHost(vecServerHost,dwHostPort,strAddr));
		}
		else
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("UpdateOL.dat->[server]->port配置有误，升级失败返回！"));
			return FALSE;
		}	
	}
	else
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("UpdateOL.dat->[server]->count配置有误，升级失败返回！"));
		return FALSE;
	}
	//检测到了取消升级消息
	if (m_bIsCancelUpdate)
	{
		return FALSE;
	}
	return TRUE;
}

//选取可用服务器地址
BOOL CUpdateOperate::SelAvailableHost(const vector<CString>& vecHost,DWORD strHostPort,const CString& strAddr)
{
	if (vecHost.empty())
	{	
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("UpdateOL.dat->[server]->host配置有误，升级失败返回！"));
		return FALSE;
	}
	else if (vecHost.size() == 1)
	{
		m_szHttpServer.Format(_T("http://%s:%d"),vecHost[0],strHostPort);
		return TRUE;
	}
	else
	{	
		int ilength = -1;
		CPing    Ping;
		DWORD fastTime = 9999;
		DWORD retTime = 0;
		CString strHost = _T("");
		DWORD dwSelIndex = 0;

		for (int i = 0; i < vecHost.size(); ++i)
		{	
			if (m_bIsCancelUpdate)
			{
				return FALSE;
			}
			char* ip = CStringToMutilChar(vecHost[i],ilength);
			retTime = Ping.PingHost(ip);
			if((fastTime > retTime) && (retTime >= 0))
			{
				fastTime = retTime;
				strHost = ip;
				dwSelIndex = i;			//选中的服务器序号
			}
			free(ip);
			ip = NULL;
		}
		if (strHost.IsEmpty())
		{
			strHost = vecHost[0];
			m_szHttpServer.Format(_T("http://%s:%d"),strHost,strHostPort);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("Ping操作之后无法选中可用的服务器，默认选中第一个"));
			return TRUE;
		}
		else
		{
			//此处避免升级有误需要做一个切换
			m_szHttpServer.Format(_T("http://%s:%d"), strHost,strHostPort);
			CString strURL = m_szHttpServer + strAddr;
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("Ping之后选择的服务器地址为:%s"),m_szHttpServer);

			if (IsUrlAvailable(strURL))
			{	
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("拼接URL连接可用URL:%s"),strURL);
				return TRUE;
			}	
			else
			{	
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("Ping选择服务器:%s,拼接URL连接不可用URL:%s"),m_szHttpServer,strURL);

				for (int m = 0; m < vecHost.size(); ++m)
				{	
					if (m_bIsCancelUpdate)
					{
						return FALSE;
					}

					if (m == dwSelIndex)
					{
						continue;
					}
					else
					{	
						m_szHttpServer.Format(_T("http://%s:%d"), vecHost[m],strHostPort);
						CString strURL = m_szHttpServer + strAddr;

						if (IsUrlAvailable(strURL))
						{
							return TRUE;
						}
					}
				}
				//此处代表所有服务器进行切换之后无法找到能够可用服务器
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("自动切换服务器操作没有找到可以服务器！"));
				return FALSE;
			}

		}

	}
}

char* CUpdateOperate::CStringToMutilChar(CString str,int& chLength)
{
	char* pszMultiByte; 
	int iSize = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL); 
	pszMultiByte = (char*)malloc((iSize+1)/**sizeof(char)*/); 
	memset(pszMultiByte,0,iSize+1);
	WideCharToMultiByte(CP_ACP, 0, str, -1, pszMultiByte, iSize, NULL, NULL); 
	chLength = iSize;
	return pszMultiByte;
}

char* CUpdateOperate::CStringToMutilChar( CString& str,int& chLength,WORD wPage/*=CP_ACP*/ )
{
	char* pszMultiByte; 
	int iSize = WideCharToMultiByte(wPage, 0, str, -1, NULL, 0, NULL, NULL); 
	pszMultiByte = (char*)malloc((iSize+1)/**sizeof(char)*/); 
	memset(pszMultiByte,0,iSize+1);
	WideCharToMultiByte(wPage, 0, str, -1, pszMultiByte, iSize, NULL, NULL); 
	chLength = iSize;
	return pszMultiByte;
}

BOOL CUpdateOperate::IsUrlAvailable(const CString& strURL)
{	
	CString strTempUrl = strURL;

	CHttpUtils http;
	CString strResponse = http.GetSerRespInfo(strTempUrl.GetBuffer());

	if (!strResponse.Compare(_T("$ERR$")))
	{	
		return FALSE;
	}
	return TRUE;
}

bool CUpdateOperate::GetCheckFileInfo(T_UPDATE_INFO *pInfo,vector<CString>&vec_ExcludeCheckFile)
{
	TCHAR szXMLFile[MAX_PATH] = {0};
	_sntprintf(szXMLFile, _TRUNCATE, _T("%s\\bin\\filelist_new.dat"), g_pGlobalData->dir.GetInstallDir());

	CFile datfile;
	CString strData = _T("");
	if (!datfile.Open(szXMLFile, CFile::modeRead))
		return false;

	TiXmlDocument doc;

	int iMaxXMLen = 512 * 1024;          //512k
	TCHAR *pZipData = NULL;
	TCHAR *pUnZipData = NULL;
	char *pmsbXMLData = NULL;

	try
	{	

		ULONGLONG len = datfile.GetLength() + 1;
		pZipData = new TCHAR[len];
		memset(pZipData, 0, len * sizeof(TCHAR));
		datfile.Read(pZipData, len);

		//解压缩数据
		pUnZipData = new TCHAR[iMaxXMLen];
		memset(pUnZipData, 0, iMaxXMLen);
		m_zip.DeocdeANDUnzipString(pZipData, _tcslen(pZipData), pUnZipData, &iMaxXMLen);
		delete []pZipData;

		DWORD dwSize = _tcslen(pUnZipData) * 2;
		pmsbXMLData = new char[dwSize];
		memset(pmsbXMLData, 0, dwSize);
		WCharToMByte(pUnZipData, pmsbXMLData, &dwSize);

		doc.Parse(pmsbXMLData);

		delete []pmsbXMLData;
		delete []pUnZipData;

		return ParseNewXMLNew(&doc, pInfo,vec_ExcludeCheckFile);
	}
	catch (...)
	{
		if (!pmsbXMLData)
			delete []pmsbXMLData;

		if (!pUnZipData)
			delete []pUnZipData;

		if (!pZipData)
			delete []pZipData;

		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("解析旧的文件列表异常！"));
	}

	return false;
}

BOOL CUpdateOperate::CheckUpdateSuccess()
{
	if (!GetCheckFileInfo(&m_newUpdate,m_vec_ExcludeCheckFile))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("校验出错，filelist_new文件解析出错！"));
		return FALSE;
	}				
	/*文件校验操作，主要是对新的文件列表中的文件的存在性及与本地文件进行MD5值比较，
	同时排除不需要校验MD5值，只需要校验存在性的文件，这些存放在m_vec_ExcludeCheckFile
	*/
	else
	{
		DIR_ITEM_MAP::iterator itNewDir;
		for (itNewDir = m_newUpdate.mapDirList.begin(); itNewDir != m_newUpdate.mapDirList.end(); ++itNewDir)
		{	
			//记录根目录
			CString strTargetDir = GetDirectory(itNewDir->first);
			FILE_ITEM_MAP::iterator itFileNew;
			for (itFileNew = itNewDir->second.begin(); itFileNew != itNewDir->second.end(); ++itFileNew)
			{
				if (IsFileExcludeCRC(strTargetDir,itFileNew->second))
				{
					//只需校验存在性
					CString strFile = _T("");
					strFile.Format(_T("%s\\%s"), strTargetDir, itFileNew->second.szLocalName);

					if (PathFileExists(strFile))
					{	
						continue;
					}
					else
					{	
						g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("升级成功校验出错，文件不存在路径:%s"),strFile);
						return FALSE;
					}
				}
				else
				{
					//校验存在性与MD5值
					CString strFile,strMd5 = _T("");
					strFile.Format(_T("%s\\%s"), strTargetDir, itFileNew->second.szLocalName);
					GetFileMD5(strFile,strMd5);

					if (PathFileExists(strFile))
					{	
						if (!strMd5.CompareNoCase(itFileNew->second.szMD5))
						{
							continue;
						}
						else
						{
							g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("升级成功校验出错，本地文件:%sMD5值:%s,服务器文件MD5值:%s"),\
								strFile,strMd5,itFileNew->second.szMD5);
							return FALSE;
						}

					}
					else
					{	
						g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("升级成功校验出错，文件不存在路径:%s"),strFile);
						return FALSE;
					}
				}
			}
		}
	}
	//写入IE内核版本
	/*if (!WriteIEKernelVersion())
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("升级校验bug,写入内核版本失败，提示用户以管理员身份运行软件!"));
		return FALSE;
	}*/

	CReg reg;
	reg.WriteValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ\\Setup"), REG_KEY_VERSION, m_newUpdate.szVersion);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("升级成功校验ok,本地文件已经最新！"));
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("新写入注册表版本号为%s:"), m_newUpdate.szVersion);

	return TRUE;
}

//判断文件是否不需要进行MD5值校验，只有当onlyAbsentUpdate属性为1，或者是包含在excludeCRC中的文件才成立
BOOL CUpdateOperate::IsFileExcludeCRC(CString szTarDir, T_FILE_ITEM &fileItem)
{	

	CString strFile = _T("");
	strFile.Format(_T("%s\\%s"), szTarDir, fileItem.szLocalName);

	vector<CString>::iterator result = find( m_vec_ExcludeCheckFile.begin(), m_vec_ExcludeCheckFile.end( ), strFile); 

	if (result != m_vec_ExcludeCheckFile.end())
	{
		return TRUE;
	}
	else
	{	
		//系统文件只需要校验存在性
		if (_T("1") == fileItem.szOnlyAbsentUpdate)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

bool CUpdateOperate::GetNewUpdateInfo(CString strXMLData, T_UPDATE_INFO *Info)
{
	int iMaxXMLen = 512 * 1024;       //512k
	TCHAR *ptmpBuf = NULL;
	char *pmsbXMLData = NULL;

	try
	{
		ptmpBuf = new TCHAR[iMaxXMLen];
		memset(ptmpBuf, 0, iMaxXMLen);
		m_zip.DeocdeANDUnzipString(strXMLData.GetBuffer(), strXMLData.GetLength(), ptmpBuf, &iMaxXMLen);

		DWORD dwSize = _tcslen(ptmpBuf) * 2;
		pmsbXMLData = new char[dwSize];
		memset(pmsbXMLData, 0, dwSize);
		WCharToMByte(ptmpBuf, pmsbXMLData, &dwSize);

		TiXmlDocument doc;
		doc.Parse(pmsbXMLData);

		delete []pmsbXMLData;
		delete []ptmpBuf;

		return ParseXML(&doc, Info);
	}
	catch (...)
	{
		if (!ptmpBuf)
			delete []ptmpBuf;

		if (!pmsbXMLData)
			delete []pmsbXMLData;

		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("解析最新文件列表异常！"));
		return false;
	}

	return true;
}

//获取需要下载的URL文件列表及总大小
BOOL CUpdateOperate:: GetDownLoadFileList(T_UPDATE_INFO &newInfo, 
	std::vector<CString>&vecUrl,std::vector<CString>&vecTempPath)
{	
	DIR_ITEM_MAP::iterator itNewDir;
	CString strTempFile = _T("");          //下载到temp文件夹的文件名
	CString strTarFile = _T("");           //目标文件名，用于检查目标文件是否存在
	vecUrl.clear();
	vecTempPath.clear();
	m_vecmd5.clear();

	for (itNewDir = newInfo.mapDirList.begin(); itNewDir != newInfo.mapDirList.end(); ++itNewDir)
	{	
		if (m_bIsCancelUpdate)
		{
			return FALSE;
		}

		CString strTargetDir = GetDirectory(itNewDir->first);
		FILE_ITEM_MAP::iterator itFileNew;

		//检查此文件夹下的文件是否需要下载
		for (itFileNew = itNewDir->second.begin(); itFileNew != itNewDir->second.end(); ++itFileNew)
		{	
			//对应的本地temp目录
			strTempFile.Format(_T("%s\\temp\\%s"), g_pGlobalData->dir.GetInstallDir(), itFileNew->second.szLocalName);

			if (m_bIsCancelUpdate)
			{
				return FALSE;
			}
			//如果是系统文件，且文件存在时则不下载
			if (IsSysFileAndExist(strTargetDir, itFileNew->second))
			{
				continue;
			}
			else
			{
				//校验存在性与MD5值
				CString strFile,strMd5 = _T("");
				strFile.Format(_T("%s\\%s"), strTargetDir, itFileNew->second.szLocalName);
				GetFileMD5(strFile,strMd5);

				if (PathFileExists(strFile))
				{	
					if (!strMd5.CompareNoCase(itFileNew->second.szMD5))
					{
						continue;
					}
					else
					{
						
						//这种情况下需要获取temp目录下是否存在对应的文件，然后再判断是否需要下载
						if (GetTempFileMd5(strTempFile,strMd5))
						{
							if (!strMd5.CompareNoCase(itFileNew->second.szMD5))
							{
								continue;
							}
						}

						g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("升级检测到文件需要下载，本地文件:%sMD5值:%s,服务器文件MD5值:%s"), \
							strFile,strMd5,itFileNew->second.szMD5);
					}

				}
				else
				{	
					if (GetTempFileMd5(strTempFile,strMd5))
					{
						if (!strMd5.CompareNoCase(itFileNew->second.szMD5))
						{
							continue;
						}
					}
					g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("升级检测到文件需要下载，文件不存在路径:%s"), strFile);
				}
			}

			
			vecTempPath.push_back(strTempFile);
			//需要下载文件的URL值
			CString strFullURL = /*m_szHttpServer + */itFileNew->second.szRemoteName;

			vecUrl.push_back(strFullURL);
			m_vecmd5.push_back(itFileNew->second.szMD5);
		}
	}
	return true;
}

CString CUpdateOperate::GetDirectory(CString strShortDir)
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
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("未找到环境变量 %s,目录未替换！"), strTotalFlag);
		}
		strRet.ReleaseBuffer();
	}


	//将替换后的目录加上目录变量之后的路径
	strShortDir.Replace(strTotalFlag, strRet);
	goto FindFlag;           //继续查找下一个通配符
}

bool CUpdateOperate::IsSysFileAndExist(CString szTarDir, T_FILE_ITEM &fileItem)
{
	//检查是否设置了仅当目标文件不存在时才下载的标记
	if (_T("1") == fileItem.szOnlyAbsentUpdate)
	{
		CString strFile = _T("");
		strFile.Format(_T("%s\\%s"), szTarDir, fileItem.szLocalName);

		if (PathFileExists(strFile))
		{
			return true;
		}
	}

	return false;
}

BOOL CUpdateOperate::GetFileMD5(const CString &strFile, CString &strMd5)
{
	//文件不存在直接返回
	if (!PathFileExists(strFile))
		return FALSE;

	//已存在则比较MD5确认是否是最新
	HANDLE hFile = CreateFile(strFile, 
		GENERIC_READ, 
		FILE_SHARE_READ ,NULL,
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL); 

	if(hFile==INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD m_dwBUFSize = GetFileSize(hFile, NULL);

	HANDLE hMapFile = CreateFileMapping(
		hFile,                   // use paging file
		NULL,                    // default security 
		PAGE_READONLY,           // read/write access
		0,                       // max. object size 
		m_dwBUFSize,             // buffer size  
		NULL);                   // name of mapping object

	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) 
	{ 
		_tprintf(_T("Could not create file mapping object with %s, error(%d).\n"), strFile, GetLastError());
		CloseHandle(hFile);
		return FALSE;
	}
	PBYTE m_pByteBuf = (PBYTE) MapViewOfFile(
		hMapFile,                 // handle to mapping object
		FILE_MAP_READ,				// read/write permission
		0,                   
		0,                   
		m_dwBUFSize); 

	if (m_pByteBuf == NULL)
	{
		CloseHandle(hMapFile);
		CloseHandle(hFile);
		return FALSE;
	}

#define MD5_LENGTH      34
	char szMD5[MD5_LENGTH] = {0};
	TCHAR tszMD5[MD5_LENGTH] = {0};
	GetMD5Code(m_pByteBuf, m_dwBUFSize, szMD5);

	UnmapViewOfFile(m_pByteBuf);
	CloseHandle(hMapFile);
	CloseHandle(hFile);

	strMd5 = szMD5;

	return TRUE;
}

//开始下载文件列表，此处要处理下载失败时要移除相关容器内存放的内容才对
BOOL CUpdateOperate::StartDownLoadFile(BOOL& b_IsCancel)
{	
	//记录需要下载的总数目
	CString strEncodeData = _T("");
	DWORD dwDownLoadCount = m_vecDownLoadUrl.size();
	CString strPostUrl = m_szHttpServer + CString(m_tPtdSetting.strUpdateDownLoadpage.GetBuffer());


	for (int m = 0; m < m_vecDownLoadUrl.size(); ++m)
	{	
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("正在下载的文件URL为:%s"), m_vecDownLoadUrl[m]);

		if (m_bIsCancelUpdate)
		{
			b_IsCancel = TRUE;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到了用户点击取消升级消息!"));

			return FALSE;
		}

		CString strTargetDir = m_vecTempPath[m];		
		strTargetDir = strTargetDir.Mid(0, strTargetDir.ReverseFind('\\'));
		if (!PathFileExists(strTargetDir))
		{
			SHCreateDirectoryEx(NULL, strTargetDir, NULL);
		}
		
		//生成postData
		if (!GetDownLoadEncodeData(strEncodeData,m_vecDownLoadUrl[m]))
		{
			return FALSE;
		}
		
		CInternetHttp http;
		int iRes = http.HttpPostFile(strPostUrl,strEncodeData,m_vecTempPath[m]);

		//判断文件是否下载成功
		if (iRes != 0)
		{
			if (iRes == -2)
			{
				b_IsCancel = TRUE;
			}
			return FALSE;
		}

		//此处增加md5值的匹配
		CString strMd5;
		GetFileMD5(m_vecTempPath[m], strMd5);
		if (strMd5.CompareNoCase(m_vecmd5[m]))
		{	
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, 
				_T("下载的文件与服务器上的文件MD5值不匹配！下载文件路径为:%s:下载后的文件MD5值为%s:服务器此文件MD5值为%s"), 
				m_vecTempPath[m],strMd5, m_vecmd5[m]);
			return FALSE;
		}
		else
		{	
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("URL:%s下载成功!"), m_vecDownLoadUrl[m]);
			m_dwProgeress = ((m+1)*100)/dwDownLoadCount;
		}
		
		//发送下载进度给主界面
		PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_PRODUCT_UPDATE,m_wUpdateFlag,(LPARAM)m_dwProgeress);
	}
	return TRUE;
}

//add by zhumingxing 20141124
BOOL CUpdateOperate::DownLoadFileByURL(CString& strUrl)
{
	BOOL bRes;
	byte* szTemp = new byte[1024*500];  
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("正在下载的文件URL为:%s"), strUrl);
	bRes = FALSE;

	if (m_bIsCancelDownLoad)
	{	
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到用户取消升级消息！"));

		if (NULL != szTemp)
		{
			delete[]szTemp;
			szTemp = NULL;
		}
		return bRes;
	}
	//文件下载完之后存放在安装目录下
	CString strTargetDir = GetDirectory(_T("%temp%"));
	strTargetDir+= _T("\\setup.exe");

	DWORD dwFlags = 0;
	HINTERNET hOpen = NULL;

	if (!InternetGetConnectedState(&dwFlags, 0))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("网络未连接, err:%d"), GetLastError());
		if (NULL != szTemp)
		{
			delete[]szTemp;
			szTemp = NULL;
		}
		return bRes;
	}

	if(!(dwFlags & INTERNET_CONNECTION_PROXY))
		hOpen = InternetOpen(_T("BIZEXPRESS_UpdateOL_"), INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY, NULL, NULL, 0);
	else
		hOpen = InternetOpen(_T("BIZEXPRESS_UpdateOL_"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if(!hOpen)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("InternetOpen错误, err:%d"), GetLastError());

		if (NULL != hOpen)
			InternetCloseHandle(hOpen);

		if (NULL != szTemp)
		{
			delete[]szTemp;
			szTemp = NULL;
		}
		return bRes;
	}

	DWORD dwSize;
	TCHAR   szHead[] = _T("Accept: */*\r\n\r\n");
	HINTERNET  hConnect = NULL;
	CFileException e;

	if ( !(hConnect = InternetOpenUrl( hOpen, strUrl.GetBuffer(), szHead,
		_tcslen(szHead), INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0)))
	{

		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("InternetOpenUrl错误, err:%d"), GetLastError());

		if (NULL != hConnect)
			InternetCloseHandle(hConnect);

		if (NULL != hOpen)
			InternetCloseHandle(hOpen);

		if (NULL != szTemp)
		{
			delete[]szTemp;
			szTemp = NULL;
		}
		return bRes;
	}

	if  (!m_file.Open(strTargetDir, CFile::modeWrite | CFile::modeCreate, &e))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("打开文件失败：%d"), e.m_cause);

		if (NULL != hConnect)
			InternetCloseHandle(hConnect);
		if (NULL != hOpen)
			InternetCloseHandle(hOpen);
		if (NULL != szTemp)
		{
			delete[]szTemp;
			szTemp = NULL;
		}
		return bRes;    
	}

	DWORD dwByteToRead = 0;
	DWORD dwSizeOfRq = 4;
	DWORD dwBytes = 0;

	if (!HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
		(LPVOID)&dwByteToRead, &dwSizeOfRq, NULL))
	{
		dwByteToRead = 0;
	}

	do
	{	
		if (m_bIsCancelDownLoad)
		{	
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("检测到用户取消升级消息！"));
			m_file.Close();
			if (NULL != hConnect)
				InternetCloseHandle(hConnect);
			if (NULL != hOpen)
				InternetCloseHandle(hOpen);
			if (NULL != szTemp)
			{
				delete[]szTemp;
				szTemp = NULL;
			}
			return FALSE;
		}

		memset(szTemp,0,sizeof(szTemp));
		if (!InternetReadFile (hConnect, szTemp, 1024*500,  &dwSize))		
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("InternetReadFile错误, err:%d"), GetLastError());
			m_file.Close();

			if (NULL != hConnect)
				InternetCloseHandle(hConnect);
			if (NULL != hOpen)
				InternetCloseHandle(hOpen);
			if (NULL != szTemp)
			{
				delete[]szTemp;
				szTemp = NULL;
			}

			return bRes;
		}

		if (dwSize==0)
		{
			bRes = TRUE;
			break;
		}
		else
		{	
			//写入下载内容
			m_file.Write(szTemp, dwSize);
		}

	}while (TRUE);

	m_file.Close();

	//add by zhumingxing 20141125
	//此处增加md5值的匹配
	CString strMd5;
	GetFileMD5(strTargetDir, strMd5);

	if (strMd5.CompareNoCase(m_tPostMessage.strMd5))
	{	
		//modefy by zhumingxing 20140709  增加对MD5值的日志输出
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("下载的安装包与实际不符:md51=%s,md52=%s"),strMd5,m_tPostMessage.strMd5);
		//end 
		bRes = FALSE;
		if (NULL != hConnect)
			InternetCloseHandle(hConnect);
		if (NULL != hOpen)
			InternetCloseHandle(hOpen);
		return bRes;
	}

	if (NULL != hConnect)
		InternetCloseHandle(hConnect);

	if (NULL != hOpen)
		InternetCloseHandle(hOpen);

	if (NULL != szTemp)
	{
		delete[]szTemp;
		szTemp = NULL;
	}

	return bRes;
}

BOOL CUpdateOperate::DownLoadFileByURL(CString& strUrl, CString& strFileName)
{
	BOOL bRes;
	byte* szTemp = new byte[1024 * 500];
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("正在下载的文件URL为:%s"), strUrl);
	bRes = FALSE;

	if (m_bIsCancelSilentInstall)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到用户取消升级消息！"));

		if (NULL != szTemp)
		{
			delete[]szTemp;
			szTemp = NULL;
		}
		return bRes;
	}
	//文件下载完之后存放在安装目录下
	//CString strTargetDir = GetDirectory(_T("%temp%"));
	//strTargetDir += _T("\\setup.exe");

	DWORD dwFlags = 0;
	HINTERNET hOpen = NULL;

	if (!InternetGetConnectedState(&dwFlags, 0))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("网络未连接, err:%d"), GetLastError());
		if (NULL != szTemp)
		{
			delete[]szTemp;
			szTemp = NULL;
		}
		return bRes;
	}

	if (!(dwFlags & INTERNET_CONNECTION_PROXY))
		hOpen = InternetOpen(_T("BIZEXPRESS_UpdateOL_"), INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY, NULL, NULL, 0);
	else
		hOpen = InternetOpen(_T("BIZEXPRESS_UpdateOL_"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (!hOpen)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("InternetOpen错误, err:%d"), GetLastError());

		if (NULL != hOpen)
			InternetCloseHandle(hOpen);

		if (NULL != szTemp)
		{
			delete[]szTemp;
			szTemp = NULL;
		}
		return bRes;
	}

	DWORD dwSize;
	TCHAR   szHead[] = _T("Accept: */*\r\n\r\n");
	HINTERNET  hConnect = NULL;
	CFileException e;

	if (!(hConnect = InternetOpenUrl(hOpen, strUrl.GetBuffer(), szHead,
		_tcslen(szHead), INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0)))
	{

		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("InternetOpenUrl错误, err:%d"), GetLastError());

		if (NULL != hConnect)
			InternetCloseHandle(hConnect);

		if (NULL != hOpen)
			InternetCloseHandle(hOpen);

		if (NULL != szTemp)
		{
			delete[]szTemp;
			szTemp = NULL;
		}
		return bRes;
	}

	if (!m_file.Open(strFileName, CFile::modeWrite | CFile::modeCreate, &e))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("打开文件失败：%d"), e.m_cause);

		if (NULL != hConnect)
			InternetCloseHandle(hConnect);
		if (NULL != hOpen)
			InternetCloseHandle(hOpen);
		if (NULL != szTemp)
		{
			delete[]szTemp;
			szTemp = NULL;
		}
		return bRes;
	}

	DWORD dwByteToRead = 0;
	DWORD dwSizeOfRq = 4;
	DWORD dwBytes = 0;

	if (!HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
		(LPVOID)&dwByteToRead, &dwSizeOfRq, NULL))
	{
		dwByteToRead = 0;
	}

	do
	{
		if (m_bIsCancelSilentInstall)
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("检测到用户取消升级消息！"));
			m_file.Close();
			if (NULL != hConnect)
				InternetCloseHandle(hConnect);
			if (NULL != hOpen)
				InternetCloseHandle(hOpen);
			if (NULL != szTemp)
			{
				delete[]szTemp;
				szTemp = NULL;
			}
			return FALSE;
		}

		memset(szTemp, 0, sizeof(szTemp));
		if (!InternetReadFile(hConnect, szTemp, 1024 * 500, &dwSize))
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("InternetReadFile错误, err:%d"), GetLastError());
			m_file.Close();

			if (NULL != hConnect)
				InternetCloseHandle(hConnect);
			if (NULL != hOpen)
				InternetCloseHandle(hOpen);
			if (NULL != szTemp)
			{
				delete[]szTemp;
				szTemp = NULL;
			}

			return bRes;
		}

		if (dwSize == 0)
		{
			bRes = TRUE;
			break;
		}
		else
		{
			//写入下载内容
			m_file.Write(szTemp, dwSize);
		}

	} while (TRUE);

	m_file.Close();

	//add by zhumingxing 20141125
	//此处增加md5值的匹配
	CString strMd5;
	GetFileMD5(strFileName, strMd5);

	if (strMd5.CompareNoCase(m_tTuiSong.m_strMd5))
	{
		//modefy by zhumingxing 20140709  增加对MD5值的日志输出
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("下载的安装包与实际不符:md51=%s,md52=%s"), strMd5, m_tTuiSong.m_strMd5);
		//end 
		bRes = FALSE;
		if (NULL != hConnect)
			InternetCloseHandle(hConnect);
		if (NULL != hOpen)
			InternetCloseHandle(hOpen);
		return bRes;
	}

	if (NULL != hConnect)
		InternetCloseHandle(hConnect);

	if (NULL != hOpen)
		InternetCloseHandle(hOpen);

	if (NULL != szTemp)
	{
		delete[]szTemp;
		szTemp = NULL;
	}

	return bRes;
}

void CUpdateOperate::DelFiles(CString strFolder)
{
	//文件夹直接用CString时会出现句柄无效，所以先转成TCHAR
	TCHAR tszPath[MAX_PATH] = {0};
	_sntprintf(tszPath, _TRUNCATE, _T("%s"), strFolder);

	SHFILEOPSTRUCT op;
	memset(&op, 0, sizeof(SHFILEOPSTRUCT));
	op.wFunc = FO_DELETE;
	op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
	op.pFrom = tszPath;            
	op.pTo = NULL;
	op.lpszProgressTitle = NULL;

	if (0 != SHFileOperation(&op))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("删除文件夹%s失败！ err: %d"), tszPath, GetLastError());
	}
}

bool  CUpdateOperate::WaitProcEnd(TCHAR *pProgName, DWORD dwTimeOut, bool bForceEnd)
{
	DWORD dwStartTime = GetTickCount();

	while(true)
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
				if (!_tcsicmp(pProgName, process.szExeFile))
				{
					bRun = true;
					break;
				}

				bFlag = Process32Next(hSnapShot, &process);
			}

			CloseHandle(hSnapShot); 

			if (!bRun) return true;
		}
		else
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("CreateToolhelp32Snapshot failed, err: %d"), GetLastError());
		}

		//等待超时
		if (GetTickCount() - dwStartTime > dwTimeOut)
		{
			if (bForceEnd)
			{
				HANDLE handLe =  OpenProcess(PROCESS_TERMINATE, FALSE, process.th32ProcessID);
				BOOL bResult = TerminateProcess(handLe,0);
				//return bResult;
			}

			//return false;
		}

		//Sleep(100);
	}

	return false;
}

bool CUpdateOperate::RunUpdateCommand(T_UPDATE_INFO &newInfo, CString strRunFlag)
{
	CString strDir = _T("");
	CString strFile = _T("");
	CString strCommandLine = _T("");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si,sizeof(si));
	si.cb =sizeof(si);
	ZeroMemory(&pi,sizeof(pi));

	//依次执行本目录下的命令
	std::vector<T_EXEC_FILE_ITEM> ::iterator itExecFile = newInfo.vRunList.begin();
	for (; itExecFile != newInfo.vRunList.end(); ++itExecFile)
	{
		//只运行符合时机的命令
		if (strRunFlag != itExecFile->szRunTime)
		{
			continue;
		}

		CString strAction = itExecFile->szAction.MakeLower();
		if (_T("kill") == strAction)          //停止进程（忽略param）
		{
			if (!WaitProcEnd(itExecFile->szFileName.GetBuffer(), 0, true))
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("停止进程%s失败！ err: %d"), itExecFile->szFileName, GetLastError());
				return false;
			}
		}
		else if (_T("run") == strAction)      //运行程序
		{	
			//modefy by zhumingxing 20140808 调用createprocess时将应用程序名字和命名行分开 
			/*strFile.Format(_T("%s %s"), GetDirectory(itExecFile->szFileName), GetDirectory(itExecFile->szParam));*/
			strFile.Format(_T("%s"), GetDirectory(itExecFile->szFileName));
			strCommandLine.Format(_T(" %s"),GetDirectory(itExecFile->szParam));

			BOOL bRet = FALSE;
			bRet = CreateProcess(strFile.GetBuffer(),strCommandLine.GetBuffer(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

			if (!bRet)
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("执行%s失败！ err: %d"), strFile, GetLastError());
				return false;
			}

			//等待进程结束
			if (itExecFile->szWaitEnd == "1")
			{
				WaitForSingleObject(pi.hProcess, INFINITE);
			}
		}
		else if (_T("del") == strAction)     //删除文件（忽略param）
		{
			strFile.Format(_T("%s"), GetDirectory(itExecFile->szFileName));
			DelFiles(strFile.GetBuffer());
		}
	}

	return true;
}

void CUpdateOperate::PrepareForCopyFiles()
{
	//结束相关进程
	WaitProcEnd(_T("SeekWord.exe"), 0, true);
	WaitProcEnd(_T("MC.exe"), 0, true);
	WaitProcEnd(_T("Engine.exe"), 0, true);
	WaitProcEnd(_T("SyncDat.exe"), 0, true);
	WaitProcEnd(_T("CtrlUserTask.exe"), 0, true);
	WaitProcEnd(_T("UpdateRank.exe"), 0, true);

	WaitProcEnd(_T("AutomaticSearch.exe"), 0, true);
	CString strAppPath;
	strAppPath.Format(_T("%s\\bin\\KillService.bat"), GetInstallPath());
	ShellExecute(NULL, _T("open"), strAppPath, _T(""), NULL, SW_HIDE);
	//WaitProcEnd(_T("BugReport.exe"), 0, true);
}

//判断IECtrl是否可以复制---可根据参数来判断是IE控件还是多游览器控件
bool CUpdateOperate::CheckIEctrlCopy()
{	
	CString strTarFile = _T("");
	CString strTempFile = _T("");
	strTempFile.Format(_T("%s\\temp\\MasterZ\\plugins\\ieplugin.dll"), g_pGlobalData->dir.GetInstallDir());

	if (PathFileExists(strTempFile))
	{
		strTarFile.Format(_T("%s\\plugins\\ieplugin.dll"), g_pGlobalData->dir.GetInstallDir());

		if (!MyCopyFile(strTempFile, strTarFile,FALSE))
		{	
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("复制web控件出错，检查是否网页登陆舟大师页面:src=%s,des=%s"), strTempFile,strTarFile);

			PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_ERROR,WPARAM_IECTRL_COPYERROR,0);

			return FALSE;
		}
	}

	strTempFile.Format(_T("%s\\temp\\MasterZ\\plugins\\npszwplugin.dll"), g_pGlobalData->dir.GetInstallDir());
	if (PathFileExists(strTempFile))
	{
		strTarFile.Format(_T("%s\\plugins\\npszwplugin.dll"), g_pGlobalData->dir.GetInstallDir());

		if (!MyCopyFile(strTempFile, strTarFile,FALSE))
		{	
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("复制np控件出错，检查是否网页登陆舟大师页面:src=%s,des=%s"), strTempFile,strTarFile);

			PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_UPDATE,MSG_UPDATE_ERROR,WPARAM_NPCTRL_COPYERROR,0);

			return FALSE;
		}
	}

	return true;
}

bool CUpdateOperate::MyCopyFile(CString strSrc, CString strDest, BOOL bFailIfExists)
{
	if (CopyFile(strSrc, strDest, bFailIfExists))
	{
		return true;
	}

	g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("复制文件出错! err:%d, file: %s"), GetLastError(), strDest);
	return false;
}

void CUpdateOperate::SaveFileListToLocal(CString strData)
{	
	CString strXML = _T("");
	strXML.Format(_T("%s\\bin\\filelist_new.dat"),g_pGlobalData->dir.GetInstallDir());

	m_file.Open(strXML, CFile::modeCreate | CFile::modeWrite);
	m_file.Write(strData, strData.GetLength() * sizeof(TCHAR));
	m_file.Close();		
}

BOOL CUpdateOperate::DeleteDownLoadDir(WORD type)
{	
	if (m_file.m_hFile != CFile::hFileNull)
	{
		m_file.Close();
	}
	if (type == 0)
	{
		//删除temp文件夹
		CString strtemp;
		strtemp.Format(_T("%s\\temp"),g_pGlobalData->dir.GetInstallDir());

		if (_taccess(strtemp,0) == 0)
		{	
			DelFiles(strtemp);
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("成功删除temp文件夹！"));
		}
	}
	return TRUE;
}

//处理用户重启客户端消息
void CUpdateOperate::HanleRebootMsg()
{	
	/*注意：下载成功之后可以根据用户提示来执行下面的操作，如果说用户取消可以直接跳到取消消息即可*/
	//下载完成之后先根据下载之后的相关MD5值得到finish列表
	CString strbat = _T("");

	//执行升级前命令
	RunUpdateCommand(m_newUpdate, _T("pre"));
	//杀掉相关进程准备升级
	PrepareForCopyFiles();

	if (m_wRbootType == LPARAM_REBOOT_AUTO ||  m_wRbootType == LPARAM_REBOOT_MANUAL)
	{
		//检测相关控件是否被占用
		if (!CheckIEctrlCopy())
		{	
			return;
		}
		//执行升级后命令，主要是针对web控件注册的问题
		if (RunUpdateCommand(m_newUpdate, _T("post")))		
		{	
			//将新列表保存到安装目录
			SaveFileListToLocal(strSerResponse);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("filelist_new.dat文件生成完成！"));
		}

	}

	//给SysTool发送消息卸载全局钩子
	//PackagMessage(E_THREAD_CLEAR,E_THREAD_UPDATE,MSG_FREE_HOOK_DLL,0,0,TRUE);

	//执行bat文件
	m_strbatFilePath.Format(_T("%s\\bin\\UpdateOL.bat"),g_pGlobalData->dir.GetInstallDir());
	g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("批处理文件路径为:%s"),m_strbatFilePath);

	//begin modefy by zhumingxing 20140807 to add reboot type 
	if (m_wRbootType == LPARAM_REBOOT_AUTO /*|| m_wRbootType == LPARAM_REBOOT_AUTO_NOWRITEVERSION*/)
	{	
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("此处升级为后台强制重启！"));
		ShellExecute(NULL,_T("Open"),m_strbatFilePath,_T("/Update"),NULL,SW_HIDE );
	}
	else
	{	
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("此处升级为用户手动点击立即重启！"));
		ShellExecute(NULL,_T("Open"),m_strbatFilePath,NULL,NULL,SW_HIDE );
	}
	//end

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("正确执行了bat文件！"));
}

void CUpdateOperate::PackagMessage(DWORD dwDestThread,DWORD dwSourceThread,DWORD dwMessageType,WPARAM wParam, LPARAM lParam,BOOL bIsSync)
{
	m_tMsg = IMsgQueue::New_Message();
	m_tMsg->dwDestWork = dwDestThread;
	m_tMsg->dwSourWork = dwSourceThread;
	m_tMsg->dwMsg = dwMessageType;	//发送初始化消息
	m_tMsg->wParam = wParam;
	m_tMsg->lParam = lParam;

	if (!bIsSync)
	{
		PostMessage(m_tMsg);
	}
	else
	{
		SendMessage(m_tMsg);
	}
	
}


CString CUpdateOperate::GetAPIURL()
{	
	CString strURL;
	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\mc.dat"), GetInstallPath());
	IXMLRW iniFile;
	iniFile.init(strCfgFile);

	iniFile.ReadString(_T("MC"),_T("LOGININFO"), _T("url"), strURL);

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取到配置文件的API线上地址为%s！"),strURL);
	//如果为空，则取[SERVICE] Current 
	if (strURL.IsEmpty())
	{
		strURL = _T("http://api.sumszw.com");
		g_log.Trace(LOGL_TOP,LOGT_WARNING, __TFILE__,__LINE__, _T("API配置为空,使用默认API上线地址：http://api.sumszw.com"));
	}
	return strURL;
}

BOOL CUpdateOperate::UpdateConfigToLocal()
{
	CString strUrl = GetAPIURL();

	T_DATA_FROM_SERVER tData;
	if (GetConfigData(tData, strUrl))
	{
		if (tData.strVersion == _T("000"))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("本地配置文件已经是最新，无需更新!"));
			return TRUE;
		}
		else
		{
			if (!WriteConfigStr(tData))
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("获取到了最新配置文件信息,写配置文件失败！"));
				return FALSE;
			}
			else
			{
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("更新配置文件成功！"));
				return TRUE;
			}
		}
	} 
	else
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("获取配置信息失败！"));
		return FALSE;
	}

	return TRUE;
}

int CUpdateOperate::GetConfigData(T_DATA_FROM_SERVER &tData, CString strUrl)
{
	CString strRequestURL = _T(""); 
	CString strResponseText = _T("");
	CString strRequestData = _T("");
	CStdString strStdVersion = _T("");
	CString strVersionId = _T("");
	//CString strVersionId = _T("");
	CHttpUtils http;

	//读mc.dat中的versionId
	CString strCfgIniTemp = _T("%appdata%\\szw\\MasterZ\\data2\\version.dat");
	CString strCfgIni = GetDirectory(strCfgIniTemp);
	m_iniFile.SetFilePath(CStdString(strCfgIni));

	m_iniFile.ReadString(_T("version"), _T("ver"), strStdVersion);
	strVersionId = strStdVersion;
	if (strVersionId == _T(""))
	{
		strVersionId = _T("1.10");
	}


	strRequestURL.Format(_T("%s/api/Client/GetXmlData?versionStr=%s&product=zds"), strUrl, strVersionId);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("请求配置文件信息地址：%s"),strRequestURL);

	strResponseText = http.GetSerRespInfo(strRequestURL.GetBuffer());

	if (strResponseText == _T("\"0\""))
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("请求的版本号相同,请求到的文件配置信息为\"0\"！"));
		tData.strVersion = _T("000");
		return TRUE;
	}

	strResponseText.Replace(_T("\\\""), _T("\""));
	strResponseText.Replace(_T("\\\\"), _T("\\"));
	strResponseText.Replace(_T("\"<root>"), _T("<root>"));
	strResponseText.Replace(_T("</root>\""), _T("</root>"));

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

		if (!ParseServerResponse(&doc, tData))
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("解析任务数据失败！URL:%s\r\nData:\r\n%s\r\n"), strRequestURL, strResponseText);
			return FALSE;
		}
	}
	catch (...)
	{
		if (!ptmpBuf)
			delete []ptmpBuf;

		if (!pmsbXMLData)
			delete []pmsbXMLData;

		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("解析任务数据异常！URL:%s\r\nData:\r\n%s\r\n"), strRequestURL, strResponseText);
		return FALSE;
	}

	return TRUE;
}

bool CUpdateOperate::WriteConfigStr(T_DATA_FROM_SERVER &tConfigData)
{
	CStdString strMcconfigIniPath = _T("");
	int iRetWrite = 0;
	for (int i = 0; i != tConfigData.vMcconfig.size(); ++i)
	{
		CString strDir = tConfigData.vMcconfig[i].strTargetDir;
		CString strFileName = tConfigData.vMcconfig[i].strFileName;
		CString strCfgIniTemp = strDir + _T("\\") + strFileName;
		CString strCfgIni = GetDirectory(strCfgIniTemp);

		if (!PathFileExists(strCfgIni))
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("要写入的配置文件不存在，创建该配置文件！"));
		}

		m_XmlCfg.init(strCfgIni);
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("写入配置文件路径:%s！"), strCfgIni);

		CString strXmlCfgVer = _T("");
		m_XmlCfg.ReadXmlVer(strXmlCfgVer);

		if (tConfigData.vMcconfig[i].strVersion.CompareNoCase(strXmlCfgVer))
		{
			iRetWrite = m_XmlCfg.ReWriteXml(tConfigData.vMcconfig[i].strData);

			if (iRetWrite != XRET_SUCCESS)
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("写入配置失败:%s！"), strCfgIni);

				return false;
			}

		}
	}

	CString strCfgIniTemp = _T("%appdata%\\szw\\MasterZ\\data2\\version.dat");
	CString strCfgIni = GetDirectory(strCfgIniTemp);

	m_iniFile.SetFilePath(CStdString(strCfgIni));

	CStdString strVersion = tConfigData.strVersion;
	bool ret = m_iniFile.WriteString(_T("version"), _T("ver"), strVersion);
	if (!ret)
	{
		return false;
	}

	return true;
}

BOOL CUpdateOperate::GetTempFileMd5(const CString& strTempFilePath,CString& strMd5 )
{
	if (PathFileExists(strTempFilePath))
	{
		GetFileMD5(strTempFilePath,strMd5);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CUpdateOperate::ReadCheckUpdateFailCount()
{
	TCHAR szCfgINI[MAX_PATH] = {0};
	CIniFile iniFile;
	CStdString strVal = _T("");

	_sntprintf(szCfgINI, _TRUNCATE, _T("%s\\bin\\tempcount.dat"), g_pGlobalData->dir.GetInstallDir());
	if (!iniFile.SetFilePath(szCfgINI))
	{
		m_dwUpdateFailCount = 0;
	}
	else
	{
		iniFile.ReadInteger(_T("FailCount"),_T("count"),m_dwUpdateFailCount);
	}
}

void CUpdateOperate::WriteCheckUpdateFailCount()
{
	TCHAR szCfgINI[MAX_PATH] = {0};
	_sntprintf(szCfgINI, _TRUNCATE, _T("%s\\bin\\tempcount.dat"), g_pGlobalData->dir.GetInstallDir());

	TCHAR strValue[255] = {0};
	_stprintf(strValue,_T("%d"),m_dwUpdateFailCount);
	WritePrivateProfileString(_T("FailCount"),_T("count"),strValue,szCfgINI);
}

BOOL CUpdateOperate::GetDownLoadEncodeData(CString& strData,CString strFileName)
{	
	CString strContent = _T("");
	CString strEncode = UrlDecode(m_strEncodedata);
	CStdString strSrcData = DecodeString(strEncode);


	CStdStrUtils strUils;
	vector<CStdString> vRes;
	vRes.clear();
	strUils.SplitString(CStdString(strSrcData.GetBuffer()), _T("&"), vRes);

	if (vRes.size() != 5)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("解析用户数据错误，参数个数不等于5个:%s"),strSrcData);
		return FALSE;
	}
	else
	{	
		time_t timep;
		time(&timep);

		strContent.Format(_T("%s&%s&%d&%s&%s&%s"), vRes[0].GetBuffer(),strFileName, DWORD(timep), vRes[2].GetBuffer(), vRes[3].GetBuffer(), vRes[4].GetBuffer());
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("下载POST的数据为:%s"), strContent);
	}

	//进行一次异或操作
	TCHAR ch = _T('①');
	for (int i =0; i < strContent.GetLength(); ++i)
	{	
		DWORD Temp = strContent[i];
		Temp = Temp^ch;
		strContent.SetAt(i,Temp);
	}

	strContent = stringcoding::StringBase64Encode(strContent);
	//add by zhumingxing 20141106URLENCODE
	strContent = URLEncode(strContent);
	strData.Format(_T("msg=%s"),strContent);
	
	return TRUE;
}

CString CUpdateOperate::GetUserRequestVersion()
{
	CString strEncode = UrlDecode(m_strEncodedata);
	CStdString strSrcData = DecodeString(strEncode);


	CStdStrUtils strUils;
	vector<CStdString> vRes;
	vRes.clear();
	strUils.SplitString(CStdString(strSrcData.GetBuffer()), _T("&"), vRes);

	if (vRes.size() != 5)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("解析用户数据错误，参数个数不等于5个:%s"), strSrcData);
		return _T("");
	}

	return vRes[4];
}

BOOL CUpdateOperate::IsMatchLimitEquelVersion(CString& strLocalVersion)
{
	
	for (int iindex = 0; iindex < m_newUpdate.vCmpEquealVersion.size(); ++iindex)
	{
		if (!m_newUpdate.vCmpEquealVersion[iindex].Compare(strLocalVersion))
		{
			return TRUE;
		}
	}
	return FALSE;
}

CString CUpdateOperate::UrlDecode( LPCTSTR url )
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

CString CUpdateOperate::Utf8ToStringT( LPSTR str )
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

CStdString CUpdateOperate::DecodeString( CString& strDest)
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

CString CUpdateOperate::URLEncode( CString sIn )
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

BYTE CUpdateOperate::toHex( const BYTE &x )
{
	return x > 9 ? x + 55: x + 48;
}

BOOL CUpdateOperate::GetTuiSongFromStr(const CString& strTuiSong)
{	
	//清空结构体
	m_tTuiSong.clear();

	CString strTemp = strTuiSong;

	if (strTemp.Find(_T("|")) == -1 || strTemp.Find(_T("(;1)"))== -1)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("服务端返回的安装包下载信息错误.错误字符串为:%s"), strTemp.GetBuffer());
		return FALSE;
	}
	else
	{
		strTemp = strTemp.Mid(strTemp.Find(_T("|")) + 1);
		m_tTuiSong.m_strMd5 = strTemp.Mid(strTemp.Find(_T("(;1)")) + 4);

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("服务端返回的安装包MD5值为:%s"), m_tTuiSong.m_strMd5.GetBuffer());

		CString strReplace = CString(_T("(;1)")) + m_tTuiSong.m_strMd5;
		strTemp.Replace(strReplace, _T(""));
		
		int isize = SplitStringWithSeparator(strTemp, _T("(;0)"), m_tTuiSong.m_vecDownLoadUrl, TRUE);

		if (isize == 0)
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("服务端返回的安装包下载信息错误.错误字符串为:%s"), strTuiSong);
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}

BOOL CUpdateOperate::IsNeedUpdateServers()
{	
	//先判断temp目录下是否已经Demo.exe文件
	CString strTempDemopath = _T("");
	strTempDemopath.Format(_T("%s\\temp\\MasterZ\\bin\\Deamon.exe"), g_pGlobalData->dir.GetInstallDir());

	if (PathFileExists(strTempDemopath))
	{	
		return TRUE;
	}

	for (int i = 0; i < m_vecDownLoadUrl.size(); ++ i)
	{
		if (m_vecDownLoadUrl[i].Find(_T("Deamon.exe")) != -1)
		{
			return TRUE;
		}
	}

	return FALSE;
}

CString CUpdateOperate::GetGuidCode()
{
	GUID   guid;
	CString   szGUID;
	if (S_OK == ::CoCreateGuid(&guid))
	{
		szGUID.Format(_T("{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}")
			, guid.Data1
			, guid.Data2
			, guid.Data3
			, guid.Data4[0], guid.Data4[1]
			, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
			, guid.Data4[6], guid.Data4[7]
			);
	}

	return szGUID;
}

void CUpdateOperate::IsNeedUpdateClickPro(BOOL &bNeedUpdateService, BOOL &bNeedUpdateSerchPro)
{
	//先判断temp目录下是否已经存在文件AutoRunService.exe/测试程序AutomaticSearch.exe
	CString strTempDemopath = _T("");
	strTempDemopath.Format(_T("%s\\temp\\MasterZ\\bin\\AutoRunService.exe"), g_pGlobalData->dir.GetInstallDir());

	if (PathFileExists(strTempDemopath))
	{
		bNeedUpdateService = TRUE;
	}

	strTempDemopath = _T("");
	strTempDemopath.Format(_T("%s\\temp\\MasterZ\\bin\\AutomaticSearch.exe"), g_pGlobalData->dir.GetInstallDir());
	if (PathFileExists(strTempDemopath))
	{
		bNeedUpdateSerchPro = TRUE;
	}
	if (bNeedUpdateSerchPro && bNeedUpdateService)
		return;

	for (int i = 0; i < m_vecDownLoadUrl.size(); ++i)
	{
		if (!bNeedUpdateService && m_vecDownLoadUrl[i].Find(_T("AutoRunService.exe")) != -1)
		{
			bNeedUpdateService = TRUE;
		}

		if (!bNeedUpdateSerchPro && m_vecDownLoadUrl[i].Find(_T("AutomaticSearch.exe")) != -1)
		{
			bNeedUpdateSerchPro = TRUE;
		}

		if (bNeedUpdateSerchPro && bNeedUpdateService)
			return;
	}

	return;
}




