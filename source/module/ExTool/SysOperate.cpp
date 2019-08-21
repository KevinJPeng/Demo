#include "stdafx.h"
#include "SysOperate.h"
#include <Dbghelp.h> 
#include <algorithm>

#include "SysOperate.h"
#include "CheckSystemDll.h"
#include "CheckSvrConnect.h"
#include "CheckDiffTime.h"
#include "CheckNewVersion.h"
#include "CheckWebActiveXEsist.h"


#pragma comment(lib,  "Dbghelp.lib")

CRITICAL_SECTION g_cs;
//HMODULE g_hMod;

CSysOperate::CSysOperate(void) : IThreadUnit(E_THREAD_CLEAR,0xFFFF)
{
	m_bDiagnosisCancel = false;
	InitializeCriticalSection(&g_cs);
	//g_hMod = LoadLibrary(_T("RunDetours.dll"));
}

CSysOperate::~CSysOperate()
{
	DeleteCriticalSection(&g_cs);
}


DWORD CSysOperate::DispatchMessage(T_Message *pMsg)
{	
	switch (pMsg->dwMsg)
	{	
	//新增后台循环检测插件是否丢失操作,如果有丢失则进行修复
	case MSG_CHECK_PLUGINS:
		{
		    CreateThread(NULL, 0, &CSysOperate::ThreadCheckAndRepairplugins, this, 0, NULL);
		}
		break;
	case MSG_CHECK_REG:
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到了检测注册表变化消息!"));
			CreateThread(NULL, 0, &CSysOperate::ThreadCheckReg, this, 0, NULL );
		}
		break;

		//接收到故障检测和修复的取消消息
	case MSG_DIAGNOSIS_CANCEL:
		{
			m_bDiagnosisCancel = true;
		}
		break;

		//接收到故障自检消息
	case MSG_SELF_DIAGNOSIS:
		{	
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到了故障自检消息!"));
			m_nNewVersion = pMsg->wParam;
			m_bDiagnosisCancel = false;

			OnSelfDiagnosis();
		}
		break;

		//接收到读取故障自检的xml配置文件消息
	case MSG_READ_SELF_DIAGNOSIS_XML:
		{	
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到了读取故障自检的xml配置文件消息!"));

			OnReadSelfDiagnosisXml();
			PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_CLEAR, MSG_READ_SELF_DIAGNOSIS_XML, (WPARAM)&m_ProblemListInfo, 0);
		}
		break;

		//接收修复故障消息
	case MSG_REPAIR_FAULT:
		{
			m_bDiagnosisCancel = false;

			OnRepairFault(pMsg->wParam);
		}
		break;

	case MSG_SALF_EXIT:
		{	
			//FreeLibrary(g_hMod);
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("安全结束了当前清理！"));
		}
		break;
	case MSG_FREE_HOOK_DLL:
		{
			//FreeLibrary(g_hMod);
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("客户端重启时卸载全局钩子消息!"));
		}
		break;
	default:
		break;
	}
	return 0;

}


bool CSysOperate::OnReadSelfDiagnosisXml(void)
{
	//读出字符串
	CStdioFile file;
	CString strXmlPath = _T("");
	strXmlPath.Format(_T("%s\\bin\\skin\\ProblemList.xml"), g_pGlobalData->dir.GetInstallDir());

	if (!file.Open(strXmlPath, CStdioFile::modeReadWrite | CStdioFile::typeBinary))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("无法打开ProblemList.xml文件，err：%d！"), GetLastError());
		return FALSE;
	}
	

	int iMaxXMLen = 256 * 1024;       //256k
	char *ptmpBuf = NULL;
	
	try
	{
		int nLen = file.GetLength();
		ptmpBuf = new char[nLen + 1];
		memset(ptmpBuf, 0, nLen + 1);
		file.Read(ptmpBuf, nLen);

		file.Close();

		TiXmlDocument doc;
		doc.Parse(ptmpBuf);

		delete []ptmpBuf;
		ptmpBuf = NULL;

		if (!ParseXmlInfo(&doc, m_ProblemListInfo))
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("解析ProblemList.xml文件中的数据失败！"));
			return false;
		}
	}
	catch (...)
	{
		if (!ptmpBuf)
		{
			delete []ptmpBuf;
			ptmpBuf = NULL;
		}

		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("解析ProblemList.xml文件中的数据异常！"));
		return false;
	}

	return true;
}

