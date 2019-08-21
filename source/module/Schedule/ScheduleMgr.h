#pragma once

#include "..\..\common\Lock.h"
#include "ScheduleBase.h"
#include "ScheduleThread.h"
#include "AutoGetKeywordPages.h"
#include <map>

class CScheduleMgr
{
public:
	CScheduleMgr(CScheduleThread *pScheduleThread, CAutoGetKeywordPages *pAutoGetKeywordPages);
	~CScheduleMgr(void);

	void Init(int iMaxConncurrencyTask = 2);

	//向缓存列表中压入一个任务，根据任务数据自动创建任务对象
	void PushTask(T_TASK_DATA tData);

	//查看任务是否已在列表中
	bool FindTaskInTaskList(const T_TASK_DATA &tData);

	//轮循任务列表，控制执行
	void PollingTask(void);

	//清空任务列表
	void ClearTaskList(void);

	//任务执行完成
	void TaskFinished(CScheduleBase *pTask);

	//返回当前的任务总数
	int GetCachingTaskCount(void);
	
	//返回正在执行的任务总数
	int GetExecingTaskCount(void);

	//停止所有任务,包括正在执行的和尚未执行的
	void TerminateAllTasks(void);

private:
	CLock *m_pLock;
	int m_iMaxConcurrencyTask;  //允许同时执行的最大任务数
	CScheduleThread *m_pScheduleThread;
	CAutoGetKeywordPages *m_pAutoGetKeywordPages;

	//缓存的任务 key为任务指针，value为进入缓存的tickcount时间
	std::map<CScheduleBase *, DWORD> m_pmapCacheList;

	//正在执行的任务
	std::vector<CScheduleBase *> m_pvRunList;

};

