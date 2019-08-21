#include "stdafx.h"
#include "PostMsgWnd.h"

#define ID_TIMER_POP_WINDOW		10
#define ID_TIMER_CLOSE_WINDOW	11
#define ID_TIMER_DISPLAY_DELAY	12
#define ID_TIMER_DOCCOMPLETE    500
#define WIN_WIDTH	320
#define WIN_HEIGHT	215

CPostMsgWnd::CPostMsgWnd(void)
{
	;
}
CPostMsgWnd::~CPostMsgWnd(void)
{
	;
}

void CPostMsgWnd::InitWindow()
{	
	//此处设置对话框的相关属性
	int dwColor = 0;
	m_tPostMessage.strColor.Replace(_T("#"),_T("0xFF"));
	::StrToIntEx(m_tPostMessage.strColor.GetString(), STIF_SUPPORT_HEX, &dwColor);

	CHorizontalLayoutUI* pPushHead = static_cast<CHorizontalLayoutUI *>(m_PaintManager.FindControl(_T("push_head")));
	pPushHead->SetBkColor(dwColor);
	CButtonUI* pBtn_link = static_cast<CButtonUI *>(m_PaintManager.FindControl(_T("btn_link")));
	pBtn_link->SetBkColor(dwColor);
	pBtn_link->SetText(m_tPostMessage.strBtnText);
	CTextUI* pPushTheme = static_cast<CTextUI *>(m_PaintManager.FindControl(_T("push_theme")));
	pPushTheme->SetText(m_tPostMessage.strTitle);

	//加载web页面
	m_pWebBrowserUI = static_cast<CWebBrowserUI *>(m_PaintManager.FindControl(_T("iexxx")));
	m_pWebBrowser = m_pWebBrowserUI->GetWebBrowser2();
	if(m_pWebBrowserUI != NULL ) 
	{
		m_pWebBrowserUI->SetDelayCreate(false);
		CCustomWebEventHandler *pWebHandle = new CCustomWebEventHandler;
		pWebHandle->SetMainHwnd(m_hWnd);

		m_pWebBrowserUI ->SetWebBrowserEventHandler(pWebHandle);
		m_pWebBrowserUI ->Navigate2(_T("about:blank"));
	}
	::SetTimer(m_hWnd,ID_TIMER_DOCCOMPLETE,10,NULL);
}

CDuiString CPostMsgWnd::GetSkinFolder()
{
	return _T("skin");
}

CDuiString CPostMsgWnd::GetSkinFile()
{
	return _T("PostMessage.xml");
}

LPCTSTR CPostMsgWnd::GetWindowClassName(void) const
{
	return _T("MasterZ_postmsgwnd");
}

void CPostMsgWnd::Notify(TNotifyUI& msg)
{	
	if (msg.sType == _T("click"))
	{	
		//点击了查看详情
		if (msg.pSender->GetName() == _T("btn_link"))
		{	
			::ShellExecute(NULL, _T("open"), m_tPostMessage.strDetailUrl, NULL, NULL, SW_NORMAL);
			Close();
			return;
		}
	}
	__super::Notify(msg);
}

void CPostMsgWnd::OnFinalMessage( HWND hWnd )
{	
	//对话框资源释放
	::PostMessage(m_hParentWnd,MSG_SHOW_NEXT_POSTMESSAGE,0,0);

	m_pWebBrowser->Release();
	__super::OnFinalMessage( hWnd );
}

LRESULT CPostMsgWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	if (uMsg == WM_TIMER)
	{			
		switch(wParam)
		{
		//从下至上弹出消息框
		case ID_TIMER_POP_WINDOW:
			{	
				ShowWindow(true);
				int cy=GetSystemMetrics(SM_CYSCREEN);
				int cx=GetSystemMetrics(SM_CXSCREEN);
				RECT rect;
				SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
				int y=rect.bottom-rect.top;
				int x=rect.right-rect.left;
				x=x-WIN_WIDTH;

				if(m_height<=WIN_HEIGHT)
				{
					m_height+= 25;			
					::MoveWindow(m_hWnd,x-10,
						y-m_height,
						WIN_WIDTH,
						WIN_HEIGHT,TRUE);
				}
				else
				{
					::KillTimer(m_hWnd,ID_TIMER_POP_WINDOW);
					//推送消息一直存在不消失，直到手动操作关闭
					if (m_tPostMessage.iShowTime != 0)
					{
						::SetTimer(m_hWnd,ID_TIMER_DISPLAY_DELAY,(m_tPostMessage.iShowTime*1000),NULL);
					}
				}
			}
			break;
		//关闭消息对话框
		case ID_TIMER_CLOSE_WINDOW:
			{
				POINT point = {0};
				RECT ret = {0};
				::GetCursorPos(&point);
				::GetWindowRect(m_hWnd,&ret);

				if((point.x > ret.left&& point.x < ret.right) && (point.y > ret.top&& point.y < ret.bottom)) 
				{
					break;
				}
				else
				{
					::KillTimer(m_hWnd,ID_TIMER_CLOSE_WINDOW);
					Close();
				}
			}
			break;
		//显示时间截止，开始关闭对话框
		case ID_TIMER_DISPLAY_DELAY:
			{
				::KillTimer(m_hWnd,ID_TIMER_DISPLAY_DELAY);
				::SetTimer(m_hWnd,ID_TIMER_CLOSE_WINDOW,20,NULL);
			}
			break;
		//定时检测网页加载情况，加载完毕之后开始弹出消息对话框
		case ID_TIMER_DOCCOMPLETE:
			{
				READYSTATE vb;
				m_pWebBrowser->get_ReadyState(&vb);

				if (vb == READYSTATE_COMPLETE)
				{	
					BSTR strLocalUrl;
					m_pWebBrowser->get_LocationURL(&strLocalUrl);
					if (_tcscmp(strLocalUrl,_T("about:blank")) == 0)
					{	
						m_pWebBrowserUI ->Navigate2(m_tPostMessage.strIntroUrl); 
					}
					else
					{
						::KillTimer(m_hWnd,ID_TIMER_DOCCOMPLETE);
						m_height = 0;
						::SetTimer(m_hWnd,ID_TIMER_POP_WINDOW,20,NULL);
					}
				}
			}
			break;
		}
	}

	if(WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}
void CPostMsgWnd::SetParentWnd(HWND hWnd)
{
	m_hParentWnd = hWnd;
}

void CPostMsgWnd::SetPostInfo(const DELAY_MESSAGE& tPostMessage)
{
	m_tPostMessage = tPostMessage;
}

DWORD CPostMsgWnd::getWorktaskHeight()
{
	RECT rt;  
	SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)&rt,0);  

	DWORD dwTaskHeight =  ::GetSystemMetrics(SM_CYSCREEN)-(rt.bottom - rt.top);
	return dwTaskHeight;
}
