#include "StdAfx.h"
#include "MainWnd.h"
#include "ControlEx.h"
#include "Resource.h"
#include <Windows.h>
#include <WinInet.h>
#include "Tlhelp32.h"

//此变量为了控制用户手动点击升级多次,重复发送相关消息
static  DWORD iRequestCount = 0;
T_GLOBAL_DATA g_globalData;
TAG_HOOK_DATA g_KbHookData;
HWND g_hHwndMain = NULL; 
//HHOOK g_hCBT = NULL;

CMainWnd::CMainWnd(void)
{	
	HRESULT Hr = ::CoInitialize(NULL);
	m_bIsAutoLogin = false;
	m_bIsUseLogin = false;
	m_bIsRankRunning = false;
	m_bIsRetryAutoLogin = false;
	m_bIsUserLoginOut = false;
	m_wClearType = 0;
	m_dwClearStateFlag = CLEAR_STATE_INTIAL;
	m_strEncodeString = _T("");
	m_strWindowsName = _T("");
	m_wVersionId = 0;
	m_strClientStartTime = _T("");
	m_strClientVersion = _T("");
	m_bIsNewestVersion = FALSE;
	m_quePostMessage.empty();
	m_dwUpdateProgress = 0;
	m_bIsCancelUpdate = false;
	m_bIsUpdateChking = false;
	m_pActiveXFlash = NULL;
	m_pActiveXExposureFlash = NULL;
	m_iNewKeywordCnt = 0;
	m_bWriteStart = FALSE;
	m_iUnstallFlag = 0;
	m_iTimeCnt = 0;
	m_bUpdateReboot = FALSE;
	m_bPostInstall = FALSE;
	m_hProcess = NULL;
	m_strEmptyKeywordPath = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\nokeyworddata.html");
	//m_strEmptyExposurePath = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\noexposuredata.html");
	m_strTemplatePath = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\ZhenCiTemplate.html");
	m_strZhenciPath = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\index.html");
}	

CMainWnd::~CMainWnd(void)
{		
	;
}

CDuiString CMainWnd::GetSkinFolder()
{
	return _T("skin");
}

CDuiString CMainWnd::GetSkinFile()
{
	return _T("index.xml");
}

LPCTSTR CMainWnd::GetWindowClassName(void) const
{
	return _T("舟大师");
}

void CMainWnd::InitWindow()
{	 
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("开始初始化对话框!"));

	//初始化组件
	if (!InitComponent())
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("初始化组件失败！"));
	}

	//初始化相关操作，包括初始化窗口,控件及相关启动消息自动发送
	InitialOpearate();


	//加载第一个图表网页
	m_pActiveXFlash = static_cast<CWebBrowserUI*>(m_PaintManager.FindControl(_T("webpdtpage")));

//	m_pActiveXFlash->SetVisible(false);
	if (m_pActiveXFlash != NULL)
	{
		m_pActiveXFlash->SetDelayCreate(false);
		CCustomWebEventHandler *pWebHandle = new CCustomWebEventHandler;
		pWebHandle->SetMainHwnd(m_hWnd);
		m_pActiveXFlash->SetWebBrowserEventHandler(pWebHandle);
		m_pActiveXFlash->Navigate2(_T("about:blank"));
		m_pActiveXFlash->Navigate2(m_strEmptyKeywordPath);   //http://198.18.0.254:8009/ClientPage/ChartStatisticData?userName=kehu0051
	}
	
	//初始化画图状态	
	m_showchart.SetPaintManager(&m_PaintManager,GetHWND(),m_pActiveXFlash);
	/*
	::PostMessage(m_hWnd, MSG_SHOW_CHART, 0, 0);*/
	//获取是否有卸载标记
// 	CReg reg;
// 	BYTE* szUnFlag = reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ\\Setup"), _T("uninstall"));
// 
// 	if (szUnFlag != NULL)
// 	{
// 		m_iUnstallFlag = *((int*)(szUnFlag));
// 	}
	WriteRegInfo();
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("初始化界面完成！"));

	ShowWindow(false);
}
//写注册表信息
void CMainWnd::WriteRegInfo()
{
	//获取是否有卸载标记
	CReg reg;
	BYTE* szUnFlag = reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ\\Setup"), _T("uninstall"));

	if (szUnFlag != NULL)
	{
		m_iUnstallFlag = *((int*)(szUnFlag));
	}

	//写自定义URL协议
	CString strCmd = _T("");
	strCmd.Format(_T("%s"), g_globalData.dir.GetInstallDir());
	CString sURLProtocol;
	CString sDefaultIcon;
	CString sCommand;
	sURLProtocol.Format(_T("%s\\bin\\CtrlUserTask.exe"), strCmd);
	sDefaultIcon.Format(_T("%s\\bin\\CtrlUserTask.exe,1"), strCmd);
	sCommand.Format(_T("\"%s\\bin\\CtrlUserTask.exe"), strCmd);
	sCommand += _T("\" \"%1\"");
	CReg regEdit;
	if (!regEdit.WriteValueOfKey(HKEY_CLASSES_ROOT, _T("mctp"), _T(""), _T("mctpProtocol")))
	{
		Sleep(200);
		if (!regEdit.WriteValueOfKey(HKEY_CURRENT_USER, _T("Software\\Classes\\mctp"), _T(""), _T("mctpProtocol")))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("写注册表失败：mctpProtocol"));
		}
	}

	if (!regEdit.WriteValueOfKey(HKEY_CLASSES_ROOT, _T("mctp"), _T("URL Protocol"), sURLProtocol))
	{
		Sleep(200);
		if (!regEdit.WriteValueOfKey(HKEY_CURRENT_USER, _T("Software\\Classes\\mctp"), _T("URL Protocol"), sURLProtocol))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("写注册表失败：%s"), sURLProtocol);
		}
	}


	if (!regEdit.WriteValueOfKey(HKEY_CLASSES_ROOT, _T("mctp\\DefaultIcon"), _T(""), sDefaultIcon))
	{
		Sleep(200);
		if (!regEdit.WriteValueOfKey(HKEY_CURRENT_USER, _T("Software\\Classes\\mctp\\DefaultIcon"), _T(""), sDefaultIcon))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("写注册表失败：%s"), sDefaultIcon);
		}
	}

	if (!regEdit.WriteValueOfKey(HKEY_CLASSES_ROOT, _T("mctp\\shell\\open\\command"), _T(""), sCommand))
	{
		Sleep(200);
		if (!regEdit.WriteValueOfKey(HKEY_CURRENT_USER, _T("Software\\Classes\\mctp\\shell\\open\\command"), _T(""), sCommand))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("写注册表失败：%s"), sCommand);
		}
	}

}

CControlUI* CMainWnd::CreateControl(LPCTSTR pstrClass)
{	
	CDialogBuilderCallbackEx2 cb(&m_PaintManager);
	if( _tcsicmp(pstrClass, _T("HistoryData")) == 0 )
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("HistoryData.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if( _tcsicmp(pstrClass, _T("KeywordAnalysis")) == 0 )
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("KeywordAnalysis.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if( _tcsicmp(pstrClass, _T("GarbageRemoval")) == 0 )
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("GarbageRemoval.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if( _tcsicmp(pstrClass, _T("WebsiteSearch")) == 0 )
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("WebsiteSearch.xml"), 0, &cb, &m_PaintManager);
		return pUI; 
	}
	else if( _tcsicmp(pstrClass, _T("website")) == 0 )
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("websiteinfo.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if (_tcsicmp(pstrClass, _T("website2")) == 0 )
	{
		
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("website.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if( _tcsicmp(pstrClass, _T("Faq")) == 0 )
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("Faq.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if ( _tcsicmp(pstrClass, _T("GarbageRemoval2")) == 0 )
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("GarbageRemoval2.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if (_tcsicmp(pstrClass, _T("GarbageRemoval3")) == 0)
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("GarbageRemoval3.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if (_tcsicmp(pstrClass, _T("PromotionDetails")) == 0)
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("PromotionDetails.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if (_tcsicmp(pstrClass, _T("wechat")) == 0)
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("wechat.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if (_tcsicmp(pstrClass, _T("wechat2")) == 0)
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("wechat2.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	else if (_tcscmp(pstrClass,_T("keywordrank")) == 0)
	{
		CDialogBuilder builder;

		CControlUI* pUI = builder.Create(_T("Keywordrank.xml"), 0, &cb, &m_PaintManager);
		return pUI;
	}
	return __super::CreateControl(pstrClass);
}

LRESULT CMainWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	try
	{	
		if (uMsg == WM_ACTIVATE)
		{
			if (m_dwClientType != MAIN_LINE_VERSION)
			{
				ShowWindow(false);
			}			
		}
		if (uMsg == MSG_SALF_EXIT)
		{
			CReg reg;
			CString strFlag = (TCHAR*)(reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("hideClient")));
			if (strFlag == _T("1"))
			{
				HideClient();
				return 0;
			}
			
			try
			{	
				//杀掉隔天发送消息定时器
				::KillTimer(m_hWnd,ID_AUTO_SEND_MESSAGE);
				//杀掉获取通知消息定时器
				::KillTimer(m_hWnd,ID_GET_POST_MESSAGE);
				//杀掉与服务器交互timer
				::KillTimer(m_hWnd,ID_CONNECT_SERVER);

				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("广播安全退出消息！"));
				m_threadMgr.BroadcastMsg(MSG_SALF_EXIT,eMSG_SYNC);
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("安全退出消息已经全部收回！"));
				OperateTray(NIM_DELETE);
				WaitProcEnd(_T("SyncDat.exe"), 0, true);
			}
			catch(...)
			{

			}
			::CoUninitialize();

			//销毁钩子
			UnregisterHotKey(m_hWnd,ID_VKF4);
			if (NULL != g_KbHookData.hHook)  
			{
				UnhookWindowsHookEx(g_KbHookData.hHook);   
			}
			exit(0);
		}
		//屏蔽鼠标右键消息
		if (uMsg == WM_MOUSEACTIVATE)
		{	
			if (lParam == MOUSE_RIGHT_ACTIVE)
			{	
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到鼠标右键消息,屏蔽！"));
				return MA_ACTIVATEANDEAT;
			}
			return 0;
		}
		//软件启动时显示falsh
		if (uMsg == MSG_SHOW_CHART)
		{	
			m_showchart.LoadKeyWordCountChart(m_tDataSets.keyWordList);
			//m_pActiveXFlash->SetInternVisible(true);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到显示关键词变化图表数据flash消息!"));
			return 0;
		}
		
		//启动时显示曝光量控件
		if (uMsg == MSG_SHOW_EXPOSURECHART)
		{
			m_showchart.LoadExposureCountChart(m_tDataSets.productList);
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到显示曝光量图表数据消息!"));
			return 0;
		}
		//模态弹出对话框
		if (uMsg == WM_SHOW_MODAL)
		{	
			if (m_dwClientType != MAIN_LINE_VERSION)
			{	
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("客户端为定制版本，屏蔽弹框！"));
				HidePopWindow();
				return 0;
			}

			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到对话框弹出消息,wParam=%d!"),wParam);
			OnPopDlg(wParam,lParam);
			return 0;
		}
		//tab键处理
		if (uMsg == WM_TAB_KEY_PRESS)
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到Tab键消息!"));
			OnTableKeys(wParam,lParam);
			return 0;
		}
		// enter键处理
		if (uMsg == WM_ENTER_KEY_PRESS)
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到Enter键消息!"));
			OnEnterKeys(wParam,lParam);
			return 0;
		}
		//屏幕alt+f4键
		if (uMsg == WM_HOTKEY)
		{	
			return 0;
		}
		//add by zhumingxing 20140618 【新建exploer进程重新显示托盘】
		if (uMsg == WM_TASKBARCREATED)
		{
			OperateTray(NIM_ADD);
			return 0;
		}
		//end add
		//当用户右键任务栏窗口关闭时需要将相应的窗口全部关闭【每增加一个窗口都需要关闭】
		if (uMsg == WM_CLOSE)
		{	
			OnRBTaskClose(wParam,lParam);
			return 0;
		}
		//相关定时器消息集中处理
		if (uMsg == WM_TIMER)
		{	
			OnTimerMsg(wParam,lParam);
			//return 0;			//注意此处不能够return 否则会导致gif不能正常显示
		}
		//当下载安装包结束之后返回成功之后就可以弹出当前消息
		if (uMsg == MSG_DOWNLOAD_INSTALLEXE)
		{
			//下载安装包成功
			if (lParam == RET_SUCCESS)
			{	
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("收到下载安装包成功消息！"));

				DELAY_MESSAGE tempPostMessage = *((DELAY_MESSAGE*)wParam);
				HWND hwnd = ::FindWindow(NULL,_T("MasterZ_postmsgwnd"));

				if (hwnd == NULL)
				{
					CPostMsgWnd* pClientTips = new CPostMsgWnd();
					pClientTips->SetParentWnd(m_hWnd);
					pClientTips->SetPostInfo(tempPostMessage);

					pClientTips->Create(NULL, _T("MasterZ_postmsgwnd"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
					pClientTips->ShowWindow(FALSE);
				}
				else
				{	
					g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("当前有推送消息正在显示，添加到队列中下次显示！"));
					m_quePostMessage.push(tempPostMessage);
				}
			}
		}
		//当前消息通知关闭后，从队列里面取出一个消息显示然后移除
		if (uMsg == MSG_SHOW_NEXT_POSTMESSAGE)
		{	
			if (m_quePostMessage.size() == 0)
			{	
				return 0;
			}
			else
			{	
				CPostMsgWnd* pClientTips = new CPostMsgWnd();
				pClientTips->SetParentWnd(m_hWnd);
				pClientTips->SetPostInfo(m_quePostMessage.front());
				pClientTips->Create(NULL, _T("MasterZ_postmsgwnd"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
				pClientTips->ShowWindow(FALSE);

				m_quePostMessage.pop();			
			}	
		}
		//此处新增对静默推送消息处理
		if (uMsg == MSG_DOWNLOAD_SILENT_EXE)
		{
			if (lParam == RET_SUCCESS)
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("收到静默推送下载安装包成功消息！"));
				
				m_bPostInstall = TRUE;
				::KillTimer(m_hWnd, ID_CHECK_SILENT_INTSTALL);
				::SetTimer(m_hWnd, ID_CHECK_SILENT_INTSTALL, 500, NULL);
			}
			else
			{	
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("收到静默推送下载安装包失败消息！"));
				::KillTimer(m_hWnd, ID_CHECK_SILENT_INTSTALL);
			}
		}
		//屏蔽对标题栏的双击操作 
		if(WM_NCLBUTTONDBLCLK != uMsg)
			return __super::HandleMessage(uMsg, wParam, lParam);
		else
			return 0;
	}
	catch (...)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("捕获到了消息处理异常,消息id为%d"),uMsg);
	}	

}

LRESULT CMainWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
	switch (uMsg)
	{
		//取消登录消息
	case MSG_LOGIN_CANCEL:
		{
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_LOGIN_CANCEL,0,0);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("取消用户登录消息已经由界面发出!"));
		}
		break;
		//处理托盘系统消息
	case WM_TRAY_MSG:
		{	
			OnOpearateTray(wParam,lParam);
		}
		break;
		//处理托盘菜单消息
	case MSG_TRAY_FUNCTION_MSG:
		{
			OnTaryMenue(wParam,lParam); 
		}
		break;
		//升级消息
	case MSG_PRODUCT_UPDATE:
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到升级模块返回的正常升级消息!"));
			OnUpdateMessage(wParam,lParam);
		}
		break;
		//取消升级消息
	case MSG_CANCEL_UPDATE:
		{	
			//取消升级，后设置升级进度为0
			m_bIsCancelUpdate = TRUE;
			m_dwUpdateProgress = 0;
			PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_MGR,MSG_CANCEL_UPDATE,0,0);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户取消升级消息已经由界面发出"));

			if (m_wCurrentUpdateFlag == WPARAM_REPAIR_UPDATE)
			{
				OnHandleProRepairMsg(RET_ERROR,WPARAM_REPAIR_UPDATE);
			}
		}
		break;
		//最新版本---当用户为最新版本时才去刷新排名
	case MSG_LATEST_VERSION:
		{	
			OnNewVerson(wParam,lParam);
		}	
		break;
		//升级出现错误
	case MSG_UPDATE_ERROR:
		{	
			OnUpdateError(wParam,lParam);
		}
		break;
		//垃圾清理消息
	case MSG_SYSTEM_CLEAR:
		{	
			OnClearMessage(wParam,lParam,CLEAR_SYESTEM_RUBISH);
		}
		break;
		//快照清理消息
	case MSG_CLEAR_QUICK_PHOTOS:
		{	
			OnClearMessage(wParam,lParam,CLEAR_PHOTO);
		}
		break;
		//综合网站查询消息
	case MSG_WEB_SEARCH:
		{	
			OnHandleWebSearchData(wParam,lParam);		
		}
		break;
		//取消网站综合查询
	case MSG_CANCEL_WEB_SEARCH:
		{	
			OnCancelWebSearchData(wParam,lParam);
		}
		break;
		//关键词分析
	case  MSG_KEYWORD_ANALYSIS:
		{	
			OnKeyWordAnysis(wParam,lParam);
		}
		break;
		//取消关键词分析
	case MSG_CANCEL_KEYWORD_ANALYSIS:
		{	
			OnCancelKeyWorsAnysis(wParam,lParam);
		}
		break;
		//登录初始化处理消息，由登录框发出
	case MSG_SERVEREX_USER_VALIDATE:
		{	
			OnInitialLogin(wParam,lParam);
		}
		break;
		//登录消息
	case  MSG_LOGIN_CLIENT:
		{	
			OnLoginMessage(wParam,lParam);
		}
		break;
		//上线关键词图表、曝光量图表、关键词数据统计消息数据处理
	case MSG_KEYWORD_PRODUCT_DATA:
		{	
			OnKeyWordData(wParam,lParam);
		}
		break;
		//关键词排名效果数据消息
	case MSG_KEYWORD_RESULT_DATA:
		{	
			OnLoadKeyWordRank(wParam,lParam);
		}
		break;
		//获取检测关键词刷新模块状态
	case MSG_CHECK_REFRESH_KEYWORD_STATA:
		{
			OnHandleRefreshState(wParam,lParam);
		}
		break;
		//收到关键词刷新模块返回的当前状态消息
	case MSG_AUTO_REFRESH_KEYWORD:
		{
			OnHanleRefreshKeyWord(wParam,lParam);
		}
		break;
		//微信通数据处理
	case MSG_WEIXIN_DATA:
		{
			OnWeixinDate(wParam,lParam);
		}
		break;
		//处理建站系统数据
	case MSG_GET_JZ_INFORMATION:
		{	
			OnHandleJZData(wParam,lParam);
		}
		break;
		//气泡消息
	case MSG_TIP_MSG:
		{	
			OnTimerTips(wParam,lParam);	
		}
		break;
		//新增推送通知消息处理-----20141117
	case MSG_GET_DELAYMSG:
		{
			OnHandlePostMessage(wParam,lParam);
		}
		break;
		//新增升级校验消息处理
	case MSG_UPDATE_SUCCESS_CHECK:
		{
			OnHandleUpdateCheck(wParam,lParam);
		}
		break;
		//新增处理与服务器互通消息
	case MSG_CONNECT_SERV_WITH_UDP:
		{
			OnHandleServerMsg(wParam,lParam);
		}
		break;
		//检测到注册表支持版本或者是用户名发生变化
	case MSG_CHECK_REG:
		{
			OnHandleRegChange(wParam,lParam);
		}
		break;
		//新增舟大师诊断问题列表获取消息----20150423
	case MSG_READ_SELF_DIAGNOSIS_XML:
		{
			OnHandleGetProlemList(wParam,lParam);
		}
		break;
		//新增舟大师诊断消息处理------20150423
	case MSG_SELF_DIAGNOSIS:
		{
			OnHandleCheckProMsg(wParam,lParam);
		}
		break;
		//新增舟大师修复消息处理------20150423
	case MSG_REPAIR_FAULT:
		{
			OnHandleProRepairMsg(wParam,lParam);
		}
		break;
		//取消诊断及修复消息
	case MSG_DIAGNOSIS_CANCEL:
		{
			PackagMessage(E_THREAD_CLEAR,E_THREAD_TYPE_MGR,MSG_DIAGNOSIS_CANCEL,0,0);
		}
		break;
		//收到web控件丢失消息，弹出诊断框，触发诊断消息
	case MSG_WEB_ACTIVEX_LOSE:
		{
			OnHandlePluginLose(wParam, lParam);
		}
		break;
	//case MSG_SUBMIT_USERINFO:
	//	{	
	//		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到提交用户信息失败消息，继续发送提交消息！"));
	//		PackagMessage(E_THREAD_DATASTATISTICS, E_THREAD_TYPE_UIHELPER, MSG_SUBMIT_USERINFO, 0, 0);
	//	}
	//	break;
	//甄词消息
	case MSG_ZHENCI_INFO:
		{
			OnHandleZhenciMsg(wParam, lParam);
		}
		break;
	//开始刷5*20排名消息
	case MSG_START_PAIMING:
		{
			OnHandlePaimingMsg(wParam,lParam);
		}
		break;
	default:
		break;
	}
	return 0;
}

