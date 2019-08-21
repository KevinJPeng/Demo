#pragma once
#include "CommClient.h"
#include "TaskBase.h"
#include "AutoGetKeywordPages.h"


class CAutoQuickPhoto: public CTaskBase
{
public:
	CAutoQuickPhoto(CTaskThread *pTaskThread, CAutoGetKeywordPages *pAutoGetKeywordPages);
	~CAutoQuickPhoto(void);

	//设置任务数据
	virtual void SetData(const T_TASK_DATA &tData);

	//返回任务数据
	virtual T_TASK_DATA GetData(void);

	//初始化对象操作
	virtual bool Init(void);

	//检测任务是否可以立即执行
	virtual bool CanExecNow(DWORD dwEnterTickCount);

	//执行任务
	virtual DWORD Exec(void);

	//停止任务
	virtual void Stop(void);

	//向服务器提交执行结果
	virtual bool SendResultToServer(void);

	//通知任务结束的回调函数
	virtual void SetTaskMgr(CTaskMgr *pMgr);

	//处理收到的结果数据
	virtual void OnReceive(CString strData);

	//获取任务的标识信息
	virtual CString GetTaskInfo(void);

	//获取统计信息
	CString GetStatisticalInfo(int &nKeywordAcount, int &nFirstPageOfBaiduAcount, int &nFirstPageAcount, int &nFirstThreePagesAcount);

	//过滤抓取的关键词排名数据
	CString FilterData(CString strMod);

	//本地统计数据
	void StatisticalInfoInLoacl(void);

	//组合过滤的关键词
	CString CombineQuickPhotoData(CString strMode);

	//缓存快照数据和快照名称
	void CacheDataAndQuickPhotoName(CString strData);

	/*
	@brief 将html上传到服务器
	@param [in/out]html路径
	*/
	bool SaveHtmlToServer(CString &strFilePath);

	/*
	@brief 将压缩的zip文件上传到ftp服务器
	@param [in/out]压缩的zip文件路径
	*/
	bool SaveZipToServer(CString &strFilePath);


	//把文件上传到服务器
	bool UploadFileToServer(void);

	//判断是否需要上传快照
	bool WhehterSendToServerIsRequire(void);

	//获取要上传的文件路径
	void GetUploadFilePath(CString &strUploadPath, CString strMode);

	//读取mcconfig配置文件获取ftp的登录信息
	void GetFtpLoginInfo(const CString &strIniPath);

	//读取mcconfig配置文件获取http的登录信息
	void GetHttpLoginInfo(const CString &strIniPath);

	void GetFtpDesCode(TCHAR *code, TCHAR *ip, TCHAR *account, TCHAR *psd, int &port);

	void GetHttpDesCode(TCHAR *code, TCHAR *pHome, TCHAR *pFileUpload, TCHAR *account, TCHAR *psd);
	//写整型数据到配置文件进去
	//void WritePrivateProfileInt(int nInfo, CString strSection);

private:
	T_TASK_DATA m_tTaskData;
	//HANDLE m_hProcess;                         //执行任务的进程句柄
	TCHAR m_tszDataCacheFile[MAX_PATH];        //缓存执行结果的INI文件名
	CString m_strCacheDataInSuccess;            //执行结果的缓存数据(返回状态为成功时提交的数据)
	CString m_strCacheDataInFail;               //执行结果的缓存数据(返回状态为失败时提交的数据)
    CHttpUtils m_http;
	vector<CString> m_vPageName;               //存放快照名字
	map<CString, CString> m_mapPageName;        //key:快照名字 value:单条快照数据 
	vector<CString> m_vCacheData;               //缓存返回的快照数据
	vector<CString> m_vCacheUploadFailData;     //缓存上传失败的单条快照数据
	bool m_bUseFtpZipUpload;                    //是否使用ftp上传zip包（测试用）
	bool m_isFtpUpload;                         //是否是ftp上传文件
	bool m_bUploadZipFromBreak;                 //上传zip包是否使用断点续传
	T_CONN_INFO m_ftpLoginInfo;                 //ftp登录信息的结构体
	T_LOGIN_INFO m_httpLoginInfo;               //http登录信息的结构体

	//int m_nReceivedRecord;                     //接收到的记录数
	//int m_nThresholdValToSubmit;               //提交阀值
	int m_nFirstPageOfBaiduSum;                 //百度首页排名总量
	int m_nFirstPageSum;                        //首页排名总量
	int m_nFirstThreePagesSum;                  //前三页排名总量
	int m_nKeywordSum;                          //抓取关键词总量
	bool m_bStopWebc;                        //主控被正常终止
	int m_nJudgeWebcDisconnected;            //返回给上层判断主控是否异常断开，1表示异常断开，2表示正常退出

	CTaskMgr *m_pMgr;
	CCommClient m_comm;

	CTaskThread *m_pTaskThread;
	CAutoGetKeywordPages *m_pAutoGetKeywordPages;
};

