#pragma once

//线程类型
enum
{
    E_THREAD_TYPE_MGR = 0,           //管理线程
	E_THREAD_UPDATE,				 //客户端升级线程
	E_THREAD_CLEAR,					 //系统垃圾清理
    E_THREAD_TYPE_UIHELPER,          //将线程消息转发给UI的线程
	//E_THREAD_CLEARQUICKPHOTOS,       //清理快照的线程
	//E_THREAD_KEYWORDANALYSIS,        //关键词分析线程
	//E_THREAD_WEBSEARCHES,			 //网站综合查询线程
	//E_THREAD_WEBSEARCHWITHWEBCTRL,   //网站综合查询线程（用web控件）
	E_THREAD_DATASTATISTICS,         //数据统计线程
	E_THREAD_TIMERTASK,				 //定时任务线程
	E_THREAD_YUN_TASK,				//云任务
	E_THREAD_UTILITY,                //整合清理快照，关键词分析，网站综合查询为一个模块的线程
};

struct T_POPUP_MSG
{
	TCHAR szTitle[512];
	TCHAR szContent[1024];
	DWORD dwShowTime;
};

///////////消息定义，添加时请指出WPARAM及LPARAM的类型及含义////////////////
enum
{
	//测试消息
	//WPARAM:NULL
	//LPARAM:NULL
	MSG_DEMO_TEST = WM_USER + 1,

	//管理线程向功能组件（被管理线程）发送的定时器消息
	//WPARAM:NULL
	//LPARAM:NULL
	MSG_TIMER,   

	//管理线程向功能组件发送的线程重启消息
	//WPARAM:NULL
	//LPARAM:NULL
	MSG_RESETTHREAD,

	//功能组件向UIHelper线程发送的弹框消息
	//WPARAM:NULL   
	//LPARAM: &T_POPUP_MSG
	MSG_POPUP_MSG,


	//托盘提示消息
	//WPARAM:NULL   
	//LPARAM: CString 指针  格式：TipTitle##TipContent##TipTime
	MSG_TIP_MSG,


	//托盘消息
	//lParam托盘行为
	WM_TRAY_MSG,

	//点击托盘选项消息
	//WPARAM:菜单序列编号   
	//LPARAM:0
	MSG_TRAY_FUNCTION_MSG,

	//客户端升级消息
	//WPARAM=101 表示用户确定重启客户端升级 LPARAM=1手动点击立即重启，2表示后台强制重启
	//WPARAM=102 表示自动升级请求消息

	/*返回时:
	WPARAM:表示总进度，类型为int   
	LPARAM:表示当前进度，类型为int
	LPARAM = 102 表示是自动升级有更新，需要弹出自动升级框提示用户
	*/
	MSG_PRODUCT_UPDATE,
	
	//自动升级时用户点击升级
	//WPARAM:0   
	//LPARAM:0
	MSG_AUTO_CLICK,

	//取消升级
	//WPARAM:NULL   
	//LPARAM:NULL
	MSG_CANCEL_UPDATE,

	//最新版本,无需升级
	//WPARAM:NULL   
	//LPARAM:NULL
	 //返回时:自动升级LPARAM = 102非自动升级 LPARAM = 0
	 MSG_LATEST_VERSION,
	 
	 //升级过程中出现错误
	 //WPARAM:NULL   
	 //LPARAM:NULL
	 //返回时
	 //WPARAM =1表示服务器正忙，稍后再试
	 //WPARAM =2表示IE控件无法复制
	 //返回时:自动升级LPARAM = 102非自动升级 LPARAM = 0

	 MSG_UPDATE_ERROR,

	 //清理系统垃圾消息
	 //WPARAM:0  
	 //LPARAM:0
	 //返回时
	 //WPARAM:代表进度值设置   
	 //LPARAM:代表已经清理的垃圾总数
	 MSG_SYSTEM_CLEAR,

	 //取消清理垃圾
	 //WPARAM:0
	 //LPARAM:0
	 MSG_SYSTEM_CLEAR_CANCEL,

	//（ServerEx模块）验证用户登陆消息
	//WPARAM:NULL   
	//LPARAM: &T_POPUP_MSG
	MSG_SERVEREX_USER_VALIDATE,

