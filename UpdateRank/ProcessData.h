#ifndef _PROCESSDATA_H_
#define _PROCESSDATA_H_


#include "ServerSocket.h"

class CProcessData  
{

	friend class CServerSocket;
public:  
	CProcessData();
	virtual ~CProcessData();
public:
	DWORD CovertBufData(char *Orgbuf,DWORD dwTotalLen);
	
	DWORD SetPtrOfServer(CServerSocket *pServerSocket);
private:

	DWORD CovertBufDataWeb(char *Orgbuf,DWORD dwTotalLen);
 	void GetSearchPhoto(const CStdString &strData);         //抓取各搜索引擎快照 
	void InitDebugUrl();
	void InitDelayTime();
	void InitCfgInfo();
	bool AllisNum(CString str);
public:
	CServerSocket *m_pServerSocket;
	BOOL bAlreadySend;  //已经发送过数据，后期再有数据就是错误
};

#endif // !_PROCESSDATA_H_
