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

// CWebDlg �Ի���

class CWebDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWebDlg)

public:
	CWebDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CWebDlg();

// �Ի�������
	enum { IDD = IDD_WEB_DLG };

	BOOL m_bBusy;
	long m_lHtmlStatus;
	CStringA m_strSingleUrl;       // ����ҳ����Ŀ����ַpjEdit(eg: .baidu.)
	int m_iPageOpenState;			//��ҳ��״̬
	CString m_strErrorWebId;			//����򲻿�����վid
	int m_iTimerCount;
protected:
	HANDLE hThread;
	CWebBrowser2 m_WebBrowser;
	CIntelligenceFind m_IntelligFind;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
	// ʵ��
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
	@breif ����IEģʽ
	*/
	void SetIEMode(E_IeLimit eVer);
	/*
	@brief ��Win8����ϵͳ������Ȩ
	*/
	BOOL RaisePrivileges();

	/*
	@brief ����IE����ģʽ
	@param eVer IE�汾
	*/
	void SetIECoreVersion(E_IeLimit eVer);

	/*
	@breif �ж��Ƿ�64λϵͳ
	*/
	BOOL IsWOW64();

	/*
	@breif ��ȡIE�汾��
	*/
	int GetIEVersion();

	/*
	Add By ����
	@brief  �Ƴ�Ҫ����HTML�������script��ǩ
	@param  string strHTMLData HTML����
	@return void
	*/
	void RemoveAllScriptTag(string &strHTMLData);

	/*
	Add By pj
	@brief  �Ƴ�������
	@param  string strHTMLData HTML����
	@return void
	*/
	void RemoveSpecialTag(string &strHTMLData, int iFlag);

	/*
	Add By ����
	@brief  ��HTML��style��ǩ�����滻��css����
	@param  string strHTMLData HTML����
	@param  char* strData Ҫ�滻������
	@return void
	*/
	void ReplaceStyleToCss(string &strHTMLData, const char* strData);

	/*
	Add By ����
	@brief  �ж�HTML�Ƿ���Ҫ����
	@param  int iFlag ������־
	@return bool
	*/
	bool IsNeedChange(int iFlag);

	/*
	Add By ����
	@brief  ����������־��ȡstyle��ǩ��Ҫ�滻������
	@param  int iFlag ������־
	@param  string& szCss �滻���ݡ������
	@return void
	*/
	void ChangeData(int iFlag, string& szCss, int index);

