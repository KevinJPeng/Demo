#pragma once

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

#define  IE_AGENT  _T("BIZEXPRESS_UpdateOL_")

class CInternetHttp
{

public:
	CInternetHttp(LPCTSTR strAgent = IE_AGENT);
	//CInternetHttp(LPCTSTR strAgent = _T("IE_AGENT"));
	virtual ~CInternetHttp(void);

	int HttpGet(LPCTSTR strUrl, LPCTSTR strPostData, CString &strResponse);
	int HttpPost(LPCTSTR strUrl, LPCTSTR strPostData, CString &strResponse);
	int HttpPostFile(LPCTSTR strUrl, LPCTSTR strPostData, LPCTSTR strLocalFileName);
private:
	int ExecuteRequest(LPCTSTR strMethod, LPCTSTR strUrl, LPCTSTR strPostData, CString &strResponse);
	int ExecuteRequestFile(LPCTSTR strMethod, LPCTSTR strUrl, LPCTSTR strPostData, LPCTSTR strLocalFileName);

	void Clear();

private:
	CInternetSession *m_pSession;
	CHttpConnection *m_pConnection;
	CHttpFile *m_pFile;
};

