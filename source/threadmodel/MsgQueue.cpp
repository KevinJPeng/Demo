// MsgQueue.cpp: implementation of the CMsgQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MsgQueue.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMsgQueue::CMsgQueue()
{
	InitializeCriticalSection(&m_cs);
}

CMsgQueue::~CMsgQueue()
{
	DeleteCriticalSection(&m_cs);
}

void CMsgQueue::Lock()
{
	EnterCriticalSection(&m_cs);
}

void CMsgQueue::Unlock()
{
	LeaveCriticalSection(&m_cs);
}

void CMsgQueue::PostMsg(T_Message *pMsgParam, E_MSG_PRI ePri)
{
	if (pMsgParam != NULL)
	{
// 		T_Message *pMsg = IMsgQueue::New_Message();
// 		pMsg->dwSourWork = pMsgParam->dwSourWork;
// 		pMsg->dwDestWork = pMsgParam->dwDestWork;
// 		pMsg->dwMsg = pMsgParam->dwMsg;
// 		pMsg->wParam = pMsgParam->wParam;
// 		pMsg->lParam = pMsgParam->lParam;
// 		pMsg->dwMsgType = pMsgParam->dwMsgType;
// 		pMsg->hSynHandle = pMsgParam->hSynHandle;

		if(ePri == eMSG_PUSH_TO_END)
			PostBackMsg(pMsgParam);
		else
			PostFrontMsg(pMsgParam);
	}
}

void CMsgQueue::PostBackMsg(T_Message *pMsg)
{
	m_MsgQueue.push_back(pMsg);
}

void CMsgQueue::PostFrontMsg(T_Message *pMsg) 
{
		m_MsgQueue.push_front(pMsg);
}

T_Message *CMsgQueue::GetMsg(DWORD dwDestWork)
{
	T_Message *pMsg = NULL;
	if(m_MsgQueue.empty() == false)
	{
		pMsg = m_MsgQueue.front();
		if(pMsg->dwDestWork == dwDestWork)
		{
			m_MsgQueue.pop_front();
			pMsg->EnterTime = ::GetTickCount(); //消息出队列时间
			return pMsg;
		}
	}	
	return NULL;
}


bool CMsgQueue::Empty()
{
	bool bRet = m_MsgQueue.empty();
	return bRet;
}

unsigned int CMsgQueue::Size()
{
	unsigned int iSize = m_MsgQueue.size();
	return iSize;
}
