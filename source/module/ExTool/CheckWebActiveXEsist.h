#pragma once;
#include "..\..\common\FTP.h"

class CCheckWebActiveXExist:public CDiagnoseBase
{
public:
	CCheckWebActiveXExist(void);
	virtual ~CCheckWebActiveXExist(void);
public:
	virtual BOOL CheckPro(void);
	virtual BOOL RepairPro(void);
	virtual void GetBackCheckStruct(T_PROBLEM_DATA &tData);
	virtual void GetBackRepairStruct(T_PROBLEM_DATA &tData);

public:
	//检测网络是否连接
	bool IsInternetConnect();

	//用ftp从服务器上下载pe文件
	bool DownloadDllFile();

	//读取mcconfig配置文件获取ftp的登录信息
	void GetFtpLoginInfo(const CString &strIniPath);

	void GetFtpDesCode(TCHAR *code, TCHAR *ip, TCHAR *account, TCHAR *psd, int &port);

	//用ftp从服务下载mc.dat文件
	bool DownLoadDatFile();

	//注册下载下来的dll文件
	bool RegistDll();

	//将简写目录转换为实际的目录
	CString GetDirectory(CString strShortDir);

private:
	bool m_bCheckFlag;               //检测的结果是否成功
	T_CONN_INFO m_ftpLoginInfo;      //ftp登录信息的结构体
	vector<CString> m_vRegDll;       //记录需要注册的dll
	bool m_bRepairSucc;              //修复成功
	short int m_iCheckcnt;			//检查次数
};