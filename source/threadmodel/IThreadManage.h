// IThreadManage.h: interface for the IThreadManage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITHREADMANAGE_H__AA3B1BD7_F99D_4830_A853_6550C823D954__INCLUDED_)
#define AFX_ITHREADMANAGE_H__AA3B1BD7_F99D_4830_A853_6550C823D954__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class IThreadUnit;
#include "IMsgQueue.h"

class IThreadManage  
{
public:
	IThreadManage();
	virtual ~IThreadManage();

	 //创建
	virtual DWORD Create() = 0;

	//释放
	virtual DWORD Destory() = 0; 

	//增加线程
	virtual BOOL InsertThread(IThreadUnit *pThread) = 0; 

	//删除线程
	virtual void RemoveThread(IThreadUnit *pThread) = 0; 

	//向线程发送异步消息
	virtual void PostMessage(T_Message *pMsg, E_MSG_PRI ePri = eMSG_PUSH_TO_END) = 0;

	//向线程发送同步消息
	virtual void SendMessage(T_Message *pMsg, E_MSG_PRI ePri = eMSG_PUSH_TO_END) = 0;


	//广播消息
	virtual void BroadcastMsg(DWORD dwMsg, E_MSG_TYPE eMsgType = eMSG_ASYNC) = 0;

};

#endif // !defined(AFX_ITHREADMANAGE_H__AA3B1BD7_F99D_4830_A853_6550C823D954__INCLUDED_)
