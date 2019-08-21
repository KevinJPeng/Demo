#pragma once

#include "stdafx.h"
#include "UpdateWnd.h"
#include "LoginWnd.h"
#include "NewVersionWnd.h"
#include "UpdateErrorWnd.h"
#include "UpdateTipsWnd.h"
#include "TrayWnd.h"
#include "WarningWnd.h"
#include  "PostMsgWnd.h"
#include "SettingWnd.h"
#include "ClientSettingWnd.h"
#include "SafeExit.h"
#include "ShowChart.h"
#include "CheckProWnd.h"
#include "ZhenCiWnd.h"

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL(WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
#define PRODUCT_PROFESSIONAL	0x00000030
#define VER_SUITE_WH_SERVER	0x00008000

class CMainWnd :
	public WindowImplBase
{
public:
	CMainWnd(void);
	~CMainWnd(void);
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	virtual void InitWindow();
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	void Notify(TNotifyUI& msg);
	void OnPrepare(TNotifyUI& msg);
	void OnExit(TNotifyUI& msg);
	void SetClientType(DWORD dwType);
	void setTrayTips(CString strTipContent, CString strTipTitle, CString strTipInfo, int ioutTime = 8000);
private:
	//相关控件
	CThreadManage m_threadMgr;
	CUiHelper *m_pUiHelper;
	CProgressUI *m_pProgressTest;
	CButtonUI *m_pBtnChkUpdate;
	CButtonUI *m_pBtnLogin;		
	CWebBrowserUI *m_pActiveXFlash;
	CWebBrowserUI *m_pActiveXExposureFlash;
	CTabLayoutUI* m_pControl;				//主界面页面切换布局控件
	std::vector<CEditUI*>m_EditKeyWord;		//存放关键词分析输入控件，控制tab键输入
	std::vector<CTextUI*>m_vecRankText;		//存放关键词排名界面相关控件指针
	std::vector<CButtonUI*>m_vecRankButton; //存放关键词排名查看按钮先关界面指针
	CComboUI*m_pPageCombox;					//关键词排名分页combox

private:
	//相关子窗口
	CUpdateWnd m_UpdateWnd;
	CLoginWnd m_LoginWnd;
	CNewVersionWnd m_MessageWnd;
	CUpdateErrorWnd m_UpdateErrorWnd;
	CUpdateTipsWnd m_UpdateTipsWnd;
	CTrayWnd m_TrayWnd;
	CWarningWnd m_WarningWnd;
	CSettingWnd m_SettingWnd;
	CClientSetWnd m_ClientSetWnd;
	CSafeExitWnd m_SafeExitWnd;
	CCheckProWnd m_CheckProWnd;

private:
	CString m_strClientStartTime;				//客户端启动时的时间
	DWORD m_dwClientType;						//客户端当前类型:0表示主线版本 1:表示定制版本
	CString m_strClientVersion;				    //舟大师客户端版本号
	queue<DELAY_MESSAGE> m_quePostMessage;		//推送消息队列,存放未及时显示的推送消息
	WORD m_wClearType;						    //1 系统垃圾清理 2 快照清理
	DWORD m_dwClearStateFlag;					//存储快照和垃圾清理的状态 0 初始状态 1 正在清理状态 2 已经清理完成状态 
	NOTIFYICONDATA m_NotifyIcon;				//托盘		
	CString m_strUrl;							//传递给网站综合查询模块URL
	CString m_strEncodeString;					//存放加密字符串
	CString m_strDataKey;						//构造关键词分析字符串
	CString m_strEmptyKeywordPath;					//没有关键词数据的网页路径
	CString m_strEmptyExposurePath;				//没有曝光量数据的网页路径
	CString m_strZhenciPath;					//甑词网页路径
	CString m_strTemplatePath;					//甑词网页模板路径
	bool m_bIsAutoLogin;						//判断是否为正在自动登录
	bool m_bIsUseLogin;						    //判断当前是否已经有用户登录，如果是则不再弹出登录框，注销用户改变它的状态
	bool m_bIsUserLoginOut;					    //用户是否进行了注销操作
	int m_iPage;							 	//数据总共有多少页
	int m_icurPage;
	BOOL m_bIsNewestVersion;					//客户客户端是否已经为最新版本
	DWORD m_dwUpdateProgress;					//保存当前升级进度，以便能够继续下载
	bool m_bIsCancelUpdate;						//是否用户取消升级
	WORD m_wCurrentUpdateFlag;					//当前升级类型,如果为用户诊断升级修复，需要特殊处理
	bool m_bIsUpdateChking;						//是否正在进行升级校验操作，确保升级校验返回之后才能够发送自动升级消息

//	int m_iYunMsgType;					//云任务消息类型；

private:
	BOOL m_bIsRetryAutoLogin;					  //是否进行了登录重试操作----舟大师20140808
	BOOL m_bIsRankRunning;						  //是否正在刷新关键词排名
	PassInfo m_tUserInfo;						  //登录客户端时传给登录模块用的用户信息结构体
	PassInfo m_tRegistweixin;					  //用户开通微信通
	PassInfo m_tJzInfo;						      //用于开通建站系统
	PassInfo m_tUserInfoFreshWord;				  //最新版本刷新关键词排名结构体
	PassInfo m_tUserInfoWeb;					  //由于处理流程不同，不能与登录客户端用同一个结构体
	PassInfo m_tConnectServer;					  //与服务器进行交换结构体
	tuserInfotodb m_userInfodb;				      //保存登录模块返回的登录数据
	PassInfo m_tPostMsgInfo;					  //请求推送消息时传递给取数据模块的结构体------------20141117
	KEYWORD_PRODUCT_LIST m_tDataSets;			  //界面相关数据集合 -------舟大师20140809
	KEYWORDSDETAILRESPONSELIST m_tKeyWordRank;	  //产品推广排名数据结构体----舟大师
	WEIXINTONG_DATA m_tWeiXinData;				  //微信通数据结构体
	JZ_MESSAGE m_TjzData;						  //建站系统数据结构体
	DELAY_MESSAGE m_tPostMessage;				  //推送消息结构体
	CShowChart m_showchart;
	DWORD m_wVersionId;						   //记录登录成功之后返回的版本ID
	CString m_strWindowsName;					   //从配置文件中读取当前版本客户端窗口名字
	CString m_strUpdateEncodeData;				   //封装升级字符串
	CString m_strTuiSong;						   //由服务端下发的推送字符串，其中包含了安装包下载URL及MD5值等信息
	ZhenciInfo m_tZcInfo;						 //甑词消息数据
	int m_iNewKeywordCnt;							 //请求的关键词比原来的数量
	BOOL  m_bWriteStart;							//写开机启动
	int m_iUnstallFlag;								//卸载标记
	int m_iTimeCnt;									//计时器
	BOOL m_bUpdateReboot;							//升级后一直没有重启的标记
	BOOL m_bPostInstall;							//推送安装标记

	// 执行任务的进程句柄
	HANDLE               m_hProcess;

private:
	//窗口初始化
	void InitialWindowUI();

	//初始化控件
	void InitialCtrlUI();

	//初始化组件
	bool InitComponent(void);

	//写注册表信息
	void WriteRegInfo();

	//消息封装 bIsSync:同步标志  bIsNeedChangeTime:是否动态需要改变m_tUserInfo结构体参数的时间戳值
	void PackagMessage(DWORD dwDestThread,DWORD dwSourceThread,DWORD dwMessageType,WPARAM wParam, LPARAM lParam,bool bIsSync=false,bool bIsNeedChangeTime = false);

	//初始化界面操作，包括托盘创建，自动登录判断，自动升级消息发出等...
	BOOL InitialOpearate();

	//托盘操作--ADD,DELETE
	void OperateTray(DWORD dwType);

	//设置程序版本号
	bool setVersion();

	//add by zhumingxing 20140825 首次登录取出注册表中用户名，发送登录消息
	BOOL CheckInstallLogin();

	//如果是自动登录则从数据库中取出用户信息进行自动登录
	void CheckAutoLogin();

	//根据用户存放信息来判断是否要进行开机启动刷新关键词排名---20140809
	void StartReFreshKeyWordRank(WORD wSendkeyWordFlag = NORMAL_KEYWORD_MSG);

	//发送云任务消息 -------20150825
	void SendYunMsg();

	//新增定时发送获取推送消息-----20141117
	void SendPostMsg();

	//add by zhumingxing 20150121----新增与服务器交换数据
	void SendConnectSerMsg();

	//开机默认发送相关消息
	void AutoMsgSend();

	//清空网站综合查询记录
	void ClearRecodeOfSearch();

	//新增对关键词排名界面的显示类型封装----20140904
	void ShowRankUiByState(DWORD wState);

	//根据用户所选择的页码数，显示出对应的数据
	void ShowRankPageData(DWORD dwPage);										

	//当登录消息返回之后，封装一个发送给每个取数据模块一个消息---20140808----舟大师
	void PostGetDatasMsg();														

	/*登录用户登录成功之后做的一系列处理	
	bIsUserManualLogin主要用来控制启动客户端时多次刷新关键词排名导致
	排名状态错乱的问题
	*/
	void LoginSuccessOpt(BOOL bIsUserManualLogin = FALSE);

	//产品详情处理
	void HanlePublicDetail(PRODUCT_KEYWORDSTATISTICS& tProductDetail);	

	//定制版本不显示弹出框
	void HidePopWindow();

	//判断是否满足发送甄词消息
	BOOL CheckPostZhenci(CString strUser);

	//获取IP并判断是否为公司电脑
	BOOL CheckIP();

	// 取得当前系统版本信息
	std::wstring GetWindowsVersionName();

	//打印当前系统基本信息
	void PrintCurrSysInfo();

	//判断客户端隔夜是否需要重启
	void CheckReBootClient();

	//隐藏客户端
	void HideClient();

	//启动进程
	bool IsOwnerCtrlUserTaskProcess();
	bool ProcessExist(TCHAR *pstrProcName, HANDLE *m_hProcess = NULL);
	bool StartProcess(TCHAR *pstrProcName, TCHAR *pstrPort, HANDLE *phProcess = NULL);
	bool StopProcess(TCHAR *pstrProcName);
	bool StopProcess(HANDLE m_hProcess);

private:
	//customer 消息响应
	//响应定时器消息
	afx_msg void OnTimerMsg(WPARAM wParam, LPARAM lParam);	
	//托盘响应
	afx_msg void OnOpearateTray(WPARAM wParam, LPARAM lParam);		
	//响应托盘菜单
	afx_msg void OnTaryMenue(WPARAM wParam, LPARAM lParam);			
	//响应升级消息
	afx_msg void OnUpdateMessage(WPARAM wParam, LPARAM lParam);		
	//处理升级错误消息
	afx_msg void OnUpdateError(WPARAM wParam, LPARAM lParam);				
	//响应最新版本
	afx_msg void OnNewVerson(WPARAM wParam, LPARAM lParam);				
	//响应垃圾清理消息【系统垃圾及快照】
	afx_msg void OnClearMessage(WPARAM wParam, LPARAM lParam,WORD wFlag=CLEAR_SYESTEM_RUBISH);	
	//响应网站查询
	afx_msg void OnHandleWebSearchData(WPARAM wParam, LPARAM lParam);	
	//响应取消网站查询
	afx_msg void OnCancelWebSearchData(WPARAM wParam, LPARAM lParam);					
	//处理关键词分析
	afx_msg void OnKeyWordAnysis(WPARAM wParam, LPARAM lParam);				
	//响应取消关键词分析返回消息，恢复按钮状态
	afx_msg void OnCancelKeyWorsAnysis(WPARAM wParam, LPARAM lParam);		
	//用户准备登录消息
	afx_msg void OnInitialLogin(WPARAM wParam, LPARAM lParam);			
	//响应登录消息
	afx_msg void OnLoginMessage(WPARAM wParam, LPARAM lParam);				
	//响应定时气泡消息
	afx_msg void OnTimerTips(WPARAM wParam, LPARAM lParam);		
	//响应tab键操作
	afx_msg void OnTableKeys(WPARAM wParam, LPARAM lParam);			
	//响应enter键
	afx_msg void OnEnterKeys(WPARAM wParam, LPARAM lParam);				
	//响应微信通数据处理
	afx_msg void OnWeixinDate(WPARAM wParam, LPARAM lParam);			
	//响应模态弹出对话框消息，防止界面阻塞
	afx_msg void OnPopDlg(WPARAM wParam, LPARAM lParam);		
	//处理用户右键任务栏窗口关闭客户端操作【注意:这个地方就需要将其他窗口都关闭掉，以后增加窗口都需要关闭】
	afx_msg void OnRBTaskClose(WPARAM wParam, LPARAM lParam);				
	//处理关键词变化图表曲线数据---舟大师
	afx_msg void OnKeyWordData(WPARAM wParam, LPARAM lParam);				
	//响应关键排名效果消息 -----舟大师
	afx_msg void OnLoadKeyWordRank(WPARAM wParam, LPARAM lParam);	
	//关键词刷新完成消息或者是正在刷新消息
	afx_msg void OnHanleRefreshKeyWord(WPARAM wParam, LPARAM lParam);	
	//检测当前关键词模块状态消息处理
	afx_msg void OnHandleRefreshState(WPARAM wParam, LPARAM lParam);	
	//处理推送消息
	afx_msg void OnHandlePostMessage(WPARAM wParam, LPARAM lParam);
	//处理升级校验消息
	afx_msg void OnHandleUpdateCheck(WPARAM wParam, LPARAM lParam);
	//处理建站系统数据显示
	afx_msg void OnHandleJZData(WPARAM wParam, LPARAM lParam);
	//与服务器互通消息处理
	afx_msg void OnHandleServerMsg(WPARAM wParam, LPARAM lParam);
	//检测到注册表中用户名有修改之后的消息处理函数-------20150312
	afx_msg void OnHandleRegChange(WPARAM wParam, LPARAM lParam);
	//新增舟大师问题列表获取消息
	afx_msg void OnHandleGetProlemList(WPARAM wParam, LPARAM lParam);
	//新增舟大师诊断问题消息处理
	afx_msg void OnHandleCheckProMsg(WPARAM wParam, LPARAM lParam);
	//新增舟大师问题修复消息处理
	afx_msg void OnHandleProRepairMsg(WPARAM wParam, LPARAM lParam);
	//新增web控件丢失消息处理
	afx_msg void OnHandlePluginLose(WPARAM wParam, LPARAM lParam);
	//新增甑词数据消息处理
	afx_msg void OnHandleZhenciMsg(WPARAM wParam, LPARAM lParam);
	//新增开始刷5*20的排名
	afx_msg void OnHandlePaimingMsg(WPARAM wParam, LPARAM lParam);
private:
	//Notify 消息响应
	//响应设置按钮消息	
	afx_msg void OnNotifySetting(TNotifyUI& msg);			
	//点击登录按钮
	afx_msg void OnNotifyLoginCilck(TNotifyUI& msg);		
	//垃圾清理返回
	afx_msg void OnNotifyClearBack(TNotifyUI& msg);			
	//切换option
	afx_msg void OnNotifyClickOption(TNotifyUI& msg);			
	//响应selectChange消息
	afx_msg void OnNotifySelectchanged(TNotifyUI& msg);			
	//响应关键词分析
	afx_msg void OnNotifyKeyWordAnysis(TNotifyUI& msg);		
	//响应网站综合查询
	afx_msg void OnNotifySearch(TNotifyUI& msg);				
	//响应初始化升级消息
	afx_msg void OnNotifyInitialUpdate(TNotifyUI& msg);					
	//用户点击进入微信通
	afx_msg void OnNotifyEnterWeixin(TNotifyUI& msg);		
	//窗口初始化消息响应
	afx_msg void OnNotifyWindowsInitial(TNotifyUI& msg);				
	//查看关键字排名快照
	afx_msg void OnNotifyCheckSnap(TNotifyUI& msg);			
	//进入舟大师产品推广
	afx_msg void OnNotifyEnterPost(TNotifyUI& msg);			
	//进入舟大师产品展示
	afx_msg void OnNotifyEnterShow(TNotifyUI& msg);
	//开通微信通及建站系统
	afx_msg void OnNotifyBuyProduct(WORD wtype);
};

