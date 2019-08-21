#include "stdafx.h"
#include "Keyword.h"


CKeyword::CKeyword(CUtilityThread *pUtility)
{
	m_pUtility = pUtility;
	initVariable();
}


CKeyword::~CKeyword(void)
{
}


struct TRAN_CHAR
{
	WCHAR _Char;
	BSTR _Val;
};	

TRAN_CHAR const_char[29]=
{
	{L' ', _T("+")},
	{L'!', _T("%21")},
	{L'\"', _T("%22")},
	{L'#', _T("%23")},
	{L'$', _T("%24")},
	//{L'%', _T("%25")},	//解决上面的%问题
	{L'&', _T("%26")},
	{L'\'', _T("%27")},
	{L'(', _T("%28")},
	{L')', _T("%29")},
	{L',', _T("%2C")},
	{L'+', _T("%2B")},
	{L'/', _T("%2F")},
	{L':', _T("%3A")},
	{L';', _T("%3B")},
	{L'<', _T("%3C")},
	{L'=', _T("%3D")},
	{L'>', _T("%3E")},
	{L'?', _T("%3F")},
	{L'[', _T("%5B")},
	{L'\\',_T("%5C")},
	{L']', _T("%5D")},
	{L'^', _T("%5E")},
	{L'`', _T("%60")},
	{L'{', _T("%7B")},
	{L'|', _T("%7C")},
	{L'}', _T("%7D")},
	{L'~', _T("%7E")},
	{0x0D, _T("%0D")},
	{0x0A, _T("%0A")}
};

void CKeyword::KeywordAnalysis(const CString &strMsg)
{
	//DWORD dwExitCode; 
	//GetExitCodeThread(m_hNewExe, &dwExitCode);
	////表示上一个searchKeyWord还在运行
	//while (dwExitCode == STILL_ACTIVE)
	//{
	//	Sleep(100);
	//}

	initVariable();
	m_strMsg = strMsg;
	m_thread = CreateThread(NULL, 0, &CKeyword::ThreadProc, this, 0, NULL);
}

void CKeyword::SafeExit(void)
{
	m_bCancel = true;
	if (m_hNewExe != NULL)
	{
		TerminateProcess(m_hNewExe, NULL);
	}
	//TerminateThread(m_thread,0);

	DWORD dwExitCode;
	GetExitCodeThread(m_thread, &dwExitCode);
	if (dwExitCode == STILL_ACTIVE)
	{
		WaitForSingleObject(m_thread, 500);
		CloseHandle(m_thread);
	}

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("Keyword程序已安全退出！"));
}

void CKeyword::CancelKeywordAnalysis(void)
{
	m_bCancel = true;
	if (m_hNewExe != NULL)
	{
		TerminateProcess(m_hNewExe, NULL);
	}
	//TerminateThread(m_thread,0);
	WaitForSingleObject(m_thread, 500);
	CloseHandle(m_thread);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("Keyword程序已取消搜索操作！"));
}

void CKeyword::initVariable(void)
{
	m_strKeyword = _T("");
	m_strCompany = _T("");
	m_strProvince = _T("");
	m_strCity = _T("");
	m_strBrand = _T("");
	m_strTexture = _T("");
	m_strMsg = _T("");
	m_bIsCompleted = false;
	m_bFlat = false;
	m_bCancel = false;	
	m_thread = NULL;
	m_hNewExe = NULL;
}


DWORD WINAPI CKeyword::ThreadProc(LPVOID lpParameter)
{
	CKeyword *pThis = (CKeyword *)lpParameter;
	pThis->OpenAndConnectProc();
	pThis->SendCombineDataToProc(pThis->m_strMsg);
	return 0;
}

void CKeyword::OpenAndConnectProc(void)
{
	TCHAR szPath[MAX_PATH] = {0};
	TCHAR szPort[10] = {0};

	int nPort = m_comm.GetPort();
	swprintf(szPort, _T(" %d"), nPort);

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	bool fRet = CreateProcess(g_pGlobalData->dir.GetKeyWordProcPath(), szPort, NULL, FALSE, NULL, NULL, NULL, NULL, &si, &pi);
	m_hNewExe = pi.hProcess;
	//m_hNewExe = ShellExecute(NULL, _T("open"), g_pGlobalData->dir.GetKeyWordProcPath(), szPort, NULL, SW_HIDE);
	if (!fRet)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("SearchKeyword程序打开失败！err:%d"), GetLastError());
		ReturnDataToUI(_T(""), 0);
	}

	int nInit = m_comm.Init(nPort, this);
	if (-1 == nInit)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("CommClient初始化失败！"));
		ReturnDataToUI(_T(""), 0);
		TerminateProcess(m_hNewExe, NULL);
	}
}


