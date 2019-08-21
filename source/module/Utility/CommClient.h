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
//   修改内容：收到数据后，直接转发给CAutoTaskBase的OnReceive处理，
//             解决通过函数指针作为回调时无法同时执行多个自动任务的问题。
// 2. 日期：2014/4/18
//   作者：何培田
//   修改内容：将CCommClient的回调函数封装到接口ICommCallback


#pragma once

class ICommCallback;

class CCommClient
{
public:
	CCommClient(void);
	~CCommClient(void);

	int Init(int iPort, ICommCallback *pBase);
	int SendData(CString strData);

	void Stop(void);   //停止数据接收线程
	bool StopIsSet(void);  //检测是否设置了停止标记

	DWORD OnReceive(char *Orgbuf, DWORD dwTotalLen);

	//获取一个当前可用的端口号，用于启动主控
	int  GetPort(int nDefault = 28016);

private:
	SOCKET m_socket;
	HANDLE m_hThread;
	bool m_bStop;
	ICommCallback *m_pCommCallback;

private:
	int Connect(int iPort);
	DWORD DisConnect(void);
	DWORD CreateRecvThread(void);

	static DWORD WINAPI StaticThreadFunc(LPVOID lpParam);
};
