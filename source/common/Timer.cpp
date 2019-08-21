#include "StdAfx.h"
#include "Timer.h"


CTimer::CTimer(void)
{
	m_hTimer = NULL;
}


/*
@brief: 销毁匿名定时器
@param: null
@return: null
*/
CTimer::~CTimer(void)
{
	if (m_hTimer != NULL)
	{
		CloseHandle(m_hTimer);
	}
}

void CTimer::SetData(HANDLE hTimer)
{
	m_hTimer = hTimer;
}


/*
@brief: 创建匿名定时器
@param: null
@return: 是否创建定时器成功, true表示成功
*/
// bool CTimer::CreateTimer(void)
// {
// 	m_hTimer = CreateWaitableTimer(NULL, true, NULL);
// 
// 	if (m_hTimer == NULL)
// 	{
// 		return false;
// 	}
// 
// 	return true;
// }

/*
@brief: 设置定时器
@param: pDueTime:第一次触发定时器的间隔时间，以毫秒为单位  lPeriod:之后每次触发定时器的间隔时间
@return: 是否设置定时器成功, true表示成功
*/
bool CTimer::InstallTimer(HANDLE &hTimer, LONGLONG dwTime, LONG lPeriod)
{
	SetData(hTimer);

	hTimer = CreateWaitableTimer(NULL, true, NULL);

	if (hTimer == NULL)
	{
		return false;
	}

	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -(10000 * dwTime); 

	SetWaitableTimer(hTimer, &liDueTime, lPeriod, NULL, NULL, false);

	return true;
}


/*
@brief: 取消匿名定时器
@param: null
@return: 是否取消定时器成功, true表示成功
*/
bool CTimer::CancelTimer(void)
{
	if (!CancelWaitableTimer(m_hTimer))
	{
		return false;
	}
	
	return true;
}

/*
@brief: 销毁匿名定时器
@param: null
@return: 是否销毁定时器成功, true表示成功
*/
bool CTimer::DestroyTimer(void)
{
	CloseHandle(m_hTimer);

	if (m_hTimer != NULL)
	{
		return false;
	}

	return true;
}