	//清除快照消息
	//WPARAM:返回给ui删除的文件大小 
	//LPARAM:返回给ui进度条数据
	MSG_CLEAR_QUICK_PHOTOS,

	//刷新关键词排名
	//WPARAM:前两个字节表示版本号；后两个字节表示是否是点击立即刷新按钮，1为点击刷新按钮
	//LPARAM:passinfo结构体指针
	//LPARAM返回时:1表示操作完成，返回成功。
	MSG_AUTO_REFRESH_KEYWORD,

	//清理code文件夹消息
	//WPARAM:NULL
	//LPARAM:NULL
	MSG_CLEAR_CODE,

	//关键词分析消息
	//WPARAM:ui发过来关键字，返回给ui查询结果，其结果关键词，标度收录量，相关搜索条目等，一般在用户输入省份城市的情况下返回21条，
	//最后一条为结束标志。   
	//LPARAM:1表示成功获取数据，返回成功。0表示获取数据失败，返回失败。2表示获取数据完成。
	MSG_KEYWORD_ANALYSIS,

	//取消关键词分析消息 【需要返回界面】
	//WPARAM 0
	//LPARM 0
	MSG_CANCEL_KEYWORD_ANALYSIS,

	//网站综合查询消息
	//WPARAM:ui发过来要查询的url，返回给ui查询结果，其结果为1.百度权重；2.GooglePR；3.反链数；4.站内链接；5.域名年龄；
	//6.备案号；7.性质；8.名称；9.审核时间；10.百度收录；11.谷歌收录；12.360收录；13.搜狗收录；14.百度反链；15.谷歌反链；
	//16.360反链；17.搜狗反链。
	//LPARAM:1表示成功获取数据，返回成功。0表示获取数据失败，返回失败。2表示用户输入的网址格式不对。
	MSG_WEB_SEARCH,

	//取消网站综合查询 需要返回界面】
	//WPARAM 0
	//LPARM 0
	MSG_CANCEL_WEB_SEARCH,

	//近七天数据
	//WPARAM:  0
	//LPARAM:  PassInfo 结构体指针
	//返回时LPARAM  用这个结构体SEVEEN_DATA指针返回
	MSG_SEVEN_DATA,     //近七天数据

	//产品推广详情
	//WPARAM:  0
	//LPARAM:  PassInfo 结构体指针
	//返回时LPARAM  用这个结构体ProductDetail指针返回
	MSG_POST_DETAIL_DATA,     //产品推广详情

	//登录商舟网
	//WPARAM:  0
	//LPARAM:  PassInfo 结构体指针
	MSG_LOGIN_SHANGZHOU,     // 登录商舟网 

	//登录客户端
	//WPARAM:  0为登录   1为取密码
	//LPARAM:  PassInfo 结构体指针
	//返回时WPARAM  返回结果 1:请求成功,2:参数验证失败,3:参数错误,4：身份验证未通过,5：账户信息错误,6.请求失败  0表示打不开页面，可能是断网情况或者服务器出问题
	//LPARAM   PassInfo 结构体指针
	MSG_LOGIN_CLIENT,     // 登录客户端


	//登录商舟微信通
	//WPARAM:  0
	//LPARAM:  PassInfo 结构体指针
	MSG_LOGIN_WEIXIN,     // 登录商舟微信通

	//取得微信通数据
	//WPARAM:  0
	//LPARAM:  PassInfo 结构体指针
	//返回时LPARAM  用这个结构体WEIXINTONG_DATA指针返回
	MSG_WEIXIN_DATA,     // 微信通数据

	//关键词推广效果数据
	//WPARAM:  0
	//LPARAM:  PassInfo 结构体指针
	//返回时LPARAM  用这个结构体KEYWORDSDETAILRESPONSELIST指针返回
	MSG_KEYWORD_RESULT_DATA,     // 关键词推广效果数据

	//产品曝光量   上线关键词变化数据   产品和关键词统计数据
	//WPARAM:  传回传过来的值
	//LPARAM:  PassInfo 结构体指针
	//返回时LPARAM  用这个结构体KEYWORD_PRODUCT_LIST指针返回
	MSG_KEYWORD_PRODUCT_DATA,     // 产品曝光量   上线关键词变化数据   产品和关键词统计数据
	

