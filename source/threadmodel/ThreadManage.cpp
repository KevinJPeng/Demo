// ThreadManage.cpp: implementation of the CThreadManage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ThreadManage.h"
#include "IThreadUnit.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static UINT TIMER_PERIOD = 1000; //定时器的时间间隔(线程超时)
static UINT TASK_TIME_OUT = 2*1000; //线程超时



CThreadManage::CThreadManage()
{
	
}

CThreadManage::~CThreadManage()
{

}

//线程入口点
DWORD WINAPI CThreadManage::StaticThreadFunc(LPVOID lpParam)
{
	CThreadManage * pThread = (CThreadManage *)lpParam;
    
    if (pThread)
    {
        return pThread->Run();
    } 
	return 0;
}

//线程运行
DWORD CThreadManage::Run()
{
	while (m_bExitFlag)
	{
		::WaitForSingleObject(m_hTimer,INFINITE);
		CheckThreadState();	//检测线程状态
	}
	return 0;
}

DWORD CThreadManage::Create()
{
	m_bExitFlag = TRUE;
	m_hTimer = CreateWaitableTimer(NULL, FALSE, NULL);	
    if (NULL == m_hTimer)
    {
		return 1;
    }
    LARGE_INTEGER DueTimeOther;
    DueTimeOther.QuadPart = 0;
    if (!SetWaitableTimer(m_hTimer, &DueTimeOther, TIMER_PERIOD,
        NULL, NULL, TRUE))
	{
		return 2;
	}

	//创建用于同步消息事件句柄
	m_synMsgHandle = ::CreateEvent(NULL,TRUE,TRUE,NULL);

	DWORD dwThreadID = 0;
	m_hHandle = ::CreateThread(NULL, 0, 
					StaticThreadFunc, 
                    this, 0, &dwThreadID);

    if (!m_hHandle || !dwThreadID) //线程创建失败
    {
		return 1;
    }

	return 0;
}

DWORD CThreadManage::Destory()
{
	m_bExitFlag = FALSE;
	if(WaitForSingleObject(m_hHandle,200)==WAIT_TIMEOUT)
	{
		//线程停止超时 强制停止
		if(TerminateThread(m_hHandle,0)==FALSE)
			return -1;
	}
	CloseHandle(m_hHandle);

	std::list<IThreadUnit*>::iterator IterThread;
	for(IterThread=m_listThread.begin();
			IterThread!=m_listThread.end();IterThread++)
	{
		IThreadUnit *pThread = (*IterThread);
		BOOL bRet = pThread->Terminated(); //终止
		if(bRet==FALSE)printf("%d 终止失败\n",pThread->m_dwThreadType);
		printf("%d 终止成功\n",pThread->m_dwThreadType);
		delete pThread;
	}
	m_listThread.clear();

	//释放用于同步消息事件句柄
	SetEvent(m_synMsgHandle);
	CloseHandle(m_synMsgHandle); 

	return 0;
}

void CThreadManage::CheckThreadState()
{
	//g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,\
		_T("%d 检测线程状态 线程数量(%d)\n"),GetCurrentThreadId() ,m_listThread.size());

	std::list<IThreadUnit*>::iterator IterThread;
	for(IterThread=m_listThread.begin(); IterThread!=m_listThread.end(); IterThread++)
	{

		IThreadUnit *pThread = (*IterThread);
		//线程超时
		DWORD dwCurTime = ::GetTickCount();
		if(pThread->m_CurMsg.EnterTime != 0xFFFFFFFF) //线程当前没有处理消息
		{
			if(pThread->m_dwTimeOut != 0xFFFF)
			{
				if( dwCurTime - pThread->m_CurMsg.EnterTime > pThread->m_dwTimeOut*1000 && pThread->m_bBusy == FALSE)
				{
				g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__,\
 					_T("%d线程消息处理消息%d发过来的(%d)超时,终止后重启 %d\n"),
 					pThread->m_CurMsg.dwSourWork,
 					pThread->m_CurMsg.dwDestWork,
 					pThread->m_CurMsg.dwMsg,
 					pThread->m_dwThreadType);
 					printf("%d线程消息处理消息%d发过来的(%d)超时,终止后重启 %d\n",
 						pThread->m_CurMsg.dwSourWork,
 						pThread->m_CurMsg.dwDestWork,
 						pThread->m_CurMsg.dwMsg,
 						pThread->m_dwThreadType);

					pThread->m_CurMsg.dwMsg = MSG_RESETTHREAD;
					PostMessage(&(pThread->m_CurMsg), eMSG_PUSH_TO_FRONT);

					pThread->RestThread(); //终止
				}

			}
		}
	}

	//delete by zhumingxing 20140821
	//BroadcastMsg(MSG_TIMER);	
}

