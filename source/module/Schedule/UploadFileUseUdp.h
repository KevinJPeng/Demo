#include "ScheduleBase.h"
#include "CommClient.h"

const int nSendDataMaxLen = 1024;

class CUploadFileUseUdp
{
public:
	CUploadFileUseUdp();
	~CUploadFileUseUdp();

public:
	/*
	@brief: 执行udp通信
	@param: 要发送的数据
	@return: succ is true
	*/
	bool Exec(const CString& strSendData);

	/*
	@brief: 初始化，读取配置文件中的端口号和服务器地址
	@param: null
	@return: null
	*/
	void Init(void);

	/*
	@brief: 连接和发送数据
	@param: 要发送的数据
	@return: succ is true
	*/
	bool ConnectAndSendData(const CString& strSendData);

	/*
	@brief: 发送数据
	@param: [in]要发送的数据
	@return: succ is strData.getlenth()， fail is -1
	*/
	int SendData(CString &strData);

	/*
	@brief: 接收服务端发过来的数据
	@param: null
	@return: succ is true
	*/
	bool OnReceive(void);

	/*
	@brief: 获得接收到的数据
	@param: null
	@return: 接收到的数据
	*/
	const CString& GetReciveData(void);


private:
	int Connect(int iPort, const char *pIp);
	DWORD DisConnect(void);

private:
	SOCKET m_socket;
	sockaddr_in m_service;
	CString m_strRecvData;
	char *m_pIp;
};