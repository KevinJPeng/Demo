#include "stdafx.h"
#include "TaskMgr.h"


CTaskMgr::CTaskMgr(CTaskThread *pTaskThread, CYunTaskStreamCtr* pTaskStreamCtr)
{
	m_iMaxConcurrencyTask = 1;
	m_pTaskThread         = pTaskThread;
	m_pTaskStreamCtr      = pTaskStreamCtr;
	m_pLock               = new CLock;
	m_pLockDelete         = new CLock;
	m_bPollingThreadStop  = false;
	m_hThreadTaskPolling  = NULL;

	m_pvRunList.clear();
	m_pFinishTaskList.clear();
	m_pCacheList.clear();
	m_pTaskList.clear();
}


CTaskMgr::~CTaskMgr(void)
{
	ClearTaskList();
	if (m_pLock != NULL)
	{
		delete m_pLock;
		m_pLock = NULL;
	}
	if (m_pLockDelete != NULL)
	{
		delete m_pLockDelete;
		m_pLockDelete = NULL;
	}
}

void CTaskMgr::Init(int iMaxConncurrencyTask)
{
	m_iMaxConcurrencyTask = iMaxConncurrencyTask;
}

//向缓存列表中压入一个任务，根据任务数据自动创建任务对象
void CTaskMgr::PushTask(T_TASK_DATA tData)
{
	m_pLock->Lock();
	if (!FindTaskInTaskList(tData))
	{
		pTaskInfo pInfoTask = CTastFactory::Create(g_TypeMap[tData.strType], m_pTaskThread, m_pTaskStreamCtr);
		if (pInfoTask->_pTask != NULL)
		{
			pInfoTask->_pTask->Init();
			pInfoTask->_pTask->SetData(tData);
			pInfoTask->_pTask->SetTaskMgr(this);
			m_pCacheList.push_back(pInfoTask);
		}
		std::sort(m_pCacheList.begin(), m_pCacheList.end(), CompareTaskInfo);
	}
	m_pLock->Unlock();
}

// 判断任务列表中是否存在相同类型的任务
bool CTaskMgr::IsInTaskList(int iType)
{
	std::vector<pTaskInfo>::iterator itRunList = m_pvRunList.begin();
	for (; itRunList != m_pvRunList.end(); ++itRunList)
	{
		if ((*itRunList)->_iType == iType)
			return true;
	}

	std::vector<pTaskInfo>::iterator itCacheList = m_pCacheList.begin();
	for (; itCacheList != m_pCacheList.end(); ++itCacheList)
	{
		if ((*itCacheList)->_iType == iType)
			return true;
	}

	return false;
}

//查看任务是否已在列表中
bool CTaskMgr::FindTaskInTaskList(const T_TASK_DATA &tData)
{
	std::vector<pTaskInfo>::iterator itRunList = m_pvRunList.begin();
	for (; itRunList != m_pvRunList.end(); ++itRunList)
	{
		if (tData == (*itRunList)->_pTask->GetData())
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("任务已经在缓存列表,丢弃!"));
			return true;
		}
	}

	std::vector<pTaskInfo>::iterator itCacheList = m_pCacheList.begin();
	for (; itCacheList != m_pCacheList.end(); ++itCacheList)
	{
		if (tData == (*itCacheList)->_pTask->GetData())
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("任务已经在执行列表,丢弃!"));
			return true;
		}
	}

	return false;
}

//轮循任务列表，采取由任务本身检测是否需要执行的控制机制
//任务启动执行后，将其从缓冲列表移入运行列表
void CTaskMgr::PollingTask(void)
{
	m_pLock->Lock();

ReChk:

	//未达到最大允许的并发任务数时检查是否有任务可以执行
	if (m_pvRunList.size() < m_iMaxConcurrencyTask)
	{
		std::vector<pTaskInfo>::iterator itCacheTask = m_pCacheList.begin();

		for (; itCacheTask != m_pCacheList.end(); ++itCacheTask)
		{
			CTaskBase* pTask = (*itCacheTask)->_pTask;
			if (pTask->CanExecNow())
			{
				//执行任务，将其移入执行列表并从缓冲列表中移出
				if (pTask->Exec())
				{
					m_pvRunList.push_back(*itCacheTask);
					m_pCacheList.erase(itCacheTask);
					goto ReChk;
				}
			}
		}
	}
	m_pLock->Unlock();
}

//清空任务列表
void CTaskMgr::ClearTaskList(void)
{
	m_pLock->Lock();

	if (!m_pCacheList.empty())
	{
		std::vector<pTaskInfo>::iterator itCacheTask = m_pCacheList.begin();

		for (; itCacheTask != m_pCacheList.end();)
		{
			std::vector<pTaskInfo>::iterator itTmp = itCacheTask;
			++itCacheTask;

			delete *itTmp;
		}
		m_pCacheList.clear();
	}

	for (int i = m_pvRunList.size(); i > 0; i--)
	{
		delete m_pvRunList[i - 1];
	}
	m_pvRunList.clear();

	m_pLock->Unlock();

}


