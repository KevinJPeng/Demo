/*******************************************************************
* 文件说明：关键字排名云任务处理类
********************************************************************/
#ifndef _CTASKQUICKPHOTO_H_
#define _CTASKQUICKPHOTO_H_

#include "CommClient.h"
#include "TaskBase.h"
#include "YunTaskStreamCtr.h"
#include <shellapi.h>
#include "ALiOssApi.h"


class CTaskQuickPhoto: public CTaskBase
{
public:
	CTaskQuickPhoto(CTaskThread *pTaskThread, CYunTaskStreamCtr *pYunTaskStreamCtr);
	~CTaskQuickPhoto(void);

	//判断是否需要发送通告请求任务
	virtual bool IsSendNitify();

	//设置任务数据
	virtual void SetData(const T_TASK_DATA &tData);

	//返回任务数据
	virtual T_TASK_DATA GetData(void);

	//初始化对象操作
	virtual bool Init(void);

	//检测任务是否可以立即执行
	virtual bool CanExecNow();

	//执行任务
	virtual DWORD Exec(void);

	// 请求下个云任务
	virtual void RequestNextTask();

	//停止任务
	virtual void Stop();

	//向服务器提交执行结果
	virtual bool SendResultToServer(void);

	//通知任务结束的回调函数
	virtual void SetTaskMgr(CTaskMgr *pMgr);

	//处理收到的结果数据
	virtual void OnReceive(CString strData);

	//获取任务的标识信息
	virtual CString GetTaskInfo(void);

	//等待接收线程结束
	virtual void WaitExit();

	// 判断任务超时 
	virtual bool IsTimeOut();

	//获取超时标记
	virtual bool GetTimeOutFlag();

	//获取当前任务类型
	virtual int GetCurrTaskType();

	//释放内存通告
	virtual void ReleaseTask();

	virtual void WaitForRunningEvent();

	virtual void SetEvent();

	virtual BOOL IsRunningSendResult();

	//过滤抓取的关键词排名数据
	CString FilterData();

	//组合过滤的关键词
	CString CombineQuickPhotoData();

	//缓存快照数据和快照名称
	void CacheDataAndQuickPhotoName(CString strData);

	/*
	@brief 将html上传到服务器
	@param [in/out]html路径
	*/
//	bool SaveHtmlToServer(CString &strFilePath);

	/*
	@brief 将压缩的zip文件上传到ftp服务器
	@param [in/out]压缩的zip文件路径
	*/
//	bool SaveZipToServer(CString &strFilePath);

	// Ftp上传
//	bool FtpUpload();

	// Http上传
//	bool HttpUpload();
	// Http上传
	bool HttpUpload2();
	CString GetResponseErrorCode(const char *pJson);
	// 利用阿里云api上传
	bool ALiYunAPIUpload();
	bool parseOssAccessInfo(const char *pJson, OssAccessInfo& _ossAccessInfo);


	//把文件上传到服务器
	bool UploadFileToServer(void);

	//判断是否需要上传快照
	bool WhehterSendToServerIsRequire(void);

	//获取要上传的文件路径
	void GetUploadFilePath(CString &strUploadPath, CString strMode);

	//读取mcconfig配置文件获取ftp的登录信息
//	void GetFtpLoginInfo(const CString &strIniPath);

	//读取mcconfig配置文件获取http的登录信息
	void GetHttpLoginInfo(const CString &strIniPath);

	//遍历快照目录下的所有公司名目录并把目录名存进vector中
	void FindAddSaveCompanyNameDir(vector<LPCTSTR> &vComNameDir);

	//删除快照文件下的所有子目录和文件
	bool DeleteAllFiles(LPCTSTR szDirPath);

	void GetFtpDesCode(TCHAR *code, TCHAR *ip, TCHAR *account, TCHAR *psd, int &port);

	void GetHttpDesCode(TCHAR *code, TCHAR *pHome, TCHAR *pFileUpload, TCHAR *account, TCHAR *psd);

	char *ToMultiByte(LPCWSTR pszSource);

	bool CheckUpdateRankModule(int nWaitTime);

private:

	bool m_bStopWebc;                             //主控是否被正常终止
// 	bool m_bUseFtpZipUpload;                      //是否使用ftp上传zip包
// 	bool m_isFtpUpload;                           //是否是ftp上传文件
// 	bool m_bUploadZipFromBreak;                   //上传zip包是否使用断点续传
	bool m_bIsPrintMCLog;						  //控制打印主控已经存在日志,只打印一次,避免循环打印日志
	bool m_bUsehttpZipUpload;                   //是否使用http上传zip包
	bool m_bUseOssSDKUpload;                    //是否使用oss c-sdk上传文件

	DWORD                 m_dwExecTime;           //开始执行时间(用于判断任务是否超时)

	HANDLE                m_hProcess;             //执行任务的进程句柄
	CTaskMgr*             m_pMgr;
	CCommClient           m_comm;
	CTaskThread*          m_pTaskThread;
	CInternetHttp         g_http;
	CYunTaskStreamCtr*    m_pYunTaskStreamCtr;

	vector<CString>       m_vPageName;            //存放快照名字
	map<CString, CString> m_mapPageName;          //key:快照名字 value:单条快照数据 
	vector<CString>       m_vCacheData;           //缓存返回的快照数据
	vector<CString>       m_vCacheUploadFailData; //缓存上传失败的单条快照数据

	T_TASK_DATA           m_tTaskData;            // 任务数据
// 	T_CONN_INFO           m_ftpLoginInfo;         //ftp登录信息的结构体
 	T_LOGIN_INFO          m_httpLoginInfo;        //http登录信息的结构体(上传.zip快照)
	T_LOGIN_INFO          m_httpLoginInfo2;        //http登录信息的结构体(获取OSS登陆信息)
	BOOL                  m_bIsRunningSendResult; //是否正在提交结果等操作
	HANDLE				  m_hRunning;
	BOOL				  m_bIsTaskTimeOut;
	BOOL				  m_bIsMcException;	    //任务是否正常结束(如主控异常断开就属于此种情况)
	OssAccessInfo		m_ossInfo;
};

#endif // _CTASKQUICKPHOTO_H_