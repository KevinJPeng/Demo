#pragma once;

class CCheckDiffTime:public CDiagnoseBase
{
public:
	CCheckDiffTime(void);
	virtual ~CCheckDiffTime(void);
public:
	virtual BOOL CheckPro(void);
	virtual BOOL RepairPro(void);
	virtual void GetBackCheckStruct(T_PROBLEM_DATA &tData);
	virtual void GetBackRepairStruct(T_PROBLEM_DATA &tData);

	//获取网络时间
	BOOL GetInternetTime(CTime* pTime, CString strServer);

	//同步系统时间为网络时间
	BOOL SyncSystemClock(CTime tmServer);

	bool IsInternetConnect();

	//宽字节转多字节
	char* CStringToMutilChar(CString& str,int& chLength,WORD wPage=CP_ACP);
private:
	bool m_bCheckFlag;  //记录检查是否成功
	bool m_bRepairFlag; //记录修复是否成功 
	bool m_bNetConnect; //判断网络状态是否连接
	bool m_bIsGetNetTime; //判断是否获取到了网络时间
};