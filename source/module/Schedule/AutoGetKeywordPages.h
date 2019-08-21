#pragma once

#include "resource.h"
#include "..\..\common\tinyxml.h"
//#include "ScheduleThread.h"
#include "Schedulemgr.h"

#define ERRCODE_SUCCESS   0
#define ERRCODE_FAIL      1
#define ERRCODE_NO_TASK   2

class CScheduleThread;


class CAutoGetKeywordPages
{
public:
	CAutoGetKeywordPages(CScheduleThread *pScheduleThread);
	~CAutoGetKeywordPages(void);

public:
	//初始化
	bool init(void);

	//brief:开始执行抓关键词排名
	bool StartExec(CString strUserName, int strVesionId, int nRefreshFlag);  

	//选择服务器
	CStdString ChooseHttpServer(CStdString strSer1, CStdString strSer2, CStdString strFastSer);

	//选择两个服务器中比较快的那一个
	CString ChooseBestServer(CStdString strSer1, CStdString strSer2);

	//获取用户对应产品下的自动任务
	void RequestSchedule(void);

	//按时请求服务器获取自动任务
	void RequestScheduleForTime(void);

	//准备自动任务
	bool PrepareSchedule(T_DATA_FROM_SERVER &tData);

	//从服务器请求自动任务数据
	int GetScheduleData(CString strUseAccount, CString strVersionId, T_DATA_FROM_SERVER &tData);

	//分解从服务端请求到的自动任务数据
	bool ParseServerResponse(TiXmlDocument *pDoc, T_DATA_FROM_SERVER &tScheduleInfo,  CString strVersionId);

	//轮循自动任务
	void PollingSchedule(void);

	//解密获取的用户名
	CString DecodeString( CString& strDest);

	//Rul编码
	CString  URLEncode(CString sIn);

	//宽字节转多字节
	char* CStringToMutilChar(CString& str,int& chLength);

	//宽字节转utf_8
	char* CStringToUtf8Char(CString& str,int& chLength);

	//创建自动任务定时器
	int CreateScheduleTimer(LONGLONG dwTime);

	void StopTask(void);

	//清除快照
	bool RemoveQuickPhoto(const CString &strUser, const CString &strVerId);


public:
	bool m_bAlreadySendDataToProc;  //已经将消息发给主控
	CStdString m_strFastServer;

private:
	static DWORD WINAPI ThreadProcPollingTask(LPVOID lpParameter);
	bool m_bPollingThreadStop;

	HANDLE m_hTimer;

	T_DATA_FROM_SERVER m_tScheduleInfo;
	CScheduleThread *m_pScheduleThread;
	CScheduleMgr *m_objScheduleMgr;
	HANDLE m_hThreadTaskPolling;
	CString m_strUserName;           //保存未解密的当前客户端用户名
	CString m_strUseAccount;         //保存解密的当前客户端的用户名
	CString m_strVersinId;
	int m_nRefreshFlag;


};