	//取消登录客户端
	//WPARAM:  0
	//LPARAM:  0
	MSG_LOGIN_CANCEL,     // 取消登录客户端


	//登录商舟网进行推广
	//WPARAM:   1：智能引擎推广，2：优势产品信息推广，3：网站信息推广，4：采购信息推广
	//舟大师  1产品推广，2产品展示
	//LPARAM: PassInfo 结构体指针
	MSG_PRODUCT_POST,

	//登记用户mac地址，记录客户端使用量
	//WPARAM: 0
	//LPARAM: 0
	MSG_SUBMIT_USERINFO,

	// 启动定时任务窗口
	//WPARAM: 0
	//LPARAM: 0
	MSG_START_TIMERDLG,

	//安全退出消息
	//WPAPAM:0
	//LPARAM:0
	MSG_SALF_EXIT,

	//Tab键消息
	//WPAPAM:0
	//LPARAM:0
	WM_TAB_KEY_PRESS,

	//Enter键消息
	//WPAPAM:0
	//LPARAM:0
	WM_ENTER_KEY_PRESS,

	//模态弹出对话框消息
	//WPAPM: 0--登录对话框   1--升级对话框 2--升级后重启提示对话框 3--升级提示最新版本对话框 4--升级提示错误对话框  5--警告对话框 6--设置对话框
	//LPARAM:0
	WM_SHOW_MODAL,

	//显示画图控件,为了避免界面因为flash加载而导致阻塞，先隐藏flash控件然后再显示
	MSG_SHOW_CHART,

	//新增一个消息，用来当客户端升级重启后来显示托盘已经升级到最新版本
	MSG_SHOW_NEWVERSION,

	//自动从服务器更新本地的配置消息
	//WPARAM: 0
	//LPARAM: 0
	MSG_UPDATE_CONFIG_TO_LOCAL,

	//取得推送消息数据
	//WPARAM: 0
	//LPARAM: PassInfo 结构体指针
	//返回  wparam  1为取得成功，0为无消息  
	//   lparam  DELAY_MESSAGE结构体指针
	MSG_GET_DELAYMSG,
	
	//新增消息通知当前通知消息已经关闭，显示下一个推送消息窗口
	//WPAPM: 0
	//LPARAM: 0
	MSG_SHOW_NEXT_POSTMESSAGE,

	//新增升级校验消息
	//WPAPM: 0
	//LPARAM: 0
	//返回
	//WPAPM: 0
	//LPARAM: 0：失败  1:成功
	MSG_UPDATE_SUCCESS_CHECK,

	//检测刷关键词排名状态
	//WPAPAM:0
	//LPARAM:0
	MSG_CHECK_REFRESH_KEYWORD_STATA,

	//新增下载安装包消息----------20141124
	//WPAPM: DELAY_MESSAGE结构体
	//LPARAM: 0
	//返回
	//WPAPM: DELAY_MESSAGE结构体，详细页面URL替换为安装包路径
	//LPARAM: 0：失败  1:成功
	MSG_DOWNLOAD_INSTALLEXE,


	//取得建站系统信息
	//WPARAM:  暂时不用
	//LPARAM:  PassInfo 结构体指针
	//返回时wparam   -1 表示未购买建站系统 1:请求成功,2:参数验证失败,3:参数错误,4：身份验证未通过,5：账户信息错误,6.请求失败  0表示打不开页面，可能是断网情况或者服务器出问题
	//返回时LPARAM  用这个结构体JZ_MESSAGE指针返回
	MSG_GET_JZ_INFORMATION,


	//或开通微信通   开通建站系统
	//WPARAM:  0
	//LPARAM:  PassInfo 结构体指针
	MSG_BUY_PRODUCT,

	//通过udp跟服务端交互
	//WPAPAM:null
	//LPARAM:PassInfo 结构体指针
	//WPAPAM返回时：服务端返回的数据： CMD_CHECK_UPDATE   //检查升级 CMD_SUBMIT_LOG   //提交日志 NOTASK//无任务
	//LPARAM返回时：null
	MSG_CONNECT_SERV_WITH_UDP,

