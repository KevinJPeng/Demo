#pragma once
#include "..\..\duilib\utils\winimplbase.h"

class CLoginWnd :
	public WindowImplBase
{
public:
	CLoginWnd(void);
	~CLoginWnd(void);

	void SetParentWnd(HWND hWnd);
	virtual void InitWindow();

	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Notify(TNotifyUI& msg);

private:
	HWND m_hParentWnd;
	COptionUI *m_pOptRember;
	COptionUI *m_pOptAutoLogin;
	CEditUI* m_PUserName;
	CEditUI* m_PPassWord;
	CButtonUI* m_PButtonLogin;
	CButtonUI* m_PButtonLoginCancel;
	CLabelUI* m_PLableError;	
private:
	CString m_strURL;
	CString m_strUserName;
	
	//从配置文件中读取用户是否保存了密码，保存了
	void InitialUserInfo();
	//用户点击登录按钮，则需要将相关信息发送到主界面进行登录操作
	void SendLoginMessageToMianUI();
};

