// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include <iostream>
#include <vector>
using namespace std;

#include "StdString.h"

#include "log4z.h"  
using namespace zsummer::log4z;
//extern LoggerId g_logger;

//#include "3rdparty/sqlite3/SQLiteOperate.h"


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


typedef struct _utilityVar 
{
	std::string sJsData;
	HANDLE hRecvJSData;				//����js���������ݵ��¼�
	LoggerId loggerId;
//	SQLiteOperate  sqlIte;
	_utilityVar()
	{
		sJsData = "";
		loggerId = 1;
		hRecvJSData = CreateEvent(nullptr, false, false, nullptr);
	}
}utilityVar, *putilityVar;


extern utilityVar g_utilityVar;