void CMainWnd::Notify(TNotifyUI& msg)
{	
	if (msg.sType == _T("click"))
	{	
		//点击关闭
		if (msg.pSender->GetName() == _T("closebtn_index"))
		{	
			::ShowWindow(m_hWnd,SW_HIDE);
		}
		//点击升级
		if (msg.pSender == m_pBtnChkUpdate)
		{	
			OnNotifyInitialUpdate(msg);
		}
		//登录
		if (msg.pSender == m_pBtnLogin)
		{	
			OnNotifyLoginCilck(msg);
		}
		//设置
		if (msg.pSender->GetName() == _T("setbtn"))
		{
			OnNotifySetting(msg);
		}
		//网站综合查询
		if (msg.pSender->GetName() == _T("btn_search_start"))
		{	
			//设置焦点到按钮上，避免出现焦点在登录框上的异常情况
			msg.pSender->SetFocus();
			OnNotifySearch(msg);
		}
		//取消网站综合查询
		if (msg.pSender->GetName() == _T("btn_search_cancel"))
		{			
			PackagMessage(E_THREAD_UTILITY, E_THREAD_TYPE_UIHELPER, MSG_CANCEL_WEB_SEARCH, 0, 0);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("取消网站综合查询消息已经由界面发出!"));
		}
		//关键词分析
		if (msg.pSender->GetName() == _T("btn_search_anysis"))
		{	
			msg.pSender->SetFocus();
			OnNotifyKeyWordAnysis(msg);		
		}
		//取消关键词分析
		if (msg.pSender->GetName() == _T("btn_aynsis_cancel"))
		{	
			PackagMessage(E_THREAD_UTILITY, E_THREAD_TYPE_UIHELPER, MSG_CANCEL_KEYWORD_ANALYSIS, 0, 0);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("取消关键词分析消息已经由界面发出!"));
		}
		//清理完毕之后的返回按钮
		if (msg.pSender->GetName() == _T("btn_clear_back"))
		{	
			OnNotifyClearBack(msg);		
		}
		//切换option操作
		if (msg.pSender->GetName().Find(_T("option_")) != -1) 
		{	
			OnNotifyClickOption(msg);
		}
		//立即开通微信通
		if (msg.pSender->GetName() == _T("btnregistweixin"))
		{	
			OnNotifyBuyProduct(PRODUCT_WEIXIN);
		}
		//开通建站系统
		if (msg.pSender->GetName() == _T("btnregistjz"))
		{	
			OnNotifyBuyProduct(PRODUCT_JZ);
		}
		//立即绑定
		if (msg.pSender->GetName() == _T("jzbindbutton"))
		{	
			if (!m_TjzData.strBindUrl.IsEmpty())
			{
				ShellExecute(NULL, _T("open"), m_TjzData.strBindUrl, NULL, NULL, SW_NORMAL);
			}
		}
		//进入建站系统
		if (msg.pSender->GetName() == _T("btnEnterJz"))
		{
			if (!m_TjzData.strAccessUrl.IsEmpty())
			{
				ShellExecute(NULL, _T("open"), m_TjzData.strAccessUrl, NULL, NULL, SW_NORMAL);
			}
		}	
		//点击网站地址
		if (msg.pSender->GetName() == _T("jzweburl"))
		{
			if (!m_TjzData.strWebSiteUrl.IsEmpty())
			{	
				if (m_TjzData.strWebSiteUrl.Find(_T("http://")) == 0)
				{
					ShellExecute(NULL, _T("open"), m_TjzData.strWebSiteUrl, NULL, NULL, SW_NORMAL);
				}
				else
				{	
					CString strURL = _T("");
					strURL.Format(_T("http://%s"),m_TjzData.strWebSiteUrl);
					ShellExecute(NULL, _T("open"),strURL, NULL, NULL, SW_NORMAL);
				}				
			}
		}	

		//点击绑定域名
		if (msg.pSender->GetName() == _T("jzbindDomain"))
		{
			if (!m_TjzData.strBindDomain.IsEmpty())
			{	
				if (m_TjzData.strBindDomain.Find(_T("http://")) == 0)
				{
					ShellExecute(NULL, _T("open"), m_TjzData.strBindDomain, NULL, NULL, SW_NORMAL);
				}
				else
				{	
					CString strURL = _T("");
					strURL.Format(_T("http://%s"),m_TjzData.strBindDomain);
					ShellExecute(NULL, _T("open"),strURL, NULL, NULL, SW_NORMAL);
				}				
			}
		}	

		//登录微信通
		if (msg.pSender->GetName() == _T("btnloginweixin"))
		{	
			OnNotifyEnterWeixin(msg);
		}
		//进入舟大师产品推广
		if (msg.pSender->GetName() == _T("btnposssumsz2"))
		{	
			OnNotifyEnterPost(msg);
		}
		//进入舟大师产品展示
		if (msg.pSender->GetName() == _T("btnshowsumsz2"))
		{
			OnNotifyEnterShow(msg);
		}
		//关键词排名上一页
		if (msg.pSender->GetName() == _T("btnpageup"))
		{		
			int icursel = m_pPageCombox->GetCurSel();
			if (icursel == 0)
			{
				return;
			}
			else
			{	
				m_pPageCombox->SelectItem(icursel-1,true);
			}
		}
		//关键词排名下一页
		if (msg.pSender->GetName() == _T("btnpagedown"))
		{	
			int icursel = m_pPageCombox->GetCurSel();
			if (icursel == m_iPage-1)
			{
				return;
			}
			else
			{		
				m_pPageCombox->SelectItem(icursel+1,true);
			}
		}
		//查看快照
		if (msg.pSender->GetName().Find(_T("rank_check")) != -1)
		{
			OnNotifyCheckSnap(msg);
		}
		//立即刷新排名-----add by zhumingxing 20150429
		if (msg.pSender->GetName() == _T("btn_rank_refresh"))
		{	
			if(m_bIsNewestVersion)
			{	
				//暂时去掉立即刷新功能，立即刷新不清排名，就是普通的请求刷新排名
				StartReFreshKeyWordRank(/*START_REFRESH_KEYWORD_MSG*/);
			}
			else
			{	
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("当前客户端版本不是最新版本无法进行立即刷新关键词排名操作!"));
				return;
			}
		}

	}
	else if (msg.sType == _T("selectchanged"))
	{	
		OnNotifySelectchanged(msg);
	}
	//切换关键词排名页数时触发此事件
	else if (msg.sType == _T("itemselect"))
	{	
		if (msg.pSender->GetName() == _T("page_select"))
		{	
			ShowRankPageData(msg.wParam);
		}
	}
	//窗口初始化后的第一条消息
	else if (msg.sType == _T("windowinit"))
	{
		OnNotifyWindowsInitial(msg);
	}
	__super::Notify(msg);
}

void CMainWnd::OnPrepare(TNotifyUI& msg)
{

}

void CMainWnd::OnExit(TNotifyUI& msg)
{
	Close();
}

bool CMainWnd::InitComponent(void)
{	

	//创建线程管理对象并启动UIHelper
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("开始装载组件!"));
	m_threadMgr.Create();
	m_pUiHelper = new CUiHelper();

	if (NULL == m_pUiHelper)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("装载UIhelper线程模块失败!"));
	}
	if (m_hWnd)
	{	
		m_pUiHelper->SetUiWnd(m_hWnd);
	}
	else
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("装载UIhelper时主窗口句柄无效!"));
	}
	m_threadMgr.InsertThread(m_pUiHelper);
	g_globalData.sqlIte.InitDb();

	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\component.dat"), GetInstallPath());
	IXMLRW iniFile;
	iniFile.init(strCfgFile);

	for(int i = 0; i < 10; i++)
	{
		TCHAR strField[200] = {0};
		CString strFile = _T("");
		wsprintf(strField,_T("THREAD%d"),i);

		int nCount = 0;
		iniFile.ReadInt(_T("component"),strField, _T("count"), nCount);
		iniFile.ReadString(_T("component"),strField, _T("file"), strFile);

		if(nCount > 0)
		{
			//加载功能组件
			HMODULE hDll = LoadLibrary(strFile.GetBuffer());
			if (!hDll)
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__,__LINE__, _T("加载组件%s失败！err=%d"), CString(strFile), GetLastError());
				continue;
			}

			GETITHREADOBJECT pfnCreateThread = (GETITHREADOBJECT)GetProcAddress(hDll, "GetIThreadObject");
			if (!pfnCreateThread)
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("获取组件%s中的GETITHREADOBJECT接口失败！"), CString(strFile));
				continue;
			}

			if (!m_threadMgr.InsertThread(pfnCreateThread(&g_globalData)))
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("创建组件%s线程失败，请检查线程是否正确！"), CString(strFile));
				continue;
			}
		}
	}

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("装载组件完成!"));

	return true;
}

BOOL CMainWnd::InitialOpearate()
{	
	//初始化窗口
	InitialWindowUI();

	//初始化Tab控件,初始化关键词排名页面相关控件等
	InitialCtrlUI();

	//初始化托盘
	OperateTray(NIM_ADD);

	//启动测试服务---add by qy 2018.6.11
	CString strAppPath;
	strAppPath.Format(_T("%s\\bin\\RunService.bat"), GetInstallPath());
	if (PathFileExists(strAppPath))
	{
		ShellExecute(NULL, _T("open"), strAppPath, _T(""), NULL, SW_HIDE);
	}

	//发送CtrlUserTask.exe启动消息
	::SetTimer(m_hWnd, ID_START_CTRLUSERTASK_MSG, 10 * 1000, NULL);


	//发送升级校验和后台检测升级消息
	AutoMsgSend();

	//设置定时器,1.5min中之后首次请求最新推送消息----新修改为50s
	::SetTimer(m_hWnd,ID_TIMING_GET_MESSAGE,50*1000,NULL);

	//设置定时器，10分钟后弹出托盘提示甄词信息
	::SetTimer(m_hWnd,ID_ZHENCI_TIPS,10*60*1000,NULL);

	//检测是否为首次安装登录
	if (!CheckInstallLogin())
	{
		//进行自动登录判断
		CheckAutoLogin();
	}

	//打印当前系统基本信息
	PrintCurrSysInfo();

	//开始监测注册表变化---------20150312
	PackagMessage(E_THREAD_CLEAR,E_THREAD_TYPE_MGR,MSG_CHECK_REG,0,0);

	//自动检测插件丢失
	PackagMessage(E_THREAD_CLEAR, E_THREAD_TYPE_MGR, MSG_CHECK_PLUGINS, 0, 0);

	return TRUE;
}

//托盘操作
void CMainWnd::OperateTray(DWORD dwType)
{	
	if (m_dwClientType != MAIN_LINE_VERSION)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("客户端为定制版本，无需对托盘进行操作!"));
		return;
	}
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("开始初始化托盘！"));
	m_NotifyIcon.cbSize=sizeof(NOTIFYICONDATA);
	m_NotifyIcon.hWnd=m_hWnd;
	m_NotifyIcon.uID = IDI_MasterZ;
	m_NotifyIcon.hIcon = ::LoadIcon(m_PaintManager.GetInstance(),MAKEINTRESOURCE(IDI_MasterZ));
	m_NotifyIcon.uCallbackMessage = WM_TRAY_MSG;
	m_NotifyIcon.uFlags= NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_NotifyIcon.dwInfoFlags = NIIF_INFO; 

	if (dwType == NIM_ADD)
	{
		_tcscpy((LPTSTR)m_NotifyIcon.szTip,m_strWindowsName);
		Shell_NotifyIcon(NIM_ADD,&m_NotifyIcon);
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("初始化托盘完成！"));
	}
	if (dwType == NIM_DELETE)
	{
		Shell_NotifyIcon(NIM_DELETE,&m_NotifyIcon);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("托盘删除完成！"));
	}
}

//设置托盘气泡消息
void CMainWnd::setTrayTips(CString strTipContent, CString strTipTitle, CString strTipInfo, int ioutTime)
{	
	if (m_dwClientType != MAIN_LINE_VERSION)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("客户端为定制版本，因此没有托盘，无需弹出气泡!"));
		return;
	}

	//先清空
	_tcscpy((LPTSTR)m_NotifyIcon.szTip, m_strWindowsName);
	_tcscpy((LPTSTR)m_NotifyIcon.szInfo, _T(""));
	_tcscpy((LPTSTR)m_NotifyIcon.szInfoTitle, _T(""));

	//然后重新赋值
	m_NotifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	_tcscpy((LPTSTR)m_NotifyIcon.szTip, m_strWindowsName);
	_tcscpy((LPTSTR)m_NotifyIcon.szInfo, strTipContent);
	_tcscpy((LPTSTR)m_NotifyIcon.szInfoTitle, strTipTitle);
	m_NotifyIcon.dwInfoFlags = NIIF_INFO; 
	Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIcon); 

	::SetTimer(m_hWnd,ID_KILLTIPS,ioutTime,NULL);
}

bool CMainWnd::setVersion()
{	
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("开始设置主界面程序版本！"));
	CTextUI* pVersion = (CTextUI*)m_PaintManager.FindControl(_T("textVersions"));

	CReg reg;
	m_strClientVersion = (TCHAR*)reg.ReadValueOfKey(REG_USER_ROOT, _T("Software\\szw\\MasterZ\\Setup"), REG_KEY_VERSION);
	CString strInfo;
	strInfo.Format(_T("主程序版本： %s"),m_strClientVersion);

	pVersion->SetText(strInfo);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("主界面程序版本设置完成！"));
	return true;
}

void CMainWnd::PackagMessage(DWORD dwDestThread,DWORD dwSourceThread,DWORD dwMessageType,WPARAM wParam, LPARAM lParam,bool bIsSync,bool bIsNeedChangeTime)
{	
	T_Message *tMsg = IMsgQueue::New_Message();
	tMsg->dwDestWork = dwDestThread;
	tMsg->dwSourWork = dwSourceThread;
	tMsg->dwMsg = dwMessageType;	//发送初始化消息
	tMsg->wParam = wParam;
	tMsg->lParam = lParam;

	//此处统一修改最新版本标志，每次升级之前将最新版本修改为不是最新
	if (dwMessageType == MSG_PRODUCT_UPDATE)
	{
		m_bIsNewestVersion = FALSE;					//初始化版本最新标志
	}
	else if (dwMessageType == MSG_LOGIN_CLIENT)    //定制版不登录
	{
		if (m_dwClientType != MAIN_LINE_VERSION)
		{
			return;
		}
	}
	//需要动态改变时间戳参数
	if (bIsNeedChangeTime)
	{
		//实时获取时间戳参数
		tuserInfotodb temptb;
		DecodeString(m_tUserInfo.strParam,temptb);
		GetEncodeDate(temptb.strUserName,temptb.strPassWord,m_tUserInfo.strParam);
	}
	if (!bIsSync)
	{
		m_threadMgr.PostMessage(tMsg);
	}
	else
	{
		m_threadMgr.SendMessage(tMsg);
	}
}

//首次安装登录检测
BOOL CMainWnd::CheckInstallLogin()
{
	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\bin\\install.flag"), GetInstallPath());

	if (_taccess(strCfgFile,0))
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("install标志文件不存在，用户不是首次安装登录！"));
		return FALSE;
	}
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("install标志文件存在，用户首次安装登录！"));

	//取出注册表中的用户名
	CReg reg;
	TCHAR* pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),_T("SupportProduct"));
	if (pContent == NULL)
	{	
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("安装登录注册表Software\\szw\\MasterZ\\SupportProduct版本支持为空！"));
		return FALSE;
	}
	else
	{	
		TCHAR* pUserName = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),pContent);

		if (pUserName == NULL)
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("安装登录注册表Software\\szw\\MasterZ\\%s用户名为空!"),pContent);
			return FALSE;
		}
		//发送安装登录消息给登录模块
		//add by zhumingxing 20140909 设置自动登录标志为true
		m_bIsAutoLogin = true;

		ChangeLoginfo(m_tUserInfo, _T(""),EncodeString(CString(pUserName)).GetBuffer());
		PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_LOGIN_CLIENT,WPAPAM_INSTALL_LOGIN, (LPARAM)(&m_tUserInfo));
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("首次安装登录消息已经由界面发出,wparam = 1"));
	}
	return TRUE;
}

//自动登录检测
void CMainWnd::CheckAutoLogin()
{	
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("开始进行自动登录判定！"));
	CStdString strUrl = _T("");
	CString strUserName, strPassWord= _T("");
	int iAutoFlag,iSavePassWord = -1;

	if (!ReadUserInfo(strUserName,strPassWord,iAutoFlag,iSavePassWord))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("从数据库中读取用户账户信息失败！"));

		//弹出登录对
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		return;
	}	

	//如果用户选择了自动登录且账号密码不空则发送自动登录消息
	if (iAutoFlag == 1 && !strUserName.IsEmpty() && !strPassWord.IsEmpty())
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户需要进行自动登录!"));

		//取出注册表中的用户名
		TCHAR* pUserName = GetRegCurrentUserName();

		if (pUserName == NULL)
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("注册表中用户名获取失败为空!"));
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
			return ;
		}

		//add by zhumingxing 20140909 设置自动登录标志为true
		m_bIsAutoLogin = true;

		ChangeLoginfo(m_tUserInfo, _T(""),EncodeString(CString(pUserName)).GetBuffer());
		PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_LOGIN_CLIENT,WPARAM_AUTO_LOGIN, (LPARAM)(&m_tUserInfo));
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("首次安装登录消息已经由界面发出,wparam = 1"));
	}
	else
	{	
		//非自动登录
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户需要进行手动登录！"));
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
	}
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("自动登录判定完成！"));
}

//发送云任务消息
void CMainWnd::SendYunMsg()
{
	CReg reg;
	TCHAR* pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("SupportProduct"));
	if (pContent == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("注册表Software\\szw\\MasterZ\\SupportProduct版本支持为空,无法进行刷新关键词排名操作!"));
		/*return;*/
	}

	TCHAR* pUserName = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), pContent);
	if (pUserName == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("注册表Software\\szw\\MasterZ\\%s用户名为空,无法进行刷新关键词排名操作!"), pContent);
		/*return;*/
	}

	CString strVersion(pContent);
	if (strVersion.Find(_T("_")) == -1)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("注册表中支持版本格式有误，未找到下划线标示符,无法刷新排名!supportProduct:%s"), strVersion);
		/*return;*/
	}
	else
	{
		strVersion = strVersion.Mid(strVersion.ReverseFind(_T('_')) + 1);
		m_wVersionId = _ttoi(strVersion);
		m_tUserInfoFreshWord.strUserName = EncodeString(CString(pUserName));
	}
	//发送消息
	PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_YUN_TASK, m_wVersionId, (LPARAM)&m_tUserInfoFreshWord);
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("云任务消息已经发出！"));

}

//客户端启动刷新关键词检测
void CMainWnd::StartReFreshKeyWordRank (WORD wSendkeyWordFlag)
{
	
	CReg reg;
	TCHAR* pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),_T("SupportProduct"));
	if (pContent == NULL)
	{	
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("注册表Software\\szw\\MasterZ\\SupportProduct版本支持为空,无法进行刷新关键词排名操作!"));
		return;
	}

	TCHAR* pUserName = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),pContent);
	if (pUserName == NULL)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("注册表Software\\szw\\MasterZ\\%s用户名为空,无法进行刷新关键词排名操作!"),pContent);
		return;
	}

	CString strVersion(pContent);
	if (strVersion.Find(_T("_")) == -1)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("注册表中支持版本格式有误，未找到下划线标示符,无法刷新排名!supportProduct:%s"),strVersion);
		return;
	}
	else
	{
		strVersion = strVersion.Mid(strVersion.ReverseFind(_T('_')) + 1);
		m_wVersionId = _ttoi(strVersion);
		m_tUserInfoFreshWord.strUserName = EncodeString(CString(pUserName));
	}

	//如果用户名不为空而且ID不为0则发送刷新关键词排名消
	/*ShowRankUiByState(RANKUI_RUNNING);*/

	//普通刷新排名消息
	DWORD dwRefreshKeyWord = 0;
	if (wSendkeyWordFlag == NORMAL_KEYWORD_MSG)
	{
		memcpy(&dwRefreshKeyWord,&m_wVersionId,2);
	}
	else
	{
		WORD wRefreshFlag = 1;
		memcpy(&dwRefreshKeyWord,&m_wVersionId,2);
		memcpy((char*)&dwRefreshKeyWord + 2,&wRefreshFlag,2);
	}
	
	PackagMessage(E_THREAD_YUN_TASK,E_THREAD_TYPE_UIHELPER,MSG_AUTO_REFRESH_KEYWORD,dwRefreshKeyWord,(LPARAM)&m_tUserInfoFreshWord);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("刷新关键词排名消息已经由界面发出！"));
}

//新增发送推送消息
void CMainWnd::SendPostMsg()
{	
	//begin add by zhumingxing 20150618
	if (m_dwClientType != MAIN_LINE_VERSION)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("客户端为定制版本，无需发送推送消息!"));
		return;
	}
	//end add

	CString strParam = _T("");

	UserInfo tuser;
	g_globalData.sqlIte.GetUserInfo(tuser);

	CString strUserName = (CString)DecodeString((CString)(tuser.szUName));
	CString strMac = GetPhysicalAddress();

	//用户名为空，则去掉用户名参数/*%d*/,GetRand(10000000,5)
	if (strUserName.IsEmpty())
	{
		strParam.Format(_T("userName=&versionNumber=%s&mac=%s"),m_strClientVersion,strMac);
	}
	else
	{
		strParam.Format(_T("userName=%s&versionNumber=%s&mac=%s"),URLEncode(strUserName),m_strClientVersion,strMac);
	}

	//获取URL
	CStdString strURL = _T("");
	GetURL(strURL);

	m_tPostMsgInfo.strUrl = strURL.GetBuffer();
	m_tPostMsgInfo.strParam =strParam;
	m_tPostMsgInfo.strUserName = strUserName;

	PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_GET_DELAYMSG,0,(LPARAM)&m_tPostMsgInfo);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取舟大师推送消息已经由界面发出！"));
}

void CMainWnd::SendConnectSerMsg()
{
	CString strParam = _T("");
	m_tConnectServer.strUserName = EncodeString(CString(GetRegCurrentUserName()));

	PackagMessage(E_THREAD_YUN_TASK,E_THREAD_TYPE_UIHELPER,MSG_CONNECT_SERV_WITH_UDP,0,(LPARAM)&m_tConnectServer);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("与服务器连通消息已经由界面发出！"));

}

