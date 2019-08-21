#pragma once
#include "..\..\threadmodel\ithreadunit.h"
#include "CommClient.h"
#include "ScheduleBase.h"
#include "UploadFileUseUdp.h"

//

class CAutoGetKeywordPages;

class CScheduleThread :
	public IThreadUnit
{
public:
	CScheduleThread(void);
	~CScheduleThread(void);

	//线程消息处理
	DWORD DispatchMessage(T_Message *pMsg);

	//通知ui自动任务是否完成
	void OnNotifyUI(DWORD flag, DWORD dwMsg = MSG_AUTO_REFRESH_KEYWORD, DWORD dwLparam = NULL);

	//处理自动刷新任务
	void HandleRefreshKeyword(T_Message *pMsg = NULL);

	//处理与服务端连接发送消息的任务
	void HandleConnServWithUdp(T_Message *pMsg);

	//提交日志
	void HandleSubmitLog(T_Message *pMsg);

	//获得mac地址
	CString GetClientMacAddr(void);

	//组合要发给服务端的数据
	const CString CombineSendData(T_Message *tMsg);

	//将zip文件通过ftp上传
	bool CScheduleThread::SumbitZipToServer(CString &strFilePath);

	//读取mcconfig配置文件获取ftp的登录信息
	void GetFtpLoginInfo(void);

	void GetDesCode(TCHAR *code,TCHAR *ip,TCHAR *account,TCHAR *psd,int &port);


private:
	static DWORD WINAPI MessageThread(LPVOID lpParameter);  //消息队列线程

	static DWORD WINAPI StartExecThread(LPVOID lpParameter);  //执行刷新任务线程

	static DWORD WINAPI ConnectServWithUdpThread(LPVOID lpParameter); //执行与服务端交互的线程

	static DWORD WINAPI SubmitLogThread(LPVOID lpParameter);   //执行提交日志的线程

	static DWORD WINAPI SubmitLogAndQuickPhotoThread(LPVOID lpParameter);   //执行提交日志及快照的线程


private:
	DWORD m_dwThreadId;        //线程id   
	CString m_strUseName;  //用户名
	CString m_strUseNameBakeup;   //用户名备份
	DWORD m_dwStartTime;      //记录时间判断同一用户请求的时间间隔
	int m_nVersionId;   //版本号
	int m_nRefreshFlag; //刷排名的标记，0表示普通刷排名，1表示点击立即刷新按钮会先清掉排名再刷
	CString m_strSubmitLogUseName;  //提交日志的用户名信息

	CString m_strSendData;  //发送给服务端的数据
	CString m_strReciveData;  //接收服务端的数据
	T_CONN_INFO m_ftpLoginInfo;   //ftp登录信息的结构体
	vector <CString> m_vstrUserName;

	HANDLE m_hThread1;  //提交日志线程的句柄
	HANDLE m_hThread2;  //提交日志及快照线程句柄

	CAutoGetKeywordPages *m_pAutoGetKeywordPages;
	CUploadFileUseUdp *m_pUploadFile;
};

