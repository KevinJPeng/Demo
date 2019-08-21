/*******************************************************************
* 文件说明：普通云任务处理类（包括刷新商铺流量,信息刷新,自动推广等）
* 时间    ：2015/8/21
* 作者    ：周林
********************************************************************/

#ifndef _CTASKGENERAL_H_
#define _CTASKGENERAL_H_

#include "CommClient.h"
#include "TaskBase.h"
#include "YunTaskStreamCtr.h"


class CTaskGeneral : public CTaskBase
{
public:
	CTaskGeneral(CTaskThread *pTaskThread, CYunTaskStreamCtr *pYunTaskStreamCtr);
	~CTaskGeneral();

	// 判断是否需要发送通告请求任务
	virtual bool IsSendNitify();

	// 设置任务数据
	virtual void SetData(const T_TASK_DATA &tData);

	// 返回任务数据
	virtual T_TASK_DATA GetData(void);

	// 初始化对象操作
	virtual bool Init(void);

	// 检测任务是否可以立即执行
	virtual bool CanExecNow();

	// 执行任务
	virtual DWORD Exec(void);

	// 停止任务
	virtual void Stop();

	// 向服务器提交执行结果
	virtual bool SendResultToServer(void);

	// 通知任务结束的回调函数
	virtual void SetTaskMgr(CTaskMgr *pMgr);

	// 处理收到的结果数据
	virtual void OnReceive(CString strData);

	// 获取任务的标识信息
	virtual CString GetTaskInfo(void);

	// 等待接收线程结束
	virtual void WaitExit();

	// 释放内存通告
	virtual void ReleaseTask();

	// 判断任务超时 
	virtual bool IsTimeOut();

	//获取超时标记
	virtual bool GetTimeOutFlag();

	//获取当前任务类型
	virtual int GetCurrTaskType();

	// 请求下个云任务
	virtual void RequestNextTask();

	virtual void WaitForRunningEvent();

	virtual void SetEvent();

	virtual BOOL IsRunningSendResult();
public:
	void GetHttpDesCode(TCHAR *code, TCHAR *pHome, TCHAR *pFileUpload, TCHAR *account, TCHAR *psd);

	// 读取mcconfig配置文件获取http的登录信息
	void GetHttpLoginInfo(const CString &strIniPath);

	// 缓存主控发送过来的结果
	void CacheData(CString strData);

	// 组合所有缓存数据
	CString GetAllCacheDate();

private:
	//保存数据到本地（将主控返回的数据存到本地文件，当数据上传给上层后在删除本地文件）
	bool SaveResultDataToFile(CString _strData);
	bool GetResultDataFromFile(CString& _strData, T_TASK_DATA& _tTaskData, CString& _strLocalAccount);
	bool SendLocalResultToServer();
	bool DelResultDataFile();
	bool WriteString(CStdioFile& file, CString _str);
	BOOL ReadStringToUnicode(CString &str);
	int CharToUnicode(char *pchIn, CString *pstrOut);
private:
	//开始执行时间(用于判断任务是否超时)
	DWORD                m_dwExecTime;          
	// 当前任务数据
	T_TASK_DATA          m_tTaskData;
	// 执行任务的进程句柄
	HANDLE               m_hProcess;                         
	// http登录信息的结构体
	T_LOGIN_INFO         m_httpLoginInfo;               

	CTaskMgr*            m_pMgr;
	CCommClient          m_comm;

	CTaskThread*         m_pTaskThread;
	CYunTaskStreamCtr*   m_pYunTaskStreamCtr;
	// 缓存主控发送过来的数据
	std::vector<CString> m_vecCacheData;

	CString m_strLocalAccount;

	CInternetHttp        g_http1;
	CInternetHttp		 g_http2;
	HANDLE				  m_hRunning;
	BOOL				m_bIsTaskTimeOut;

	//任务是否正常完成
	BOOL				m_bIsTaskComplete;

	//主控是否被正常终止
	bool m_bStopWebc;     

	//控制打印主控已经存在日志,只打印一次,避免循环打印日志
	bool m_bIsPrintMCLog;

	CLock m_lock; //锁
};

#endif // _CTASKGENERAL_H_