#include "StdAfx.h"
#include "ScheduleMgr.h"
#include "AutoQuickPhoto.h"


extern HANDLE g_hEvent;

CScheduleMgr::CScheduleMgr(CScheduleThread *pScheduleThread, CAutoGetKeywordPages *pAutoGetKeywordPages)
{
	m_iMaxConcurrencyTask = 1;
	m_pScheduleThread = pScheduleThread;
	m_pAutoGetKeywordPages = pAutoGetKeywordPages;
	m_pmapCacheList.clear();
	m_pvRunList.clear();

	m_pLock = new CLock;
}


CScheduleMgr::~CScheduleMgr(void)
{
	ClearTaskList();
	delete m_pLock;
}

void CScheduleMgr::Init(int iMaxConncurrencyTask)
{
	m_iMaxConcurrencyTask = iMaxConncurrencyTask;
}

//向缓存列表中压入一个任务，根据任务数据自动创建任务对象
void CScheduleMgr::PushTask(T_TASK_DATA tData)
{	
	CLocalLock lock(m_pLock);

	if (FindTaskInTaskList(tData))
	{
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("目标任务(type:%s subject:%s)已在缓冲列表或正在执行,PushTask返回!"), tData.strType, tData.strSubject);
		return;
	}

	//创建任务对象
	if (!tData.strType.CompareNoCase(_T("QuickPhoto")))
	{
		CScheduleBase *pTask = new CAutoQuickPhoto(m_pScheduleThread, m_pAutoGetKeywordPages);
		pTask->Init();
		pTask->SetData(tData);
		pTask->SetTaskMgr(this);

		m_pmapCacheList[pTask] = GetTickCount();
		g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("压入自动任务,进入缓冲：type:%s subject:%s"), tData.strType, tData.strSubject);
	}
	else
	{
		//创建其它任务
		//...
	}
}

//查看任务是否已在列表中
bool CScheduleMgr::FindTaskInTaskList(const T_TASK_DATA &tData)
{
	CLocalLock lock(m_pLock);

	if (!m_pmapCacheList.empty())
	{
		std::map<CScheduleBase *, DWORD>::iterator itCacheTask = m_pmapCacheList.begin();

		for (; itCacheTask != m_pmapCacheList.end(); ++itCacheTask)
		{
			CScheduleBase *pTask = itCacheTask->first;

			if (tData == pTask->GetData())
			{
				return true;
			}
		}
	}

	std::vector<CScheduleBase *>::iterator itRunList = m_pvRunList.begin();
	for (; itRunList != m_pvRunList.end(); ++itRunList)
	{
		if (tData == (*itRunList)->GetData())
			return true;
	}

	return false;
}

//轮循任务列表，采取由任务本身检测是否需要执行的控制机制
//任务启动执行后，将其从缓冲列表移入运行列表
void CScheduleMgr::PollingTask(void)
{
	CLocalLock lock(m_pLock);
	
ReChk:

	//未达到最大允许的并发任务数时检查是否有任务可以执行
	if (m_pvRunList.size() < m_iMaxConcurrencyTask)
	{
		std::map<CScheduleBase *, DWORD>::iterator itCacheTask = m_pmapCacheList.begin();

		for (; itCacheTask != m_pmapCacheList.end(); ++itCacheTask)
		{
			CScheduleBase *pTask = itCacheTask->first;
			DWORD dwEnterTime = itCacheTask->second;

			if (pTask->CanExecNow(dwEnterTime))
			{
				//执行任务，将其移入执行列表并从缓冲列表中移出
				if (pTask->Exec())
				{
					m_pvRunList.push_back(pTask);

					m_pmapCacheList.erase(itCacheTask);
					SetEvent(g_hEvent);
					goto ReChk;
				}
			}
		}
	}
}

//清空任务列表
void CScheduleMgr::ClearTaskList(void)
{
	CLocalLock lock(m_pLock);

	if (!m_pmapCacheList.empty())
	{
		std::map<CScheduleBase *, DWORD>::iterator itCacheTask = m_pmapCacheList.begin();

		for (; itCacheTask != m_pmapCacheList.end(); )
		{
			std::map<CScheduleBase *, DWORD>::iterator itTmp = itCacheTask;
			++itCacheTask;

			delete itTmp->first;
		}
		m_pmapCacheList.clear();
	}
	
	for (int i = m_pvRunList.size(); i > 0; i--)
	{
		delete m_pvRunList[i - 1];
	}
	m_pvRunList.clear();

}


//任务执行完成
void CScheduleMgr::TaskFinished(CScheduleBase *pTask)
{
	if (NULL != pTask)
	{
		//从执行列表中移除
		CLocalLock lock(m_pLock);
		if (m_pvRunList.size() <= 0)
		{
			return;
		}

		//向上层提交任务结果
		pTask->SendResultToServer();
		pTask->Stop();

		std::vector<CScheduleBase *>::iterator itTask = m_pvRunList.begin();
		for (; itTask != m_pvRunList.end(); ++itTask)
		{
			if (*itTask == pTask)
			{
				m_pvRunList.erase(itTask);
				g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,_T("自动任务执行结束，TaskInfo:%s"), pTask->GetTaskInfo());
			    break;
			}
		}

		//向ui发送任务完成消息
		//m_pScheduleThread->OnNotifyUI(WARAM_AUTO_REFRESH_KEYWORD_FINISH);
	}
}

//返回已经缓冲的任务总数
int CScheduleMgr::GetCachingTaskCount(void)
{
	return m_pmapCacheList.size();
}


//返回正在执行的任务总数
int CScheduleMgr::GetExecingTaskCount(void)
{
	return m_pvRunList.size();
}

//停止所有任务,包括正在执行的和尚未执行的
void CScheduleMgr::TerminateAllTasks(void)
{
	//停止正在执行的任务
	CLocalLock lock(m_pLock);
	std::vector<CScheduleBase *>::iterator itRunTask = m_pvRunList.begin();
	for (; itRunTask != m_pvRunList.end(); ++itRunTask)
	{
		(*itRunTask)->Stop();
	}

	//清空任务列表
	ClearTaskList();
	//delete m_pLock;
}