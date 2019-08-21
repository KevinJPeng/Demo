#include "stdafx.h"
#include "ClientSettingWnd.h"
#include <shlobj.h>
#include "..\..\threadmodel\MsgDef.h"


CClientSetWnd::CClientSetWnd(void)
{
}


CClientSetWnd::~CClientSetWnd(void)
{
}

void CClientSetWnd::SetParentWnd(HWND hWnd)
{	

	m_hParentWnd = hWnd;
}

void CClientSetWnd::InitWindow()
{	
	
	m_pOptStart = (COptionUI *)m_PaintManager.FindControl(_T("option_start"));
	m_pOptPlain = (COptionUI *)m_PaintManager.FindControl(_T("option_plain"));
	
	GetSettingInfo(m_tSettingInfo);
	if (m_tSettingInfo.strRebootStart.IsEmpty() || m_tSettingInfo.strRebootStart.Compare(_T("1")) == 0)
	{
		m_pOptStart->Selected(true);
	}
	else
	{
		m_pOptStart->Selected(false);
	}
	if (m_tSettingInfo.strMasterZPlain.IsEmpty() || m_tSettingInfo.strMasterZPlain.Compare(_T("1")) == 0)
	{
		m_pOptPlain->Selected(true);
	}
	else
	{
		m_pOptPlain->Selected(false);
	}
}

CDuiString CClientSetWnd::GetSkinFolder()
{
	return _T("skin");
}

CDuiString CClientSetWnd::GetSkinFile()
{
	return _T("set.xml");
}

LPCTSTR CClientSetWnd::GetWindowClassName(void) const
{
	return _T("Client_SetWnd");
}

LRESULT CClientSetWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//屏蔽对标题栏的双击操作
	if(WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}

void CClientSetWnd::Notify(TNotifyUI& msg)
{	
	if (msg.sType == _T("click"))
	{	
		if (msg.pSender->GetName() == _T("btnOk"))
		{
			if (m_pOptStart->IsSelected())
			{
				m_tSettingInfo.strRebootStart = _T("1");
			}
			else
			{
				m_tSettingInfo.strRebootStart = _T("0");
			}
			if (m_pOptPlain->IsSelected())
			{
				m_tSettingInfo.strMasterZPlain = _T("1");
			}
			else
			{
				m_tSettingInfo.strMasterZPlain = _T("0");
			}

			WriteConfig(m_tSettingInfo);
			Close();
		}

	}
	__super::Notify(msg);
}
