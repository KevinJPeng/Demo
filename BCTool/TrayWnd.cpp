#include "StdAfx.h"
#include "TrayWnd.h"
#include <shlobj.h>
#include "UserDef.h"

const TCHAR* const kCancelButtonControlName = _T("closebtn");
const TCHAR* const kUpdateProgressControlName = _T("Progress_update");


CTrayWnd::CTrayWnd(void)
{
}


CTrayWnd::~CTrayWnd(void)
{
}

void CTrayWnd::SetParentWnd(HWND hWnd)
{	

	m_hParentWnd = hWnd;
}

void CTrayWnd::InitWindow()
{	
	//将对话框显示在鼠标右击的位置
	RECT rcDlg = { 0 };
	::GetWindowRect(m_hWnd, &rcDlg);
	::SetWindowPos(m_hWnd, HWND_TOPMOST, (m_point.x - rcDlg.right), (m_point.y - rcDlg.bottom), -1, -1, SWP_NOSIZE | SWP_NOZORDER);
	::SetForegroundWindow(m_hWnd);

	//COptionUI* PLogin = (COptionUI*)(m_PaintManager.FindControl(_T("nav6")));
	//PLogin->SetText(m_strLoginText);
}

CDuiString CTrayWnd::GetSkinFolder()
{
	 return _T("skin");
}

CDuiString CTrayWnd::GetSkinFile()
{
	return _T("Tray.xml");
}

LPCTSTR CTrayWnd::GetWindowClassName(void) const
{
	return _T("BCTOOL_Traywnd");
}

LRESULT CTrayWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	//屏蔽鼠标右键消息
	case WM_NCLBUTTONDOWN:
		{
			return 0;
		}
		break;
	//当窗口处于未激活状态时关闭，模拟托盘菜单效果
	case WM_ACTIVATE:
		{
			if(LOWORD(wParam) == WA_INACTIVE) 
			{ 
				Close();
			} 
			return 0;
		}
		break;
	default:
		break;
	}

	//屏蔽对标题栏的双击操作
	if(WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}

void CTrayWnd::Notify(TNotifyUI& msg)
{	
	if (msg.sType == _T("click"))
	{	
		//打开客户端
		if (msg.pSender->GetName() == _T("nav2"))
		{
			::SendMessage(m_hParentWnd, MSG_TRAY_FUNCTION_MSG, WPARAM_SHOW_CLIENT, 0);
			Close();
		}
		//安全退出
		if (msg.pSender->GetName() == _T("nav8"))
		{	
			::SendMessage(m_hParentWnd, MSG_TRAY_FUNCTION_MSG, WPARAM_SAFE_EXIT, 0);
			 Close();
		}
	}
	__super::Notify(msg);
}

void CTrayWnd::SetPoint(POINT point)
{
	m_point = point;
}

TCHAR* CTrayWnd::GetProgPath(void)
{

	static TCHAR s_szProgPath[MAX_PATH] = {0};
	SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 1, s_szProgPath);
	return s_szProgPath;

}