//任务执行完成
void CTaskMgr::TaskFinished(CTaskBase *pTask)
{
	if (NULL != pTask)
	{
		if (m_pvRunList.size() <= 0)
		{
			return;
		}

		pTask->WaitForRunningEvent();
		//向上层提交任务结果
		pTask->Stop();

		//如果提交任务结果失败或者是任务超时的情况均作为需要等待请求任务
		if (!pTask->SendResultToServer() || pTask->GetTimeOutFlag())
		{	
			g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("本次普通任务超时或失败,需要限制请求,TaskInfo:%s"), pTask->GetTaskInfo());
			m_pTaskStreamCtr->ResetRoundCount(pTask->GetCurrTaskType());
		}
		else
		{
			//Modified Date 2018/05/04
			m_pTaskStreamCtr->SetRoundCount(pTask->GetCurrTaskType());
		}

		PushStopTask(pTask);
		//请求下一个任务
		pTask->RequestNextTask();

		pTask->SetEvent();

	}
}

//返回已经缓冲的任务总数
int CTaskMgr::GetCachingTaskCount(void)
{
	return m_pCacheList.size();
}


//返回正在执行的任务总数
int CTaskMgr::GetExecingTaskCount(void)
{
	return m_pvRunList.size();
}

//停止所有任务,包括正在执行的和尚未执行的
void CTaskMgr::TerminateAllTasks(void)
{
	//停止正在执行的任务
	m_pLock->Lock();
	std::vector<pTaskInfo>::iterator itRunTask = m_pvRunList.begin();
	for (; itRunTask != m_pvRunList.end(); ++itRunTask)
	{
		(*itRunTask)->_pTask->Stop();
	}
	//清空任务列表
	ClearTaskList();
	m_pLock->Unlock();
}

// 接收到通告函数创建轮循任务线程
void CTaskMgr::OnNotifyRollingTask()
{
	// 如果线程已经存在,无需创建
	if (m_hThreadTaskPolling != NULL)
	{
		DWORD dwExitCode = 0;
		::GetExitCodeThread(m_hThreadTaskPolling, &dwExitCode);
		if (dwExitCode == STILL_ACTIVE)
		{
			//g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("轮循线程已经存在"));
			return;
		}
	}
	m_hThreadTaskPolling = CreateThread(NULL, 0, &CTaskMgr::ThreadProcPollingTask, this, 0, NULL);
}

// 轮循任务线程
DWORD WINAPI CTaskMgr::ThreadProcPollingTask(LPVOID lpParameter)
{
	CTaskMgr *pThis = (CTaskMgr *)lpParameter;

	while (!pThis->m_bPollingThreadStop)
	{
		Sleep(100);
		pThis->PollingTask();

		//相关规则检测
		pThis->CheckUserInput();

		//任务超时检测
		pThis->CheckOutTimeTask();
	}

	return 0;
}

// 设置轮循线程标志
void CTaskMgr::SetPollingFlag(bool bFlag)
{
	m_bPollingThreadStop = bFlag;
}

// 等待轮循线程结束
void CTaskMgr::WaitEnd()
{
	WaitForSingleObject(m_hThreadTaskPolling, INFINITE);
	CloseHandle(m_hThreadTaskPolling);
	m_hThreadTaskPolling = NULL;
}

// 清理TASK内存通告
void CTaskMgr::NotifyReleaseTask(CTaskBase* pTask)
{
	// 要创建单独的线程防止阻塞
	m_pLockDelete->Lock();
	m_pTaskList.push_back(pTask);
	m_pLockDelete->Unlock();

	HANDLE hThread = CreateThread(NULL, 0, &CTaskMgr::ThreadProcDeleteTask, this, 0, NULL);
	if (hThread)
	{
		CloseHandle(hThread);
	}
	return;
}

// 压入结束任务到任务完成列表等待删除
void CTaskMgr::PushStopTask(CTaskBase* pTask)
{
	//从执行列表中移除
	m_pLock->Lock();
	std::vector<pTaskInfo>::iterator itTask = m_pvRunList.begin();
	for (; itTask != m_pvRunList.end(); ++itTask)
	{
		if ((*itTask)->_pTask == pTask)
		{
			m_pLockDelete->Lock();
			m_pFinishTaskList.push_back(*itTask);
			m_pLockDelete->Unlock();
			m_pvRunList.erase(itTask);
			g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("自动任务执行结束，TaskInfo:%s"), pTask->GetTaskInfo());
			break;
		}
	}
	m_pLock->Unlock();
}

