#include "StdAfx.h"
#include "commclient.h"
#include "shlobj.h"
/*#include "Trace.h"*/
#include "ScheduleBase.h"

CLogTrace g_commLog(_T("ScheduleComm.log"),NULL);
CLock g_lock;


CCommClient::CCommClient(void)
{
	m_hThread = NULL;
	m_socket = NULL;
	m_bStop = false;
}

CCommClient::~CCommClient(void)
{
	DisConnect();
}

//连接到服务端
int  CCommClient::Init(int iPort, CScheduleBase *pBase)
{
	if (-1 == Connect(iPort) || NULL == pBase)
		return -1;
		
	m_pScheduleBase = pBase;
	m_bStop = false;

	return CreateRecvThread();
}

// int CCommClient::SetDataHandler(pfnDataHandler fn)
// {
// 	if (!fn)
// 	{
// 		g_commLog.Trace(LOGL_TOP,LOGC_ERROR,__TFILE__, __LINE__,_T("回调函数不能为空！"));
// 		return -1;
// 	}
// 
// 	m_pfnDataHandler = fn;
// 
// 	return 0;
// }

//连接到服务端
int  CCommClient::Connect(int iPort)
{
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR )
		return -1;

	// 创建socket
	m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if ( m_socket == INVALID_SOCKET ) {
		WSACleanup();
		return -1;
	}
	sockaddr_in service;

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	service.sin_port = htons(iPort);

	g_commLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Connect %d"), iPort);
	DWORD dwBeginTime = GetTickCount();
	while(connect(m_socket,(SOCKADDR*)&service,sizeof(service))==SOCKET_ERROR)
	{
		Sleep(60);
		if (GetTickCount() - dwBeginTime > 20000)
		{
			g_commLog.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("Connect timeout!"));
			return -1;
		}
	}

	g_commLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Connect success"));

	return 0;
}

//与服务端断开连接
DWORD CCommClient::DisConnect(void)
{
    if(m_socket!=NULL)
	{
		g_commLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("closesocket"));
		closesocket(m_socket);
		m_socket = NULL;
	}
	return 0;
}

//发送数据到服务进程
int CCommClient::SendData(CStdString strData)
{
	if(m_socket == NULL)return -1;

	int iDataSize = 0;
	char *pSendData = NULL;
	DWORD dwLen = 0;

	iDataSize = strData.GetLength() * sizeof(TCHAR);
	pSendData = new char[iDataSize];
	if (NULL == pSendData)
		return -1;

	memset(pSendData, 0, iDataSize);

#ifdef _UNICODE
	dwLen = iDataSize;
	WCharToMByte(strData.GetBuffer(), pSendData, &dwLen);
#endif

	char *buff=new char[iDataSize+sizeof(DWORD)];
	dwLen = iDataSize+sizeof(DWORD);
	memcpy(buff,&dwLen,sizeof(DWORD));
	memcpy(&buff[sizeof(DWORD)],pSendData,iDataSize);
	int nSendByte = send(m_socket,buff,dwLen,0);

	delete []pSendData;
	delete []buff;

	return nSendByte;
}

//创建接收线程
DWORD CCommClient::CreateRecvThread(void)
{	
	DWORD dwThreadID = 0;
	m_hThread = ::CreateThread(NULL, 0, 
		StaticThreadFunc, 
		this, 0, &dwThreadID);

	if (!m_hThread || !dwThreadID) //线程创建失败
	{
		g_commLog.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("创建接收线程失败"));
		return -1;
	}
	return 0;
}

