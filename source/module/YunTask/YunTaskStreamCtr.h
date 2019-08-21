/*************************************************
* 文件说明：云任务流控制基类,主要负责从服务器
*           请求任务，把任务压入任务缓存，通告
*           YunTaskMgr执行任务缓存里的任务      
*           等等（对原关键字排名模块重构）
* 时间    ：2015/8/14
* 作者    ：周林
*************************************************/


#ifndef _YUNTASKSTREAMCTR_H_
#define _YUNTASKSTREAMCTR_H_

#include "resource.h"
#include "..\..\common\tinyxml.h"
#include "Taskmgr.h"
#include <WinVer.h>
#pragma comment(lib, "Version.lib")

#define ERRCODE_SUCCESS   0
#define ERRCODE_FAIL      1
#define ERRCODE_NO_TASK   2

class CTaskThread;

class CYunTaskStreamCtr
{
public:
	CYunTaskStreamCtr(CTaskThread* pTaskThread);
	~CYunTaskStreamCtr();

public:
	// 请求任务回调函数,iType用于请求何种任务
	bool NotifyRequestTask(int iType);

	// 初始化配置
	bool InitCfg();

	//初始化其他参数
	void InitOtherVar();

	// 根据类型获取请求URL地址
	CString GetURLAddress(int iType);

	// 保存用户信息
	void SetUserInfo(CString strUserName, int iVersionId);

	// 执行云任务请求任务
	bool StartExecYunTaskRequest();

	// 停止任务
	bool StopTask();

	//获取用户对应产品下的普通任务
	void RequestYunTask(int iType);

	//准备自动任务,完成后发送通告执行任务
	bool PrepareTask(T_DATA_FROM_SERVER &tData, bool bType);

	//从服务器请求自动任务数据
	int GetTaskData(CString strURL, T_DATA_FROM_SERVER &tData);

	//从配置文件请求任务数据(debug keyword)
	int GetTaskDebugData(T_DATA_FROM_SERVER &tData, int itype);
	
	//根据任务类型iType,获取调试数据
	void GetDebugIniData(int itype);

	//解析从服务端请求到的自动任务数据
	bool ParseServerResponse(TiXmlDocument *pDoc, T_DATA_FROM_SERVER &tTaskInfo, CString strVersionId);

	//获取相关任务的超时时间(min) 0:抓取快照任务 1:普通任务
	int GetOutTime(int iType);

	//是否需要提交排名数据 debug 数据不需要提交
	BOOL IsNeedSumitResult();

	//是否可以请求相关任务数据，根据当前的执行的任务轮数与需要等待的轮数是否相等来判断
	BOOL CanRequestTask(int iType);

	//根据配置文件中的请求标志，itype类型的任务是否可以请求数据
	BOOL MathCanRequestType(int iType);

	//一轮任务结束之后如果有任务失败的情况，则当前的任务轮数应该要递增
	void AddTaskRoundCount();

	//如果任务满足等待请求条件，则重置相关执行轮数
	void ResetRoundCount(int Type);

	//如果任务之前有超时或提交而造成的轮数等待，在后续任务正常之后，相关等待置零
	void SetRoundCount(int Type);

public:
	//获取云推广任务是否为抢占点任务标记
	BOOL GetRunObjTaskFlag();

	//系统空闲时间是否满足配置
	BOOL IsInputIdleOk();

	//检测是否启用锁屏或者是屏保
	BOOL IsLockScreen();
	
	//判断用户当前系统是否为为win10,由于为为win10系统锁屏状态无法判断准确，所以统一请求非强制焦点任务
	BOOL IsWin10SystemVersion();

	//获取当前推广任务类型是否为抢占焦点类型
	BOOL IsObjTaskRunning();

	//设置系统输入检测停止标记
	void StopCheckThread();

	//提交数据加密
	DWORD EncryptRequData(CString & strDest);

public:
	//解密获取的用户名
	CString DecodeString(CString& strDest);

	//Rul编码
	CString  URLEncode(CString sIn);

