#include "stdafx.h"
#include "Resource.h"
#include "indexWnd.h"
#include "CustomWebEventHandler.h"
#include <atlcomcli.h>
#include "UserDef.h"
#include "utility.h"


CIndexWnd::CIndexWnd(void)
{
	m_bThreadRunFlag = true;
	m_pBrowser = NULL;
	m_iDLFailCount = 0;
	m_iDLStatus = CLIENT_DL_NULL;
	m_bUserExit = false;
	m_bTwinkling = false;
	m_bVisible = true;
	m_iTwinklingCount = 0;
}
CIndexWnd::~CIndexWnd(void)
{

}
CDuiString CIndexWnd::GetSkinFolder()
{
	return _T("");
}
CDuiString CIndexWnd::GetSkinFile()
{
	CDuiString sSkinName = _T("");

	sSkinName = _T("index.xml");

	return sSkinName;
}
LPCTSTR CIndexWnd::GetWindowClassName(void) const
{
	return _T("_BCTOOL_GUI_");
}
LPCTSTR CIndexWnd::GetResourceID() const
{
	return MAKEINTRESOURCE(IDR_ZIPRES1);
}
UILIB_RESOURCETYPE CIndexWnd::GetResourceType() const
{
	return UILIB_ZIPRESOURCE;
}
CControlUI* CIndexWnd::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, _T("WebBrowserEx")) == 0)
	{
		return new CWebBrowserExUI;
	}

	return NULL;
}
void CIndexWnd::InitWindow()
{
//	::MessageBox(m_hWnd, _T("有新消息啦!"), _T("啦啦啦"), 0);

	::SetTimer(m_hWnd, WPARAM_TIMER_DL_CLIENTPACKAGE, 500, NULL);

	//::SetTimer(m_hWnd, 10, 10000, NULL);


	//初始化托盘
	OperateTray(NIM_ADD);

	//加载第一个图表网页
	m_pBrowser = static_cast<CWebBrowserExUI*>(m_PaintManager.FindControl(_T("web")));
	ASSERT(m_pBrowser);

	//	m_pBrowser->SetVisible(false);
	if (m_pBrowser != NULL)
	{
		m_pBrowser->SetDelayCreate(false);
		CCustomWebEventHandler *pWebHandle = new CCustomWebEventHandler;
		pWebHandle->SetMainHwnd(m_hWnd);
		m_pBrowser->SetWebBrowserEventHandler(pWebHandle);
		m_pBrowser->Navigate2(_T("about:blank"));
		//m_pBrowser->Navigate2(_T("C:/Users/Administrator/Desktop/skin/test.html"));   //http://198.18.0.254:8009/ClientPage/ChartStatisticData?userName=kehu0051

		//m_pBrowser->Navigate2(_T("http://192.168.1.223:8099/Personal/Manage"));   //http://198.18.0.254:8009/ClientPage/ChartStatisticData?userName=kehu0051
		//m_pBrowser->Navigate2(_T("http://192.168.1.245:1966/Personal/chat"));   //http://198.18.0.254:8009/ClientPage/ChartStatisticData?userName=kehu0051
		m_pBrowser->Navigate2(_T("http://47.107.165.10:10010/Login/index"));   //http://198.18.0.254:8009/ClientPage/ChartStatisticData?userName=kehu0051
	}

	CreateThread(NULL, 0, DoDataFromJSThread, this, 0, 0);
}


void CIndexWnd::OnClick(TNotifyUI& msg)
{
	CDuiString sCtrlName = msg.pSender->GetName();

	if (sCtrlName.CompareNoCase(_T("btnCheck")) == 0)
	{
		// C++调用js方法，示例
		// 注意参数顺序，反向
		VARIANT arg[2] = { CComVariant(7), CComVariant(3) };//JsFunSub(3,7)
		VARIANT varRet;
		m_pBrowser->InvokeMethod(m_pBrowser->GetHtmlWindow(), _T("JsFunSub"), &varRet, arg, 2);
		int nResult = varRet.intVal;//-4
		return;
	}


	WindowImplBase::OnClick(msg);
}


void CIndexWnd::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);

	delete this;
}


LRESULT CIndexWnd::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// 退出程序
	PostQuitMessage(0);

	bHandled = FALSE;
	return 0;
}
void CIndexWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("closebtn_index"))
		{
			::PostMessage(m_hWnd, MSG_PROCESS_EXIT, WPARAM_PROCESS_EXIT_MANUAL, 0);
		}
	}
	else if (msg.sType == _T("selectchanged"))
	{
		//OnNotifySelectchanged(msg);
	}
	//窗口初始化后的第一条消息
	else if (msg.sType == _T("windowinit"))
	{
		//OnNotifyWindowsInitial(msg);
	}
	__super::Notify(msg);
}
LRESULT CIndexWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
		case VK_RETURN:
		case VK_SPACE:
		case VK_ESCAPE://拦截ESC退出界面
			return FALSE;
		default:
			break;
		}
	}
	return FALSE;
}