// 清除操作
void CTaskMgr::Release()
{
	m_pLockDelete->Lock();
	CTaskBase* pTask = NULL;
	while (m_pTaskList.size() > 0)
	{
		std::vector<CTaskBase*>::iterator itTaskList = m_pTaskList.begin();
		pTask = *itTaskList;
		if (pTask != NULL)
		{
			std::vector<pTaskInfo>::iterator itList = m_pFinishTaskList.begin();
			for (; itList != m_pFinishTaskList.end(); ++itList)
			{
				if ((*itList)->_pTask == pTask)
				{
					pTask->WaitExit();
					g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("DELETE任务，TaskInfo:%s"), pTask->GetTaskInfo());
					delete (*itList);
					m_pFinishTaskList.erase(itList);
					m_pTaskList.erase(itTaskList);
					break;
				}
			}
		}
	}
	m_pLockDelete->Unlock();
}


// 清除任务内存线程函数
DWORD WINAPI CTaskMgr::ThreadProcDeleteTask(LPVOID lpParameter)
{
	CTaskMgr *pThis = (CTaskMgr *)lpParameter;
	pThis->Release();
	return 0;
}

// 检测运行任务超时(超时就结束)
void CTaskMgr::CheckOutTimeTask()
{
	m_pLock->Lock();
	std::vector<pTaskInfo>::iterator itTask = m_pvRunList.begin();
	for (; itTask != m_pvRunList.end();)
	{
		CTaskBase* pTask = (*itTask)->_pTask;
		if (pTask->IsTimeOut())
		{
			g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("任务超时，TaskInfo:%s"), pTask->GetTaskInfo());

			//任务结束，提交任务结果
			TaskFinished(pTask);
			// 通知清理内存
			NotifyReleaseTask(pTask);

			//此处必须要跳出循环，否则会出现异常，因为m_pvRunList大小已经改变
			break;
		}
		++itTask;
	}
	m_pLock->Unlock();
}

void CTaskMgr::CheckUserInput()
{
	m_pLock->Lock();
	std::vector<pTaskInfo>::iterator itTask = m_pvRunList.begin();
	for (; itTask != m_pvRunList.end();)
	{
		CTaskBase* pTask = (*itTask)->_pTask;

		//如果未配置RunObj标记，则如果用户正在进行输入或者是锁屏需要终止相关推广任务
		if (!m_pTaskStreamCtr->GetRunObjTaskFlag())
		{	
			if (!m_pTaskStreamCtr->IsInputIdleOk() && pTask->GetCurrTaskType() == eType_MainTask)
			{	
				g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("推广过程中用户正在输入,停止任务:TaskInfo:%s,提交任务结果"), pTask->GetTaskInfo());
				//任务结束，提交任务结果
				//TerminateTask(pTask);
				TerminateTaskAndSendResult(pTask);

				// 通知清理内存
				NotifyReleaseTask(pTask);

				//此处必须要跳出循环，否则会出现异常，因为m_pvRunList大小已经改变
				break;
			}
			else if (m_pTaskStreamCtr->IsLockScreen() && pTask->GetCurrTaskType() == eType_MainTask &&  m_pTaskStreamCtr->IsObjTaskRunning())
			{	
				g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("推广过程中屏幕锁定或出现屏保,停止任务:TaskInfo:%s,提交任务结果"), pTask->GetTaskInfo());
				//任务结束，提交任务结果
				//TerminateTask(pTask);
				TerminateTaskAndSendResult(pTask);

				// 通知清理内存
				NotifyReleaseTask(pTask);

				//此处必须要跳出循环，否则会出现异常，因为m_pvRunList大小已经改变
				break;
			}
		}
		++itTask;
	}
	m_pLock->Unlock();
}

void CTaskMgr::TerminateTaskAndSendResult(CTaskBase *pTask)
{
	if (NULL != pTask)
	{
		if (m_pvRunList.size() <= 0)
		{
			return;
		}
		pTask->WaitForRunningEvent();
		//停止任务
		pTask->Stop();
		//提交结果
		if (!pTask->SendResultToServer())
		{
			g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("主控没有返回数据或提交任务结果失败,TaskInfo:%s"), pTask->GetTaskInfo());
		}

		//加入任务停止队列
		PushStopTask(pTask);
		//请求下一个任务
		pTask->RequestNextTask();
		pTask->SetEvent();
	}
}

// void CTaskMgr::TerminateTask(CTaskBase *pTask)
// {
// 	if (NULL != pTask)
// 	{
// 		if (m_pvRunList.size() <= 0)
// 		{
// 			return;
// 		}
// 
// 		//停止任务不提交结果
// 		pTask->Stop();
// 		
// 		//加入任务停止队列
// 		PushStopTask(pTask);
// 		//请求下一个任务
// 		pTask->RequestNextTask();
// 	}
// }
