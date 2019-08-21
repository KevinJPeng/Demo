/***************************************************************
系统垃圾清理模块实现
***************************************************************/

#ifndef _SYS_OPERATE_H_
#define _SYS_OPERATE_H_
#include "..\..\threadmodel\ithreadunit.h"

#include "..\..\common\tinystr.h"
#include "..\..\common\tinyxml.h"
#include "CheckWebActiveXEsist.h"
#pragma comment(lib, "..\\..\\lib\\tinyxml.lib")

const int nCheckCount = 5;
const int nRepairCount = 3;

typedef struct _tRubishSize
{
	DWORD dwRecycleSize;


}_TRUBISHSIZE,*PRUBSIHSIZE;


class CSysOperate : public IThreadUnit
{
public:
	CSysOperate(void);
	~CSysOperate(void);
	//线程消息处理
	DWORD DispatchMessage(T_Message *pMsg);
public:
	//处理自检故障消息
	void OnSelfDiagnosis();

	//处理自检故障消息
	bool OnReadSelfDiagnosisXml(void);

	//解析ProblemList.xml中的数据
	bool ParseXmlInfo(TiXmlDocument *pDoc, T_DATA_FROM_XML &tXmlInfo);

	//检测线程是否还在运行
	bool CheckThreadRun(HANDLE thread);

	//处理修复故障消息
	void OnRepairFault(WPARAM nParam);
	
	//检测注册表的值是否发生变化----20150312
	void CheckRegValue();
private:
	HANDLE m_threadCheck[nCheckCount];
	HANDLE m_threadRepair[nRepairCount];
	BOOL m_bDiagnosisCancel;            //控制故障自检取消的变量
	T_Message *m_tMsg;
	CString m_strCurrentUserName;		//当前注册表中的用户名
	CString m_strSupportVersion;		//当前注册表版本号
	CString m_strlastWriteTime;			//记录最后一次上层调用web控件写版本及用户名的时间
	T_DATA_FROM_XML m_ProblemListInfo;  //问题列表中的内容
	WPARAM m_nNewVersion;               //记录客户端是否是最新版本号
	T_CHECK_SYSTEM_DLL m_tData;         //保留CheckSystemDll检查的数据
	CCheckWebActiveXExist m_CheckObj;	//检测插件是否存在接口
private:		
	void PackagMessage(DWORD dwDestThread,DWORD dwSourceThread,DWORD dwMessageType,WPARAM wParam, LPARAM lParam, bool bSync = false);
	//add by zhumingxing 20150312----获取舟大师注册表中当前用户名
	TCHAR* GetRegCurrentUserName();

private:
	static DWORD WINAPI ThreadCheckReg(LPVOID lpParameter);
	static DWORD WINAPI ThreadCheckSystemDll(LPVOID lpParameter);
	static DWORD WINAPI ThreadCheckSvrConnect(LPVOID lpParameter);
	static DWORD WINAPI ThreadCheckDiffTime(LPVOID lpParameter);
	static DWORD WINAPI ThreadCheckNewVersion(LPVOID lpParameter);
	static DWORD WINAPI ThreadCheckWebActiveXExist(LPVOID lpParameter);
	static DWORD WINAPI ThreadRepairSystemDll(LPVOID lpParameter);
	static DWORD WINAPI ThreadRepairDiffTime(LPVOID lpParameter);
	static DWORD WINAPI ThreadRepairWebActiveXExist(LPVOID lpParameter);
	static DWORD WINAPI ThreadCheckAndRepairplugins(LPVOID lpParameter);

};

#endif