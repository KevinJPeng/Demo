#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CSafeExitWnd :
	public WindowImplBase
{
public:
	CSafeExitWnd(void);
	~CSafeExitWnd(void);

	void SetParentWnd(HWND hWnd);
	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	HWND m_hParentWnd;
};