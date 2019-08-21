#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CPostMsgWnd :
	public WindowImplBase
{
public:
	CPostMsgWnd(void);
	~CPostMsgWnd(void);

	void SetParentWnd(HWND hWnd);
	void SetPostInfo(const DELAY_MESSAGE& tPostMessage);
	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnFinalMessage( HWND hWnd );
private:
	HWND m_hParentWnd;
private:
	DWORD getWorktaskHeight();			//获取任务栏的高度，暂时不区分左右上，只考虑在弹框在下面的情况	
	IWebBrowser2* m_pWebBrowser;
	CWebBrowserUI  *m_pWebBrowserUI;
	int m_height;
	DELAY_MESSAGE m_tPostMessage;
};