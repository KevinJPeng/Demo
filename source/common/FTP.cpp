#include "StdAfx.h"
#include "FTP.h"
#include "FtpSocket.h"

/*上传时一次性写入的数据大小-----实际写入的数据大小
跟网络有关系
*/
#define UPLOAD_SIZE  (/*2*1024*/512*1024)

CFTP::CFTP(void):m_session(_T("BIZEXPRESS_LIVEUPDATE_"))
{	
	m_pConnect = NULL;
	m_pRemoteFile = NULL;
	m_lRestPos = 0;
}


CFTP::~CFTP(void)
{	
	DisConnect();
}

/*
	@brief 连接FTP服务器
	@param  ftp用户信息结构体
	*/
BOOL CFTP::Connect(const T_CONN_INFO &tInfo)
{
	m_tConnInfo.strServer = tInfo.strServer;
	m_tConnInfo.strUser = tInfo.strUser;
	m_tConnInfo.strPwd = tInfo.strPwd;
	m_tConnInfo.nPort = tInfo.nPort;
	m_tConnInfo.bPassive = tInfo.bPassive;
	try
	{
		m_pConnect = m_session.GetFtpConnection(tInfo.strServer, tInfo.strUser, tInfo.strPwd, tInfo.nPort, tInfo.bPassive);
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);

		m_strErrorMsg.Format(_T("连接FTP服务器失败:host:%s##username:%s##password:%s，错误信息:%s"),m_tConnInfo.strServer,m_tConnInfo.strUser,m_tConnInfo.strPwd,sz);
		pEx->Delete();
		return FALSE;
	}

	return TRUE;
}

/*
	@brief 检测当前是否连接
	*/
BOOL CFTP::IsConnect()
{
	if (m_pConnect != NULL)
	{
		return TRUE;
	}
	return FALSE;
}

/*
	@brief 断开当前连接重新连接ftp服务器
	*/
BOOL CFTP::ReConnect()
{
	if (m_pConnect != NULL)
	{
		m_pConnect->Close();
		delete m_pConnect;
		m_pConnect = NULL;
	}
	return Connect(m_tConnInfo);
}


	/*
	@brief 断开与FTP连接
	*/
void CFTP::DisConnect()
{	
	try 
	{
		if (NULL != m_pConnect)
		{
			m_pConnect->Close() ;
			m_session.Close() ;
			delete m_pConnect ;
			m_pConnect = NULL ;
		}
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		m_strErrorMsg = sz;
		pEx->Delete();

	}

}

/*
	@brief 从FTP下载文件到本地，此方法支持断点续传
	@param  strRFile  ftp文件名全路径
	@param  strLFile  本地文件全路径
	@param  IsNeedRest  True 表示需要断点续传 False表示不需要断点续传
	@param  dwRetryCount  失败之后的重试次数
	@return  True 表示下载成功 False表示下载失败
	*/
