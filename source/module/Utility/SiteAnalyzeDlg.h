#pragma once
#include "webbrowser2.h"
#include "Resource.h"
#include <Mshtml.h>

// CSiteAnalyzeDlg 对话框

class CSiteAnalyze;

class CSiteAnalyzeDlg : public CDialog
{
	DECLARE_DYNAMIC(CSiteAnalyzeDlg)

public:
	CSiteAnalyzeDlg(CWnd* pParent = NULL);   // 标准构造函数

	/*
	@brief 重载构造函数
	@param url       用户搜索的url
	@param pSearch   对象CSiteAnalyzeThread的指针
	@return Null
	*/
	CSiteAnalyzeDlg(CString url, CSiteAnalyze* pSearch, CWnd* pParent = NULL);
	virtual ~CSiteAnalyzeDlg();

public:
	//在搜索之前确定页面打开
	//bool ReadySearchData(void);   


	//开始搜索数据
	DWORD StartSearch(void); 


	//得到搜索的数据
	CString GetSearchData(void); 


	//执行搜索操作
	DWORD ExecuteSearchData(void);  


	//判断用户输入的url是否有效
	BOOL JudegeIsValidUrl(CString strPageHtml);


	//获取网页的html
	CString GetHtmlData(IHTMLElementCollection *pElementColl);    


	//解析数据
	CString PaserData(CString strPageHtml,const CString &strAllData);   


	//提取出所需数据
	void FetchData(CString strSourceCode, CString strBefore, CString strLast, 
						const CString &strFlagName, CString id = _T(""),CString strFindFlag=_T("</span>"));


// 对话框数据
	enum { IDD = IDD_WEB_SEARCH };

protected:
	HANDLE hThread;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

private:
	CString m_strUrl;    //用户搜索的url
	CString m_strDomain;   //搜索后获得的域名
	vector<CString> m_strVec;    //保存搜索的结果
	CWebBrowser2 m_WebBrowser;
	bool m_bIsLoadComplete;    //是否加载完成
	DWORD m_dwTimeOutCounts;   //超时次数
	DWORD m_dwRetryCounts;     //重试次数

	//用来调用CSiteAnalyze对象的方法给ui返回数据
	CSiteAnalyze *m_pWebSearch;   

	

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_EVENTSINK_MAP()
	void DocumentCompleteWebSearch(LPDISPATCH pDisp, VARIANT* URL);
};
