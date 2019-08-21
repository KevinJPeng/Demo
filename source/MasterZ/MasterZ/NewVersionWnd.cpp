#include "stdafx.h"
#include "NewVersionWnd.h"

CNewVersionWnd::CNewVersionWnd(void)
{
	;
}
CNewVersionWnd::~CNewVersionWnd(void)
{
	;
}

void CNewVersionWnd::InitWindow()
{
	m_PEdit = (CEditUI *)m_PaintManager.FindControl(_T("text_tips"));
}
CDuiString CNewVersionWnd::GetSkinFolder()
{
	 return _T("skin");
}

CDuiString CNewVersionWnd::GetSkinFile()
{
	return _T("NewVersion.xml");
}

LPCTSTR CNewVersionWnd::GetWindowClassName(void) const
{
	return _T("MasterZ_messagewnd");
}

void CNewVersionWnd::Notify(TNotifyUI& msg)
{	
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("btn_close") || msg.pSender->GetName() == _T("btn_cancel"))
		{
			Close();
		}
	}
	__super::Notify(msg);
}

LRESULT CNewVersionWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//屏蔽对标题栏的双击操作
	if(WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}
