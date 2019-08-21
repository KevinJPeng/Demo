#include "StdAfx.h"
#include "UtilityThread.h"

#include "SiteAnalyze.h"
#include "ClearFiles.h"
#include "Keyword.h"

CRITICAL_SECTION g_cs;

CUtilityThread::CUtilityThread(void)
:IThreadUnit(E_THREAD_UTILITY, 0xFFFF)
{
	m_pSiteAnalyze = new CSiteAnalyze(this);
	m_pKeyword = new CKeyword(this);
	m_pClearFile = new CClearFiles;
	InitializeCriticalSection(&g_cs);
}


CUtilityThread::~CUtilityThread(void)
{
	if (m_pSiteAnalyze != NULL)
	{
		delete m_pSiteAnalyze;
		m_pSiteAnalyze = NULL;
	}

	if (m_pKeyword != NULL)
	{
		delete m_pKeyword;
		m_pKeyword = NULL;
	}

	if (m_pClearFile != NULL)
	{
		delete m_pClearFile;
		m_pClearFile = NULL;
	}
	DeleteCriticalSection(&g_cs);
}


DWORD CUtilityThread::DispatchMessage(T_Message *pMsg)
{	
	switch (pMsg->dwMsg)
	{
	//网站综合查询 --begin--
	case MSG_WEB_SEARCH:
		{
			CString strUrl = *((LPTSTR*)pMsg->wParam);
			m_pSiteAnalyze->WebSearch(strUrl);
		}
		break;

	case MSG_CANCEL_WEB_SEARCH:
		{
			m_pSiteAnalyze->CancelWebSearch();

			T_Message* pMsg = IMsgQueue::New_Message();
			pMsg->dwDestWork = E_THREAD_TYPE_UIHELPER;
			pMsg->dwSourWork = E_THREAD_UTILITY;
			pMsg->dwMsg = MSG_CANCEL_WEB_SEARCH;
			PostMessage(pMsg);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("WebSearch程序已取消搜索操作！"));
		}
		break;
		//网站综合查询 --end--
    
	//清理快照 --begin--
	case MSG_CLEAR_QUICK_PHOTOS:
	    {
			m_pClearFile->ClearQuickPhotos();
	    }
		break;

	case MSG_SYSTEM_CLEAR_CANCEL:
	    {
			m_pClearFile->ClearCancel();
	    }
		break;

	case MSG_CLEAR_CODE:
	    {
			/*m_pClearFile->ClearCodeImg();*/
	    }
		break;
	//清理快照 --end--

	//关键词分析 --begin--
	case MSG_KEYWORD_ANALYSIS:
		{		
			CString strParam = *((CString*)pMsg->wParam);
			m_pKeyword->KeywordAnalysis(strParam);
		}
		break;


	case MSG_CANCEL_KEYWORD_ANALYSIS:
		{
			m_pKeyword->CancelKeywordAnalysis();

			T_Message* pMsg = IMsgQueue::New_Message();
			pMsg->dwDestWork = E_THREAD_TYPE_UIHELPER;
			pMsg->dwSourWork = E_THREAD_UTILITY;
			pMsg->dwMsg = MSG_CANCEL_KEYWORD_ANALYSIS;
			PostMessage(pMsg);
		}
		break;
	//关键词分析 --end--


	case MSG_SALF_EXIT:
		{
			//网站综合查询安全退出
			m_pSiteAnalyze->SafeExit();

			//关键词查询安全退出
			m_pKeyword->SafeExit();
		}
		break;

	default:
		break;
	}

	return 0;
}


void CUtilityThread::ReturnDataToUI(const CString &strData, DWORD Msg,DWORD flag)
{
	EnterCriticalSection(&g_cs);
	T_Message *pMsg = IMsgQueue::New_Message();
	pMsg->dwDestWork = E_THREAD_TYPE_UIHELPER;
	pMsg->dwSourWork = E_THREAD_UTILITY;
	pMsg->dwMsg = Msg;
	pMsg->wParam = (WPARAM)(strData.GetString());
	pMsg->lParam = flag;

	SendMessage(pMsg);

	LeaveCriticalSection(&g_cs);
}