LRESULT CIndexWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	try
	{
		//屏蔽鼠标右键消息
// 		if (uMsg == WM_MOUSEACTIVATE)
// 		{
// 			if (lParam == MOUSE_RIGHT_ACTIVE)
// 			{
// 				return MA_ACTIVATEANDEAT;
// 			}
// 			return 0;
// 		}
		//屏幕alt+f4键
		if (uMsg == WM_HOTKEY)
		{
			return 0;
		}
		//相关定时器消息集中处理
		if (uMsg == WM_TIMER)
		{
			OnTimerMsg(wParam, lParam);
			//return 0;			//注意此处不能够return 否则会导致gif不能正常显示
		}

// 		if (uMsg == WM_SYSCOMMAND)
// 		{
			if (m_bTwinkling && HasFocus()/*wParam == SC_RESTORE*/)
			{
				StopTwinkling();
			}
//		}
		//屏蔽对标题栏的双击操作 
		if (WM_NCLBUTTONDBLCLK != uMsg)
			return __super::HandleMessage(uMsg, wParam, lParam);
		else
			return 0;
	}
	catch (...)
	{
	}
}
void CIndexWnd::OnTimerMsg(WPARAM wParam, LPARAM lParam)
{
	if (WPARAM_TIMER_DL_CLIENTPACKAGE == wParam)
	{
		CreateThread(NULL, 0, ThreadWorkDL, this, 0, NULL);
	}
	else if (WPARAM_TRAYICON_TWINKLING == wParam)
	{
// 		if (m_hWnd != GetForegroundWindow())
// 		{
// 			FlashWindow(m_hWnd, TRUE);
// 			////    this->ShowWindow(SW_RESTORE);
// 			////    MoveWindow(&m_rect);
// 			HWND hCurwnd = NULL;
// 			hCurwnd = ::GetForegroundWindow();
// 			DWORD threadID = ::GetCurrentThreadId();
// 			DWORD threadprocessid = ::GetWindowThreadProcessId(hCurwnd, &threadID);
// 			::AttachThreadInput(threadID, threadprocessid, TRUE);
// 			::SetForegroundWindow(m_hWnd);
// 			::AttachThreadInput(threadID, threadprocessid, FALSE);
//		}


		if (m_bVisible)
		{
			m_bVisible = false;
			m_NotifyIcon.hIcon = NULL;
			//m_NotifyIcon.uFlags = NIF_ICON;
			Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIcon);

		}
		else
		{
			m_bVisible = true;
			m_NotifyIcon.hIcon = ::LoadIcon(m_PaintManager.GetInstance(), MAKEINTRESOURCE(IDI_BCTOOL_NEW));
			//m_NotifyIcon.uFlags = NIF_ICON;
			Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIcon);
		}

	}
	else if (WPARAM_ICON_TWINKLING == wParam)
	{
		m_iTwinklingCount++;
		FlashWindow(m_hWnd, true);
		if (m_iTwinklingCount >= 2)
		{
			m_iTwinklingCount = 0;
			::KillTimer(m_hWnd, WPARAM_ICON_TWINKLING);
		}
	}
	else if (10 == wParam)
	{
		g_utilityVar.sJsData = "1(;0)2(;0)深圳";
		//消息来源(;0)消息类型(;0)数据  1(;0)2(;0)深圳
		if (-1 != g_utilityVar.sJsData.find("(;0)"))
		{
			SetEvent(g_utilityVar.hRecvJSData);
			LOG_INFO(g_utilityVar.loggerId, "收到JS传过来的数据：" << g_utilityVar.sJsData);
		}
		LOG_ERROR(g_utilityVar.loggerId, "收到JS传过来的数据：" << g_utilityVar.sJsData);
	}

}

/*******************************用户消息处理************************************/
LRESULT CIndexWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (uMsg)
	{
	//处理托盘系统消息
	case WM_TRAY_MSG:
	{
		OnOpearateTray(wParam, lParam);
		break;
	}
	//处理托盘菜单消息
	case MSG_TRAY_FUNCTION_MSG:
	{
		OnTaryMenue(wParam, lParam);
		break;
	}
	//处理托盘菜单消息
	case MSG_FROM_SERVWE:
	{
		OnServerMsg(wParam, lParam);
		break;
	}
	//处理托盘菜单消息
	case MSG_PROCESS_EXIT:
	{
		if (WPARAM_PROCESS_EXIT_MANUAL == wParam)
		{
			ShowWindow(false);
			OperateTray(NIM_DELETE);
			m_bUserExit = true;
		}
		else if (WPARAM_PROCESS_EXIT_AUTO == wParam)
		{
			m_iDLStatus = CLIENT_DL_OVER;
		}
		if (isExit())
		{
			PostQuitMessage(0);
		}

		break;
	}

	default:
		break;
	}
	return 0;
}

