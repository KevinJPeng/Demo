#pragma once
#include <WinSock2.h>
#pragma  comment(lib,"WS2_32")

#include <vector>
using std::vector;
#define   RECV_BUF_LEN 1024 //接收命令字符数组长度

class CFTPSocket
{
public:
	CFTPSocket(void);
	~CFTPSocket(void);
protected:
	SOCKET m_ControlSocket;
	SOCKET m_DataSocket;
	char *m_pszConnectServerIp;
	char *m_pszDataServerIp;//服务器被动传输数据IP
	USHORT m_nPort;//服务器被动传输数据端口
	char m_szBuf[RECV_BUF_LEN];//接收命令回复数组
	char m_szPasvString[RECV_BUF_LEN];//被动模式下接受 PASV返回 IP和端口号的字符
protected:
	bool SendControl(const char * pszData);//给服务器发送命令
	bool  RecvControl(const char * pszResult);//接收命令回复
public:
	bool InitFtp(char *sIP, int iPort,int nTimeOut, char *sUsr, char *sPwd);//初始化并连接Ftp服务器
	void clearFtp();

	bool SetConnectServerIp(char * pszServerIp);//设置服务器Ip地址 用于连接服务器
	bool CreateControlSocket(u_short nServerPort , int nTimeOut);//创建命令套接字并连接服务器
	bool CreateDataSocket();//创建传输套接字 并连接服务器
	BOOL GetIPAndPortByPasvString(char * lpszPasvString, OUT char * csIP, OUT USHORT &nPort);
	bool SendUSER(char * pszUserName);//发送用户名 成功返回真
	bool SendPASS(char * pszPassWord);//发送密码  成功返回真

	
	bool SavePasvStr( );//保存PASV命令返回的字符数组
	bool SendAndRecv(const char * pszCMD,char * pszResult = NULL );//发送命令并判断是否成功执行

	//nTimes为创建数据套接失败次数,nTimeSleep创建套接字间隔
	bool Download(char * pszFileName,char *pszSavePath,ULONGLONG uRestPos);
	bool RecvAndWriteFile(char* pSavePath,ULONGLONG iSeekPos,ULONGLONG iFileSize);
	ULONGLONG  GetSize(char * pszStr);
	void  CloseCtrlSocket();
	void CloseDataSocket();
public:
	static int DATA_TRY_TIMES;//重试次数
	static int DATA_TRY_INTERVAL;//重试间隔 
};


