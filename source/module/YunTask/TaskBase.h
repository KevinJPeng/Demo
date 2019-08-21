/******************************************************************
/* 云任务基类，所有云任务从此继承
/*****************************************************************/

#pragma once
#include "..\..\threadmodel\UiHelper.h"

class CTaskBase;
class CTaskMgr;

class CTaskBase
	:public CUiHelper
{
public:
	CTaskBase(void);
	virtual ~CTaskBase();

	// 判断是否发送通告请求任务
	virtual bool IsSendNitify() = 0;
	
	// 设置任务数据
	virtual void SetData(const T_TASK_DATA &tData) = 0;

	// 返回任务数据
	virtual T_TASK_DATA GetData(void) = 0;

	// 初始化对象操作
	virtual bool Init(void) = 0;

	// 检测任务是否可以立即执行
	virtual bool CanExecNow() = 0;

	// 开始执行任务
	virtual DWORD Exec(void) = 0;

	// 停止任务 bDelete用于判断是否delete指针
	virtual void Stop() = 0;

	// 向服务器提交执行结果
	virtual bool SendResultToServer(void) = 0;

	// 通知任务结束的回调函数
	virtual void SetTaskMgr(CTaskMgr *pMgr) = 0;

	// 处理收到的结果数据
	virtual void OnReceive(CString strData) = 0;

	// 获取任务的标识信息
	virtual CString GetTaskInfo(void) = 0;

	// 等待接收线程结束
	virtual void WaitExit() = 0;

	// 判断任务超时 
	virtual bool IsTimeOut() = 0;

	//获取超时标记
	virtual bool GetTimeOutFlag() = 0;
	
	//获取当前任务类型
	virtual int GetCurrTaskType() = 0;

	// 释放内存通告
	virtual void ReleaseTask() = 0;

	// 请求下个云任务
	virtual void RequestNextTask() = 0;

	virtual void WaitForRunningEvent() = 0;

	virtual void SetEvent() = 0;

	virtual BOOL IsRunningSendResult() = 0;

	// 进程操作相关
	bool IsOwnerMCProcess();
	bool ProcessExist(TCHAR *pstrProcName, HANDLE *m_hProcess = NULL);
	bool StartProcess(TCHAR *pstrProcName, TCHAR *pstrPort, HANDLE *phProcess = NULL);
	bool StopProcess(TCHAR *pstrProcName);
	bool StopProcess(HANDLE m_hProcess);

	// 获取一个当前可用的端口号，用于启动主控
	int  GetPort(int nDefault = 28016);

};




