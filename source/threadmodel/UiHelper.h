// UiHelper.h: interface for the CSaveThread class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_UIHELPER_H__64960593_7EA6_4892_9D30_5F9B9D76032D__INCLUDED_)
#define AFX_UIHELPER_H__64960593_7EA6_4892_9D30_5F9B9D76032D__INCLUDED_

#if _MSC_VER > 1000
#pragma once 

#endif // _MSC_VER > 1000

#include "IThreadUnit.h"


class CUiHelper  :public IThreadUnit
{
public:
	CUiHelper();
	virtual ~CUiHelper();

public:

	//消息分发
    DWORD DispatchMessage(T_Message *pMsg);

	//设置接收消息的窗口句柄
	void SetUiWnd(HWND hWnd);

private:
	HWND m_hWnd;       //UI窗口句柄
};

#endif // !defined(AFX_UIHELPER_H__64960593_7EA6_4892_9D30_5F9B9D76032D__INCLUDED_)
