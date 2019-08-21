#ifndef _BCT_INDEXWND_H_
#define _BCT_INDEXWND_H_

#include "webBrowserExUI.h"
#include "ShellAPI.h"
#include "TrayWnd.h"



class CIndexWnd : public WindowImplBase
{
public:
	CIndexWnd(void);
	~CIndexWnd(void);
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	virtual void InitWindow();
	virtual LPCTSTR GetResourceID() const;
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/);
	void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	//处理从JS传过来的数据
	static DWORD WINAPI DoDataFromJSThread(LPVOID lpParameter);
	void DoDataFromJS();

	//工作线程
	static DWORD WINAPI ThreadWorkDL(LPVOID lpParameter);
	void DoDL();

private:
	//托盘操作--ADD,DELETE
	void OperateTray(DWORD dwType);
	//托盘开始闪烁
	bool StartTwinkling();
	//托盘停止闪烁
	void StopTwinkling();

	//打开托盘菜单
	void OnOpearateTray(WPARAM wParam, LPARAM lParam);
	//响应托盘菜单
	void OnTaryMenue(WPARAM wParam, LPARAM lParam);
	//相应服务器消息
	void OnServerMsg(WPARAM wParam, LPARAM lParam);
	//响应定时器消息
	void OnTimerMsg(WPARAM wParam, LPARAM lParam);
	//重新设定定时器
	void ReSetTimer();

	bool SplitString(const string & input, const string & delimiter, std::vector<string >& results);	//分割字符
	bool isExit();

	DWORD GetCurrentActiveWindowsProcessId();
	BOOL HasFocus();

protected:
private:
	bool m_bThreadRunFlag;				//线程退出标志
	CWebBrowserExUI *m_pBrowser;		//浏览器控件
	NOTIFYICONDATA m_NotifyIcon;				//托盘		
	CTrayWnd m_TrayWnd;	
	int m_iDLFailCount;		//下载安装包失败次数计数	
	int m_iDLStatus;			//程序运行状态
	bool m_bUserExit;
	bool m_bTwinkling;		//托盘闪烁
	bool m_bVisible;			//托盘可见
	int m_iTwinklingCount;	//任务栏图标闪烁次数

};



#endif