void CMainWnd::AutoMsgSend()
{	
	
	//add by zhumingxing 20150423----获取问题列表信息
	PackagMessage(E_THREAD_CLEAR,E_THREAD_TYPE_UIHELPER,MSG_READ_SELF_DIAGNOSIS_XML,0,0);
	//end add

	//一个小时之后开启启动刷排名，失败的话，十分钟执行一次
	m_iTimeCnt = 0;
	::SetTimer(m_hWnd, ID_PAIMING_MSG, 10*60*1000, NULL);
/*
	PackagMessage(E_THREAD_UTILITY, E_THREAD_TYPE_UIHELPER, MSG_CLEAR_CODE, 0, 0);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("清理验证码图片消息已经发出！"));*/

	//清理网页快照文件
	PackagMessage(E_THREAD_UTILITY, E_THREAD_TYPE_MGR, MSG_CLEAR_QUICK_PHOTOS, 0, 0);
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("快照垃圾清理消息已经由界面发出!"));

	//当检测到安装目录下面有filelist_new.dat时暂时不进行自动升级，等升级校验完成后再进行自动升级请求
	TCHAR szXMLFile[MAX_PATH] = {0};
	_sntprintf(szXMLFile, _TRUNCATE, _T("%s\\bin\\filelist_new.dat"), g_globalData.dir.GetInstallDir());

	if (!_taccess(szXMLFile,0))
	{
		PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_UIHELPER,MSG_UPDATE_SUCCESS_CHECK,0,0);
		m_bIsUpdateChking = TRUE;
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检测到了filelist_new需要做升级成功校验!消息已经由界面发出！"));
	}
	else
	{	
		GetEncodeUpdateData(m_strUpdateEncodeData,m_dwClientType);
		PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_UIHELPER,MSG_PRODUCT_UPDATE,WPARAM_AUTOCHECK_UPDATE,(LPARAM)&m_strUpdateEncodeData);
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("自动升级检测消息已由界面发出MSG_PRODUCT_UPDATE wparm=102！"));
	}

	//启动SyncDat.exe进行data文件下载
	if (GetUpdateDatCfg())
	{
		CString strAppPath;
		strAppPath.Format(_T("%s\\bin\\SyncDat.exe"), GetInstallPath());
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("SyncDat.exe完整路径为:%s"), strAppPath);
		ShellExecute(NULL, _T("open"), strAppPath, _T(""), NULL, SW_HIDE);
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("配置文件中配置无需客户端启动SyncDat.exe!"));
	}
}

void CMainWnd::ClearRecodeOfSearch()
{	
	CString strTextName;
	for (int m = 1;  m <= 16; ++ m)
	{
		strTextName.Format(_T("websitesearchText_%d"),m);
		CTextUI* pText = static_cast<CTextUI*>(m_PaintManager.FindControl(strTextName));
		pText->SetText(_T(""));
	}
	CTextUI* pText = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("websitesearchText")));
	pText->SetText(_T(""));
}

//初始化话窗口相关属性
void CMainWnd::InitialWindowUI()
{	
	//记录客户端启动时间
	m_strClientStartTime = GetLocalTimeFormat();
	::SetTimer(m_hWnd,ID_AUTO_SEND_MESSAGE,(5*60*1000),NULL);		

	//设置窗口名称
	CString strCfgFile = _T("");
	CString strWindowsName = _T("");
	strCfgFile.Format(_T("%s\\data2\\UpdateOL.dat"), GetInstallPath());

	//不存在该文件
	if (_taccess(strCfgFile,0) != 0)
	{	
		m_strWindowsName = _T("商务快车");
		return;
	}

	IXMLRW iniFile;
	iniFile.init(strCfgFile);
	iniFile.ReadString(_T("UpdateOL"),_T("product"), _T("Name"), strWindowsName);

	if (strWindowsName.IsEmpty())
	{	
		m_strWindowsName = _T("商务快车");
		return;
	}
	else
	{
		m_strWindowsName = strWindowsName;
	}

	//设置窗口钩子
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("开始装载键盘钩子！"));
	//屏蔽alt+f4键
	RegisterHotKey(m_hWnd,ID_VKF4,MOD_ALT,VK_F4);

	g_hHwndMain = this->GetHWND();  
	g_KbHookData.nType = WH_KEYBOARD;  
	g_KbHookData.hkProc = KeyboardProc;  

	g_KbHookData.hHook = ::SetWindowsHookEx(   
		g_KbHookData.nType,  
		g_KbHookData.hkProc,   
		(HINSTANCE)NULL,  
		GetCurrentThreadId());

	//g_hCBT = SetWindowsHookEx(WH_CBT, (HOOKPROC)CBTProc, 0, GetCurrentThreadId());

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("装载键盘钩子完成！"));

	//设置应用程序版本号
	setVersion();
}

//初始化关键词分析界面的控件，用作TAB键切换
void CMainWnd::InitialCtrlUI()
{	
	CString str = _T("");
	CTextUI* pText = NULL;
	/*m_vecRankText.clear();
	m_vecRankButton.clear();
	m_EditKeyWord.clear();

	//关键词分析控件装载，用于进行tab键切换
	for (int i = 1; i <= 6; ++i)
	{	
		str.Format(_T("word%d"),i);
		CEditUI* pEdit = (CEditUI*)m_PaintManager.FindControl(str);
		m_EditKeyWord.push_back(pEdit);

	}
	//关键词排名相关控件装载，用于排名信息切换
	for (int m = 0; m <= 7; ++m)
	{
		str.Format(_T("rank_check%d"),m);
		CButtonUI* pButton = (CButtonUI*)m_PaintManager.FindControl(str);
		m_vecRankButton.push_back(pButton);
	}
	for (int n = 0; n <=7; ++n)
	{
		str.Format(_T("rank_keyword%d"),n);
		pText = (CTextUI*)m_PaintManager.FindControl(str);
		m_vecRankText.push_back(pText);

		str.Format(_T("rank_search%d"),n);
		pText = (CTextUI*)m_PaintManager.FindControl(str);
		m_vecRankText.push_back(pText);

		str.Format(_T("rank_page%d"),n);
		pText = (CTextUI*)m_PaintManager.FindControl(str);
		m_vecRankText.push_back(pText);

		str.Format(_T("rank_date%d"),n);
		pText = (CTextUI*)m_PaintManager.FindControl(str);
		m_vecRankText.push_back(pText);
	}

	//设置关键词排名界面为初始化状态
	ShowRankUiByState(RANKUI_INITIAL);*/

	//获取相关控件指针
	m_pControl = (CTabLayoutUI*)m_PaintManager.FindControl(kTabLayOutControlName);
	m_pProgressTest = (CProgressUI *)m_PaintManager.FindControl(KClearProgress);
	m_pBtnChkUpdate = (CButtonUI *)m_PaintManager.FindControl(kChkUpdateControlName);
	m_pBtnLogin = (CButtonUI *)m_PaintManager.FindControl(kLoginControlName);
	m_pPageCombox = (CComboUI*)m_PaintManager.FindControl(KRankSelectControlName);
}

//根据不同状态切换关键词排名界面显示效果
void CMainWnd::ShowRankUiByState(DWORD wState)
{
	switch (wState)
	{
	case RANKUI_INITIAL:		//初始化状态
		{
			for (int iindex = 0; iindex < m_vecRankButton.size(); ++ iindex)
			{
				m_vecRankButton[iindex]->SetVisible(false);
			}
			for (int iindex1 = 1; iindex1 < m_vecRankText.size(); ++iindex1 )
			{
				m_vecRankText[iindex1]->SetVisible(false);
			}

			m_vecRankText[0]->SetText(_T("总共0条记录 , 当前1/0页"));  
			CHorizontalLayoutUI* pHLayout = (CHorizontalLayoutUI*)m_PaintManager.FindControl(_T("keywordtitle"));
			pHLayout->SetVisible(false);

		}
		break;
	case RANKUI_HIDE:		    //隐藏状态
		{
			for (int iindex = 0; iindex < m_vecRankButton.size(); ++ iindex)
			{
				m_vecRankButton[iindex]->SetVisible(false);
			}
			for (int iindex1 = 0; iindex1 < m_vecRankText.size(); ++iindex1 )
			{
				m_vecRankText[iindex1]->SetVisible(false);
			}
			CHorizontalLayoutUI* pHLayout = (CHorizontalLayoutUI*)m_PaintManager.FindControl(_T("keywordtitle"));
			pHLayout->SetVisible(true);
		}
		break;
	case RANKUI_RUNNING:		//正在获取状态
		{
			CButtonUI* pbtnRunning = (CButtonUI*)m_PaintManager.FindControl(_T("btn_keywordLoading"));
			pbtnRunning->SetVisible(true);
// 
// 			CLabelUI* PLable =  (CLabelUI*)m_PaintManager.FindControl(_T("rank_getnewest"));
// 			PLable->SetVisible(false);
// 
			CLabelUI* PLable =  (CLabelUI*)m_PaintManager.FindControl(_T("rank_getrunning"));
			PLable->SetVisible(true);
// 
// 			CButtonUI* pStartRefresh = (CButtonUI*)m_PaintManager.FindControl(_T("btn_rank_refresh"));
// 			pStartRefresh->SetVisible(false);

			PLable = (CLabelUI*)m_PaintManager.FindControl(_T("rank_showtips"));
			PLable->SetVisible(false);

		}
		break;
	case RANKUI_ERROR:		   //获取错误状态
		{
			for (int iindex = 0; iindex < m_vecRankButton.size(); ++ iindex)
			{
				m_vecRankButton[iindex]->SetVisible(false);
			}
			for (int iindex1 = 1; iindex1 < m_vecRankText.size(); ++iindex1 )
			{
				m_vecRankText[iindex1]->SetVisible(false);
			}

			m_vecRankText[0]->SetText(_T("总共0条记录 , 当前1/0页"));  

// 			CLabelUI* PLable =  (CLabelUI*)m_PaintManager.FindControl(_T("rank_getnewest"));
// 			PLable->SetAttribute(_T("text"),_T("提示：暂无最新关键词排名！"));
// 			PLable->SetVisible(true);
// 
// 			CButtonUI* pStartRefresh = (CButtonUI*)m_PaintManager.FindControl(_T("btn_rank_refresh"));
// 			pStartRefresh->SetVisible(false);
// 
			CButtonUI* pbtnRunning = (CButtonUI*)m_PaintManager.FindControl(_T("btn_keywordLoading"));
			pbtnRunning->SetVisible(false);
// 
			CLabelUI* PLable =  (CLabelUI*)m_PaintManager.FindControl(_T("rank_getrunning"));
			PLable->SetVisible(false);

			PLable = (CLabelUI*)m_PaintManager.FindControl(_T("rank_showtips"));
			PLable->SetVisible(true);

			CHorizontalLayoutUI* pHLayout = (CHorizontalLayoutUI*)m_PaintManager.FindControl(_T("keywordtitle"));
			pHLayout->SetVisible(false);
		}
		break;
	case RANKUI_SUCCESS:		//获取成功状态
		{	
			CString strLastUpdateTime = _T("");
// 			CButtonUI* pStartRefresh = (CButtonUI*)m_PaintManager.FindControl(_T("btn_rank_refresh"));
// 
// 			if (m_bIsNewestVersion)
// 			{
// 				strLastUpdateTime.Format(_T("提示：排名最新统计时间:%s 若排名有误请"),CString(m_tKeyWordRank.szKeyWordLastTime));	
// 
// 				pStartRefresh->SetVisible(true);
// 			}
// 			else
// 			{
// 				strLastUpdateTime.Format(_T("提示：排名最新统计时间:%s"),CString(m_tKeyWordRank.szKeyWordLastTime));	
// 				pStartRefresh->SetVisible(false);
// 			}
// 			CLabelUI* PLable =  (CLabelUI*)m_PaintManager.FindControl(_T("rank_getnewest"));
// 			PLable->SetAttribute(_T("text"),strLastUpdateTime);
// 			PLable->SetAttribute(_T("textcolor"),_T("#ff000000"));
// 			PLable->SetVisible(true);
			
			if (m_iNewKeywordCnt <= 0)
			{
				strLastUpdateTime = _T("请勿关闭客户端，系统正在工作中，请稍后查看最新上线关键词。");
			}
			else
			{
				strLastUpdateTime.Format(_T("舟大师为您上线%d个排名，请保持客户端开启状态，系统将实时推送最新上线关键词。"),m_iNewKeywordCnt);
			}

			CButtonUI* pbtnRunning = (CButtonUI*)m_PaintManager.FindControl(_T("btn_keywordLoading"));
			pbtnRunning->SetVisible(false);

			CLabelUI* PLable =  (CLabelUI*)m_PaintManager.FindControl(_T("rank_getrunning"));
			PLable->SetVisible(false);

			PLable = (CLabelUI*)m_PaintManager.FindControl(_T("rank_showtips"));
			PLable->SetAttribute(_T("text"), strLastUpdateTime);
			PLable->SetAttribute(_T("textcolor"), _T("#ffb7b5b5"));
			PLable->SetVisible(true);			
		}
		break;
	case RANKUI_LOGINOUT:     //用户注销登录状态
		{
// 			CLabelUI* PLable =  (CLabelUI*)m_PaintManager.FindControl(_T("rank_getnewest"));
// 			PLable->SetVisible(false);
// 
// 			CButtonUI* pStartRefresh = (CButtonUI*)m_PaintManager.FindControl(_T("btn_rank_refresh"));
// 			pStartRefresh->SetVisible(false);
// 
			CButtonUI* pbtnRunning = (CButtonUI*)m_PaintManager.FindControl(_T("btn_keywordLoading"));
			pbtnRunning->SetVisible(false);

			CLabelUI* PLable =  (CLabelUI*)m_PaintManager.FindControl(_T("rank_getrunning"));
			PLable->SetVisible(false);

			PLable = (CLabelUI*)m_PaintManager.FindControl(_T("rank_showtips"));
			PLable->SetVisible(false);
			ShowRankUiByState(RANKUI_INITIAL);
		}
		break;
	case RANKUI_SAVEOLD:     //保留原有数据状态
		{
// 			CLabelUI* PLable =  (CLabelUI*)m_PaintManager.FindControl(_T("rank_getnewest"));
// 			PLable->SetVisible(false);
// 
// 			CButtonUI* pStartRefresh = (CButtonUI*)m_PaintManager.FindControl(_T("btn_rank_refresh"));
// 			pStartRefresh->SetVisible(false);

			CButtonUI* pbtnRunning = (CButtonUI*)m_PaintManager.FindControl(_T("btn_keywordLoading"));
			pbtnRunning->SetVisible(false);

			CLabelUI* PLable = (CLabelUI*)m_PaintManager.FindControl(_T("rank_getrunning"));
			PLable->SetVisible(false);

			PLable = (CLabelUI*)m_PaintManager.FindControl(_T("rank_showtips"));
			PLable->SetVisible(true);
		}
		break;
	case RANKUI_EMPTY:
		{
			CButtonUI* pbtnRunning = (CButtonUI*)m_PaintManager.FindControl(_T("btn_keywordLoading"));
			pbtnRunning->SetVisible(false);

			CLabelUI* PLable = (CLabelUI*)m_PaintManager.FindControl(_T("rank_getrunning"));
			PLable->SetVisible(false);

			CString strTips = _T("请勿关闭客户端，系统正在工作中，请稍后查看最新上线关键词。");			
			PLable =  (CLabelUI*)m_PaintManager.FindControl(_T("rank_showtips"));
			PLable->SetAttribute(_T("text"),strTips);
			PLable->SetAttribute(_T("textcolor"),_T("#ffb7b5b5"));
			PLable->SetVisible(true);
			
			for (int iindex = 0; iindex < m_vecRankButton.size(); ++iindex)
			{
				m_vecRankButton[iindex]->SetVisible(false);
			}
			for (int iindex1 = 1; iindex1 < m_vecRankText.size(); ++iindex1)
			{
				m_vecRankText[iindex1]->SetVisible(false);
			}

			m_vecRankText[0]->SetText(_T("总共0条记录 , 当前1/0页"));
	
		}
		break;
	default:
		break;
	}

}

void  CMainWnd::ShowRankPageData(DWORD dwPage)
{	
	CString strFormat = _T("");
	if (dwPage == -1)
	{	
		return;
	}
	//新页数的数据段值
	int iszie = 0;			
	if ((dwPage+1)*8 < m_tKeyWordRank.keyList.size())
	{
		iszie = (dwPage+1)*8;
	}
	else
	{
		iszie = m_tKeyWordRank.keyList.size();
	}
	ShowRankUiByState(RANKUI_HIDE);

	for (int m = dwPage*8, n=0; m < iszie; ++m,++n)
	{	
		//关键词
		m_vecRankText[n*4+0]->SetVisible(true);
		m_vecRankText[n*4+0]->SetText(CString(m_tKeyWordRank.keyList[m].szKeyWordName));
		m_vecRankText[n*4+0]->SetToolTip(CString(m_tKeyWordRank.keyList[m].szKeyWordName));

		//引擎名
		m_vecRankText[n*4+1]->SetVisible(true);
		m_vecRankText[n*4+1]->SetText(CString(m_tKeyWordRank.keyList[m].szSearchEngineName));

		//排名----此处要判断是不是在首页来选择设置不同的背景颜色属性
		m_vecRankText[n*4+2]->SetVisible(true);
		int iRank = m_tKeyWordRank.keyList[m].iRankings;

		if (iRank < 1 || iRank == 100)
		{	
			m_vecRankText[n*4+2]->SetAttribute(_T("text"),_T("--"));
			m_vecRankText[n*4+2]->SetAttribute(_T("bkcolor"),_T(""));
			m_vecRankText[n*4+2]->SetAttribute(_T("textcolor"),_T("#00000000"));
			m_vecRankButton[n]->SetVisible(true);
			m_vecRankButton[n]->SetEnabled(false);
		}
		else
		{
			strFormat.Format(_T("第%d页"),iRank);
			m_vecRankText[n*4+2]->SetAttribute(_T("bkcolor"),_T("#ff057b05"));
			m_vecRankText[n*4+2]->SetAttribute(_T("textcolor"),_T("#ffffffff"));
			m_vecRankText[n*4+2]->SetText(strFormat);
			m_vecRankButton[n]->SetVisible(true);
			m_vecRankButton[n]->SetEnabled(true);
		}
		//日期
		m_vecRankText[n*4+3]->SetVisible(true);
		m_vecRankText[n*4+3]->SetText(CString(m_tKeyWordRank.keyList[m].szDate));
	}

	//设置combox为当前选择页
	m_pPageCombox->SelectItem(dwPage,true);
	m_icurPage = dwPage;
}

//当登录消息返回之后，封装一个发送给每个取数据模块一个消息
void  CMainWnd::PostGetDatasMsg()
{
/*
	PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_KEYWORD_PRODUCT_DATA,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取上线关键词变化、曝光、详细信息、图表数据消息已经由界面发出！"));*/

	PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_WEIXIN_DATA,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取微信通数据消息已经由界面发出！"));

	//add by zhumingxing 20141220
	PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_GET_JZ_INFORMATION,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取建站系统数据消息已经由界面发出！"));

	//登录成功或者连接服务器失败之后进行关键词排名获取
/*
	if (!m_bIsRankRunning)
	{
		/ *ShowRankUiByState(RANKUI_RUNNING);* /
		PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_KEYWORD_RESULT_DATA,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取最新关键词排名消息已经由界面发出！"));
		m_bIsRankRunning = true;
	}*/

	m_bIsUserLoginOut = false;
}

//当有登录成功时就必须要做的操作
void CMainWnd::LoginSuccessOpt(BOOL bIsUserManualLogin)
{	
	m_bIsUseLogin = true;
	m_bIsRetryAutoLogin = FALSE;

	DecodeString(m_tUserInfo.strParam,m_userInfodb);
	CString strInfo = _T("");
	strInfo.Format(_T("<u>%s</u>"),m_userInfodb.strUserName);
	m_pBtnLogin->SetText(strInfo);
	//设置定时发送甄词消息
	::SetTimer(m_hWnd,ID_ZHENCI_MSESSAGE,2*60*1000,NULL);
	//add by zhumingxing 20141104 增加登录成功的账号密码打印
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("登录成功账号:%s,密码:%s"),m_userInfodb.strUserName,m_userInfodb.strPassWord);

	//发送相关获取用户数据消息
	PostGetDatasMsg();


	if (m_pActiveXFlash != NULL)
	{
		CString strUrl = _T("");
		CString strCfgFile = _T("");
		CString strLoadUrl = _T("");
		strCfgFile.Format(_T("%s\\data2\\mc.dat"), GetInstallPath());

		IXMLRW iniMcFile;
		iniMcFile.init(strCfgFile);

		iniMcFile.ReadString(_T("MC"),_T("RequestData"), _T("url"), strUrl);
		
		if (strUrl.GetLength() <= 0)
		{
			strUrl = _T("http://198.18.0.254:8009");
		}

		strLoadUrl.Format(_T("%s/ClientPage/ChartStatisticData?userName=%s"),strUrl,m_userInfodb.strUserName);

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("开始载入网址:%s"),strLoadUrl.GetBuffer());

		m_pActiveXFlash->Navigate2(strLoadUrl);


	}
/*
	if (m_bIsNewestVersion && bIsUserManualLogin)
	{*/
		//发送云任务消息
// 		PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_YUN_TASK, 0, 0);
// 		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("登录成功后客户端最新，发送云任务消息!"));
// 		if (m_iYunMsgType != 0)
// 		{
// 			/*ShowRankUiByState(RANKUI_RUNNING);*/
// 
// 			DWORD dwRefreshKeyWord = 0;
// 			memcpy(&dwRefreshKeyWord, &m_wVersionId, 2);
// 
// 			PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_AUTO_REFRESH_KEYWORD, dwRefreshKeyWord, (LPARAM)&m_tUserInfo);
// 			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("登录成功后客户端最新，刷新关键词排名消息已经由界面发出!"));
// 		}
/*	}*/
	//不是最新版本则登录时自动检测升级，确保用最新的用户名最新刷新排名的操作
	if (!m_bIsNewestVersion && !m_bIsUpdateChking && bIsUserManualLogin)
	{	
		GetEncodeUpdateData(m_strUpdateEncodeData,m_dwClientType);
		PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_UIHELPER,MSG_PRODUCT_UPDATE,WPARAM_AUTOCHECK_UPDATE,(LPARAM)&m_strUpdateEncodeData);
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("登录成功之后,自动检测升级消息已经由界面发出!"));
	}

	//杀掉重试登录定时器
	::KillTimer(m_hWnd,ID_RETRY_LOGIN);

	//登录成功,删除首次安装标志文件
	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\bin\\install.flag"), GetInstallPath());

	if (_taccess(strCfgFile,0) == 0)
	{
		::DeleteFile(strCfgFile);
	}

}