BOOL CFTP::GetFileByte(CString strRFile, CString strLFile,DWORD dwRetryCount,BOOL IsNeedRest)
{	
	DWORD dwRetryCountAll = 0;

	//先断开ftp连接，然后用sockt的方式来进行ftp断点续传
	DisConnect();
	CString strTarDir = strLFile.Left(strLFile.ReverseFind('\\'));
	SHCreateDirectoryEx(NULL, strTarDir,NULL);
	CString strLFileTemp = strLFile + _T("_SyncTemp");
Retry:
	if (dwRetryCountAll > dwRetryCount)
	{
		return FALSE;
	}
	int ilength = -1;
	CFTPSocket ftpsocket;

#ifdef _UNICODE
	char* strServerIp = CStringToMutilChar(m_tConnInfo.strServer,ilength);
	int iport = m_tConnInfo.nPort;

	CString strUserCommand = _T("");
	CString strPassWordCommand = _T("");

	strUserCommand.Format(_T("USER %s\r\n"),m_tConnInfo.strUser);
	char* struser = CStringToMutilChar(strUserCommand,ilength);

	strPassWordCommand.Format(_T("PASS %s\r\n"),m_tConnInfo.strPwd);
	char* strpassword= CStringToMutilChar(strPassWordCommand,ilength);

	char* strRFilePath =  CStringToMutilChar(strRFile,ilength);
	char* strLocalFilePath = CStringToMutilChar(strLFileTemp,ilength);
#else
	char* strServerIp = m_tConnInfo.strServer.GetBuffer();
	int iport = m_tConnInfo.nPort;

	CString strUserName = CString(_T("USER ")) + m_tConnInfo.strUser + _T("\r\n");
	char* struser = strUserName.GetBuffer();

	CString strPassWord = CString(_T("PASS ")) + m_tConnInfo.strPwd + _T("\r\n");
	char* strpassword= strPassWord.GetBuffer();

	char* strRFilePath =  strRFile.GetBuffer();
	char* strLocalFilePath = strLFileTemp.GetBuffer();
#endif

	//获取本地文件信息
	if (!IsNeedRest)
	{	
		DeleteFile(strLFileTemp);
		m_lRestPos = 0;
	}
	else
	{
		CFileStatus tFileInfo = {0};
		CFile::GetStatus(strLFileTemp,tFileInfo);
		m_lRestPos = tFileInfo.m_size;
	}

	if (!ftpsocket.InitFtp(strServerIp,iport,5000,struser,strpassword))
	{
		ftpsocket.clearFtp();
		Sleep(1000);
		++ dwRetryCountAll;
		goto Retry;
	}

	if (!ftpsocket.Download(strRFilePath,strLocalFilePath,m_lRestPos))
	{
		ftpsocket.clearFtp();
		Sleep(1000);
		++ dwRetryCountAll;
		goto Retry;
	}
	else
	{
		//删除本地已经存在的相关文件
		DeleteFile(strLFile);
		CFile::Rename(strLFileTemp,strLFile);
	}
	return TRUE;
}

	/*
	@brief 从本地上传文件到ftp服务器，此方法支持断点续传
	@param  strRFile	ftp文件全路径
	@param  strLFile  本地文件全路径
	@param  IsNeedRest  True 表示需要断点续传 False表示不需要断点续传
	@param  dwRetryCount  失败之后的重试次数
	@return  True 表示上传成功 False表示上传失败
	*/
BOOL CFTP::UpLoadFileByte(CString strRFile,CString strLFile,DWORD dwRetryCount,BOOL IsNeedRest,BOOL IsDelTempFile)
{	
	DWORD len,dwRetryCountAll = 0;
	BOOL bIsUpLoadData = FALSE;
	ULONGLONG dwLastDataSize = 0;
	DWORD dwSleepTime = 800;
	ULONGLONG ulFileLength = 0;

	char *buffer = new char[UPLOAD_SIZE]; 
	memset(buffer,0,UPLOAD_SIZE);
	CString strTarDir = strRFile.Left(strRFile.ReverseFind('/'));

	CFile localFile; 
	CFileException e;
	DWORD nRet = localFile.Open(strLFile,CFile::modeRead | CFile::shareDenyWrite| CFile::typeBinary,&e);

	if (nRet == 0)
	{	
		m_strErrorMsg = _T("打开本地文件失败%s", strLFile);
		delete []buffer;
		buffer = NULL;
		return FALSE;

	}
	else
	{	
		//此处对上传文件的有效性做一个判断----此判断理论上应该在使用上传函数之前就对文件进行判断才对
		ulFileLength = localFile.GetLength();
		if (ulFileLength == 0)
		{		
			localFile.Close();
			delete []buffer;
			buffer = NULL;

			m_strErrorMsg.Format(_T("上传文件无效,文件%s大小为0KB"), strLFile) ;
			return FALSE;
		}
	}
	//如果服务器上已经存在此temp文件则必须将原有的删除之后再上传
	if (IsDelTempFile)
	{	
		if (!DeleteFtpFile(strRFile +_T("_SyncTemp")))
		{	
			localFile.Close();
			delete []buffer;
			buffer = NULL;

			m_strErrorMsg.Format(_T("无法删除服务器上已经存在的temp文件:%s--错误码:%d"),strRFile +_T("_SyncTemp"),GetLastError());
			return FALSE;
		}
	}
Retry:
	if (dwRetryCountAll > dwRetryCount)
	{	
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("已经重试最大重试次数%d次,文件上传失败!"),dwRetryCount);

		localFile.Close();
		delete []buffer;
		buffer = NULL;
		return FALSE;
	}
	//如果不需要断点续传，则删掉之前的已经上传的文件
	if (!IsNeedRest)
	{	
		if (!DeleteFtpFile(strRFile +_T("_SyncTemp")))
		{	
			localFile.Close();
			delete []buffer;
			buffer = NULL;

			m_strErrorMsg.Format(_T("无法删除服务器上已经存在的temp文件:%s--错误码:%d"),strRFile +_T("_SyncTemp"),GetLastError());
			return FALSE;
		}
	}
