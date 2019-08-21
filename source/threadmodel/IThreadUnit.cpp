// IThreadUnit.cpp: implementation of the IThreadUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IThreadUnit.h"
#include "IThreadManage.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


IThreadUnit::IThreadUnit(DWORD dwThreadType,DWORD dwTimeOut)
{
	m_dwThreadType = dwThreadType;
	m_dwTimeOut = dwTimeOut;
	m_bBusy = FALSE;
}

IThreadUnit::~IThreadUnit()
{
	
}


DWORD IThreadUnit::Create(IMsgQueue *pIMsgQueue)
{
	m_bRunFlag = TRUE;	//表示运行
	m_pMsgQueue = pIMsgQueue;
	DWORD dwThreadID = 0;
	
	//创建用于同步消息事件句柄
	m_synMsgHandle = ::CreateEvent(NULL,TRUE,TRUE,NULL);

	m_hHandle = ::CreateThread(NULL, 0, StaticThreadFunc, this, 0, &dwThreadID);
	
    if (!m_hHandle || !dwThreadID) //线程创建失败
    {
		::MessageBoxW(NULL,_T("创建线程失败"),_T("IThreadUnit Error"),MB_OK);
		return 1;
    }

	return 0;
}

//锁定线程不可以重启
void IThreadUnit::Lock()
{
	m_bBusy = TRUE;
}

//解除锁定，可以重启
void IThreadUnit::UnLock() 
{
	m_CurMsg.EnterTime = ::GetTickCount();	//修改出队时间
	m_bBusy=FALSE;
}

BOOL IThreadUnit::Terminated()
{
	m_bRunFlag = FALSE;
	if(WaitForSingleObject(m_hHandle, 200) == WAIT_TIMEOUT)
	{
		m_CurMsg.EnterTime = 0xFFFFFFFF; 

		//线程停止超时 强制停止
		if(TerminateThread(m_hHandle, 0)==FALSE)
			return FALSE;
	}
	CloseHandle(m_hHandle);

	//释放用于同步消息事件句柄
	SetEvent(m_synMsgHandle);
	CloseHandle(m_synMsgHandle);

	return TRUE;
}

DWORD IThreadUnit::RestThread()
{
	Terminated(); //终止
	Create(m_pMsgQueue);

	return 0;
}

DWORD WINAPI IThreadUnit::StaticThreadFunc(LPVOID lpParam)
{
	//CoInitialize(NULL);
	IThreadUnit * pThread = (IThreadUnit *)lpParam;
    if (pThread)
    {
		pThread->ThreadInit();

		while(pThread->m_bRunFlag)
		{
			pThread->DefaultProc();
			if(pThread->m_dwThreadType == 0xFFFF) //线程单元基类无法使用
				return 0;

			//默认一个线程处理3条消息后进行一次线程切换
			for(int i=0;i<3;i++)
			{
				pThread->m_pMsgQueue->Lock();
				T_Message *pMsg = pThread->m_pMsgQueue->GetMsg(pThread->m_dwThreadType);
				pThread->m_pMsgQueue->Unlock();
				if(pMsg!=NULL)
				{
					::CopyMemory(&(pThread->m_CurMsg), pMsg, sizeof(T_Message));
					 
					DWORD dwRet = pThread->DispatchMessage(pMsg); //对消息进行分流处理
		 
					//同步消息且处理成功时，激活事件对象
					if (dwRet == 0 && pMsg->dwMsgType == eMSG_SYNC)
						::SetEvent(pMsg->hSynHandle);

					if(dwRet == -1) //返回-1，表示没有处理消息，再将消息返回到队列中
					{
						g_log.Trace(LOGL_TOP,LOGT_WARNING, __TFILE__,__LINE__, _T("消息未处理（返回-1）, 消息类型:%d, 线程类型:%d, 消息:%d！"), pMsg->dwMsgType, pThread->m_dwThreadType, pMsg->dwMsg);
						pThread->m_pMsgQueue->Lock();
						pThread->m_pMsgQueue->PostMsg(pMsg, eMSG_PUSH_TO_FRONT);
						pThread->m_pMsgQueue->Unlock();
					}
					else
					{
						pThread->m_CurMsg.EnterTime = 0xFFFFFFFF; //0xFFFFFFFF时间表示没有消息
						IMsgQueue::Free_Message(pMsg); //处理完了之后进行释放
					}
				}
				else	//没有消息让系统切换线程
				{
					break;
				}
			}

			Sleep(1);
		}

		pThread->ThreadClean();
    } 

	//CoUninitialize ();
	return 0;
}

//异步消息发送
void IThreadUnit::PostMessage(T_Message *pMsg, E_MSG_PRI ePri)
{
	m_pThreadManage->PostMessage(pMsg, ePri);
}

//同步消息发送
void IThreadUnit::SendMessage(T_Message *pMsg, E_MSG_PRI ePri)
{
	::ResetEvent(m_synMsgHandle);
	pMsg->hSynHandle = m_synMsgHandle;
	pMsg->dwMsgType = eMSG_SYNC;
	m_pThreadManage->PostMessage(pMsg, ePri);
	::WaitForSingleObject(m_synMsgHandle,INFINITE); //同步等待
}