#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CUpdateErrorWnd :
	public WindowImplBase
{
public:
	CUpdateErrorWnd(void);
	~CUpdateErrorWnd(void);

	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	CEditUI* m_PEdit;
public:
	void setErrorText();
	void SetPowerErrorText();
};
