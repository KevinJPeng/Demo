//////////////////////////////////////////////////////////////////////////////////////
//
// 版权所有(C), 2013, 商讯网信息有限公司
// 版本：
// 文件说明：SOCKET服务端类 通过回调函数处理收到的数据
// 生成日期：
// 作者：何培田
//
// 修改历史：
// 1. 日期：
//   作者：
//   修改内容：
// 2. 
//

#ifndef _CSERVERSOCKET__
#define _CSERVERSOCKET__

#include "IServerSocket.h"
#include "StdString.h"
#include "Trace.h"
#include "Lock.h"

#pragma comment(lib, "wsock32.lib")
 
typedef DWORD(*pfnDataHandler)(SOCKET& _ClientSocket, TCHAR *Orgbuf, DWORD dwTotalLen);

class CCommServer : public IServerSocket
{
public:
	CCommServer(void);
	~CCommServer(void);

	int Init(int nPort, pfnDataHandler fn);	//初始化
	int UnInit();	//解除初始化
	int CloseClient() ;  //关闭ClientSocket
	int SendData(DWORD dwLen,DWORD Flag,char *buff);
	int SendData(TCHAR *buff, int dwLen);
	int SendData(CStdString strData);
	bool  IsClientConnected() const;

	bool ReRunServer();
	int ReInit(int nPort);	//重新初始化

	void SetPort(vector<int> _vIport);
	friend DWORD WINAPI ThreadRecv(PVOID param);

	friend DWORD WINAPI ThreadWaitAccept(PVOID param);
	friend DWORD WINAPI MThreadRecv(PVOID param);

public:
	SOCKET m_socket;
	SOCKET m_ClientSocket;
	int m_iPort;
	HANDLE m_eventListen;
	BOOL m_bRun;

private:
	HANDLE m_hThread;
	pfnDataHandler m_pfnDataHandler;        //收到数据包之后的回调函数
	CLock m_lockSend;                     //发送数据的互斥锁
	vector<int> m_vIPort;

	int WaitClient(); //等待客户端连接
	int WaitRecData(BOOL IsThread);	//接收数据

	//获取http协议通过post方法发过来的数据
	bool GetHttpPostData(TCHAR* _pRecvbuff, TCHAR*& _pParsebuff, DWORD& _len);
	//获取http协议通过get方法发过来的数据
	bool GetHttpGetData(TCHAR* _pRecvbuff, TCHAR*& _pParsebuff, DWORD& _len);


	/*
	@brief  多字节转宽字符
	@param  要转化的多字符串
	@return 返回宽字节
	*/
	wchar_t * MultitoWide(const char *pMulti, DWORD dwCode = CP_ACP);

};

typedef struct _ThreadParam
{
	CCommServer *Server;
	SOCKET ClientSocket;

	_ThreadParam()
	{
		Server = NULL;
		ClientSocket = INVALID_SOCKET;
	}

	~_ThreadParam()
	{
		Server = NULL;
		ClientSocket = INVALID_SOCKET;
	}
}ThreadParam, *pThreadParam;


#endif