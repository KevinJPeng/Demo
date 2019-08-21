#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CWarningWnd :
	public WindowImplBase
{
public:
	CWarningWnd(void);
	~CWarningWnd(void);

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
private:
	//获取程序的安装路径
	TCHAR* GetProgPath(void);


};

