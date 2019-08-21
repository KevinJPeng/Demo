#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CTrayWnd :
	public WindowImplBase
{
public:
	CTrayWnd(void);
	~CTrayWnd(void);

	void SetParentWnd(HWND hWnd);
	void SetPoint(POINT point);
	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);
	void SetLoginText(const CString& strLoginText);

private:
	HWND m_hParentWnd;
	POINT m_point;
private:
	//获取程序的安装路径
	TCHAR* GetProgPath(void);
	CString m_strLoginText;


};

