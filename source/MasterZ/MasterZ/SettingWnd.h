#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CSettingWnd :
	public WindowImplBase
{
public:
	CSettingWnd(void);
	~CSettingWnd(void);

	void SetParentWnd(HWND hWnd);
	void SetPoint(POINT point,int iSettingbtnWidth,int iSettingHeight);
	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);
	void SetLogintext(const CString& strText);

private:
	HWND m_hParentWnd;
	POINT m_point;
	int m_iSettingbtnWidth;
	int m_iSettingbtnHeight;
	CString m_strLoginText;
};