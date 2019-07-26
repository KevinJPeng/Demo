#include "stdafx.h"
#include "WebBrowserExUI.h"
#include <atlcomcli.h>


int g_FunSub(int x, int y)
{
	return (x - y);
}

int JSCallCPP(string _sParam)
{
	g_utilityVar.sJsData = _sParam;
	//消息来源(;0)消息类型(;0)数据  1(;0)2(;0)深圳
	if (-1 != g_utilityVar.sJsData.find("(;0)"))
	{
		SetEvent(g_utilityVar.hRecvJSData);
		LOG_INFO(g_utilityVar.loggerId, "收到JS传过来的数据：" << g_utilityVar.sJsData);
		return 0;
	}
	LOG_ERROR(g_utilityVar.loggerId, "收到JS传过来的数据：" << g_utilityVar.sJsData);
	return -1;
}

CWebBrowserExUI::CWebBrowserExUI()
{
}


CWebBrowserExUI::~CWebBrowserExUI()
{
}


LPCTSTR CWebBrowserExUI::GetClass() const
{
	return _T("WebBrowserExUI");
}

LPVOID CWebBrowserExUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcsicmp(pstrName, _T("WebBrowserEx")) == 0)
		return static_cast<CWebBrowserExUI*>(this);

	return CActiveXUI::GetInterface(pstrName);
}


HRESULT CWebBrowserExUI::GetIDsOfNames(const IID& riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
{
	//DISP ID 从200开始
	if (_tcscmp(rgszNames[0], _T("JSCallCPP")) == 0)
		*rgDispId = 500;

	return S_OK;
}

HRESULT CWebBrowserExUI::Invoke(DISPID dispIdMember, const IID& riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	//MyOutputDebugStringW(L"%d\n", dispIdMember);
	switch (dispIdMember)
	{
	case 500:
	{
				// 注意参数顺序，反向
				VARIANTARG *varArg = pDispParams->rgvarg;
				string sParam = static_cast<_bstr_t>(varArg[0]);
// 				int x = _ttoi(static_cast<_bstr_t>(varArg[1]));
// 				int y = _ttoi(static_cast<_bstr_t>(varArg[0]));
				int n = JSCallCPP(sParam);
				*pVarResult = CComVariant(n);
				return S_OK;
	}

	default:
		break;
	}

	return CWebBrowserUI::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}
