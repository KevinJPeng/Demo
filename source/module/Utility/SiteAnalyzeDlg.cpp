// SiteAnalyzeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SiteAnalyze.h"
#include "SiteAnalyzeDlg.h"
//#include "afxdialogex.h"


// CSiteAnalyzeDlg 对话框

const DWORD RETRYTIMES = 3;   //重试三次
const DWORD TIMEOUTCOUNTS = 8;  //超时次数，每次1000毫秒


IMPLEMENT_DYNAMIC(CSiteAnalyzeDlg, CDialog)

CSiteAnalyzeDlg::CSiteAnalyzeDlg(CString strUrl, CSiteAnalyze* pSearch, CWnd* pParent /*=NULL*/)
	: CDialog(CSiteAnalyzeDlg::IDD, pParent)
{
	m_strUrl = strUrl;
	m_pWebSearch = pSearch;
	m_strDomain = _T("");
	m_strVec.clear();
	hThread = NULL;
	m_bIsLoadComplete = false;
	m_dwTimeOutCounts = 0;
	m_dwRetryCounts = 0;
}

CSiteAnalyzeDlg::~CSiteAnalyzeDlg()
{
}

void CSiteAnalyzeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WEB_SEARCH, m_WebBrowser);
}


BEGIN_MESSAGE_MAP(CSiteAnalyzeDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSiteAnalyzeDlg 消息处理程序


BOOL CSiteAnalyzeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW);//从任务栏中去掉.

	//隐藏对话框(win8下使用会出现异常)
	/*WINDOWPLACEMENT wp;
	wp.length=sizeof(WINDOWPLACEMENT);
	wp.flags=WPF_RESTORETOMAXIMIZED;
	wp.showCmd=SW_HIDE;
	SetWindowPlacement(&wp);*/
	::SetWindowPos(this->m_hWnd, NULL, 0, 0, 0, 0, NULL);

	m_WebBrowser.SetSilent(TRUE);

	SetTimer(1, 100, NULL);
	m_strUrl = _T("http://seo.chinaz.com/?host=") + m_strUrl;  //不能放到StartSearch中，重试会修改url

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

DWORD CSiteAnalyzeDlg::StartSearch()
{
	DWORD dwTimeOutBegin = 0;
	DWORD dwTimeOutEnd = 0;
	dwTimeOutBegin = GetTickCount();

	//防止web控件打开网页读缓存问题
	VARIANT index = {0};
	V_VT(&index) = VT_I4;
	V_I4(&index) = navTrustedForActiveX | navNoReadFromCache | navNoWriteToCache;

	m_WebBrowser.Navigate(_T("about:blank"), NULL, NULL, NULL, NULL);
	Sleep(200);
	m_WebBrowser.Navigate(m_strUrl, &index, NULL, NULL, NULL);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("搜索的Url为：%s！"), m_strUrl);

	SetTimer(2, 1000, NULL);


	return 0;
}

CString CSiteAnalyzeDlg::GetSearchData(void)
{
	HRESULT hret;
	CString strHtmlData = _T("");
	CString strResultData = _T("");
	CString strAllData = _T("");
	CComBSTR bstr = NULL;
	CComPtr<IHTMLDocument2> pDocument2;
	CComPtr<IDispatch> pDispatch;
	CComPtr<IDispatch> pDispatchTemp;
	
	CComQIPtr<IHTMLElementCollection> pAllCollection;
	CComQIPtr<IHTMLElementCollection> pTags;

	CComPtr<IHTMLElement> pElem;

	try
	{
		do 
		{
			pDispatch = m_WebBrowser.GetDocument();
			if (NULL == pDispatch)
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("web控件GetDocument失败！"));
				strResultData = _T("000");
				break;
			}

			hret = pDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&pDocument2);
			if (FAILED(hret))
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("web控件QueryInterface失败！"));
				strResultData = _T("000");
				break;
			}

			hret = pDocument2->get_body(&pElem);
			if (SUCCEEDED(hret))
			{
				pElem->get_innerHTML(&bstr);
				strAllData = bstr;
				strAllData.MakeLower();
			}

			hret = pDocument2->get_all(&pAllCollection);
			if (FAILED(hret) ) 
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("web控件get_all失败！"));
				strResultData = _T("000");
				break;
			}

			hret = pAllCollection->tags(COleVariant(_T("div")), &pDispatchTemp);
			if (FAILED(hret) ) 
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("web控件tags失败！"));
				strResultData = _T("000");
				break;
			}

			pTags = pDispatchTemp;

			strHtmlData = GetHtmlData(pTags);
			if (strHtmlData == _T("000"))
			{
				strResultData = _T("000");
				break;
			}

			/*if (JudegeIsValidUrl(strHtmlData))
			{*/
			strResultData = PaserData(strHtmlData,strAllData);
