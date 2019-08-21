// IMsgQueue.h: interface for the IMsgQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMSGQUEUE_H__80456B97_1A94_4133_B9E6_235810D22751__INCLUDED_)
#define AFX_IMSGQUEUE_H__80456B97_1A94_4133_B9E6_235810D22751__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//消息优先级
enum E_MSG_PRI
{
	eMSG_PUSH_TO_END = 0,          //插入消息队列尾部
	eMSG_PUSH_TO_FRONT,            //插入消息队列头部
};


enum E_MSG_TYPE
{
	eMSG_ASYNC = 0,                 //异步消息
	eMSG_SYNC,                      //同步消息
};

struct T_Message{
	DWORD dwSourWork;	//发起者（用于区分是由谁发起）
	DWORD dwDestWork;	//目的地（用于区分是由谁处理）
	DWORD dwMsg;	    //消息(用于命令路由)
	WPARAM wParam;      //消息参数，通常用来存储小段信息（如标志）
	LPARAM lParam;      //消息参数，通常用于存储消息所需的对象

	DWORD EnterTime;	//消息开始处理的时间
	DWORD dwMsgType;    //消息类型,1表示同步消息，0表示异步消息
	HANDLE hSynHandle;	//用于同步消息类型

	T_Message()
	{
		dwSourWork = 0;
		dwDestWork = 0;
		dwMsg = 0;
		wParam = NULL;
		lParam = NULL;
		EnterTime = 0xFFFFFFFF;
		dwMsgType = eMSG_ASYNC;    //默认为异步消息
		hSynHandle = NULL;
	}

	friend class CThreadManage;
	friend class IThreadUnit;
	friend class CMsgQueue;
};


class IMsgQueue  
{
public:

	//消息队列锁定
	virtual void Lock() = 0;             

	//释放消息队列锁定
	virtual void Unlock() = 0;           

	//压入消息(dwLevel:0表示从插入队列尾部  1表示插入队列头部)
	virtual	void PostMsg(T_Message *pMsg, E_MSG_PRI ePri = eMSG_PUSH_TO_END) = 0;       

	//检测当前消息类型
	virtual T_Message *GetMsg(DWORD dwDestWork) = 0;       

	//判断是否为空
	virtual bool Empty() = 0;     

	virtual unsigned int Size() = 0;

	//新建消息内存
	static T_Message* New_Message();                     

	 //释放消息内存
	static void Free_Message(T_Message *pMsg);          
};

#endif // !defined(AFX_IMSGQUEUE_H__80456B97_1A94_4133_B9E6_235810D22751__INCLUDED_)
