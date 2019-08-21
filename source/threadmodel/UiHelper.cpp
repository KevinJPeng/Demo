// UiHelper.cpp: implementation of the CSaveThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UiHelper.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CUiHelper::CUiHelper()
:IThreadUnit(E_THREAD_TYPE_UIHELPER, 0xFFFF)
{
	
}

CUiHelper::~CUiHelper()
{

}

///////////////////////////////////////////////////////////////////////////////
//
// 函数功能描述：线程消息分发
// 输入：
// 输出：
// 返回值：
// 其它说明：
//
///////////////////////////////////////////////////////////////////////////////
DWORD CUiHelper::DispatchMessage(T_Message *pMsg)
{
    switch (pMsg->dwMsg)
    {		
	case MSG_TIMER:
		{
			break;
		}
		//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Timer:源线程ID为%d,消息类型为:%d"),pMsg->dwSourWork,pMsg->dwMsg);
    default:
		{
			//默认将消息转发给UI窗口
			//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("UIhelper收到消息:源线程ID为%d,消息类型为:%d"),pMsg->dwSourWork,pMsg->dwMsg);

			if (m_hWnd)
			{
				DWORD dwRet = 0;
				if (pMsg->dwMsgType == eMSG_SYNC)
				{	
					//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("before send消息:源线程ID为%d,消息类型为:%d"),pMsg->dwSourWork,pMsg->dwMsg);

					if (::SendMessageTimeout(m_hWnd, pMsg->dwMsg, pMsg->wParam, pMsg->lParam, SMTO_BLOCK, 5000, &dwRet) == 0)
					{
						//消息处理失败或响应超时返回-1，本消息将重新放入到消息队列头部
						//且不激活同步事件（此时发起消息的线程仍将等待）				
						g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("消息超时:源线程ID为%d,消息类型为:%d，错误码为:%d"),pMsg->dwSourWork,pMsg->dwMsg,GetLastError());
						return 0;
					}

					//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("After send消息:源线程ID为%d,消息类型为:%d"),pMsg->dwSourWork,pMsg->dwMsg);


					//if (::SendMessage(m_hWnd, pMsg->dwMsg, pMsg->wParam, pMsg->lParam))
					//{
					//	//消息处理失败或响应超时返回-1，本消息将重新放入到消息队列头部
					//	//且不激活同步事件（此时发起消息的线程仍将等待）

					//	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("错误字符串:%d"),GetLastError());
					//	return 0;
					//}
				}
				else
				{
					//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("before post消息:源线程ID为%d,消息类型为:%d"),pMsg->dwSourWork,pMsg->dwMsg);

					::PostMessage(m_hWnd, pMsg->dwMsg, pMsg->wParam, pMsg->lParam);

					//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("After post消息:源线程ID为%d,消息类型为:%d"),pMsg->dwSourWork,pMsg->dwMsg);
				}

			}
			else
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("Hwnd 为空:源线程ID为%d,消息类型为:%d"),pMsg->dwSourWork,pMsg->dwMsg);
			}
		}	
        break;
    }

    return 0;
}

void CUiHelper::SetUiWnd(HWND hWnd)
{
	m_hWnd = hWnd;
}