void CSysOperate::OnSelfDiagnosis()
{
	for (int i = 0; i != m_ProblemListInfo.vProblem.size(); ++i)
	{
		if (m_ProblemListInfo.vProblem[i].strIndex == _T("1"))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测系统依赖dll是否存在！"));

			if (!CheckThreadRun(m_threadCheck[0]))
			{
				continue;
			}

			m_threadCheck[0] = CreateThread(NULL, 0, &CSysOperate::ThreadCheckSystemDll, this, 0, NULL );
		}
		/*********检测服务器不稳定，暂时取消**********/
 		/*else if (m_ProblemListInfo.vProblem[i].strIndex == _T("2"))
 		{
 			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测服务器连接是否正常（快照服务器、API服务器、百度、SEO综合查询）"));
 
 			if (!CheckThreadRun(m_threadCheck[1]))
 			{
 				continue;
 			}
 
 			m_threadCheck[1] = CreateThread(NULL, 0, &CSysOperate::ThreadCheckSvrConnect, this, 0, NULL );
 		}*/
		else if (m_ProblemListInfo.vProblem[i].strIndex == _T("2"))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测客户端web控件是否存在！"));

			if (!CheckThreadRun(m_threadCheck[1]))
			{
				continue;
			}

			m_threadCheck[1] = CreateThread(NULL, 0, &CSysOperate::ThreadCheckWebActiveXExist, this, 0, NULL );
		}
		else if (m_ProblemListInfo.vProblem[i].strIndex == _T("3"))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测客户端时间（与标准时间差超过10分钟提示）"));

			if (!CheckThreadRun(m_threadCheck[2]))
			{
				continue;
			}

			m_threadCheck[2] = CreateThread(NULL, 0, &CSysOperate::ThreadCheckDiffTime, this, 0, NULL );
		}
		else if (m_ProblemListInfo.vProblem[i].strIndex == _T("4"))
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测客户端是否是新版本！"));

			if (!CheckThreadRun(m_threadCheck[3]))
			{
				continue;
			}

			m_threadCheck[3] = CreateThread(NULL, 0, &CSysOperate::ThreadCheckNewVersion, this, 0, NULL );
		}
		else
		{
			break;
		}
	}	
}


void CSysOperate::OnRepairFault(WPARAM nParam)
{
	switch (nParam)
	{
	case 1:
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("修复系统缺少的dll！"));

			if (!CheckThreadRun(m_threadRepair[0]))
			{
				return;
			}

			m_threadRepair[0] = CreateThread(NULL, 0, &CSysOperate::ThreadRepairSystemDll, this, 0, NULL );
		}
		break;
	case 2:
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("修复web控件缺失"));

			if (!CheckThreadRun(m_threadRepair[2]))
			{
				return;
			}

			m_threadRepair[2] = CreateThread(NULL, 0, &CSysOperate::ThreadRepairWebActiveXExist, this, 0, NULL );
		}
		break;
	case 3:
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("修复客户端时间差"));

			if (!CheckThreadRun(m_threadRepair[1]))
			{
				return;
			}

			m_threadRepair[1] = CreateThread(NULL, 0, &CSysOperate::ThreadRepairDiffTime, this, 0, NULL );
		}
		break;

	default:
		break;
	}
}


bool CSysOperate::CheckThreadRun(HANDLE thread)
{
	DWORD dwExitCode; 

	GetExitCodeThread(thread, &dwExitCode);
	//表示当前线程正在运行
	if (dwExitCode == STILL_ACTIVE)
	{	
		return false;
	}

	return true;
}

DWORD WINAPI CSysOperate::ThreadCheckReg( LPVOID lpParameter )
{
	CSysOperate* pThis = (CSysOperate*)lpParameter;
	pThis->CheckRegValue();

	return 1;
}

