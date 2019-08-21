// stdafx.cpp : 只包括标准包含文件的源文件
// YunTask.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"
#include "TaskInfo.h"

std::map<CString, int> g_TypeMap;
bool g_bDelFile;

void InitTypeMap()
{
	g_TypeMap[_T("QuickPhoto")]  = eType_QuickPhoto;
	g_TypeMap[_T("ShopTraffic")] = eType_ShopTraffic;
	g_TypeMap[_T("Inforefr")]    = eType_Inforefr;
	g_TypeMap[_T("MainTask")]	 = eType_MainTask;
	g_bDelFile = true;
}