void CThreadManage::BroadcastMsg(DWORD dwMsg, E_MSG_TYPE eMsgType)
{
	std::map<DWORD,IMsgQueue*>::iterator IterMsgQueue;
	for(IterMsgQueue = m_MsgQueueMap.begin();	IterMsgQueue != m_MsgQueueMap.end(); IterMsgQueue++)
	{
		T_Message *pMsg = IMsgQueue::New_Message();  //由消息处理线程负责释放
		pMsg->dwMsg = dwMsg;
		pMsg->dwSourWork = E_THREAD_TYPE_MGR;
		pMsg->dwDestWork = IterMsgQueue->first;

		if (eMsgType == eMSG_ASYNC)
		{
    		PostMessage(pMsg, eMSG_PUSH_TO_END);	//发送异步消息
		}
		else
		{
			if (pMsg->dwDestWork != E_THREAD_TYPE_UIHELPER)
			{
     			SendMessage(pMsg, eMSG_PUSH_TO_FRONT); //发送同步消息
			}
		}
	}
}

//发送消息
void CThreadManage::PostMessage(T_Message *pMsg, E_MSG_PRI ePri)
{
	
	IMsgQueue *MsgQueue = m_MsgQueueMap[pMsg->dwDestWork];
	if(MsgQueue == NULL) //队列为空
	{
		m_MsgQueueMap[pMsg->dwDestWork] = new CMsgQueue;
    	MsgQueue = 	m_MsgQueueMap[pMsg->dwDestWork] ;
	}

	MsgQueue->Lock();
	MsgQueue->PostMsg(pMsg, ePri);
	MsgQueue->Unlock();

	if (pMsg->dwDestWork == E_THREAD_TYPE_UIHELPER && pMsg->dwMsg != MSG_TIMER)
	{
		//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("ui收到消息，消息数为：%d  线程ID%d 消息ID为:%d"), MsgQueue->Size(), pMsg->dwSourWork,pMsg->dwMsg);
	}
}

//向线程发送同步消息
void CThreadManage::SendMessage(T_Message *pMsg, E_MSG_PRI ePri)
{
	IMsgQueue *MsgQueue = m_MsgQueueMap[pMsg->dwDestWork];
	if(MsgQueue == NULL) //队列为空
	{
		m_MsgQueueMap[pMsg->dwDestWork] = new CMsgQueue;
		MsgQueue = 	m_MsgQueueMap[pMsg->dwDestWork] ;
	}

	MsgQueue->Lock();
	::ResetEvent(m_synMsgHandle);
	pMsg->hSynHandle = m_synMsgHandle;
	pMsg->dwMsgType = eMSG_SYNC;
	MsgQueue->PostMsg(pMsg, ePri);
	MsgQueue->Unlock();

	::WaitForSingleObject(m_synMsgHandle,INFINITE); //同步等待

}

BOOL CThreadManage::InsertThread(IThreadUnit *pThread)
{

	m_listThread.push_back(pThread);
	if(pThread->m_dwThreadType == 0xFFFF) //不能创建此线程
	{
		return  FALSE;
	}
	if(m_MsgQueueMap[pThread->m_dwThreadType]==NULL)
	{
		m_MsgQueueMap[pThread->m_dwThreadType] = new CMsgQueue;
	}
	pThread->m_pThreadManage = this; 	
	pThread->Create(m_MsgQueueMap[pThread->m_dwThreadType]);
	return TRUE;
}

//删除工作线程
void CThreadManage::RemoveThread(IThreadUnit *pThread)
{
	pThread->Terminated();
	m_listThread.remove(pThread);
	delete pThread;
}
