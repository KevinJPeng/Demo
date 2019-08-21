#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CClientSetWnd :
	public WindowImplBase
{
public:
	CClientSetWnd(void);
	~CClientSetWnd(void);

	void SetParentWnd(HWND hWnd);
	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);

private:
	HWND m_hParentWnd;
	POINT m_point;
	int m_iSettingbtnWidth;
	int m_iSettingbtnHeight;
private:
	tclientsetting m_tSettingInfo;
	COptionUI *m_pOptStart;
	COptionUI *m_pOptPlain;
};