	//提交日志文件
	//WPAPAM:0表示只提交日志，1表示提交日志及快照
	//LPARAM:null
	MSG_SUBMIT_LOG,

	//新增检测注册表变化消息
	MSG_CHECK_REG,

	//新增故障自检消息
	//WPAPAM:客户端是否是最新版本，WPARAM_CLIENT_NEW_VERSION表示是最新版本，WPARAM_CLIENT_OLD_VERSION表示不是最新版本
	//LPARAM:null
	//WPAPAM返回时：T_PROBLEM_DATA结构体指针
	//LPARAM返回时：null
	MSG_SELF_DIAGNOSIS,


	//新增读故障自检配置文件的消息
	//WPAPAM:null
	//LPARAM:null
	//WPAPAM返回时：T_DATA_FROM_XML结构体指针
	//LPARAM返回时：null
	MSG_READ_SELF_DIAGNOSIS_XML,

	//新增修复故障消息
	//WPAPAM:要修复功能的序列号
	//LPARAM:null
	//WPAPAM返回时：T_PROBLEM_DATA结构体指针
	//LPARAM返回时：null
	MSG_REPAIR_FAULT,

	//新增故障检测修复的取消消息
	//WPAPAM:null
	//LPARAM:null
	MSG_DIAGNOSIS_CANCEL,

	//新增监控web控件是否丢失消息
	//WPAPAM:null
	//LPARAM:null
	MSG_WEB_ACTIVEX_LOSE,

	//新增消息卸载升级重启客户端时卸载全局钩子
	MSG_FREE_HOOK_DLL,

	//新增静默推送安装消息
	//WPAPM: 服务器下发的安装包下载信息
	//LPARAM: 0
	//返回
	//WPAPM: 0
	//LPARAM: 0：失败  1:成功
	MSG_DOWNLOAD_SILENT_EXE,

	//显示曝光量控件,为了避免界面因为浏览器加载而导致阻塞，先隐藏浏览器控件然后再显示
	MSG_SHOW_EXPOSURECHART,

	//云任务消息
	//WPAPAM:0
	//LPAPAM:0
	MSG_YUN_TASK,

	//甄词消息
	MSG_ZHENCI_INFO,

	//后台循环检测web插件是否丢失(RunDetours可能导致游览器崩溃，所以采用此种方法)
	MSG_CHECK_PLUGINS,

	//发送请求刷排名的API
	MSG_START_PAIMING,
};


/*
add by zhumingxing 20140904
规范各个模块消息之间传递参数含义
用户自定义消息参数宏规范:
WPARAM: WPARAM_含义
LPARAM: LPARAM_含义

注意：参数请定义在对应的消息下，以免造成混乱
*/
enum msgparam
{
	//common returm code
	RET_ERROR = 0,				//失败
	RET_SUCCESS = 1,			//成功

	//user define parpam
	//MSG_PRODUCT_UPDATE
	WPARAM_MANUALCHECK_UPDATE = 0,						//手动检测升级请求
	WPARAM_AUTOCHECK_UPDATE = 102,						//自动检测升级请求
	WPARAM_REPAIR_UPDATE = 103,							//用户在诊断界面修复升级
	WPARAM_REBOOT_CLIENT = 101,							//用户重启客户端
	LPARAM_REBOOT_MANUAL = 1,							//用户手动点击重启
	LPARAM_REBOOT_AUTO = 2,								//后台自动重启

	//WPARAM_NOMAL_UPDATE = 0,							//正常升级文件下载成功
	WPARAM_CONFIGFILE_UPDATE = 1,						//只是配置文件在线成功
	LPARAM_UPDATE_SUCCESS = 102,					    //升级请求到需要升级并且文件已经下载完毕,提示重启客户端
	LPARAM_DOWNLOAD_FINISH = 100,						//文件下载完成

	//MSG_UPDATE_ERROR
	WPARAM_WINNET_ERROR = 0,							//网络错误
	WPARAM_SERVER_BUSY_UPDATE = 1,					    //升级时服务器正忙
	WPARAM_IECTRL_COPYERROR = 2,						//升级过程中IE控件无法复制
	WPARAM_UPDATE_CONFIG_ERROR = 3,						//更新本地配置文件错误
	WPARAM_POWER_ERROR = 4,								//权限问题
	WPARAM_NPCTRL_COPYERROR = 5,							//升级过程中多浏览器插件无法复制