void CKeyword::SendCombineDataToProc(CString strData)
{
	if (m_bCancel)
	{
		return;
	}
	CStdStrUtils utl;
	std::vector<CStdString> vRes;
	utl.SplitString(CStdString(strData.GetBuffer()), _T("\,"), vRes);

	vector <CStdString>::size_type res = vRes.size();
	if (7 == res)
	{
		m_strKeyword = vRes[0];
		m_strCompany = vRes[1];
		m_strProvince = vRes[2];
		m_strCity = vRes[3];
		m_strBrand = vRes[4];
		m_strTexture = vRes[5];
	}
	else
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("数据解析格式不正确，data: %s"), strData);
		ReturnDataToUI(_T(""), 0);
		TerminateProcess(m_hNewExe, NULL);
	}
	

	HandleData(m_strKeyword, m_strCompany, m_strProvince, m_strCity, m_strBrand, m_strTexture);
	CString strCombineData = CombineData();
	if (m_bCancel)
	{
		return;
	}
	CString strEncryptData = EncryptData(strCombineData);
	if (m_bCancel)
	{
		return;
	}
	strEncryptData = _T("seoword(;0)") + strEncryptData;
	m_comm.SendData(strEncryptData);
}

void CKeyword::OnReceive(CString strData)
{
	if (m_bCancel)
	{
		return;
	}
	if (strData == _T("DISCONNECTED") && m_bIsCompleted == false && m_bCancel == false && m_bFlat == false)      //socket断开连接
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("SearchKeyword程序异常！err:%d"), GetLastError());
		ReturnDataToUI(strData, 0);
		TerminateProcess(m_hNewExe, NULL);
		return;
	}
	else if (strData == _T("DISCONNECTED") && m_bIsCompleted == false && m_bCancel == true)
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Keyword取消搜索！data：%s"), strData);
		TerminateProcess(m_hNewExe, NULL);
		return;
	}
	else if (strData == _T("DISCONNECTED") && m_bIsCompleted == true)
	{
		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Keyword搜索全部完成！data：%s"), strData);
		TerminateProcess(m_hNewExe, NULL);
		return;
	}

	int res = strData.Find(_T("(;0)"));
	if (0 == res)          //最后一次返回"(;0)%d(;0)%d(;0)"表示结束
	{
		int temp = strData.Find(_T("(;0)"), res + 4);
		CString strTemp = strData.Right(strData.GetLength() - temp - 4);
		res = strTemp.Find(_T("(;0)"));
		CString strCount = strTemp.Left(res);
		int nCount = _ttoi(strCount);

		if (nCount == 0)             //返回标记中"(;0)%d(;0)nCount(;0)",如果nCount等于0代表没有找到一条
		{
			ReturnDataToUI(strData, 0);
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("Keyword搜索失败！data：%s"), strData);
		}
		else
		{
			m_bIsCompleted = true;
			ReturnDataToUI(strData, 2);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("部分关键词搜索完成，出现异常或百度搜不到其它关键词! data: %s"), strData);
		}	
		TerminateProcess(m_hNewExe, NULL);
	}
	else
	{
		int temp = strData.Find(_T("(;0)"));
		CString strTemp = strData.Right(strData.GetLength() - temp - 4);
		res = strTemp.Find(_T("(;0)"));
		CString strCount = strTemp.Left(res);

		if (strCount.IsEmpty())                 //如果返回的第二个参数是空，代表获取失败
		{
			if (false == m_bFlat)
			{
				ReturnDataToUI(strData, 0);
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("Keyword搜索失败！data：%s"), strData);
			}
			else
			{
				ReturnDataToUI(strData, 2);
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("部分关键词搜索完成，出现异常或百度搜不到其它关键词! data: %s"), strData);
			}
			TerminateProcess(m_hNewExe, NULL);
		}
		else
		{
			m_bFlat = true;
			ReturnDataToUI(strData, 1);
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Keyword搜索当条成功！data：%s"), strData);
		}	
	}
}


void CKeyword::ReturnDataToUI(const CString &strData, DWORD flag)
{
	m_pUtility->ReturnDataToUI(strData, MSG_KEYWORD_ANALYSIS, flag);
}

