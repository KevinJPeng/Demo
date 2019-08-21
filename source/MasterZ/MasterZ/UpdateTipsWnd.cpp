#include "stdafx.h"
#include "UpdateTipsWnd.h"
CUpdateTipsWnd::CUpdateTipsWnd(void)
{
	;
}
	
CUpdateTipsWnd::~CUpdateTipsWnd(void)
{
	;
}

void CUpdateTipsWnd::InitWindow()
{	
	m_pclose = (CButtonUI*)m_PaintManager.FindControl(_T("btn_close"));
	m_PEdit = (CTextUI*)m_PaintManager.FindControl(_T("text_tips"));
	m_PEdit->SetVisible(false);
}
CDuiString CUpdateTipsWnd::GetSkinFolder()
{
	 return _T("skin");
}

CDuiString CUpdateTipsWnd::GetSkinFile()
{
	return _T("rebootclient.xml");
}

LPCTSTR CUpdateTipsWnd::GetWindowClassName(void) const
{
	return _T("MasterZ_messagetipswnd");
}

void CUpdateTipsWnd::Notify(TNotifyUI& msg)
{	
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("btn_close"))
		{	
			::SendMessage(m_hParentWnd, MSG_CANCEL_UPDATE, NULL, NULL);
			Close();
		}
		if (msg.pSender->GetName() == _T("reload"))
		{	
			m_pclose->SetEnabled(false);
			::SendMessage(m_hParentWnd, MSG_PRODUCT_UPDATE, 101, NULL);
		}
	}
	__super::Notify(msg);
}

LRESULT CUpdateTipsWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//屏蔽对标题栏的双击操作
	if(WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}
void CUpdateTipsWnd::SetParentWnd(HWND hWnd)
{
	m_hParentWnd = hWnd;
}

void CUpdateTipsWnd::setTipsVisble(WORD wType)
{	
	if (wType == WPARAM_IECTRL_COPYERROR)
	{
		m_PEdit->SetText(_T("请先关闭舟大师浏览器页面！"));
	}
	if (wType == WPARAM_NPCTRL_COPYERROR)
	{
		m_PEdit->SetText(_T("请先关闭当前浏览器!"));
	}
	m_PEdit->SetVisible(true);
}
void CUpdateTipsWnd::setCloseEnable()
{
	m_pclose->SetEnabled(true);
}