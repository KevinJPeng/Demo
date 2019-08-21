#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CNewVersionWnd :
	public WindowImplBase
{
public:
	CNewVersionWnd(void);
	~CNewVersionWnd(void);

	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	CEditUI* m_PEdit;

};

