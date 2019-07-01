#include "stdafx.h"
#include <ShellAPI.h>
#include "Resource.h"
#include "UpdateWnd.h"


CUpdateWnd::CUpdateWnd(void)
{	
}	

CUpdateWnd::~CUpdateWnd(void)
{	
}

CDuiString CUpdateWnd::GetSkinFolder()
{
	return _T("");
}

CDuiString CUpdateWnd::GetSkinFile()
{
	return _T("update.xml");
}

LPCTSTR CUpdateWnd::GetWindowClassName(void) const
{
	return _T("MagicBox_UpdateWnd");
}

void CUpdateWnd::InitWindow()
{	
}

LPCTSTR CUpdateWnd::GetResourceID() const
{
	return MAKEINTRESOURCE(IDR_ZIPRES1);
};
UILIB_RESOURCETYPE CUpdateWnd::GetResourceType() const
{
	return UILIB_ZIPRESOURCE;
};
CControlUI* CUpdateWnd::CreateControl(LPCTSTR pstrClass)
{
	return nullptr;
}

void CUpdateWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("Button_OK"))
		{
			ShellExecute(NULL, _T("Open"), m_sBATPath, NULL, NULL, SW_HIDE);
		}
		else if (msg.pSender->GetName() == _T("Button_Cancel"))
		{
			Close();
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
void CUpdateWnd::SetBATPath(CStdString _sBATPath)
{
	m_sBATPath = _sBATPath;
}
