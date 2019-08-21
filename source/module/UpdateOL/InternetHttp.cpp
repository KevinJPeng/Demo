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

int CInternetHttp::HttpPostFile( LPCTSTR strUrl, LPCTSTR strPostData, LPCTSTR strLocalFileName)
{
	return ExecuteRequestFile(_T("POST"), strUrl, strPostData, strLocalFileName);
}

int CInternetHttp::ExecuteRequestFile( LPCTSTR strMethod, LPCTSTR strUrl, LPCTSTR strPostData, LPCTSTR strLocalFileName)
{	

	CString strServer;
    CString strObject;
    DWORD dwServiceType;
    INTERNET_PORT nPort;

    AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort);

    if(AFX_INET_SERVICE_HTTP != dwServiceType && AFX_INET_SERVICE_HTTPS != dwServiceType)
    {	
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("AfxParseURL返回失败，错误码:%d"),GetLastError());

        return -1;
    }

	byte* szTemp = new byte[1024*500];  
	memset(szTemp,0,1024*500);

    try
    {
		m_pConnection = m_pSession->GetHttpConnection(strServer,
			dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_CONNECT : SECURE_CONNECT,
			nPort);
		m_pFile = m_pConnection->OpenRequest(strMethod, strObject, 
			NULL, 1, NULL, NULL, 
			(dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_REQUEST : SECURE_REQUEST));

        m_pFile->AddRequestHeaders(_T("Accept: *,*/*"));
        m_pFile->AddRequestHeaders(_T("Accept-Language: zh-cn"));
        m_pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded; charset=utf-8"));

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
			delete[]szTemp;
			szTemp = NULL;
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("SendRequest返回失败，错误码:%d"),GetLastError());
			return -1;
		}

		UINT nReaded = 0;
		//此处进行一次判断是返回错误码还是返回的是文件
		CString strErrorCode = _T("");
		CString strPosition = _T("");
		m_pFile->QueryInfo(HTTP_QUERY_CONTENT_DISPOSITION ,strPosition);
		
		if (strPosition.IsEmpty())
		{
			while ((nReaded = m_pFile->Read((void*)szTemp, 1024*500)) > 0)
			{	
				strErrorCode += CString(szTemp);
			}

			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("下载文件失败上层返回的错误码为:%s下载文件路径为:%s"),strErrorCode,strLocalFileName);
			return -1;
		}


		//此处接收数据
		CFile file;
		if (!file.Open(strLocalFileName, CFile::modeWrite | CFile::modeCreate))
		{	
			delete[]szTemp;
			szTemp = NULL;

			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("创建本地文件失败:FilePath:%s"),strLocalFileName);
			return -1;
		}
		
		while ((nReaded = m_pFile->Read((void*)szTemp, 1024*500)) > 0)
		{	
			//用户取消升级
			if (g_bIsCancelUpdate)
			{
				Clear();
				delete[]szTemp;
				szTemp = NULL;
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("检测到用户点击取消消息!"));
				return -2;
			}

			file.Write(szTemp,nReaded);
		}
		file.Close();

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
			delete[]szTemp;
			szTemp = NULL;

			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("下载文件超时!FilePath:%s"),strLocalFileName);
            return OUTTIME;
        }
        else
        {	
			delete[]szTemp;
			szTemp = NULL;

			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("下载文件失败!FilePath:%s&&CInternetExceptionCode:%d&&ErrorCode:%d"),strLocalFileName,dwErrorCode,dwError);

            return FAILURE;
        }
    }

	delete[]szTemp;
	szTemp = NULL;
    return SUCCESS;

}
