#include "stdafx.h"
#include "IKeyRW.h"


IKeyRW::IKeyRW()
{
	m_pKeyObj = nullptr;
	m_hDll = nullptr;
}


IKeyRW::~IKeyRW()
{
	
	DELETEMEM pfnDelete = (DELETEMEM)GetProcAddress(m_hDll, "DeleteMem");
	if (pfnDelete && m_pKeyObj)
		pfnDelete(m_pKeyObj);

	if (nullptr != m_hDll)
	{
		//释放dll
		FreeLibrary(m_hDll);
	}
}

TCHAR * IKeyRW::GetCurPath(void)
{
	static TCHAR s_szCurPath[MAX_PATH] = { 0 };

	GetModuleFileName(NULL, s_szCurPath, MAX_PATH);

	TCHAR *p = _tcsrchr(s_szCurPath, '\\');
	*p = 0;

	return s_szCurPath;
}

E_KEYRET IKeyRW::InitDll(_EM_KEYTYPE_ eType)
{
	if (nullptr == m_pKeyObj)
	{
		//加载dll
		CString strDllPath = CString(GetCurPath()) + _T("\\KeyRW.dll");
		m_hDll = LoadLibrary(strDllPath);

		if (!m_hDll)
			return KRET_INITFAIL;

		//调用创建对象
		CREATEOBJ pfnCreateObj = (CREATEOBJ)GetProcAddress(m_hDll, "CreateObj");

		if (!pfnCreateObj)
			return KRET_LOADDLLFAIL;

		//创建对象
		m_pKeyObj =pfnCreateObj(eType);

		if (!m_pKeyObj)
			return KRET_LOADDLLFAIL;		
	}

	return KRET_SUCCESS;
}

E_KEYRET IKeyRW::EncryptData(const CString& strSrcData, const CString& strKey, CString& strEncryptData)
{
	if (nullptr != m_pKeyObj)
	{
		return m_pKeyObj->EncryptData(strSrcData, strKey, strEncryptData);
	}
	else
	{
		strEncryptData = _T("");
		return KRET_INITFAIL;
	}
}

E_KEYRET IKeyRW::DecryptData(const CString& strEncrtptData, const CString& strKey, CString& strScrData)
{
	if (nullptr != m_pKeyObj)
	{
		return m_pKeyObj->DecryptData(strEncrtptData, strKey, strScrData);
	}
	else
	{
		strScrData = _T("");
		return KRET_INITFAIL;
	}
}