/*******************************用户消息处理************************************/
afx_msg void CMainWnd::OnTimerMsg(WPARAM wParam, LPARAM lParam)
{	
	if (wParam == ID_AUTO_SEND_MESSAGE)
	{
		CString strNewTimeDate = GetLocalTimeFormat();

		//同一天忽略
		if (strNewTimeDate.CompareNoCase(m_strClientStartTime) == 0)
		{	
			return;
		}
		else
		{	
			g_log.Trace(LOGL_TOP,LOGT_WARNING, __TFILE__,__LINE__, _T("客户端上次发送关键词排名消息:%s一直运行到:%s,未见退出，需要重新发送关键词排名等消息！"),m_strClientStartTime,strNewTimeDate);

			//重置最新版本标记
			iRequestCount = 0;
			m_bIsNewestVersion = FALSE;

			//检查是否需要重启或者安装客户端
			CheckReBootClient();
			
			//隔天结束云任务
		//	PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_SALF_EXIT, 0, 0, true);

			//隔天请求一次升级
			AutoMsgSend();

			//如果用户处于登录状态则获取相关图表显示数据
			if (m_bIsUseLogin)
			{
				/*PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_KEYWORD_PRODUCT_DATA,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("隔天获取相关图表数据请求已经由界面发出！"));*/

				PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_WEIXIN_DATA,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("隔天获取微信通数据消息已经由界面发出！"));

				//add by zhumingxing 20150104,新增隔天发送获取建站系统数据消息
				PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_GET_JZ_INFORMATION,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取建站系统数据消息已经由界面发出！"));
			}
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("隔天自动发送消息已经发送完毕！"));

			//将当前时间复制给启动时间，也就是最近一次发送相关消息的时间
			m_strClientStartTime = strNewTimeDate;
		}
	}
	if (wParam == ID_START_CTRLUSERTASK_MSG)
	{
		KillTimer(m_hWnd, ID_START_CTRLUSERTASK_MSG);

		CDirectory sInstallDirectory;
		CString strCmd = _T("");
		CString pCmdParam = _T(" CtrlUserTask Started by MasterZ");
		strCmd.Format(_T("%s\\bin\\CtrlUserTask.exe"), sInstallDirectory.GetInstallDir());

		if (StartProcess(strCmd.GetBuffer(), pCmdParam.GetBuffer(), &m_hProcess))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("CtrlUserTask进程启动成功"));
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("CtrlUserTask进程启动失败,path=%s"), strCmd);

			HANDLE hToken;
			if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
			{
				TOKEN_PRIVILEGES tp; //新特权结构体
				LUID Luid;
				if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid))
				{
					//给TP和TP里的LUID结构体赋值
					tp.PrivilegeCount = 1;
					tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
					tp.Privileges[0].Luid = Luid;

					AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
					if (GetLastError() != ERROR_SUCCESS)
					{
						g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("修改特权不完全或失败,error=%d"), GetLastError());
					}
					else
					{
						g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("修改特权成功"));
					}

					if (StartProcess(strCmd.GetBuffer(), pCmdParam.GetBuffer(), &m_hProcess))
					{
						g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("CtrlUserTask进程启动成功"));
					}
					else
					{
						g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("CtrlUserTask进程启动失败,path=%s"), strCmd);
						::SetTimer(m_hWnd, ID_START_CTRLUSERTASK_MSG, 20 * 60 * 1000, NULL);
					}
				}
				else
				{
					g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("获取Luid失败,error=%d"), GetLastError());
					::SetTimer(m_hWnd, ID_START_CTRLUSERTASK_MSG, 20 * 60 * 1000, NULL);
				}

				CloseHandle(hToken);
			}
			else
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("获取令牌句柄失败,error=%d"), GetLastError());
				::SetTimer(m_hWnd, ID_START_CTRLUSERTASK_MSG, 20 * 60 * 1000, NULL);
			}
		}
	}

	if (wParam == ID_WINDOW_SHOWMAX)
	{	
		if (::IsWindowVisible(m_hWnd) && !::IsIconic(m_hWnd))
		{	
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_REBOOTCLIENT_WND,0);
			KillTimer(m_hWnd,ID_WINDOW_SHOWMAX);
		}
	}
	if (wParam == ID_WINDOW_SHOWMIN)
	{	
		//当最小化窗口到托盘，就进行升级操作,此处还需要判断当前没有主控内核进程运行
		if (((::IsWindowVisible(m_hWnd)&& ::IsIconic(m_hWnd)) || ! ::IsWindowVisible(m_hWnd) )&& !CheckProcessRun(_T("MC.exe")) && !CheckProcessRun(_T("Engine.exe")))
		{	
			if (m_wCurrentUpdateFlag == WPARAM_REPAIR_UPDATE)
			{
				return;
			}
			::KillTimer(m_hWnd,ID_WINDOW_SHOWMIN);
			setTrayTips(_T("正在更新最新版本..."),m_strWindowsName,m_strWindowsName,5000);
			Sleep(2000);
			OperateTray(NIM_DELETE);

			//重启客户端消息
			PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_MGR,MSG_PRODUCT_UPDATE,WPARAM_REBOOT_CLIENT,LPARAM_REBOOT_AUTO);	

			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("客户端后台自动重启消息已经由界面发出！"));
		}
	}
	if (wParam == ID_KILLTIPS)
	{	
		//去掉气泡显示
		m_NotifyIcon.uFlags=NIF_INFO;
		_tcscpy((LPTSTR)m_NotifyIcon.szTip, m_strWindowsName);
		_tcscpy((LPTSTR)m_NotifyIcon.szInfo,_T(""));
		_tcscpy((LPTSTR)m_NotifyIcon.szInfoTitle,_T(""));
		Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIcon);
		::KillTimer(m_hWnd,ID_KILLTIPS);
	}
	//重试自动自动登录
	if (wParam == ID_RETRY_LOGIN)
	{
		//避免用户手动改掉m_tUserInfo，需要重新从数据库中加载这个信息
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("开始进行自动登录重试！"));
		CStdString strUrl = _T("");
		CString strUserName, strPassWord= _T("");
		int iAutoFlag,iSavePassWord = -1;

		if (!ReadUserInfo(strUserName,strPassWord,iAutoFlag,iSavePassWord))
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("从数据库中读取用户账户信息失败!杀掉ID为300的重试定时器！"));
			::KillTimer(m_hWnd,ID_RETRY_LOGIN);
			m_bIsRetryAutoLogin = false;
			return;
		}	
		//得到加密字符串
		//GetEncodeDate(strUserName,strPassWord,m_strEncodeString);

		if (iAutoFlag == 1 && !strUserName.IsEmpty() && !strPassWord.IsEmpty())
		{	
			//自动登录，设置标志为true
			//取出注册表中的用户名
			TCHAR* pUserName = GetRegCurrentUserName();
			if (pUserName == NULL)
			{	
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("注册表中用户名为空!"));
				::KillTimer(m_hWnd,ID_RETRY_LOGIN);
				m_bIsRetryAutoLogin = false;
				return ;
			}

			ChangeLoginfo(m_tUserInfo, _T(""),EncodeString(CString(pUserName)).GetBuffer());
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_LOGIN_CLIENT,WPARAM_AUTO_LOGIN, (LPARAM)(&m_tUserInfo));

			m_bIsRetryAutoLogin = true;			//设置重试判断标志为真
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("重试自动登录消息已经由界面发出！"));
		}
		else
		{	
			::KillTimer(m_hWnd,ID_RETRY_LOGIN);			
			m_bIsRetryAutoLogin = false;
			return;
		}
	}
	//add by zhumingxing 20141117 定时发送获取推送消息定时器
	if (wParam == ID_GET_POST_MESSAGE)
	{	
		SendPostMsg();
	}
	if (wParam == ID_CONNECT_SERVER)
	{
		SendConnectSerMsg();
	}
	//add by zhumingxing 20141211----首次取推送消息定时器处理
	if (wParam == ID_TIMING_GET_MESSAGE)
	{	
		SendPostMsg();
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("开机启动30s钟请求推送信息定时消息!"));

		SendConnectSerMsg();
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("开机启动30s钟与服务器交互消息发出!"));

		::KillTimer(m_hWnd,ID_TIMING_GET_MESSAGE);
		//此处设置定时器每隔30-60分钟定时获取此消息
		::SetTimer(m_hWnd,ID_GET_POST_MESSAGE,(GetRand(30,30)*60*1000),NULL);

		DWORD dwTime = GetConnectServer();

		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("每隔%d分钟将自动请求服务器!"),dwTime);

		::SetTimer(m_hWnd,ID_CONNECT_SERVER,(dwTime*60*1000),NULL);
	}
	if (wParam == ID_RETRY_AUTO_UPDATE)
	{
		if (m_bIsNewestVersion)
		{
			KillTimer(m_hWnd,ID_RETRY_AUTO_UPDATE);
		}
		else
		{	
			GetEncodeUpdateData(m_strUpdateEncodeData, m_dwClientType);
			PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_UIHELPER,MSG_PRODUCT_UPDATE,WPARAM_AUTOCHECK_UPDATE,(LPARAM)&m_strUpdateEncodeData);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("上次自动升级失败后30分钟客户端依然不是最新，重新发送自动升级消息!"));
		}
	}
	//新增定时器消息来检测客户端是否最小化且没有推广，此时满足静默安装条件进行静默安装
	if (wParam == ID_CHECK_SILENT_INTSTALL)
	{
		//当最小化窗口到托盘，就进行升级操作,此处还需要判断当前没有主控内核进程运行
		if (((::IsWindowVisible(m_hWnd) && ::IsIconic(m_hWnd)) || !::IsWindowVisible(m_hWnd)) && !CheckProcessRun(_T("MC.exe")) && !CheckProcessRun(_T("Engine.exe")))
		{
			::KillTimer(m_hWnd, ID_CHECK_SILENT_INTSTALL);
			//对下载完成的安装包进行静默安装
			CString strInstallExePath = GetDirectory(_T("%temp%"));
			strInstallExePath += _T("\\MasterZ_Slient_setup.exe");

			if (PathFileExists(strInstallExePath))
			{
				setTrayTips(_T("正在更新最新版本..."), m_strWindowsName, m_strWindowsName);
				//Sleep(2000);

				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("正在执行静默安装客户端程序!"));
				ShellExecute(NULL, _T("Open"), strInstallExePath, _T("/S"), NULL, SW_HIDE);
			}
			else
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("temp目录下的安装包已经不存在,等待下一次服务器请求推送!"));
			}
		}
	}
	//新增默认提示甄词消息在10分钟后托盘提示
	if (wParam == ID_ZHENCI_TIPS)
	{
		::KillTimer(m_hWnd, ID_ZHENCI_TIPS);
		setTrayTips(_T("“大师甄词”正在为您优化短词中，请保持\r\n客户端开启状态。"), m_strWindowsName, m_strWindowsName,15*1000);

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("大师甄词提示已限时完成！"));
	}
	//新增延时2分钟之后发送甄词消息
	if (wParam == ID_ZHENCI_MSESSAGE)
	{
		::KillTimer(m_hWnd,ID_ZHENCI_MSESSAGE);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("甄词消息定时任务已停止！"));
		//发送甄词消息
		if (CheckPostZhenci(m_userInfodb.strUserName))
		{
			m_tZcInfo.strUserName = m_userInfodb.strUserName;
			m_tZcInfo.strParam = m_tUserInfo.strParam;
			m_tZcInfo.iType = WARAM_ENTER_PRODUCT_ZHENCI;
			m_tZcInfo.strPostUrl = m_tUserInfo.strUrl;
			//实时获取时间戳参数
			tuserInfotodb temptb;
			DecodeString(m_tZcInfo.strParam, temptb);
			GetEncodeDate(temptb.strUserName, temptb.strPassWord, m_tZcInfo.strParam);
			PackagMessage(E_THREAD_DATASTATISTICS, E_THREAD_TYPE_UIHELPER, MSG_ZHENCI_INFO, 0, (LPARAM)&m_tZcInfo);
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("获取甑词消息已经发出!"));
		}		
	}
	//1个小时之后开始刷排名
	if (wParam == ID_PAIMING_MSG)
	{	
		if (m_dwClientType != MAIN_LINE_VERSION)
		{
			::KillTimer(m_hWnd, ID_PAIMING_MSG);
			return;
		}
					
		m_iTimeCnt ++;
		if (m_iTimeCnt >= 6)
		{
			//确保当前客户已登录
			if (m_userInfodb.strUserName.GetLength() > 0)
			{
				//开始发送排名数据
				PackagMessage(E_THREAD_DATASTATISTICS, E_THREAD_TYPE_UIHELPER, MSG_START_PAIMING, 0, (LPARAM)&m_userInfodb.strUserName);
			}			
		}
	}

}

//托盘响应
afx_msg void CMainWnd::OnOpearateTray(WPARAM wParam, LPARAM lParam)			
{	
	//此处修改为WM_RBUTTONUP以解决win8系统上弹出任务栏菜单的问题,原因可能是消息冲突
	if (lParam == WM_RBUTTONUP)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到用户右键点击托盘消息!"));

		if (m_TrayWnd.GetHWND() != NULL)
		{	
			//add by zhumingxing 20140919 for win8 OS
			m_TrayWnd.Close();		
			return;
		}

		POINT point ;
		GetCursorPos(&point);

		m_TrayWnd.SetPoint(point);
		m_TrayWnd.SetParentWnd(m_hWnd);

		/*if (m_bIsUseLogin)
		{
			m_TrayWnd.SetLoginText(_T("注销登录"));
		}
		else
		{
			m_TrayWnd.SetLoginText(_T("登录"));
		}*/
		m_TrayWnd.Create(NULL, _T("MasterZ_Traywnd"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("托盘菜单弹出完成！"));
	}	
	//双击托盘
	if (lParam == WM_LBUTTONDBLCLK)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到用户双击点击托盘消息!"));
		//显示对话框，居中,前端显示		
		ShowWindow();
		CenterWindow();
		::SetForegroundWindow(m_hWnd);

		//如果用户未登陆需要显示登录框
		if (!m_bIsUseLogin && !m_bIsAutoLogin && !m_bIsRetryAutoLogin)
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户还没有登录，需要显示登录对话框!"));

			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		}
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("主对话框显示完成!"));
	}
}

//托盘菜单响应
afx_msg void CMainWnd::OnTaryMenue(WPARAM wParam, LPARAM lParam)
{	
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到用户点击相应菜单消息:wparm=%d!"),wParam);

	//显示客户端，居中
	if (wParam == WPARAM_SHOW_CLIENT)
	{	
		if (!::IsWindowVisible(m_hWnd) || ::IsIconic(m_hWnd))

		{
			ShowWindow(); 
			CenterWindow();
		}
		::SetForegroundWindow(m_hWnd);

		//如果用户未登陆需要显示登录框
		if (!m_bIsUseLogin && !m_bIsAutoLogin && !m_bIsRetryAutoLogin)
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户还没有登录，需要显示登录对话框!"));
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		}
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("主对话框显示完成!"));
	}
	//登录商舟网
	else if(wParam == WPARAM_LOGIN_SHANGZHOU)
	{		
		int iAutoSavePassWordFlag,iAutoLoginFlag = -1;
		CString strUserName,strPassword = _T("");
		static CStdString strUrl;
		ReadUserInfo(strUserName,strPassword,iAutoLoginFlag,iAutoSavePassWordFlag);

		if (iAutoSavePassWordFlag == 1)
		{	
			GetEncodeDate(strUserName,strPassword,m_strEncodeString);

			ChangeLoginfo(m_tUserInfoWeb,m_strEncodeString,(CString)EncodeString(strUserName));
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_LOGIN_SHANGZHOU,0,(LPARAM)(&m_tUserInfoWeb));
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户点击了登录商周网，消息已经由界面发出加密字符串为:%s"),m_strEncodeString);
		}
		else
		{	
			GetEncodeDate(CString(_T("")),CString(_T("")),m_strEncodeString);

			ChangeLoginfo(m_tUserInfoWeb,m_strEncodeString,(CString)EncodeString(CString(_T(""))));
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_LOGIN_SHANGZHOU,0,(LPARAM)(&m_tUserInfoWeb));
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户点击了登录商周网，消息已经由界面发出加密字符串为:%s"),m_strEncodeString);
		}
	}
	//通知界面显示
	else if (wParam == WPARAM_SHOW_SZTONGZHI)
	{	
		//显示对话框，居中
		if (!::IsWindowVisible(m_hWnd) || ::IsIconic(m_hWnd))

		{
			ShowWindow(); 
			CenterWindow();
		}

		COptionUI* pControl = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("optionWebsite")));
		pControl->Selected(true);

	}
	//清理快照
	else if (wParam == WPARAM_CLEAR_PHOTO)
	{		
		//add by zhumingxing 20140904
		//如果当前正在进行清理操作给出提示稍后再试
		if (m_dwClearStateFlag ==  CLEAR_STATE_RUNNING)
		{
			setTrayTips(_T("舟大师正在进行清理操作，请稍后再试!"),m_strWindowsName,m_strWindowsName);
			return;
		}
		if (!::IsWindowVisible(m_hWnd) || ::IsIconic(m_hWnd))
		{
			ShowWindow(); 
			CenterWindow();
		}

		COptionUI* pControl = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("optionRubbish")));
		pControl->Selected(true);

		m_pControl->SelectItem(PAGE_CLEAR_INDEX);

		CButtonUI*pControl1 = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnPhoto")));
		pControl1->Activate();

	}
	//检测更新
	else if (wParam == WPARAM_START_UPDATE)
	{			
		if (!::IsWindowVisible(m_hWnd) || ::IsIconic(m_hWnd))
		{
			ShowWindow(); 
			CenterWindow();
		}

		/*if (m_UpdateWnd.GetHWND() == NULL)
		{*/
			m_pBtnChkUpdate->Activate();
		/*}
		else
		{
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_UPDATE_WND,0);
		}*/
	}
	//注销登录
	else if (wParam == WPARAM_LOGIN_OUT)
	{	
		if (m_bIsUseLogin)
		{		
			m_bIsUserLoginOut = true;
			m_bIsRankRunning = false;

			//清空图表数据
			if (m_pActiveXFlash != NULL)
			{
				m_pActiveXFlash->Navigate2(m_strEmptyKeywordPath);
			}
/*
			KEYWORD_PRODUCT_LIST tempdata;
			m_tDataSets = tempdata;
			m_showchart.LoadKeyWordCountChart(m_tDataSets.keyWordList);
			m_showchart.LoadExposureCountChart(m_tDataSets.productList);
			HanlePublicDetail(m_tDataSets.pro_key);*/

			//清空微信数据
			WEIXINTONG_DATA tempWeixin;
			m_tWeiXinData = tempWeixin;
			if (m_pControl->GetCurSel() == PAGE_WEIXIN_DATA || m_pControl->GetCurSel() == PAGE_NO_WEIXIN_USER)
			{
				//COptionUI*pOptioncare = (COptionUI*)m_PaintManager.FindControl(_T("option_care"));
				//pOptioncare->Selected(true);

				m_pControl->SelectItem(PAGE_NO_WEIXIN_USER);
				//m_showchart.LoadUserCareChart(m_tWeiXinData);
			}

/*
			//清空关键词排名数据
			ShowRankUiByState(RANKUI_LOGINOUT);*/

			//清除弹出甄词消息
			HWND hwnd = ::FindWindow(NULL, _T("MasterZ_zhenciwnd"));
			if (hwnd != NULL)
			{
				::SendMessage(hwnd, WM_CLOSE, 0, 0);
			}
			::KillTimer(m_hWnd,ID_ZHENCI_MSESSAGE);
			::KillTimer(m_hWnd,ID_ZHENCI_TIPS);

			//清空建站用户信息直接
			JZ_MESSAGE tmpjzdata;
			m_TjzData = tmpjzdata;
			if (m_pControl->GetCurSel() == PAGE_SZ_WEBSITE || m_pControl->GetCurSel() == PAGE_SZ_NOWEBSITE)
			{
					m_pControl->SelectItem(PAGE_SZ_NOWEBSITE);
			}
		}

		//弹出登录框
		m_bIsUseLogin = false;
		m_bIsAutoLogin = false;
		m_pBtnLogin->SetText(_T("<u>登 录</u>"));
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,LPARAM_SHOW_ANYWAY);
	}
	//弹出设置对话框
	else if (wParam == WPARAM_SHOW_SETTINGWND)
	{
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_SETTING_WND,0);
	}
	//安全退出【需要进行相关进行的kill处理】
	else if (wParam == WPARAM_SAFE_EXIT)
	{	
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_SAFEEXIT_WMD,0);
	}
	//新增诊断工具界面-----add by zhumingxing 20150420
	else if (wParam == WPARAM_CHECK_PROBLEM)
	{	
		if (!::IsWindowVisible(m_hWnd) || ::IsIconic(m_hWnd))
		{
			ShowWindow(); 
			CenterWindow();
			SetForegroundWindow(m_hWnd);
		}
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_CHECKPROBLEM_WMD,lParam);
	}
}

