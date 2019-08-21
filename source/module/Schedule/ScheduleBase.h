///////////////////////////////////////////////////////////////////
//自动任务基类，所有自动任务（包括定时任务）从此继承
//////////////////////////////////////////////////////////////////

#pragma once
#include "..\..\threadmodel\UiHelper.h"

class CScheduleBase;
class CScheduleMgr;

class CScheduleBase
	:public CUiHelper
{
public:
	CScheduleBase(void);
	~CScheduleBase(void);
	
	//设置任务数据
	virtual void SetData(const T_TASK_DATA &tData) = 0;

	//返回任务数据
	virtual T_TASK_DATA GetData(void) = 0;

	//初始化对象操作
	virtual bool Init(void) = 0;

	//检测任务是否可以立即执行
	virtual bool CanExecNow(DWORD dwEnterTickCount) = 0;

	//开始执行任务
	virtual DWORD Exec(void) = 0;

	//停止任务
	virtual void Stop(void) = 0;

	//向服务器提交执行结果
	virtual bool SendResultToServer(void) = 0;

	//通知任务结束的回调函数
	virtual void SetTaskMgr(CScheduleMgr *pMgr) = 0;

	//处理收到的结果数据
	virtual void OnReceive(CString strData) = 0;

	//获取任务的标识信息
	virtual CString GetTaskInfo(void) = 0;

	//进程操作相关
	bool ProcessExist(TCHAR *pstrProcName, HANDLE *phProcess = NULL);
	bool StartProcess(TCHAR *pstrProcName, TCHAR *pstrPort, HANDLE *phProcess = NULL);
	bool StopProcess(TCHAR *pstrProcName);
	bool StopProcess(HANDLE hProcess);

	//获取一个当前可用的端口号，用于启动主控
	int  GetPort(int nDefault = 28016);

	//获取请求自动任务的地址
	CString GetRequestURL(void);

	//获取提交自动任务结果的地址
	CString GetPostDataURL(void);
};