//托盘操作
void CIndexWnd::OperateTray(DWORD dwType)
{
	m_NotifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_NotifyIcon.hWnd = m_hWnd;
	m_NotifyIcon.uID = IDI_BCTOOL_NEW;
	m_NotifyIcon.hIcon = ::LoadIcon(m_PaintManager.GetInstance(), MAKEINTRESOURCE(IDI_BCTOOL_NEW));
	m_NotifyIcon.uCallbackMessage = WM_TRAY_MSG;
	m_NotifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_NotifyIcon.dwInfoFlags = NIIF_INFO;

	if (dwType == NIM_ADD)
	{
		_tcscpy((LPTSTR)m_NotifyIcon.szTip, _T("获客工具"));
		Shell_NotifyIcon(NIM_ADD, &m_NotifyIcon);
	}
	if (dwType == NIM_DELETE)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIcon);
	}
}
//托盘响应
void CIndexWnd::OnOpearateTray(WPARAM wParam, LPARAM lParam)
{
	//此处修改为WM_RBUTTONUP以解决win8系统上弹出任务栏菜单的问题,原因可能是消息冲突
	if (lParam == WM_RBUTTONUP)
	{
		if (m_TrayWnd.GetHWND() != NULL)
		{
			//for win8 OS
			m_TrayWnd.Close();
			return;
		}

		POINT point;
		GetCursorPos(&point);

		m_TrayWnd.SetPoint(point);
		m_TrayWnd.SetParentWnd(m_hWnd);
		m_TrayWnd.Create(NULL, _T("BCTOOL_Traywnd"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
	}
	//双击托盘
	if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONDOWN)
	{
		//显示对话框，居中,前端显示		
		ShowWindow();
		CenterWindow();
		::SetForegroundWindow(m_hWnd);
		//停止托盘图标闪烁
		StopTwinkling();
	}
}

//托盘菜单响应
void CIndexWnd::OnTaryMenue(WPARAM wParam, LPARAM lParam)
{
	//显示客户端，居中
	if (wParam == WPARAM_SHOW_CLIENT)
	{
		if (!::IsWindowVisible(m_hWnd) || ::IsIconic(m_hWnd))
		{
			ShowWindow();
			CenterWindow();
		}
		::SetForegroundWindow(m_hWnd);
	}
	//安全退出【需要进行相关进行的kill处理】
	else if (wParam == WPARAM_SAFE_EXIT)
	{
		::PostMessage(m_hWnd, MSG_PROCESS_EXIT, WPARAM_PROCESS_EXIT_MANUAL, 0);
	}
}
DWORD CIndexWnd::GetCurrentActiveWindowsProcessId()
{
	HWND hWnd = GetActiveWindow();
	DWORD processId = 0;
	GetWindowThreadProcessId(hWnd, &processId);
	return processId;
}

BOOL CIndexWnd::HasFocus()
{
	DWORD active_process = GetCurrentActiveWindowsProcessId();
	DWORD current_process = ::GetCurrentProcessId();
	return current_process == active_process;
}

//托盘开始闪烁
bool CIndexWnd::StartTwinkling()
{
	if (!m_bTwinkling && (!HasFocus()))
	{
		::SetTimer(m_hWnd, WPARAM_TRAYICON_TWINKLING, 500, NULL);
		::SetTimer(m_hWnd, WPARAM_ICON_TWINKLING, 1200, NULL);
		
		m_bTwinkling = true;
	}

	return true;
}
//托盘停止闪烁
void CIndexWnd::StopTwinkling()
{
	if (m_bTwinkling)
	{
		m_bTwinkling = false;
		::KillTimer(m_hWnd, WPARAM_TRAYICON_TWINKLING);
		::KillTimer(m_hWnd, WPARAM_ICON_TWINKLING);
		m_iTwinklingCount = 0;

		m_bVisible = true;
		m_NotifyIcon.hIcon = ::LoadIcon(m_PaintManager.GetInstance(), MAKEINTRESOURCE(IDI_BCTOOL_NEW));
		//m_NotifyIcon.uFlags = NIF_ICON;
		Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIcon);
	}

}