//升级消息
afx_msg void CMainWnd::OnUpdateMessage(WPARAM wParam, LPARAM lParam)
{	
	if (lParam == LPARAM_UPDATE_SUCCESS)
	{	
		m_bUpdateReboot = TRUE;
		m_wCurrentUpdateFlag = wParam;							//此处定义升级类型去判断是诊断框诊断时弹出的重启框
		::SetTimer(m_hWnd,ID_WINDOW_SHOWMAX,20,NULL);			//定时器检测用户是否界面正常显示
		::SetTimer(m_hWnd,ID_WINDOW_SHOWMIN,50,NULL);			//定时器检测用户是否最小化或者是到托盘
		return;
	}
	//用户升级时点击了重启客户端按钮
	if (wParam == WPARAM_REBOOT_CLIENT)
	{	
		//删除托盘
		OperateTray(NIM_DELETE);
		PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_MGR,MSG_PRODUCT_UPDATE,WPARAM_REBOOT_CLIENT,LPARAM_REBOOT_MANUAL);

		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户点升级时击了重启客户端，消息已经由界面发出MSG_PRODUCT_UPDATE wParam=101,lParam=1"));
		return;
	}
	//当手动升级进度达到100%时弹出重启提示框
	if (lParam == LPARAM_DOWNLOAD_FINISH && wParam == WPARAM_MANUALCHECK_UPDATE)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("文件下载进度已经达到100%"));
		m_dwUpdateProgress = lParam;

		if (m_UpdateWnd.GetHWND() != NULL)
		{
			//m_UpdateWnd.SendMessage(MSG_PRODUCT_UPDATE, wParam, lParam);
			m_UpdateWnd.Close();
		}
		//begin add by zhumingxing 20150115
		if (m_UpdateErrorWnd.GetHWND() != NULL)
		{
			m_UpdateErrorWnd.Close();
		}
		//end by zhumingxing 20150115

		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_REBOOTCLIENT_WND,0);
		return;
	}
	//升级进度没有达到100%，在升级对话框中进行显示
	if (m_UpdateWnd.GetHWND() != NULL)
	{	
		m_UpdateWnd.SendMessage(MSG_PRODUCT_UPDATE, wParam, lParam);
	}

	m_dwUpdateProgress = lParam;

}

//最新版本
afx_msg void CMainWnd::OnNewVerson(WPARAM wParam, LPARAM lParam)
{	
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到升级返回最新版本消息！"));

	//最新版本结束掉计时器
	::KillTimer(m_hWnd,ID_RETRY_AUTO_UPDATE);

	//自动升级无需弹出最新版本对话框，手动升级需弹框
	if (lParam == WPARAM_MANUALCHECK_UPDATE)
	{
		m_UpdateWnd.Close();
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LATESTVERSION_WND,0);
	}
	//最新版本界面显示最新版本号
	setVersion();	

	//避免用户手动点击，多次刷新关键词排名
	if (m_bIsNewestVersion)
	{
		return;				
	}

	m_bIsNewestVersion = TRUE;		

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("客户端版本最新,且配置文件最新,进行提交MAC及刷新关键词排名操作!"));

	PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_SUBMIT_USERINFO,0,0);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("统计用户装机数量消息已经发出！"));

	int iYunType = 0;
	CReg reg;

	TCHAR* pYunFlag = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("YunTask"));
	if (pYunFlag == NULL)
	{
		iYunType = 0;
	}
	else
	{
		iYunType = _ttoi(pYunFlag);
	}	
// 
// 	if (m_iYunMsgType == 0)
// 	{
	//返回最新版本之后开始执行云任务
	//如果配置为内网测试或者检测到地址为198.18开头则不发送云任务
	if (iYunType != 0 || !CheckIP())
	{
		SendYunMsg();
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("不发送云任务消息！"));
	}
//	}
// 	else
// 	{
// 		//返回最新版本之后开始刷新关键词排名
// 		StartReFreshKeyWordRank();
// 	}

	//如果用户为手动诊断升级，那么需要做特殊处理
	if (lParam == WPARAM_REPAIR_UPDATE)
	{
		OnHandleProRepairMsg(RET_SUCCESS,WPARAM_REPAIR_UPDATE);
		return;
	}
}

//升级模块返回升级错误
void CMainWnd::OnUpdateError(WPARAM wParam, LPARAM lParam)
{	
	g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("客户端检测升级失败!"));
	
	//诊断修复升级
	if (lParam == WPARAM_REPAIR_UPDATE)
	{
		OnHandleProRepairMsg(RET_ERROR,WPARAM_REPAIR_UPDATE);
		return;
	}
	if (wParam == WPARAM_POWER_ERROR && m_dwClientType == MAIN_LINE_VERSION)
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到用户三次校验升级失败返回消息!"));
		m_UpdateWnd.Close();

		if (m_UpdateErrorWnd.GetHWND() == NULL)
		{
			m_UpdateErrorWnd.Create(m_hWnd, _T("MasterZ_messageErrorwnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
			m_UpdateErrorWnd.SetPowerErrorText();
		}	

		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_UPDATEERROR_WND,0);

		return;
	}

	//升级失败，30分钟重试
	if (wParam == WPARAM_WINNET_ERROR || wParam == WPARAM_UPDATE_CONFIG_ERROR || wParam == WPARAM_SERVER_BUSY_UPDATE)
	{	
		::KillTimer(m_hWnd,ID_RETRY_AUTO_UPDATE);
		::SetTimer(m_hWnd,ID_RETRY_AUTO_UPDATE,30*60*1000,NULL);

		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到升级失败消息，启动定时器，每30分钟自动请求升级!"));
	}
	if (lParam == WPARAM_MANUALCHECK_UPDATE && (wParam == WPARAM_WINNET_ERROR || wParam == WPARAM_UPDATE_CONFIG_ERROR))
	{
		m_UpdateWnd.Close();
		if (m_UpdateErrorWnd.GetHWND() == NULL)
		{
			m_UpdateErrorWnd.Create(m_hWnd, _T("MasterZ_messageErrorwnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
		}	
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_UPDATEERROR_WND,0);
	}
	//服务器正忙请稍后再试
	if (lParam == WPARAM_MANUALCHECK_UPDATE && wParam == WPARAM_SERVER_BUSY_UPDATE)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到升级模块返回的服务器正忙消息!"));
		m_UpdateWnd.Close();
		if (m_UpdateErrorWnd.GetHWND() == NULL)
		{
			m_UpdateErrorWnd.Create(m_hWnd, _T("MasterZ_messageErrorwnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
			m_UpdateErrorWnd.setErrorText();
		}	

		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_UPDATEERROR_WND,0);
	}
	//需要先退出商务快车登录界面,升级重启时判断IE控件是需要用到
	if (lParam == WPARAM_MANUALCHECK_UPDATE && wParam == WPARAM_IECTRL_COPYERROR)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到升级模块返回的IE控件被占用无法复制消息！"));
		if (m_UpdateTipsWnd.GetHWND() != NULL)
		{	
			//重新显示托盘
			OperateTray(NIM_ADD);
			//显示提示错误信息
			m_UpdateTipsWnd.setTipsVisble(WPARAM_IECTRL_COPYERROR);
			//使关闭按钮可用
			m_UpdateTipsWnd.setCloseEnable();
		}
	}
	if (wParam == WPARAM_NPCTRL_COPYERROR)
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到升级模块返回的NP控件被占用无法复制消息！"));

		if (m_UpdateTipsWnd.GetHWND() != NULL)
		{	
			//重新显示托盘
			OperateTray(NIM_ADD);
			//显示提示错误信息
			m_UpdateTipsWnd.setTipsVisble(WPARAM_NPCTRL_COPYERROR);
			//使关闭按钮可用
			m_UpdateTipsWnd.setCloseEnable();
		}
	}

	//避免手动点击升级导致多次提交用户信息
	if (iRequestCount == 0 && !m_bIsNewestVersion)
	{	
		//表示当前配置已经是最新
		if (wParam != WPARAM_UPDATE_CONFIG_ERROR)
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("客户端检测升级失败,且配置文件最新,进行提交MAC操作!"));

			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_SUBMIT_USERINFO,0,0);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("统计用户装机数量消息已经发出！"));	
			++iRequestCount;
		}	
	}	
}

//响应初始化升级消息
afx_msg void CMainWnd::OnNotifyInitialUpdate(TNotifyUI& msg)
{	
	//手动升级时需要kill掉之前用户的检测界面状态timer
	ShowWindow(true);

	if (m_bIsCancelUpdate)
	{
		m_dwUpdateProgress = 0;
		m_bIsCancelUpdate = FALSE;
	}

	//表示后台下载文件已经100%
	if (m_dwUpdateProgress == 100)
	{	
		//直接弹出立即重启
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_REBOOTCLIENT_WND,0);
	}
	else
	{
		if (m_UpdateWnd.GetHWND() == NULL)
		{	
			//表示需要重新发送升级消息
			if (m_dwUpdateProgress == 0)
			{	
				GetEncodeUpdateData(m_strUpdateEncodeData, m_dwClientType);
				PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_MGR,MSG_PRODUCT_UPDATE,WPARAM_MANUALCHECK_UPDATE,(LPARAM)&m_strUpdateEncodeData);
			}
			m_UpdateWnd.SetParentWnd(m_hWnd);
			m_UpdateWnd.Create(m_hWnd, _T("MasterZ_updatewnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
			
			if (m_dwUpdateProgress != 0)
			{	
				//此处需要清0,否则后台升级失败时，进度会退回0
				m_UpdateWnd.SendMessage(MSG_PRODUCT_UPDATE, 0, 0);
				GetEncodeUpdateData(m_strUpdateEncodeData, m_dwClientType);
				PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_MGR,MSG_PRODUCT_UPDATE,WPARAM_MANUALCHECK_UPDATE,(LPARAM)&m_strUpdateEncodeData);
			}
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_UPDATE_WND,0);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户升级消息已经由界面发出!"));
		}
		else
		{	
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_UPDATE_WND,0);
		}
	}	
}

//响应垃圾清理消息【系统垃圾及快照】
afx_msg void CMainWnd::OnClearMessage(WPARAM wParam, LPARAM lParam,WORD wFlag)
{	
	//设置当前清理页面状态
	m_pProgressTest->SetValue(int(wParam));
	m_pProgressTest->SetText(_T(""));

	//清理完成了100%，需要跳转页面
	if (wParam == 100)
	{	
		CString strInfo;
		m_dwClearStateFlag = CLEAR_STATE_FINISH;			//完成状态

		//清理系统垃圾
		if (wFlag == CLEAR_SYESTEM_RUBISH)
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("本次垃圾清理垃圾为 %d M"),lParam);

			//做一个取随机值操作
			if ( 1 <= lParam && lParam < 310)
			{	
				strInfo.Format(_T("%d M"),(lParam + GetRand(100,310)));
			}
			else
			{
				strInfo.Format(_T("%d M"),lParam);
			}
		}
		//清理快照
		else
		{
			strInfo.Format(_T("%d M"),lParam);
		}

		//显示共处理多少M垃圾
		CTextUI *pRubishSize = (CTextUI *)m_PaintManager.FindControl(_T("TextUI2"));
		pRubishSize->SetText(strInfo);

		//如果当前处在清理垃圾界面，则直接跳转到清理完成界面
		COptionUI* p = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("optionRubbish")));
		if (p->IsSelected())
		{
			m_pControl->SelectItem(PAGE_CLEAR_FINNISH);
		}	
	}
}

//响应网站查询消息
afx_msg void  CMainWnd::OnHandleWebSearchData(WPARAM wParam, LPARAM lParam)
{	
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到网站综合查询返回消息,lParam=%d!"),lParam);
	//成功
	if (lParam == RET_SUCCESS)
	{
		TCHAR*pContent = (TCHAR*)wParam;
		CString strContent(pContent);

		CGetSearchData tempObj;
		for (int m = 1;  m <= 16; ++ m)
		{
			CString strTextName = _T("");
			strTextName.Format(_T("websitesearchText_%d"),m);
			CTextUI* pText = static_cast<CTextUI*>(m_PaintManager.FindControl(strTextName));
			CString str = tempObj.GetSearchWebData(strContent,m);
			pText->SetText(str);
		}
		CTextUI* pText = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("websitesearchText")));
		pText->SetText(tempObj.GetSearchWebData(strContent,0));
	}
	//失败
	else if (lParam == RET_ERROR)
	{

		CLabelUI*pError = (CLabelUI*)m_PaintManager.FindControl(_T("search_error"));
		pError->SetText(_T("查询失败!"));
	}
	else if (lParam == LPARAM_URL_ERROR)
	{
		CLabelUI*pError = (CLabelUI*)m_PaintManager.FindControl(_T("search_error"));
		pError->SetText(_T("域名错误!"));
	}

	OnCancelWebSearchData(wParam,lParam);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("网站综合查询消息处理完毕！"));
}

afx_msg void CMainWnd::OnCancelWebSearchData(WPARAM wParam, LPARAM lParam)
{
	CButtonUI* pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("btn_search_start"));
	pbutton->SetVisible(true);
	pbutton->SetFocus();
	pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("btn_search_loading"));
	pbutton->SetVisible(false);
	pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("btn_search_cancel"));
	pbutton->SetVisible(false);
}

afx_msg void CMainWnd::OnKeyWordAnysis(WPARAM wParam, LPARAM lParam)		//处理关键词分析
{	
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到关键词分析返回消息,lParam=%d!"),lParam);
	//分析相关消息接口
	if (lParam == RET_SUCCESS)
	{	
		TCHAR *p = (TCHAR*)wParam;
		CString strContent(p);
		CGetSearchData tempObj;
		Keyword obj;
		tempObj.GetSearchKeywordData(strContent, obj);

		CListUI*m_pList = (CListUI*)m_PaintManager.FindControl(_T("analysisResultView"));
		CListTextElementUI* pListElement = new CListTextElementUI; 
		pListElement->SetTag(1); 
		m_pList->Add(pListElement);
		pListElement->SetText(0, obj.strKeyword);   
		pListElement->SetText(1, obj.strBaiduSearchNums);  
		pListElement->SetText(2, obj.strHeatLevel);  
		pListElement->SetText(3, obj.strDifficultyLevel);
	}
	//出错
	else if (lParam == RET_ERROR)
	{	
		CLabelUI* pError = (CLabelUI*)m_PaintManager.FindControl(_T("anysis_error"));
		pError->SetText(_T("查询失败!"));

		OnCancelKeyWorsAnysis(wParam,lParam);
	}
	//完成分析
	else if (lParam == LPARAM_GETDATA_COMPLETE)
	{	
		OnCancelKeyWorsAnysis(wParam,lParam);
	}
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("关键词分析消息处理完毕！"));
}

//取消关键词分析
afx_msg void CMainWnd:: OnCancelKeyWorsAnysis(WPARAM wParam, LPARAM lParam)
{
	CButtonUI*pbutton =  (CButtonUI*)m_PaintManager.FindControl(_T("btn_search_anysis"));
	pbutton->SetVisible(true);
	pbutton->SetFocus();

	CLabelUI* pError =  (CLabelUI*)m_PaintManager.FindControl(_T("lable_aynsis_tips"));
	pError->SetVisible(true);

	pbutton =  (CButtonUI*)m_PaintManager.FindControl(_T("btn_aynsis_loading"));
	pbutton->SetVisible(false);

	pbutton =  (CButtonUI*)m_PaintManager.FindControl(_T("btn_aynsis_cancel"));
	pbutton->SetVisible(false);

	pError =  (CLabelUI*)m_PaintManager.FindControl(_T("lable_aynsis_running"));
	pError->SetVisible(false);
}

afx_msg void CMainWnd::OnKeyWordData(WPARAM wParam, LPARAM lParam)
{	
	if (m_bIsUserLoginOut)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("当前用户处于loginout状态，不处理相关图表和详细数据信息!"));
		return;
	}	

	KEYWORD_PRODUCT_LIST _tTempDataSets = *((KEYWORD_PRODUCT_LIST*)(lParam));
	if (_tTempDataSets.iSuccessFlag == 0)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("收到舟大师两个图表及详细信息数据，成功标志=%d！"),m_tDataSets.iSuccessFlag);
	}

	if (IsChartDataChange(m_tDataSets,_tTempDataSets,KEYWORD_CHART))
	{
		m_showchart.LoadKeyWordCountChart(_tTempDataSets.keyWordList);
	}
	if (IsChartDataChange(m_tDataSets,_tTempDataSets,PRODUCT_CHART))
	{	
		m_showchart.LoadExposureCountChart(_tTempDataSets.productList);
	}

	m_tDataSets = _tTempDataSets;
	HanlePublicDetail(m_tDataSets.pro_key);
}

//当取到关键词排名数据之后作处理
afx_msg void  CMainWnd::OnLoadKeyWordRank(WPARAM wParam, LPARAM lParam)
{	
	if (m_bIsUserLoginOut)
	{   
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("当前用户处于loginout状态，不处理关键词排名效果数据！"));
		m_bIsRankRunning = false;
		return;
	}
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到关键词排名效果数据！"));
	KEYWORDSDETAILRESPONSELIST _tTempKeyWordRank =  *((KEYWORDSDETAILRESPONSELIST*)(lParam));

	if (_tTempKeyWordRank.iSuccessFlag == 0 || _tTempKeyWordRank.keyList.size() == 0)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("关键词排名效果数据有误！iSuccessFlag=%d,列表数据大小:%d"),\
			_tTempKeyWordRank.iSuccessFlag,_tTempKeyWordRank.keyList.size());

		if (wParam == 1)
		{
			//此时发送检测排名模块状态消息
			ShowRankUiByState(RANKUI_EMPTY);
// 			if (m_iYunMsgType != 0)
// 			{
// 				PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_CHECK_REFRESH_KEYWORD_STATA, 0, 2);
// 			}
		}
		else
		{	
			//保证不刷排名的时候才出现ERROR状态，否则状态会重叠
			if (!m_bIsNewestVersion)
			{
				ShowRankUiByState(RANKUI_ERROR);
			}
			ShowRankUiByState(RANKUI_EMPTY);
		}
	}
	else
	{	
		//最新排名和上次排名之差
		m_iNewKeywordCnt = _tTempKeyWordRank.keyList.size() - m_tKeyWordRank.keyList.size();

		m_tKeyWordRank = *((KEYWORDSDETAILRESPONSELIST*)(lParam));
		
		//ShowRankUiByState(RANKUI_SUCCESS);
		
		//初始化时都处于隐藏状态只有获取到数据之后才会显示
	/*	ShowRankUiByState(RANKUI_HIDE);*/

		//add by zhumingxing 20140926---清空之前先初始化选中项
		m_pPageCombox->SelectItem(-1,false);
		m_pPageCombox->RemoveAll();

		CString strTempPage= _T("");
		//计算出返回的数据一共有多少页
		if (m_tKeyWordRank.keyList.size()%8 == 0)
		{
			m_iPage = m_tKeyWordRank.keyList.size()/8;
		}
		else
		{
			m_iPage = m_tKeyWordRank.keyList.size()/8 + 1;
		}
		//填充combox数据
		for (int i=1; i <=m_iPage; ++i)
		{	
			strTempPage.Format(_T("%d"),i);
			CListLabelElementUI* plable = new CListLabelElementUI;
			plable->SetAttribute(_T("text"),strTempPage);
			m_pPageCombox->Add(plable);
		}

		//显示出第一页数据
		ShowRankPageData(0);

		//wParm = 0 表示登录成功取数据 wParm = 1表示是刷排名模块返回Finish消息的时候取数据
		if (wParam == 1 || !m_bIsNewestVersion)
		{	
			//此时发送检测排名模块状态消息
// 			if (m_iYunMsgType != 0)
// 			{
// 				PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_CHECK_REFRESH_KEYWORD_STATA, 0, 1);
// 			}
		}
		//数据载入完后显示
		ShowRankUiByState(RANKUI_SUCCESS);

	}
	m_bIsRankRunning = false;
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("关键词排名效果数据UI显示完成！"));
}


void CMainWnd::HanlePublicDetail(PRODUCT_KEYWORDSTATISTICS& tProductDetail)
{	
	CString strContent = _T("");
	CTextUI*pText = (CTextUI*)m_PaintManager.FindControl(_T("textYq1"));
	strContent.Format(_T("%d"),tProductDetail.iKeyWordCount);
	pText->SetText(strContent);
	pText = (CTextUI*)m_PaintManager.FindControl(_T("textYq1_Detail"));
	pText->SetText(strContent);

	pText = (CTextUI*)m_PaintManager.FindControl(_T("textYq2"));
	strContent.Format(_T("%d"),tProductDetail.iKeyWordHasRankCount);
	pText->SetText(strContent);
	pText = (CTextUI*)m_PaintManager.FindControl(_T("textYq2_Detail"));
	pText->SetText(strContent);

	pText = (CTextUI*)m_PaintManager.FindControl(_T("textPro1"));
	strContent.Format(_T("%d"),tProductDetail.iProductCoverCount);
	pText->SetText(strContent);
	pText = (CTextUI*)m_PaintManager.FindControl(_T("textPro1_Detail"));
	pText->SetText(strContent);

	pText = (CTextUI*)m_PaintManager.FindControl(_T("textprocure1"));
	strContent.Format(_T("%d"),tProductDetail.iProductCount);
	pText->SetText(strContent);
	pText = (CTextUI*)m_PaintManager.FindControl(_T("textprocure1_Detail"));
	pText->SetText(strContent);

	pText = (CTextUI*)m_PaintManager.FindControl(_T("textprocure2"));
	strContent.Format(_T("%d"),tProductDetail.iProductDownCount);
	pText->SetText(strContent);
	pText = (CTextUI*)m_PaintManager.FindControl(_T("textprocure2_Detail"));
	pText->SetText(strContent);

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("舟大师产品及关键词详细信息解析完毕！"));
}

afx_msg void CMainWnd::OnInitialLogin(WPARAM wParam, LPARAM lParam)
{	
	//此处取出相关消息然后发出去
	PTUSERINTO pUserinfo = (PTUSERINTO)wParam;
	//做一些封装之后发出去
	GetEncodeDate(pUserinfo->strUserName,pUserinfo->strPassWord,m_strEncodeString);
	ChangeLoginfo(m_tUserInfo,m_strEncodeString,(CString)EncodeString(pUserinfo->strUserName));

	//add by zhumingxing to delete memory 20140527
	delete pUserinfo;
	pUserinfo = NULL;
	//end
	PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_LOGIN_CLIENT,0,(LPARAM)(&m_tUserInfo));
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户登录消息已经发出!"));
}

