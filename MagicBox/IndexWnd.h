#pragma once
#include "stdafx.h"
#include "CurlAPI.h"
#include "controls_ex.h"
#include "CommServer.h"
#include "UpdateWnd.h"
#include "duilib/UIlib.h"
using namespace DuiLib;

#ifdef _DEBUG
#   ifdef _UNICODE
#       pragma comment(lib, "DuiLib_ud.lib")
#   else
#       pragma comment(lib, "DuiLib_d.lib")
#   endif
#else
#   ifdef _UNICODE
#       pragma comment(lib, "DuiLib_u.lib")
#   else
#       pragma comment(lib, "DuiLib.lib")
#   endif
#endif


typedef DWORD(*pfnDataHandler)(SOCKET& _ClientSocket, TCHAR *Orgbuf, DWORD dwTotalLen);

class CIndexWnd :public WindowImplBase, public IObserver
{
public:
	CIndexWnd(void);
	~CIndexWnd(void);
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	virtual void InitWindow();
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);

	virtual LPCTSTR GetResourceID() const;
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	void Notify(TNotifyUI& msg);
	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void Update(int _iRate);

private:
	//响应定时器消息
	void OnTimerMsg(WPARAM wParam, LPARAM lParam);
	//重新设定定时器
	void ReSetTimer();
	//工作线程
	static DWORD WINAPI ThreadWorkDL(LPVOID lpParameter);
	void DoDL();
	//刷新UI
	void UI_Redraw();
	void init();
	int GetBinaryDigit(int _iNum, int _iBitIndex);
	bool isExit();

	//socket监听服务
	bool RunServer(pfnDataHandler fn);
	static DWORD GetData(SOCKET& _ClientSocket, TCHAR *Orgbuf, DWORD dwTotalLen);
	static bool SplitCString(const CDuiString & input, const CDuiString & delimiter, std::vector<CDuiString>& results);	//分割字符
	static char* HttpResponse(const char* pC_Reponse);

private:
	enum 
	{
		SKIN_DEFAULT = 0,
		SKIN_APP_XX1,
	};
private:

	//皮肤类型
	int m_iSkinType;
	//下载安装包失败次数计数
	int m_iDLFailCount;
	//程序运行状态
	int m_iUIStatus;
	int m_iDLStatus;

	bool m_bModifiedTimerValue;
	int m_iStepMax;
	int m_iStepIndex;
	int m_iTinyStepIndex;

	//相关控件
	CLabelUI* m_pShow_Start;
	CLabelUI* m_pShow_Exit;
	CControlUI* m_controlLoading;
	CControlUI* m_controlBlack;

	CCommServer m_server;

	CUpdateWnd m_UpdateWnd;

};
