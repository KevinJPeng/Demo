#pragma once
#include "..\..\threadmodel\ithreadunit.h"

class CDemo :
	public IThreadUnit
{
public:
	CDemo(void);
	~CDemo(void);

	//线程消息处理
	DWORD DispatchMessage(T_Message *pMsg);
};