void CIndexWnd::OnServerMsg(WPARAM wParam, LPARAM lParam)
{
	//来新消息啦
	if (wParam == WPARAM_NEWINFO_MSG)
	{
		StartTwinkling();
		//::MessageBox(m_hWnd, _T("有新消息啦!"), _T("啦啦啦"), 0);
	}
}

bool CIndexWnd::SplitString(const string & input, const string & delimiter, std::vector<string >& results)
{
	int iPos = 0;
	int newPos = -1;
	int sizeS2 = (int)delimiter.length();
	int isize = (int)input.length();

	int offset = 0;
	string  s;

	if (
		(isize == 0)
		||
		(sizeS2 == 0)
		)
	{
		return 0;
	}

	std::vector<int> positions;

	newPos = input.find(delimiter, 0);

	if (newPos < 0)
	{
		if (!input.empty())
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
			newPos = input.find(delimiter, iPos + sizeS2);
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
		s.clear();
		if (i == 0)
		{
			s = input.substr(i, positions[i]);
		}
		else
		{
			offset = positions[i - 1] + sizeS2;

			if (offset < isize)
			{
				if (i == positions.size())
				{
					s = input.substr(offset);
				}
				else if (i > 0)
				{
					s = input.substr(positions[i - 1] + sizeS2, positions[i] - positions[i - 1] - sizeS2);
				}
			}
		}

		if ((s.length() > 0))
		{
			//去首尾空格
			s.erase(0, s.find_first_not_of(" "));
			s.erase(s.find_last_not_of(" ") + 1);
			//s = s.Trim();
			results.push_back(s);
		}
	}
	return true;
}

DWORD WINAPI CIndexWnd::DoDataFromJSThread(LPVOID lpParameter)
{
	CIndexWnd* pThis = (CIndexWnd*)lpParameter;
	pThis->DoDataFromJS();
	return 0;
}
void CIndexWnd::DoDataFromJS()
{
	while (m_bThreadRunFlag)
	{
		WaitForSingleObject(g_utilityVar.hRecvJSData, INFINITE);
		std::vector<string > v_results;
		SplitString(g_utilityVar.sJsData, "(;0)", v_results);
		int ilen = v_results.size();

		LOG_INFO(g_utilityVar.loggerId, "收到JS数据：" << g_utilityVar.sJsData);
		if (ilen >= 2)
		{
			if ("1" == v_results[0])
			{
				//数据来自服务器
				if ("2" == v_results[1])
				{
					//有新消息到来
					::PostMessage(m_hWnd, MSG_FROM_SERVWE, WPARAM_NEWINFO_MSG, 0);
				}
			}
		} 
		else
		{
			LOG_ERROR(g_utilityVar.loggerId, "收到的JS数据格式错误：" << g_utilityVar.sJsData);
		}
	}
}
void CIndexWnd::ReSetTimer()
{
	//定时器时间设定规则  i^2 * 2 (分钟)
	m_iDLFailCount++;
	if (m_iDLFailCount < 1)
	{
		m_iDLFailCount = 1;
	}
	else if (m_iDLFailCount > 10)
	{
		m_iDLFailCount = 10;
	}
	::SetTimer(m_hWnd, WPARAM_TIMER_DL_CLIENTPACKAGE, (m_iDLFailCount * m_iDLFailCount * 2 * 60 * 1000), NULL);
}
bool CIndexWnd::isExit()
{
	if (m_bUserExit && CLIENT_DL_OVER == m_iDLStatus)
	{
		return true;
	}

	return false;
}

DWORD WINAPI CIndexWnd::ThreadWorkDL(LPVOID lpParameter)
{
	CIndexWnd* pThis = (CIndexWnd*)lpParameter;
	pThis->DoDL();
	return 0;
}
void CIndexWnd::DoDL()
{
	::KillTimer(m_hWnd, WPARAM_TIMER_DL_CLIENTPACKAGE);
	m_iDLStatus = CLIENT_DL_START;
	CUtility utilityObj;
	utilityObj.SetParentWnd(m_hWnd);
	int iCode = utilityObj.DLAndInstallClient();
	if (CLIENT_CLIENT_RUN != iCode && CLIENT_START_SUCCESS != iCode && CLIENT_INSTALL_SUCCESS != iCode)
	{
		LOG_INFO(g_utilityVar.loggerId, "重新设定定时器，Code = " << iCode);
		ReSetTimer();
	}
	else
	{
		LOG_INFO(g_utilityVar.loggerId, "软件运行正常，Code = " << iCode);
		::PostMessage(m_hWnd, MSG_PROCESS_EXIT, WPARAM_PROCESS_EXIT_AUTO, 0);
	}
}
