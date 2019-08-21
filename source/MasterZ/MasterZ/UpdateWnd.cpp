#include "StdAfx.h"
#include "UpdateWnd.h"
#include "controls_ex.h"
#include "..\..\threadmodel\MsgDef.h"

const TCHAR* const kCancelButtonControlName = _T("closebtn");
const TCHAR* const kUpdateProgressControlName = _T("Progress_update");


CUpdateWnd::CUpdateWnd(void)
{
}


CUpdateWnd::~CUpdateWnd(void)
{
}

void CUpdateWnd::SetParentWnd(HWND hWnd)
{
	m_hParentWnd = hWnd;
}

void CUpdateWnd::InitWindow()
{
	//获取界面控件指针
	m_pProcessUpdate = (CProgressUI *)m_PaintManager.FindControl(kUpdateProgressControlName);
	m_PTextTips = (CTextUI*)m_PaintManager.FindControl(_T("update_tips"));
}

CControlUI* CUpdateWnd::CreateControl(LPCTSTR pstrClass)
{
	if (_tcscmp(pstrClass,_T("ButtonGif")) == 0)
	{
		return  new CButtonGifUI;
	}
}
CDuiString CUpdateWnd::GetSkinFolder()
{
	return _T("skin");
}

CDuiString CUpdateWnd::GetSkinFile()
{
	return _T("update.xml");
}

LPCTSTR CUpdateWnd::GetWindowClassName(void) const
{
	return _T("MasterZ_updatewnd");
}

LRESULT CUpdateWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case MSG_PRODUCT_UPDATE:
		{	
			m_PTextTips->SetText(_T("发现新版本，正在努力升级中..."));
			int nPos = (int)lParam;
			m_pProcessUpdate->SetValue(nPos);

			CDuiString strText = _T("");
			strText.Format(_T("%d%%"), nPos);
			m_pProcessUpdate->SetText(strText);
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

void CUpdateWnd::Notify(TNotifyUI& msg)
{	
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("closebtn"))
		{
			::SendMessage(m_hParentWnd, MSG_CANCEL_UPDATE, NULL, NULL);
		}
		//后台升级
		if (msg.pSender->GetName() == _T("btn_min"))
		{
			Close();
		}
	} 
	__super::Notify(msg);
}