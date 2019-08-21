///////////////////////////////////////////////////////////////////////////////
//
// 版权所有(C), 2012, 商讯网信息有限公司
//
// 版本：1.0
// 文件说明：通过HTTP协议与服务器交互的接口类
// 生成日期：2012-12-3
// 作者：何培田
//
// 修改历史：
// 1. 日期：
//   作者：
//   修改内容：
// 2. 
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HttpUtils.h"
#include "CommFunc.h"
#include <Wininet.h>
#include <shlobj.h>
// #include <afx.h>


#pragma comment(lib, "Wininet.lib")

BOOL CHttpUtils::GetFile(TCHAR *pURL, TCHAR *pLocalName, int nRetry)
{
	CString strTargetDir = pLocalName;
	strTargetDir = strTargetDir.Mid(0, strTargetDir.ReverseFind('\\'));
	if (!PathFileExists(strTargetDir))
	{
		SHCreateDirectoryEx(NULL, strTargetDir, NULL);
	}

	try
	{
		for (int i = 0; i < nRetry; ++i)
		{
			if (InternetGetFile(pURL, pLocalName))
			{
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("下载成功：%s  本地路径：%s"), pURL, pLocalName);
				return TRUE;
			}

			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("下载失败：%s  本地路径：%s"), pURL, pLocalName);
		}
	}
	catch (...)
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("下载文件时异常：%s  本地路径：%s"), pURL, pLocalName);
	}

	return FALSE;
}

BOOL CHttpUtils::InternetGetFile(TCHAR *szUrl, TCHAR *szFileName)
{
    DWORD dwFlags = 0;
	BOOL bRes = FALSE;
	HINTERNET hOpen = NULL;
	if (!InternetGetConnectedState(&dwFlags, 0))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("网络未连接, err:%d"), GetLastError());
	}
	
	if(!(dwFlags & INTERNET_CONNECTION_PROXY))
		hOpen = InternetOpen(_T("BIZEXPRESS_UpdateOL_"), INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY, NULL, NULL, 0);
	else
		hOpen = InternetOpen(_T("BIZEXPRESS_UpdateOL_"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if(!hOpen)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("InternetOpen错误, err:%d"), GetLastError());
		return bRes;
	}

	DWORD dwSize;
	TCHAR   szHead[] = _T("Accept: */*\r\n\r\n");
	VOID* szTemp[16384];             //16kb
	HINTERNET  hConnect = NULL;
	//CFile file;
	//CFileException e;

	if ( !(hConnect = InternetOpenUrl( hOpen, szUrl, szHead,
		 _tcslen(szHead), INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0)))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("InternetOpenUrl错误, err:%d"), GetLastError());
        goto end;
	}
		
	//if  (!file.Open(szFileName, CFile::modeWrite | CFile::modeCreate, &e))
	//{
	//	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("打开文件失败：%d"), e.m_cause);
 //       goto end;	    
	//}

	HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("创建文件失败：%d"), GetLastError());
		hFile = NULL;
		goto end;
	}


	DWORD dwByteToRead = 0;
	DWORD dwSizeOfRq = 4;
	DWORD dwBytes = 0;

    if (!HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
                  (LPVOID)&dwByteToRead, &dwSizeOfRq, NULL))
	{
		dwByteToRead = 0;
	}
	
	do
	{
		memset(szTemp,0,sizeof(szTemp));
		if (!InternetReadFile (hConnect, szTemp, 16384,  &dwSize))
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("InternetReadFile错误, err:%d"), GetLastError());
			break;
		}

		if (dwSize==0)
		{
			bRes = TRUE;
			break;
		}
		else
// 			file.Write(szTemp, dwSize);
	    	WriteFile(hFile, szTemp, dwSize, &dwSize, NULL);
		
	}while (TRUE);

end:
	if (NULL != hConnect)
		InternetCloseHandle(hConnect);

	if (NULL != hOpen)
		InternetCloseHandle(hOpen);

	if (hFile)
	    CloseHandle(hFile);

	return bRes;
}

CString CHttpUtils::GetSerRespInfo(TCHAR *szUrl)
{	
    DWORD dwFlags = 0;
	BOOL bRes = FALSE;
	CString strRetValue = _T("");
	string strRawResponse = "";
	DWORD dwCpSize = 0;
	HINTERNET hOpen = NULL;

	if (!InternetGetConnectedState(&dwFlags, 0))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("网络未连接, err:%d"), GetLastError());
	}
	
	if(!(dwFlags & INTERNET_CONNECTION_PROXY))
		hOpen = InternetOpen(_T("BIZEXPRESS_UpdateOL_"), INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY, NULL, NULL, 0);
	else
		hOpen = InternetOpen(_T("BIZEXPRESS_UpdateOL_"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if(!hOpen)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("InternetOpen错误, err:%d"), GetLastError());
		return _T("$ERR$");
	}
	
	DWORD dwSize;
	TCHAR   szHead[] = _T("Accept: */*\r\nContent-Type:text/html; charset=utf-8\r\n\r\n");
	VOID* szTemp[16384];             //16kb
	HINTERNET  hConnect = NULL;

	//Modified Date 2018/05/04
	int iRetry = 0;
	bool bOpenUrlSuccess = false;
	do 
	{
		if (!(hConnect = InternetOpenUrl(hOpen, szUrl, szHead,
			_tcslen(szHead), INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0)))
		{
			iRetry++;
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("InternetOpenUrl错误, err:%d, iRetry:%d"), GetLastError(), iRetry);
			Sleep(20);
		}
		else
		{
			bOpenUrlSuccess = true;
			break;
		}
	} while (!bOpenUrlSuccess && (iRetry < 3));
	if (!bOpenUrlSuccess)
	{
		goto end;
	}

	DWORD dwByteToRead = 0;
	DWORD dwSizeOfRq = 4;
	DWORD dwBytes = 0;

    if (!HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
                  (LPVOID)&dwByteToRead, &dwSizeOfRq, NULL))
	{
		dwByteToRead = 0;
	}
	
	do
	{
		memset(szTemp,0,sizeof(szTemp));
		if (!InternetReadFile (hConnect, szTemp, 16383,  &dwSize))
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("InternetReadFile错误, err:%d"), GetLastError());
			break;
		}

		if (dwSize == 0)
		{
			bRes = TRUE;
			break;
		}
		else
		{
			strRawResponse += (char*)szTemp;
		}
		
	}while (TRUE);
	
	//此处为了统一编码需要将UTF-8转化为宽字节------add by zhumingxing 20150526
	MByteToWChar((LPSTR)(strRawResponse.c_str()), NULL, &dwCpSize, CP_UTF8);
	WCHAR *pwszBuf = new WCHAR[dwCpSize];
	MByteToWChar((LPSTR)(strRawResponse.c_str()), pwszBuf, &dwCpSize, CP_UTF8);
	strRetValue = pwszBuf;
	delete []pwszBuf;
	pwszBuf = NULL;
end:
	if (NULL != hConnect)
		InternetCloseHandle(hConnect);

	if (NULL != hOpen)
		InternetCloseHandle(hOpen);

	return bRes ? strRetValue : _T("$ERR$");
}
