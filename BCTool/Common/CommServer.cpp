
#include "stdafx.h"
#include "commserver.h"
#include "commFunc.h"
#include "StringUtils.h"

CCommServer::CCommServer(void) :m_iPort(10037)
{
	m_iPort = 29354;
	m_eventListen = CreateEvent(nullptr, false, false, nullptr);	
}

CCommServer::~CCommServer(void)
{
}

void CCommServer::SetPort(vector<int> _vIport)
{
	m_vIPort = _vIport;
}

//初始化
int CCommServer::Init(int nPort, pfnDataHandler fn)
{
	WSADATA wsaData;
	int errid;

	if (!fn)
	{
		//g_log.Trace(LOGL_TOP,LOGT_ERROR,__TFILE__, __LINE__,_T("回调函数不能为空！"));
		return -1;
	}

	m_pfnDataHandler = fn;


	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR )
	{
	    errid = WSAGetLastError();
	    //g_log.Trace(LOGL_TOP,LOGT_ERROR,__TFILE__, __LINE__,_T("WSAStartup，错误, error:%d"), errid);
		printf("WSAStartup() 错误:\n");
	}
	// 创建socket
	m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if ( m_socket == INVALID_SOCKET ) {
		errid = WSAGetLastError();
		//g_log.Trace(LOGL_TOP,LOGT_ERROR,__TFILE__, __LINE__,_T("socket，错误, error:%d"), errid);
		printf( "socket() 错误: %ld\n", WSAGetLastError() );
		WSACleanup();
		return -1;
	}

	// 绑定socket
	sockaddr_in service;

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	service.sin_port = htons( nPort );

	if ( ::bind( m_socket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR ) 
	{
#ifdef _DEBUG
		::MessageBox(0,_T("客户端运行时发现一个错误,\r\n请检查是否有第三方软件(如:防火墙)禁止了Mcproc.exe权限\r\n"),_T("\n运行时检测"),MB_OK|MB_ICONWARNING);

		CStdString strErro;
		strErro.Format(_T("bind 端口：%d ,错误码:%d "),nPort, GetLastError());
		::MessageBox(0,strErro.GetBuffer(0),_T("\n运行时检测"),MB_OK|MB_ICONWARNING);
#endif
		closesocket(m_socket);
		WSACleanup();

		return -1;
	}

	// 监听socket
	if ( listen( m_socket, 5 ) == SOCKET_ERROR )
	{
#ifdef _DEBUG
		::MessageBox(0,_T("客户端运行时发现一个错误,\r\n请检查是否有第三方软件(如:防火墙)禁止了Mcproc.exe权限\r\n"),_T("\n运行时检测"),MB_OK|MB_ICONWARNING);
#endif
		//g_log.Trace(LOGL_TOP,LOGT_ERROR,__TFILE__, __LINE__,_T("listen( m_socket, 1 ) == SOCKET_ERROR"));
		closesocket(m_socket);
		WSACleanup();

		return -1;
	}

	/*BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	if (setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bOptVal, bOptLen) != SOCKET_ERROR) 
	{
		//g_log.Trace(LOGL_TOP,LOGT_ERROR,__TFILE__, __LINE__,_T("设置发送KEEPALIVE, error:%d"), WSAGetLastError());
	}*/

	//设置发送超时(10秒）
	int iTimeOut = 1000*50;
	//int iTimeOut = 1000*100;
	if(setsockopt(m_socket,SOL_SOCKET,SO_SNDTIMEO,(char*) &iTimeOut,sizeof(int)) == SOCKET_ERROR)
	{
		//g_log.Trace(LOGL_TOP,LOGT_ERROR,__TFILE__, __LINE__,_T("设置发送超时错误, error:%d"), WSAGetLastError());
	}

	//struct linger m_sLinger;
	//m_sLinger.l_onoff = 1; //在调用closesocket()时还有数据未发送完，允许等待
	//// 若m_sLinger.l_onoff=0;则调用closesocket()后强制关闭
	//m_sLinger.l_linger = 5; //设置等待时间为5秒
	//if(setsockopt( m_socket, SOL_SOCKET, SO_LINGER, (const char*)&m_sLinger, sizeof(struct linger)))
	//{
	//	//g_log.Trace(LOGL_TOP,LOGT_ERROR,__TFILE__, __LINE__,_T("设置发送超时错误, error:%d"), WSAGetLastError());
	//}

	m_ClientSocket = INVALID_SOCKET;
	m_bRun = TRUE;


	WaitRecData(TRUE);

	return 0;
}

