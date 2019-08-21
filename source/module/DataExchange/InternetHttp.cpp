#include "StdAfx.h"
#include "InternetHttp.h"
#include "..\..\common\CommFunc.h"

CInternetHttp::CInternetHttp(LPCTSTR strAgent)
{
    m_pSession = new CInternetSession(strAgent);
    m_pConnection = NULL;
    m_pFile = NULL;
}


CInternetHttp::~CInternetHttp(void)
{
    Clear();
    if(NULL != m_pSession)
    {
        m_pSession->Close();
        delete m_pSession;
        m_pSession = NULL;
    }
}

void CInternetHttp::Clear()
{
    if(NULL != m_pFile)
    {
        m_pFile->Close();
        delete m_pFile;
        m_pFile = NULL;
    }

    if(NULL != m_pConnection)
    {
        m_pConnection->Close();
        delete m_pConnection;
        m_pConnection = NULL;
    }
}

int CInternetHttp::ExecuteRequest(LPCTSTR strMethod, LPCTSTR strUrl, LPCTSTR strPostData, CString &strResponseText)
{
    CString strServer;
    CString strObject;
    DWORD dwServiceType;
    INTERNET_PORT nPort;
    strResponseText = _T("");

    AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort);

    if(AFX_INET_SERVICE_HTTP != dwServiceType && AFX_INET_SERVICE_HTTPS != dwServiceType)
    {
        return -1;
    }

    try
    {
        m_pConnection = m_pSession->GetHttpConnection(strServer,
            dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_CONNECT : SECURE_CONNECT,
            nPort);
        m_pFile = m_pConnection->OpenRequest(strMethod, strObject, 
            NULL, 1, NULL, NULL, 
            (dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_REQUEST : SECURE_REQUEST));

        //DWORD dwFlags;
        //m_pFile->QueryOption(INTERNET_OPTION_SECURITY_FLAGS, dwFlags);
        //dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
        ////set web server option
        //m_pFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS, dwFlags);

        m_pFile->AddRequestHeaders(_T("Accept: *,*/*"));
        m_pFile->AddRequestHeaders(_T("Accept-Language: zh-cn"));
        m_pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded; charset=utf-8"));
		//m_pFile->AddRequestHeaders(_T("Content-Type:text/html; charset=utf-8"));  //统一用utf-8防止乱码
        //m_pFile->AddRequestHeaders(_T("Accept-Encoding: gzip, deflate"));

		//Unicode转utf-8，否则上层收到的数据解析不正确
		DWORD dwSize = 0;
		WCharToMByte((LPWSTR)(strPostData), NULL, &dwSize, CP_UTF8);
		char *pszBuf = new char[dwSize];
		WCharToMByte((LPWSTR)(strPostData), pszBuf, &dwSize, CP_UTF8);

		BOOL bRes = m_pFile->SendRequest(NULL, 0, pszBuf, dwSize);
		delete []pszBuf;

		if(!bRes)
		{
			Clear();
			return -1;
		}

		char szChars[BUFFER_SIZE + 1] = {0};
		string strRawResponse = "";
		UINT nReaded = 0;
		while ((nReaded = m_pFile->Read((void*)szChars, BUFFER_SIZE)) > 0)
		{
			szChars[nReaded] = '\0';
			strRawResponse += szChars;
			memset(szChars, 0, BUFFER_SIZE + 1);
		}

		//UTF8转宽字节
 		dwSize = 0;
		MByteToWChar((LPSTR)(strRawResponse.c_str()), NULL, &dwSize, CP_UTF8);
		WCHAR *pwszBuf = new WCHAR[dwSize];
		MByteToWChar((LPSTR)(strRawResponse.c_str()), pwszBuf, &dwSize, CP_UTF8);

		strResponseText = pwszBuf;
		delete []pwszBuf;

		Clear();
    }
    catch (CInternetException* e)
    {
        Clear();
        DWORD dwErrorCode = e->m_dwError;
        e->Delete();

        DWORD dwError = GetLastError();

        if (ERROR_INTERNET_TIMEOUT == dwErrorCode)
        {
            return OUTTIME;
        }
        else
        {
            return FAILURE;
        }
    }

    return SUCCESS;
}

int CInternetHttp::HttpGet(LPCTSTR strUrl, LPCTSTR strPostData, CString &strResponse)
{
    return ExecuteRequest(_T("GET"), strUrl, strPostData, strResponse);
}

int CInternetHttp::HttpPost(LPCTSTR strUrl, LPCTSTR strPostData, CString &strResponse)
{
    return ExecuteRequest(_T("POST"), strUrl, strPostData, strResponse);
}