#ifndef _USERDEFINE_H_
#define _USERDEFINE_H_
//#include <minwindef.h>


//0：线上（50）	1：线下（253）	2：本地	
#define RUN_DEBUG 0		


//软件版本号
const TCHAR* const g_sVersion = _T("1&1.0.00");


#if(RUN_DEBUG == 0)
	//web服务器地址（ClientUpdateService.exe所在地址）
	const string g_sServerUrl = "http://112.74.102.50:2015/downloadfilelist/index";
	//程序自身下载地址
	const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/MagicBox.exe");
	//安装包下载地址
	const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");
#elif(RUN_DEBUG == 1)
	//web服务器地址（ClientUpdateService.exe所在地址）
	const string g_sServerUrl = "http://192.168.1.253:2015/downloadfilelist/index";
	//程序自身下载地址
	const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/MagicBox.exe");
	//安装包下载地址
	const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");
#else
	//web服务器地址（ClientUpdateService.exe所在地址）
	const string g_sServerUrl = "http://127.0.0.1:2015";
	//程序自身下载地址
	const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/MagicBox.exe");
	//安装包下载地址
	const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");
#endif


// web服务器地址（ClientUpdateService.exe所在地址）
// const string g_sServerUrl = "http://127.0.0.1:2015";
// 程序自身下载地址
// const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/MagicBox.exe");
// 安装包下载地址
// const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");




//捕获到鼠标右键消息
const LPARAM MOUSE_RIGHT_ACTIVE = 0x2040001;

/*-------------------------------------相关定时器ID定义--------------------------------------*/
//下载安装包
const UINT ID_TIMER_DL_CLIENTPACKAGE = 200;
//界面刷新定时器
//const UINT ID_TIMER_UI_REDRAW = 201;
//
const UINT ID_TIMER_UI_REFRESH = 202;
const UINT ID_TIMER_UI_HOWLONG = 203;

/*--------------------------------------界面相关常量定义-------------------------------------*/
//Common
const TCHAR* const sButtonControlName_Ok = _T("Button_OK");
const TCHAR* const sProgressControlName_DL = _T("Progress_DL");
//SKIN_DEFAULT
const TCHAR* const sLabel_Show_Start = _T("Label_Show_Start");
const TCHAR* const sLabel_Show_Exit = _T("Label_Show_Exit");
const TCHAR* const sControl_Loading = _T("Control_loading");
const TCHAR* const sControl_Black = _T("Control_Blak");
const TCHAR* const sButtonGif_Loading = _T("ButtonGif");

const TCHAR* const sLabel_FunctionDeclaration = _T("Label_Function_Declaration");
const TCHAR* const sLabel_ExitTip = _T("Label_Exit_Tip");
const int c_iExit_initTime = 5;

/*--------------------------------------自定义消息-------------------------------------*/
//SKIN_DEFAULT
const LPARAM USERMSG_PROCESS_EXIT = 1000;
const LPARAM USERMSG_SINGLE_RUN = 1100;


enum
{
	CLIENT_NULL = 0,
	CLIENT_GET_SERVERINFO_FAIL,	//获取服务器信息失败
	CLIENT_EXIST,				//客户端已安装
	CLIENT_OPEN_MUTEX_FAIL,		//创建互斥器失败
	CLIENT_CLIENT_RUN,			//客户端已在运行中
	CLIENT_START_SUCCESS,		//启动客户端成功
	CLIENT_START_FAIL,			//启动客户端失败
	CLIENT_SELECTSERVERFAIL,	//选择服务器失败
	CLIENT_DL_FAIL,				//下载客户端安装包失败
	CLIENT_INSTALL_FAIL,		//客户端安装失败
	CLIENT_INSTALL_SUCCESS		//客户端安装成功
};


enum ENUM_LOGGER
{
	L_MAIN = 0,		//the main logger, It away exist.
	L_MagicBox,		//the user-defined logger.
	L_LOG_SUM
};
//程序运行状态
//ui状态
enum E_UI_STATUS
{
	CLIENT_UI_NULL = 0,		//初始状态
	CLIENT_UI_START,		//UI启动
	CLIENT_UI_EXIT,			//UI退出
};
//下载状态
enum E_DL_STATUS
{
	CLIENT_DL_NULL = 0,		//初始状态
	CLIENT_DL_START,		//下载开始
	CLIENT_DL_OVER,			//下载完成
};

#endif