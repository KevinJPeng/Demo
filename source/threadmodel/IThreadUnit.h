// IThreadUnit.h: interface for the IThreadUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITHREADUNIT_H__F84C5776_9989_4C7A_A8BF_57769BC5A93E__INCLUDED_)
#define AFX_ITHREADUNIT_H__F84C5776_9989_4C7A_A8BF_57769BC5A93E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MsgDef.h"
#include "IMsgQueue.h"
#include "IThreadManage.h"

class CThreadManage;

class IThreadUnit  
{
	friend class CThreadManage;

private:
	T_Message m_CurMsg;        //正在处理的当前消息
	IMsgQueue *m_pMsgQueue;     //消息队列指针
	DWORD m_dwThreadType;	   //消息类型，用于区分线程功能
	DWORD m_dwTimeOut;         //线程超时时间
	HANDLE m_hHandle;	       //线程句柄
	HANDLE m_synMsgHandle;     //用于同步消息句柄
	BOOL m_bRunFlag;		   //运行标志，FALSE时线程退出
	BOOL m_bBusy;              //标志如果为TRUE,哪么，不重启线程，为FALSE否则才重启
	IThreadManage *m_pThreadManage;

	//线程入口点
	static DWORD WINAPI StaticThreadFunc(LPVOID lpParam);

public:
	//创建线程
	DWORD Create(IMsgQueue *pIMsgQueue);

	//终止线程
	BOOL Terminated();	

	//重启线程
	virtual DWORD RestThread();

	//异步消息
	void PostMessage(T_Message *pMsg, E_MSG_PRI ePri = eMSG_PUSH_TO_END); 

	//同步消息
	void SendMessage(T_Message *pMsg, E_MSG_PRI ePri = eMSG_PUSH_TO_END); 

	//锁定线程不可以重启
	void Lock();

	//解除锁定，可以重启
	void UnLock();

public:
	IThreadUnit(DWORD dwThreadType = 0xFFFF, DWORD dwTimeOut = 0xFFFF); //线程类型、超时时间以秒为单位, dwTimeOut=0xFFFF表示无超时
	virtual ~IThreadUnit();

 	//线程初始化
	virtual DWORD ThreadInit() { return 0; }

	//线程退出时清理
	virtual DWORD ThreadClean() { return 0; }

	//线程消息处理
	virtual DWORD DispatchMessage(T_Message *pMsg) = 0;

	//默认线程处理
	virtual DWORD DefaultProc(){ return 0;}

};

#endif // !defined(AFX_ITHREADUNIT_H__F84C5776_9989_4C7A_A8BF_57769BC5A93E__INCLUDED_)
