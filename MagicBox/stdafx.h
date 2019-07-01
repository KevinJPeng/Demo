// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO:  在此处引用程序需要的其他头文件
#include <iostream>
#include <vector>
using namespace std;

#include "StdString.h"

#include "log4z.h"  
using namespace zsummer::log4z;
extern LoggerId g_logger;

#define _NEW(p, type) \
	(p) = new type; \
if (!(p))\
{\
	printf("new operator fail!!"); \
}

#define _NEWA(p, type, count) \
	(p) = new type[count]; \
if (!(p))\
{\
	printf("new operator fail!!"); \
}

#define _DELETE(p) \
if (p)\
{\
	delete (p); \
	(p) = NULL; \
}

#define _DELETEA(p) \
if (p)\
{\
	delete[](p); \
	(p) = NULL; \
}

