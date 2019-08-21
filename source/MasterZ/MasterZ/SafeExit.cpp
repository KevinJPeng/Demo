#include "stdafx.h"
#include "SafeExit.h"

CSafeExitWnd::CSafeExitWnd(void)
{
	;
}
CSafeExitWnd::~CSafeExitWnd(void)
{
	;
}

void CSafeExitWnd::InitWindow()
{	
	
}

CDuiString CSafeExitWnd::GetSkinFolder()
{
	return _T("skin");
}

CDuiString CSafeExitWnd::GetSkinFile()
{
	return _T("exit.xml");
}

LPCTSTR CSafeExitWnd::GetWindowClassName(void) const
{
	return _T("MasterZ_exitwnd");
}

void CSafeExitWnd::Notify(TNotifyUI& msg)
{	

	if (msg.sType == _T("click"))
	{	
		if (msg.pSender->GetName() == _T("btn_exit"))
		{

			Close();
			::PostMessage(m_hParentWnd,MSG_SALF_EXIT,0,0);	
		}
		if (msg.pSender->GetName() == _T("btn_back"))
		{
			Close();
		}
	}
	
	__super::Notify(msg);
}

LRESULT CSafeExitWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	
	if(WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}

void CSafeExitWnd::SetParentWnd(HWND hWnd)
{
	m_hParentWnd = hWnd;
}

