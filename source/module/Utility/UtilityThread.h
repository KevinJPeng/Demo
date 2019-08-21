#pragma once
#include "..\..\threadmodel\ithreadunit.h"

class CSiteAnalyze;
class CClearFiles;
class CKeyword;

class CUtilityThread
	:public IThreadUnit
{
public:
	CUtilityThread(void);
	~CUtilityThread(void);

	//线程消息处理
	DWORD DispatchMessage(T_Message *pMsg);


	//将获得的结果返回给ui
	void ReturnDataToUI(const CString &strData, DWORD Msg, DWORD flag);


private:
	CSiteAnalyze *m_pSiteAnalyze;
	CClearFiles *m_pClearFile;
	CKeyword *m_pKeyword;

};

