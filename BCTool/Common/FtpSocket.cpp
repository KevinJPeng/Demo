#include "stdafx.h"
#include "FtpSocket.h"
#include <string>
#include <io.h>
using std::string;
#define  IP_LEN  20//发送数据的服务器IP数组长度
int CFTPSocket::DATA_TRY_TIMES  = 3;  //数据套接字失败重试次数
int CFTPSocket::DATA_TRY_INTERVAL  = 2000;//数据套接字失败重试间隔

CFTPSocket::CFTPSocket(void)
{
	m_DataSocket = INVALID_SOCKET;
	m_ControlSocket = INVALID_SOCKET;

	m_pszConnectServerIp = new char[IP_LEN];
	m_pszDataServerIp = new char [IP_LEN];
	memset(m_pszConnectServerIp,0,IP_LEN);
	memset(m_pszDataServerIp,0,IP_LEN);
	m_nPort = 0 ;
}

CFTPSocket::~CFTPSocket(void)
{
	if(m_pszConnectServerIp != NULL)
		delete []m_pszConnectServerIp;
	if (m_pszDataServerIp != NULL)
		delete [] m_pszDataServerIp;
	m_pszConnectServerIp = NULL;
	m_pszDataServerIp = NULL;

	CloseDataSocket();
	CloseCtrlSocket();
	::WSACleanup();
}

bool CFTPSocket::InitFtp(char *sIP, int iPort,int nTimeOut, char *sUsr, char *sPwd)
{	
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2,2);

	if(::WSAStartup(sockVersion,&wsaData) != 0)
	{
		return FALSE;
	}

	if(!SetConnectServerIp(sIP))
		return FALSE;
	do 
	{
		if(!CreateControlSocket(iPort,nTimeOut))
			break;
		if(!SendUSER(sUsr))
			break;
		if(!SendPASS(sPwd))
			break;
		/*if(!SendAndRecv("OPTS UTF8 OFF\r\n","200"))
			break;*/
		return TRUE;
	} while (0);

	if (m_ControlSocket!=INVALID_SOCKET)
	{
		closesocket(m_ControlSocket);
	}
	return FALSE;
}
bool CFTPSocket::SetConnectServerIp(char * pszServerIp)
{
	if(m_pszConnectServerIp == NULL)
		return FALSE;
	else
	{
		memset(m_pszConnectServerIp,0,IP_LEN);
		if(!strcpy_s(m_pszConnectServerIp,IP_LEN,pszServerIp))
			return TRUE;
		else
			return FALSE;
	}
}
bool CFTPSocket::CreateControlSocket(u_short nServerPort , int nTimeOut)
{
	sockaddr_in Server;
	Server.sin_family = AF_INET;
	Server.sin_port = htons(nServerPort);
	Server.sin_addr.S_un.S_addr = inet_addr(m_pszConnectServerIp);

	m_ControlSocket = socket(AF_INET,SOCK_STREAM,0);
	if(m_ControlSocket == INVALID_SOCKET)
	{
		return FALSE;
	}
	bool bislinger = true;
	//设置直接返回
	setsockopt(m_ControlSocket,SOL_SOCKET,SO_DONTLINGER,(char *)&bislinger,sizeof(bool));
	//发送延迟
	setsockopt(m_ControlSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&nTimeOut,sizeof(int));
	//接收延迟
	setsockopt(m_ControlSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nTimeOut,sizeof(int));

	if(connect(m_ControlSocket,(sockaddr *)&Server,sizeof(Server)))
	{
		return FALSE;
	}
	if(!RecvControl("220"))
		return FALSE;
	return TRUE;
}
bool CFTPSocket::CreateDataSocket()
{
	if(!GetIPAndPortByPasvString(m_szPasvString,m_pszDataServerIp,m_nPort))
		return FALSE;
	if(m_DataSocket != INVALID_SOCKET)
	{
		closesocket(m_DataSocket);
	}
	m_DataSocket = socket(AF_INET,SOCK_STREAM,0);
	if(m_DataSocket == INVALID_SOCKET)
		return FALSE;
	sockaddr_in RecvAddr ;
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(m_nPort);
	RecvAddr.sin_addr.S_un.S_addr = inet_addr(m_pszDataServerIp);
	if(DATA_TRY_TIMES <= 0)
		DATA_TRY_TIMES = 1;
	for (int i = 0 ;i< DATA_TRY_TIMES; i++)
	{
		if( connect(m_DataSocket,(sockaddr *)&RecvAddr,sizeof(sockaddr_in)) == 0 )
		{
			return TRUE;
		}
		Sleep(DATA_TRY_INTERVAL);
	}
	OutputDebugStringA("Create Data Socket Failed!\r\n");
	return FALSE;
	
}

