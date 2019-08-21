// ThreadManage.h: interface for the CThreadManage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREADMANAGE_H__8925966E_E55A_461C_8D2F_BE420AB5ACB4__INCLUDED_)
#define AFX_THREADMANAGE_H__8925966E_E55A_461C_8D2F_BE420AB5ACB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

 
#include "MsgQueue.h"
#include "IThreadManage.h"
#include <list>
#include <map>

class IThreadUnit;

class CThreadManage :public IThreadManage 
{
private:
	std::list<IThreadUnit*>m_listThread;
	std::map<DWORD,IMsgQueue*> m_MsgQueueMap;     //消息队列map
	HANDLE m_hTimer;                              //记时器
	HANDLE m_hHandle;	                          //线程句柄
	BOOL m_bExitFlag;	                          //退出标志
	HANDLE m_synMsgHandle;                        //用于同步消息句柄

	//线程入口点
	static DWORD WINAPI StaticThreadFunc(LPVOID lpParam);

	//检测线程状态
	void CheckThreadState();

	//线程运行
	DWORD Run();

public:
	CThreadManage();
	virtual ~CThreadManage();

	//创建
	virtual DWORD Create();

	//释放
	virtual DWORD Destory(); 

	//增加线程
	virtual BOOL InsertThread(IThreadUnit *pThread); 

	//删除线程
	virtual void RemoveThread(IThreadUnit *pThread); 

	//压入消息(dwLevel:0表示从插入队列尾部  1表示插入队列头部)
	virtual void PostMessage(T_Message *pMsg, E_MSG_PRI ePri = eMSG_PUSH_TO_END);

	//向线程发送同步消息(dwLevel:0表示从插入队列尾部  1表示插入队列头部)
	virtual void SendMessage(T_Message *pMsg, E_MSG_PRI ePri = eMSG_PUSH_TO_FRONT);

	//广播消息
	virtual void BroadcastMsg(DWORD dwMsg, E_MSG_TYPE eMsgType = eMSG_ASYNC);
};

#endif // !defined(AFX_THREADMANAGE_H__8925966E_E55A_461C_8D2F_BE420AB5ACB4__INCLUDED_)