	//MSG_KEYWORD_ANALYSIS
	LPARAM_GETDATA_COMPLETE = 2,						//获取关键词分析数据完成

	//MSG_WEB_SEARCH
	LPARAM_URL_ERROR = 2,								//用户输入URL有误

	//MSG_LOGIN_CLIENT
	WPARAM_CONNECTSERVER_ERROR = 0,						//登录时连接服务器失败
	WPARAM_USERINFO_ERROR = 5,							//登录用户账号信息错误
	WPAPAM_INSTALL_LOGIN = 1,							//用户进行首次安装登录
	WPARAM_AUTO_LOGIN = 2,								//用户进行取注册表自动登录
	WPARAM_CHANGE_USER = 3,								//当注册表用户改变之后进行切换用户操作

	//WM_SHOW_MODA
	WPARAM_SHOW_LOGIN_WND = 0,							//弹出登录对话框
	WPARAM_SHOW_UPDATE_WND = 1,							//弹出升级进度对话框
	WPARAM_SHOW_REBOOTCLIENT_WND = 2,					//弹出重启客户端对话框
	WPARAM_SHOW_LATESTVERSION_WND = 3,					//弹出最新版本对话框
	WPARAM_SHOW_UPDATEERROR_WND = 4,					//弹出升级错误对话框
	WPARAM_SHOW_NETWORKERROR_WND = 5,					//弹出网络错误警告对话框
	WPARAM_SHOW_SETTING_WND = 6,						//弹出设置对话框
	WPARAM_SHOW_SAFEEXIT_WMD = 7,						//弹出安全退出对话框
	WPARAM_SHOW_CHECKPROBLEM_WMD = 8,					//弹出诊断对话框
	LPARAM_SHOW_ANYWAY = 1,								//无论什么情况下都要弹出，不管当前客户端为最小化或者是最大化

	//MSG_TRAY_FUNCTION_MSG
	WPARAM_SHOW_CLIENT = 0,								//显示客户端
	WPARAM_LOGIN_SHANGZHOU = 1,							//登录商周网
	WPARAM_SHOW_SZTONGZHI = 2,							//显示商周通知
	WPARAM_CLEAR_PHOTO = 3,								//清理快照
	WPARAM_START_UPDATE = 4,							//检测升级
	WPARAM_LOGIN_OUT = 5,								//注销登录
	WPARAM_SHOW_SETTINGWND = 6,							//弹出设置对话框
	WPARAM_SAFE_EXIT = 7,								//安全退出
	WPARAM_CHECK_PROBLEM = 8,							//诊断工具

	//MSG_KEYWORD_RESULT_DATA
	//WPARAM_UPDATE_STATE_FINISH= 0,						 //刷新关键词已完成，获取数据成功后显示已经获取最新排名
	//WPARAM_SAVE_STATE_RUNNING = 1,						 //定时提交完成，获取排名数据完成后不更新UI为完成状态，而是继续保持Running状态

	//MSG_PRODUCT_POST
	WARAM_ENTER_PRODUCT_POST = 1,							//进入产品推广
	WARAM_ENTER_PRODUCT_SHOW = 2,							//进入产品展示
	WARAM_ENTER_PRODUCT_ZHENCI=5,							//进入大师甄词

	//MSG_CHECK_REFRESH_KEYWORD_STATA
	WARAM_AUTO_REFRESH_KEYWORD_FINISH = 1,					 //完成刷新
	WAPAM_AUTO_REFRESH_KEYWORD_RUNNING = 2,					//主控正在刷新
	//WAPAM_AUTO_REFRESH_KEYWORD_REGULARLY_SUBMIT = 3,        //定时提交

	//MSG_SELF_DIAGNOSIS
	WPARAM_CLIENT_NEW_VERSION = 1,							 //客户端是最新版本
	WPARAM_CLIENT_OLD_VERSION = 0,							 //客户端不是最新版本
	LPARAM_CHILD_TO_MIAN = 1,								 //此参数代表诊断消息是由诊断窗口发给主窗口的
}; 