	//宽字节转多字节
	char* CStringToMutilChar(CString& str, int& chLength);

	//获取客户公网IP所在地址
	CString GetAreaByIp();

	//通过新浪接口获取IP区域信息 add by qy -- 20170927
	BOOL	IsNeedToUpdateArea();	//是否需要重新获取ip区域信息
	CString GetArea();				//获取区域
	BOOL	IPAreaQuery(CString &strArea);	//通过新浪接口获取IP区域信息
	BOOL	IsInAreaList(CString &strArea);	//判断区域是否在区域列表中
	bool	SplitCString(const CString & input, const CString & delimiter, std::vector<CString >& results);	//分割字符

	//宽字节转utf_8
	char* CStringToUtf8Char(CString& str, int& chLength);

	BYTE ToHex(const BYTE &x);

public:
	// 商铺流量服务器
	CString m_strShopTrafServer;
	// 关键词排名服务器
	CString m_strKeyWordServer;
	// 信息刷新服务器
	CString m_strRefreshServer;
	//云推广网站服务器地址
	CString m_strMainTaskServer;
	//保存请求的任务类型
	int m_iTypeTask; 
	//定时时间(分钟为单位)
	int m_iTime;
	//定时时间（old)
	int m_iOldTime;
	//抓取快照任务超时时间
	int m_iTaskKeyWordTimeOut;
	//普通任务超时时间
	int m_iTaskGeneralTimeOut;
	//任务失败等待轮数结构体
	T_LIMIT_REQUEST_COUNT m_tRoundCount;
	//是否请求抢占焦点标记(此标记内部使用，忽略相关限制规则)
	BOOL m_bIsRequestMainObjTask;
	//任务数据请求标记(可限制任务是否执行)
	CString m_strTaskRequestFlag;
	//与内核模拟鼠标及模拟键盘互斥
	HANDLE  m_hSystemInputMuext;
	//检测线程停止标记
	BOOL  m_bCheckStop;
	//连续X分钟(可配，默认60分钟)未检测键盘鼠标事件触发云推广任务
	int m_iSystemInputIdle;
	//记录用户最后输入时间
	BOOL m_dwLastInputTime;
	//当前请求的推广任务是否为抢占焦点任务
	BOOL m_bIsObjTask;
	//系统是否为为win10或以上
	BOOL m_bIsWin10Version;
	//获取本机IP和区域的网址
	CString m_strGetPublicIpUrl;
private:
	// 请求其他任务线程
	static DWORD WINAPI ThreadProcReqYunTask(LPVOID lpParaSWmeter);
	// 接受通告的线程
	static DWORD WINAPI ThreadProcNotify(LPVOID lpParameter);
	//循环检测用户输入行为及用户屏幕锁定及屏保状态
	static DWORD WINAPI ThreadProcCheckState(LPVOID lpParameter);

	CString m_strUserName;           //保存未解密的当前客户端用户名
	CString m_strUseAccount;         //保存解密的当前客户端的用户名
	CString m_strVersinId;

	CLock*       m_pLockRequest;          
	CLock*       m_pLockInfo;
	CLock*       m_pLockRoundCount;
	CLock*		 m_pCheckIdle;
	CLock*		 m_pCheckLockScreen;

	CTaskMgr*    m_objTaskMgr;
	CTaskThread* m_pTaskThread;

	T_DATA_FROM_SERVER m_tTaskInfo;
	BOOL			   m_bIsSubmitResult;		 //是否需要提交关键词排名数据
	CStdString		   m_strDebugTaskData;		 //任务调试数据
	HANDLE			   m_hCheckThread;			 //保留系统空闲检测线程句柄

	CArray<SIPQueryAPI>		m_ipQueryAPIArray;	//ip地址查询集合
	CInternetHttp			m_internetHttp;
	std::vector<CString >	m_strAreaArray;		//配置文件区域列表
	CString					m_strAreaWriteTime;	//配置文件中区域信息写入时间
	CString					m_strArea;			//区域
};



#endif // _YUNTASKSTREAMCTR_H_