//////////////////////////////////////////////////////////////////////////////////////
//
// 版权所有(C), 2013, 商讯网信息有限公司
// 版本：
// 文件说明：SOCKET客户端类 通过回调函数处理收到的数据，用于向主控发送ANSI字符数据
// 生成日期：
// 作者：何培田
//
// 修改历史：
// 1. 日期：2013/12/5
//   作者：何培田
//   修改内容：收到数据后，直接转发给CTaskBase的OnReceive处理，
//             解决通过函数指针作为回调时无法同时执行多个自动任务的问题。
// 2. 
//

#pragma once

class CTaskBase;
// typedef DWORD (*pfnDataHandler)(char *Orgbuf, DWORD dwTotalLen);

class CCommClient
{
public:
	CCommClient(void);
	~CCommClient(void);

	int Init(int iPort, CTaskBase *pBase);
	int SendData(CStdString strData);

	void Stop();   //停止数据接收线程
	bool StopIsSet(void);  //检测是否设置了停止标记

	DWORD OnReceive(char *Orgbuf, DWORD dwTotalLen);

	// 等待线程结束 add by zhoulin
	void WaitReceiveExit();

private:
	SOCKET m_socket;
	HANDLE m_hThread;
	bool m_bStop;
	CTaskBase *m_pTaskBase;

	//pfnDataHandler m_pfnDataHandler;        //收到数据包之后的回调函数
// 	pfnDataHandler *m_pfnDataHandler;        //收到数据包之后的回调函数

private:
	int Connect(int iPort);
	DWORD DisConnect(void);
	DWORD CreateRecvThread(void);

	/*
	@brief  多字节转宽字符
	@param  要转化的多字符串
	@return 返回宽字节
	*/
	wchar_t * MultitoWide( const char *pMulti, DWORD dwCode = CP_ACP );

	//	int SetDataHandler(pfnDataHandler fn);

	static DWORD WINAPI StaticThreadFunc(LPVOID lpParam);
};
