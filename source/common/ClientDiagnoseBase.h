#pragma once
#include <vector>

const TCHAR* const ProblemCheckDllSuggest = _T("您的电脑缺少%s建议您点击一键修复！");
const TCHAR* const ProblemCheckSvrConSuggest = _T("您的电脑无法连接%s建议您检查本地网络连接是否正常！");
const TCHAR* const ProblemCheckDiffTimeSuggest = _T("您的电脑时间与北京时间相差十分钟以上，建议您点击一键修复！");
const TCHAR* const ProblemCheckDiffTimeFault = _T("获取北京时间失败，请重新检测！");
const TCHAR* const ProblemCheckNewVersionSuggest = _T("您的客户端不是最新版本，建议您点击客户端检测升级！");
const TCHAR* const ProblemCheckNetConnect = _T("网络连接超时，请检查您的网络或者本机防火墙设置！");
const TCHAR* const RepairCheckDiffTimeSuggest = _T("请您以管理员身份手动修改或以管理员身份运行客户端点击修复！");
const TCHAR* const ProblemCheckWebActiveX = _T("您的电脑缺失web插件，建议您点击一键修复，修复成功后重启浏览器！");

struct T_PROBLEM_DATA;

//抽象接口
class CDiagnoseBase
{
public:
	virtual ~CDiagnoseBase(){}

	//问题检测
	virtual BOOL CheckPro() = 0;  
	//问题修复
	virtual BOOL RepairPro() = 0;
	//获取要返回的检测故障的结构体
	virtual void GetBackCheckStruct(T_PROBLEM_DATA &tData) = 0;
    //获取要返回的修复故障的结构体
	virtual void GetBackRepairStruct(T_PROBLEM_DATA &tData) = 0;
};

//接口调用类
class CDiagnoseProblem
{

public:  
	CDiagnoseProblem (CDiagnoseBase *pBase) 
	{ 
		m_pBase = pBase; 
	}  
	~CDiagnoseProblem() 
	{ 
		if (NULL != m_pBase)
		{
			delete m_pBase;
			m_pBase = NULL;
		}
	}  

public:
	BOOL CheckPro()  { return(m_pBase->CheckPro());  } 
	BOOL RepairPro() { return (m_pBase->RepairPro()); } ;
	void GetBackCheckStruct(T_PROBLEM_DATA &tData){return (m_pBase->GetBackCheckStruct(tData)); } ;
	void GetBackRepairStruct(T_PROBLEM_DATA &tData){return (m_pBase->GetBackRepairStruct(tData)); } ;

private:  
	CDiagnoseBase *m_pBase; 
};


struct T_PROBLEM_DATA
{
	CString strIndex;         //问题序号
	CString strPromble;       //问题描述
	CString strSuggestion;    //问题建议
	bool bIsRepair;           //是否需要修复
	bool bCheckFlag;          //检测结果(检测的结果是否成功)
	bool bRepairFlag;         //修复结果（修复的结果是否成功）

	T_PROBLEM_DATA(void)
	{
		strIndex = _T("");
		strPromble = _T("");
		strSuggestion = _T("");
		bIsRepair = false;
		bCheckFlag = false;
		bRepairFlag = false;
	}

	T_PROBLEM_DATA &operator =(const T_PROBLEM_DATA &tData)
	{
		if (this == &tData)
			return *this;

		strIndex = tData.strIndex;
		strPromble = tData.strPromble;
		strSuggestion = tData.strSuggestion;
		bIsRepair = tData.bIsRepair;
		bCheckFlag = tData.bCheckFlag;
		bRepairFlag = tData.bRepairFlag;
	}

	bool operator ==(T_PROBLEM_DATA &tData) const
	{
		return (strIndex == tData.strIndex && strPromble == tData.strPromble 
			&& strSuggestion == tData.strSuggestion &&
			bIsRepair == tData.bIsRepair && bCheckFlag == tData.bCheckFlag &&
			bRepairFlag == tData.bRepairFlag);
	}
};

//从xml读出来的数据
struct T_DATA_FROM_XML
{
	std::vector<T_PROBLEM_DATA> vProblem;       //问题列表
};


struct T_CHECK_SYSTEM_DLL
{
	CString strOsName;              //操作系统版本
	int nSystemBits;          //操作系统位数
	std::vector<CString> vLackDllNames;  //缺失的dll

	T_CHECK_SYSTEM_DLL(void)
	{
		strOsName = _T("");
		nSystemBits = 0;
		vLackDllNames.clear();
	}
};

struct T_CHECK_SVR_CONNECT
{
	bool bQuickPhotoSvr;
	bool bApiSvr;
	bool bBaiDuSvr;
	bool bSeoSvr;
	bool bNetConnect;

	T_CHECK_SVR_CONNECT(void)
	{
		bQuickPhotoSvr = false;
		bApiSvr = false;
		bBaiDuSvr = false;
		bSeoSvr = false;
		bNetConnect = false;
	}
};


struct T_DLL_PATH_INFO
{
	CString strDllName;      //dll的名称  
	CString strMd5;          //文件的md5值
	CString strSysMatchFlag;  //获取文件时除了文件名匹配，是否还需要系统版本和位数匹配
	CString strSysVersion;   //系统版本
	CString strSysBits;      //系统位数
	CString strLibPath;      //文件所在的路径
	CString strRunFlag;      //该文件是否运行
	CString strParam;        //运行参数

	T_DLL_PATH_INFO(void)
	{
		strDllName = _T("");  
		strMd5 = _T("");
		strSysMatchFlag = _T("false");
		strSysVersion = _T("");      
		strSysBits = _T("");    
		strLibPath = _T("");
		strRunFlag = _T("");
		strParam = _T("");
	}

    T_DLL_PATH_INFO &operator =(const T_DLL_PATH_INFO &tData)
	{
		if (this == &tData)
			return *this;

		strDllName = tData.strDllName;
		strMd5 = tData.strMd5;
		strSysMatchFlag = tData.strSysMatchFlag;
		strSysVersion = tData.strSysVersion;
		strSysBits = tData.strSysBits;
		strLibPath = tData.strLibPath;
		strRunFlag = tData.strRunFlag;
		strParam = tData.strParam;
	}
};