#ifndef _CYUNTASKTHREAD_H_
#define _CYUNTASKTHREAD_H_

#include "..\..\threadmodel\ithreadunit.h"
#include "CommClient.h"
#include "TaskBase.h"
#include "UploadFileUseUdp.h"

//

class CYunTaskStreamCtr;

class CTaskThread :
	public IThreadUnit
{
public:
	CTaskThread(void);
	~CTaskThread(void);

	//线程消息处理
	DWORD DispatchMessage(T_Message *pMsg);

	//通知ui自动任务是否完成
	void OnNotifyUI(DWORD flag, DWORD dwMsg = MSG_AUTO_REFRESH_KEYWORD, DWORD dwLparam = NULL);

	//处理与服务端连接发送消息的任务
	void HandleConnServWithUdp(T_Message *pMsg);

	// 处理云任务
	void HandleYunTask(T_Message *pMsg = NULL);

	//提交日志
	void HandleSubmitLog(T_Message *pMsg);

	//组合要发给服务端的数据
	const CString CombineSendData(T_Message *tMsg);

	//将zip文件通过ftp上传
	bool CTaskThread::SumbitZipToServer(CString &strFilePath);

	//读取mcconfig配置文件获取ftp的登录信息
	void GetFtpLoginInfo(void);

	void GetDesCode(TCHAR *code,TCHAR *ip,TCHAR *account,TCHAR *psd,int &port);

	//递归目录
	BOOL RecuscivPath(const CString& strFilePath, CString strExpansion = _T(".sln"));

	//查找所有硬盘下路径
	BOOL SearchAllDrive(CString strExpansion = _T(".sln"));

	//查看路径是否包含要找的文件
	BOOL FindMainFile(const CString& strFilePath);

private:
	static DWORD WINAPI MessageThread(LPVOID lpParameter);  //消息队列线程

	static DWORD WINAPI ConnectServWithUdpThread(LPVOID lpParameter);        //执行与服务端交互的线程

	static DWORD WINAPI SubmitLogThread(LPVOID lpParameter);                 //执行提交日志的线程

	static DWORD WINAPI SubmitBackLog(LPVOID lpParameter);					 //提交备份日志线程

	static DWORD WINAPI SubmitLogAndQuickPhotoThread(LPVOID lpParameter);    //执行提交日志及快照的线程

	static DWORD WINAPI SubmitLodAndErrorCodeThread(LPVOID lpParameter);	 //提交日志及ErrorCode	

	static DWORD WINAPI SubmitSpecilePath(LPVOID lpParameter);		//提交指定路径文件

	static DWORD WINAPI SubmitDirDetail(LPVOID lpParameter);     //提交目录的详细信息；

private:
	bool    m_bFirstTask;             //是否是第一次收到云任务消息不是第一次就丢弃

	HANDLE  m_hThread1;               //提交日志线程的句柄
	HANDLE  m_hThread2;               //提交日志及快照线程句柄
	HANDLE  m_hThread3;				  //提交日志及ErrorCode
	HANDLE  m_hThread4;				  //提交备份日志
	HANDLE  m_hThread5;				  //提交目录详细信息
	HANDLE  m_hThread6;				  //提交指定目录
	DWORD   m_dwThreadId;             //线程id   
	DWORD   m_dwStartTime;            //记录时间判断同一用户请求的时间间隔
	int     m_nVersionId;             //版本号

	CString m_strUseName;              //用户名
	CString m_strUseNameBakeup;        //用户名备份
	CString m_strSubmitLogUseName;     //提交日志的用户名信息
	CString m_strSendData;             //发送给服务端的数据
	CString m_strReciveData;           //接收服务端的数据

	T_CONN_INFO      m_ftpLoginInfo;   //ftp登录信息的结构体
	vector <CString> m_vstrUserName;

	CYunTaskStreamCtr* m_pTaskCtr;
	CUploadFileUseUdp* m_pUploadFile;

	BOOL m_bCatchDir;				//是否要抓取需要的目录结构
	vector <CStdString> m_vFindFileName;	//要查找的文件名称

};

#endif // _CYUNTASKTHREAD_H_