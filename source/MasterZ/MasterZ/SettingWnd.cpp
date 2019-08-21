#include "StdAfx.h"
#include "SettingWnd.h"
#include <shlobj.h>
#include "..\..\threadmodel\MsgDef.h"


CSettingWnd::CSettingWnd(void)
{
}


CSettingWnd::~CSettingWnd(void)
{
}

void CSettingWnd::SetParentWnd(HWND hWnd)
{	

	m_hParentWnd = hWnd;
}

void CSettingWnd::InitWindow()
{	
	//将对话框显示在鼠标右击的位置
	RECT rcDlg = { 0 };
	::GetWindowRect(m_hWnd, &rcDlg);
	
	//对四个方向进行显示处理
	if ((::GetSystemMetrics(SM_CYSCREEN) - m_point.y) < (rcDlg.bottom - rcDlg.top))
	{	
		//左上
		if ((::GetSystemMetrics(SM_CXSCREEN) - m_point.x) < (rcDlg.right - rcDlg.left))
		{
			::SetWindowPos(m_hWnd, HWND_TOPMOST, m_point.x-(m_iSettingbtnWidth)-(rcDlg.right - rcDlg.left), m_point.y-(rcDlg.bottom - rcDlg.top)-m_iSettingbtnHeight, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
		}
		//右上
		else
		{	
			
			::SetWindowPos(m_hWnd, HWND_TOPMOST, m_point.x-m_iSettingbtnWidth, m_point.y-(rcDlg.bottom - rcDlg.top)-m_iSettingbtnHeight, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
		}	
	}
	else
	{	
		//左下
		if ((::GetSystemMetrics(SM_CXSCREEN) - m_point.x) < (rcDlg.right - rcDlg.left))
		{
			::SetWindowPos(m_hWnd, HWND_TOPMOST, m_point.x-(rcDlg.right - rcDlg.left)-m_iSettingbtnWidth, m_point.y, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
		}
		//右下
		else
		{
			::SetWindowPos(m_hWnd, HWND_TOPMOST, m_point.x-(m_iSettingbtnWidth), m_point.y, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
		}
		
	}
	
	::SetForegroundWindow(m_hWnd);
	COptionUI* PLogin = (COptionUI*)(m_PaintManager.FindControl(_T("nav6")));
	PLogin->SetText(m_strLoginText);
}

CDuiString CSettingWnd::GetSkinFolder()
{
	return _T("skin");
}

CDuiString CSettingWnd::GetSkinFile()
{
	return _T("Setting.xml");
}

LPCTSTR CSettingWnd::GetWindowClassName(void) const
{
	return _T("MasterZ_Settingwnd");
}

LRESULT CSettingWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
			if(LOWORD(wParam) ==WA_INACTIVE) 
			{ 
				Close();
			} 
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

void CSettingWnd::Notify(TNotifyUI& msg)
{	
	if (msg.sType == _T("click"))
	{	
		//登录商舟网
		if (msg.pSender->GetName() == _T("nav2"))
		{
			::SendMessage(m_hParentWnd,MSG_TRAY_FUNCTION_MSG,1,0);
			Close();
		}
		//商舟通知
		if (msg.pSender->GetName() == _T("nav3"))
		{
			::SendMessage(m_hParentWnd,MSG_TRAY_FUNCTION_MSG,2,0);
			Close();
		}
		//清除快照
		if (msg.pSender->GetName() == _T("nav4"))
		{
			::SendMessage(m_hParentWnd,MSG_TRAY_FUNCTION_MSG,3,0);
			Close();
		}
		//系统升级
		if (msg.pSender->GetName() == _T("nav5"))
		{
			::SendMessage(m_hParentWnd,MSG_TRAY_FUNCTION_MSG,4,0);
			Close();
		}
		//注销登录
		if(msg.pSender->GetName() == _T("nav6"))
		{
			::SendMessage(m_hParentWnd,MSG_TRAY_FUNCTION_MSG,5,0);
			Close();
		}	
		//设置界面
		if (msg.pSender->GetName() == _T("nav7"))
		{
			::SendMessage(m_hParentWnd,MSG_TRAY_FUNCTION_MSG,6,0);
			Close();
		}
		//安全退出
		if (msg.pSender->GetName() == _T("nav8"))
		{
			::SendMessage(m_hParentWnd,MSG_TRAY_FUNCTION_MSG,7,0);
			Close();
		}
		//问题诊断
		if (msg.pSender->GetName() == _T("nav9"))
		{
			::SendMessage(m_hParentWnd,MSG_TRAY_FUNCTION_MSG,8,0);
			Close();
		}
	}
	__super::Notify(msg);
}

void CSettingWnd::SetPoint(POINT point,int iSettingbtnWidth,int iSettingHeight)
{
	m_point = point;
	m_iSettingbtnWidth = iSettingbtnWidth;
	m_iSettingbtnHeight = iSettingHeight;
}

void CSettingWnd::SetLogintext(const CString& strText)
{	
	m_strLoginText = strText;
}