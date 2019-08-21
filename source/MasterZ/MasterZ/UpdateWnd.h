#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CUpdateWnd :
	public WindowImplBase
{
public:
	CUpdateWnd(void);
	~CUpdateWnd(void);

	void SetParentWnd(HWND hWnd);
	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);

private:
	HWND m_hParentWnd;
	CButtonUI *m_pBtnBkUpdate;
	CProgressUI *m_pProcessUpdate;
	CTextUI*m_PTextTips;

};