/*
			}
			else
			{
				strResultData = _T("");
			}*/
		} 
		while (0);

	}
	catch (...)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("处理html时出现异常！作为搜索失败的情况！"));
		strResultData = _T("000");   //搜索失败的情况
	}

	return strResultData;
}


CString CSiteAnalyzeDlg::GetHtmlData(IHTMLElementCollection *pElementColl)
{
	CString strPage = _T("");

	CComQIPtr<IHTMLElement> pElement;
	IDispatch  *pDisp = NULL;
	HRESULT hr;
	CString strTemp = _T("seoinfo");


	VARIANT index = {0};
	V_VT(&index) = VT_I4;
	V_I4(&index) = 0;

	VARIANT varID = {0};
	VariantInit(&varID);
	varID.vt = VT_BSTR;

	varID.bstrVal = strTemp.AllocSysString();
	hr = pElementColl->item(varID,index,&pDisp);
	SysFreeString(varID.bstrVal);

	if (FAILED(hr) || pDisp == NULL)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("web控件item失败！"));
		strPage = _T("000");
		return strPage;
	}

	pElement = pDisp;
	pDisp->Release();
	
	CComBSTR bstr = NULL;
	pElement->get_outerHTML(&bstr);
	strPage = bstr;

	strPage.MakeLower();       //将获取的数据英文全变成小写

	return strPage;
}


