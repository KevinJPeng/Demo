#pragma once
#include "SiteAnalyzeDlg.h"
#include "UtilityThread.h"

class CUtilityThread;
class CSiteAnalyze
{
public:
	CSiteAnalyze(CUtilityThread *pUtility);
	~CSiteAnalyze(void);

	//搜索消息的线程
	static DWORD WINAPI ThreadProc(LPVOID lp);

	//将处理的消息或者失败的消息发给ui,flag表示获取数据成功或失败或完成
	void ReturnDataToUI(const CString &strData, DWORD flag);

	void WebSearch(const CString &strUrl);
	void SafeExit(void);
	void CancelWebSearch(void);

private:
	CSiteAnalyzeDlg *m_pWebSearch;
	HANDLE m_thread;
	CString m_strUrl;
	bool m_bCancel;

	CUtilityThread *m_pUtility;

};