#ifndef _GLOBAL_DEF_H_
#define _GLOBAL_DEF_H_
//#include "stdafx.h"
typedef struct _tag_HOOKDATA   
{   
	int nType;   
	HOOKPROC hkProc;   
	HHOOK hHook;   
} TAG_HOOK_DATA;  

extern TAG_HOOK_DATA g_KbHookData;
extern HWND g_hHwndMain;
//extern HHOOK g_hCBT;	

//全局钩子函数
inline LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	static LONG_PTR lVkCodePrev_Tab = 0;  
	static LONG_PTR lVkCode_Tab = 0;  
	static LONG_PTR lVkCode_Enter = 0;  
	static LONG_PTR lVkCode_Cancel = 0;  
	LONG_PTR        lTemp = 0;  

	if (0 == nCode)  
	{  
		switch (wParam)  
		{  
		case VK_TAB:  
			{  
				lVkCode_Tab = (LONG_PTR)lParam;  
				lTemp = lVkCode_Tab & 0xFFF00000;  
				if (0 == lTemp)  
				{  
					// tab key press down  
					lVkCodePrev_Tab = lVkCode_Tab;  
				}  
				else  
				{  
					// tab key press up  
					if (lVkCode_Tab != lVkCodePrev_Tab)  
					{  
						lVkCodePrev_Tab = lVkCode_Tab;  
						if (NULL != g_hHwndMain)  
						{  
							::PostMessageW(g_hHwndMain, WM_TAB_KEY_PRESS, 0, 0);  
						}  
					}  
				}  
			}  
			break;  
		case VK_RETURN:  
			{  
				lVkCode_Enter = (LONG_PTR)lParam;  
				lTemp = lVkCode_Enter & 0xFFF00000;  
				if (lTemp > 0)  
				{  
					// key press up  
					if (NULL != g_hHwndMain)  
					{  
						::PostMessageW(g_hHwndMain, WM_ENTER_KEY_PRESS, 0, 0);  
					}  
				}  
			}  
			break;  
		default:  
			break;  
		}  
	}  
	return CallNextHookEx(g_KbHookData.hHook, nCode, wParam, lParam);  
}

//begin add by zhumingxing 20150710
//CBT钩子消息处理过程
//inline LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
//{
//	CBT_CREATEWND *pWndParam = NULL;
//	CREATESTRUCT *pCreateStruct = NULL;
//	CString strText;
//	CString strParentTitle = _T("");
//
//	switch (nCode)
//	{
//		case HCBT_CREATEWND:
//		{
//			pWndParam = (CBT_CREATEWND*)lParam;
//			pCreateStruct = pWndParam->lpcs;
//			CString strText = pCreateStruct->lpszName;
//
//			if (!strText.CompareNoCase(_T("Adobe Flash Player")))
//			{	
//				return 1;
//			}
//			else
//			{
//				  break;
//			}
//		 }
//		 break;
//	}
//
//	// 继续传递消息
//	return CallNextHookEx(g_hCBT, nCode, wParam, lParam);
//}
//end

//客户端类型控制
enum ClientType
{
	MAIN_LINE_VERSION = 0,
	CUSTOMIZE_VERSION = 1,
	HIDE_VERSION = 2,
};

//主界面tab页枚举与index.xml中Tablayout对应
enum tabpage
{
	PAGE_KEYWORD_COUNT = 0,			//关键词统计页面
	PAGE_KEYWORD_ANALYSIS,			//关键词分析页面
	PAGE_CLEAR_INDEX,			    //垃圾清理主页面
	PAGE_WEBSITE_SEARCH,			//关键词分析页面
	PAGE_SZ_WEBSITE,				//商舟建站
	PAGE_SZ_NOWEBSITE,				//未开通建站系统
	PAGE_SZ_FAQ,					//常见问题页面
	PAGE_CLEAR_RUNNING,				 //正在清理垃圾页面
	PAGE_CLEAR_FINNISH,				//垃圾清理完毕显示清理结果页面
	PAGE_PRODUCT_EXPOSURE,			//产品曝光页面
	PAGE_WEIXIN_DATA,				//显示微信通数据页面
	PAGE_NO_WEIXIN_USER,			//未开通微信通页面
	PAGE_KEYWORD_RANK,				//关键词排名页面
};

//垃圾清理时记录相关状态参数
enum clearflag
{
	CLEAR_SYESTEM_RUBISH = 1,		//系统垃圾清理
	CLEAR_PHOTO = 2,				//快照清理
	CLEAR_STATE_INTIAL = 0,		//初始化清理状态，还未开始清理
	CLEAR_STATE_RUNNING = 1,		//正在清理
	CLEAR_STATE_FINISH = 2,		//清理完毕状态
};

