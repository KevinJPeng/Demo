
// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// EngineValidation.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"


stack<pEngineInfo> g_EngineInfos;
vector<CString> g_ValidationText;
CLock g_critSection;
map<int, int> g_mapEngineOpenCount;
HANDLE g_notifyUiEvent;