//线程静态函数
DWORD WINAPI CCommClient::StaticThreadFunc(LPVOID lpParam)
{
	CCommClient *pChannel = (CCommClient*)lpParam;
	SOCKET ClientSocket = pChannel->m_socket;

	int bytesRecv = SOCKET_ERROR;
	char recvbuf[8192] = {0};	//接收区
	char *pDataBuff = NULL;	//缓冲区
	DWORD dwDataBuffLen = 0;	//缓冲区长度

	if(ClientSocket==NULL)
	{
		return 0;
	}
	while(!pChannel->StopIsSet())
	{
		bytesRecv = recv( ClientSocket, (char*)recvbuf, 8192, 0 );

		if( bytesRecv <= 0 )	//服务端断开连接 等待连接
		{
			g_commLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("服务器断开连接 err:%d"), WSAGetLastError());
			pChannel->OnReceive(NULL, -1);

			ClientSocket = NULL;
			return 0;
		}

		if(pDataBuff == NULL)
		{
			pDataBuff = new char[bytesRecv];
			memcpy(pDataBuff,recvbuf,bytesRecv);
			dwDataBuffLen = bytesRecv;
		}
		else
		{
			char *tmpBuff = new char[dwDataBuffLen];
			memcpy(tmpBuff,pDataBuff,dwDataBuffLen);

			delete []pDataBuff;

			pDataBuff = new char[dwDataBuffLen + bytesRecv];

			memcpy(pDataBuff,tmpBuff,dwDataBuffLen);
			delete []tmpBuff;

			memcpy(&(pDataBuff[dwDataBuffLen]),recvbuf,bytesRecv);
			dwDataBuffLen += bytesRecv;
		}

		//包处理
		if(dwDataBuffLen<sizeof(DWORD)*2)continue; //小于最小包头不做如下处理 

		DWORD dwPacketSize = 0; //包大小
		while(TRUE)
		{
			if(dwDataBuffLen == 0)break; //缓冲区没有数据
			memcpy(&dwPacketSize,pDataBuff,sizeof(DWORD));
			if(dwPacketSize > dwDataBuffLen) 
				break; //包长度大于缓冲区长度不做处理

			char *pData = new char[dwPacketSize];	//拼一个包
			memcpy(pData,pDataBuff + 4,dwPacketSize - 4);
			pChannel->OnReceive(pData,dwPacketSize - 4);
			delete []pData;

			char *tmpBuff = new char[dwDataBuffLen];
			memcpy(tmpBuff,pDataBuff,dwDataBuffLen);

			delete []pDataBuff;
			dwDataBuffLen = dwDataBuffLen - dwPacketSize;
			pDataBuff = new char[dwDataBuffLen];
			memcpy(pDataBuff,&(tmpBuff[dwPacketSize]),dwDataBuffLen);
			delete []tmpBuff;
		} 
	}

	g_commLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("检测到停止标记，接收线程退出！"));


	return 0;
}

//停止数据接收线程
void CCommClient::Stop(void)
{
	m_bStop = true;
	m_pScheduleBase = NULL;
	DisConnect();
}

bool CCommClient::StopIsSet(void)
{
	return m_bStop;
}

wchar_t * CCommClient::MultitoWide( const char *pMulti, DWORD dwCode /*= CP_ACP*/ )
{
	wchar_t *pWide = NULL;
	int iAlen = 0;

	if (pMulti == NULL
		|| (iAlen = strlen(pMulti)) == 0)
	{
		return pWide;
	}

	int iLen = MultiByteToWideChar( dwCode, 0, pMulti, iAlen, NULL, NULL );
	if (iLen > 0)
	{
		pWide = new wchar_t[iLen + 1];
		if (pWide != NULL)
		{
			memset(pWide, 0, (iLen+1)*sizeof(wchar_t));
			MultiByteToWideChar( dwCode, 0, pMulti, iAlen, pWide, iLen );
		}

	}

	return pWide;
}

//接收主控返回数据的回调函数
DWORD CCommClient::OnReceive(char *Orgbuf, DWORD dwTotalLen)
{
	wchar_t *cBuf = MultitoWide(Orgbuf);  //防止电脑当前不是中文字符集（cstring转换）造成乱码的情况
	CString strData(cBuf);

	if (cBuf != NULL)
	{
		delete cBuf;
		cBuf = NULL;
	}
	//断开连接
	if (dwTotalLen == -1)
	{
		strData = "DISCONNECTED";
	}

	if (!m_bStop)
	{
		g_lock.Lock();
		if (m_pScheduleBase != NULL)
		{	
			m_pScheduleBase->OnReceive(strData);
		}
		g_lock.Unlock();
	}

	return 0;
}