breakOut:
	if (!ReConnect())
	{	
		++ dwRetryCountAll;		
		Sleep(1000);	
		bIsUpLoadData = FALSE;
		goto Retry;
	}
	try
	{	
		m_pConnect->CreateDirectory(strTarDir);
		m_pRemoteFile = NULL;
		m_pRemoteFile = m_pConnect->Command(_T("APPE ") + strRFile + _T("_SyncTemp"),CFtpConnection::CmdRespWrite);
		ULONGLONG ulRestPos = GetServerRestPos();

		//避免获取服务器文件大小错误
		if (!bIsUpLoadData)
		{
			dwLastDataSize = ulRestPos;
		}
		else
		{
			if (ulRestPos <= dwLastDataSize)
			{	
				m_strErrorMsg.Format(_T("文件%s,上一次Rest:%d,本地Rest:%d,数据有误,重试!"),strRFile,dwLastDataSize,ulRestPos);
				dwSleepTime += 200;		//此处适当延长等待时间，保证数据大小获取准确
				++ dwRetryCountAll;		
				Sleep(1000);
				bIsUpLoadData = FALSE;
				goto Retry;
			}
			else
			{
				dwLastDataSize = ulRestPos;
			}
		}

		localFile.Seek(ulRestPos,CFile::begin);   
		len=localFile.Read(buffer,UPLOAD_SIZE);
		
		//此处文件已经上传完毕
		if (len == 0)
		{	
			localFile.Close();
			delete []buffer;
			buffer = NULL;

			if (ulRestPos != ulFileLength)
			{
				m_strErrorMsg.Format(_T("文件上传失败,REST最后的大小无法与文件大小匹配!"));
				DeleteFtpFile(strRFile + _T("_SyncTemp"));
				return FALSE;
			}
			if (ReNameFtpFile(strRFile + _T("_SyncTemp"),strRFile))
			{	
				return TRUE;
			}
			else
			{	
				m_strErrorMsg.Format(_T("文件上传成功重命名temp文件失败!错误码%d"),GetLastError());
				return FALSE;
			}
		}

		m_pRemoteFile->Write(buffer,len);
		Sleep(dwSleepTime);						//此处必须延时等待数据写入数据，因为Write为非阻塞
		m_pRemoteFile->Close();
		bIsUpLoadData = TRUE;
		goto breakOut;
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		m_strErrorMsg.Format(_T("上传文件时异常:错误信息:%s"),sz);
		pEx->Delete();
		++ dwRetryCountAll;	
		bIsUpLoadData = FALSE;
		//此处适当的等待之后再重试
		Sleep(5000);
		goto Retry;
	}
}


	/*
	@brief 获取FTP文件大小及最后修改时间
	@param  strRFile  ftp文件全路径
	@param  strLastModefyTime ftp文件最后修改时间
	*/

ULONGLONG CFTP::GetServerFileLength(CString strRFile,CString&strLastModefyTime)
{	
	//增加延时解决无法正确获取服务器文件大小问
	CFtpFileFind finder(m_pConnect);

	BOOL bFind = finder.FindFile(strRFile);

	if (!bFind)
	{	
		strLastModefyTime = _T("");
		return 0;
	}
	else
	{	
		finder.FindNextFile();

		CTime LastModifyTime ;
		finder.GetLastWriteTime(LastModifyTime) ;
		strLastModefyTime = LastModifyTime.Format(_T("%Y-%m-%d %H:%M:%S")) ;
		return  (finder.GetLength());
	}
}


/*
	@brief 从FTP下载文件到本地，此处为文件级的下载
	@param  strRFile  ftp文件名
	@param  strLFile  本地文件全路径
	@return  TRUE 表示成功 False表示失败
	*/