BOOL CSiteAnalyzeDlg::JudegeIsValidUrl(CString strPageHtml)
{
	int nPos = 0;
	nPos = strPageHtml.Find(_T("请输入网站地址"));
	if (nPos != -1)
	{
		int res = 0;
		res = strPageHtml.Find(_T("value"), nPos);

		if (_T("=") == strPageHtml.Mid(res+5, 1))
		{
			CString strTemp = strPageHtml.Right(strPageHtml.GetLength()-res-6);
			int ret = strTemp.Find(_T("\x20"));    //查找value后面的空格
			m_strDomain = strTemp.Left(ret);   
			if (m_strDomain.Find(_T("查")) != -1)
			{
				return FALSE;
			}

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}


CString CSiteAnalyzeDlg::PaserData(CString strPageHtml, const CString &strAllData)
{
	FetchData(strPageHtml, _T("baiduapp/"), _T(".gif"), _T("$SEO-BaiduPR$"), _T("百度权重"),_T("</a>"));
	FetchData(strPageHtml, _T("rank_"), _T(".gif"), _T("$SEO-GooglePR$"), _T("id=pr"), _T("</a>"));
	FetchData(strPageHtml, _T("<font color=red>"), _T("</font>"), _T("$SEO-AntiChain$"), _T("id=outlink1"), _T("</a>"));   //id后面留空格防止数据处理时可能出现巧合
	FetchData(strPageHtml, _T("<font color=red>"), _T("</font>"), _T("$SEO-InnerChain$"), _T("id=innerlink"), _T("</a>"));
	FetchData(strPageHtml, _T("target=_blank>"), _T("（创建"), _T("$SEO-DomainAge$"), _T("域名年龄"));
	FetchData(strPageHtml, _T("target=_blank>"), _T("</a>"), _T("$ICP-Num$"), _T("备案号："), _T("</div>"));
	//FetchData(strPageHtml, _T("性质:&nbsp;</FONT>"), _T("&nbsp;&nbsp"), m_strProperty, _T("$$"));
	FetchData(strPageHtml, _T("<strong>"), _T("</strong>"), _T("$ICP-CompanyName$"),_T("名称"),_T("</div>"));
	FetchData(strPageHtml, _T("<strong>"), _T("</strong>"), _T("$ICP-AuditTime$"),_T("审核时间"),_T("/div"));
	FetchData(strAllData, _T("target=_blank>"), _T("</a>"), _T("$Baidu-Included$"), _T("id=seo_baidupages "));
	FetchData(strAllData, _T("target=_blank>"), _T("</a>"), _T("$Google-Included$"), _T("id=seo_googlepages"));
	FetchData(strAllData, _T("target=_blank>"), _T("</a>"), _T("$360-Included$"), _T("id=seo_pages360"));
	FetchData(strAllData, _T("target=_blank>"), _T("</a>"), _T("$Sougou-Included$"), _T("id=seo_sogoupages"));
	FetchData(strAllData, _T("target=_blank>"), _T("</a>"), _T("$Baidu-AntiChain$"), _T("id=seo_baidulink "));
	FetchData(strAllData, _T("target=_blank>"), _T("</a>"), _T("$Google-AntiChain$"), _T("id=seo_googlelink"));
	FetchData(strAllData, _T("target=_blank>"), _T("</a>"), _T("$360-AntiChain$"), _T("id=seo_link360"));

	CString strResult = _T("");
	strResult = _T("$HandleUrl$=") + m_strDomain + _T(",");

	vector<CString>::iterator iter;
	for (iter = m_strVec.begin(); iter != m_strVec.end(); ++iter)
	{
		strResult += *iter;
		strResult += _T(",");
	}

	strResult += _T("$Sougou-AntiChain$=--,");

	return strResult;
}


//参数1：源代码
//参数2：寻找数据前的一段字符
//参数3：寻找数据后的一段字符
//参数4：给得到的结果加上的标示符
//参数5：定位id
void CSiteAnalyzeDlg::FetchData(CString strSourceCode, CString strBeforeWord, CString strLastWord, 
			const CString &strFlagName, CString id, CString strFindFlag)
{
	CString strResult = _T("");

	if (_T("") == id)
	{
		int nLength = 0;
		int res = 0;
		CString strTemp = _T("");

		nLength = strBeforeWord.GetLength();
		res = strSourceCode.Find(strBeforeWord);
		if (res != -1)
		{
			strTemp = strSourceCode.Right(strSourceCode.GetLength() - res - nLength);
			res = strTemp.Find(strLastWord);
			if (res != -1)
			{
				strResult = strTemp.Left(res);
			}
			else
			{
				strResult = _T("--");
			}
		}
		else
		{
			strResult = _T("--");
		}
		
	}
	else
	{
		int nLength = 0;
		int res1 = 0;
		int res2 = 0;
		int res3 = 0;
		int res4 = 0;
		CString strTemp1 = _T("");
		CString strTemp2 = _T("");
		CString strTemp3 = _T("");

		nLength = strBeforeWord.GetLength();
		res1 = strSourceCode.Find(id);
		if (res1 != -1)
		{
			strTemp1 = strSourceCode.Right(strSourceCode.GetLength()- res1);
			res2 = strTemp1.Find(strFindFlag);
			if (res2 != -1)
			{
				strTemp2 = strTemp1.Left(res2);
				res3 = strTemp2.Find(strBeforeWord);
				if (res3 != -1)
				{
					strTemp3 = strTemp2.Right(strTemp2.GetLength() - res3 - nLength);
					res4 = strTemp3.Find(strLastWord);
					if (res4 != -1)
					{
						strResult = strTemp3.Left(res4);
						if (strResult.Find(_T("#999999")) != -1)
						{
							strResult.Replace(_T("<font color=#999999>"), NULL);
							strResult.Replace(_T("</font>"), NULL);
						}
					}
					else
					{
						strResult = _T("--");
					}
				}
				else
				{
					strResult = _T("--");
				}
			}
			else
			{
				strResult = _T("--");
			}
			
		}
		else
		{
			strResult = _T("--");
		}	
	}


	strResult = strFlagName + _T("=") + strResult;
	m_strVec.push_back(strResult);
}

void CSiteAnalyzeDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (nIDEvent == 1)
	{
		KillTimer(nIDEvent);
		StartSearch();	
	}

	if (nIDEvent == 2)
	{
		if (!m_bIsLoadComplete)
		{
			++m_dwTimeOutCounts;
		}
		else
		{
			KillTimer(nIDEvent);
		}

		if (m_dwTimeOutCounts > TIMEOUTCOUNTS)//超过TIMEOUTCOUTS*1000毫秒，默认失败
		{
			++m_dwRetryCounts;
			if (m_dwRetryCounts > RETRYTIMES)
			{
				KillTimer(nIDEvent);
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("搜索超时！"));
				m_pWebSearch->ReturnDataToUI(_T(""), 0);   //搜索失败
				PostQuitMessage(0);
			}
			else
			{
				KillTimer(nIDEvent);
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("重试！"));
				m_dwTimeOutCounts = 0;   //重试时超时清零
				StartSearch();
			}	
		}
	}

	CDialog::OnTimer(nIDEvent);
}


