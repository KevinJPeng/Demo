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


	//�����JS������������
	static DWORD WINAPI DoDataFromJSThread(LPVOID lpParameter);
	void DoDataFromJS();

	//�����߳�
	static DWORD WINAPI ThreadWorkDL(LPVOID lpParameter);
	void DoDL();

private:
	//���̲���--ADD,DELETE
	void OperateTray(DWORD dwType);
	//���̿�ʼ��˸
	bool StartTwinkling();
	//����ֹͣ��˸
	void StopTwinkling();

	//�����̲˵�
	void OnOpearateTray(WPARAM wParam, LPARAM lParam);
	//��Ӧ���̲˵�
	void OnTaryMenue(WPARAM wParam, LPARAM lParam);
	//��Ӧ��������Ϣ
	void OnServerMsg(WPARAM wParam, LPARAM lParam);
	//��Ӧ��ʱ����Ϣ
	void OnTimerMsg(WPARAM wParam, LPARAM lParam);
	//�����趨��ʱ��
	void ReSetTimer();

	bool SplitString(const string & input, const string & delimiter, std::vector<string >& results);	//�ָ��ַ�
	bool isExit();

	DWORD GetCurrentActiveWindowsProcessId();
	BOOL HasFocus();

protected:
private:
	bool m_bThreadRunFlag;				//�߳��˳���־
	CWebBrowserExUI *m_pBrowser;		//������ؼ�
	NOTIFYICONDATA m_NotifyIcon;				//����		
	CTrayWnd m_TrayWnd;	
	int m_iDLFailCount;		//���ذ�װ��ʧ�ܴ�������	
	int m_iDLStatus;			//��������״̬
	bool m_bUserExit;
	bool m_bTwinkling;		//������˸
	bool m_bVisible;			//���̿ɼ�
	int m_iTwinklingCount;	//������ͼ����˸����

};



#endif