//系统dll检测
DWORD WINAPI CSysOperate::ThreadCheckSystemDll(LPVOID lpParameter)
{
	CSysOperate* pThis = (CSysOperate*)lpParameter;

	T_PROBLEM_DATA tCheckDllData;
	CDiagnoseBase *pCheckSystemDll = new CCheckSystemDll();
	CDiagnoseProblem systemDllProblem(pCheckSystemDll);
	systemDllProblem.CheckPro();
	systemDllProblem.GetBackCheckStruct(tCheckDllData);

	//保存检查dll后的结构体数据
	CCheckSystemDll* pSystemDllStruct = (CCheckSystemDll*)pCheckSystemDll;
	pSystemDllStruct->GetCheckData(pThis->m_tData);

	if (pThis->m_bDiagnosisCancel)
	{
		return 1;
	}

	EnterCriticalSection(&g_cs);
	pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_CLEAR, MSG_SELF_DIAGNOSIS, (WPARAM)&tCheckDllData, 0, true);
	LeaveCriticalSection(&g_cs);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测系统依赖dll是否存在已完成！"));

	return 1;
}

//系统dll缺失修复
DWORD WINAPI CSysOperate::ThreadRepairSystemDll(LPVOID lpParameter)
{
	CSysOperate* pThis = (CSysOperate*)lpParameter;

	T_PROBLEM_DATA tRepairDllData;
	CDiagnoseBase *pCheckSystemDll = new CCheckSystemDll(pThis->m_tData);
	CDiagnoseProblem systemDllProblem(pCheckSystemDll);
	systemDllProblem.RepairPro();
	systemDllProblem.GetBackRepairStruct(tRepairDllData);

	if (pThis->m_bDiagnosisCancel)
	{
		return 1;
	}

	EnterCriticalSection(&g_cs);
	pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_CLEAR, MSG_REPAIR_FAULT, (WPARAM)&tRepairDllData, 0, true);
	LeaveCriticalSection(&g_cs);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("修复系统缺少的dll已完成！"));

	return 1;
}

//本地时间与网络时间对比检测
DWORD WINAPI CSysOperate::ThreadCheckDiffTime(LPVOID lpParameter)
{
	CSysOperate* pThis = (CSysOperate*)lpParameter;

	T_PROBLEM_DATA tCheckDiffTimeData;
	CDiagnoseBase *pCheckDiffTime = new CCheckDiffTime;
	CDiagnoseProblem diffTimeProblem(pCheckDiffTime);
	diffTimeProblem.CheckPro();
	diffTimeProblem.GetBackCheckStruct(tCheckDiffTimeData);

	if (pThis->m_bDiagnosisCancel)
	{
		return 1;
	}

	EnterCriticalSection(&g_cs);
	pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_CLEAR, MSG_SELF_DIAGNOSIS, (WPARAM)&tCheckDiffTimeData, 0, true);
	LeaveCriticalSection(&g_cs);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测客户端时间已完成！"));

	return 1;
}


//本地时间与网络时间对比修复
DWORD WINAPI CSysOperate::ThreadRepairDiffTime(LPVOID lpParameter)
{
	CSysOperate* pThis = (CSysOperate*)lpParameter;

	T_PROBLEM_DATA tRepairDiffTimeData;
	CDiagnoseBase *pCheckDiffTime = new CCheckDiffTime;
	CDiagnoseProblem diffTimeProblem(pCheckDiffTime);
	diffTimeProblem.RepairPro();
	diffTimeProblem.GetBackRepairStruct(tRepairDiffTimeData);

	if (pThis->m_bDiagnosisCancel)
	{
		return 1;
	}

	EnterCriticalSection(&g_cs);
	pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_CLEAR, MSG_REPAIR_FAULT, (WPARAM)&tRepairDiffTimeData, 0, true);
	LeaveCriticalSection(&g_cs);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("修复客户端时间差已完成！"));

	return 1;
}

//客户端是否是最新版本检测
DWORD WINAPI CSysOperate::ThreadCheckNewVersion(LPVOID lpParameter)
{	
	CSysOperate* pThis = (CSysOperate*)lpParameter;

	T_PROBLEM_DATA tCheckNewVerData;
	CDiagnoseBase *pCheckNewVersion = new CCheckNewVersion(pThis->m_nNewVersion);
	CDiagnoseProblem newVersionProblem(pCheckNewVersion);

	newVersionProblem.GetBackCheckStruct(tCheckNewVerData);

	if (pThis->m_bDiagnosisCancel)
	{
		return 1;
	}

	EnterCriticalSection(&g_cs);
	pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_CLEAR, MSG_SELF_DIAGNOSIS, (WPARAM)&tCheckNewVerData, 0, true);
	LeaveCriticalSection(&g_cs);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测客户端是否是新版本已完成！"));

	return 1;
}