DWORD CSiteAnalyzeDlg::ExecuteSearchData(void)
{
	CString strSearchResult = _T("");

	
	strSearchResult = GetSearchData();
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("搜索的结果为:%s！"),  strSearchResult);
	
	if (strSearchResult != _T(""))
	{
		if (strSearchResult == _T("000"))
		{
			m_pWebSearch->ReturnDataToUI(_T(""), 0);   //搜索失败
		}
		else
		{
			m_pWebSearch->ReturnDataToUI(strSearchResult, 1);
		}
	}
	else
	{
		m_pWebSearch->ReturnDataToUI(_T(""), 2);    //域名错误
	}

	return 0;
}


BEGIN_EVENTSINK_MAP(CSiteAnalyzeDlg, CDialog)
	ON_EVENT(CSiteAnalyzeDlg, IDC_WEB_SEARCH, 259, CSiteAnalyzeDlg::DocumentCompleteWebSearch, VTS_DISPATCH VTS_PVARIANT)
END_EVENTSINK_MAP()


void CSiteAnalyzeDlg::DocumentCompleteWebSearch(LPDISPATCH pDisp, VARIANT* URL)
{
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("导航触发DocumentComplete的URL:%s！"),  URL->bstrVal);
	if (_tcscmp(URL->bstrVal, _T("about:blank")) == 0)  //导航空白页不做处理
	{
		return;
	}


	bool bMayCloseWnd = false;
	IUnknown*  pUnk;  
	LPDISPATCH lpWBDisp;  
	HRESULT    hr;  

	pUnk = m_WebBrowser.GetControlUnknown();  
	ASSERT(pUnk);  

	hr = pUnk->QueryInterface(IID_IDispatch, (void**)&lpWBDisp);  
	ASSERT(SUCCEEDED(hr));  

	if (pDisp == lpWBDisp )      //判断页面是否已加载完成
	{  
		m_bIsLoadComplete = true;
		bMayCloseWnd = true;

		ExecuteSearchData();
	}  

	lpWBDisp->Release();  

	if (bMayCloseWnd)
	{
		m_strVec.clear();
		PostQuitMessage(0);
	}	
}


// bool CSiteAnalyzeDlg::ReadySearchData()
// {
// 	BOOL bBusy = FALSE;
// 	long lReadyState = 0;
// 	int  iTimeleft = 0;
// 
// 	do 
// 	{
// 		bBusy = m_WebBrowser.GetBusy();
// 		lReadyState = m_WebBrowser.GetReadyState();	
// 
// 		if ( !bBusy && lReadyState >= READYSTATE_COMPLETE )
// 		{
// 			return true;
// 		}
// 
// 		Sleep(500);
// 		if (iTimeleft++ > TIMEOUTCOUTS)//超过TIMEOUTCOUTS*200毫秒，默认失败
// 		{
// 			break;
// 		}
// 	} while (TRUE);	
// 
// 
// 	return false;
// }