afx_msg void CMainWnd::OnLoginMessage(WPARAM wParam, LPARAM lParam)
{	
	WORD wLoginResult = wParam;				//返回结果
	m_tUserInfo = *((PassInfo*)lParam);		//返回结构体
	m_wVersionId = m_tUserInfo.iExressVersionId;

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到由登录模块返回的登录结果消息返回码=%d"),wLoginResult);
				
	//首次安装登录消息返回
	if (m_tUserInfo.iLoginType == WPAPAM_INSTALL_LOGIN)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到由登录模块返回的首次安装登录结果消息返回码=%d"),wLoginResult);	

		//成功
		if (wLoginResult == RET_SUCCESS)
		{	
			//需要写数据库，将账号和密码都写到数据库中，然后保存密码和自动登录标志设为成功
			DecodeString(m_tUserInfo.strParam,m_userInfodb);
			g_globalData.sqlIte.DeleteUserInfo();
			WriteUserInfo(m_userInfodb.strUserName,m_userInfodb.strPassWord,1,1,/*m_userInfodb.iProductID*/m_wVersionId);

			//更新注册表信息
			UpdateRegUserInfo(m_userInfodb.strUserName,m_wVersionId);

			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户信息写入数据库成功！处理用户登录消息完成！"));
			LoginSuccessOpt();
		}
		//失败
		else
		{	
			//弹出登录框提示用户登录
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		}
		//add by zhumingxing 20140909 安装登录返回后重设自动登录标志
		m_bIsAutoLogin = false;
		return;
	}

	//add by zhumingxing 20150312----新增注册表账号发生变化之后动态切换用户账号
	if (m_tUserInfo.iLoginType == WPARAM_CHANGE_USER)
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("切换账号登录消息已经返回,登录结果返回码为%d"),wLoginResult);
		//m_tUserInfo.strUserName = EncodeString(m_tUserInfo.strUserName);

		if (wLoginResult == RET_SUCCESS)
		{
			CString strUserName, strPassWord= _T("");
			int iAutoFlag,iSavePassWord = -1;

			if (!ReadUserInfo(strUserName,strPassWord,iAutoFlag,iSavePassWord))
			{
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("读取用户数据库信息失败!"));
				return;
			}
			else
			{
				//需要写数据库，将账号和密码都写到数据库中，然后保存密码和自动登录标志设为成功
				DecodeString(m_tUserInfo.strParam,m_userInfodb);
				g_globalData.sqlIte.DeleteUserInfo();
				WriteUserInfo(m_userInfodb.strUserName,m_userInfodb.strPassWord,iAutoFlag,iSavePassWord,m_wVersionId);
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户信息写入数据库成功！处理用户登录消息完成！"));

				//更新注册表信息
				UpdateRegUserInfo(m_userInfodb.strUserName,m_wVersionId);

				LoginSuccessOpt(TRUE);
				if (m_LoginWnd.GetHWND() != NULL)
				{
					m_LoginWnd.Close();
				}
			}
		}
		else
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("切换账号登录失败！"));
		}
		return;
	}
	//自动重试登录
	if (m_bIsRetryAutoLogin)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("重试自动登录消息已经返回，登录结果返回码为%d"),wLoginResult);
		//m_tUserInfo.strUserName = EncodeString(m_tUserInfo.strUserName);

		if (wLoginResult == RET_SUCCESS)
		{	
			//需要写数据库，将账号和密码都写到数据库中，然后保存密码和自动登录标志设为成功
			DecodeString(m_tUserInfo.strParam,m_userInfodb);
			g_globalData.sqlIte.DeleteUserInfo();
			WriteUserInfo(m_userInfodb.strUserName,m_userInfodb.strPassWord,1,1,/*m_userInfodb.iProductID*/m_wVersionId);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户信息写入数据库成功！处理用户登录消息完成！"));

			//更新注册表信息
			UpdateRegUserInfo(m_userInfodb.strUserName,m_wVersionId);

			LoginSuccessOpt(TRUE);
			if (m_LoginWnd.GetHWND() != NULL)
			{
				m_LoginWnd.Close();
			}
		}
		//重置Retry标志为false
		m_bIsRetryAutoLogin = FALSE;
		return;
	}
	//自动登录
	if (m_bIsAutoLogin)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("自动登录消息已经返回，登录结果返回码为%d"),wLoginResult);
		//m_tUserInfo.strUserName = EncodeString(m_tUserInfo.strUserName);

		if (wLoginResult == RET_SUCCESS)
		{	
			//需要写数据库，将账号和密码都写到数据库中，然后保存密码和自动登录标志设为成功
			DecodeString(m_tUserInfo.strParam,m_userInfodb);
			g_globalData.sqlIte.DeleteUserInfo();
			WriteUserInfo(m_userInfodb.strUserName,m_userInfodb.strPassWord,1,1,/*m_userInfodb.iProductID*/m_wVersionId);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户信息写入数据库成功！处理用户登录消息完成！"));

			//更新注册表信息
			UpdateRegUserInfo(m_userInfodb.strUserName,m_wVersionId);

			LoginSuccessOpt();
			if (m_LoginWnd.GetHWND() != NULL)
			{
				m_LoginWnd.Close();
			}
		}
		//失败之后要提示错误
		else
		{	
			//如果自动登录失败，为了能够及时刷新关键词排名，每隔十分钟再登录一次，这样就失败了就不需要显示出登录框
			::SetTimer(m_hWnd,ID_RETRY_LOGIN,(10*60*1000),NULL);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("自动登录失败，设置定时器10min钟后自动登录!"));

			//连接服务器失败
			if (wLoginResult == WPARAM_CONNECTSERVER_ERROR)	
			{	
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("登录连接服务器失败！"));
				PostGetDatasMsg();
			}
			if (m_LoginWnd.GetHWND() != NULL)
			{	
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("登录消息已经收到结果码为= %d"),wLoginResult);
				m_LoginWnd.SendMessage(MSG_LOGIN_CLIENT,wLoginResult,0);
			}
			else
			{
				m_LoginWnd.SetParentWnd(m_hWnd);
				m_LoginWnd.Create(m_hWnd, _T("MasterZ_loginWnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
				m_LoginWnd.SendMessage(MSG_LOGIN_CLIENT,wLoginResult,0);
			}	
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		}
		m_bIsAutoLogin = false;
	}
	//用户手动登录
	else
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到登录消息已经返回，登录结果返回码为%d"),wLoginResult);

		DecodeString(m_tUserInfo.strParam,m_userInfodb);
		m_userInfodb.iProductID = m_wVersionId;

		//连接服务器失败需要取数据库中的数据
		if (wLoginResult == WPARAM_CONNECTSERVER_ERROR)	
		{	
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("登录连接服务器失败！"));
			PostGetDatasMsg();
		}
		if (m_LoginWnd.GetHWND() != NULL)
		{		
			m_LoginWnd.SendMessage(MSG_LOGIN_CLIENT,wLoginResult,(LPARAM)&m_userInfodb);
		}
		if (wLoginResult == RET_SUCCESS)
		{	
			LoginSuccessOpt(TRUE);
		}
	}		
}

//响应定时气泡消息
afx_msg void CMainWnd::OnTimerTips(WPARAM wParam, LPARAM lParam)
{	
	CString strTipsInfo = *((CString*)lParam);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到气泡消息:%s!"),strTipsInfo);
	CString strTitle,strContent,strTime;
	AnysisTipsInfo(strTipsInfo,strTitle,strContent,strTime);
	int ioutTime = _ttoi(strTime);
	setTrayTips(strContent,strTitle,strTitle,ioutTime*1000);
}

//响应tab键操作
afx_msg void CMainWnd::OnTableKeys(WPARAM wParam, LPARAM lParam)
{	
	//当存在登录界面时，先响应登录界面tab
	if (m_LoginWnd.GetHWND() != NULL)
	{
		m_LoginWnd.PostMessage(WM_TAB_KEY_PRESS,0,0);
	}

	//当主界面当前为关键词分析页面时响应tab键
	if (m_pControl->GetCurSel() == PAGE_KEYWORD_ANALYSIS)
	{	
		for (int i = 0; i < m_EditKeyWord.size(); ++i)
		{	
			if (m_EditKeyWord[i]->IsFocused())
			{	
				if (i == m_EditKeyWord.size()-1)
				{
					m_EditKeyWord[0]->SetFocus();
					break;
				}
				m_EditKeyWord[i+1]->SetFocus();
				break;
			}
		}
	}
	return ;
}

//响应鼠标enter键
afx_msg void CMainWnd::OnEnterKeys(WPARAM wParam, LPARAM lParam)
{	
	//如果登录界面可见，则响应登录按钮
	if (::IsWindowVisible(m_LoginWnd.GetHWND()))
	{
		m_LoginWnd.PostMessage(WM_ENTER_KEY_PRESS,0,0);
		return;

	}

	//如果主界面当前显示为网站综合查询页面则响应enter键
	if (::IsWindowVisible(m_hWnd) && m_pControl->GetCurSel() == PAGE_WEBSITE_SEARCH)
	{	
		CButtonUI* pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("btn_search_start"));
		pbutton->Activate();
		return;
	}

	return;
}

afx_msg void CMainWnd::OnWeixinDate(WPARAM wParam, LPARAM lParam)
{	
	if (m_bIsUserLoginOut)
	{
		return;
	}
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到微信通数据返回消息"));
	/*m_tWeiXinData = *((WEIXINTONG_DATA*)lParam);*/
	WEIXINTONG_DATA tTempWeixinData = *((WEIXINTONG_DATA*)lParam);

	//未开通微信通,切换到微信通开通界面
	if (tTempWeixinData.iSuccessFlag == -1)
	{	
		g_log.Trace(LOGL_TOP,LOGT_WARNING, __TFILE__,__LINE__, _T("收到微信通数据返回消息，iSuccessFlag==-1用户没有开通！"));
		if (m_pControl->GetCurSel() == PAGE_WEIXIN_DATA || m_pControl->GetCurSel() == PAGE_NO_WEIXIN_USER)
		{	
			m_pControl->SelectItem(PAGE_NO_WEIXIN_USER);
		}
	}
	else
	{	
		if (m_pControl->GetCurSel() == PAGE_WEIXIN_DATA)
		{	
			COptionUI*pOptionReQuest = (COptionUI*)m_PaintManager.FindControl(_T("option_request"));
			if (pOptionReQuest->IsSelected() && pOptionReQuest->IsVisible())
			{	
				pOptionReQuest->Selected(true);
				if (IsWeixinUserCareDataChange(m_tWeiXinData,tTempWeixinData,WEIXIN_USERREQUEST_CHART))
				{
					m_showchart.LoadUserRequestChart(tTempWeixinData);
				}
			}
			else
			{
				COptionUI*pOptioncare = (COptionUI*)m_PaintManager.FindControl(_T("option_care"));
				pOptioncare->Selected(true);

				if (IsWeixinUserCareDataChange(m_tWeiXinData,tTempWeixinData,WEIXIN_USERCARE_CHART))
				{
					m_showchart.LoadUserCareChart(tTempWeixinData);
				}
				
			}
				
		}
		if (m_pControl->GetCurSel() == PAGE_NO_WEIXIN_USER)
		{
			m_pControl->SelectItem(PAGE_WEIXIN_DATA);
			COptionUI*pOptioncare = (COptionUI*)m_PaintManager.FindControl(_T("option_care"));
			pOptioncare->Selected(true);

			if (IsWeixinUserCareDataChange(m_tWeiXinData,tTempWeixinData,WEIXIN_USERCARE_CHART))
			{
				m_showchart.LoadUserCareChart(tTempWeixinData);
			}
		}
	}
	m_tWeiXinData = tTempWeixinData;
}

//建站系统数据处理
afx_msg void CMainWnd::OnHandleJZData(WPARAM wParam, LPARAM lParam)
{	
	if (m_bIsUserLoginOut)
	{
		return;
	}

	m_TjzData = *((JZ_MESSAGE*)lParam);
	
	if (m_TjzData.iSuccessFlag == -1)
	{	
		if (m_pControl->GetCurSel() == PAGE_SZ_WEBSITE || m_pControl->GetCurSel() == PAGE_SZ_NOWEBSITE)
		{	
			m_pControl->SelectItem(PAGE_SZ_NOWEBSITE);
		}
	}
	else
	{

		if (m_pControl->GetCurSel() == PAGE_SZ_WEBSITE || m_pControl->GetCurSel() == PAGE_SZ_NOWEBSITE)
		{	
			m_pControl->SelectItem(PAGE_SZ_WEBSITE);
		}
		
		//加载相关数据
		CString strInfo = _T("");
		CTextUI*pText = (CTextUI*)m_PaintManager.FindControl(_T("jzweburl"));
		strInfo.Format(_T("<u>%s</u>"),m_TjzData.strWebSiteUrl);
		pText->SetText(strInfo);
		pText->SetToolTip(m_TjzData.strWebSiteUrl);

		////表示未绑定
		//if (m_TjzData.iIsBind == 0)
		//{
		//	CButtonUI* pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("jzbindDomain"));
		//	pbutton->SetVisible(false);

		//	CHorizontalLayoutUI* pLayout = (CHorizontalLayoutUI*)m_PaintManager.FindControl(_T("unbind"));
		//	pLayout->SetVisible(true);
		//}
		//else
		//{
		//	CHorizontalLayoutUI* pLayout = (CHorizontalLayoutUI*)m_PaintManager.FindControl(_T("unbind"));
		//	pLayout->SetVisible(false);

		//	CButtonUI* pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("jzbindDomain"));
		//	pbutton->SetVisible(true);
		//	strInfo.Format(_T("<f 1><u>%s</u></f>"),m_TjzData.strBindDomain);
		//	pbutton->SetText(strInfo);
		//}

		pText = (CTextUI*)m_PaintManager.FindControl(_T("jzexpirationdate"));
		strInfo.Format(_T("%s"),m_TjzData.strExpirationDate);
		pText->SetText(strInfo);
	}
}

afx_msg void  CMainWnd::OnHandleServerMsg(WPARAM wParam, LPARAM lParam)
{
	TCHAR* pInfo = (TCHAR*)(wParam);
	CString strInfo(pInfo);

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到服务器返回的互通消息:%s"),strInfo);

	//自动升级消息
	if (!strInfo.Compare(_T("CMD_CHECK_UPDATE")))
	{	
		GetEncodeUpdateData(m_strUpdateEncodeData, m_dwClientType);
		PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_UIHELPER,MSG_PRODUCT_UPDATE,WPARAM_AUTOCHECK_UPDATE,(LPARAM)&m_strUpdateEncodeData);

		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("自动升级消息已经由界面发出!"));
	}
	else if (!strInfo.Compare(_T("CMD_SUBMIT_LOG")))
	{

		PackagMessage(E_THREAD_YUN_TASK,E_THREAD_TYPE_UIHELPER,MSG_SUBMIT_LOG,0,(LPARAM)&m_tConnectServer);

		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("提交日志消息已经由界面发出!"));
	}
	else if(!strInfo.Compare(_T("CMD_SUBMIT_LOGANDPHOTO")))
	{
		PackagMessage(E_THREAD_YUN_TASK,E_THREAD_TYPE_UIHELPER,MSG_SUBMIT_LOG,1,(LPARAM)&m_tConnectServer);

		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("提交日志AND快照消息已经由界面发出!"));
	}
	else if (!strInfo.Compare(_T("CMD_SUBMIT_LOGANDERROROCR")))
	{
		PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_SUBMIT_LOG, 2, (LPARAM)&m_tConnectServer);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交日志ANDErrorCode消息已经由界面发出!"));
	}
	else if (!strInfo.Compare(_T("CMD_SUBMIT_BACK_LOG")))
	{
		PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_SUBMIT_LOG, 3, (LPARAM)&m_tConnectServer);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交备份压缩日志消息已经由界面发出!"));
	}
	else if (!strInfo.Compare(_T("CMD_SUBMIT_DIR_LIST")))
	{
		PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_SUBMIT_LOG, 4, (LPARAM)&m_tConnectServer);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交目录列表消息已经由界面发出!"));
	}
	else if (!strInfo.Compare(_T("CMD_SUBMIT_SPECIFY_PATH")))
	{
		PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_SUBMIT_LOG, 5, (LPARAM)&m_tConnectServer);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("提交指定目录消息已经由界面发出!"));
	}
	//新增安装包推送功能
	else if (strInfo.Find(_T("CMD_INSTALL_UPDATE")) != -1)
	{
		//此处新增对安装包推送的处理
		m_strTuiSong = strInfo;

		if (m_iUnstallFlag != 1)
		{
			PackagMessage(E_THREAD_UPDATE, E_THREAD_TYPE_UIHELPER, MSG_DOWNLOAD_SILENT_EXE, (WPARAM)&m_strTuiSong, 0);
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("当前为卸载版本，不执行推送!"));
		}
		
	}
	else if (!strInfo.Compare(_T("CMD_WRITE_AUTO_REGEDIT")))
	{
		//开机启动
		if (!m_bWriteStart)
		{	
			//写开机启动；
			CReg reg;
			BOOL bBootStart = FALSE;
			CString strFlag;
			strFlag = (TCHAR*)(reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("nostart")));
			if (strFlag != _T("1"))
			{
				OperateRegistOfStart();
				bBootStart = TRUE;
			}

			//退出软件隐藏
			strFlag = (TCHAR*)(reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("hideClient")));
			if (strFlag != _T("1"))
			{
				//通过设置勾选的开机不启动，则启动程序也不用再去检测
				if(reg.WriteValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("hideClient"), _T("1"))
					&& bBootStart == TRUE)
					m_bWriteStart = TRUE;
			}

			
		}
	}

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("互通消息处理完毕!"));
}

afx_msg void CMainWnd::OnHandleRegChange(WPARAM wParam, LPARAM lParam)
{	
	//如果当前用户已经登录就切换成注册表中的用户
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到注册表中用户名改变的消息!"));

	//取出注册表中的用户名
	TCHAR* pUserName = GetRegCurrentUserName();

	if (pUserName == NULL)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("切换账号失败!"));
		return ;
	}

	//如果注册表中的账号和当前用户登录的账号一致，则无需切换
	CString strUserName, strPassWord= _T("");
	int iAutoFlag,iSavePassWord = -1;

	ReadUserInfo(strUserName,strPassWord,iAutoFlag,iSavePassWord);

	if (!strUserName.Compare(CString(pUserName)) && m_bIsUseLogin)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("注册表中用户名与当前用户登录的名字一致，无需切换账号!"));

		return;
	}

	ChangeLoginfo(m_tUserInfo, _T(""),EncodeString(CString(pUserName)).GetBuffer());
	PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_LOGIN_CLIENT,WPARAM_CHANGE_USER, (LPARAM)(&m_tUserInfo));

}

