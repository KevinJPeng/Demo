#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CUpdateTipsWnd :
	public WindowImplBase
{
public:
	CUpdateTipsWnd(void);
	~CUpdateTipsWnd(void);

	void SetParentWnd(HWND hWnd);
	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	CButtonUI*m_pclose;
	CTextUI* m_PEdit;
	HWND m_hParentWnd;
public:
	void setTipsVisble(WORD wType = 0);
	void setCloseEnable();

};
