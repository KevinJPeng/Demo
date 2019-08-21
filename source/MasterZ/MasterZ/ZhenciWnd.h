#pragma once
#include "..\..\duilib\Utils\WinImplBase.h"

class CZhenCiWnd :
	public WindowImplBase
{
public:
	CZhenCiWnd();
	~CZhenCiWnd();

	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Notify(TNotifyUI& msg);
	//传递URL进来
	void SetInfo(const CString& strInfourl);
	
private:
	IWebBrowser2* m_Web;
	CWebBrowserUI* m_WebUI;

	CString m_strWebUrl;


};

