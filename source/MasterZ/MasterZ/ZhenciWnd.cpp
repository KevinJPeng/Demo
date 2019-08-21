#include "stdafx.h"
#include "ZhenCiWnd.h"

#define ID_TIMER_DOCCOMPLETE    1
#define ID_TIMER_SHOWWINDOW		2
#define WIN_WIDTH	312
#define WIN_HEIGHT	205

CZhenCiWnd::CZhenCiWnd()
{
	m_strWebUrl = _T("");
}


CZhenCiWnd::~CZhenCiWnd()
{

}

void CZhenCiWnd::InitWindow()
{
	m_WebUI = static_cast<CWebBrowserUI *>(m_PaintManager.FindControl(_T("webxxx")));
	m_Web = m_WebUI->GetWebBrowser2();
	if (m_WebUI != NULL)
	{
		m_WebUI->SetDelayCreate(false);
		CCustomWebEventHandler *pHandleWeb = new CCustomWebEventHandler;
		pHandleWeb->SetMainHwnd(m_hWnd);

		m_WebUI->SetWebBrowserEventHandler(pHandleWeb);
		m_WebUI->Navigate2(_T("about:blank"));
//		m_WebUI->Navigate2(_T("http:://www.baidu.com"));
/*		Sleep(1000);*/
/*		m_WebUI->Navigate2(m_strWebUrl);*/
	}

	::SetTimer(m_hWnd,ID_TIMER_DOCCOMPLETE,20,NULL);
	
}

DuiLib::CDuiString CZhenCiWnd::GetSkinFolder()
{
	return _T("skin");
}

DuiLib::CDuiString CZhenCiWnd::GetSkinFile()
{
	return _T("zhenci.xml");
}

LPCTSTR CZhenCiWnd::GetWindowClassName() const
{
	return _T("MasterZ_zhenciwnd");
}

LRESULT CZhenCiWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TIMER)
	{
		switch (wParam)
		{
		case ID_TIMER_DOCCOMPLETE:
				{
					READYSTATE rs;
					m_Web->get_ReadyState(&rs);
					if (rs == READYSTATE_COMPLETE)
					{
						BSTR strUrl;
						m_Web->get_LocationURL(&strUrl);
						if (_tcscmp(strUrl, _T("about:blank")) == 0)
						{
							m_WebUI->Navigate2(m_strWebUrl);
						}
						else
						{
							::KillTimer(m_hWnd, ID_TIMER_DOCCOMPLETE);
							::SetTimer(m_hWnd, ID_TIMER_SHOWWINDOW, 20, NULL);
						}
					}
				}
			break;
		case ID_TIMER_SHOWWINDOW:
				{
					//移动位置
					RECT rect;
					SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
					int y = rect.bottom - rect.top;
					int x = rect.right - rect.left;
					x = x - WIN_WIDTH;
					y = y - WIN_HEIGHT;

					::MoveWindow(m_hWnd, x - 4, y - 4, WIN_WIDTH, WIN_HEIGHT, TRUE);
					ShowWindow(true);
					//::SetForegroundWindow(m_hWnd);
					::KillTimer(m_hWnd,ID_TIMER_SHOWWINDOW);
				}
		}
	}	

	if (WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}

void CZhenCiWnd::Notify(TNotifyUI& msg)
{
	//if (msg.sType = _T("click"))
	//{
	//	//点击了查看详情
	//	if (msg.pSender->GetName() == _T("closezhencibtn"))
	//	{
	//		Close();
	//		return;
	//	}
	//}
	__super::Notify(msg);
}
/*
@breif 传递URL进来
@param strInfourl 显示的URL页面
*/
void CZhenCiWnd::SetInfo(const CString& strInfourl)
{
	m_strWebUrl = strInfourl;
}