//关键词排名界面的显示类型封装
enum 
{
	RANKUI_INITIAL = 0,				//初始化关键词排名界面
	RANKUI_HIDE,					//隐藏整个界面
	RANKUI_RUNNING,					//正在获取关键词排名界面
	RANKUI_ERROR,					//获取关键词排名失败界面
	RANKUI_SUCCESS,					//获取关键词排名成功界面
	RANKUI_LOGINOUT,				//用户未登陆或者是登录失败界面显示
	RANKUI_SAVEOLD,					//保持原有数据界面
	RANKUI_EMPTY,					//清空排名数据，原有状态
};

//推送消息类型
enum NOTIFY_TYPE
{
	NOTIFY_TYPE_MSG = 1,             //通用类型
	NOTIFY_TYPE_PERSONALITY = 2,     //个性化
	NOTIFY_TYPE_UPDATE = 3,          //表示客户端强制升级
};

//图表类型
enum CHART_TYPE
{
	KEYWORD_CHART = 1,				//关键词图表
	PRODUCT_CHART = 2,				//曝光图表
	WEIXIN_USERCARE_CHART = 3,		//微信关注图表
	WEIXIN_USERREQUEST_CHART = 4,   //微信请求图表
};

//开通产品类型
enum PRODUCT_TYPE
{
	PRODUCT_WEIXIN = 0,
	PRODUCT_JZ = 1,
};

//刷新排名消息类型控制
enum KEYWORDMSG_TYPE
{
	NORMAL_KEYWORD_MSG = 0,			 //普通刷新排名消息
	START_REFRESH_KEYWORD_MSG = 1,   //立即刷新消息
};
//捕获到鼠标右键消息
const LPARAM MOUSE_RIGHT_ACTIVE = 0x2040001;	

//VK_F4ID
const int ID_VKF4 = 0x1000;

/*-------------------------------------相关定时器ID定义--------------------------------------*/
//定时隐藏托盘气泡
const UINT ID_KILLTIPS = 200;

//定时器检测窗口是否最大化正常显示
const UINT ID_WINDOW_SHOWMAX= 100;

//定时器检测窗口是否最小化或者是最小化到托盘
const UINT ID_WINDOW_SHOWMIN = 101;

//自动登录失败后，且用户未登录成功，进行定时重试登录
const UINT ID_RETRY_LOGIN = 300;

//定时器防止用户不关机，客户端不进行关键词排名等操作
const UINT ID_AUTO_SEND_MESSAGE = 301;

//定时器定时获取推送信息30~60min
const UINT ID_GET_POST_MESSAGE = 302;

//开机启动1.5min之后首次获取最新推送消息，之后每隔30~60min钟获取
const UINT ID_TIMING_GET_MESSAGE = 303;

//新增升级失败之后每隔30分钟重试自动升级
const UINT ID_RETRY_AUTO_UPDATE = 304;	

//新增10分钟让客户端与服务器进行消息通信
const UINT ID_CONNECT_SERVER = 305;

//新增静默推送成功后启动定时器检测是否可以静默安装
const UINT ID_CHECK_SILENT_INTSTALL = 306;

//新增默认提示甄词信息在10分钟后托盘提示
const UINT ID_ZHENCI_TIPS = 307;

//新增延时2分钟之后发送甄词消息
const UINT ID_ZHENCI_MSESSAGE = 308;

//新增1小时候开始启动刷排名
const UINT ID_PAIMING_MSG = 309;

//新增软件启动后开启CtrlUserTask.exe进程消息
const UINT ID_START_CTRLUSERTASK_MSG = 310;

/*--------------------------------------界面相关常量定义-------------------------------------*/
const TCHAR* const kTabLayOutControlName = _T("switch");
const TCHAR* const kChkUpdateControlName = _T("btnCheck");
const TCHAR* const kLoginControlName = _T("btnLogin_index");
const TCHAR* const KClearProgress = _T("Progress_rub");
const TCHAR* const KRankSelectControlName = _T("page_select"); 
const UINT WM_TASKBARCREATED = ::RegisterWindowMessage(_T("TaskbarCreated"));

/*--------------------------------------舟大师客户端中使用的相关URL地址常量--------------------*/
//舟大师客户端缓存URL
const TCHAR* const URL_CLIENT_CACHE = _T("http://www.sumszw.com/web/boat/html/");
//默认登录API
const TCHAR* const URL_DEFAULT_LOGINAPI = _T("http://api.sumszw.com");
//商舟注册页面URL
const TCHAR* const URL_SZ_REGISTE = _T("http://z.sumszw.com/RegistSumsz/Regist");
//商舟找回密码页面URL
const TCHAR* const URL_PASSWORD_FIND = _T("http://www.sumszw.com/Web/BusinessInfo/BusPasswordBack.aspx?li=1");
//商舟通知页面URL
const TCHAR* const URL_SZ_MESSAGE = _T("http://www.sumszw.com/web/boat/html/msglist.html?id=");
//商舟常见问题URL
const TCHAR* const URL_SZ_FAQ = _T("http://www.sumszw.com/web/boat/html/faqlist.html?id=");
//开通微信通URL及建站系统跳转URL
const CString URL_REGIST_WEIXINT = _T("/Member/ApplyCenter/ProductDetail.aspx?rid=%d"); 
#endif