bool CFTPSocket::SendControl(const char * pszData)
{	
	if( send(m_ControlSocket,pszData,strlen(pszData),0) == SOCKET_ERROR )
	{
		return FALSE;
	}

	OutputDebugStringA(pszData);
	return TRUE;
}
bool CFTPSocket::RecvControl(const char * pszResult)//pszResult为判断是否成功执行命令
{	
	Sleep(150);			//此处等待服务器响应完毕
	memset(m_szBuf,0,RECV_BUF_LEN);
	if( recv(m_ControlSocket,m_szBuf,RECV_BUF_LEN,0) == SOCKET_ERROR)
	{
		return FALSE;
	}

	if(pszResult != NULL)
	{
		if(!strstr(m_szBuf,pszResult))
		{
			OutputDebugStringA(m_szBuf);
			return FALSE;
		}
	}
	OutputDebugStringA(m_szBuf);
	return TRUE;
}

bool CFTPSocket::SendAndRecv(const char * pszCMD,char * pszResult /*= NULL*/)
{
	if(!SendControl(pszCMD))
		return FALSE;
	if(!RecvControl(pszResult))
		return FALSE;
	return TRUE;
}

bool CFTPSocket::SendUSER( char *pszUserName)
{
	if(!SendControl(pszUserName))
		return FALSE;
	if(!RecvControl("331"))
		return FALSE;
	return TRUE;
}
bool CFTPSocket::SendPASS(char * pszPassWord)
{
	if(!SendControl(pszPassWord))
		return FALSE;
	if(!RecvControl("230"))
		return FALSE;
	return TRUE;
}
bool CFTPSocket::SavePasvStr()
{
	///////////////获取服务器 数据传输的Ip和端口号字符串
	//if( strcmp(m_szPasvString,m_szBuf) == 0 )
	//	return TRUE;//没有改变 不需要改变
	memset(m_szPasvString,0,RECV_BUF_LEN);
	memcpy(m_szPasvString,m_szBuf,strlen(m_szBuf));
	if(m_szPasvString != NULL)
		return TRUE;
	else
		return FALSE;

}
//获取Ip地址和端口号从类似 (192,168,20,12,111) 字符串里面
BOOL CFTPSocket::GetIPAndPortByPasvString(char * lpszPasvString, OUT char * csIP, OUT USHORT &nPort)
{
	string csIP1;
	if ( !lpszPasvString ) 
		return FALSE;
	const char *p = strchr ( lpszPasvString, '(' );
	if ( !p ) 
		return FALSE;
	string csPasvStr = p+1, csTemp;
	int nPosStart = 0, nPosEnd = 0;
	int nMultiple = 0, nMantissa = 0;
	for ( int i=0; ; i++ )
	{
		nPosEnd = csPasvStr.find ( ",", nPosStart );
		if ( nPosEnd < 0 )
		{
			if ( i == 5 )
			{
				nPosEnd = csPasvStr.find ( ")", nPosStart );
				csTemp = csPasvStr.substr ( nPosStart, nPosEnd-nPosStart );
				nMantissa = atoi ( csTemp.c_str() );
				break;
			}
			else 
				return FALSE;
		}
		csTemp = csPasvStr.substr ( nPosStart, nPosEnd-nPosStart );
	/*	csTemp.TrimLeft(); 
		csTemp.TrimRight();*/
		if ( i < 4 )
		{
			if ( !csIP1.empty () ) 
				csIP1 += ".";
			csIP1 += csTemp;

		}
		else if ( i == 4 )
		{
			nMultiple = atoi ( csTemp.c_str() );
		}
		else 
			return FALSE;
		nPosStart = nPosEnd + 1;
	}
	memcpy(csIP,csIP1.c_str(),csIP1.length());
	nPort = nMultiple*256 + nMantissa;
	return TRUE;
}

 bool CFTPSocket::Download(char * pszFileName,char * pszSavePath,ULONGLONG uRestPos)
 {
	 if(!SendAndRecv("PASV \r\n","227"))
	 {
		 return FALSE;
	 }
	 if( !SavePasvStr())//保存PASV返回的字符数组
		 return FALSE;
	 if (!CreateDataSocket())
	 {
		 return FALSE;
	 }
	 char szPathCmd[MAX_PATH + 7]   = {0};
	 sprintf(szPathCmd,"SIZE %s\r\n",pszFileName);
	 if(!SendAndRecv(szPathCmd,"213"))
	 {		
		 return FALSE;
	 }

	 ULONGLONG nSize = GetSize(m_szBuf);//获取文件大小
	 if (nSize <= 0 )
	 {
		 return FALSE;
	 }

	 sprintf(szPathCmd,"REST %I64d\r\n",uRestPos);
	 if(!SendAndRecv(szPathCmd,"350"))
	 {
		 return FALSE;
	 }

	 sprintf(szPathCmd,"RETR %s\r\n",pszFileName);
	 if(!SendAndRecv(szPathCmd,"150"))
	 {
		  return FALSE;
	 }

	  sprintf(szPathCmd,"%s",pszSavePath);
	 if (!RecvAndWriteFile(szPathCmd, uRestPos,nSize-uRestPos))
	 {
		 return FALSE;
	 }
	 return TRUE;
 }

 bool CFTPSocket::RecvAndWriteFile(char* pSavePath,ULONGLONG iSeekPos,ULONGLONG iFileSize)
 {
	 
#define TEMPSIZE    (512*1024)
	 char *acRecv = new char[TEMPSIZE];
	 memset(acRecv,0,TEMPSIZE);
	 
	 bool bRes = false;
	if (NULL != pSavePath && 0 < iFileSize)
	{
		CFile localFile; 
		CFileException e;
		DWORD nRet = localFile.Open(CString(pSavePath),CFile::modeCreate |CFile::modeNoTruncate | CFile::modeWrite | CFile::typeBinary,&e);

		if (nRet == 0)
		{	
			return FALSE;
		}
		else
		{
			localFile.Seek(iSeekPos,CFile::begin);
		}

		int iReturn = 0;
		bool bFindErr = false;
		while ( iFileSize > 0 )
		{
			ULONGLONG iNeedSize = iFileSize > TEMPSIZE ? TEMPSIZE : iFileSize;
			iReturn = recv(m_DataSocket, acRecv, iNeedSize, 0);
			if(iReturn == SOCKET_ERROR)
			{
				bFindErr = TRUE;
				break;
			}
			localFile.Write(acRecv,iReturn);
			iFileSize -= iReturn;
		}
		localFile.Close();
		if (!bFindErr)
		{
			bRes = TRUE;
		}
	}
	CloseDataSocket();
	if (acRecv != NULL)
	{
		delete []acRecv;
		acRecv = NULL;
	}
	 return bRes;
 }
 ULONGLONG  CFTPSocket::GetSize(char * pszStr)
 {
	 char * pszPos =strstr(pszStr,"213 ");
	 if(pszPos == NULL)
	 {
		 return FALSE;
	 }
	 pszPos += strlen("213 ");
	 while (strstr(pszPos," ") == pszPos)
	 {
		 pszPos++;
	 }
	 return _ttoi64(CString(pszPos)); //返回字节数
 }

void  CFTPSocket::CloseCtrlSocket()
{
	if(m_ControlSocket != INVALID_SOCKET)
	{
		SendAndRecv("QUIT \r\n","");
		closesocket(m_ControlSocket);
	}
	m_ControlSocket = INVALID_SOCKET;

}
void  CFTPSocket::CloseDataSocket()
{
	if(m_DataSocket != INVALID_SOCKET)
	{
		closesocket(m_DataSocket);
	}
	m_DataSocket = INVALID_SOCKET;
}

void CFTPSocket::clearFtp()
{
	if(m_pszConnectServerIp != NULL)
		delete []m_pszConnectServerIp;
	if (m_pszDataServerIp != NULL)
		delete [] m_pszDataServerIp;
	m_pszConnectServerIp = NULL;
	m_pszDataServerIp = NULL;

	CloseDataSocket();
	CloseCtrlSocket();
	::WSACleanup();
}