afx_msg void CMainWnd::OnPopDlg(WPARAM wParam, LPARAM lParam)
{	
	if (wParam == WPARAM_SHOW_LOGIN_WND)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("开始处理弹出登录对话框消息!"));
		if (m_LoginWnd.GetHWND() == NULL)
		{	
			m_LoginWnd.SetParentWnd(m_hWnd);
			m_LoginWnd.Create(m_hWnd, _T("MasterZ_loginWnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);	
		}
		m_LoginWnd.ShowWindow(FALSE);		//主客户端不弹出，登录对话框不单独弹出

		if (::IsWindowVisible(m_hWnd) && !::IsIconic(m_hWnd))
		{	
			m_LoginWnd.CenterWindow();
			m_LoginWnd.ShowModal();
		}
		else
		{	
			if (lParam == LPARAM_SHOW_ANYWAY)
			{
				ShowWindow(); 
				CenterWindow();
				m_LoginWnd.CenterWindow();
				m_LoginWnd.ShowModal();
			}	
		}
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("弹出登录对话框消息处理完毕！"));
	}
	else if (wParam == WPARAM_SHOW_UPDATE_WND)
	{	
		m_UpdateWnd.CenterWindow();
		m_UpdateWnd.ShowModal();
	}
	else if (wParam == WPARAM_SHOW_REBOOTCLIENT_WND)
	{	
		if (m_UpdateErrorWnd.GetHWND() != NULL)
		{
			return;			//还存在错误框时不弹出重启框，此处主要是因为舟大师登录成功之后会发送升级消息，校验失败提示权限问题时就可能会有这种问题
		}
		if (m_UpdateWnd.GetHWND() != NULL)
		{
			//m_UpdateWnd.SendMessage(MSG_PRODUCT_UPDATE, wParam, lParam);
			m_UpdateWnd.Close();
		}
		if (m_UpdateTipsWnd.GetHWND() == NULL)
		{
			m_UpdateTipsWnd.SetParentWnd(m_hWnd);
			m_UpdateTipsWnd.Create(m_hWnd, _T("MasterZ_messagetipswnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
		}
		::SetForegroundWindow(m_hWnd);
		m_UpdateTipsWnd.CenterWindow();
		m_UpdateTipsWnd.ShowModal();
	}
	else if (wParam == WPARAM_SHOW_LATESTVERSION_WND)
	{	
		if (m_MessageWnd.GetHWND() == NULL)
		{
			m_MessageWnd.Create(m_hWnd, _T("MasterZ_message"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
		}
		m_MessageWnd.CenterWindow();
		m_MessageWnd.ShowModal();
	}
	else if (wParam == WPARAM_SHOW_UPDATEERROR_WND)
	{	
		ShowWindow(); 
		m_UpdateErrorWnd.CenterWindow();
		m_UpdateErrorWnd.ShowModal();
	}
	else if (wParam == WPARAM_SHOW_NETWORKERROR_WND)
	{	
		if (m_WarningWnd.GetHWND() == NULL)
		{
			m_WarningWnd.Create(m_hWnd, _T("MasterZ_Warningwnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
		}
		m_WarningWnd.CenterWindow();
		m_WarningWnd.ShowModal();
	}
	else if (wParam == WPARAM_SHOW_SETTING_WND)
	{	
		if (m_ClientSetWnd.GetHWND() == NULL)
		{
			m_ClientSetWnd.Create(NULL, _T("Client_SetWnd"), UI_WNDSTYLE_FRAME, WS_EX_TOOLWINDOW);
		}
		::SetForegroundWindow(m_ClientSetWnd.GetHWND());
		m_ClientSetWnd.CenterWindow();
		m_ClientSetWnd.ShowModal();		
	}
	else if (wParam == WPARAM_SHOW_SAFEEXIT_WMD)
	{	
		if (m_SafeExitWnd.GetHWND() == NULL)
		{	
			m_SafeExitWnd.SetParentWnd(m_hWnd);
			m_SafeExitWnd.Create(NULL, _T("MasterZ_exitwnd"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
		}
		::SetForegroundWindow(m_SafeExitWnd.GetHWND());
		m_SafeExitWnd.CenterWindow();
		m_SafeExitWnd.ShowModal();
	}
	else if (wParam == WPARAM_SHOW_CHECKPROBLEM_WMD)
	{
		if (m_CheckProWnd.GetHWND() == NULL)
		{	
			m_CheckProWnd.SetParentWnd(m_hWnd);
			m_CheckProWnd.Create(m_hWnd, _T("Client_CheckWnd"), UI_WNDSTYLE_FRAME, WS_EX_TOOLWINDOW);
		}
		if (lParam == 1)
		{
			m_CheckProWnd.ActiveCheckButton();
		}
		m_CheckProWnd.CenterWindow();
		m_CheckProWnd.ShowModal();
		
	}
}

//鼠标右键任务栏窗口关闭时，需要将其他子窗口一起关闭然后最小化到托盘
afx_msg void CMainWnd::OnRBTaskClose(WPARAM wParam, LPARAM lParam)
{
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户点击了任务栏窗口关闭!"));
	::ShowWindow(m_hWnd,SW_HIDE);
	if (m_LoginWnd.GetHWND() != NULL)
	{
		m_LoginWnd.Close();
	}
	if (m_UpdateWnd.GetHWND() != NULL)
	{
		m_UpdateWnd.Close();
	}
	if (m_UpdateErrorWnd.GetHWND() != NULL)
	{
		m_UpdateErrorWnd.Close();
	}
	if (m_UpdateTipsWnd.GetHWND() != NULL)
	{
		m_UpdateTipsWnd.Close();
	}
	if (m_MessageWnd.GetHWND() != NULL)
	{
		m_MessageWnd.Close();
	}
	if (m_WarningWnd.GetHWND() != NULL)
	{
		m_WarningWnd.Close();
	}
	if (m_ClientSetWnd.GetHWND() != NULL)
	{
		m_ClientSetWnd.Close();
	}
	if (m_SafeExitWnd.GetHWND() != NULL)
	{
		m_SafeExitWnd.Close();
	}
	if (m_CheckProWnd.GetHWND() != NULL)
	{
		m_CheckProWnd.Close();
	}
}

afx_msg void CMainWnd::OnHanleRefreshKeyWord(WPARAM wParam, LPARAM lParam)
{	
/*
	if (!m_bIsUseLogin)
	{   
		return;
	}
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到刷新关键词排名模块消息：wParam=%d"),wParam);

	if (wParam == WARAM_AUTO_REFRESH_KEYWORD_FINISH)
	{
		if (m_bIsUseLogin)
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("刷新关键词排名返回刷新完成消息,进行获取关键词排名及用户图表详细数据操作!"));
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_KEYWORD_PRODUCT_DATA,1,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_KEYWORD_RESULT_DATA,1,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
			m_bIsRankRunning = true;
			/ *ShowRankUiByState(RANKUI_RUNNING);* /
		}
	}*/
/*
	else
	{
		ShowRankUiByState(RANKUI_RUNNING);
	}*/
}

//检测刷新关键词排名状态
afx_msg void CMainWnd::OnHandleRefreshState(WPARAM wParam, LPARAM lParam)
{	
	if (!m_bIsUseLogin)
	{   
		return;
	}

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("checkstate:收到检测关键词排名状态消息：wParam=%d"),wParam);

	if (lParam == 0)
	{
		//表示当前刷新关键词模块没有进行刷新关键词排名
		if (wParam == WARAM_AUTO_REFRESH_KEYWORD_FINISH)
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检测到用户当前没有进行刷新关键词排名和获取排名操作，发送刷新关键词排名消息！lParam == 0"));
			StartReFreshKeyWordRank();
		}
		else
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检测到用户正在进行刷新关键词排名操作...lParam == 0"));
			//add by zhumingxing 20141127----随时切换当前刷新关键词模块状态
			/*ShowRankUiByState(RANKUI_RUNNING);*/
			//end by zhumingxing 20141127
			return;
		}
	}
	else if (lParam == 1)
	{
		//表示当前刷新关键词模块没有进行刷新关键词排名
		if (wParam == WARAM_AUTO_REFRESH_KEYWORD_FINISH)
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检测到用户当前没有进行刷新关键词排名和获取排名操作显示排名数据获取完成状态！lParam == 1"));
			//StartReFreshKeyWordRank();
			ShowRankUiByState(RANKUI_SUCCESS);
		}
		else
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检测到用户正在进行刷新关键词排名操作...lParam == 1"));
			//add by zhumingxing 20141127----随时切换当前刷新关键词模块状态
			//ShowRankUiByState(RANKUI_RUNNING);
			//end by zhumingxing 20141127
			return;
		}
	}
	else
	{
		//表示当前刷新关键词模块没有进行刷新关键词排名
		if (wParam == WARAM_AUTO_REFRESH_KEYWORD_FINISH)
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到用户当前没有进行刷新关键词排名和获取排名操作显示排名数据获取完成状态！lParam == 2"));
			//StartReFreshKeyWordRank();
			ShowRankUiByState(RANKUI_ERROR);
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到用户正在进行刷新关键词排名操作...lParam == 2"));
			return;
		}
	}
	
}

//新增推送消息处理
afx_msg void CMainWnd::OnHandlePostMessage(WPARAM wParam, LPARAM lParam)
{	
	if (wParam == RET_SUCCESS)
	{	
		m_tPostMessage = *((DELAY_MESSAGE*)(lParam));
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到服务器推送消息，Type=%d"),m_tPostMessage.iType);

		//强制升级消息
		if (m_tPostMessage.iType == NOTIFY_TYPE_UPDATE)
		{
			//发送给升级模块进行安装包下载
			PackagMessage(E_THREAD_UPDATE, E_THREAD_TYPE_UIHELPER, MSG_DOWNLOAD_INSTALLEXE, WPARAM(&m_tPostMessage), 0);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("下载安装包消息已经由界面发出！"));
		}
		else
		{
			HWND hwnd = ::FindWindow(NULL,_T("MasterZ_postmsgwnd"));

			if (hwnd == NULL)
			{
				CPostMsgWnd* pClientTips = new CPostMsgWnd();
				pClientTips->SetParentWnd(m_hWnd);
				pClientTips->SetPostInfo(m_tPostMessage);

				pClientTips->Create(NULL, _T("MasterZ_postmsgwnd"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
				pClientTips->ShowWindow(FALSE);
			}
			else
			{
				m_quePostMessage.push(m_tPostMessage);
			}
		}
	}
}

afx_msg void CMainWnd::OnHandleUpdateCheck(WPARAM wParam, LPARAM lParam)
{	
	m_bIsUpdateChking = FALSE;
	if (lParam == RET_SUCCESS)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("升级成功检测ok！"));	
		setTrayTips(_T("已经升级到最新版本!"),m_strWindowsName,m_strWindowsName);

		setVersion();
	}
	else
	{	
		setTrayTips(_T("升级失败，正在尝试重新升级!"),m_strWindowsName,m_strWindowsName);
	}
	GetEncodeUpdateData(m_strUpdateEncodeData, m_dwClientType);
	PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_UIHELPER,MSG_PRODUCT_UPDATE,WPARAM_AUTOCHECK_UPDATE,(LPARAM)&m_strUpdateEncodeData);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("升级成功检测ok！自动升级检测消息已由界面发出MSG_PRODUCT_UPDATE wparm=102！"));
}

/*************************系统notify 消息*****************************************/
//点击登录按钮
afx_msg void CMainWnd::OnNotifyLoginCilck(TNotifyUI& msg)
{	
	if (m_bIsUseLogin)
	{	
		//发送登录客户端消息
		PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_LOGIN_SHANGZHOU,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户点击用户名，发送一键登录商舟网消息:%s"),m_strEncodeString);
		return;
	}
	::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
}

//响应设置按钮消息
afx_msg void CMainWnd::OnNotifySetting(TNotifyUI& msg) 					
{
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到用户点击设置消息!"));

	if (m_SettingWnd.GetHWND() != NULL)
	{
		return;
	}

	CButtonUI* pBtnSetting = (CButtonUI*)m_PaintManager.FindControl(_T("setbtn"));
	RECT ret = pBtnSetting->GetPos();

	POINT pt;
	pt.x = ret.right;
	pt.y = ret.bottom;
	::ClientToScreen(m_hWnd,&pt);	

	m_SettingWnd.SetPoint(pt,pBtnSetting->GetWidth(),pBtnSetting->GetHeight());
	m_SettingWnd.SetParentWnd(m_hWnd);
	if (m_bIsUseLogin)
	{
		m_SettingWnd.SetLogintext(_T("注销登录"));
	}
	else
	{
		m_SettingWnd.SetLogintext(_T("登录"));
	}
	m_SettingWnd.Create(NULL, _T("MasterZ_Settingwnd"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("设置框弹出完成！"));
}

//垃圾清理完成后返回
afx_msg void CMainWnd:: OnNotifyClearBack(TNotifyUI& msg)
{	
	//还原垃圾清理初始化状态
	m_dwClearStateFlag = CLEAR_STATE_INTIAL;
	m_pProgressTest->SetValue(0);
	m_pControl->SelectItem(PAGE_CLEAR_INDEX);	
}

//切换option
afx_msg void CMainWnd::OnNotifyClickOption(TNotifyUI& msg)
{	
	//关键词统计页面
	if (msg.pSender->GetName() == _T("option_keyword1") || msg.pSender->GetName() == _T("option_keyword2"))
	{	
		m_pControl->SelectItem(PAGE_KEYWORD_COUNT);
		COptionUI* pOption1 = (COptionUI*)m_PaintManager.FindControl(_T("option_keyword"));
		pOption1->Selected(true);
//		m_showchart.LoadKeyWordCountChart(m_tDataSets.keyWordList);
		if (!m_bIsUseLogin && !m_bIsAutoLogin)
		{
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
			return;
		}
		//动态获取一次详细信息数据
		if (m_bIsUseLogin)
		{	
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_KEYWORD_PRODUCT_DATA,1,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("option切换,获取产品详细信息消息已经由界面发出!"));
		}
	}
	//产品曝光页面
	if (msg.pSender->GetName() == _T("option_exposure") || msg.pSender->GetName() == _T("option_exposure2") )
	{	
		m_pControl->SelectItem(PAGE_PRODUCT_EXPOSURE);
		COptionUI*pOption5 = (COptionUI*)m_PaintManager.FindControl(_T("option_exposure1"));
		pOption5->Selected(true);

//		m_showchart.LoadExposureCountChart(m_tDataSets.productList);

		if (!m_bIsUseLogin && !m_bIsAutoLogin)
		{
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
			return;
		}
		//动态获取一次详细信息数据
		if (m_bIsUseLogin)
		{	
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_KEYWORD_PRODUCT_DATA,1,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("option切换,获取产品详细信息消息已经由界面发出!"));
		}
	}
	//关键词排名页面
	if (msg.pSender->GetName() == _T("option_keywordrank1") || msg.pSender->GetName() == _T("option_keywordrank"))
	{
		m_pControl->SelectItem(PAGE_KEYWORD_RANK);

		COptionUI*pOption9 = (COptionUI*)m_PaintManager.FindControl(_T("option_keywordrank2"));
		pOption9->Selected(true);

		if (!m_bIsUseLogin && !m_bIsAutoLogin)
		{	
			if (m_bIsUserLoginOut)
			{
				ShowRankUiByState(RANKUI_LOGINOUT);
			}
			else		//当连接服务器失败时，如果未登陆时则无需清空排名页面
			{	
				if (!m_bIsRankRunning)
				{
					ShowRankUiByState(RANKUI_SAVEOLD);
				}
			}
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		}
		if (m_bIsUseLogin && m_bIsNewestVersion)
		{	
			//判断当前刷新关键词状态，如果说当前没有正在刷新关键词那么且m_bIsRankRunning没有正在获取,那么发刷新关键词排名消息----20141125
// 			if (m_iYunMsgType != 0)
// 			{
// 				/*ShowRankUiByState(RANKUI_RUNNING);*/
// 				PackagMessage(E_THREAD_YUN_TASK, E_THREAD_TYPE_UIHELPER, MSG_CHECK_REFRESH_KEYWORD_STATA, 0, 0);
// 				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测当前刷新关键词排名模块状态消息已经由界面发出！"));
// 			}
// 			else
// 			{
			ShowRankUiByState(RANKUI_RUNNING);
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到切换到了关键词页面,请求关键词数据消息已发出!"));			
			PackagMessage(E_THREAD_DATASTATISTICS, E_THREAD_TYPE_UIHELPER, MSG_KEYWORD_RESULT_DATA, 1, (LPARAM)(&m_tUserInfo), FALSE, TRUE);				
			//}
		}
		if (m_pPageCombox->IsVisible())
		{
			m_pPageCombox->SelectItem(m_icurPage,TRUE);
		}
	}
	//微信通用户请求曲线页面
	if (msg.pSender->GetName() == _T("option_request"))
	{	
		m_showchart.LoadUserRequestChart(m_tWeiXinData);
		if (!m_bIsUseLogin && !m_bIsAutoLogin)
		{
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		}

		//add by zhumingxing 20141226-------重新请求微信通数据
		if (m_bIsUseLogin)
		{
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_WEIXIN_DATA,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取微信通数据消息已经由界面发出！"));
		}
		return;
	}
	//微信通用户关注曲线页面
	if (msg.pSender->GetName() == _T("option_care"))
	{	
		m_showchart.LoadUserCareChart(m_tWeiXinData);

		if (!m_bIsUseLogin && !m_bIsAutoLogin)
		{
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		}
		//add by zhumingxing 20141226-------重新请求微信通数据
		if (m_bIsUseLogin)
		{
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_WEIXIN_DATA,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取微信通数据消息已经由界面发出！"));
		}
		return;
	}
}	

//响应selectChange消息
afx_msg void CMainWnd::OnNotifySelectchanged(TNotifyUI& msg)
{	
	CDuiString name = msg.pSender->GetName();

	if(_tcsicmp(name, _T("optionCount")) == 0)
	{	
/*
		COptionUI*pOption1 = (COptionUI*)m_PaintManager.FindControl(_T("option_keyword"));
		pOption1->SetVisible(true);
		pOption1->Selected(true);
		COptionUI*pOption2 = (COptionUI*)m_PaintManager.FindControl(_T("option_exposure"));
		pOption2->SetVisible(true);
		pOption2->Selected(false);

		m_pControl->SelectItem(PAGE_KEYWORD_COUNT);
//		m_showchart.LoadKeyWordCountChart(m_tDataSets.keyWordList);

		if (!m_bIsUseLogin && !m_bIsAutoLogin)
		{
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
			return;
		}
		if (m_bIsUseLogin)
		{	
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_KEYWORD_PRODUCT_DATA,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("option切换,获取产品详细信息消息已经由界面发出!"));
		}*/

		m_pControl->SelectItem(PAGE_KEYWORD_COUNT);
	}
	else if(_tcsicmp(name, _T("optionAnalysis")) == 0)
	{	
		m_pControl->SelectItem(PAGE_KEYWORD_ANALYSIS);
	}

	else if(_tcsicmp(name, _T("optionSearch")) == 0)
	{	
		m_pControl->SelectItem(PAGE_WEBSITE_SEARCH);
	}
	else if(_tcsicmp(name, _T("optionWebsite")) == 0)
	{	
		if (!m_bIsUseLogin && !m_bIsAutoLogin)
		{	
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		}
		//add by zhumingxing 20141226-------重新请求建站信息
		if (m_bIsUseLogin)
		{
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_GET_JZ_INFORMATION,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取建站系统数据消息已经由界面发出！"));
		}
		if (m_TjzData.iSuccessFlag == -1)
		{
			m_pControl->SelectItem(PAGE_SZ_NOWEBSITE);
			return;
		}
		else
		{
			m_pControl->SelectItem(PAGE_SZ_WEBSITE);
		}
	}
	else if(_tcsicmp(name, _T("wechat")) == 0)
	{	
		if (m_tWeiXinData.iSuccessFlag == -1)
		{	
			m_pControl->SelectItem(PAGE_NO_WEIXIN_USER);
		}
		else
		{	
			m_pControl->SelectItem(PAGE_WEIXIN_DATA);
			COptionUI*pOptionCare = (COptionUI*)m_PaintManager.FindControl(_T("option_care"));
			pOptionCare->Selected(true);

			m_showchart.LoadUserCareChart(m_tWeiXinData);
		}
		if (!m_bIsUseLogin && !m_bIsAutoLogin)
		{	
			::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		}
		//add by zhumingxing 20141226-------重新请求微信通数据
		if (m_bIsUseLogin)
		{
			PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_WEIXIN_DATA,0,(LPARAM)(&m_tUserInfo),FALSE,TRUE);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("获取微信通数据消息已经由界面发出！"));
		}
	}
}

afx_msg void CMainWnd::OnNotifyKeyWordAnysis(TNotifyUI& msg)
{
	if (!IsInternetConnect())
	{
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_NETWORKERROR_WND,0);
		return;
	}
	CListUI*m_pList = (CListUI*)m_PaintManager.FindControl(_T("analysisResultView"));
	m_pList->RemoveAll();
	CLabelUI* pError = (CLabelUI*)m_PaintManager.FindControl(_T("anysis_error"));
	pError->SetText(_T(""));

	CEditUI* pText1 = (CEditUI*)m_PaintManager.FindControl(_T("word1"));
	CEditUI* pText2 = (CEditUI*)m_PaintManager.FindControl(_T("word2"));
	CEditUI* pText3 = (CEditUI*)m_PaintManager.FindControl(_T("word5"));
	CEditUI* pText4 = (CEditUI*)m_PaintManager.FindControl(_T("word6"));

	if (pText1->GetText().IsEmpty() || pText2->GetText().IsEmpty() || pText3->GetText().IsEmpty() || pText4->GetText().IsEmpty())
	{
		pError->SetText(_T("*部分必填"));
		return;
	}

	m_strDataKey.Empty();
	CString strCtrlName = _T("");
	for (int i = 1; i <=6; ++i)
	{
		strCtrlName.Format(_T("word%d"),i);
		CEditUI* pEdit = (CEditUI*)m_PaintManager.FindControl(strCtrlName);
		m_strDataKey += pEdit->GetText();
		m_strDataKey += _T(",");
	}

	CButtonUI* pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("btn_search_anysis"));
	pbutton->SetVisible(false);

	//显示动态
	pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("btn_aynsis_loading"));
	pbutton->SetVisible(true);

	//显示正在查询请稍后
	CLabelUI* pLable = (CLabelUI*)m_PaintManager.FindControl(_T("lable_aynsis_running"));
	pLable->SetVisible(true);

	//显示取消
	pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("btn_aynsis_cancel"));
	pbutton->SetVisible(true);

	pLable = (CLabelUI*)m_PaintManager.FindControl(_T("lable_aynsis_tips"));
	pLable->SetVisible(false);

	PackagMessage(E_THREAD_UTILITY, E_THREAD_TYPE_UIHELPER, MSG_KEYWORD_ANALYSIS, (WPARAM)&m_strDataKey, 0);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("关键词分析消息已经由界面发出!发送字符串:%s"),m_strDataKey);
}

afx_msg void CMainWnd::OnNotifySearch(TNotifyUI& msg)
{	
	if (!IsInternetConnect())
	{
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_NETWORKERROR_WND,0);
		return;
	}

	CLabelUI*pError = (CLabelUI*)m_PaintManager.FindControl(_T("search_error"));
	pError->SetText(_T(""));
	CEditUI*pEdit = (CEditUI*)m_PaintManager.FindControl(_T("text_search"));
	m_strUrl = pEdit->GetText();

	//去掉前后空格
	CString strTempUrl = m_strUrl;
	strTempUrl.TrimLeft(); strTempUrl.TrimRight();

	if (strTempUrl.IsEmpty())
	{
		return;
	}
	//清空原有信息
	ClearRecodeOfSearch();

	CButtonUI* pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("btn_search_start"));
	pbutton->SetVisible(false);

	//显示动态
	pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("btn_search_loading"));
	pbutton->SetVisible(true);

	//显示取消综合查询按钮
	pbutton = (CButtonUI*)m_PaintManager.FindControl(_T("btn_search_cancel"));
	pbutton->SetVisible(true);

	PackagMessage(E_THREAD_UTILITY, E_THREAD_TYPE_MGR, MSG_WEB_SEARCH, WPARAM(&m_strUrl), 0);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("网站综合查询消息已经由界面发出!发送URL:%s"),m_strUrl);
}

afx_msg void CMainWnd::OnNotifyEnterWeixin(TNotifyUI& msg)
{
	if (!IsInternetConnect())
	{	
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_NETWORKERROR_WND,0);
		return;
	}
	if (!m_bIsUseLogin && !m_bIsAutoLogin)
	{
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		return;
	}
	PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_LOGIN_WEIXIN,0, (LPARAM)(&m_tUserInfo),FALSE,TRUE);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("进入微信通消息已经由界面发出!"));
}

