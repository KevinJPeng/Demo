#pragma once;

#include "InternetHttp.h"
#include "FTP.h"

class CCheckSvrConnect:public CDiagnoseBase
{
public:
	CCheckSvrConnect(void);
	virtual ~CCheckSvrConnect(void);
public:
	virtual BOOL CheckPro(void);
	virtual BOOL RepairPro(void);
	virtual void GetBackCheckStruct(T_PROBLEM_DATA &tData);
	virtual void GetBackRepairStruct(T_PROBLEM_DATA &tData);

	//检测快照服务器连接是否正常
	bool CheckQuickPhotoSvr(void);

	//检测api服务器连接是否正常
	bool CheckApiSvr(void);

	//检测百度服务器
	bool CheckBaiDuSvr(void);

	//检测SEO服务器
	bool CheckSeoSvr(void);

	//读取mcconfig配置文件获取ftp的登录信息
	void GetFtpLoginInfo(const CString &strIniPath);

	void GetFtpDesCode(TCHAR *code, TCHAR *ip, TCHAR *account, TCHAR *psd, int &port);

	//从配置文件中获取URL
	bool GetURL(CString& strURL);

	//检测网络是否连接
	bool IsInternetConnect();

private:
	T_CONN_INFO m_ftpLoginInfo;          //ftp登录信息的结构体
	T_CHECK_SVR_CONNECT m_svrConnect;    //服务器连接结构体
};