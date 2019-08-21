#include "stdafx.h"
#include "UploadFileUseUdp.h"


CLogTrace g_UdpLog(_T("TaskUdp.log"),NULL);

CUploadFileUseUdp::CUploadFileUseUdp()
{
	
}

CUploadFileUseUdp::~CUploadFileUseUdp()
{
	DisConnect();
}


bool CUploadFileUseUdp::Exec(const CString& strSendData)
{
	Init();

	if (!ConnectAndSendData(strSendData))
	{
		return false;
	}

	if (!OnReceive())
	{
		return false;
	}

	return true;
}


void CUploadFileUseUdp::Init(void)
{
	CString strCfgFile = _T("");
	CString strIp = _T("");
	DWORD dwLen = 64;
	m_pIp = new char[dwLen];
	memset(m_pIp, 0, sizeof(m_pIp));

	strCfgFile.Format(_T("%s\\data2\\YunTask.dat"), g_pGlobalData->dir.GetInstallDir());
	g_UdpLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__, __LINE__, _T("读服务器地址和端口的配置文件路径: %s! "), strCfgFile);

	g_iniCfgFile.init(strCfgFile);
	g_iniCfgFile.ReadString(_T("YunTask"),_T("ConnectUseUdp"), _T("Ip"), strIp, _T("103.25.23.10"));

	WCharToMByte(strIp.GetBuffer(), m_pIp, &dwLen);

}


bool CUploadFileUseUdp::ConnectAndSendData(const CString& strSendData)
{
	CString strData = strSendData;
	int iPort = 40001;
	const char *pIp = m_pIp;

	if (-1 == Connect(iPort, pIp))
	{
		return false;
	}

	SendData(strData);

	return true;
}


//连接到服务端
int  CUploadFileUseUdp::Connect(int iPort, const char *pIp)
{
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR )
	{
		return -1;
	}

	// 创建socket
	m_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( m_socket == INVALID_SOCKET )
	{
		WSACleanup();
		return -1;
	}

	m_service.sin_family = AF_INET;
	m_service.sin_addr.s_addr = inet_addr(pIp);
	m_service.sin_port = htons(iPort);

	g_UdpLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Connect %d"), iPort);
	DWORD dwBeginTime = GetTickCount();
	while (connect(m_socket, (SOCKADDR*)&m_service, sizeof(m_service)) == SOCKET_ERROR)
	{
		Sleep(60);
		if (GetTickCount() - dwBeginTime > 20000)
		{
			g_UdpLog.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("Connect timeout!"));
			return -1;
		}
	}

	g_UdpLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Connect success"));

	return 0;
}

//与服务端断开连接
DWORD CUploadFileUseUdp::DisConnect(void)
{
	if (m_socket != NULL)
	{
		g_UdpLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("closesocket"));
		closesocket(m_socket);
		m_socket = NULL;
	}

	if (m_pIp != NULL)
	{
		delete []m_pIp;
		m_pIp = NULL;
	}
	WSACleanup();

	return 0;
}


//发送数据到服务进程
int CUploadFileUseUdp::SendData(CString &strData)
{
	if(m_socket == NULL)return -1;

	char *pSendData = NULL;
	DWORD dwLen = 0;

	pSendData = new char[nSendDataMaxLen];
	if (NULL == pSendData)
		return -1;

	memset(pSendData, 0, nSendDataMaxLen);

#ifdef _UNICODE
	dwLen = nSendDataMaxLen;
	WCharToMByte(strData.GetBuffer(), pSendData, &dwLen);
#endif

	int nSendByte = sendto(m_socket, pSendData, dwLen, 0, (SOCKADDR*)&m_service, sizeof(m_service));

	g_UdpLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("发送给服务端的的数据为：%s"), strData);

	delete []pSendData;

	return nSendByte;
}


bool CUploadFileUseUdp::OnReceive()
{
	char buff[MAX_PATH] = {0};
	DWORD dwLen = sizeof(m_service);
	
	//设置接收超时
	int timeout = 10000;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	int ret = recvfrom(m_socket, buff, MAX_PATH, 0, (SOCKADDR*)&m_service, (int *)&dwLen);

	if (ret == -1)
	{
		DWORD dwErr = WSAGetLastError();
		g_UdpLog.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("接收失败！err：%d"), dwErr);
		return false;
	}
	m_strRecvData = buff;

	g_UdpLog.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("接收成功，接收到的数据为：%s"), m_strRecvData);

	return true;
}


const CString& CUploadFileUseUdp::GetReciveData()
{
	return m_strRecvData;
}