//进入舟大师产品推广
afx_msg void  CMainWnd::OnNotifyEnterPost(TNotifyUI& msg)
{
	if (!IsInternetConnect())
	{	
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_NETWORKERROR_WND,0);
		return;
	}
	if (!m_bIsUseLogin && !m_bIsAutoLogin)
	{
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		return;
	}
	PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_PRODUCT_POST,WARAM_ENTER_PRODUCT_POST, (LPARAM)(&m_tUserInfo),FALSE,TRUE);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("进入产品一键推广消息已经由界面发出!"));
}

//进入舟大师产品展示
afx_msg void  CMainWnd::OnNotifyEnterShow(TNotifyUI& msg)
{
	if (!IsInternetConnect())
	{	
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_NETWORKERROR_WND,0);
		return;
	}
	if (!m_bIsUseLogin && !m_bIsAutoLogin)
	{
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		return;
	}
	PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_PRODUCT_POST,WARAM_ENTER_PRODUCT_SHOW, (LPARAM)(&m_tUserInfo),FALSE,TRUE);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("进入产品一键展示消息已经由界面发出!"));
}

//窗口初始化消息响应
afx_msg void CMainWnd::OnNotifyWindowsInitial(TNotifyUI& msg)
{
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("进入windowinit方法！"));

	////
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("完成近关键词变化图表默认加载，出windowinit方法！"));

	
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("设置关键词变化flash控件默认隐藏完成！"));
	//加载第二个图表网页
/*
	m_pActiveXExposureFlash = static_cast<CWebBrowserUI*>(m_PaintManager.FindControl(_T("flash_exposurecount")));

	//	m_pActiveXFlash->SetVisible(false);
	if (m_pActiveXExposureFlash != NULL)
	{
		m_pActiveXExposureFlash->SetDelayCreate(false);
		CCustomWebEventHandler *pWebHandle = new CCustomWebEventHandler;
		pWebHandle->SetMainHwnd(m_hWnd);
		m_pActiveXExposureFlash->SetWebBrowserEventHandler(pWebHandle);
		m_pActiveXExposureFlash->Navigate2(_T("about:blank"));
		m_pActiveXExposureFlash->Navigate2(m_strEmptyExposurePath.GetBuffer());

		m_showchart.SetWebPoint(m_pActiveXExposureFlash);

	}

	PostMessage(MSG_SHOW_EXPOSURECHART);*/
}

//查看快照
afx_msg void CMainWnd::OnNotifyCheckSnap(TNotifyUI& msg)
{
	CString strCheckBtnName = msg.pSender->GetName();
	strCheckBtnName.Replace(_T("rank_check"),_T(""));
	strCheckBtnName.TrimLeft();
	strCheckBtnName.TrimRight();
	int iindex = _ttoi(strCheckBtnName);
	CString strSnapURl = CString(m_tKeyWordRank.keyList[m_icurPage*8+iindex].szLocalFile);

	if (strSnapURl.IsEmpty())
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("快照地址为空！"));
		return;
	}

	strSnapURl += _T("?");
	ShellExecute(NULL, _T("open"), strSnapURl, NULL, NULL, SW_NORMAL);
}

//开通产品
afx_msg void CMainWnd::OnNotifyBuyProduct(WORD wtype)
{	
	if (!IsInternetConnect())
	{	
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_NETWORKERROR_WND,0);
		return;
	}
	if (!m_bIsUseLogin && !m_bIsAutoLogin)
	{
		::PostMessage(m_hWnd,WM_SHOW_MODAL,WPARAM_SHOW_LOGIN_WND,0);
		return;
	}

	if (wtype == PRODUCT_WEIXIN)
	{
		
		m_tRegistweixin = m_tUserInfo;
		CString str = _T("");
		CString strname = URLEncode((CString)DecodeString(m_tRegistweixin.strUserName));
		str.Format(_T("userName=%s&clientProductId=%d&productId=%d"),strname,m_tRegistweixin.iProductId,m_tRegistweixin.iWeiXinId);
		m_tRegistweixin.strParam = str;

		PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_BUY_PRODUCT,0,(LPARAM)(&m_tRegistweixin),FALSE,TRUE);	
	}
	if (wtype == PRODUCT_JZ)
	{
		m_tJzInfo = m_tUserInfo;
		CString str = _T("");
		CString strname = URLEncode((CString)DecodeString(m_tJzInfo.strUserName));
		str.Format(_T("userName=%s&clientProductId=%d&productId=%d"),strname,m_tJzInfo.iProductId,m_tJzInfo.iJzId);
		m_tJzInfo.strParam = str;

		PackagMessage(E_THREAD_DATASTATISTICS,E_THREAD_TYPE_UIHELPER,MSG_BUY_PRODUCT,0,(LPARAM)(&m_tJzInfo),FALSE,TRUE);	
	}
}

void CMainWnd::SetClientType( DWORD dwType )
{
	m_dwClientType = dwType;
}

void CMainWnd::OnHandleGetProlemList( WPARAM wParam, LPARAM lParam )
{
	
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到诊断问题列表返回消息!"));

	T_DATA_FROM_XML tProList = *((T_DATA_FROM_XML*)(wParam));	

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("共有%d条问题需要诊断!"),tProList.vProblem.size());

	//设置问题列表参数
	m_CheckProWnd.SetProblemList(tProList);
}

void CMainWnd::OnHandleCheckProMsg( WPARAM wParam, LPARAM lParam )
{	
	//向诊断模块发送诊断消息
	if (lParam == LPARAM_CHILD_TO_MIAN)
	{	
		if (m_bIsNewestVersion)
		{
			PackagMessage(E_THREAD_CLEAR,E_THREAD_TYPE_MGR,MSG_SELF_DIAGNOSIS,WPARAM_CLIENT_NEW_VERSION,0);
		}
		else
		{
			PackagMessage(E_THREAD_CLEAR,E_THREAD_TYPE_MGR,MSG_SELF_DIAGNOSIS,WPARAM_CLIENT_OLD_VERSION,0);
		}
	}
	//此时为诊断模块返回的诊断结果消息，此时需要对诊断窗口界面进行相关操作
	else
	{	
		if (m_CheckProWnd.GetHWND() == NULL || m_CheckProWnd.GetCancelFlag())
		{
			return;
		}
		else
		{	
			T_PROBLEM_DATA tBackItemData = *((T_PROBLEM_DATA*)(wParam));
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到问题诊断返回消息,诊断项:%s!"),tBackItemData.strIndex);

			m_CheckProWnd.HandleCheckRetrunMsg(tBackItemData);
		}
	}
}

void CMainWnd::OnHandleProRepairMsg( WPARAM wParam, LPARAM lParam )
{
	//向诊断模块发送诊断消息
	if (lParam == WPARAM_REPAIR_UPDATE)
	{
		if (m_CheckProWnd.GetHWND() == NULL)
		{
			return;
		}
		else
		{	
			T_PROBLEM_DATA tBackItemData;
			tBackItemData.bRepairFlag = wParam;
			tBackItemData.strIndex = _T("4");
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到问题修复返回结果消息,修复项:%s!"),tBackItemData.strIndex);

			m_CheckProWnd.HandleRepairRetrunMsg(tBackItemData);
		}
	}
	else if (lParam == LPARAM_CHILD_TO_MIAN)
	{	
		//升级修复直接调到升级模块,无需再调到诊断模块
		if (wParam == 4)
		{
			GetEncodeUpdateData(m_strUpdateEncodeData, m_dwClientType);
			PackagMessage(E_THREAD_UPDATE,E_THREAD_TYPE_UIHELPER,MSG_PRODUCT_UPDATE,WPARAM_REPAIR_UPDATE,(LPARAM)&m_strUpdateEncodeData);
		}
		else
		{
			PackagMessage(E_THREAD_CLEAR,E_THREAD_TYPE_MGR,MSG_REPAIR_FAULT,wParam,0);
		}
	}
	//此时为诊断模块返回的诊断结果消息，此时需要对诊断窗口界面进行相关操作
	else
	{
		if (m_CheckProWnd.GetHWND() == NULL)
		{
			return;
		}
		else
		{	
			T_PROBLEM_DATA tBackItemData = *((T_PROBLEM_DATA*)(wParam));
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到问题修复返回结果消息,修复项:%s!"),tBackItemData.strIndex);
			m_CheckProWnd.HandleRepairRetrunMsg(tBackItemData);
		}
	}
}

void CMainWnd::OnHandlePluginLose( WPARAM wParam, LPARAM lParam )
{	
	//lParam 0表示不需要激活诊断按钮，1表示需要激活诊断按钮
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到Web控件丢失消息,弹出诊断框，激活诊断按钮!"));

	if (m_dwClientType != MAIN_LINE_VERSION)
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("客户端为定制版本,直接返回!"));
		
		return;
	}

	OnTaryMenue(WPARAM_CHECK_PROBLEM,1);
}

//定制版本隐藏所有对话框弹出
void CMainWnd::HidePopWindow()
{	
	if (m_UpdateWnd.GetHWND() != NULL)
	{
		m_UpdateWnd.ShowWindow(false);
	}
	if (m_LoginWnd.GetHWND() != NULL)
	{
		m_LoginWnd.ShowWindow(false);
	}
	if (m_MessageWnd.GetHWND() != NULL)
	{
		m_MessageWnd.ShowWindow(false);
	}
	if (m_UpdateErrorWnd.GetHWND() != NULL)
	{
		m_UpdateErrorWnd.ShowWindow(false);
	}
	if (m_UpdateTipsWnd.GetHWND() != NULL)
	{
		m_UpdateTipsWnd.ShowWindow(false);
	}
	if (m_TrayWnd.GetHWND() != NULL)
	{
		m_TrayWnd.ShowWindow(false);
	}
	if (m_WarningWnd.GetHWND() != NULL)
	{
		m_WarningWnd.ShowWindow(false);
	}
	if (m_SettingWnd.GetHWND() != NULL)
	{
		m_SettingWnd.ShowWindow(false);
	}
	if (m_ClientSetWnd.GetHWND() != NULL)
	{
		m_ClientSetWnd.ShowWindow(false);
	}
	if (m_SafeExitWnd.GetHWND() != NULL)
	{
		m_SafeExitWnd.ShowWindow(false);
	}
	if (m_CheckProWnd.GetHWND() != NULL)
	{
		m_CheckProWnd.ShowWindow(false);
	}
}

//新增甑词数据消息处理
void CMainWnd::OnHandleZhenciMsg(WPARAM wParam, LPARAM lParam)
{
	if (m_dwClientType != MAIN_LINE_VERSION)
	{
		return;
	}
	//返回甄词结构数据
	m_tZcInfo = *(ZhenciInfo*)lParam;
	if (m_tZcInfo.strTitle.GetLength() <= 0 ||
		m_tZcInfo.strContent.GetLength() <= 0)
	{
		return;
	}

	//判断返回的数据是否为无关键词排名，是的话，只弹出一次大师甄词
	CString strHistory = _T("");
	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\zhenci.dat"), GetInstallPath());

	IXMLRW iniMcFile;
	iniMcFile.init(strCfgFile);
		
	iniMcFile.ReadString(_T("zhenci"), _T("ZhenciTime"), _T("historyuser"), strHistory);
	if (strHistory.Find(m_tZcInfo.strUserName, 0) != -1 &&
		m_tZcInfo.strTitle.Find(_T("大师甄词")) != -1 &&
		m_tZcInfo.strTitle.Find(_T("功能震撼来袭")) != -1)
	{
		return;
	}
	else
	{
		if (strHistory.Find(m_tZcInfo.strUserName) == -1)
		{
			strHistory += m_tZcInfo.strUserName + _T(",");
			iniMcFile.WriteString(_T("zhenci"),_T("ZhenciTime"), _T("historyuser"), strHistory);
		}
	}

	//----开始读取写入html网页文件
	DWORD dwSize = 10 * 1024; //10k
	char *pBuffer;
	CStdioFile webfile;
	//先读取
	if (!webfile.Open(m_strTemplatePath, CStdioFile::modeReadWrite | CStdioFile::typeBinary))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("打开甑词网页模板文件失败,文件路径%s"), m_strTemplatePath);
		return ;
	}

	DWORD dwLen = webfile.GetLength();
	pBuffer = new char[dwLen + 1];
	memset(pBuffer, 0, dwLen + 1);
	webfile.Read(pBuffer, dwLen);

	webfile.Close();
	//转换成宽字节
	MByteToWChar(pBuffer, NULL, &dwLen,CP_UTF8);
	TCHAR* pWebData = new TCHAR[dwLen];
	MByteToWChar(pBuffer, pWebData, &dwLen,CP_UTF8);

	int iLength = 0;
	CString strTempData = pWebData;

	delete[]pWebData;
	delete[]pBuffer;

	//开始替换；
	strTempData.Replace(_T("#Title#"), m_tZcInfo.strTitle);
	strTempData.Replace(_T("#Content#"), m_tZcInfo.strContent);
	strTempData.Replace(_T("#DetailURL#"), m_tZcInfo.strRequestUrl);
	strTempData.Replace(_T("#ViewURL#"), m_tZcInfo.strRequestUrl);
	//存放
	if (!webfile.Open(m_strZhenciPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("创建甑词网页文件失败,文件路径%s"), m_strZhenciPath);
		return ;
	}

	char* pMultiData = CStringToMutilChar(strTempData, iLength, CP_UTF8);

	webfile.Write(pMultiData, iLength);
	webfile.Close();

	//显示网页
	HWND hwnd = ::FindWindow(NULL, _T("MasterZ_zhenciwnd"));
	if (hwnd == NULL)
	{
		CZhenCiWnd* pZhenci = new CZhenCiWnd();
		pZhenci->SetInfo(m_strZhenciPath);
		pZhenci->Create(NULL, _T("MasterZ_zhenciwnd"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
		pZhenci->ShowWindow(false);
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("当前有甑词消息正在显示！"));
		return;
	}


}

//判断是否满足发送甄词消息
BOOL CMainWnd::CheckPostZhenci(CString strUser)
{
	//获取上次发送甄词时间
	BOOL bResult = FALSE;
	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\zhenci.dat"), GetInstallPath());
	CString strDate, strName;

	IXMLRW iniMcFile;
	iniMcFile.init(strCfgFile);

	iniMcFile.ReadString(_T("zhenci"),_T("ZhenciTime"), _T("lastpostdate"), strDate);
	iniMcFile.ReadString(_T("zhenci"),_T("ZhenciTime"), _T("olduser"), strName);

	CString strCurrDate;
	time_t tim;
	time(&tim);
	struct tm *t;
	t = localtime(&tim);

	strCurrDate.Format(_T("%d-%d-%d"), 1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday);
	//比较两个日期是否相同

	if (strCurrDate != strDate)
	{
		strName = _T("");
		iniMcFile.WriteString(_T("zhenci"), _T("ZhenciTime"), _T("olduser"), strName);
		iniMcFile.WriteString(_T("zhenci"), _T("ZhenciTime"), _T("lastpostdate"), strCurrDate);
		bResult = TRUE;
	}

	if (strName.Find(strUser) < 0)
	{
		strName += strUser +_T(",");
		iniMcFile.WriteString(_T("zhenci"), _T("ZhenciTime"), _T("olduser"), strName);
		bResult = TRUE;
	}		

	return bResult;
}

//获取IP并判断是否为公司电脑
BOOL CMainWnd::CheckIP()
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
	{
		return FALSE;
	}

	char host_name[255];
	if (gethostname(host_name, sizeof(host_name)) == SOCKET_ERROR)
	{
		WSACleanup();
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("Error %d when getting local host name\n", WSAGetLastError()));
		return FALSE;
	}
	printf("host name:%s\n", host_name);
	struct hostent *phe = gethostbyname(host_name);
	if (phe == 0)
	{
		WSACleanup();
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("Error host lookup\n"));
		return FALSE;

	}
	//遍历所有IP地址
	for (int i = 0; phe->h_addr_list[i] != 0; ++i)
	{
		DWORD dwLen;
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		char pBuffer[16] = { 0 };
		sprintf_s(pBuffer, inet_ntoa(addr));
		//转换成宽字节
		MByteToWChar(pBuffer, NULL, &dwLen);
		TCHAR* pIpAddr = new TCHAR[dwLen];
		MByteToWChar(pBuffer, pIpAddr, &dwLen);

		CString strTempData = pIpAddr;
		delete[]pIpAddr;

		if (strTempData.Left(6) == _T("198.18"))
		{
			WSACleanup();
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("遍历到网段内的IP地址:%s；"),strTempData.GetBuffer());
			return TRUE;
		}
		

	}
	WSACleanup();
	return FALSE;
}

//打印当前系统基本信息
void CMainWnd::PrintCurrSysInfo()
{
	try
	{
		CString strInfo;
		CString tmp;
		//获取用户的系统格式
		CReg reg;
		TCHAR* pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Control Panel\\International"), _T("Locale"));
		strInfo = pContent;
		pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Control Panel\\International"), _T("sCountry"));
		tmp = pContent;
		strInfo = strInfo + _T(",") + tmp;
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("当前系统的基本语言信息：%s"), strInfo.GetBuffer());
	}
	catch (...)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("获取基本信息失败！错误号：%d", GetLastError()));
	}

}

//新增开始刷5*20的排名
void CMainWnd::OnHandlePaimingMsg(WPARAM wParam, LPARAM lParam)
{
	if (m_dwClientType != MAIN_LINE_VERSION)
	{
		return;
	}

	CString strResult = *(CString*)lParam;
	//如果执行成功，关掉定时器
	if (strResult == _T("0"))
	{
		KillTimer(m_hWnd,ID_PAIMING_MSG);
	}
}

//判断客户端隔夜是否需要重启
void CMainWnd::CheckReBootClient()
{
	//检查是否下载安装包是否成功未安装
	if (m_bPostInstall)
	{
		//对下载完成的安装包进行静默安装
		CString strInstallExePath = GetDirectory(_T("%temp%"));
		strInstallExePath += _T("\\MasterZ_Slient_setup.exe");

		if (PathFileExists(strInstallExePath))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到客户长时间未关闭界面，正在强制执行安装客户端程序!"));
			ShellExecute(NULL, _T("Open"), strInstallExePath, _T("/S"), NULL, SW_HIDE);
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("temp目录下安装包不存在,等待第二天请求安装!"));
		}

	}

	//检查是否升级成功未重启
	if (m_bUpdateReboot)
	{
		if (m_wCurrentUpdateFlag == WPARAM_REPAIR_UPDATE)
		{
			return;
		}
		//重启客户端消息
		OperateTray(NIM_DELETE);
		PackagMessage(E_THREAD_UPDATE, E_THREAD_TYPE_MGR, MSG_PRODUCT_UPDATE, WPARAM_REBOOT_CLIENT, LPARAM_REBOOT_AUTO);

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测到客户长时间未关闭界面，强制重启客户端！"));
	}

}

//隐藏客户端
void CMainWnd::HideClient()
{
	OperateTray(NIM_DELETE);
	::ShowWindow(m_hWnd, SW_HIDE);
	m_dwClientType = HIDE_VERSION;

	CString strCfgFile = _T("");

	strCfgFile.Format(_T("%s\\data2\\version.dat"), GetInstallPath());

	CIniFile iniFile;
	iniFile.SetFilePath((CStdString)strCfgFile);

	iniFile.WriteInteger(_T("Control"), _T("HideClient"), HIDE_VERSION);

/*
	strCfgFile.Format(_T("%s\\data2\\mc.dat"), GetInstallPath());

	IXMLRW iniMcFile;
	iniMcFile.init(strCfgFile);

	iniMcFile.WriteInt(_T("MC"), _T("CONTROL"), _T("HideClient"), HIDE_VERSION);*/

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("用户点击退出，隐藏客户端！"));
}


bool CMainWnd::IsOwnerCtrlUserTaskProcess()
{
	HANDLE hMcMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T("_CTRL_USERTASK_GUI_"));

	if (hMcMutex != NULL)
	{
		CloseHandle(hMcMutex);
		hMcMutex = NULL;

		return TRUE;
	}
	return FALSE;
}
//判断进程是否存在
bool CMainWnd::ProcessExist(TCHAR *pstrProcName, HANDLE *phProcess)
{
	ASSERT(pstrProcName != NULL);

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		if (_tcsicmp(pstrProcName, pe32.szExeFile) == 0)
		{
			//需要传出进程句柄
			if (phProcess)
			{
				*phProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			}

			return true;
		}

		bMore = ::Process32Next(hProcessSnap, &pe32);
	}

	return false;
}

//启动进程
bool CMainWnd::StartProcess(TCHAR *pstrProcName, TCHAR *pstrPort, HANDLE *phProcess)
{
	ASSERT(pstrProcName != NULL);

	TCHAR strDir[MAX_PATH] = { _T('\0') };
	TCHAR strExePath[MAX_PATH] = { 0 };

	_tcscpy_s(strDir, pstrProcName);

	TCHAR *pch = _tcsrchr(strDir, _T('\\'));
	if (pch == NULL/* || !PathFileExists(pstrProcName)*/)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("pch == NULL"));
		return false;
	}
	//文件存在返回TRUE
	if (!PathFileExists(pstrProcName))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("PathFileExists:%s"), pstrProcName);
	}
	*pch = 0;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	si.dwFlags = STARTF_FORCEOFFFEEDBACK;

	if (CreateProcess(pstrProcName, pstrPort, NULL,
		NULL, FALSE, 0, NULL, strDir, &si, &pi) == TRUE)
	{
		if (NULL != phProcess)
			*phProcess = pi.hProcess;

		return true;
	}
	g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("执行%s失败！ err: %d"), pstrProcName, GetLastError());

	return false;
}
//结束指定进程
bool CMainWnd::StopProcess(TCHAR *pstrProcName)
{
	HANDLE hProcess = NULL;

	if (ProcessExist(pstrProcName, &hProcess))
	{
		if (!hProcess)
			TerminateProcess(hProcess, 1);
	}

	return true;
}

//结束指定进程
bool CMainWnd::StopProcess(HANDLE hProcess)
{
	if (!hProcess && TerminateProcess(hProcess, 1))
	{
		return true;
	}

	return false;
}