void CKeyword::HandleData(CString strKeyword, CString strCompany, CString strProvince, 
								CString strCity, CString strBrand, CString strTexture)
{
	if (!strProvince.IsEmpty())
	{
		CString strKey1 = strProvince + strKeyword + _T("代理");
		CString strKey2 = strProvince + strKeyword + _T("批发");
		CString strKey3 = strProvince + strKeyword + _T("厂家");
		CString strKey4 = strProvince + strKeyword + _T("价格");
		CString strKey5 = strProvince + strCompany + strKeyword;
		CString strKey6 = strProvince + strBrand + strKeyword;
		CString strKey7 = strProvince + strCompany + strBrand + strKeyword;
		CString strKey8 = strProvince + strBrand + strCompany + strKeyword;

		DataFormat(strKey1);
		DataFormat(strKey2);
		DataFormat(strKey3);
		DataFormat(strKey4);
		DataFormat(strKey5);
		DataFormat(strKey6);
		DataFormat(strKey7);
		DataFormat(strKey8);
	} 

	if (!strCity.IsEmpty())
	{
		CString strKey9 = strCity + strKeyword + _T("代理");
		CString strKey10 = strCity + strKeyword + _T("批发");
		CString strKey11 = strCity + strKeyword + _T("厂家");
		CString strKey12 = strCity + strKeyword + _T("价格");
		CString strKey13 = strCity + strCompany + strKeyword;
		CString strKey14 = strCity + strBrand + strKeyword;
		CString strKey15 = strCity + strCompany + strBrand + strKeyword;
		CString strKey16 = strCity + strBrand + strCompany + strKeyword; 

		DataFormat(strKey9);
		DataFormat(strKey10);
		DataFormat(strKey11);
		DataFormat(strKey12);
		DataFormat(strKey13);
		DataFormat(strKey14);
		DataFormat(strKey15);
		DataFormat(strKey16);
	}

	if (strProvince.IsEmpty() && strCity.IsEmpty())
	{
		CString strKey21 =strKeyword + _T("代理");
		CString strKey22 =strKeyword + _T("批发");
		CString strKey23 =strKeyword + _T("厂家");
		CString strKey24 =strKeyword + _T("价格");
		CString strKey25 =strCompany + strBrand + strKeyword;
		CString strKey26 =strBrand + strCompany + strKeyword; 

		DataFormat(strKey21);
		DataFormat(strKey22);
		DataFormat(strKey23);
		DataFormat(strKey24);
		DataFormat(strKey25);
		DataFormat(strKey26);
	}

	CString strKey17 = strCompany + strKeyword;
	CString strKey18 = strBrand + strKeyword;
	CString strKey19 = strBrand + strKeyword + strCompany;
	CString strKey20 = strTexture + strKeyword;

	DataFormat(strKey17);
	DataFormat(strKey18);
	DataFormat(strKey19);
	DataFormat(strKey20);
}


void CKeyword::DataFormat(CString strKeyword)
{
	CString strCombineKeyword = strKeyword + _T("(;2)") + GetUTF8String(strKeyword) + _T("(;2)0(;1)");
	m_vKeyword.push_back(strCombineKeyword);
}


CString CKeyword::CombineData()
{
	CString strResult;
	vector<CString>::iterator iter;

	for (iter = m_vKeyword.begin(); iter != m_vKeyword.end(); ++iter)
	{
		strResult += *iter;
	}
	m_vKeyword.clear();      //容器用完清空

	strResult += _T("(;0)3(;0)");
	strResult += _T("china.makepolo.com(;1)cn.china.cn(;1)99inf.com(;1)qjy168.com(;1)");
	strResult += _T("china.nowec.com(;1)kvov.com(;1)hc360.com(;1)b2b168.com(;1)");
	strResult += _T	("sz.100ye.com(;1)cnlinfo.net(;1)shengyidi.com(;1)sg560.com(;1)");
	strResult += _T	("youboy.com(;1)99114.com(;1)china.eb80.com(;1)51sole.com(;1)qy6.com(;1)");
	strResult += _T	("007swz.com(;1)china.toocle.com(;1)buy.ch.gongchang.com(;1)net114.com(;1)");
	strResult += _T	("bestb2b.com(;1)china.coovee.net(;1)haolaba.com(;1)zhongsou.net(;1)");
	strResult += _T	("cntrades.com(;1)ic98.com(;1)cn.made-in-china.com(;1)qianyan.biz(;1)");
	strResult += _T	("sugou.com(;1)ccn.mofcom.gov.cn(;1)b2b.cn(;1)jdol.com.cn(;1)china.herostart.com(;1)");
	strResult += _T	("gtobal.com(;1)autocontrol.com.cn(;1)jiaomai.com(;1)booksir.com.cn(;1)");
	strResult += _T	("gongqiu.com.cn(;1)wanye68.com(;1)waaku.com(;1)hardwareinfo.cn(;1)86mai.com(;1)");
	strResult += _T	("chinaccm.com(;1)eb.cn(;1)longooog.com(;1)vooec.com(;1)cn716.com(;1)587766.com(;1)baimao.com"); 
	strResult += _T("(;0)50(;0)");
	strResult += _T("么(;1)很(;1)多少(;1)怎么(;1)这么(;1)哪里(;1)什么(;1)下载(;1)如何");
	strResult += _T("(;0)1(;0)");

	return strResult;
}


