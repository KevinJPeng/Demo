#include "stdafx.h"
#include "UpdateErrorWnd.h"
CUpdateErrorWnd::CUpdateErrorWnd(void)
{
	;
}
CUpdateErrorWnd::~CUpdateErrorWnd(void)
{
	;
}

void CUpdateErrorWnd::InitWindow()
{
	m_PEdit = (CEditUI *)m_PaintManager.FindControl(_T("text_tips"));
}
CDuiString CUpdateErrorWnd::GetSkinFolder()
{
	 return _T("skin");
}

CDuiString CUpdateErrorWnd::GetSkinFile()
{
	return _T("updatefail.xml");
}

LPCTSTR CUpdateErrorWnd::GetWindowClassName(void) const
{
	return _T("MasterZ_messageErrorwnd");
}

void CUpdateErrorWnd::Notify(TNotifyUI& msg)
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

LRESULT CUpdateErrorWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//屏蔽对标题栏的双击操作
	if(WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}
void  CUpdateErrorWnd::setErrorText()
{
	CTextUI* ptext = (CTextUI*)m_PaintManager.FindControl(_T("text_tips"));
	ptext->SetText(_T("服务器正忙，请稍后再试..."));
}

void CUpdateErrorWnd::SetPowerErrorText()
{
	CTextUI* ptext = (CTextUI*)m_PaintManager.FindControl(_T("text_tips"));
	ptext->SetText(_T("升级权限不足，请以管理员身份运行客户端！"));
}