//解除初始化
int CCommServer::UnInit()
{
	////g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__,_T("-------------UnInit------------"));
	if(m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	if (m_ClientSocket != INVALID_SOCKET)
	{
		shutdown(m_ClientSocket, 2); //断开发送接收功能
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}

	WSACleanup();
	m_bRun = FALSE;
	return -1;
}

int CCommServer::CloseClient()
{
	////g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__,_T("-------------CloseClient------------"));
	//优雅地关闭SOCKET的方式
	shutdown(m_ClientSocket,SD_SEND);
	closesocket(m_ClientSocket) ;
	m_ClientSocket = INVALID_SOCKET;
	return -1 ;
}

//等待客户端连接
int CCommServer::WaitClient()
{
	// 接受
	//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__,_T("等待客户端连接..." ));
	m_ClientSocket = INVALID_SOCKET;
	while ( m_ClientSocket == INVALID_SOCKET ) {
		m_ClientSocket = accept(m_socket, NULL, NULL );
	}

//	g_ClientSocket = m_ClientSocket;

	//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__,_T("客户端连接成功！" ));
	return 0;
}


DWORD WINAPI ThreadWaitAccept(PVOID param)
{
	CCommServer *Server = (CCommServer*)param;


	while (Server->m_bRun)
	{
		SOCKET ClientSocket = INVALID_SOCKET;
		ClientSocket = accept(Server->m_socket, NULL, NULL);
		if (INVALID_SOCKET != ClientSocket)
		{
			pThreadParam threadParam = new ThreadParam();
			threadParam->Server = Server;
			threadParam->ClientSocket = ClientSocket;
			HANDLE hThread = CreateThread(NULL, 0, MThreadRecv, threadParam, 0, 0); //线程方式运行（非阻塞进程）
		}
		else if (SOCKET_ERROR == ClientSocket)
		{
			//监听套接字失效重新绑定
			closesocket(Server->m_socket);
			Server->m_socket = INVALID_SOCKET;
			Server->ReRunServer();
			break;
		}
	}

	return 0;
}

//获取http协议通过post方法发过来的数据
bool CCommServer::GetHttpPostData(TCHAR* _pRecvbuff, TCHAR*& _pParsebuff, DWORD& _len)
{
	_len = 0;
	if (NULL == _pRecvbuff)
	{
		_pParsebuff = NULL;
		return false;
	}
	CStdString sRecvData(_pRecvbuff);
	CStdString sTemp = sRecvData;
	sTemp.MakeLower();
	std::vector<CStdString> vecKeyWordInfo;
	StringUtils StringUtil;
	StringUtil.SplitString((CStdString)sTemp, _T("\r\n"), vecKeyWordInfo, true);
	int ivLen = vecKeyWordInfo.size();
	if (ivLen <= 0)
	{
		return false;
	}
	for (int i = 0; i < ivLen; i++)
	{
		if (-1 != vecKeyWordInfo[i].find(_T("content-length")))
		{
			std::vector<CStdString> vecTemp;
			StringUtil.SplitString((CStdString)vecKeyWordInfo[i], _T(":"), vecTemp, true);
			if (vecTemp.size() > 1)
			{
				CStdString sDataLen = vecTemp[1];
				sDataLen.Trim();
				_len = _ttoi(sDataLen);
				break;
			}
		}
	}

	vecKeyWordInfo.clear();
	StringUtil.SplitString((CStdString)sRecvData, _T("\r\n\r\n"), vecKeyWordInfo, true);
	ivLen = vecKeyWordInfo.size();
	if (ivLen < 1)
	{
		return false;
	}
	vecKeyWordInfo[1] = vecKeyWordInfo[1].Trim();
	int idataLen = vecKeyWordInfo[1].GetLength();
	if (_len > idataLen)
	{
		return false;
	}
	_pParsebuff = new TCHAR[_len * sizeof(TCHAR)+2];
	if (NULL == _pParsebuff)
	{
		return false;
	}
	memset(_pParsebuff, 0, _len * sizeof(TCHAR)+2);
	memcpy(_pParsebuff, (TCHAR*)vecKeyWordInfo[1].GetBuffer(0), _len * sizeof(TCHAR));

	return true;
}
//获取http协议通过get方法发过来的数据
bool CCommServer::GetHttpGetData(TCHAR* _pRecvbuff, TCHAR*& _pParsebuff, DWORD& _len)
{
	// 	GET / ? RequestCode = 10 HTTP / 1.1
	// 	Accept : image / gif, image / x - xbitmap, image / jpeg, image / pjpeg, */*
	// 	User-Agent: Mozilla/4.0 (compatible; MSIE 5.5; Windows 98)
	// 	Host: 127.0.0.1:29354
	// 	Connection: Keep-Alive
	// 	Cache-Control: no-cache
	// 
	// 	?

	_len = 0;
	if (NULL == _pRecvbuff)
	{
		_pParsebuff = NULL;
		return false;
	}
	CStdString sParam;
	CStdString sRecvData(_pRecvbuff);
// 	CStdString sTemp = sRecvData;
// 	sTemp.MakeLower();
	std::vector<CStdString> vecKeyWordInfo;
	StringUtils StringUtil;
	StringUtil.SplitString((CStdString)sRecvData, _T("\r\n"), vecKeyWordInfo, true);
	int ivLen = vecKeyWordInfo.size();
	if (ivLen <= 0)
	{
		return false;
	}
	for (int i = 0; i < ivLen; i++)
	{
		CStdString sTemp = vecKeyWordInfo[i];
		sTemp.MakeLower();
		int iPos = sTemp.Find(_T("http"));
		if (-1 != iPos)
		{
			sParam = vecKeyWordInfo[i];
			sParam = sParam.Left(iPos);
			iPos = sParam.Find(_T("?"));
			if (-1 != iPos)
			{
				sParam = sParam.Right(sParam.GetLength() - iPos - 1);
				sParam.Trim();
				_len = sParam.GetLength();
				break;
			}
		}
	}

	if (_len <= 0)
	{
		return false;
	}
	_pParsebuff = new TCHAR[_len * sizeof(TCHAR)+2];
	if (NULL == _pParsebuff)
	{
		return false;
	}
	memset(_pParsebuff, 0, _len * sizeof(TCHAR)+2);
	memcpy(_pParsebuff, (TCHAR*)sParam.GetBuffer(0), _len * sizeof(TCHAR));

	return true;
}
wchar_t * CCommServer::MultitoWide(const char *pMulti, DWORD dwCode /*= CP_ACP*/)
{
	wchar_t *pWide = NULL;
	int iAlen = 0;

	if (pMulti == NULL
		|| (iAlen = strlen(pMulti)) == 0)
	{
		return pWide;
	}

	int iLen = MultiByteToWideChar(dwCode, 0, pMulti, iAlen, NULL, NULL);
	if (iLen > 0)
	{
		pWide = new wchar_t[iLen + 1];
		if (pWide != NULL)
		{
			memset(pWide, 0, (iLen + 1)*sizeof(wchar_t));
			MultiByteToWideChar(dwCode, 0, pMulti, iAlen, pWide, iLen);
		}

	}

	return pWide;
}

DWORD WINAPI MThreadRecv(PVOID param)
{
	pThreadParam pParam = (pThreadParam)param;
	CCommServer *Server = pParam->Server;
	SOCKET ClientSocket = pParam->ClientSocket;

	int bytesRecv = SOCKET_ERROR;
	TCHAR recvbuf[8192] = { 0 };	//接收区
	DWORD dwPackteCount = 0;
	char *pDataBuff = NULL;	//缓冲区
	DWORD dwDataBuffLen = 0;	//缓冲区长度
	TCHAR* c_acp_or_utf8 = NULL;
	TCHAR* p_c_PraseData = NULL;
	DWORD dwPraseDataLen = 0;	//有效数据长度
	bool bResponse = false;

	while (Server->m_bRun)
	{
		TCHAR recvbuf[8192] = { 0 };	//接收区
		bytesRecv = recv(ClientSocket, (char*)recvbuf, 8192, 0);

		if (bytesRecv == 0 || bytesRecv < 0) //连接关闭
		{
			closesocket(ClientSocket);
			break;
		}
		else if (bytesRecv > 0)
		{
			if (pDataBuff == NULL)
			{
				pDataBuff = new char[bytesRecv];
				memcpy(pDataBuff, recvbuf, bytesRecv);
				dwDataBuffLen = bytesRecv;
			}
			else
			{
				char *tmpBuff = new char[dwDataBuffLen];
				memcpy(tmpBuff, pDataBuff, dwDataBuffLen);

				delete[]pDataBuff;

				pDataBuff = new char[dwDataBuffLen + bytesRecv];

				memcpy(pDataBuff, tmpBuff, dwDataBuffLen);
				delete[]tmpBuff;

				memcpy(&(pDataBuff[dwDataBuffLen]), recvbuf, bytesRecv);
				dwDataBuffLen += bytesRecv;
			}
		}

		if (NULL != c_acp_or_utf8)
		{
			delete[] c_acp_or_utf8;
			c_acp_or_utf8 = NULL;
		}
		if (1)
		{
			c_acp_or_utf8 = Server->MultitoWide((char*)pDataBuff, CP_ACP);
		}
		else
		{
			c_acp_or_utf8 = Server->MultitoWide((char*)pDataBuff, CP_UTF8);
		}
		if (Server->GetHttpGetData(c_acp_or_utf8, p_c_PraseData, dwPraseDataLen))
		{
 			bResponse = true;
 			break;
		}
	}
	if (NULL != c_acp_or_utf8)
	{
		delete[] c_acp_or_utf8;
		c_acp_or_utf8 = NULL;
	}

	if (NULL != pDataBuff)
	{
		delete[] pDataBuff;
		pDataBuff = NULL;
	}
	if (bResponse)
	{
		//调用回调
		Server->m_pfnDataHandler(ClientSocket, p_c_PraseData, dwPraseDataLen);
	}

	if (NULL != pParam)
	{
		delete pParam;
		pParam = NULL;
	}
	return 0;

}

DWORD WINAPI ThreadRecv(PVOID param)
{
// 	CCommServer *Server = (CCommServer*)param;
// 
// WaitConnect:
// 	Server->WaitClient();
// 	int bytesRecv = SOCKET_ERROR;
// 	TCHAR recvbuf[8192] = {0};	//接收区
// 	//TCHAR recvbuf[1024] = {0};	//接收区
// 	DWORD dwPackteCount = 0;
// 	TCHAR *pDataBuff = NULL;	//缓冲区
// 	DWORD dwDataBuffLen = 0;	//缓冲区长度
// 	while(Server->m_bRun)
// 	{
// 		TCHAR recvbuf[8192] = { 0 };	//接收区
// 		bytesRecv = recv(Server->m_ClientSocket, (char*)recvbuf, 8192, 0);
// 		//bytesRecv = recv( Server->m_ClientSocket, (char*)recvbuf, 1024, 0 );
// 		////g_log.Trace(LOGL_TOP,LOGT_PROMPT,__TFILE__, __LINE__,_T("==========================>>>>>>>>>>>>WSAGetLastErrorWSAGetLastError err:%d"), WSAGetLastError());
// 		////g_log.Trace(LOGL_TOP,LOGT_PROMPT,__TFILE__, __LINE__,_T("%%%%%%%%%%%%%%%%%%%%%%%%%%bytesRecv = %d"),bytesRecv);
// 		if( bytesRecv == -1 || bytesRecv == 0)	//客户端断开连接 等待连接
// 		{
// 			if (bytesRecv == -1)
// 			{
// 				Server->m_pfnDataHandler(_T("DISCONNECT"),10);
// 				//g_log.Trace(LOGL_TOP,LOGT_PROMPT,__TFILE__, __LINE__,_T("客户端异常退出，结束4代所有事件"));
// 			}
// 			Server->m_ClientSocket = INVALID_SOCKET ;
// 			//g_log.Trace(LOGL_TOP,LOGT_PROMPT,__TFILE__, __LINE__,_T("客户端断开连接，退出任务 err:%d"), WSAGetLastError());
// 			break;   //连接断开，退出循环
// 		}
// 
// #ifdef _UNICODE		
// 		bytesRecv /= 2;  //UNICODE下收到的字符数为字节数/2
// 		if(pDataBuff == NULL)
// 		{
// 			pDataBuff = new TCHAR[bytesRecv];
// 			wmemcpy(pDataBuff, recvbuf, bytesRecv);
// 			dwDataBuffLen = bytesRecv;
// 		}
// 		else
// 		{
// 			TCHAR *tmpBuff = new TCHAR[dwDataBuffLen];
// 			wmemcpy(tmpBuff,pDataBuff,dwDataBuffLen);
// 
// 			delete []pDataBuff;
// 
// 			pDataBuff = new TCHAR[dwDataBuffLen + bytesRecv];
// 
// 			wmemcpy(pDataBuff,tmpBuff,dwDataBuffLen);
// 			delete []tmpBuff;
// 
// 			wmemcpy(&(pDataBuff[dwDataBuffLen]), recvbuf, bytesRecv);
// 			dwDataBuffLen += bytesRecv;
// 		}
// 
// 		//包处理
// 		if(dwDataBuffLen<sizeof(DWORD)*2)continue; //小于最小包头不做如下处理 
// 
// 		DWORD dwPacketSize = 0; //包大小
// 		while(TRUE)
// 		{
// 			if(dwDataBuffLen == 0)break; //缓冲区没有数据
// 			memcpy(&dwPacketSize,pDataBuff,sizeof(DWORD));
// 			dwPacketSize /= 2;       //UNICODE下接收到包的实际长度为收到的字符数*2
// 			if(dwPacketSize > dwDataBuffLen)        
// 				break; //包长度大于缓冲区长度不做处理
// 
// 			TCHAR *pData = new TCHAR[dwPacketSize];	//拼一个包
// 			wmemcpy(pData,pDataBuff + 4,dwPacketSize - 4);
// 
// 			Server->m_pfnDataHandler(pData,dwPacketSize - 4);
// 			delete []pData;
// 
// 			TCHAR *tmpBuff = new TCHAR[dwDataBuffLen];
// 			wmemcpy(tmpBuff,pDataBuff,dwDataBuffLen);
// 
// 			delete []pDataBuff;
// 			pDataBuff = NULL;
// 
// 			dwDataBuffLen = dwDataBuffLen - dwPacketSize;
// 			pDataBuff = new TCHAR[dwDataBuffLen];
// 			wmemcpy(pDataBuff,&(tmpBuff[dwPacketSize]),dwDataBuffLen);
// 			delete []tmpBuff;
// 			dwPackteCount ++ ; //包计数器
// 		} 
// 
// 		printf("总共收到数据包 %d\n",dwPackteCount); //包计数器
// 		//Sleep(200);	//线程切换
// 	}
// #else
// 		if(pDataBuff == NULL)
// 		{
// 			pDataBuff = new TCHAR[bytesRecv];
// 			memcpy(pDataBuff,recvbuf,bytesRecv);
// 			dwDataBuffLen = bytesRecv;
// 		}
// 		else
// 		{
// 			TCHAR *tmpBuff = new TCHAR[dwDataBuffLen];
// 			memcpy(tmpBuff,pDataBuff,dwDataBuffLen);
// 
// 			delete []pDataBuff;
// 
// 			pDataBuff = new TCHAR[dwDataBuffLen + bytesRecv];
// 
// 			memcpy(pDataBuff,tmpBuff,dwDataBuffLen);
// 			delete []tmpBuff;
// 
// 			memcpy(&(pDataBuff[dwDataBuffLen]),recvbuf,bytesRecv);
// 			dwDataBuffLen += bytesRecv;
// 		}
// 
// 		//包处理
// 		if(dwDataBuffLen<sizeof(DWORD)*2)continue; //小于最小包头不做如下处理 
// 
// 		DWORD dwPacketSize = 0; //包大小
// 		while(TRUE)
// 		{
// 			//if(dwDataBuffLen == 0)break; //缓冲区没有数据
// 			memcpy((WCHAR*)&dwPacketSize,pDataBuff,sizeof(DWORD));
// 			if(dwPacketSize > dwDataBuffLen)        
// 				break; //包长度大于缓冲区长度不做处理
// 
// 			TCHAR *pData = new TCHAR[dwPacketSize];	//拼一个包
// 			memcpy(pData,pDataBuff + 4,dwPacketSize);
// 
// 			Server->m_pfnDataHandler(pData,dwPacketSize);
// 			delete []pData;
// 
// 			TCHAR *tmpBuff = new TCHAR[dwDataBuffLen];
// 			memcpy(tmpBuff,pDataBuff,dwDataBuffLen);
// 
// 			delete []pDataBuff;
// 			pDataBuff = NULL;
// 
// 			dwDataBuffLen = dwDataBuffLen - dwPacketSize;
// 			pDataBuff = new TCHAR[dwDataBuffLen];
// 			memcpy(pDataBuff,&(tmpBuff[dwPacketSize]),dwDataBuffLen);
// 			delete []tmpBuff;
// 			dwPackteCount ++ ; //包计数器
// 		} 
// 
// 		printf("总共收到数据包 %d\n",dwPackteCount); //包计数器
// 		//Sleep(200);	//线程切换
// 	}
// #endif
// 
// 	if (Server->m_bRun)
// 		goto WaitConnect;

	return 0;
}
//接收数据
int CCommServer::WaitRecData(BOOL IsThread)
{
	SetEvent(m_eventListen);

	if(IsThread == FALSE)
 		ThreadRecv(this); //普通方式运行（阻塞进程）
	else
		m_hThread = CreateThread(NULL, 0, ThreadWaitAccept, this, 0, 0); //线程方式运行（非阻塞进程）

	return 0;
}

int CCommServer::SendData(DWORD dwLen,DWORD dwFlag,char *pBuff)
{
    //防止多线程调用出错
    CLocalLock lock(&m_lockSend);

	DWORD dwTotalLen = dwLen + 8;
	if(m_ClientSocket != INVALID_SOCKET)
	{
		char *buffer = new char[dwTotalLen];
		memcpy(buffer,&dwTotalLen,sizeof(DWORD));
		memcpy(buffer + sizeof(DWORD),&dwFlag,sizeof(DWORD));
		memcpy(buffer + sizeof(DWORD)*2,pBuff,dwLen);
		int nRet = send(m_ClientSocket,buffer,dwTotalLen,0);	//发送数据

		delete []buffer;
		return nRet;
	}
	return -1;
}


//发送数据到服务进程
int CCommServer::SendData(TCHAR *pData,int nLen)
{
	//防止多线程调用出错
	CLocalLock lock(&m_lockSend);
	
	if(m_ClientSocket == INVALID_SOCKET )return -1;

	DWORD dwLen = (nLen + 1 + sizeof(DWORD)) * sizeof(TCHAR);
	TCHAR *buff=new TCHAR[dwLen];
	ZeroMemory(buff, dwLen * sizeof(TCHAR));

#ifdef _UNICODE
	memcpy_s((char*)buff, sizeof(DWORD), (char*)&dwLen, sizeof(DWORD));
	wmemcpy_s(&buff[sizeof(DWORD)], dwLen - sizeof(DWORD) - sizeof(TCHAR), pData, nLen);
#else
	memcpy(buff, sizeof(DWORD), &dwLen,sizeof(DWORD));
	memcpy(&buff[sizeof(DWORD)], dwLen - sizeof(DWORD) - sizeof(TCHAR), pData,nLen);
#endif

	int nSendByte = send(m_ClientSocket,(char*)buff,dwLen,0);

	delete []buff;

	return nSendByte;
}

int CCommServer::SendData(CStdString strData)
{
	char *pSend = NULL;
	DWORD dwLen = 0;
	int iLen = strData.GetLength();

	return SendData(strData.GetBuffer(), iLen + 1);
}

bool CCommServer::IsClientConnected() const
{
	if (m_ClientSocket != INVALID_SOCKET )
		return true;

	return false;
}


bool CCommServer::ReRunServer()
{
	int iPortCount = m_vIPort.size();
	for (int i = 0; i < iPortCount; i++)
	{
		if (ReInit(m_vIPort[i]) == 0)
		{
			LOG_INFO(g_logger, "重新监听端口:" << m_vIPort[i]);
			return true;
		}
		i++;
	}
	LOG_INFO(g_logger, "重新绑定监听端口失败");

	return false;

}
int CCommServer::ReInit(int nPort)
{
	int errid;

	// 创建socket
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_socket == INVALID_SOCKET) {
		errid = WSAGetLastError();
		printf("socket() 错误: %ld\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	// 绑定socket
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("127.0.0.1");
	service.sin_port = htons(nPort);

	if (::bind(m_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		::MessageBox(0, _T("客户端运行时发现一个错误,\r\n请检查是否有第三方软件(如:防火墙)禁止了Mcproc.exe权限\r\n"), _T("\n运行时检测"), MB_OK | MB_ICONWARNING);

		CStdString strErro;
		strErro.Format(_T("bind 端口：%d ,错误码:%d "), nPort, GetLastError());
		::MessageBox(0, strErro.GetBuffer(0), _T("\n运行时检测"), MB_OK | MB_ICONWARNING);
#endif
		closesocket(m_socket);
		WSACleanup();

		return -1;
	}

	// 监听socket
	if (listen(m_socket, 5) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		::MessageBox(0, _T("客户端运行时发现一个错误,\r\n请检查是否有第三方软件(如:防火墙)禁止了Mcproc.exe权限\r\n"), _T("\n运行时检测"), MB_OK | MB_ICONWARNING);
#endif
		//g_log.Trace(LOGL_TOP,LOGT_ERROR,__TFILE__, __LINE__,_T("listen( m_socket, 1 ) == SOCKET_ERROR"));
		closesocket(m_socket);
		WSACleanup();

		return -1;
	}

	/*BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	if (setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bOptVal, bOptLen) != SOCKET_ERROR)
	{
	//g_log.Trace(LOGL_TOP,LOGT_ERROR,__TFILE__, __LINE__,_T("设置发送KEEPALIVE, error:%d"), WSAGetLastError());
	}*/

	//设置发送超时(10秒）
	int iTimeOut = 1000 * 50;
	//int iTimeOut = 1000*100;
	if (setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeOut, sizeof(int)) == SOCKET_ERROR)
	{
		//g_log.Trace(LOGL_TOP,LOGT_ERROR,__TFILE__, __LINE__,_T("设置发送超时错误, error:%d"), WSAGetLastError());
	}

	m_ClientSocket = INVALID_SOCKET;
	m_bRun = TRUE;

	WaitRecData(TRUE);
	return 0;
}