CString CKeyword::EncryptData(CString strCombineData)
{
	CStdString strResult;
	m_encryptCode.ZipEncodeStdString(strCombineData.GetString(), strResult);
	
	return strResult;
}


/*wstring CKeyword::ANSIToUnicode(const string& str)
{
	int len = 0;
	len = str.length();
	int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0); 

	wchar_t * pUnicode;  
	pUnicode = new wchar_t[unicodeLen+1];  
	memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));  

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, (LPWSTR)pUnicode, unicodeLen);  
	wstring rt;  
	rt = (wchar_t*)pUnicode;
	delete pUnicode; 

	return rt;  
}*/


/*string CKeyword::UnicodeToANSI(const wstring& str)
{
	char* pElementText;
	int iTextLen;
	iTextLen = WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

	pElementText = new char[iTextLen + 1];
	memset(( void* )pElementText, 0, sizeof( char )*( iTextLen + 1 ));

	::WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, pElementText, iTextLen, NULL, NULL);
	string strText;
	strText = pElementText;
	delete[] pElementText;

	return strText;
}*/


/*CString CKeyword::KeywordToUrl(CString strKeyword)
{
	CString strResult = _T("");
	string strUnicodeToANSI = UnicodeToANSI(strKeyword.GetString());
	string strTemp = m_base64.EncodeBase64(strUnicodeToANSI);

	strResult = ANSIToUnicode(strTemp).c_str();
	return strResult;
}*/

CString CKeyword::GetUTF8String(IN CString sour)
{
	CString strResult = _T("");
	BSTR bstrTmp;
	BSTR bstrSrc = sour.AllocSysString();
	ConvertUrlToUTF8(bstrSrc, &bstrTmp);

	strResult = bstrTmp;

	if (bstrSrc)
		SysFreeString(bstrSrc);

	if (bstrTmp)
		SysFreeString(bstrTmp);

	return strResult;
}

BOOL CKeyword::ConvertUrlToUTF8(IN BSTR sour,OUT BSTR *dest)
{
	if(sour == NULL||dest == NULL)
	{
		return FALSE;
	}

	UINT i = 0;
	UINT nLen = SysStringLen(sour);
	_bstr_t bstrDest;

	for(i=0; i<nLen; i++)
	{	
		if (sour[i] > 0x80)
		{
			BSTR tmp = SysAllocStringLen(NULL, 6);
			tmp[5] = 0;
			wsprintfW(tmp, _T("%c"), sour[i]);
			PCHAR tmpByte = NULL;
			ToMultiByteUTF8(tmp, &tmpByte);
			BYTE ByteVal = tmpByte[0];
			wsprintfW(tmp, _T("%%%X"), ByteVal);
			bstrDest += tmp;
			ByteVal = tmpByte[1];
			wsprintfW(tmp, _T("%%%X"), ByteVal);
			bstrDest += tmp;
			ByteVal = tmpByte[2];
			wsprintfW(tmp, _T("%%%X"), ByteVal);
			bstrDest += tmp;
			delete []tmpByte;
			BStrFree(tmp);
		}
		else
		{
			BSTR tmp = NULL; 
			tmp = FindTranChar(sour[i]);

			if (tmp != NULL)
			{
				bstrDest += tmp;
			}
			else
			{
				tmp = SysAllocStringLen(NULL, 2);
				tmp[1] = 0;
				wsprintfW(tmp, _T("%c"), sour[i]);
				bstrDest += tmp;
				BStrFree(tmp);
			}
		}
	}
	*dest = SysAllocString(bstrDest.GetBSTR());
	return TRUE;
}


INT CKeyword::ToMultiByteUTF8(IN BSTR pszSource, OUT PCHAR *pszOut) 
{
	int nLanguage = CP_UTF8;
	
	INT nLength = WideCharToMultiByte(nLanguage, 0, pszSource, -1, NULL, 0, NULL, FALSE);
	*pszOut = new char[nLength];

	WideCharToMultiByte(nLanguage, 0, pszSource, -1, *pszOut, nLength, NULL, FALSE);

	return nLength;
}


BSTR CKeyword::FindTranChar(WCHAR Param)
{
	for (int i=0; i<29; i++)
	{
		if (Param == const_char[i]._Char)
		{
			return const_char[i]._Val;
		}		
	}

	return NULL;
}