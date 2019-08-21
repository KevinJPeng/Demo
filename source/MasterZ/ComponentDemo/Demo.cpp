#include "StdAfx.h"
#include "Demo.h"


CDemo::CDemo(void)
:IThreadUnit(E_THREAD_DEMO, 0xFFFF)
{
}


CDemo::~CDemo(void)
{
}

DWORD CDemo::DispatchMessage(T_Message *pMsg)
{
	T_Message *tMsg = IMsgQueue::New_Message();
	switch (pMsg->dwMsg)
	{
	case MSG_TIMER:
		{
 //			TCHAR tmpbuf[128] = {0};
// 			_tstrtime_s( tmpbuf, 128);
// 
// 			tMsg->dwDestWork = E_THREAD_TYPE_UIHELPER;
// 			tMsg->dwMsg = MSG_DEMO_TEST;
// 			tMsg->dwSourWork = E_THREAD_DEMO;
// 			tMsg->lParam = (LPARAM)tmpbuf;
// 			SendMessage(tMsg);

//			TCHAR tmpbuf[128] = {0};
//			_tstrtime_s( tmpbuf, 128);
//			static int iProgress = 0;
//			iProgress = iProgress + 2;
//			iProgress = iProgress % 100;
//
//			tMsg->dwDestWork = E_THREAD_TYPE_UIHELPER;
//			tMsg->dwMsg = MSG_PRODUCT_UPDATE;
//			tMsg->dwSourWork = E_THREAD_DEMO;
//			tMsg->lParam = (LPARAM)iProgress;
//		SendMessage(tMsg);

			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到Timer消息！"));
		}
		break;

	default:
		break;
	}

	return 0;
}
