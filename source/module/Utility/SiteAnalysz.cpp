#include "StdAfx.h"
#include "SiteAnalyze.h"

CSiteAnalyze::CSiteAnalyze(CUtilityThread *pUtility)
{
	m_pUtility = pUtility;
	m_pWebSearch = NULL;
	m_thread = NULL;
	m_strUrl = _T("");
	m_bCancel = false;
}


CSiteAnalyze::~CSiteAnalyze(void)
{
	if (m_pWebSearch != NULL)
	{
		delete m_pWebSearch;
		m_pWebSearch = NULL;
	}
}

void CSiteAnalyze::WebSearch(const CString &strUrl)
{
	m_bCancel = false;
	m_strUrl = strUrl;

	m_thread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
}

void CSiteAnalyze::SafeExit(void)
{
	if (m_pWebSearch != NULL)
	{
		if (IsWindow(m_pWebSearch->m_hWnd))
		{
			m_pWebSearch->PostMessage(WM_QUIT);
		}
	}

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("WebSearch程序已安全退出！"));
}

void CSiteAnalyze::CancelWebSearch(void)
{
	if (m_pWebSearch != NULL)
	{
		if (IsWindow(m_pWebSearch->m_hWnd))
		{
			m_pWebSearch->PostMessage(WM_QUIT);
		}
		while (!m_bCancel)
		{
			Sleep(50);
		}
	}

}

DWORD WINAPI CSiteAnalyze::ThreadProc(LPVOID lp)
{
	CSiteAnalyze *pThis = (CSiteAnalyze*)lp;
	CoInitialize(NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	pThis->m_pWebSearch = new CSiteAnalyzeDlg(pThis->m_strUrl, pThis);
	pThis->m_pWebSearch->DoModal();

	if (pThis->m_pWebSearch != NULL)
	{
		delete pThis->m_pWebSearch;
		pThis->m_pWebSearch = NULL;
	}

	pThis->m_bCancel = true;

	CoUninitialize();
	return 0;
}

void CSiteAnalyze::ReturnDataToUI(const CString &strData, DWORD flag)
{
	m_pUtility->ReturnDataToUI(strData, MSG_WEB_SEARCH, flag);
}