#include "StdAfx.h"
#include "WarningWnd.h"
#include <shlobj.h>
#include "..\..\threadmodel\MsgDef.h"

const TCHAR* const kCancelButtonControlName = _T("closebtn");
const TCHAR* const kUpdateProgressControlName = _T("Progress_update");


CWarningWnd::CWarningWnd(void)
{
}


CWarningWnd::~CWarningWnd(void)
{
}

void CWarningWnd::SetParentWnd(HWND hWnd)
{	

	m_hParentWnd = hWnd;
}

void CWarningWnd::InitWindow()
{	
	//将对话框显示在鼠标右击的位置
}

CDuiString CWarningWnd::GetSkinFolder()
{
	return _T("skin");
}

CDuiString CWarningWnd::GetSkinFile()
{
	return _T("warning.xml");
}

LPCTSTR CWarningWnd::GetWindowClassName(void) const
{
	return _T("MasterZ_Warningwnd");
}

LRESULT CWarningWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	default:
		break;
	}

	//屏蔽对标题栏的双击操作
	if(WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}

void CWarningWnd::Notify(TNotifyUI& msg)
{	
	if (msg.sType == _T("click"))
	{	
		if (msg.pSender->GetName() == _T("button_ok"))
		{
			Close();
		}
	}
	__super::Notify(msg);
}


TCHAR* CWarningWnd::GetProgPath(void)
{

	static TCHAR s_szProgPath[MAX_PATH] = {0};
	SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 1, s_szProgPath);
	return s_szProgPath;

}