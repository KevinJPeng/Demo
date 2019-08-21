// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once
#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

/////////手动添加/////////////////

#include "targetver.h" 
#include <afxwin.h> 
#ifndef _SECURE_ATL 
#define _SECURE_ATL 1 
#endif 
#ifndef VC_EXTRALEAN 
#define VC_EXTRALEAN // 从 Windows 头中排除极少使用的资料 
#endif 

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
//  #include <windows.h>
#include <afxwin.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "UIlib.h"

using namespace DuiLib;
#ifdef _DEBUG
#   ifdef _UNICODE
#       pragma comment(lib, "..\\..\\lib\\DuiLib_ud.lib")
#   else
#       pragma comment(lib, "..\\..\\lib\\DuiLib_d.lib")
#   endif
#else
#   ifdef _UNICODE
#       pragma comment(lib, "..\\..\\lib\\DuiLib_u.lib")
#   else
#       pragma comment(lib, "..\\..\\lib\\DuiLib.lib")
#   endif
#endif

//#pragma comment(lib, "..\\..\\lib\\ZipEnCode.lib")
#ifdef _DEBUG
#pragma comment(lib, "../../lib/ZipEnCode_d.lib")
#else
#pragma  comment(lib, "../../lib/ZipEnCode.lib")
#endif

// TODO: 在此处引用程序需要的其他头文件
#include <vector>
#include <queue>
#include <map>
#include <atlstr.h>
#include "Reg.h"
#include "Trace.h"
#include "StdString.h"
#include "IniFile.h"
#include "CommFunc.h"
#include "commondef.h"
#include "GetSearchData.h"
#include "ClientInterface.h"
#include "ZipEncode\ZipEncode.h"
#include "Base64.h"
#include "..\duilib\Utils\winimplbase.h"
#include "..\threadmodel\ThreadManage.h"
#include "..\threadmodel\UiHelper.h"
#include "GlobalDef.h"
#include "CustomWebEventHandler.h"
#include "ClientDiagnoseBase.h"
#include "IXMLRW.h"

extern CLogTrace g_log;
extern T_GLOBAL_DATA g_globalData;


#define _NEW(p, type) \
	(p) = new type;\
	if (!(p))\
	{\
	printf("new operator fail!!");\
	}

#define _NEWA(p, type, count) \
	(p) = new type[count];\
	if (!(p))\
	{\
	printf("new operator fail!!");\
	}

#define _DELETE(p) \
	if (p)\
	{\
	delete (p);\
	(p) = NULL;\
	}

#define _DELETEA(p) \
	if (p)\
	{\
	delete[] (p);\
	(p) = NULL;\
	}