//客户端web控件是否存在
DWORD WINAPI CSysOperate::ThreadCheckWebActiveXExist(LPVOID lpParameter)
{	
	CSysOperate* pThis = (CSysOperate*)lpParameter;

	T_PROBLEM_DATA tCheckWebActiveXEsistData;
	CDiagnoseBase *pCheckWebActiveXEsist = new CCheckWebActiveXExist();
	CDiagnoseProblem webActiveXEsistProblem(pCheckWebActiveXEsist);

	webActiveXEsistProblem.CheckPro();
	webActiveXEsistProblem.GetBackCheckStruct(tCheckWebActiveXEsistData);

	if (pThis->m_bDiagnosisCancel)
	{
		return 1;
	}

	EnterCriticalSection(&g_cs);
	pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_CLEAR, MSG_SELF_DIAGNOSIS, (WPARAM)&tCheckWebActiveXEsistData, 0, true);
	LeaveCriticalSection(&g_cs);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("检测客户端web控件是否存在已完成！"));

	return 1;
}

//缺少web控件修复
DWORD WINAPI CSysOperate::ThreadRepairWebActiveXExist(LPVOID lpParameter)
{
	CSysOperate* pThis = (CSysOperate*)lpParameter;

	T_PROBLEM_DATA tRepairWebActiveXEsistData;
	CDiagnoseBase *pCheckWebActiveXEsist = new CCheckWebActiveXExist();
	CDiagnoseProblem webActiveXEsistProblem(pCheckWebActiveXEsist);

	webActiveXEsistProblem.RepairPro();
	webActiveXEsistProblem.GetBackRepairStruct(tRepairWebActiveXEsistData);

	if (pThis->m_bDiagnosisCancel)
	{
		return 1;
	}

	EnterCriticalSection(&g_cs);
	pThis->PackagMessage(E_THREAD_TYPE_UIHELPER, E_THREAD_CLEAR, MSG_REPAIR_FAULT, (WPARAM)&tRepairWebActiveXEsistData, 0, true);
	LeaveCriticalSection(&g_cs);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("修复web控件缺失已完成！"));

	return 1;
}

DWORD WINAPI CSysOperate::ThreadCheckAndRepairplugins(LPVOID lpParameter)
{	
	CSysOperate* pThis = (CSysOperate*)lpParameter;

	while (true)
	{
		pThis->m_CheckObj.RepairPro();

		//每10s钟检测一次
		Sleep(10000);
	}
}

