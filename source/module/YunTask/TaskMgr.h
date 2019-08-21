/********************************************************
* 文件说明：任务列表管理,任务的创建,压入,执行,与释放
* 时间    ：2015/8/14
* 作者    ：周林
********************************************************/

#ifndef _YUNTASKMGR_H_
#define _YUNTASKMGR_H_

#include "..\..\common\Lock.h"
#include "TaskThread.h"
#include "YunTaskStreamCtr.h"
#include "TaskFactory.h"
#include "TaskInfo.h"


class CTaskMgr
{
public:
	CTaskMgr(CTaskThread *pTaskThread, CYunTaskStreamCtr* pTaskStreamCtr);
	~CTaskMgr();

	// 初始化并发运行的任务最大数
	void Init(int iMaxConncurrencyTask = 2);

	// 判断某个类型的任务是否在任务列表中存在
	bool IsInTaskList(int iType);

	// 向缓存列表中压入一个任务，根据任务数据自动创建任务对象
	void PushTask(T_TASK_DATA tData);

	// 查看任务是否已在列表中
	bool FindTaskInTaskList(const T_TASK_DATA &tData);

	// 轮循任务列表，控制执行
	void PollingTask(void);

	// 清空任务列表
	void ClearTaskList(void);

	// 任务执行完成
	void TaskFinished(CTaskBase *pTask);

	// 强制终止当前任务，不提交执行结果
	//void TerminateTask(CTaskBase *pTask);
	// 强制终止当前任务，提交执行结果
	void TerminateTaskAndSendResult(CTaskBase *pTask);

	// 返回当前的任务总数
	int GetCachingTaskCount(void);
	
	// 返回正在执行的任务总数
	int GetExecingTaskCount(void);

	// 停止所有任务,包括正在执行的和尚未执行的
	void TerminateAllTasks(void);

	// 接收轮循任务通告
	void OnNotifyRollingTask();

	// 设置轮循线程标志
	void SetPollingFlag(bool bFlag);

	// 等待轮循线程结束
	void WaitEnd();

	// 释放TASK内存通告
	void NotifyReleaseTask(CTaskBase* pTask);

	// 清除操作
	void Release();

	// 压入结束任务到任务完成列表等待删除
	void PushStopTask(CTaskBase* pTask);

	// 检测运行任务超时
	void CheckOutTimeTask();

	//用户输入行为检测
	void CheckUserInput();

private:
	static DWORD WINAPI ThreadProcPollingTask(LPVOID lpParameter);
	static DWORD WINAPI ThreadProcDeleteTask(LPVOID lpParameter);

private:
	bool m_bPollingThreadStop;

	//允许同时执行的最大任务数
	int                m_iMaxConcurrencyTask; 
	//轮循任务线程句柄
	HANDLE             m_hThreadTaskPolling; 
	CLock*             m_pLock;
	CLock*             m_pLockDelete;
	CTaskThread*       m_pTaskThread;
	CYunTaskStreamCtr* m_pTaskStreamCtr;

	// 缓存的任务
	std::vector<pTaskInfo>  m_pCacheList;
	// 正在执行的任务
	std::vector<pTaskInfo>  m_pvRunList;
	// 缓存执行完的任务 在发送通告之后释放内存
	std::vector<pTaskInfo>  m_pFinishTaskList;
	// 释放列表
	std::vector<CTaskBase*> m_pTaskList;

};

#endif //_YUNTASKMGR_H_
