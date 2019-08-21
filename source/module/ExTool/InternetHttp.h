﻿#pragma once

#include <afxinet.h>
#include <string>

using namespace std;

// 操作成功
#define SUCCESS        0

// 操作失败
#define FAILURE        1

// 操作超时
#define OUTTIME        2

#define  BUFFER_SIZE       1024
#define  NORMAL_CONNECT             INTERNET_FLAG_KEEP_CONNECTION
#define  SECURE_CONNECT             NORMAL_CONNECT | INTERNET_FLAG_SECURE
#define  NORMAL_REQUEST             INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE 
#define  SECURE_REQUEST             NORMAL_REQUEST | INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID

#define  IE_AGENT  _T("BIZEXPRESS_LIVEUPDATE_")

//服务器配置
struct T_LOGIN_INFO
{
	CString strServer;
	CString strUploadUrl;
	CString strUser;
	CString strPwd;

	T_LOGIN_INFO(void)
	{
		strServer = _T("");
		strUploadUrl = _T("");
		strUser = _T("");
		strPwd = _T("");
	}
};

#define  ULEN  100

class CInternetHttp
{

public:
	CInternetHttp(LPCTSTR strAgent = IE_AGENT);
	//CInternetHttp(LPCTSTR strAgent = _T("IE_AGENT"));
	virtual ~CInternetHttp(void);

	int HttpGet(LPCTSTR strUrl, LPCTSTR strPostData, CString &strResponse, DWORD &dwRet/*, CString strUserName = _T(""), CString strPwd = _T("")*/);
	int HttpPost(LPCTSTR strUrl, LPCTSTR strPostData, CString &strResponse, DWORD &dwRet/*, CString strUserName = _T(""), CString strPwd = _T("")*/);
	void UploadFile(const CString& strFileURLInServer, const CString& strFileLocalFullPath, CString &strResponse/*, CString strUserName = _T(""), CString strPwd = _T("")*/);

	BOOL LoginAndUploadFile(const CString& strFileLocalFullPath);

	//获取http登录信息
	void GetHttpLoginInfo(const T_LOGIN_INFO &tInfo);

private:
	int ExecuteRequest(LPCTSTR strMethod, LPCTSTR strUrl, LPCTSTR strPostData, CString &strResponse, DWORD &dwRet);
	void Clear();

	CString MakeRequestHeaders(const CString &strBoundary);

	//生成文件头数据
	CString MakePreFileData(const CString &strBoundary, const CString &strFileName);

	//生成文件结尾数据
	CString MakePostFileData(const CString &strBoundary, const CString strFilePath);
	
private:
	CInternetSession *m_pSession;
	CHttpConnection *m_pConnection;
	CHttpFile *m_pFile;

	T_LOGIN_INFO m_LoginInfo;
};

