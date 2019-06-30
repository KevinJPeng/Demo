
// stdafx.cpp : 只包括标准包含文件的源文件
// EngineValidation.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"


stack<pEngineInfo> g_EngineInfos;
vector<CString> g_ValidationText;
CLock g_critSection;
map<int, int> g_mapEngineOpenCount;
HANDLE g_notifyUiEvent;