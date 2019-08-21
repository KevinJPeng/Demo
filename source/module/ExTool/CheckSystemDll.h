#pragma once;

#include "..\..\common\FTP.h"

#include "..\..\common\tinystr.h"
#include "..\..\common\tinyxml.h"

#pragma comment(lib, "..\\..\\lib\\tinyxml.lib")

class CCheckSystemDll:public CDiagnoseBase
{
public:
	CCheckSystemDll();
	CCheckSystemDll(T_CHECK_SYSTEM_DLL &tData);
	virtual ~CCheckSystemDll(void);
public:
	virtual BOOL CheckPro(void);
	virtual BOOL RepairPro(void);
	virtual void GetBackCheckStruct(T_PROBLEM_DATA &tData);
	virtual void GetBackRepairStruct(T_PROBLEM_DATA &tData);

	//获取得到的结果
	void GetCheckData(T_CHECK_SYSTEM_DLL &tData);

	//遍历指定目录获取pe文件名称
	void GetPeName(TCHAR* strDirName, vector <CString> &vecPeName);

	//获取依赖的dll
	bool GetDependsDll(CString strPePath, vector <CString> &vecDllName);

	//获取操作系统版本号
	void getOsVersion(CString &strOSName);

	// 安全的取得真实系统信息
	VOID SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo);

	//获取操作系统位数
	int GetSystemBits(void);

	//读取mcconfig配置文件获取ftp的登录信息
	void GetFtpLoginInfo(const CString &strIniPath);

	void GetFtpDesCode(TCHAR *code, TCHAR *ip, TCHAR *account, TCHAR *psd, int &port);

	//检测网络是否连接
	bool IsInternetConnect();

	//用ftp从服务器上下载xml文件
	bool DownloadXmlFile(void);

	//用ftp从服务器上下载pe文件
	bool DownloadPeFile(const CString &strPePath, CString &strLocalPath);

	//用ftp上传空文件到服务器
	bool UploadPeFile(const CString &strPeName, const CString &strPePath);

	//读从服务器上下载下来的xml文件
	bool ReadRepairXml(void);

	//解析xml
	bool ParseXmlInfo(TiXmlDocument *pDoc, vector<T_DLL_PATH_INFO> &tXmlInfo);

	//查找所需dll
	void FindDllInfo(void);

	//重新加载看是否修复成功
	bool ReloadDll(void);

    //启动下载下来的应用程序
	bool RunExe(const CString &strExePath, const CString &strCommandLine);

	//注册下载下来的dll文件
	bool RegistDll(const CString &strDllPath, const CString &strCommandLine);

	//获取文件的md5值
	BOOL GetFileMD5(const CString &strFile, CString &strMd5);

	//校验下载下来的文件的md5值是否正确
	bool CompareFileMd5(const CString &strLocalPePath, CString &strMd5FromXml);

	//将简写目录转换为实际的目录
	CString GetDirectory(CString strShortDir);							
private:
	T_CHECK_SYSTEM_DLL m_tData;
	T_CONN_INFO m_ftpLoginInfo;          //ftp登录信息的结构体
	vector<T_DLL_PATH_INFO> m_dllInfo; //获取所需动态链接库路径等信息
	vector<T_DLL_PATH_INFO> m_tDllFinded;   //记录在服务器端已找到的dll
	vector<CString> m_vRecordUnfindedDll;   //记录未找到的dll
	bool m_bLackDll;   //判断是否缺少dll
	bool m_bRepairSucc; //修复成功
};