public:
	/*
	@brief  ȡ�ðٶ���Ŀ�߶ȣ�����������
	@param  ���˱�ǩ����
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return ���ط��������ĸ߶�
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

	//�жϸùؼ��ʵ��Ѷ����Ƿ��й��
	int IsADWord(int _SearchEngineId, int _index, IHTMLElementCollection *_pAllColls);
	/*
	@brief  ȡ���ֻ��ٶ���Ŀ�߶ȣ�����������
	@param  ���˱�ǩ����
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return ���ط��������ĸ߶�
	add by zhoulin
	*/
	int GetPhoneBaiduPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	@brief  ȡ���ֻ��ѹ���Ŀ�߶ȣ�����������
	@param  ���˱�ǩ����
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return ���ط��������ĸ߶�
	add by zhoulin
	*/
	int GetPhoneSougouPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	@breif ȡ���ֻ�360��������
	@param  ���˱�ǩ����
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return ִ�н��
	*/
	int GetPhone360Photo(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	@brief  ȡ�ùȸ���Ŀ�߶ȣ�����������
	@param  ���˱�ǩ����
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return ���ط��������ĸ߶�
	*/
	std::vector<long> CalGoogleHeight(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);

	/*
	@brief  ȡ��il��ǩ��Ŀ�߶ȣ�����������
	@param  ���˱�ǩ����
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return	0��������	1��������	2������ҳ����
	*/
	int GetILStylePhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	/ *
	@brief  ץȡ�е���������
	* /
	int GetYouDaoPhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);*/

	/*
	/ *
	@brief  360�İ�󷵻ص�URL����360����ҳ�淵�ص�URL�����ҵ�������URL
	@param  strURL  360����ҳ��õ���URL
	@return ����������URL(URL�����)
	* /
	CString GetRealURL(CString strURL);*/


	/*
	@brief  ȡ��������Ŀ�߶ȣ�����������
	@param  ���˱�ǩ����
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return ���ط��������ĸ߶�
	*/
	/*std::vector<long> CalSoSoHeight(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);*/

	/*
	@brief  ȡ���ѹ���Ŀ�߶ȣ�����������
	@param  ���˱�ǩ����
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return ���ط��������ĸ߶�
	*/
	//std::vector<long> CalSoGouHeight(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);
	int GetSoGouPhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	@breif ȡ��΢��ץȡ���
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return ִ�н��
	*/
	int GetWeixinPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp);

	/*
	@brief  ȡ���ֻ�������Ŀ�߶ȣ�����������
	@param  ���˱�ǩ����
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return ���ط��������ĸ߶�
	add by pengju
	*/
	int GetPhoneShenMaPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *pAllColls);

	/*
	/ *
	@brief  ȡ�ñ�Ӧ��Ŀ�߶ȣ�����������
	@param  ���˱�ǩ����
	@param  sData  �ؼ����������
	@param  strFileName  Ҫ�����ͼƬ·��   ���
	@return ���ط��������ĸ߶�
	* /
	//std::vector<long> CalBingHeight(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);
	int GetBingPhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult);*/

	/*
	@brief  ����bmpͼƬ
	@param  hBitmap  bmpͼƬ����
	@param  lpszFileName  �ļ�·��
	@return ����ɹ�����true
	*/
	BOOL SaveToFile(HBITMAP hBitmap, LPCTSTR lpszFileName);

	/*
	@brief  ����urlȡ��
	@param  [in/out]strUrl  ��ַ
	*/
	static BOOL GetMainDomain(CString &strUrl, const std::vector<CString> &vHostlist);

	/*
	@brief  ����url �ж������htmlԴ���Ƿ������˾��
	@param  strUrl  ��ַ
	@param  strCompay  ��˾��
	@param  bOwnOfficial �Ƿ����
	@param  bAceWeb   �Ƿ�����
	@param dwWebID    �������Ƶ�ID;
	@return 0û�У� 1�У�  2�����쳣�򲻿�
	*/
	static int HasCompanyInHtml(CString &strUrl, const KeyWordDataInfo &sData, CatchInfo &sCatch, DWORD &dwWebID, LPVOID lp, CString& sSiteName, CString& _strAllRedirectUrl, CString& _strUserfulDomain, bool _bIsOfficialWebFlag, int& _iAllOpenState, CString& _strAllErrorWebId);

	/*
	@brief  ͨ��CHttpFile������ҳԴ������
	@param  strUrl  ��ַ
	@param  strWebSrc  ��ȡ����ҳԴ��
	@param  sData �ؼ�������
	@return ������ҳ�򿪵�״̬
	*/
	static int GetSrcByHttp(CString &strUrl, CStringA &strWebSrc, CString& _strContentType, const KeyWordDataInfo &sData, CString& _sRedirectUrl/*, bool _bFlag = false*/);
	static int GetSrcByLibcurl(CString& _strUrl, CStringA& _strWebSrc, CString& _strContentType, const KeyWordDataInfo& _sData, CString& _sRedirectUrl, long& _iHttpStatusCode, bool _bIsOfficialWebFlag);

	//�ȽϿ��յ�ַ����ҳ��ַ
	static bool compareInsideAndOutSiteUrl(const CString& strUserfulDomain, const CString& _sRedirectUrl, bool _bIsOfficialWebFlag);

	static void GetRedirectUrl(const KeyWordDataInfo &_sData, CString &strUrl, CString _strUserfulDomain, CString &strRedirectUrl, CString _sRedirectUrl);
	static void addRedirectUrl(CString &strRedirectUrl, CString _sRedirectUrl);

	static void addErrorWebId(CString& strAllErrorWebId, DWORD dwWebID, long _iHttpStatusCode);



	//static void changeOpenUrlState(int& _iCurrentOpenState, int _iCurrentState);
	void changeOpenUrlState(int _iCurrentState);
	void GetRedirectUrl(const KeyWordDataInfo &sData, CString& _strUrl, CString& _strRedirectUrl);

	//ͨ�����ӻ�ȡ��ʵ��ַ
	void GetAddrByUrl(CString& _strUrl, CString& _strRedirectUrl);

	/*
	@brief  ͨ��CHtmlView������ҳԴ������
	@param  strUrl  ��ַ
	@return ������ҳԴ��
	*/
	//CString GetTargetHtmlSrc(CString strUrl);

	/*
	void addBackData(BackDataInfo* _pBdata, const KeyWordDataInfo& _sData, int& _iRetValue, CString& _strFileName, int _iRankCnt, CString _strWebRecord, CString _strAllRedirectUrl);
	@brief  ����ҳ����ת��Ϊָ���ı����ʽ������
	@param  pszRawData  ���յ�����ҳԴ����
	@param  strContentType �����������͵��ַ���
	@param  strRetData ָ������������
	@return �Ƿ�����ڹ�����
	*/
	static BOOL GetPageData(CStringA strRawData, CString &strContentType, CString &strRetData, const std::vector<CStdString> &vCompanys, CString& _sRedirectUrl, CString _UserfulDomain, int& _ErrorCode, long& _iHttpStatusCode);

	//������վ����Ԥ����
	static BOOL PageDataPreproccess(CString &strRetData, CString _UserfulDomain);

	/*
	@brief  ����ҳ����ת��Ϊָ���ı����ʽ������
	@param  pszRawData  Ҫת��������
	@param  dwRawDataLen ���ݳ���
	@param  codePage ת������
	@return ָ������������
	*/
	static CString ToWideCharString(LPSTR pszRawData, DWORD codePage);
	/*
	@brief  CStringתCStringA
	@param  cstrSrcW  Ҫת��������
	@return ת���������
	*/
	static CStringA CStrW2CStrA(const CStringW &cstrSrcW);
	/*
	@brief  CStringAתCString
	@param  cstrSrcA  Ҫת��������
	@return ת���������
	*/
	static CStringW CStrA2CStrW(const CStringA &cstrSrcA);

	/*
	@brief  ���ݰٶ���ĿhtmlԴ��ȡ�ö�Ӧ����url
	@param  strPage  Դ��
	@param  Index	 ��������
	*/
	static CString GetUrlFormContent(const CString &strPage, int Index);

	/*
	@brief  ���ݹؼ���ȡ��ͼƬ·��
	@param  strTitle  �ؼ���
	@param  strFile  �ļ�·��  ���
	@param  strComp  ��˾��
	@param  iIndex  ��������
	@return ����ɹ�����true
	*/
	static BOOL GetImageFilePath(const CString &strTitle, const CString &strComp, CString &strFile, int iIndex);

	/*
	@brief  ����ҳ�淢����������Ѱ�Ұٶȹȸ���������   �߳�
	@param  lp  thisָ��
	@return
	*/
	static DWORD WINAPI GetUpdateRank(LPVOID lp);

	/*
	@brief  �ȴ�web�ؼ���ҳ��
	@param  sData
	@return
	*/
	BOOL WaitURlAndSaveImageFile(const KeyWordDataInfo &sData, LPVOID lp);

	/*
	@brief  �ȴ�web�ؼ���ҳ��
	@param  sData
	@return
	*/
	BOOL WaitURlAndSaveImageFileNew(const KeyWordDataInfo &sData, LPVOID lp/*, BOOL _BNextPage, CString& _NextPageUrl*/);

	/*
	@brief  �жϰٶ���������ַ�Ƿ���   ��¼����վ
	@param  strHtml  ĳ����Ŀ��html����
	@param  sData    �����ṹ
	@return TRUE�����
	*/
	BOOL  FindIsOwnWebInBaidu(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficial, DWORD& dwWebId, CString& sSiteName);

	/*
	@brief  �ж��ֻ��ٶ���������ַ�Ƿ���   ��¼����վ
	@param  strHtml  ĳ����Ŀ��html����
	@return TRUE�����
	add by zhoulin
	*/
	BOOL  FindIsOwnWebInPhoneBaidu(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName);

	/*
	@brief  �ж��ֻ��ѹ���������ַ�Ƿ���   ��¼����վ
	@param  strHtml  ĳ����Ŀ��html����
	@return TRUE�����
	add by zhoulin
	*/
	BOOL  FindIsOwnWebInPhoneSougou(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName);


	/*
	@brief  �жϹȸ���������ַ�Ƿ���   ��¼����վ
	@param  strHtml  ĳ����Ŀ��html����
	@return TRUE�����
	*/
	BOOL  FindIsOwnWebInGoogle(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName);

	/*
	@brief  �ж�360��������ַ�Ƿ���   ��¼����վ
	@param  strHtml  ĳ����Ŀ��html����
	@return TRUE�����
	*/
	BOOL  FindIsOwnWebIn360(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName);

	/*
	@brief  �ж�΢����������ַ�Ƿ���   ��¼����վ
	@param  strHtml  ĳ����Ŀ��html����
	@return TRUE�����
	*/
	BOOL  FindIsOwnWebInWeixin(CString strHtml, const KeyWordDataInfo &sData, CString& sSiteName);


	/*
	@brief  �ж��ѹ�����ַ�Ƿ���   ��¼����վ
	@param  strHtml  ĳ����Ŀ��html����
	@return TRUE�����
	*/
	BOOL  FindIsOwnWebInSoGou(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName);


	/*
	@brief  ����ץ����ʱ�쳣�������ҳû�򿪻��������֤�룩
	*/
	void exceptionHandling(const KeyWordDataInfo &sData);


	/*
	@brief  ��ץȡ�����ݷ��ظ��ϲ�
	*/
	void BackMsg();


	/*
	@brief ����������־�õ�������������
	@param iFlag  ������־
	@return ������������
	*/
	CString GetSearchFlag(int iFlag);


	/*
	@brief ��html���浽һ���ļ�
	@param pHtml  html����
	@param strFileName  html�ļ�·��
	@param iFlag ���������־
	Add By ����
	*/
	// void SaveToHtml(IHTMLElementCollection *pHtml, const CString &strFileName);
	void SaveToHtml(IHTMLElementCollection *pHtml, const CString &strFileName, const KeyWordDataInfo *pSearchData);

	// �е����⴦��
	void SaveToHtmlYouDao(IHTMLElementCollection *pHtml, const CString &strFileName, const KeyWordDataInfo *pSearchData);

	void ChangeButtonStyle(IHTMLElementCollection *pHtml, const KeyWordDataInfo *pSearchData);      //�ı�������ťҳ����ת
	void ChangeSpecialButtonStyle(CString& strData, const KeyWordDataInfo *pSearchData); //�޸���ҳ������ť������ID����ʽ
	void SetInternetFeature(void);          //����web�ؼ���ҳ��ת����


	/*
	@brief ��ǰҳ��ûץȡ������ʱ���жϸ�ҳ�Ƿ��޷����ǳ�����֤������
	@param pHtml  html����
	@param pSearchData   �����ؼ�����Ϣ
	@return 0��ʾ����ҳ��  1��ʾ�޷���ҳ�棬ҳ��Ϊ�ջ����޷���ʾ��ҳ��  2��ʾ������֤�����
	*/
	int  JudgeHtml(IHTMLElementCollection *pHtml, const KeyWordDataInfo *pSearchData);

	/*
	@brief ��ȡ��ǰurl��Դ�ļ���
	@param url ��ַ
	@return Դ�ı�
	*/
	static CStringA urlopen(CString url, int& _ErrorCode, long& _iHttpStatusCode);

	/*
	@brief ��ȡǶ����ַ��Դ�ļ��������Ƿ�����ڹ�����
	@param strData c����������
	@return TRUE �����ڹ�����
	*/
	static BOOL GetEmbedUrlData(CStringA& strData, CString& _sRedirectUrl, bool& _boolValue, int& _ErrorCode, long& _iHttpStatusCode);

	/*
	@brief �ж��������Ƿ������˾�����
	@param vComp ���й�˾����ǣ� strData Ҫ���ҵ��ַ���,iIndex ��ǰ��������������
	@return iResult ���
	*/
	static int CheckWebFlag(const std::vector<CString>& vComp, const CString& strData, int iIndexconst, const std::vector<CStdString>& vCompName, const bool& _bOnlyRareWord);

	/*
	@brief ��⵱ǰ��վǶ����ַ���Ƿ������˾���
	@param sData �ؼ�����Ϣ��
	@param strData ��վ���ݣ�
	@return
	*/
	static int InspectEmbedWeb(CString &strData, CString& _strContentType, const KeyWordDataInfo &sData, int &iResult, CString& _sRedirectUrl, CString _userfulDomain, int& _ErrorCode, long& _iHttpStatusCode);

	/*
	@breif �����Ƿ�����ó��վ������·��վ��ַ��
	@param strWebList  ��վ�б�
	@param WebId ���ҵ�����վID;
	@param pChar Ҫ���ҵ��ַ�����
	@return True ���ҳɹ���
	*/
	static int GetWebId(const KeyWordDataInfo &sData, DWORD &dwWebId, const CStringA &pChar);

	/*
	@breif �����վ���������У�
	@param sData  �ؼ������ݣ�
	@param SigUrl ��վ��;
	@return True ���ҳɹ���
	*/
	//static void PushSingleUrl(const KeyWordDataInfo &sData, const CStringA &SigUrl);
	/*
	@breif ��ȡ�ض���վ��JS�ű���ַ��
	@param sData  �ؼ������ݣ�
	@param _cstrHtmlSrcA ��ҳԴ��;
	@param _strJsUrl ��ȡ��js�ű���ַ;
	@return True ���ҳɹ���
	*/
	static bool GetSingleUrl(const KeyWordDataInfo &sData, const CStringA &_cstrHtmlSrcA, CString& _strJsUrl, const CString &sSiteName);
	/*
	@breif �����վ�����棻
	*/
	//static void ClearSingleUrl(void);

	/*
	@brief ȥ��������ҳԴ���е�<a>��ǩ
	@param strData ��ҳԴ����
	@param strStartTag Ҫ�滻�Ŀ�ʼ��ǩ
	@param strEndTag Ҫ�滻�Ľ�����ǩ
	*/
	static int ReplaceTag(CString &strData, const CString& strStartTag, const CString& strEndTag);

	/*
	@brief �ж��Ƿ�Ϊ������վ������ID
	@param strData ��ҳԴ
	@param dwWebId ��վID
	@param dwIndex ������������
	@return TRUE �ǹ���
	*/
	static BOOL CheckAceWeb(const CString& strData, DWORD &dwWebId, int dwIndex);

	void addBackData(BackDataInfo* _pBdata, const KeyWordDataInfo& _sData, int& _iRetValue, CString& _strFileName, int _iRankCnt, CString _strWebRecord, CString _strAllRedirectUrl);

	//ָ�Ʋ��ң���Ƨ�� + ��˾����
	static bool keyFingerSearch(CString _str, sFingerRecogn _Finger, const std::vector<CStdString> &vCompanys);


	void GetAllAttributes(CComQIPtr<IHTMLElement> _pElement, std::map<_bstr_t, _bstr_t>&mapAttribs);
	bool GetTargetElement(CComQIPtr<IHTMLElementCollection> _pAllColls, TagNode	 _TagNode, CComQIPtr<IHTMLElement>& _pIHTMLElement);
	//�ж�ҳ������Ƿ����
	bool bHtmlLoadComplete();

	bool WaitWithUIMsgDisp(int nWaitTime);

	static DWORD WINAPI CheckHtmlLoadCompleteThread(LPVOID lp);
	bool CheckHtmlLoadComplete(LPVOID lp);
	bool CheckHtmlLoadComplete(bool _bFlag, KeyWordDataInfo* _pSdata, CComQIPtr<IHTMLElementCollection> _pAllColls);
};