void CSysOperate::CheckRegValue()
{	
	CReg reg;

	//获取最后修改时间，默认为空
	TCHAR*pLastWriteTime = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),_T("timestamp"));
	m_strlastWriteTime = pLastWriteTime;

	//获取版本号及用户名
	TCHAR* pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),_T("SupportProduct"));
	m_strSupportVersion = pContent;
	m_strCurrentUserName = GetRegCurrentUserName();

	HANDLE hNotify;
	HKEY hKeyx;

	hNotify = CreateEvent(NULL,FALSE, TRUE,_T("RegistryNotify"));
	
	if (hNotify == 0)
	{ 
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("CreateEvent 失败:错误码:%d"),GetLastError());
		return;
	} 
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),0,KEY_NOTIFY,&hKeyx) != ERROR_SUCCESS)
	{	
		CloseHandle(hNotify);
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("打开注册表Software\\szw\\MasterZ失败:错误码:%d"),GetLastError());
		return;
	}

	//循环检测注册表变化
	while(TRUE)
	{
		if (RegNotifyChangeKeyValue(hKeyx,TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, 
			hNotify,	
			TRUE) != ERROR_SUCCESS)
		{
			CloseHandle(hNotify);
			RegCloseKey(hKeyx);
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("RegNotifyChangeKeyValue失败:错误码:%d"),GetLastError());
			return;
		}
		if (WaitForSingleObject(hNotify, INFINITE) != WAIT_FAILED)
		{	
			CString strSupportVersion = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),_T("SupportProduct"));
			CString strNewUserName = GetRegCurrentUserName();
			CString strLastWriteTime = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),_T("timestamp"));

			if (strSupportVersion.Compare(m_strSupportVersion))
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("检测到注册表当前支持版本发生变化!OldSupportVersion:%s###NewSupportVersion:%s"),m_strSupportVersion,strSupportVersion);
				
				//向主界面发送消息
				PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_CLEAR,MSG_CHECK_REG,0,0);
				m_strSupportVersion = strSupportVersion;
				m_strCurrentUserName = strNewUserName;
				m_strlastWriteTime = strLastWriteTime;
				SetEvent(hNotify);
				continue;
			}
			if (strNewUserName.Compare(m_strCurrentUserName) )
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("检测到注册表当前用户名发生了变化!OldUserName:%s###NewUserName:%s"),m_strCurrentUserName,strNewUserName);

				//向主界面发送消息
				PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_CLEAR,MSG_CHECK_REG,0,0);
				m_strSupportVersion = strSupportVersion;
				m_strCurrentUserName = strNewUserName;
				m_strlastWriteTime = strLastWriteTime;
				SetEvent(hNotify);
				continue;
			}

			//新增对最后修改时间的判断----如果不一样的情况
			if (strLastWriteTime.CompareNoCase(m_strlastWriteTime))
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("检测到注册表用户名最后写入时间发生变化!Oldtimestamp:%s###Newtimestamp:%s"),m_strlastWriteTime,strLastWriteTime);

				//向主界面发送消息
				PackagMessage(E_THREAD_TYPE_UIHELPER,E_THREAD_CLEAR,MSG_CHECK_REG,0,0);
				m_strSupportVersion = strSupportVersion;
				m_strCurrentUserName = strNewUserName;
				m_strlastWriteTime = strLastWriteTime;
				SetEvent(hNotify);
				continue;
			}

			SetEvent(hNotify);
		} 
	}

}

TCHAR* CSysOperate::GetRegCurrentUserName()
{
	CReg reg;
	TCHAR* pContent = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),_T("SupportProduct"));
	if (pContent == NULL)
	{	
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("注册表Software\\szw\\MasterZ\\SupportProduct版本支持为空！"));
		return NULL;
	}
	else
	{	
		TCHAR* pUserName = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER,_T("Software\\szw\\MasterZ"),pContent);

		if (pUserName == NULL)
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("注册表Software\\szw\\MasterZ\\%s用户名为空!"),pContent);
			return NULL;
		}
		else
		{
			return pUserName;
		}
	}
}

void CSysOperate::PackagMessage(DWORD dwDestThread,DWORD dwSourceThread,DWORD dwMessageType,WPARAM wParam, LPARAM lParam, bool bSync)
{
	m_tMsg = IMsgQueue::New_Message();
	m_tMsg->dwDestWork = dwDestThread;
	m_tMsg->dwSourWork = dwSourceThread;
	m_tMsg->dwMsg = dwMessageType;	//发送初始化消息
	m_tMsg->wParam = wParam;
	m_tMsg->lParam = lParam;
	if (bSync)
	{
		SendMessage(m_tMsg);
	}
	else
	{
		PostMessage(m_tMsg);
	}	
}


//解析ProblemList.xml中的数据
bool CSysOperate::ParseXmlInfo(TiXmlDocument *pDoc, T_DATA_FROM_XML &tXmlInfo)
{
	tXmlInfo.vProblem.clear();

	if (!pDoc)
	{
		return false;
	}

	TiXmlElement *pRoot = pDoc->FirstChildElement("root");

	if (!pRoot)
	{
		return false;
	}

	TiXmlElement *pItem = pRoot->FirstChildElement("item");

	if (!pItem)
	{
		return false;
	}

	//遍历读取命令列表
	for (; pItem; pItem = pItem->NextSiblingElement())
	{
		T_PROBLEM_DATA tData;
		tData.strIndex = pItem->Attribute("index");
		tData.strPromble = pItem->Attribute("problem");

		tXmlInfo.vProblem.push_back(tData);
	}

	return true;
}

