// MsgQueue.h: interface for the CMsgQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGQUEUE_H__2E162580_810B_4C9B_B614_21C9D8BC0AE6__INCLUDED_)
#define AFX_MSGQUEUE_H__2E162580_810B_4C9B_B614_21C9D8BC0AE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMsgQueue.h"
#include <deque>

class CMsgQueue  : public IMsgQueue
{
private:
	std::deque<T_Message*> m_MsgQueue;
	CRITICAL_SECTION m_cs;

public:
	CMsgQueue();
	virtual ~CMsgQueue();

	//消息队列锁定
	virtual void Lock();             

	//释放消息队列锁定
	virtual void Unlock();           

	//压入消息(dwLevel:0表示从插入队列尾部  1表示插入队列头部)
	virtual	void PostMsg(T_Message *pMsg, E_MSG_PRI ePri = eMSG_PUSH_TO_END);       

	//检测当前消息类型
	virtual T_Message *GetMsg(DWORD dwDestWork);       

	//判断是否为空
	virtual bool Empty();              

	//消息数量
	virtual unsigned int Size();	


private:

	//向消息队列的尾部压入消息
	virtual void PostBackMsg(T_Message *pMsg);      

	//向消息队列的头部压入消息
	virtual void PostFrontMsg(T_Message *pMsg);     

};

#endif // !defined(AFX_MSGQUEUE_H__2E162580_810B_4C9B_B614_21C9D8BC0AE6__INCLUDED_)
