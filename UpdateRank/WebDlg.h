#pragma once
#include "CWebBrowser2.h"
#include "Resource.h"
#include <mshtml.h>   
#include <atlbase.h>   
#include <oleacc.h> 
#include <string>
#include <afxhtml.h>                
#include <afxmt.h>   
#import <mshtml.tlb>
#include "IntelligenceFind.h"
using namespace std;

// CWebDlg 对话框

class CWebDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWebDlg)

public:
	CWebDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWebDlg();

// 对话框数据
	enum { IDD = IDD_WEB_DLG };

	BOOL m_bBusy;
	long m_lHtmlStatus;
	CStringA m_strSingleUrl;       // 引擎页面条目的网址pjEdit(eg: .baidu.)
	int m_iPageOpenState;			//网页打开状态
	CString m_strErrorWebId;			//保存打不开的网站id
	int m_iTimerCount;
protected:
	HANDLE hThread;
	CWebBrowser2 m_WebBrowser;
	CIntelligenceFind m_IntelligFind;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	// 实现
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnBeforeNavigate2Explorer(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedOk();
	afx_msg LRESULT  GetSearchData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT  GetSearchDataNew(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT  RedirectURL(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT  RedirectURLNew(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT  AnalysisHtmlData(WPARAM wParam, LPARAM lParam);

private:
	std::vector<pBackDataInfo> m_vBackData;
private:
	void initPageOpenState(int _iPageState);
	int getPageOpenState(void);

	void initErrorWebId();
	CString getErrorWebId(void);
	void setErrorWebId(CString _sErrorWebId);

	/*
	@breif 设置IE模式
	*/
	void SetIEMode(E_IeLimit eVer);
	/*
	@brief 对Win8以下系统进行提权
	*/
	BOOL RaisePrivileges();

	/*
	@brief 设置IE运行模式
	@param eVer IE版本
	*/
	void SetIECoreVersion(E_IeLimit eVer);

	/*
	@breif 判断是否64位系统
	*/
	BOOL IsWOW64();

	/*
	@breif 获取IE版本；
	*/
	int GetIEVersion();

	/*
	Add By 周林
	@brief  移除要保存HTML里的所有script标签
	@param  string strHTMLData HTML内容
	@return void
	*/
	void RemoveAllScriptTag(string &strHTMLData);

	/*
	Add By pj
	@brief  移除特殊标记
	@param  string strHTMLData HTML内容
	@return void
	*/
	void RemoveSpecialTag(string &strHTMLData, int iFlag);

	/*
	Add By 周林
	@brief  把HTML里style标签内容替换成css引用
	@param  string strHTMLData HTML内容
	@param  char* strData 要替换的内容
	@return void
	*/
	void ReplaceStyleToCss(string &strHTMLData, const char* strData);

	/*
	Add By 周林
	@brief  判断HTML是否需要更改
	@param  int iFlag 搜索标志
	@return bool
	*/
	bool IsNeedChange(int iFlag);

	/*
	Add By 周林
	@brief  根据搜索标志获取style标签需要替换的内容
	@param  int iFlag 搜索标志
	@param  string& szCss 替换内容【输出】
	@return void
	*/
	void ChangeData(int iFlag, string& szCss, int index);

public:
	/*
	@brief  取得百度条目高度，及反馈数据
	@param  过滤标签内容
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return 返回符合条件的高度
	*/
	int  GetBaiduPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	bool getSnapUrlAndSnapShotUrl(const CComQIPtr<IHTMLElement>& _htmlElement, const KeyWordDataInfo &sData, CString& _strSnapUrl, CString& _strSnapShotUrl);
	bool getSnapShotUrlAssist(const CString _strPage, const KeyWordDataInfo &_sData, CString& _strSnapShotUrlAssist);
	bool getUrlPath(const CComQIPtr<IHTMLElement>& _htmlElement, vector<locationStruct>& _vstrUrlFlag, targetAddressSign& _targetAddressSign, CString& _strOutUrl);
	bool getSingleItem(CComQIPtr<IHTMLElementCollection>& _pTags, long lindex, CComQIPtr<IHTMLElement>& _pElement);
	bool LinkUrlIsInOurSitelist(const CString _strUrl, const CString _strUrlLink, const KeyWordDataInfo &sData, BOOL& bOwnOfficial, DWORD& dwWebId, CString& _strUserfulDomain, CString& _strPage, bool& _bIsOfficialWebFlag);
	bool UrlIsInOfficialList(const CString _strUrlLink, const vector<CString> &vHostlist);
	bool cutUrl(const CString _strUrlLink, CString& _strMainUrl, CString& _strAssistUrl);
	bool cutDomain(const CString _strDomain, CString& _strUsefulDomain);
	bool SplitCString(const CString & input, const CString & delimiter, std::vector<CString >& results, bool includeEmpties);
	int DomainCompare(const CString & _sCurDomain, const CString & _sCurUrlPath, const KeyWordDataInfo &sData, DWORD& dwWebId, bool& _bIsOfficialWebFlag, bool _bFlag);
	bool addPageTags(CComQIPtr<IHTMLElement>& _pElement, CatchInfo& _cInfo, DWORD& _dwWebId, const KeyWordDataInfo& _sData, BOOL& _bIsAce);

	//判断该关键词的搜多结果是否含有广告
	int IsADWord(int _SearchEngineId, int _index, IHTMLElementCollection *_pAllColls);
	/*
	@brief  取得手机百度条目高度，及反馈数据
	@param  过滤标签内容
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return 返回符合条件的高度
	add by zhoulin
	*/
	int GetPhoneBaiduPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	@brief  取得手机搜狗条目高度，及反馈数据
	@param  过滤标签内容
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return 返回符合条件的高度
	add by zhoulin
	*/
	int GetPhoneSougouPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	@breif 取得手机360反馈数据
	@param  过滤标签内容
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return 执行结果
	*/
	int GetPhone360Photo(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	@brief  取得谷歌条目高度，及反馈数据
	@param  过滤标签内容
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return 返回符合条件的高度
	*/
	std::vector<long> CalGoogleHeight(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);

	/*
	@brief  取得il标签条目高度，及反馈数据
	@param  过滤标签内容
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return	0：有排名	1：无排名	2：打开网页错误
	*/
	int GetILStylePhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	/ *
	@brief  抓取有道搜索快照
	* /
	int GetYouDaoPhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);*/

	/*
	/ *
	@brief  360改版后返回的URL不对360搜索页面返回的URL处理找到真正的URL
	@param  strURL  360搜索页面得到的URL
	@return 返回真正的URL(URL解码后)
	* /
	CString GetRealURL(CString strURL);*/


	/*
	@brief  取得搜搜条目高度，及反馈数据
	@param  过滤标签内容
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return 返回符合条件的高度
	*/
	/*std::vector<long> CalSoSoHeight(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);*/

	/*
	@brief  取得搜狗条目高度，及反馈数据
	@param  过滤标签内容
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return 返回符合条件的高度
	*/
	//std::vector<long> CalSoGouHeight(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);
	int GetSoGouPhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	@breif 取得微信抓取结果
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return 执行结果
	*/
	int GetWeixinPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp);

	/*
	@brief  取得手机神马条目高度，及反馈数据
	@param  过滤标签内容
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return 返回符合条件的高度
	add by pengju
	*/
	int GetPhoneShenMaPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	/ *
	@brief  取得必应条目高度，及反馈数据
	@param  过滤标签内容
	@param  sData  关键词相关数据
	@param  strFileName  要保存的图片路径   输出
	@return 返回符合条件的高度
	* /
	//std::vector<long> CalBingHeight(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);
	int GetBingPhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);*/

	/*
	@brief  保存bmp图片
	@param  hBitmap  bmp图片数据
	@param  lpszFileName  文件路径
	@return 保存成功返回true
	*/
	BOOL SaveToFile(HBITMAP hBitmap, LPCTSTR lpszFileName);

	/*
	@brief  根据url取得
	@param  [in/out]strUrl  网址
	*/
	static BOOL GetMainDomain(CString &strUrl, const std::vector<CString> &vHostlist);

	/*
	@brief  请求url 判断里面的html源码是否包含公司名
	@param  strUrl  网址
	@param  strCompay  公司名
	@param  bOwnOfficial 是否官网
	@param  bAceWeb   是否王牌
	@param dwWebID    返回王牌的ID;
	@return 0没有， 1有，  2出现异常打不开
	*/
	static int HasCompanyInHtml(CString &strUrl, const KeyWordDataInfo &sData, CatchInfo &sCatch, DWORD &dwWebID, LPVOID lp, CString& sSiteName, CString& _strAllRedirectUrl, CString& _strUserfulDomain, bool _bIsOfficialWebFlag, int& _iAllOpenState, CString& _strAllErrorWebId);

	/*
	@brief  通过CHttpFile请求网页源码数据
	@param  strUrl  网址
	@param  strWebSrc  获取的网页源码
	@param  sData 关键词数据
	@return 返回网页打开的状态
	*/
	static int GetSrcByHttp(CString &strUrl, CStringA &strWebSrc, CString& _strContentType, const KeyWordDataInfo &sData, CString& _sRedirectUrl/*, bool _bFlag = false*/);
	static int GetSrcByLibcurl(CString& _strUrl, CStringA& _strWebSrc, CString& _strContentType, const KeyWordDataInfo& _sData, CString& _sRedirectUrl, long& _iHttpStatusCode, bool _bIsOfficialWebFlag);

	//比较快照地址与内页网址
	static bool compareInsideAndOutSiteUrl(const CString& strUserfulDomain, const CString& _sRedirectUrl, bool _bIsOfficialWebFlag);

	static void GetRedirectUrl(const KeyWordDataInfo &_sData, CString &strUrl, CString _strUserfulDomain, CString &strRedirectUrl, CString _sRedirectUrl);
	static void addRedirectUrl(CString &strRedirectUrl, CString _sRedirectUrl);

	static void addErrorWebId(CString& strAllErrorWebId, DWORD dwWebID, long _iHttpStatusCode);



	//static void changeOpenUrlState(int& _iCurrentOpenState, int _iCurrentState);
	void changeOpenUrlState(int _iCurrentState);
	void GetRedirectUrl(const KeyWordDataInfo &sData, CString& _strUrl, CString& _strRedirectUrl);

	//通过链接获取真实网址
	void GetAddrByUrl(CString& _strUrl, CString& _strRedirectUrl);

	/*
	@brief  通过CHtmlView请求网页源码数据
	@param  strUrl  网址
	@return 返回网页源码
	*/
	//CString GetTargetHtmlSrc(CString strUrl);

	/*
	void addBackData(BackDataInfo* _pBdata, const KeyWordDataInfo& _sData, int& _iRetValue, CString& _strFileName, int _iRankCnt, CString _strWebRecord, CString _strAllRedirectUrl);
	@brief  将网页数据转化为指定的编码格式并返回
	@param  pszRawData  接收到的网页源数据
	@param  strContentType 包含编码类型的字符串
	@param  strRetData 指定编码后的数据
	@return 是否包含在官网中
	*/
	static BOOL GetPageData(CStringA strRawData, CString &strContentType, CString &strRetData, const std::vector<CStdString> &vCompanys, CString& _sRedirectUrl, CString _UserfulDomain, int& _ErrorCode, long& _iHttpStatusCode);

	//特殊网站数据预处理
	static BOOL PageDataPreproccess(CString &strRetData, CString _UserfulDomain);

	/*
	@brief  将网页数据转化为指定的编码格式并返回
	@param  pszRawData  要转换的数据
	@param  dwRawDataLen 数据长度
	@param  codePage 转换代码
	@return 指定编码后的数据
	*/
	static CString ToWideCharString(LPSTR pszRawData, DWORD codePage);
	/*
	@brief  CString转CStringA
	@param  cstrSrcW  要转换的数据
	@return 转换后的数据
	*/
	static CStringA CStrW2CStrA(const CStringW &cstrSrcW);
	/*
	@brief  CStringA转CString
	@param  cstrSrcA  要转换的数据
	@return 转换后的数据
	*/
	static CStringW CStrA2CStrW(const CStringA &cstrSrcA);

	/*
	@brief  根据百度条目html源码取得对应链接url
	@param  strPage  源码
	@param  Index	 引擎索引
	*/
	static CString GetUrlFormContent(const CString &strPage, int Index);

	/*
	@brief  根据关键词取得图片路径
	@param  strTitle  关键词
	@param  strFile  文件路径  输出
	@param  strComp  公司名
	@param  iIndex  搜索索引
	@return 保存成功返回true
	*/
	static BOOL GetImageFilePath(const CString &strTitle, const CString &strComp, CString &strFile, int iIndex);

	/*
	@brief  根据页面发过来的数据寻找百度谷歌搜索数据   线程
	@param  lp  this指针
	@return
	*/
	static DWORD WINAPI GetUpdateRank(LPVOID lp);

	/*
	@brief  等待web控件打开页面
	@param  sData
	@return
	*/
	BOOL WaitURlAndSaveImageFile(const KeyWordDataInfo &sData, LPVOID lp);

	/*
	@brief  等待web控件打开页面
	@param  sData
	@return
	*/
	BOOL WaitURlAndSaveImageFileNew(const KeyWordDataInfo &sData, LPVOID lp/*, BOOL _BNextPage, CString& _NextPageUrl*/);

	/*
	@brief  判断百度搜索的网址是否在   收录的网站
	@param  strHtml  某个条目的html代码
	@param  sData    搜索结构
	@return TRUE表存在
	*/
	BOOL  FindIsOwnWebInBaidu(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficial, DWORD& dwWebId, CString& sSiteName);

	/*
	@brief  判断手机百度搜索的网址是否在   收录的网站
	@param  strHtml  某个条目的html代码
	@return TRUE表存在
	add by zhoulin
	*/
	BOOL  FindIsOwnWebInPhoneBaidu(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName);

	/*
	@brief  判断手机搜狗搜索的网址是否在   收录的网站
	@param  strHtml  某个条目的html代码
	@return TRUE表存在
	add by zhoulin
	*/
	BOOL  FindIsOwnWebInPhoneSougou(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName);


	/*
	@brief  判断谷歌搜索的网址是否在   收录的网站
	@param  strHtml  某个条目的html代码
	@return TRUE表存在
	*/
	BOOL  FindIsOwnWebInGoogle(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName);

	/*
	@brief  判断360搜索的网址是否在   收录的网站
	@param  strHtml  某个条目的html代码
	@return TRUE表存在
	*/
	BOOL  FindIsOwnWebIn360(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName);

	/*
	@brief  判断微信搜索的网址是否在   收录的网站
	@param  strHtml  某个条目的html代码
	@return TRUE表存在
	*/
	BOOL  FindIsOwnWebInWeixin(CString strHtml, const KeyWordDataInfo &sData, CString& sSiteName);


	/*
	@brief  判断搜狗的网址是否在   收录的网站
	@param  strHtml  某个条目的html代码
	@return TRUE表存在
	*/
	BOOL  FindIsOwnWebInSoGou(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName);


	/*
	@brief  处理抓排名时异常情况（网页没打开或打开遇到验证码）
	*/
	void exceptionHandling(const KeyWordDataInfo &sData);


	/*
	@brief  把抓取的数据返回给上层
	*/
	void BackMsg();


	/*
	@brief 根据搜索标志得到搜索引擎名称
	@param iFlag  搜索标志
	@return 搜索引擎名称
	*/
	CString GetSearchFlag(int iFlag);


	/*
	@brief 将html保存到一个文件
	@param pHtml  html对象
	@param strFileName  html文件路径
	@param iFlag 搜索引擎标志
	Add By 周林
	*/
	// void SaveToHtml(IHTMLElementCollection *pHtml, const CString &strFileName);
	void SaveToHtml(IHTMLElementCollection *pHtml, const CString &strFileName, const KeyWordDataInfo *pSearchData);

	// 有道特殊处理
	void SaveToHtmlYouDao(IHTMLElementCollection *pHtml, const CString &strFileName, const KeyWordDataInfo *pSearchData);

	void ChangeButtonStyle(IHTMLElementCollection *pHtml, const KeyWordDataInfo *pSearchData);      //改变搜索按钮页面跳转
	void ChangeSpecialButtonStyle(CString& strData, const KeyWordDataInfo *pSearchData); //修改网页搜索按钮不包含ID的样式
	void SetInternetFeature(void);          //屏蔽web控件网页跳转声音


	/*
	@brief 当前页面没抓取到快照时，判断该页是否无法打还是出现验证码等情况
	@param pHtml  html对象
	@param pSearchData   搜索关键词信息
	@return 0表示正常页面  1表示无法打开页面，页面为空或者无法显示此页，  2表示出现验证码情况
	*/
	int  JudgeHtml(IHTMLElementCollection *pHtml, const KeyWordDataInfo *pSearchData);

	/*
	@brief 获取当前url的源文件；
	@param url 地址
	@return 源文本
	*/
	static CStringA urlopen(CString url, int& _ErrorCode, long& _iHttpStatusCode);

	/*
	@brief 获取嵌入网址的源文件，返回是否包含在官网中
	@param strData c待检测的数据
	@return TRUE 包含在官网中
	*/
	static BOOL GetEmbedUrlData(CStringA& strData, CString& _sRedirectUrl, bool& _boolValue, int& _ErrorCode, long& _iHttpStatusCode);

	/*
	@brief 判断数据中是否包含公司名标记
	@param vComp 所有公司名标记， strData 要查找的字符串,iIndex 当前搜索引擎索引；
	@return iResult 结果
	*/
	static int CheckWebFlag(const std::vector<CString>& vComp, const CString& strData, int iIndexconst, const std::vector<CStdString>& vCompName, const bool& _bOnlyRareWord);

	/*
	@brief 检测当前网站嵌套网址内是否包含公司标记
	@param sData 关键词信息；
	@param strData 网站数据；
	@return
	*/
	static int InspectEmbedWeb(CString &strData, CString& _strContentType, const KeyWordDataInfo &sData, int &iResult, CString& _sRedirectUrl, CString _userfulDomain, int& _ErrorCode, long& _iHttpStatusCode);

	/*
	@breif 查找是否是商贸网站，并近路网站网址；
	@param strWebList  网站列表；
	@param WebId 查找到的网站ID;
	@param pChar 要查找的字符串；
	@return True 查找成功；
	*/
	static int GetWebId(const KeyWordDataInfo &sData, DWORD &dwWebId, const CStringA &pChar);

	/*
	@breif 添加网站名到缓存中；
	@param sData  关键词数据；
	@param SigUrl 网站名;
	@return True 查找成功；
	*/
	//static void PushSingleUrl(const KeyWordDataInfo &sData, const CStringA &SigUrl);
	/*
	@breif 获取特定网站的JS脚本网址；
	@param sData  关键词数据；
	@param _cstrHtmlSrcA 网页源码;
	@param _strJsUrl 获取的js脚本网址;
	@return True 查找成功；
	*/
	static bool GetSingleUrl(const KeyWordDataInfo &sData, const CStringA &_cstrHtmlSrcA, CString& _strJsUrl, const CString &sSiteName);
	/*
	@breif 清空网站名缓存；
	*/
	//static void ClearSingleUrl(void);

	/*
	@brief 去除整个网页源码中的<a>标签
	@param strData 网页源数据
	@param strStartTag 要替换的开始标签
	@param strEndTag 要替换的结束标签
	*/
	static int ReplaceTag(CString &strData, const CString& strStartTag, const CString& strEndTag);

	/*
	@brief 判断是否为王牌网站并返回ID
	@param strData 网页源
	@param dwWebId 网站ID
	@param dwIndex 搜索引擎索引
	@return TRUE 是官网
	*/
	static BOOL CheckAceWeb(const CString& strData, DWORD &dwWebId, int dwIndex);

	void addBackData(BackDataInfo* _pBdata, const KeyWordDataInfo& _sData, int& _iRetValue, CString& _strFileName, int _iRankCnt, CString _strWebRecord, CString _strAllRedirectUrl);

	//指纹查找（生僻字 + 公司名）
	static bool keyFingerSearch(CString _str, sFingerRecogn _Finger, const std::vector<CStdString> &vCompanys);


	void GetAllAttributes(CComQIPtr<IHTMLElement> _pElement, std::map<_bstr_t, _bstr_t>&mapAttribs);
	bool GetTargetElement(CComQIPtr<IHTMLElementCollection> _pAllColls, TagNode	 _TagNode, CComQIPtr<IHTMLElement>& _pIHTMLElement);
	//判断页面加载是否完成
	bool bHtmlLoadComplete();

	bool WaitWithUIMsgDisp(int nWaitTime);

	static DWORD WINAPI CheckHtmlLoadCompleteThread(LPVOID lp);
	bool CheckHtmlLoadComplete(LPVOID lp);
	bool CheckHtmlLoadComplete(bool _bFlag, KeyWordDataInfo* _pSdata, CComQIPtr<IHTMLElementCollection> _pAllColls);
};