BOOL CFTP::GetFile(CString strRFile, CString strLFile)
{
	CString strTarDir = strLFile.Left(strLFile.ReverseFind('\\'));
	SHCreateDirectoryEx(NULL, strTarDir,NULL);

	CString strRDir = strRFile.Left(strRFile.ReverseFind('/'));
	if (!m_pConnect->SetCurrentDirectory(strRDir))
	{	
		m_strErrorMsg.Format(_T("设置ftp当前目录失败%s"),strRDir);
		return FALSE ;
	}

	if (m_pConnect->GetFile(strRFile, strLFile, FALSE))
		return TRUE;

	DWORD dwErr = GetLastError();

	if (dwErr == ERROR_INTERNET_CONNECTION_ABORTED || dwErr == ERROR_FILE_NOT_FOUND)
	{

		ReConnect();
		if (m_pConnect != NULL)
		{
			if (m_pConnect->GetFile(strRFile, strLFile, FALSE))
			{
				return TRUE ;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}

	}
	else
	{
		DWORD dwLen = 512;
		TCHAR szErrMsg[512] = {0};
		InternetGetLastResponseInfo(&dwErr, szErrMsg, &dwLen);

		m_strErrorMsg = szErrMsg;

		return FALSE;
	}
}

/*
	@brief 从本地上传文件到ftp服务器，此处为文件级的上传
	@param  strRFile	ftp文件全路径
	@param  strLFile  本地文件全路径
	@return  返回错误码 1表示上传成功其他表示失败
	*/
BOOL CFTP::UpLoadFile(CString strRFile,CString strLFile)
{

	CString strTarDir = strRFile.Left(strRFile.ReverseFind('/'));
	m_pConnect->CreateDirectory(strTarDir);

	if (m_pConnect->PutFile(strLFile, strRFile, FTP_TRANSFER_TYPE_BINARY,0))
	{
		return TRUE;
	}

	DWORD dwErr = GetLastError();

	if (dwErr == ERROR_INTERNET_CONNECTION_ABORTED || dwErr == ERROR_FILE_NOT_FOUND)
	{

		ReConnect();
		if (m_pConnect != NULL)
		{
			if (m_pConnect->PutFile(strLFile, strRFile, FTP_TRANSFER_TYPE_BINARY,0))
			{
				return TRUE ;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}

	}
	else
	{
		DWORD dwLen = 512;
		TCHAR szErrMsg[512] = {0};
		InternetGetLastResponseInfo(&dwErr, szErrMsg, &dwLen);

		m_strErrorMsg = szErrMsg;

		return FALSE;
	}
}

/*
	@brief 获取出错信息
	*/

CString CFTP::GetErrorMessage()
{
	return m_strErrorMsg;
}

/*
	@brief 判断FTP文件是否存在
	@param  strRFile  ftp文件全路径
	*/
BOOL CFTP::IsFtpFileExist( CString strRFile )
{
	CFtpFileFind finder(m_pConnect);

	BOOL bFind = finder.FindFile(strRFile);

	if (!bFind)
	{	
		return FALSE;
	}
	else
	{	
		return TRUE;
	}
}

/*
@brief 删除ftp服务器文件
*/
BOOL CFTP::DeleteFtpFile(CString strRFilePath)
{	
	if (!ReConnect())
	{
		return FALSE;
	}
	if (IsFtpFileExist(strRFilePath))
	{	
		int iReTryDelete = 0;

		while(iReTryDelete++ < 3)
		{
			//此处一定要先删除已经存在的temp，否则文件就有可能出错
			if(!m_pConnect->Remove(strRFilePath))
			{
				Sleep(5000);			//等待服务器文件状态刷新
				continue;
			}
			else
			{
				return TRUE;
			}
		}
		return FALSE;

	}
	else
	{
		return TRUE;
	}
}


/*
	@brief 重命名ftp服务器文件
*/
BOOL CFTP::ReNameFtpFile(CString strOld,CString strNew)
{

	int iRetry = 0; //文件已经上传完成，此时如果连接服务器失败，重试3次

	while(iRetry < 3)
	{	
		//此处必须要重新连接，否则重命名会失败
		if (!ReConnect())
		{
			++iRetry;
			Sleep(2000);			//等待
			continue;
		}
		else
		{
			m_pConnect->Remove(strNew);
			m_pConnect->Rename(strOld,strNew);
			return TRUE;
		}
	}
	return FALSE;
}

ULONGLONG CFTP::GetServerRestPos()
{
	DWORD dwLastError = GetLastError() ;
	DWORD dwLen = 512;
	TCHAR szErrMsg[512] = {0};
	InternetGetLastResponseInfo(&dwLastError, szErrMsg, &dwLen);

	CString strRespone(szErrMsg);
	if (strRespone.Find(_T("restarting at offset")) == -1)
	{
		return 0;
	}
	else
	{
		strRespone = strRespone.Mid(strRespone.ReverseFind(_T(' ')));
		return(_ttoi64(strRespone));
	}
}

char* CFTP::CStringToMutilChar( CString& str,int& chLength,WORD wPage/*=CP_ACP*/ )
{

		char* pszMultiByte; 
		int iSize = WideCharToMultiByte(wPage, 0, str, -1, NULL, 0, NULL, NULL); 
		pszMultiByte = (char*)malloc((iSize+1)/**sizeof(char)*/); 
		memset(pszMultiByte,0,iSize+1);
		WideCharToMultiByte(wPage, 0, str, -1, pszMultiByte, iSize, NULL, NULL); 
		chLength = iSize;
		return pszMultiByte;
}
