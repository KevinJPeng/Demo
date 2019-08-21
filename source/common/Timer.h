#pragma once
#include <Windows.h>
class CTimer
{
public:
	CTimer(void);
	~CTimer(void);

public:
	//创建定时器
	//bool CreateTimer(void);

	//设置定时器
	bool InstallTimer(HANDLE &hTimer, LONGLONG dwTime, LONG lPeriod);

	//取消定时器
	bool CancelTimer(void);

	//销毁定时器
	bool DestroyTimer(void);

	//设置句柄
	void SetData(HANDLE hTimer);

private:
	HANDLE m_hTimer;
};

