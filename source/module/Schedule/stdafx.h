// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE 类
#include <afxodlgs.h>       // MFC OLE 对话框类
#include <afxdisp.h>        // MFC 自动化类
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC 数据库类
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                     // MFC DAO 数据库类
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "..\..\common\Trace.h"
#include "..\..\common\commondef.h"


extern CLogTrace g_log;
extern T_GLOBAL_DATA *g_pGlobalData;



#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\lib\\ZipFunction_d.lib")
#else
#pragma comment(lib, "..\\..\\lib\\ZipFunction.lib")
#endif

#ifndef _UPDATE_INCLUDE
#define _UPDATE_INCLUDE
#include <vector>
#include "..\..\common\Reg.h"
#include "..\..\common\CommFunc.h"
#include "..\..\common\StdString.h"
#include "..\..\common\StringUtils.h"
#include "..\..\common\Trace.h"
#include "..\..\common\common_define.h"
#include "..\..\common\HttpUtils.h"
#include "..\..\common\ZipEncode\ZipEncode.h"
#include "..\..\common\IniFile.h"
#include "InternetHttp.h"
#include "..\..\common\Directory.h"
#include "..\..\common\Base64.h"
#include "..\..\common\ServerData.h"
#include "..\..\common\Lock.h"
#include "ZipFunction.h"
#include "FTP.h"
#endif


//自定义
#ifndef _BizClient
#define _BizClient


#define   TIP_TIME                 3000
#define   TASK_CHK_TIME            1000
#define   TASK_CHK_STATUS          3000
#define   DEFAULT_REUPDADTE_TIME   60 * 30          //默认30分钟重试一次


#define REG_USER_ROOT               HKEY_CURRENT_USER
#define REG_LOCAL_MACHINE           HKEY_LOCAL_MACHINE
#define REG_MACHINE_ROOT            HKEY_LOCAL_MACHINE
#define REG_PATH_COMMON_UTILITY     _T("Software\\szw\\MasterZ")
#define REG_PATH_UTILITY_UPDATE     _T("Software\\szw\\MasterZ\\Setup")
#define REG_PATH_STARTUP_RUN        _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")
#define REG_KEY_VERSION             _T("version")
#define REG_KEY_SERVER_URL          _T("ServerUrl")
#define REG_MACHINE_START_PATH      _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")
#define REG_PATH_SUMINFO_PRODUCT    _T("Software\\szw")


#define MC_PROCESS_NAME             _T("MC.exe")
#define AUTO_TASK_REULT_DELIMITER   _T("")

#define PRODUCT_NAME                _T("商务快车.企业版2.0")

struct TIP_DATA
{
	DWORD dwMessage;
	DWORD dwTipTime;                 //单位：毫秒
	CStdString strTipContent;
	CStdString strTipTitle;
	CStdString strTipInfo;

	TIP_DATA()
	{
		dwMessage = 0;
		dwTipTime = TIP_TIME;
		strTipContent = _T("");
		strTipTitle = PRODUCT_NAME;
		strTipInfo = PRODUCT_NAME;
	}
};

///////////////////////////自动任务相关结构///////////////////////////////
//任务信息结构
struct T_TASK_DATA
{
	CString strUID;            //用户ID(执行刷新任务的用户id)
	CString strUIPdtVer;       //用户产品版本id
	CString strExclusive;      //能否与其它任务同时执行
	CString strTimeMode;       //时间模式：0延迟执行、1定时执行
	CString strTime;           //延迟时间（以分为单位的整数）或定时时间（格式：hh:mm:ss）
	CString strType;           //任务类型
	CString strSubject;        //任务主题
	CString strData;           //任务数据
	CString strPostAddr;       //提交地址
	CString strPdtVer;         //当前客户端产品版本id
	CString strCUID;           //当前客户端用户id
	CString strExtraData;      //上层缓存的统计信息，格式为：首页排名关键词总量(;0)百度首页排名关键词总量(;0)前三页排名关键词总量(;0)完成抓取的词总量

	T_TASK_DATA(void)
	{
		strUID = _T("");
		strUIPdtVer = _T("");
		strExclusive = _T("");
		strTimeMode = _T("");
		strTime = _T("");
		strType = _T("");
		strSubject = _T("");
		strData = _T("");
		strPostAddr = _T("");
		strPdtVer = _T("");
		strCUID = _T("");
		strExtraData = _T("");
	}

	T_TASK_DATA &operator =(const T_TASK_DATA &tData)
	{
		if (this == &tData)
			return *this;

		strUID = tData.strUID;
		strUIPdtVer = tData.strUIPdtVer;
		strExclusive = tData.strExclusive;
		strTimeMode = tData.strTimeMode;
		strTime = tData.strTime;
		strType = tData.strType;
		strSubject = tData.strSubject;
		strData = tData.strData;
		strPostAddr = tData.strPostAddr;
		strPdtVer = tData.strPdtVer;
		strCUID = tData.strCUID;
		strExtraData = tData.strExtraData;
	}

	bool operator ==(T_TASK_DATA &tData) const
	{
		return (strUID == tData.strUID && strUIPdtVer == tData.strUIPdtVer 
			&& strExclusive == tData.strExclusive &&
			strTimeMode == tData.strTimeMode && strTime == tData.strTime &&
			strType == tData.strType && strSubject == tData.strSubject && 
			strData == tData.strData && strPostAddr == tData.strPostAddr &&
			strPdtVer == tData.strPdtVer && strCUID == tData.strCUID && strExtraData == tData.strExtraData);
	}

	CString GetTaskIdentify()
	{
		//也可以用任务数据作标识，但任务数据较大，需要考虑存储空间及效率问题
		return strUID + _T("_") + strType;
	}
};

//客户端与服务器交互参数
struct T_INTERACT_PARAM
{
	CString strTimeMode;       //时间模式：0延迟执行、1定时执行、2不再次执行
	CString strTime;           //延迟时间/定时时间
	// 	CString strRequestURL;     //请求地址
	// 	CString strResponseURL;    //发送结果地址
};

//从服务器返回的数据
struct T_DATA_FROM_SERVER
{
	T_INTERACT_PARAM tInteract;           //交互参数
	std::vector<T_TASK_DATA> vTask;       //任务列表
};

struct T_PRODUCT_INFO
{
	CString strProduct;            //产品名称
	CString strPdtFlag;            //产品的标识(包括产品及版本)
	CString strServer1;            
	CString strServer2;
	CString strFastServer;         //二者中较快的服务器
	CString strPdtVersionID;       //产品版本ID

	T_PRODUCT_INFO()
	{
		strProduct = _T("");
		strPdtFlag = _T("");
		strServer1 = _T("");
		strServer2 = _T("");
		strFastServer = _T("");
		strPdtVersionID = _T("");
	}
};

#endif

extern CLogTrace g_log;
// extern CInternetHttp g_InternetHttp;
extern CIniFile g_iniCfgFile;
extern CLock g_lock;