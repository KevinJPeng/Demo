// WebDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UpdateRank.h"
#include "WebDlg.h"
#include "afxdialogex.h"

#include <atlimage.h>
#include <afxinet.h>
#include <atlstr.h>
#include "CommFunc.h"
// #include "Httputils.h"
// #include "GetNetTime.h"
#include "../Common/3rdparty/libcurl/include/curl/curl.h"
#pragma comment (lib, "../Common/3rdparty/libcurl/lib/libcurl.lib" )
#pragma comment (lib, "../Common/3rdparty/zlib/lib/zdll.lib" )

#ifdef _UNICODE
#define CStrT2CStrA(cstr) CStrW2CStrA((cstr))
#define CStrA2CStrT(cstr) CStrA2CStrW((cstr))
#else
#define CStrT2CStrA(cstr) (cstr)
#define CStrA2CStrT(cstr) (cstr)
#endif

#define   GETSEARCHDATA     WM_USER + 0x01
#define   GETREDIRECTURL    WM_USER + 0x02
#define   GETSEARCHDATANEW     WM_USER + 0x03
#define   GETREDIRECTURLNEW     WM_USER + 0x04
#define   ANALYSISDATA     WM_USER + 0x05

#define   TIMER_GET_HTML_STATUS    1			//定时器1 获取网页状态
typedef void (WINAPI *LPFN_PGNSI)(LPSYSTEM_INFO);

#define MAXSIZE 8192


// CWebDlg 对话框

IMPLEMENT_DYNAMIC(CWebDlg, CDialogEx)

CWebDlg::CWebDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWebDlg::IDD, pParent)
	, hThread(NULL)
{
	m_iTimerCount = 0;
	m_lHtmlStatus = -1;
}

CWebDlg::~CWebDlg()
{

	if (hThread != NULL)
		CloseHandle(hThread);

	for (int i = 0; i < m_vBackData.size(); i++)
	{
		delete m_vBackData[i];
	}

	m_vBackData.clear();
}

void CWebDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WEB_EXPLORER, m_WebBrowser);
}


BEGIN_MESSAGE_MAP(CWebDlg, CDialogEx)
	ON_WM_TIMER()
	ON_MESSAGE(GETSEARCHDATA, &GetSearchData)
	ON_MESSAGE(GETSEARCHDATANEW, &GetSearchDataNew)
	ON_MESSAGE(GETREDIRECTURL, &RedirectURL)
	ON_MESSAGE(GETREDIRECTURLNEW, &RedirectURLNew)
	ON_MESSAGE(ANALYSISDATA, &AnalysisHtmlData)
END_MESSAGE_MAP()


// CWebDlg 消息处理程序
BOOL CWebDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	/*ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW,0);//从任务栏中去掉.*/
	//防止输入法切换问题
	::SetWindowPos(m_hWnd, HWND_BOTTOM, -1, -1, -1, -1, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW);

	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.flags = WPF_RESTORETOMAXIMIZED;
	wp.showCmd = SW_HIDE;
	SetWindowPlacement(&wp);

	//设置IE版本
	SetIEMode(IE11);
	m_WebBrowser.put_Silent(TRUE);
	SetInternetFeature();
	hThread = CreateThread(NULL, NULL, GetUpdateRank, this, 0, NULL);
	if (hThread == NULL)
		OnBnClickedOk();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

/*
@brief  根据页面发过来的数据寻找百度谷歌搜索数据  线程
@param  lp  this指针
@return
*/
DWORD WINAPI CWebDlg::GetUpdateRank(LPVOID lp)
{
	CWebDlg *pPhotoDlg = (CWebDlg*)lp;

	curl_global_init(CURL_GLOBAL_ALL);

	//g_TaskRecord.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("*******主控收到客户端传过来的数据%d条********"), g_vAllKeyWords.size());
	while (true)
	{
		pKeyWordDataInfo pCurKeyWordDataInfo = GetSearKeyWordInfo();
		if (pCurKeyWordDataInfo == NULL)
		{
			break;
		}
		pPhotoDlg->initPageOpenState(E_ALLURLOPEN_STATE0);
		pPhotoDlg->initErrorWebId();
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("关键词:%s  开始抓取快照 搜索标志为：%d"), pCurKeyWordDataInfo->strKeyWordName, pCurKeyWordDataInfo->iFlag);
		DWORD dwFlagTime;

		Sleep(10);
		if ((pCurKeyWordDataInfo->iFlag == SEARCH_BAIDU) && bSearchFlag[BADIDU_INDEX])  //抓取百度
		{
			EnterCriticalSection(&critCount);
			g_iBaidu++;
			LeaveCriticalSection(&critCount);
			dwFlagTime = GetTickCount();

			//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("百度关键词"));
			if (_T("1") == g_mapElemCfg[BADIDU_INDEX].sKWSearchMethod)
			{
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl = g_mapElemCfg[BADIDU_INDEX].sHomePage;
				pCurKeyWordDataInfo->iIndex = BADIDU_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFileNew(*pCurKeyWordDataInfo, lp/*, bNextPage, sNextPageUrl*/);
			} 
			else
			{
				for (int i = 0; i < 3; i++) //搜索前三页
				{
					if (g_iDelayForKeyWord != -1)
					{
						Sleep(g_iDelayForKeyWord);
					}
					pCurKeyWordDataInfo->iCurPage = i;
					pCurKeyWordDataInfo->strUrl.Format(g_mapElemCfg[BADIDU_INDEX].strJumpUrl, pCurKeyWordDataInfo->strKeyHex.GetString(), i * 10);
					pCurKeyWordDataInfo->iIndex = BADIDU_INDEX;
					if (pPhotoDlg->WaitURlAndSaveImageFile(*pCurKeyWordDataInfo, lp))
					{
						break;    //已经抓取快照，不再抓取搜索后面快照
					}
					else if (E_ALLURLOPEN_STATE3 == pPhotoDlg->getPageOpenState() || E_ALLURLOPEN_STATE4 == pPhotoDlg->getPageOpenState())
					{
						break;
					}
				}
			}

			dwFlagTime = GetTickCount() - dwFlagTime;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取百度快照花费时间,%.2f,秒,关键词为：%s"), dwFlagTime / 1000.0, pCurKeyWordDataInfo->strKeyWordName.GetString());
		}

		Sleep(10);
		if ((pCurKeyWordDataInfo->iFlag == SEARCH_PHONEBAIDU) && bSearchFlag[PHONEBAIDU_INDEX])  //抓取手机百度
		{
			dwFlagTime = GetTickCount();
			//			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("手机百度关键词"));

			if (_T("1") == g_mapElemCfg[PHONEBAIDU_INDEX].sKWSearchMethod)
			{
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl = g_mapElemCfg[PHONEBAIDU_INDEX].sHomePage;
				pCurKeyWordDataInfo->iIndex = PHONEBAIDU_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFileNew(*pCurKeyWordDataInfo, lp/*, bNextPage, sNextPageUrl*/);
			}
			else
			{
				// 只抓首页
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl.Format(g_mapElemCfg[PHONEBAIDU_INDEX].strJumpUrl, pCurKeyWordDataInfo->strKeyHex.GetString(), 0);
				pCurKeyWordDataInfo->iIndex = PHONEBAIDU_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFile(*pCurKeyWordDataInfo, lp);
			}

			dwFlagTime = GetTickCount() - dwFlagTime;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取手机百度快照花费时间,%.2f,秒,关键词为：%s"), dwFlagTime / 1000.0, pCurKeyWordDataInfo->strKeyWordName.GetString());

		}

		Sleep(10);
		if ((pCurKeyWordDataInfo->iFlag == SEARCH_PHONESOGOU) && bSearchFlag[PHONESOGOU_INDEX])  //抓取手机搜狗
		{
			dwFlagTime = GetTickCount();

			//			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("手机搜狗关键词"));
			if (_T("1") == g_mapElemCfg[PHONESOGOU_INDEX].sKWSearchMethod)
			{
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl = g_mapElemCfg[PHONESOGOU_INDEX].sHomePage;
				pCurKeyWordDataInfo->iIndex = PHONESOGOU_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFileNew(*pCurKeyWordDataInfo, lp/*, bNextPage, sNextPageUrl*/);
			}
			else
			{
				// 只抓首页
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl.Format(g_mapElemCfg[PHONESOGOU_INDEX].strJumpUrl, pCurKeyWordDataInfo->strKeyHex.GetString(), 1);
				pCurKeyWordDataInfo->iIndex = PHONESOGOU_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFile(*pCurKeyWordDataInfo, lp);
			}

			dwFlagTime = GetTickCount() - dwFlagTime;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取手机搜狗快照花费时间,%.2f,秒,关键词为：%s"), dwFlagTime / 1000.0, pCurKeyWordDataInfo->strKeyWordName.GetString());

		}

		Sleep(10);
		if ((pCurKeyWordDataInfo->iFlag == SEARCH_PHONE360) && bSearchFlag[PHONE360_INDEX])  //抓取手机360
		{
			dwFlagTime = GetTickCount();

			//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("手机360关键词"));
			if (_T("1") == g_mapElemCfg[PHONE360_INDEX].sKWSearchMethod)
			{
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl = g_mapElemCfg[PHONE360_INDEX].sHomePage;
				pCurKeyWordDataInfo->iIndex = PHONE360_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFileNew(*pCurKeyWordDataInfo, lp/*, bNextPage, sNextPageUrl*/);
			}
			else
			{
				//只抓首页
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl.Format(g_mapElemCfg[PHONE360_INDEX].strJumpUrl, pCurKeyWordDataInfo->strKeyHex.GetString(), 0);
				pCurKeyWordDataInfo->iIndex = PHONE360_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFile(*pCurKeyWordDataInfo, lp);
			}

			dwFlagTime = GetTickCount() - dwFlagTime;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取手机360快照花费时间,%.2f,秒,关键词为：%s"), dwFlagTime / 1000.0, pCurKeyWordDataInfo->strKeyWordName.GetString());
		}

		Sleep(10);
		if ((pCurKeyWordDataInfo->iFlag == SEARCH_360) && bSearchFlag[S360_INDEX])  //抓取360
		{
			EnterCriticalSection(&critCount);
			g_i360++;
			LeaveCriticalSection(&critCount);
			dwFlagTime = GetTickCount();
			//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("360关键词"));

			if (_T("1") == g_mapElemCfg[S360_INDEX].sKWSearchMethod)
			{
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl = g_mapElemCfg[S360_INDEX].sHomePage;
				pCurKeyWordDataInfo->iIndex = S360_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFileNew(*pCurKeyWordDataInfo, lp/*, bNextPage, sNextPageUrl*/);
			}
			else
			{
				for (int i = 0; i < 3; i++) //搜索前三页
				{
					if (g_iDelayForKeyWord != -1)
					{
						Sleep(g_iDelayForKeyWord);
					}
					pCurKeyWordDataInfo->iCurPage = i;
					pCurKeyWordDataInfo->strUrl.Format(g_mapElemCfg[S360_INDEX].strJumpUrl, pCurKeyWordDataInfo->strKeyHex.GetString(), i + 1);
					pCurKeyWordDataInfo->iIndex = S360_INDEX;
					if (pPhotoDlg->WaitURlAndSaveImageFile(*pCurKeyWordDataInfo, lp))
					{
						break;    //已经抓取快照，不再抓取搜索后面快照
					}
					else if (E_ALLURLOPEN_STATE3 == pPhotoDlg->getPageOpenState() || E_ALLURLOPEN_STATE4 == pPhotoDlg->getPageOpenState())
					{
						break;
					}
				}
			}

			dwFlagTime = GetTickCount() - dwFlagTime;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取360快照花费时间,%.2f,秒,关键词为：%s"), dwFlagTime / 1000.0, pCurKeyWordDataInfo->strKeyWordName.GetString());
		}

		Sleep(10);
		if ((pCurKeyWordDataInfo->iFlag == SEARCH_SOGOU) && bSearchFlag[SOGOU_INDEX])  //抓取搜狗
		{
			EnterCriticalSection(&critCount);
			g_iSogou++;
			LeaveCriticalSection(&critCount);
			dwFlagTime = GetTickCount();

			//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("搜狗关键词"));
			if (_T("1") == g_mapElemCfg[SOGOU_INDEX].sKWSearchMethod)
			{
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl = g_mapElemCfg[SOGOU_INDEX].sHomePage;
				pCurKeyWordDataInfo->iIndex = SOGOU_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFileNew(*pCurKeyWordDataInfo, lp/*, bNextPage, sNextPageUrl*/);
			}
			else
			{
				for (int i = 0; i < 3; i++) //搜索前三页
				{
					if (g_iDelayForKeyWord != -1)
					{
						Sleep(g_iDelayForKeyWord);
					}
					pCurKeyWordDataInfo->iCurPage = i;
					pCurKeyWordDataInfo->strUrl.Format(g_mapElemCfg[SOGOU_INDEX].strJumpUrl, pCurKeyWordDataInfo->strKeyHex.GetString(), i + 1);
					pCurKeyWordDataInfo->iIndex = SOGOU_INDEX;
					if (pPhotoDlg->WaitURlAndSaveImageFile(*pCurKeyWordDataInfo, lp))
					{
						break;    //已经抓取快照，不再抓取搜索后面快照
					}
					else if (E_ALLURLOPEN_STATE3 == pPhotoDlg->getPageOpenState() || E_ALLURLOPEN_STATE4 == pPhotoDlg->getPageOpenState())
					{
						break;
					}
				}
			}

			dwFlagTime = GetTickCount() - dwFlagTime;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取sogou快照花费时间,%.2f,秒,关键词为：%s"), dwFlagTime / 1000.0, pCurKeyWordDataInfo->strKeyWordName.GetString());
		}

		Sleep(10);
		if ((pCurKeyWordDataInfo->iFlag == SEARCH_BING) && bSearchFlag[BING_INDEX])  //抓取必应
		{
			EnterCriticalSection(&critCount);
			g_iBing++;
			LeaveCriticalSection(&critCount);
			dwFlagTime = GetTickCount();
			//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("必应关键词"));
			if (_T("1") == g_mapElemCfg[BING_INDEX].sKWSearchMethod)
			{
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl = g_mapElemCfg[BING_INDEX].sHomePage;
				pCurKeyWordDataInfo->iIndex = BING_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFileNew(*pCurKeyWordDataInfo, lp/*, bNextPage, sNextPageUrl*/);
			}
			else
			{
				for (int i = 0; i < 3; i++) //搜索前三页
				{
					if (g_iDelayForKeyWord != -1)
					{
						Sleep(g_iDelayForKeyWord);
					}
					pCurKeyWordDataInfo->iCurPage = i;
					pCurKeyWordDataInfo->strUrl.Format(g_mapElemCfg[BING_INDEX].strJumpUrl, pCurKeyWordDataInfo->strKeyHex.GetString(), i * 10 + 1);
					pCurKeyWordDataInfo->iIndex = BING_INDEX;
					if (pPhotoDlg->WaitURlAndSaveImageFile(*pCurKeyWordDataInfo, lp))
					{
						break;    //已经抓取快照，不再抓取搜索后面快照
					}
					else if (E_ALLURLOPEN_STATE3 == pPhotoDlg->getPageOpenState() || E_ALLURLOPEN_STATE4 == pPhotoDlg->getPageOpenState())
					{
						break;
					}
				}

			}
			dwFlagTime = GetTickCount() - dwFlagTime;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取BING快照花费时间,%.2f,秒,关键词为：%s"), dwFlagTime / 1000.0, pCurKeyWordDataInfo->strKeyWordName.GetString());
		}

		Sleep(10);
		if ((pCurKeyWordDataInfo->iFlag == SEARCH_YOUDAO) && bSearchFlag[YOUDAO_INDEX])  //抓取有道
		{
			EnterCriticalSection(&critCount);
			g_iYouDao++;
			LeaveCriticalSection(&critCount);
			dwFlagTime = GetTickCount();

			/*			//由于有道搜索第二页都为验证码失效，古只搜索第一页	*/
			if (g_iDelayForKeyWord != -1)
			{
				Sleep(g_iDelayForKeyWord);
			}
			pCurKeyWordDataInfo->iCurPage = 0;  //i
			pCurKeyWordDataInfo->strUrl.Format(g_mapElemCfg[YOUDAO_INDEX].strJumpUrl, pCurKeyWordDataInfo->strKeyHex.GetString(), 0); //i * 10
			pCurKeyWordDataInfo->iIndex = YOUDAO_INDEX;
			pPhotoDlg->WaitURlAndSaveImageFile(*pCurKeyWordDataInfo, lp);

			dwFlagTime = GetTickCount() - dwFlagTime;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取YOUDAO快照花费时间,%.2f,秒,关键词为：%s"), dwFlagTime / 1000.0, pCurKeyWordDataInfo->strKeyWordName.GetString());
		}

		Sleep(10);
		if ((pCurKeyWordDataInfo->iFlag == SEARCH_WEIXIN) && bSearchFlag[WEIXIN_INDEX])		//抓取微信
		{
			dwFlagTime = GetTickCount();
			//只抓首页
			pCurKeyWordDataInfo->iCurPage = 0;
			pCurKeyWordDataInfo->strUrl.Format(g_mapElemCfg[WEIXIN_INDEX].strJumpUrl, pCurKeyWordDataInfo->strKeyHex.GetString());
			pCurKeyWordDataInfo->iIndex = WEIXIN_INDEX;
			pPhotoDlg->WaitURlAndSaveImageFile(*pCurKeyWordDataInfo, lp);

			dwFlagTime = GetTickCount() - dwFlagTime;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取微信快照花费时间,%.2f,秒,关键词为：%s"), dwFlagTime / 1000.0, pCurKeyWordDataInfo->strKeyWordName.GetString());
		}
		Sleep(10);
		if ((pCurKeyWordDataInfo->iFlag == SEARCH_PHONESHENMA) && bSearchFlag[PHONESHENMA_INDEX])  //抓取手机神马
		{
			dwFlagTime = GetTickCount();

			if (_T("1") == g_mapElemCfg[PHONESHENMA_INDEX].sKWSearchMethod)
			{
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl = g_mapElemCfg[PHONESHENMA_INDEX].sHomePage;
				pCurKeyWordDataInfo->iIndex = PHONESHENMA_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFileNew(*pCurKeyWordDataInfo, lp/*, bNextPage, sNextPageUrl*/);
			}
			else
			{
				// 只抓首页
				pCurKeyWordDataInfo->iCurPage = 0;
				pCurKeyWordDataInfo->strUrl.Format(g_mapElemCfg[PHONESHENMA_INDEX].strJumpUrl, pCurKeyWordDataInfo->strKeyHex.GetString(), 0);
				pCurKeyWordDataInfo->iIndex = PHONESHENMA_INDEX;
				pPhotoDlg->WaitURlAndSaveImageFile(*pCurKeyWordDataInfo, lp);
			}

			dwFlagTime = GetTickCount() - dwFlagTime;
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取手机神马快照花费时间,%.2f,秒,关键词为：%s"), dwFlagTime / 1000.0, pCurKeyWordDataInfo->strKeyWordName.GetString());

		}

		pPhotoDlg->exceptionHandling(*pCurKeyWordDataInfo);

		if (pCurKeyWordDataInfo != NULL)
		{
			delete pCurKeyWordDataInfo;
			pCurKeyWordDataInfo = NULL;
		}
		//返回数据
		pPhotoDlg->BackMsg();

	}

	pPhotoDlg->PostMessage(WM_QUIT);

	curl_global_cleanup();

	return 0;
}

void CWebDlg::initPageOpenState(int _iPageState)
{
	m_iPageOpenState = _iPageState;
}
int CWebDlg::getPageOpenState(void)
{
	return m_iPageOpenState;
}
void CWebDlg::initErrorWebId()
{
	m_strErrorWebId = _T("");
}
CString CWebDlg::getErrorWebId(void)
{
	return m_strErrorWebId;
}
void CWebDlg::setErrorWebId(CString _sErrorWebId)
{
	CString sErrorWebId = _sErrorWebId;
	if (!sErrorWebId.IsEmpty())
	{
		if (!m_strErrorWebId.IsEmpty())
		{
			sErrorWebId.Format(_T("(;2)%s"), _sErrorWebId);
		}
		m_strErrorWebId += sErrorWebId;
	}
}

// bool CWebDlg::getWebOpenFlag(void)
// {
// 	return m_bOpenWebSuccess;
// }
// void CWebDlg::setWebOpenFlag(bool _bFlag)
// {
// 	m_bOpenWebSuccess = _bFlag;
// }

#define  IRETRYTIMES    3       //重试次数
#define  TIMEOUTCOUTS   8      //超时时间
/*
@brief  等待web控件打开页面
@param  sData
@return
*/
BOOL CWebDlg::WaitURlAndSaveImageFile(const KeyWordDataInfo &sData, LPVOID lp)
{
	//	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ENTER WAITEURL"));
	BOOL bReady = FALSE;
	BOOL bResult = FALSE;
	int  iTimeleft;
	long lMessageBack = -1;
	pKeyWordDataInfo pData = new KeyWordDataInfo();
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据aaa...%d"), pData);

	try
	{
		if (pData != NULL)
		{
			pData->iCurPage = sData.iCurPage;
			pData->iFlag = sData.iFlag;
			pData->strKey = sData.strKey;
			pData->strKeyHex = sData.strKeyHex;
			pData->vCompanys = sData.vCompanys;
			pData->strKeyWordName = sData.strKeyWordName;
			pData->strUrl = sData.strUrl;
			pData->strWebFlag = sData.strWebFlag;
			pData->strWebList = sData.strWebList;
			pData->strComany = sData.strComany;
			pData->strWeixinName = sData.strWeixinName;
			pData->strClientType = sData.strClientType;
			pData->vAllCompanys = sData.vAllCompanys;
			pData->vOfficialList = sData.vOfficialList;
			pData->iIndex = sData.iIndex;
			pData->vCompanysTag = sData.vCompanysTag;
			pData->bOnlyRareWord = sData.bOnlyRareWord;
			Sleep(10);
			for (int i = 0; i<IRETRYTIMES; i++)  //网速比较慢打不开网页情况，重试
			{
				iTimeleft = 0;
				int iSend = -2;

				iSend = (BOOL)SendMessage(GETREDIRECTURL, 0, (LPARAM)pData);

				//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据bbb...%d"), iSend);
				//下面主要同步发消息给主线程去操作，防止不同线程操作出现崩溃现象,主要做监控效果
				if (iSend == 1)
				{
					do
					{
						//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据ccc..."));
						bReady = (BOOL)SendMessage(GETREDIRECTURL, 1, (LPARAM)pData);

						if (bReady || (iTimeleft > TIMEOUTCOUTS))
						{
							if (iTimeleft > TIMEOUTCOUTS)
							{
								g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("超时%d，当已加载完成处理,页面为：%s"), i + 1, sData.strUrl);

								SendMessage(GETREDIRECTURL, 2, (LPARAM)pData);
								break;
							}

							sthreadParam mpData;
							mpData.pData = pData;
							mpData.pPhotoDlg = (CWebDlg*)lp;

							//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据，关键词:%s  搜索标志为：%d"), sData.strKeyWordName, sData.iFlag);
							lMessageBack = SendMessage(GETSEARCHDATA, 0, (LPARAM)&mpData);
							//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("结束抓取网页数据，关键词:%s  搜索标志为：%d"), sData.strKeyWordName, sData.iFlag);
							break;
						}

						iTimeleft++;
						Sleep(500);

					} while (1);
				}

				//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("lMessageBack = %d，关键词:%s  搜索标志为：%d"), lMessageBack, sData.strKeyWordName, sData.iFlag);
				if (lMessageBack != OPEN_PAGE_FAILED)  //页面无法打开，需要重试
				{
					if (lMessageBack == NO_PAIMING)
					{
						g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("未抓取到快照,网址:%s"), sData.strUrl);

						if (sData.iCurPage == 0 && (sData.iFlag == SEARCH_PHONEBAIDU
							|| sData.iFlag == SEARCH_PHONESOGOU || sData.iFlag == SEARCH_YOUDAO
							|| sData.iFlag == SEARCH_PHONE360 || sData.iFlag == SEARCH_WEIXIN || sData.iFlag == SEARCH_PHONESHENMA)) // 手机百度第一页没抓到快照，添加排名为0的信息到容器
						{
							g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("%s未抓取到快照，添加排名为0的信息到容器"), GetSearchFlag(sData.iFlag));
							BackDataInfo* pBack = new BackDataInfo();
							pBack->strKeyWordName = sData.strKeyWordName;
							pBack->iFlag = sData.iFlag;
							pBack->strKey = sData.strKey;
							pBack->strCompanyName = sData.strComany;
							if (E_ALLURLOPEN_STATE3 == getPageOpenState() || E_ALLURLOPEN_STATE4 == getPageOpenState() || E_ALLURLOPEN_STATE5 == getPageOpenState())
							{
								pBack->iRank = 0;
								pBack->iRankCnt = -2;
								pBack->strRecordErrorInfo = getErrorWebId();
							}
							m_vBackData.push_back(pBack);
						}

						if (sData.iCurPage == 2)  //当前第三页
						{
							g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("当前第三页打开成功未抓取到快照，添加排名为0的信息到容器"));


							BackDataInfo* pBack = new BackDataInfo();
							pBack->strKeyWordName = sData.strKeyWordName;
							pBack->iFlag = sData.iFlag;
							pBack->strKey = sData.strKey;
							pBack->strCompanyName = sData.strComany;
							if (E_ALLURLOPEN_STATE3 == getPageOpenState() || E_ALLURLOPEN_STATE4 == getPageOpenState() || E_ALLURLOPEN_STATE5 == getPageOpenState())
							{
								pBack->iRank = 0;
								pBack->iRankCnt = -2;
								pBack->strRecordErrorInfo = getErrorWebId();
							}
							m_vBackData.push_back(pBack);
						}
						bResult = FALSE;  //正常打开页面，没抓取到快照
					}

					if (lMessageBack == PAGE_OFFICIAL_OPENFAILED)
					{
						//有官网没打开
						bResult = TRUE;  //
					}
					if (lMessageBack == PAIMING_EXIST)
					{
						g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取到快照，关键词:%s,网址:%s"), sData.strKeyWordName, sData.strUrl);
						bResult = TRUE;  //有快照
					}
					if (lMessageBack == ENGINE_APPEAR_VERIFYCODE)  //页面出现验证码
					{
						EnterCriticalSection(&critSearchFlag);
						bSearchFlag[pData->iIndex] = FALSE;
						LeaveCriticalSection(&critSearchFlag);
						bResult = TRUE;
					}
					break;
				}
				else
				{
					// 重试
					if (i < (IRETRYTIMES - 1))
					{
						g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("页面无法打开，重试,网址:%s,次数：%d"), sData.strUrl, i + 1);
					}
					else
					{
						// 重试3次了还没打开提交页面则不提交排名
					}
				}
			}

			delete pData;
		}
	}
	catch (...)
	{
		if (pData != NULL)
		{
			delete pData;
		}
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("执行SendMessage产生异常；"));
	}
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据return..."));

	return bResult;
}

BOOL CWebDlg::WaitURlAndSaveImageFileNew(const KeyWordDataInfo &sData, LPVOID lp/*, BOOL _BNextPage, CString& _NextPageUrl*/)
{
	BOOL bReady = FALSE;
	BOOL bResult = FALSE;
	int  iTimeleft;
	long lMessageBack = -1;
	pKeyWordDataInfo pData = new KeyWordDataInfo();
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据aaa...%d"), pData);

	try
	{
		if (pData != NULL)
		{
			pData->iCurPage = sData.iCurPage;
			pData->iFlag = sData.iFlag;
			pData->strKey = sData.strKey;
			pData->strKeyHex = sData.strKeyHex;
			pData->vCompanys = sData.vCompanys;
			pData->strKeyWordName = sData.strKeyWordName;
			pData->strUrl = sData.strUrl;
			pData->strWebFlag = sData.strWebFlag;
			pData->strWebList = sData.strWebList;
			pData->strComany = sData.strComany;
			pData->strWeixinName = sData.strWeixinName;
			pData->strClientType = sData.strClientType;
			pData->vAllCompanys = sData.vAllCompanys;
			pData->vOfficialList = sData.vOfficialList;
			pData->iIndex = sData.iIndex;
			pData->vCompanysTag = sData.vCompanysTag;
			pData->bOnlyRareWord = sData.bOnlyRareWord;
			Sleep(10);
			for (int i = 0; i < IRETRYTIMES; i++)  //网速比较慢打不开网页情况，重试
			{
				iTimeleft = 0;
				int iSend = -2;

				iSend = (BOOL)SendMessage(GETREDIRECTURL, 0, (LPARAM)pData);

				//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据bbb...%d"), iSend);
				//下面主要同步发消息给主线程去操作，防止不同线程操作出现崩溃现象,主要做监控效果
				if (iSend == 1)
				{
					do
					{
						//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据ccc..."));
						bReady = (BOOL)SendMessage(GETREDIRECTURL, 1, (LPARAM)pData);

						if (bReady || (iTimeleft > TIMEOUTCOUTS))
						{
							if (iTimeleft > TIMEOUTCOUTS)
							{
								g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("超时%d，当已加载完成处理,页面为：%s"), i + 1, sData.strUrl);

								SendMessage(GETREDIRECTURL, 2, (LPARAM)pData);
								break;
							}

							sthreadParam mpData;
							mpData.pData = pData;
							mpData.pPhotoDlg = (CWebDlg*)lp;

							//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据，关键词:%s  搜索标志为：%d"), sData.strKeyWordName, sData.iFlag);
							lMessageBack = SendMessage(GETSEARCHDATANEW, 0, (LPARAM)&mpData);
							//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("结束抓取网页数据，关键词:%s  搜索标志为：%d"), sData.strKeyWordName, sData.iFlag);
							break;
						}

						iTimeleft++;
						Sleep(500);

					} while (1);
				}

				//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("lMessageBack = %d，关键词:%s  搜索标志为：%d"), lMessageBack, sData.strKeyWordName, sData.iFlag);
				if (lMessageBack != OPEN_PAGE_FAILED)  //页面无法打开，需要重试
				{
					if (lMessageBack == NO_PAIMING)
					{
						g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("未抓取到快照,网址:%s"), sData.strUrl);

// 						if (sData.iCurPage == 0 && (sData.iFlag == SEARCH_PHONEBAIDU
// 							|| sData.iFlag == SEARCH_PHONESOGOU || sData.iFlag == SEARCH_YOUDAO
// 							|| sData.iFlag == SEARCH_PHONE360 || sData.iFlag == SEARCH_WEIXIN || sData.iFlag == SEARCH_PHONESHENMA)) // 手机百度第一页没抓到快照，添加排名为0的信息到容器
// 						{
							g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("%s未抓取到快照，添加排名为0的信息到容器"), GetSearchFlag(sData.iFlag));
							BackDataInfo* pBack = new BackDataInfo();
							pBack->strKeyWordName = sData.strKeyWordName;
							pBack->iFlag = sData.iFlag;
							pBack->strKey = sData.strKey;
							pBack->strCompanyName = sData.strComany;
							if (E_ALLURLOPEN_STATE3 == getPageOpenState() || E_ALLURLOPEN_STATE4 == getPageOpenState() || E_ALLURLOPEN_STATE5 == getPageOpenState())
							{
								pBack->iRank = 0;
								pBack->iRankCnt = -2;
								pBack->strRecordErrorInfo = getErrorWebId();
							}
							m_vBackData.push_back(pBack);
//						}

// 						if (pData->iCurPage == (g_mapElemCfg[pData->iIndex].iSearchPageNum - 1))  //当前第三页
// 						{
// 							g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("当前第三页打开成功未抓取到快照，添加排名为0的信息到容器"));
// 
// 
// 							BackDataInfo* pBack = new BackDataInfo();
// 							pBack->strKeyWordName = sData.strKeyWordName;
// 							pBack->iFlag = sData.iFlag;
// 							pBack->strKey = sData.strKey;
// 							pBack->strCompanyName = sData.strComany;
// 							if (E_ALLURLOPEN_STATE3 == getPageOpenState() || E_ALLURLOPEN_STATE4 == getPageOpenState() || E_ALLURLOPEN_STATE5 == getPageOpenState())
// 							{
// 								pBack->iRank = 0;
// 								pBack->iRankCnt = -2;
// 								pBack->strRecordErrorInfo = getErrorWebId();
// 							}
// 							m_vBackData.push_back(pBack);
// 						}
						bResult = FALSE;  //正常打开页面，没抓取到快照
					}

					if (lMessageBack == PAGE_OFFICIAL_OPENFAILED)
					{
						//有官网没打开
						bResult = TRUE;  //
					}
					if (lMessageBack == PAIMING_EXIST)
					{
						g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("抓取到快照，关键词:%s,网址:%s"), sData.strKeyWordName, sData.strUrl);
						bResult = TRUE;  //有快照
					}
					if (lMessageBack == ENGINE_APPEAR_VERIFYCODE)  //页面出现验证码
					{
						EnterCriticalSection(&critSearchFlag);
						bSearchFlag[pData->iIndex] = FALSE;
						LeaveCriticalSection(&critSearchFlag);
						bResult = TRUE;
					}
					break;
				}
				else
				{
					// 重试
					if (i < (IRETRYTIMES - 1))
					{
						g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("页面无法打开，重试,网址:%s,次数：%d"), sData.strUrl, i + 1);
					}
					else
					{
						// 重试3次了还没打开提交页面则不提交排名
					}
				}
			}

			delete pData;
		}
	}
	catch (...)
	{
		if (pData != NULL)
		{
			delete pData;
		}
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("执行SendMessage产生异常；"));
	}
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据return..."));

	return bResult;
}

bool CWebDlg::getSnapShotUrlAssist(const CString _strPage, const KeyWordDataInfo &_sData, CString& _strSnapShotUrlAssist)
{
	CStringA strListA = _sData.strWebList;
	CString strListW;
	strListW = CStrA2CStrW(strListA);

	vector<CString > vsSiteList;  //网站列表  40000811,http://www.jiu6.com/,01110011001110010
	vector<CString > vsDomainList;//保存的域名列表
	vsSiteList.clear();
	vsDomainList.clear();

	SplitCString(strListW, _T("\r\n"), vsSiteList, true);

	for (int i = 0; i < vsSiteList.size(); i++)
	{
		vector<CString > vSingleSite;
		SplitCString(vsSiteList[i], _T(","), vSingleSite, true);
		if (3 == vSingleSite.size() && (!vSingleSite[1].IsEmpty() && _T("http://") != vSingleSite[1] && _T("http:///") != vSingleSite[1]))//第4代网站列表每一条有3个参数
		{
			vsDomainList.push_back(vSingleSite[1]);
		}
		else if (2 == vSingleSite.size())//第2代网站列表每一条有2个参数
		{
			break;
		}
	}

	for (int index = 0; index < vsDomainList.size(); index++)
	{
		if (-1 != _strPage.Find(vsDomainList[index]))
		{
			_strSnapShotUrlAssist = vsDomainList[index];
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("提取快照辅助网址：_strSnapShotUrlAssist = %s\r\n"), _strSnapShotUrlAssist);
			return true;
		}
	}

	return false;
}

bool CWebDlg::getSnapUrlAndSnapShotUrl(const CComQIPtr<IHTMLElement>& _htmlElement, const KeyWordDataInfo &sData, CString& _strSnapUrl, CString& _strSnapShotUrl)
{
	//获取快照链接
	vector<locationStruct> vstrUrlTitleFlag = g_mapElemCfg[sData.iIndex].vstrLocationTitle;
	targetAddressSign loctargetTitleSign = g_mapElemCfg[sData.iIndex].strTargetTitle;
	bool bSnapUrl = getUrlPath(_htmlElement, vstrUrlTitleFlag, loctargetTitleSign, _strSnapUrl);
	//获取快照地址
	vector<locationStruct> vstrUrlLinkFlag = g_mapElemCfg[sData.iIndex].vstrLocationLink;
	targetAddressSign loctargetLinkSign = g_mapElemCfg[sData.iIndex].strTargetLink;
	bool bSnapShotUrl = getUrlPath(_htmlElement, vstrUrlLinkFlag, loctargetLinkSign, _strSnapShotUrl);


	// 	if (BADIDU_INDEX == sData.iIndex && (_T("深圳市贵喜钣金有限公司") == sData.strComany || _T("东莞市吉诺塑胶制品有限公司") == sData.strComany))
	// 	{
	// 		CComBSTR bstr;
	// 		CString strPage;
	// 		_htmlElement->get_outerHTML(&bstr);
	// 		strPage = bstr;
	// 		g_debugLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("_strSnapUrl = %s, _strSnapShotUrl = %s\r\n"), _strSnapUrl, _strSnapShotUrl);
	// 		//g_debugLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("sData.iIndex = %d引擎条目的域名查找成功,当前条目源码为%s\r\n"), sData.iIndex, strPage);
	// 		if (!bSnapUrl || !bSnapShotUrl || _strSnapUrl.IsEmpty() || _strSnapShotUrl.IsEmpty())
	// 		{
	// 			g_debugLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("链接提取失败"));
	// 		}
	// 		else
	// 		{
	// 			g_debugLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("链接提取成功"));
	// 		}
	// 	}

	if (SEARCH_PHONEBAIDU == sData.iFlag)
	{
		if (!bSnapUrl || _strSnapUrl.IsEmpty())
		{
			//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("提取链接失败：_strSnapUrl = %s, _strSnapShotUrl = %s\r\n"), _strSnapUrl, _strSnapShotUrl);
			return false;
		}
	}
	else
	{
		if (!bSnapUrl || !bSnapShotUrl || _strSnapUrl.IsEmpty() || _strSnapShotUrl.IsEmpty())
		{
			//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("提取链接失败：_strSnapUrl = %s, _strSnapShotUrl = %s\r\n"), _strSnapUrl, _strSnapShotUrl);
			return false;
		}
	}
	return true;
}

bool CWebDlg::getUrlPath(const CComQIPtr<IHTMLElement>& _htmlElement, vector<locationStruct>& _vstrUrlFlag, targetAddressSign& _targetAddressSign, CString& _strOutUrl)
{
	HRESULT hr;
	plocationStruct plocStruct = NULL;
	vector<locationStruct>::iterator it = _vstrUrlFlag.begin();
	CComQIPtr<IHTMLElement> pElement = _htmlElement;

	try
	{
		if (it == _vstrUrlFlag.end())
		{
			//容器数据为空
			if (ELEMENT_MARK_INNERTEXT == _targetAddressSign.iUrlGetType)
			{
				CComBSTR bstr;
				pElement->get_innerText(&bstr);
				_strOutUrl = bstr;
			}
			else if (ELEMENT_MARK_HREF == _targetAddressSign.iUrlGetType)
			{
				CComBSTR bstrURL;
				CString strUrl = _T("");
				CComQIPtr<IHTMLAnchorElement> pAnchor;
				pAnchor = pElement;
				pAnchor->get_href(&bstrURL);
				strUrl = bstrURL;
				_strOutUrl = strUrl;
			}
			else if (ELEMENT_MARK_ATTRIBUTE == _targetAddressSign.iUrlGetType)
			{
				_bstr_t  bstrName(_targetAddressSign.sHrefKey);
				_variant_t vtValue;
				vtValue.bstrVal = SysAllocString(NULL);
				vtValue.vt = VT_BSTR;
				hr = pElement->getAttribute(bstrName, 0, &vtValue);
				if (FAILED(hr) || NULL == vtValue.bstrVal)
				{
					return 0;
				}
				_strOutUrl = vtValue.bstrVal;
			}
			_strOutUrl.Trim();
			if (_strOutUrl.IsEmpty())
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{

			CComDispatchDriver spElem;
			CComQIPtr<IHTMLElementCollection> pAllColls;
			CComPtr<IDispatch> pDispath;
			CComQIPtr<IHTMLElementCollection> pTags;
			long lCounts;
			bool bFoundItem = false;

			hr = pElement->get_all(&spElem);
			if (FAILED(hr) || NULL == spElem)
			{
				return false;
			}

			hr = spElem.QueryInterface(&pAllColls);
			if (FAILED(hr) || NULL == pAllColls)
			{
				return false;
			}


			pAllColls->tags(COleVariant(it->sTagType.AllocSysString()), &pDispath);
			if (FAILED(hr) || NULL == pDispath)
			{
				return false;
			}

			pTags = pDispath;
			if (LOCATION_ITEM_TAG_NULL == it->iLocationType)
			{
				long lindex = 0;
				if (!getSingleItem(pTags, lindex, pElement))
				{
					return false;
				}
				bFoundItem = true;
			}
			else if (LOCATION_ITEM_TAG_INDEX == it->iLocationType)
			{
				if (!getSingleItem(pTags, it->lFuzzyIndex, pElement))
				{
					return false;
				}
				bFoundItem = true;
			}
			else if (LOCATION_ITEM_TAG_CLASS == it->iLocationType)
			{
				pTags->get_length(&lCounts);
				for (long lt = 0; lt < lCounts; lt++)
				{
					if (!getSingleItem(pTags, lt, pElement))
					{
						continue;
					}
					_bstr_t bstrVal;
					pElement->get_className(&bstrVal.GetBSTR());
					CString strClassValue = bstrVal;
					strClassValue.Trim();
					if (it->sValue == strClassValue)
					{
						bFoundItem = true;
						break;
					}
				}
			}
			if (bFoundItem)
			{
				_vstrUrlFlag.erase(it);//删除第一个元素
				if (getUrlPath(pElement, _vstrUrlFlag, _targetAddressSign, _strOutUrl))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}
	catch (CException* e)
	{
	}

	return false;
}
bool CWebDlg::getSingleItem(CComQIPtr<IHTMLElementCollection>& _pTags, long _lindex, CComQIPtr<IHTMLElement>& _pElement)
{
	HRESULT hr;
	CComDispatchDriver spElem; // IDispatch 的智能指针
	long lindex = _lindex;
	VARIANT index = { 0 };
	V_VT(&index) = VT_I4;
	V_I4(&index) = _lindex;
	hr = _pTags->item(COleVariant(lindex), index, &spElem);
	if (FAILED(hr) || spElem == NULL)
	{
		return false;
	}
	Sleep(20);

	spElem.QueryInterface(&_pElement);

	if (FAILED(hr) || _pElement == NULL)
	{
		return false;
	}
	return true;
}

bool CWebDlg::UrlIsInOfficialList(const CString _strUrlLink, const vector<CString> &vHostlist)
{
	int iPos1 = 0;
	int iPos2 = 0;
	BOOL bReturn = FALSE;
	CString strUrlLink = _strUrlLink;
	strUrlLink.MakeLower();

	//查看当前网址是否包含在官网中
	if (vHostlist.size() > 0)
	{
		for (int i = 0; i < vHostlist.size(); ++i)
		{
			CString strLow = vHostlist[i];
			strLow.MakeLower();
			iPos2 = strUrlLink.Find(strLow);
			if (iPos2 != -1)
			{
				//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("包含官网排名：%s"), sData.vOfficialList[i]);
				bReturn = TRUE;
				break;
			}
		}
	}

	return bReturn;
}
bool CWebDlg::cutUrl(const CString _strUrlLink, CString& _strDomain, CString& _strUrlPath)
{
	// 	info.b2b168.com.cn / s168 - 76540989.html
	// 	www.mao35.com > 商务服务相关 > 其他分类
	// 	www.zendang.com / info / 1573230.html
	// 	www.jixie35.com / p... - 2017 - 8 - 8
	// 	环球经贸网 - china.nowec.com / sup... - 2012 - 8 - 20
	// 	中国金属新闻网 - www.metalnews.cn / c... - 2017 - 8 - 21
	// 	shop.m.71.net
	// 	gtobal.com
	// 	www.baizhuwang.com - 22小时前
	// 	s.hc360.com - 2017 - 9 - 19
	// 	https://tieba.baidu.com/f?kw=废铜



	const int iDomainCount = 21;
	//顶级域名
	TCHAR  sDomain[iDomainCount][10] =
	{
		_T(".com"), _T(".cn"), _T(".net"), _T(".cc"), _T(".org"), _T(".biz"), _T(".info"), _T(".mobi"),
		_T(".edu"), _T(".gov"), _T(".int"), _T(".pro"), _T(".name"), _T(".coop"), _T(".aero"), _T(".xxx"),
		_T(".top"), _T(".ren"), _T(".tv"), _T(".xin"), _T(".club")
	};

	BOOL bReturn = FALSE;
	CString strUrlLink = _strUrlLink;
	strUrlLink.MakeLower();
	strUrlLink.Trim();

	//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("当前分割的域名地址为：%s"), strUrlLink);

	int index = 0;
	for (index = 0; index < strUrlLink.GetLength(); index++)
	{
		TCHAR cChar = strUrlLink.GetAt(index);
		if (_T(' ') == cChar || _T('>') == cChar)
		{
			CString strTemp = strUrlLink.Left(index);
			bool bFound = false;
			for (int j = 0; j < iDomainCount; j++)
			{
				if (-1 != strTemp.Find(sDomain[j]))
				{
					strUrlLink = strTemp;
					bFound = true;
					index = 0;
					break;
				}
			}

			if (!bFound)
			{
				strUrlLink = strUrlLink.Right(strUrlLink.GetLength() - index - 1);
				index = 0;
			}
			else
			{
				break;
			}
		}
	}


	while (true)
	{
		if (_T(".") == strUrlLink.Right(1))
		{
			strUrlLink = strUrlLink.Left(strUrlLink.GetLength() - 1);
		}
		else
		{
			break;
		}
	}
	int iPos = strUrlLink.Find(_T("..."));
	if (-1 != iPos)
	{
		strUrlLink = strUrlLink.Left(iPos);
	}
	// 	iPos = strUrlLink.Find(_T(">"));
	// 	if (-1 != iPos)
	// 	{
	// 		strUrlLink = strUrlLink.Left(iPos);
	// 	}


	CString strUrlLinkTemp = strUrlLink;

	while (true)
	{
		int iPos = strUrlLink.Find(_T("//"));
		if (-1 != iPos)
		{
			strUrlLink = strUrlLink.Mid(iPos + 2);
			strUrlLinkTemp = strUrlLink;
		}
		iPos = strUrlLink.ReverseFind('/');
		if (-1 != iPos)
		{
			strUrlLink = strUrlLink.Left(iPos);
		}
		else
		{
			_strDomain = strUrlLink;
			_strUrlPath = strUrlLinkTemp.Mid(_strDomain.GetLength());
			break;
		}
	}
	return true;
}

bool CWebDlg::SplitCString(const CString & input, const CString & delimiter, std::vector<CString >& results, bool includeEmpties)
{
	int iPos = 0;
	int newPos = -1;
	int sizeS2 = (int)delimiter.GetLength();
	int isize = (int)input.GetLength();

	int offset = 0;
	CString  s;

	if (
		(isize == 0)
		||
		(sizeS2 == 0)
		)
	{
		return 0;
	}

	std::vector<int> positions;

	newPos = input.Find(delimiter, 0);

	if (newPos < 0)
	{
		if (!input.IsEmpty())
		{
			results.push_back(input);
		}
		return 0;
	}

	int numFound = 0;

	while (newPos >= iPos)
	{
		numFound++;
		positions.push_back(newPos);
		iPos = newPos;
		if (iPos + sizeS2 < isize)
		{
			newPos = input.Find(delimiter, iPos + sizeS2);
		}
		else
		{
			newPos = -1;
		}
	}

	if (numFound == 0)
	{
		return 0;
	}

	for (int i = 0; i <= (int)positions.size(); ++i)
	{
		s.Empty();
		if (i == 0)
		{
			s = input.Mid(i, positions[i]);
		}
		else
		{
			offset = positions[i - 1] + sizeS2;

			if (offset < isize)
			{
				if (i == positions.size())
				{
					s = input.Mid(offset);
				}
				else if (i > 0)
				{
					s = input.Mid(positions[i - 1] + sizeS2, positions[i] - positions[i - 1] - sizeS2);
				}
			}
		}

		if (/*includeEmpties || */(s.GetLength() > 0))
		{
			results.push_back(s);
		}
	}
	return true;
}

bool CWebDlg::cutDomain(const CString _strDomain, CString& _strUsefulDomain)
{
	CString strDomain = _strDomain;
	const int iFirstDomainCount = 21;
	const int iSecondDomainCount = 27;
	//顶级域名
	TCHAR  sFirstDomain[iFirstDomainCount][10] =
	{
		_T("com"), _T("cn"), _T("net"), _T("cc"), _T("org"), _T("biz"), _T("info"), _T("mobi"),
		_T("edu"), _T("gov"), _T("int"), _T("pro"), _T("name"), _T("coop"), _T("aero"), _T("xxx"),
		_T("top"), _T("ren"), _T("tv"), _T("xin"), _T("club")
	};
	//二级域名
	TCHAR  sSecondDomain[iSecondDomainCount][10] =
	{
		_T("com"), _T("net"), _T("org"), _T("ac"), _T("ah"), _T("bj"), _T("cq"), _T("fj"),
		_T("gd"), _T("gov"), _T("gs"), _T("gx"), _T("gz"), _T("ha"), _T("hb"), _T("he"),
		_T("hi"), _T("hk"), _T("hl"), _T("hn"), _T("jl"), _T("js"), _T("jx"), _T("ln"),
		_T("mo"), _T("nm"), _T("nx")
	};
	//www.abc.com.cn
	vector<CString > results;
	SplitCString(strDomain, _T("."), results, true);
	int ilen = results.size();
	if (ilen < 2)
	{
		return false;
	}
	else
	{
		bool bFoundFir = false;
		for (int i = 0; i < iFirstDomainCount; i++)
		{
			if (results[ilen - 1] == sFirstDomain[i])
			{
				//找到一级域名
				bFoundFir = true;
				break;
			}
		}
		if (!bFoundFir)
		{
			//xxxx.1689......  
			return false;
		}
		else
		{
			if (2 == ilen)
			{
				_strUsefulDomain = strDomain;
			}
			else
			{
				bool bFondSec = false;
				for (int i = 0; i < iSecondDomainCount; i++)
				{
					if (results[ilen - 2] == sSecondDomain[i])
					{
						//找到二级域名
						bFondSec = true;
						break;
					}
				}

				if (3 == ilen)
				{
					if (bFondSec)
					{
						_strUsefulDomain = strDomain;
					}
					else
					{
						_strUsefulDomain = results[ilen - 2] + _T(".") + results[ilen - 1];
					}
				}
				else
				{
					if (bFondSec)
					{
						_strUsefulDomain = results[ilen - 3] + _T(".") + results[ilen - 2] + _T(".") + results[ilen - 1];
					}
					else
					{
						_strUsefulDomain = results[ilen - 2] + _T(".") + results[ilen - 1];
					}
				}
			}
		}
	}

	return true;
}

bool CWebDlg::LinkUrlIsInOurSitelist(const CString _strUrl, const CString _strUrlLink, const KeyWordDataInfo &sData, BOOL& bOwnOfficial, DWORD& dwWebId, CString& _strUserfulDomain, CString& _strPage, bool& _bIsOfficialWebFlag)
{
	CString strUrlLinkParam = _strUrl;
	CString strUrlRedirectParam;

	// 	CString jjj;
	// 	jjj = _T("http://cn.sonhoo.com/Promotion/dafak/okqm8/20161108/xsvosafb/");
	// 	jjj = _T("http://cn.sonhoo.com/dafak/okqm8/20161108/xsvosafb/");
	// 	jjj = _T("http://www.metalnews.cn/jingzhu");
	// 	jjj = _T("http://www.metalnews.cn/jingzhuntuiguang/4555");
	//	CString strUrlLink = jjj;

	CString strUrlLink = _strUrlLink;
	try
	{
		strUrlLink.MakeLower();
	}
	catch (CException* e)
	{
		strUrlLink = _strUrlLink;
	}
	//查看当前网址是否包含在官网中
	bOwnOfficial = UrlIsInOfficialList(strUrlLink, sData.vOfficialList);
	CString strDomain;
	CString strUrlPath;
	//strUrlLink		www.cnlinfo.net/dianci...
	cutUrl(strUrlLink, strDomain, strUrlPath);

	CString strUsefulDomain;
	if (!cutDomain(strDomain, strUsefulDomain))
	{
		//进一步模糊查找域名
		if (!getSnapShotUrlAssist(_strPage, sData, strUsefulDomain))
		{
			if (SEARCH_PHONEBAIDU == sData.iFlag)
			{
				GetAddrByUrl(strUrlLinkParam, strUrlRedirectParam);
			}
			else
			{
				GetRedirectUrl(sData, strUrlLinkParam, strUrlRedirectParam);
			}
			cutUrl(strUrlRedirectParam, strDomain, strUrlPath);
			cutDomain(strDomain, strUsefulDomain);
		}
	}

	//域名对比
	//bReturn = DomainCompare(strUsefulDomain, strUrlPath, sData, dwWebId, _bIsOfficialWebFlag, false);
	int iReValue = DomainCompare(strUsefulDomain, strUrlPath, sData, dwWebId, _bIsOfficialWebFlag, false);
	if (2 == iReValue)
	{
		GetRedirectUrl(sData, strUrlLinkParam, strUrlRedirectParam);
		cutUrl(strUrlRedirectParam, strDomain, strUrlPath);
		cutDomain(strDomain, strUsefulDomain);
		iReValue = DomainCompare(strUsefulDomain, strUrlPath, sData, dwWebId, _bIsOfficialWebFlag, true);
	}
	if (1 == iReValue)
	{
		_strUserfulDomain = strUsefulDomain;
		return true;
	}
	return false;
}
int CWebDlg::DomainCompare(const CString & _sCurDomain, const CString & _sCurUrlPath, const KeyWordDataInfo &sData, DWORD& dwWebId, bool& _bIsOfficialWebFlag, bool _bFlag)
{
	CStringA strListA = sData.strWebList;
	CString strListW;
	strListW = CStrA2CStrW(strListA);

	vector<CString > vsSiteList;
	SplitCString(strListW, _T("\r\n"), vsSiteList, true);

	struct webInfo
	{
		CString sWebId;
		CString sDomain;
		CString sUrlPath;
		bool bIsOfficialWebFlag;
	};
	vector<webInfo> vWebInfo;
	vWebInfo.clear();
	for (int i = 0; i < vsSiteList.size(); i++)
	{
		vector<CString > vSingleSite;
		SplitCString(vsSiteList[i], _T(","), vSingleSite, true);
		CString strSDomain;
		CString strSUrlPath;
		CString strSUsefulDomain;
		if (3 == vSingleSite.size())
		{
			cutUrl(vSingleSite[1], strSDomain, strSUrlPath);
			if (cutDomain(strSDomain, strSUsefulDomain))
			{
				if (_sCurDomain == strSUsefulDomain)
				{
					webInfo webInfoTem;
					webInfoTem.sWebId = vSingleSite[0];
					webInfoTem.sDomain = strSUsefulDomain;
					webInfoTem.sUrlPath = strSUrlPath;

					//01110011001411001 第十二位为4表示官网
					if (_T("4") == vSingleSite[2].Mid(11, 1))
					{
						webInfoTem.bIsOfficialWebFlag = true;
					}
					else
					{
						webInfoTem.bIsOfficialWebFlag = false;
					}
					vWebInfo.push_back(webInfoTem);
				}
			}
		}
		else if (2 == vSingleSite.size())
		{
			break;
		}
	}

	int iWebLen = vWebInfo.size();
	if (0 == iWebLen)
	{
		return 0;
	}
	else if (1 == iWebLen)
	{
		dwWebId = atol(CStrW2CStrA(vWebInfo[0].sWebId));
		_bIsOfficialWebFlag = vWebInfo[0].bIsOfficialWebFlag;
		return 1;
	}
	else
	{
		if (!_bFlag)
		{
			return 2;
		}
		bool bUrlPath = false;
		CString strOfficial;
		int index = 0;
		for (int i = 0; i < iWebLen; i++)
		{
			if (vWebInfo[i].sUrlPath.GetLength() > 1)
			{
				strOfficial = vWebInfo[i].sUrlPath;
				index = i;
				break;
			}
		}
		if (!strOfficial.IsEmpty() && (-1 != _sCurUrlPath.Find(strOfficial)))
		{
			dwWebId = atol(CStrW2CStrA(vWebInfo[index].sWebId));
			_bIsOfficialWebFlag = vWebInfo[index].bIsOfficialWebFlag;
		}
		else
		{
			if (0 == index)
			{
				index = 1;
			}
			else
			{
				index = 0;
			}
			dwWebId = atol(CStrW2CStrA(vWebInfo[index].sWebId));
			_bIsOfficialWebFlag = vWebInfo[index].bIsOfficialWebFlag;
		}
		return 1;
	}
}


/*
@brief  在百度页面上抓取排名
@param  过滤标签内容
@param  sData  关键词相关数据
@param  strFileName  要保存的图片路径   输出
@return
0：有排名
1：无排名
2：打开网页错误
*/
int  CWebDlg::GetBaiduPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *_pAllColls)
{
	int iRetValue = OPEN_PAGE_FAILED;
	int iRankCnt = 0;   //抓取到的排名数量
	std::vector<long> vHeights;  //返回高度，可能有多条数据
	CString strWebRecord = _T("");  //网站排名记录

	if (pTables == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("IHTMLElementCollection baidu is NULL"));
		return iRetValue;
	}
	CComQIPtr<IHTMLElement> pElement;

	BackDataInfo *pBdata = NULL;
	HRESULT hr;
	CString strAllRedirectUrl(_T(""));
	for (long l = sData.iCurPage * 10 + 1; l <= sData.iCurPage * 10 + 10; l++)
	{
		IDispatch  *pDisp = NULL;
		CComDispatchDriver spElem;
		CString strTemp;
		BOOL	bIsB2BCatch = FALSE;

		strTemp.Format(g_mapElemCfg[BADIDU_INDEX].strItemFlag1, l);

		VARIANT index = { 0 };
		V_VT(&index) = VT_I4;
		V_I4(&index) = 0;

		VARIANT varID = { 0 };
		VariantInit(&varID);
		varID.vt = VT_BSTR;

		varID.bstrVal = strTemp.AllocSysString();
		hr = pTables->item(varID, index, &spElem);
		SysFreeString(varID.bstrVal);

		if (FAILED(hr) || spElem == NULL)
			continue;

		Sleep(30);
		HRESULT hr = spElem.QueryInterface(&pElement);
		spElem.Release();


		CComBSTR bstr;
		CString strPage;
		pElement->get_innerHTML(&bstr);
		strPage = bstr;


		CString strSnapUrl(_T(""));
		CString strSnapShotUrl(_T(""));
		if (!getSnapUrlAndSnapShotUrl(pElement, sData, strSnapUrl, strSnapShotUrl))
		{
			continue;
		}


		BOOL bIsAce = FALSE;
		CatchInfo cInfo;
		DWORD dwWebId = 0;				//网站ID;
		bool bIsOfficialWebFlag = false;	//当前网站是否为官网网址;

		CString strUserfulDomain(_T(""));
		BOOL bFind = LinkUrlIsInOurSitelist(strSnapUrl, strSnapShotUrl, sData, cInfo.bIsOfficialFlag, dwWebId, strUserfulDomain, strPage, bIsOfficialWebFlag);

		if (bFind || cInfo.bIsOfficialFlag)
		{
			CString strUrl = strSnapUrl;
			int iAllOpenState = -1;
			CString strAllErrorInfo(_T(""));
			int iResult = HasCompanyInHtml(strUrl, sData, cInfo, dwWebId, lp, strSnapShotUrl, strAllRedirectUrl, strUserfulDomain, bIsOfficialWebFlag, iAllOpenState, strAllErrorInfo);
			changeOpenUrlState(iAllOpenState);
			setErrorWebId(strAllErrorInfo);
			if (iResult != FOUND_COMPANY || dwWebId <= 0)
			{
				if (iResult == NOT_FOUND_COMPANY || iResult == PAGE_NOT_FOUND)
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("baidu ：没有在目标网页中找到公司名"));

				if (!cInfo.bIsOfficialFlag)
				{
					continue;
				}
			}
			else
			{
				bIsB2BCatch = TRUE;
				iRankCnt++;
			}

			//如果找到递增1
			if (cInfo.bIsOfficialFlag)
			{
				iRankCnt++;
			}

			addPageTags(pElement, cInfo, dwWebId, sData, bIsAce);

			if (vHeights.size() == 0)
			{
				pBdata = new BackDataInfo();
				ASSERT(pBdata != NULL);

				pBdata->iFlag = SEARCH_BAIDU;
				pBdata->iRank = l;

				if (0 == sData.iCurPage)
				{
					if (pBdata->iRank > 10)
					{
						pBdata->iRank = 10;
					}
				}
				else if (1 == sData.iCurPage)
				{
					if (pBdata->iRank > 20)
					{
						pBdata->iRank = 20;
					}
				}
				else if (2 == sData.iCurPage)
				{
					if (pBdata->iRank > 30)
					{
						pBdata->iRank = 30;
					}
				}

				pBdata->strKeyWordName = sData.strKeyWordName;
				pBdata->strKey = sData.strKey;
				pBdata->strCompanyName = sData.strComany;
			}
			vHeights.push_back(l);
			//如果抓到包含官网的，设置标记为1
			if (cInfo.bIsOfficialFlag)
			{
				pBdata->iOfficialFlag = 1;
				//Modified Date 2018/05/04
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, l);
				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			//如果抓到包含其他的,设置标记为1
			else if (bIsB2BCatch)
			{
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, l);
				//是否为2代网站
				if (dwWebId < 40000000 && dwWebId != 4000065)
					pBdata->iB2BFlag = 1;
				//是否为王牌网站
				if (cInfo.bIsAceFlag && bIsAce)
					pBdata->iAceFlag = 1;
				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}

			if (pBdata != NULL && 0 == pBdata->iKeywordType)
			{
				pBdata->iKeywordType = IsADWord(sData.iFlag, sData.iIndex, _pAllColls);
			}
		}
	}
	addBackData(pBdata, sData, iRetValue, strFileName, iRankCnt, strWebRecord, strAllRedirectUrl);
	return iRetValue;
}

bool CWebDlg::addPageTags(CComQIPtr<IHTMLElement>& _pElement, CatchInfo& _cInfo, DWORD& _dwWebId, const KeyWordDataInfo& _sData, BOOL& _bIsAce)
{
	CComBSTR bstr;
	CString strPage;

	_pElement->get_innerHTML(&bstr);
	strPage = bstr;

	//if (strPage.Find(_T("style=\"padding: 5px; border: 2px solid rgb")) == -1)
	if (strPage.Find(_T("style = \"padding: 5px;")) == -1)
	{
		CString sInsertData;
		//判断客户是否为王牌客户，				
		if (_sData.strClientType == _T("1") && (_cInfo.bIsOfficialFlag || _cInfo.bIsAceFlag || _cInfo.bSpecialOfficialFlag))
		{
			//加黄框
			_bIsAce = TRUE;
			sInsertData.Format(_T("<div style = \"padding: 5px; border: 2px solid rgb(255, 215, 0); border-image: none; margin-bottom: 5px; box-shadow: 1px 1px 3px 0px rgba(0,0,0,0.6);\" id=\"sum%d\" name=\"sum%d\">"), _dwWebId, _dwWebId);
			//sInsertData.Format(_T("<div style = \"padding: 5px; border-image: none; margin-bottom: 5px;\" id=\"%d\">"), _dwWebId);
			strPage.Insert(0, sInsertData);
			//strPage.Insert(0, _T("<div style=\"padding: 5px; border: 2px solid rgb(255, 215, 0); border-image: none; margin-bottom: 5px; box-shadow: 1px 1px 3px 0px rgba(0,0,0,0.6);\">"));
		}
		else
		{
			//加红框
			//sInsertData.Format(_T(""), dwWebId)
			sInsertData.Format(_T("<div style = \"padding: 5px; border: 2px solid rgb(223, 0, 1); border-image: none; margin-bottom: 5px; box-shadow: 1px 1px 3px 0px rgba(0,0,0,0.6);\" id=\"sum%d\" name=\"sum%d\">"), _dwWebId, _dwWebId);
			//sInsertData.Format(_T("<div style = \"padding: 5px; border-image: none; margin-bottom: 5px;\" id=\"%d\">"), _dwWebId);
			strPage.Insert(0, sInsertData);
		}
		strPage.Append(_T("</div>"));
		HRESULT hr = _pElement->put_innerHTML((BSTR)strPage.GetString());

		if (FAILED(hr))
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("baidu ：加红框失败"));
			return false;
		}
		else
		{
			return true;
		}
	}
	return false;
}

/*
@brief  在手机百度页面上抓取排名
@param  过滤标签内容
@param  sData  关键词相关数据
@param  strFileName  要保存的图片路径   输出
@return	0：有排名	1：无排名	2：打开网页错误
*/
int CWebDlg::GetPhoneBaiduPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *_pAllColls)
{
	int iRetValue = OPEN_PAGE_FAILED;
	int iRankCnt = 0;   //抓取到的排名数量
	std::vector<long> vHeights;  //返回高度，可能有多条数据
	CString strWebRecord = _T("");  //网站排名记录

	if (pTables == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("IHTMLElementCollection phonebaidu is NULL"));
		return iRetValue;
	}

	CComQIPtr<IHTMLElement> pElement;
	CComQIPtr<IHTMLAnchorElement> pAnchor;
	BackDataInfo *pBdata = NULL;
	HRESULT hr;

	int iRank = 0;
	long lDivCounts = 0;
	pTables->get_length(&lDivCounts);
	CString strAllRedirectUrl(_T(""));

	for (long l = 0; l <= lDivCounts; l++)
	{
		IDispatch  *pDisp = NULL;
		IDispatch *pTemp = NULL;
		BOOL	bIsB2BCatch = FALSE;
		CString strTemp = _T("");
		CComDispatchDriver spElem;

		VARIANT index = { 0 };
		V_VT(&index) = VT_I4;
		V_I4(&index) = l;

		hr = pTables->item(COleVariant(l), index, &spElem);
		if (FAILED(hr) || spElem == NULL)
			continue;
		CComQIPtr<IHTMLElementCollection> pChild;

		Sleep(30);
		HRESULT hr = spElem.QueryInterface(&pElement);
		spElem.Release();

		CComBSTR bstr;
		CString strPage;
		pElement->get_className(&bstr);
		CString strClass(bstr);
		strClass.Trim();
		if (strClass.Find(g_mapElemCfg[sData.iIndex].strItemFlag1) == -1 /*&& strClass.Find(g_mapElemCfg[sData.iIndex].strItemFlag2 != -1)*/)
		{
			continue;
		}
		pElement->get_innerHTML(&bstr);
		strPage = bstr;

		CString strSnapUrl(_T(""));
		CString strSnapShotUrl(_T(""));
		if (!getSnapUrlAndSnapShotUrl(pElement, sData, strSnapUrl, strSnapShotUrl))
		{
			continue;
		}

		iRank++;
		BOOL bIsAce = FALSE;
		CatchInfo cInfo;
		DWORD dwWebId = 0;		//网站ID;
		bool bIsOfficialWebFlag = false;	//当前网站是否为官网网址;
		CString strUserfulDomain(_T(""));
		BOOL bFind = LinkUrlIsInOurSitelist(strSnapUrl, strSnapShotUrl, sData, cInfo.bIsOfficialFlag, dwWebId, strUserfulDomain, strPage, bIsOfficialWebFlag);

		if (bFind || cInfo.bIsOfficialFlag)
		{
			int iAllOpenState = -1;
			CString strAllErrorInfo(_T(""));
			int iResult = HasCompanyInHtml(strSnapUrl, sData, cInfo, dwWebId, lp, strSnapShotUrl, strAllRedirectUrl, strUserfulDomain, bIsOfficialWebFlag, iAllOpenState, strAllErrorInfo);
			changeOpenUrlState(iAllOpenState);
			setErrorWebId(strAllErrorInfo);
			if (iResult != FOUND_COMPANY || dwWebId <= 0)
			{
				if (iResult == NOT_FOUND_COMPANY || iResult == PAGE_NOT_FOUND)
				{
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("phonebaidu ：没有在目标网页中找到公司名"));
				}
				// 				else if (GET_WEBDATA_EXCEPTION == iResult)
				// 				{
				// 					setWebOpenFlag(true);
				// 				}

				if (!cInfo.bIsOfficialFlag)
				{
					continue;
				}
			}
			else
			{
				bIsB2BCatch = TRUE;
				iRankCnt++;
			}

			//如果找到递增1
			if (cInfo.bIsOfficialFlag)
			{
				iRankCnt++;
			}

			if (addPageTags(pElement, cInfo, dwWebId, sData, bIsAce))
			{
				l++;
				lDivCounts++;
			}

			if (vHeights.size() == 0)
			{
				pBdata = new BackDataInfo();
				ASSERT(pBdata != NULL);

				pBdata->iFlag = SEARCH_PHONEBAIDU;
				pBdata->iRank = sData.iCurPage * 10 + iRank;
				if (0 == sData.iCurPage)
				{
					if (pBdata->iRank > 10)
					{
						pBdata->iRank = 10;
					}
				}
				else if (1 == sData.iCurPage)
				{
					if (pBdata->iRank > 20)
					{
						pBdata->iRank = 20;
					}
				}
				else if (2 == sData.iCurPage)
				{
					if (pBdata->iRank > 30)
					{
						pBdata->iRank = 30;
					}
				}

				pBdata->strKeyWordName = sData.strKeyWordName;
				pBdata->strKey = sData.strKey;
				pBdata->strCompanyName = sData.strComany;
			}
			vHeights.push_back(sData.iCurPage * 10 + iRank);
			//如果抓到包含官网的，设置标记为1
			if (cInfo.bIsOfficialFlag)
			{
				pBdata->iOfficialFlag = 1;
				//Modified Date 2018/05/04
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);
				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			//如果抓到包含其他的,设置标记为1
			else if (bIsB2BCatch)
			{
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);

				//是否为2代网站
				if (dwWebId < 40000000 && dwWebId != 4000065)
					pBdata->iB2BFlag = 1;

				//是否为王牌网站
				if (cInfo.bIsAceFlag && bIsAce)
					pBdata->iAceFlag = 1;

				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			if (pBdata != NULL && 0 == pBdata->iKeywordType)
			{
				pBdata->iKeywordType = IsADWord(sData.iFlag, sData.iIndex, _pAllColls);
			}
		}
	}

	addBackData(pBdata, sData, iRetValue, strFileName, iRankCnt, strWebRecord, strAllRedirectUrl);
	return iRetValue;
}

void CWebDlg::addBackData(BackDataInfo* _pBdata, const KeyWordDataInfo& _sData, int& _iRetValue, CString& _strFileName, int _iRankCnt, CString _strWebRecord, CString _strAllRedirectUrl)
{
	if (_pBdata != NULL)
	{
		CString strCompany = _sData.strComany;
		_iRetValue = PAIMING_EXIST;
		GetImageFilePath(_sData.strKeyWordName, strCompany, _strFileName, _sData.iIndex);

		_pBdata->strPagePath = _strFileName;
		_pBdata->iRankCnt = _iRankCnt;
		_pBdata->strRecordInfo = _strWebRecord;
		_pBdata->strRedirectUrl = _strAllRedirectUrl;
		_pBdata->strRecordErrorInfo = getErrorWebId();

		if (E_ALLURLOPEN_STATE3 == getPageOpenState() || E_ALLURLOPEN_STATE5 == getPageOpenState())
		{
			//			_pBdata->strPagePath = _T("");
			_pBdata->iRank = 0;
			_pBdata->iRankCnt = -2;
			_iRetValue = PAGE_OFFICIAL_OPENFAILED;
		}
		m_vBackData.push_back(_pBdata);
	}
}

/*
@brief  在手机神马页面上抓取排名
@param  过滤标签内容
@param  sData  关键词相关数据
@param  strFileName  要保存的图片路径   输出
@return	0：有排名	1：无排名	2：打开网页错误
*/
int CWebDlg::GetPhoneShenMaPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *_pAllColls)
{
	int iRetValue = OPEN_PAGE_FAILED;
	int iRankCnt = 0;   //抓取到的排名数量
	std::vector<long> vHeights;  //返回高度，可能有多条数据
	CString strWebRecord = _T("");  //网站排名记录

	if (pTables == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("IHTMLElementCollection phoneShenMa is NULL"));
		return iRetValue;
	}

	CComQIPtr<IHTMLElement> pElement;
	CComQIPtr<IHTMLAnchorElement> pAnchor;
	BackDataInfo *pBdata = NULL;
	HRESULT hr;

	int iRank = 0;
	long lDivCounts = 0;
	pTables->get_length(&lDivCounts);
	CString strAllRedirectUrl(_T(""));

	for (long l = 0; l <= lDivCounts; l++)
	{
		IDispatch  *pDisp = NULL;
		IDispatch *pTemp = NULL;
		BOOL	bIsB2BCatch = FALSE;
		CString strTemp = _T("");
		CComDispatchDriver spElem;

		VARIANT index = { 0 };
		V_VT(&index) = VT_I4;
		V_I4(&index) = l;

		hr = pTables->item(COleVariant(l), index, &spElem);
		if (FAILED(hr) || spElem == NULL)
			continue;
		CComQIPtr<IHTMLElementCollection> pChild;

		Sleep(30);
		HRESULT hr = spElem.QueryInterface(&pElement);
		spElem.Release();

		CComBSTR bstr;
		CString strPage;
		pElement->get_className(&bstr);
		CString strClass(bstr);
		strClass.Trim();
		if (strClass.Find(g_mapElemCfg[sData.iIndex].strItemFlag1) == -1 /*&& strClass.Find(g_mapElemCfg[sData.iIndex].strItemFlag2 != -1)*/)
		{
			continue;
		}
		pElement->get_innerHTML(&bstr);
		strPage = bstr;

		CString strSnapUrl(_T(""));
		CString strSnapShotUrl(_T(""));
		if (!getSnapUrlAndSnapShotUrl(pElement, sData, strSnapUrl, strSnapShotUrl))
		{
			continue;
		}

		iRank++;
		BOOL bIsAce = FALSE;
		CatchInfo cInfo;
		DWORD dwWebId = 0;		//网站ID;
		bool bIsOfficialWebFlag = false;	//当前网站是否为官网网址;
		CString strUserfulDomain(_T(""));
		BOOL bFind = LinkUrlIsInOurSitelist(strSnapUrl, strSnapShotUrl, sData, cInfo.bIsOfficialFlag, dwWebId, strUserfulDomain, strPage, bIsOfficialWebFlag);

		if (bFind || cInfo.bIsOfficialFlag)
		{
			int iAllOpenState = -1;
			CString strAllErrorInfo(_T(""));
			int iResult = HasCompanyInHtml(strSnapUrl, sData, cInfo, dwWebId, lp, strSnapShotUrl, strAllRedirectUrl, strUserfulDomain, bIsOfficialWebFlag, iAllOpenState, strAllErrorInfo);
			changeOpenUrlState(iAllOpenState);
			setErrorWebId(strAllErrorInfo);
			if (iResult != FOUND_COMPANY || dwWebId <= 0)
			{
				if (iResult == NOT_FOUND_COMPANY || iResult == PAGE_NOT_FOUND)
				{
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("phoneShenMa ：没有在目标网页中找到公司名"));
				}
				// 				else if (GET_WEBDATA_EXCEPTION == iResult)
				// 				{
				// 					setWebOpenFlag(true);
				// 				}

				if (!cInfo.bIsOfficialFlag)
				{
					continue;
				}
			}
			else
			{
				bIsB2BCatch = TRUE;
				iRankCnt++;
			}

			//如果找到递增1
			if (cInfo.bIsOfficialFlag)
			{
				iRankCnt++;
			}

			if (addPageTags(pElement, cInfo, dwWebId, sData, bIsAce))
			{
				l++;
				lDivCounts++;
			}

			if (vHeights.size() == 0)
			{
				pBdata = new BackDataInfo();
				ASSERT(pBdata != NULL);

				pBdata->iFlag = SEARCH_PHONESHENMA;
				pBdata->iRank = sData.iCurPage * 10 + iRank;
				if (0 == sData.iCurPage)
				{
					if (pBdata->iRank > 10)
					{
						pBdata->iRank = 10;
					}
				}
				else if (1 == sData.iCurPage)
				{
					if (pBdata->iRank > 20)
					{
						pBdata->iRank = 20;
					}
				}
				else if (2 == sData.iCurPage)
				{
					if (pBdata->iRank > 30)
					{
						pBdata->iRank = 30;
					}
				}

				pBdata->strKeyWordName = sData.strKeyWordName;
				pBdata->strKey = sData.strKey;
				pBdata->strCompanyName = sData.strComany;
			}
			vHeights.push_back(sData.iCurPage * 10 + iRank);
			//如果抓到包含官网的，设置标记为1
			if (cInfo.bIsOfficialFlag)
			{
				pBdata->iOfficialFlag = 1;
				//Modified Date 2018/05/04
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);
				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			//如果抓到包含其他的,设置标记为1
			else if (bIsB2BCatch)
			{
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);

				//是否为2代网站
				if (dwWebId < 40000000 && dwWebId != 4000065)
					pBdata->iB2BFlag = 1;

				//是否为王牌网站
				if (cInfo.bIsAceFlag && bIsAce)
					pBdata->iAceFlag = 1;

				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			if (pBdata != NULL && 0 == pBdata->iKeywordType)
			{
				pBdata->iKeywordType = IsADWord(sData.iFlag, sData.iIndex, _pAllColls);
			}
		}
	}

	addBackData(pBdata, sData, iRetValue, strFileName, iRankCnt, strWebRecord, strAllRedirectUrl);
	return iRetValue;
}

/*
@brief  在手机搜狗页面上抓取排名
@param  过滤标签内容
@param  sData  关键词相关数据
@param  strFileName  要保存的图片路径   输出
@return	0：有排名	1：无排名	2：打开网页错误
*/
int CWebDlg::GetPhoneSougouPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *_pAllColls)
{
	int iRetValue = OPEN_PAGE_FAILED;
	int iRankCnt = 0;   //抓取到的排名数量
	std::vector<long> vHeights;  //返回高度，可能有多条数据
	CString strWebRecord = _T("");  //网站排名记录

	if (pTables == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("IHTMLElementCollection phoneSougou is NULL"));
		return iRetValue;
	}

	CComQIPtr<IHTMLElement> pElement;
	CComQIPtr<IHTMLAnchorElement> pAnchor;
	BackDataInfo *pBdata = NULL;
	HRESULT hr;

	int iRank = 0;
	long lDivCounts = 0;
	pTables->get_length(&lDivCounts);

	CString strAllRedirectUrl(_T(""));
	for (long l = 0; l <= lDivCounts; l++)
	{
		IDispatch  *pDisp = NULL;
		IDispatch *pTemp = NULL;
		BOOL	bIsB2BCatch = FALSE;
		CString strTemp;

		VARIANT index = { 0 };
		V_VT(&index) = VT_I4;
		V_I4(&index) = l;

		hr = pTables->item(COleVariant(l), index, &pDisp);
		if (FAILED(hr) || pDisp == NULL)
			continue;
		CComQIPtr<IHTMLElementCollection> pChild;

		Sleep(30);
		pElement = pDisp;
		pDisp->Release();

		CComBSTR bstr;
		CString strPage;
		pElement->get_innerHTML(&bstr);
		strPage = bstr;

		if (strPage.Compare(_T("")) == 0)
			continue;

		/*if (strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag1, 0) != -1)*/
		if (m_IntelligFind.FindResult(strPage, g_mapElemCfg[sData.iIndex].strItemFlag1, g_mapElemCfg[sData.iIndex].strUniversalFst, 0, 0))
		{
			continue;
		}

		pElement->get_children(&pTemp);
		pChild = pTemp;

		long childCount = 0;
		pChild->get_length(&childCount);

		if (childCount > 4)
			continue;

		//if (m_IntelligFind.FindResult(strPage, _T("class=\"abs\""), g_mapElemCfg[sData.iIndex].strItemFlag2,1))
		if (strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag2, 0) == -1 || (strPage.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag1) != -1))
			continue;


		CString strSnapUrl(_T(""));
		CString strSnapShotUrl(_T(""));
		if (!getSnapUrlAndSnapShotUrl(pElement, sData, strSnapUrl, strSnapShotUrl))
		{
			continue;
		}


		iRank++;
		CatchInfo cInfo;
		BOOL bIsAce = FALSE;
		DWORD dwWebId = 0;		//网站ID;
		bool bIsOfficialWebFlag = false;	//当前网站是否为官网网址;
		CString strUserfulDomain(_T(""));
		BOOL bFind = LinkUrlIsInOurSitelist(strSnapUrl, strSnapShotUrl, sData, cInfo.bIsOfficialFlag, dwWebId, strUserfulDomain, strPage, bIsOfficialWebFlag);

		if (bFind || cInfo.bIsOfficialFlag)
		{
			int iAllOpenState = -1;
			CString strAllErrorInfo(_T(""));
			int iResult = HasCompanyInHtml(strSnapUrl, sData, cInfo, dwWebId, lp, strSnapShotUrl, strAllRedirectUrl, strUserfulDomain, bIsOfficialWebFlag, iAllOpenState, strAllErrorInfo);
			changeOpenUrlState(iAllOpenState);
			setErrorWebId(strAllErrorInfo);
			if (iResult != FOUND_COMPANY || dwWebId <= 0)
			{
				if (iResult == NOT_FOUND_COMPANY || iResult == PAGE_NOT_FOUND)
				{
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("phoneSougou ：没有在目标网页中找到公司名"));
				}
				// 				else if (GET_WEBDATA_EXCEPTION == iResult)
				// 				{
				// 					setWebOpenFlag(true);
				// 				}

				if (!cInfo.bIsOfficialFlag)
				{
					continue;
				}
			}
			else
			{
				bIsB2BCatch = TRUE;
				iRankCnt++;
			}

			//如果找到递增1
			if (cInfo.bIsOfficialFlag)
			{
				iRankCnt++;
			}


			if (addPageTags(pElement, cInfo, dwWebId, sData, bIsAce))
			{
				l++;
				lDivCounts++;
			}

			if (vHeights.size() == 0)
			{
				pBdata = new BackDataInfo();
				ASSERT(pBdata != NULL);

				pBdata->iFlag = SEARCH_PHONESOGOU;
				pBdata->iRank = sData.iCurPage * 10 + iRank;
				if (0 == sData.iCurPage)
				{
					if (pBdata->iRank > 10)
					{
						pBdata->iRank = 10;
					}
				}
				else if (1 == sData.iCurPage)
				{
					if (pBdata->iRank > 20)
					{
						pBdata->iRank = 20;
					}
				}
				else if (2 == sData.iCurPage)
				{
					if (pBdata->iRank > 30)
					{
						pBdata->iRank = 30;
					}
				}

				pBdata->strKeyWordName = sData.strKeyWordName;
				pBdata->strKey = sData.strKey;
				pBdata->strCompanyName = sData.strComany;
			}
			vHeights.push_back(sData.iCurPage * 10 + iRank);
			//如果抓到包含官网的，设置标记为1
			if (cInfo.bIsOfficialFlag)
			{
				pBdata->iOfficialFlag = 1;
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);
				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			//如果抓到包含其他的,设置标记为1
			else if (bIsB2BCatch)
			{
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);

				//是否为2代网站
				if (dwWebId < 40000000 && dwWebId != 4000065)
					pBdata->iB2BFlag = 1;

				//是否为王牌网站
				if (cInfo.bIsAceFlag && bIsAce)
					pBdata->iAceFlag = 1;

				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			if (pBdata != NULL && 0 == pBdata->iKeywordType)
			{
				pBdata->iKeywordType = IsADWord(sData.iFlag, sData.iIndex, _pAllColls);
			}
		}
	}
	addBackData(pBdata, sData, iRetValue, strFileName, iRankCnt, strWebRecord, strAllRedirectUrl);
	return iRetValue;

}

/*
@brief  保存bmp图片
@param  hBitmap  bmp图片数据
@param  lpszFileName  文件路径
@return 保存成功返回true
*/
BOOL CWebDlg::SaveToFile(HBITMAP hBitmap, LPCTSTR lpszFileName)
{
	HDC hDC;
	//当前分辨率下每象素所占字节数    
	int iBits;
	//位图中每象素所占字节数    
	WORD wBitCount;
	//定义调色板大小， 位图中像素字节大小 ，位图文件大小 ， 写入文件字节数    
	DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//位图属性结构    
	BITMAP Bitmap;
	//位图文件头结构    
	BITMAPFILEHEADER bmfHdr;
	//位图信息头结构    
	BITMAPINFOHEADER bi;
	//指向位图信息头结构    
	LPBITMAPINFOHEADER lpbi;
	//定义文件，分配内存句柄，调色板句柄    
	HANDLE fh, hDib, hPal, hOldPal = NULL;

	//计算位图文件每个像素所占字节数    
	hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;
	GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;
	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;
	//为位图内容分配内存     
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;
	// 处理调色板      
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	// 获取该调色板下新的像素值    
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
		+dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);
	//恢复调色板    
	if (hOldPal)
	{
		SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}
	//创建位图文件      
	fh = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)
		return FALSE;
	// 设置位图文件头     
	bmfHdr.bfType = 0x4D42; // "BM"    
	dwDIBSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER)+dwPaletteSize;
	// 写入位图文件头    
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	// 写入位图文件其余内容    
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//清除    
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);
	return TRUE;
}

/*
@brief  根据关键词取得图片路径
@param  strTitle  关键词
@param  strFile  文件路径  输出
@param  strComp  公司名
@param  iIndex  搜索索引
@return 保存成功返回true
*/
BOOL CWebDlg::GetImageFilePath(const CString &strTitle, const CString &strComp, CString &strFile, int iIndex)
{
	TCHAR szFile[MAX_PATH] = { 0 };
	strFile.Empty();

	CString strNewTitle = strTitle;
	CString strNewComp = strComp;
	strNewComp.TrimLeft();
	strNewComp.TrimRight();
	ReplaceHtmlChar(strNewComp);
	ReplaceHtmlChar(strNewTitle);
	if (GetModuleFileName(NULL, szFile, MAX_PATH))
	{
		PathAppend(szFile, _T("..\\..\\"));
		//PathAppend(szFile, _T("..\\"));

		strFile = szFile;
		if (g_bIsDebug)
		{
			strFile.Append(_T("image\\DEBUG\\"));
		}
		else
		{
			if (g_bManualRefresh)
			{
				strFile.Append(_T("image\\keyword2\\"));
			}
			else
			{
				strFile.Append(_T("image\\keyword\\"));
			}
			strFile.Append(g_sKWPath);
			strFile.Append(_T("\\"));
		}
		strFile.Append(strNewComp);
		strFile.Append(_T("\\"));
		strFile.Append(strNewTitle);
		strFile.Append(_T("_"));
		strFile.Append(strNewComp);

		strFile.Append(g_mapElemCfg[iIndex].strHtmlName);

		return TRUE;
	}
	return FALSE;
}

void CWebDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 	if (TIMER_GET_HTML_STATUS == nIDEvent)
	// 	{
	// 		bool bBusy = m_HtmlView.GetBusy();
	// 		long lReadyState = m_HtmlView.GetReadyState();
	// 		if (!m_bBusy)
	// 		{
	// 			//超时
	// 			m_lHtmlStatus = lReadyState;
	// 			KillTimer(TIMER_GET_HTML_STATUS);
	// 			return;
	// 		}
	// 
	// 		if (!bBusy && lReadyState >= READYSTATE_COMPLETE)
	// 		{
	// 			//正常加载
	// 			m_bBusy = false;
	// 			m_lHtmlStatus = lReadyState;
	// 			KillTimer(TIMER_GET_HTML_STATUS);
	// 		}
	// 	}

	if (TIMER_GET_HTML_STATUS == nIDEvent)
	{
		m_lHtmlStatus = -1;
		m_iTimerCount++;
		bool bBusy = m_WebBrowser.get_Busy();
		long lReadyState = m_WebBrowser.get_ReadyState();

		if (!bBusy && lReadyState >= READYSTATE_COMPLETE)
		{
			//正常加载
			m_iTimerCount = 0;
			m_lHtmlStatus = lReadyState;
			KillTimer(TIMER_GET_HTML_STATUS);
		}
		else if (m_iTimerCount > TIMEOUTCOUTS)
		{
			//超时
			m_iTimerCount = 0;
			m_lHtmlStatus = lReadyState;
			KillTimer(TIMER_GET_HTML_STATUS);
		}

	}
	CDialog::OnTimer(nIDEvent);
}


void CWebDlg::OnBnClickedOk()
{
	CDialog::OnOK();
}

//取得元素的所有属性
void CWebDlg::GetAllAttributes(CComQIPtr<IHTMLElement> _pElement, std::map<_bstr_t, _bstr_t>&mapAttribs)
{
	CComDispatchDriver pACDisp;
	CComQIPtr<IHTMLAttributeCollection> pAttrColl;
	CComQIPtr<IHTMLDOMNode> pElemDN;
	CComQIPtr<IHTMLInputElement> qiInputBtnElem;
	CComQIPtr<IHTMLStyle> pStyle;

	LONG lACLength = 0;
	VARIANT vACIndex;
	VARIANT vValue;
	VARIANT_BOOL vbSpecified;
	HRESULT hr = 0;
	CString strElemMsg = _T("");

	/*参见MSDN中IHTMLAttributeCollection
	Use the following procedure to get an IHTMLAttributeCollection interface pointer for an element.

	1.	Call QueryInterface on the element interface to request an IHTMLDOMNode interface.
	2.	Call IHTMLDOMNode::attributes on the IHTMLDOMNode interface to get an IDispatch interface pointer for the element's IHTMLAttributeCollection interface.
	3.	Call QueryInterface on the IDispatch interface to request an IHTMLAttributeCollection interface.
	*/
	hr = _pElement->QueryInterface(IID_IHTMLDOMNode, (void**)&pElemDN);
	if (FAILED(hr) || !pElemDN)
		return;

	hr = pElemDN->get_attributes(&pACDisp);
	if (FAILED(hr) || !pACDisp)
		return;

	hr = pACDisp->QueryInterface(IID_IHTMLAttributeCollection, (void**)&pAttrColl);
	if (FAILED(hr) || !pAttrColl)
		return;

	pAttrColl->get_length(&lACLength);

	vACIndex.vt = VT_I4;
	for (int i = 0; i < lACLength; i++)
	{
		vACIndex.lVal = i;

		//CStdString strTmp3 = GetOuterHTML(pElement);

		CComDispatchDriver pItemDisp;
		CComQIPtr<IHTMLDOMAttribute> pItem;

		pAttrColl->item(&vACIndex, &pItemDisp);
		hr = pItemDisp->QueryInterface(IID_IHTMLDOMAttribute, (void**)&pItem);
		if (FAILED(hr) || !pItem)
			return;

		pItem->get_specified(&vbSpecified);
		// 		if (vbSpecified == VARIANT_TRUE)
		// 		{
		//仅取有效属性
		_bstr_t bstrName;
		pItem->get_nodeName(&bstrName.GetBSTR());
		pItem->get_nodeValue(&vValue);

		CString strTempName = (LPCTSTR)bstrName;
		strTempName.MakeLower();

		if (vValue.vt == VT_BSTR)
			mapAttribs[(_bstr_t)strTempName] = vValue.bstrVal;
		else if (vValue.vt == VT_I4)
		{
			CString strTmp = _T("");
			strTmp.Format(_T("%d"), vValue.intVal);
			mapAttribs[(_bstr_t)strTempName] = strTmp;
		}
		//}
	}

	//为INPUT 元素提取Value(环球经贸网 注册时行业选择)
	_pElement->QueryInterface(IID_IHTMLInputElement, (void**)&qiInputBtnElem);
	if (NULL != qiInputBtnElem)
	{
		_bstr_t bstrVal;
		qiInputBtnElem->get_value(&bstrVal.GetBSTR());
		//((IHTMLInputElement *)qiInputBtnElem)->Release();
		if (0 != CompareBSTR(bstrVal, _T("")))
			mapAttribs[_T("value")] = bstrVal;

		/*qiInputBtnElem->get_type(&bstrVal.GetBSTR()) ;
		if ( 0 != CompareBSTR(bstrVal, _T("")) )
		{
		mapAttribs[_T("type")] = bstrVal ;
		}*/
	}

	//读取style Display:none 属性 获取是否隐藏
	hr = _pElement->get_style(&pStyle);
	if (FAILED(hr) || !pStyle)
	{
		;
	}
	else
	{
		_bstr_t bstrVal;

		pStyle->get_display(&bstrVal.GetBSTR());
		/*_bstr_t bstrVal1;
		pStyle->get_cssText(&bstrVal1.GetBSTR()) ;*/
		if (0 != CompareBSTR(bstrVal, _T("")))
		{
			mapAttribs[_T("style")] = bstrVal;
		}
	}

	return;

//endFunc:
//	g_htmlLocate.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("GetAllAttributes失败，hr=0x%08x！"), hr);
}
bool CWebDlg::GetTargetElement(CComQIPtr<IHTMLElementCollection> _pAllColls, TagNode	 _TagNode, CComQIPtr<IHTMLElement>& _pIHTMLElement)
{
	CComPtr<IDispatch> pDispathTemp;
	CComQIPtr<IHTMLElementCollection> pTags;
	HRESULT hret;

	hret = _pAllColls->tags(COleVariant(_TagNode.sTag), &pDispathTemp);
	if (FAILED(hret))
	{
		return false;
	}

	pTags = pDispathTemp;

	bool bTagFind = false;
	long lDivCounts = 0;
	pTags->get_length(&lDivCounts);
	//遍历过滤后的标签集合，根据属性定位标签
	for (long l = 0; l <= lDivCounts; l++)
	{
		CComQIPtr<IHTMLElement> pElement;
		CComDispatchDriver spElem;
		VARIANT index = { 0 };
		V_VT(&index) = VT_I4;
		V_I4(&index) = l;
		hret = pTags->item(COleVariant(l), index, &spElem);
		if (FAILED(hret) || spElem == NULL)
			continue;
		CComQIPtr<IHTMLElementCollection> pChild;

		Sleep(5);
		HRESULT hr = spElem.QueryInterface(&pElement);
		spElem.Release();

		if (FAILED(hret) || pElement == NULL)
			continue;

		if (_T("outtext") == (CStdString)_TagNode.skey)
		{
			CComBSTR bstr;
			CString sAttriValue;
			pElement->get_innerText(&bstr);
			sAttriValue = bstr;
			if (sAttriValue == _TagNode.sValue)
			{
				//找到目标输入标签元素
				_pIHTMLElement = pElement;
				return true;
			}
		}
		else
		{
			std::map<_bstr_t, _bstr_t> mapAttribs;
			GetAllAttributes(pElement, mapAttribs);
			CStdString sAttriValue = (CStdString)mapAttribs[(LPCTSTR)_TagNode.skey];
			if (sAttriValue == (CStdString)_TagNode.sValue)
			{
				//找到目标输入标签元素
				_pIHTMLElement = pElement;
				return true;
			}
		}
	}
	return false;
}
//判断页面加载是否完成
bool CWebDlg::bHtmlLoadComplete()
{
	static int iInterval = 20;
	DWORD dwStartTime = GetTickCount();
	do
	{
		if (m_lHtmlStatus != -1)
		{
			break;
		}
		Sleep(iInterval);
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} while (GetTickCount() - dwStartTime < 10000);

	return FALSE;

// 	bool bReady = false;
// 	int iTimeleft = 0;
// 	do
// 	{
// 		//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据ccc..."));
// 		bReady = (BOOL)SendMessage(GETREDIRECTURLNEW, 0, NULL);
// 
// 		if (bReady || (iTimeleft > TIMEOUTCOUTS))
// 		{
// 			if (iTimeleft > TIMEOUTCOUTS)
// 			{
// 				//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("超时%d，当已加载完成处理,页面为：%s"), i + 1, sData.strUrl);
// 				SendMessage(GETREDIRECTURLNEW, 1, NULL);
// 				return false;
// 			}
// 			return true;
// 		}
// 
// 		iTimeleft++;
// 		Sleep(500);
// 	} while (1);
}

bool CWebDlg::WaitWithUIMsgDisp(int nWaitTime)
{
	static int iInterval = 20;
	DWORD dwStartTime = GetTickCount();
	do
	{
		Sleep(iInterval);
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} while (GetTickCount() - dwStartTime < nWaitTime);

	return FALSE;
}

LRESULT  CWebDlg::GetSearchDataNew(WPARAM wParam, LPARAM lParam)
{
	LRESULT dwResult = OPEN_PAGE_FAILED;
	HRESULT hret;
	int iPaiMingResult = OPEN_PAGE_FAILED;
	CComPtr<IHTMLDocument2> pDocument2;
	CComPtr<IHTMLElement> pElement;

	CComPtr<IDispatch> pDispath;
	CComPtr<IDispatch> pDispathTemp;

	CComQIPtr<IHTMLElementCollection> pAllColls;
	CComQIPtr<IHTMLElementCollection> pTags;

	psthreadParam pParam = (psthreadParam)lParam;

	KeyWordDataInfo  *pSdata = pParam->pData;
	long lMessageBack = -1;

	try
	{
		do
		{
			pDispath = m_WebBrowser.get_Document();

			if (pDispath == NULL)
			{
				break;
			}

			hret = pDispath->QueryInterface(IID_IHTMLDocument2, (void**)&pDocument2);
			if (FAILED(hret))
			{
				break;
			}
			//提取body标签中所有标签的集合
			hret = pDocument2->get_all(&pAllColls);
			if (FAILED(hret))
			{
				break;
			}

			//m_WebBrowser.Stop();
			CComQIPtr<IHTMLElement> pElementTemp = nullptr;
			//定位关键词输入框
			if (GetTargetElement(pAllColls, g_mapElemCfg[pSdata->iIndex].tagKWEdit, pElementTemp))
			{
				CString sAttributeName(_T(""));
				BSTR bsAttributeName = sAttributeName.AllocSysString();
				BSTR bsKeyWord = pSdata->strKeyWordName.AllocSysString();
				//CComQIPtr<IHTMLElement2> QiElem2 = pElementTemp;
				//QiElem2->focus();
				CComQIPtr<IHTMLElement2> pElement2;
				pElementTemp.QueryInterface(&pElement2);
				pElement2->focus();


// 				VARIANT vtValue;
// 				vtValue.bstrVal = pSdata->strKeyWordName.AllocSysString();
// 
// 				pElementTemp->setAttribute(bsAttributeName, vtValue);
				pElementTemp->put_innerText(bsKeyWord);
				SysFreeString(bsKeyWord);
				SysFreeString(bsAttributeName);

				WaitWithUIMsgDisp(100);
			}
			else
			{
				//定位输入框失败
				;
			}

// 			{
// 				//IHTMLDocument * pDoc = (IHTMLDocument *)GetHtmlDocument();
// 				CComQIPtr<IHTMLDocument3> pDoc3(pDocument2);
// 				CComQIPtr<IOleWindow>piOleWin(pDoc3);
// 				if (piOleWin)
// 				{
// 					HWND hWnd = NULL;
// 					//获取浏览器栏的窗口句柄
// 					if (SUCCEEDED(piOleWin->GetWindow(&hWnd)) && (hWnd) && (IsWindow(hWnd)))
// 					{
// 						//向浏览器窗口发送键盘消息
// 						::PostMessage(hWnd, WM_KEYDOWN, VK_RETURN, 0x001C0001);
// 						Sleep(10);
// 						::PostMessage(hWnd, WM_KEYUP, VK_RETURN, 0xC01C0001);
// 
// 						WaitWithUIMsgDisp(1000);
// 					}
// 					else
// 					{
// 						;
// 					}
// 				}
// 				else
// 				{
// 					;
// 				}
// 
// 			}
			//定位搜索按钮
			pElementTemp = nullptr;
			if (GetTargetElement(pAllColls, g_mapElemCfg[pSdata->iIndex].tagSearchBtn, pElementTemp))
			{
				CComQIPtr<IHTMLElement2> pElement2;
				pElementTemp.QueryInterface(&pElement2);
				pElement2->focus();
				HRESULT hr = pElementTemp->click();
				WaitWithUIMsgDisp(100);

			}
			else
			{
				//定位搜索按钮失败
				;
			}
//			SetTimer(TIMER_GET_HTML_STATUS, 500, NULL);

			for (int i = 0; i < g_mapElemCfg[pSdata->iIndex].iSearchPageNum; i++)
			{
				pSdata->iCurPage = i;

				BOOL bBusy;
				long lReadyState;
				int iTimeleft = 0;

				CString strPage(_T(""));
				CComPtr<IHTMLDocument2> _pDocument2;
				CComPtr<IDispatch> _pDispath;
				CComPtr<IHTMLElement> _pElement;
				CComQIPtr<IHTMLElementCollection> _pAllCollsTemp;
				do
				{
					_pDispath = m_WebBrowser.get_Document();
					if (_pDispath == NULL)
					{
						break;
					}

					hret = _pDispath->QueryInterface(IID_IHTMLDocument2, (void**)&_pDocument2);
					if (FAILED(hret))
					{
						break;
					}
					//提取body标签
					CComBSTR bstr;
					_pDocument2->get_title(&bstr);
					strPage = bstr;

					hret = _pDocument2->get_all(&_pAllCollsTemp);
					if (FAILED(hret))
					{
						break;
					}

					CComQIPtr<IHTMLElement> _pElementTemp = nullptr;
					if (-1 != strPage.Find(pSdata->strKeyWordName) && GetTargetElement(_pAllCollsTemp, g_mapElemCfg[pSdata->iIndex].tagMainBody, _pElementTemp))
					{
						WaitWithUIMsgDisp(100);
						break;
					}
					else if (iTimeleft > (TIMEOUTCOUTS * 3))
					{
						break;
					}
					iTimeleft++;
					WaitWithUIMsgDisp(200);
				} while (1);

// 				HANDLE ThreadHandle;
// 				ThreadHandle = CreateThread(NULL, NULL, CheckHtmlLoadCompleteThread, &m_WebBrowser, 0, 0);
// 				WaitForSingleObject(ThreadHandle, INFINITE);
// 				DWORD dExitCode = -1;
// 				GetExitCodeThread(ThreadHandle, &dExitCode);
// 				CloseHandle(ThreadHandle);
				//等待页面加载成功
// 				if (!bHtmlLoadComplete())
// 				{
// 					//页面加载失败
// 					break;
// 				}

				sthreadParam mpData;
				mpData.pData = pParam->pData;
				mpData.pPhotoDlg = (CWebDlg*)pParam->pPhotoDlg;
//				mpData.pElement = nullptr;
				//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("准备抓取网页数据，关键词:%s  搜索标志为：%d"), sData.strKeyWordName, sData.iFlag);
				lMessageBack = SendMessage(ANALYSISDATA, 0, (LPARAM)&mpData);

				//判断是否抓到排名
				if (NO_PAIMING == lMessageBack)
				{
					//是否需要打开下一页进行抓取
					if (g_mapElemCfg[pSdata->iIndex].iSearchPageNum > 1)
					{
						CComQIPtr<IHTMLElement> pElementTemp = nullptr;
						//定位下一页
						if (GetTargetElement(_pAllCollsTemp, g_mapElemCfg[pSdata->iIndex].tagNextPage, pElementTemp))
						{
							CComQIPtr<IHTMLElement2> pElement2;
							pElementTemp.QueryInterface(&pElement2);
							pElement2->focus();
							HRESULT hr = pElementTemp->click();
							WaitWithUIMsgDisp(2000);
						}
						else
						{
							//定位下一页失败
							g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("%s定位下一页失败"), g_mapElemCfg[pSdata->iIndex].sHomePage);
							break;;
						}
					}
//					pParam->pElement = pElementTemp;
// 					if (NULL != mpData.pElement)
// 					{
// 						CComQIPtr<IHTMLElement2> pElement2;
// 						mpData.pElement.QueryInterface(&pElement2);
// 						pElement2->focus();
// 						HRESULT hr = mpData.pElement->click();
// 						WaitWithUIMsgDisp(1000);
// //						SetTimer(TIMER_GET_HTML_STATUS, 500, NULL);
// 					}
// 					else
// 					{
// 						//定位下一页失败
// 						break;
// 					}
				}
				else
				{
					//已经抓到了排名，跳出循环，不抓取下一页
					break;
				}

				if (E_ALLURLOPEN_STATE3 == getPageOpenState() || E_ALLURLOPEN_STATE4 == getPageOpenState())
				{
					break;
				}
			}

		} while (0);
	}
	catch (...)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("处理html时，出现异常"));
	}

	return lMessageBack;
}
bool CWebDlg::CheckHtmlLoadComplete(bool _bFlag, KeyWordDataInfo* _pSdata, CComQIPtr<IHTMLElementCollection> _pAllColls)
{
	if (_bFlag)
	{
		if (_pSdata->iFlag == SEARCH_BAIDU)
		{
// 			<strong>
// 				<span class = "fk fk_cur">
// 					<i class = "c-icon c-icon-bear-p">< / i>
// 				< / span>
// 				<span class = "pc">3< / span>
// 			< / strong>
		} 
		else if (_pSdata->iFlag == SEARCH_360)
		{
		}
		else if (_pSdata->iFlag == SEARCH_SOGOU)
		{
		}
		else if (_pSdata->iFlag == SEARCH_BING)
		{
		}
		else if (_pSdata->iFlag == SEARCH_PHONEBAIDU)
		{
			;
		}
		else if (_pSdata->iFlag == SEARCH_PHONE360)
		{
			;
		}
		else if (_pSdata->iFlag == SEARCH_PHONESOGOU)
		{
			;
		}
		else if (_pSdata->iFlag == SEARCH_PHONESHENMA)
		{
			;
		}
		else
		{
		}
	}
}

DWORD WINAPI CWebDlg::CheckHtmlLoadCompleteThread(LPVOID lp)
{
	DWORD dReValue = -1;
	CoInitialize(NULL);
 	AFX_MANAGE_STATE(AfxGetStaticModuleState());
 	AfxEnableControlContainer();

	CWebBrowser2* p = (CWebBrowser2*)lp;

	BOOL bBusy;
	long lReadyState;
	int iTimeleft = 0;
	do
	{
		bBusy = p->get_Busy();
		lReadyState = p->get_ReadyState();

		if (!bBusy && lReadyState >= READYSTATE_COMPLETE)
		{
			dReValue = 1;
			break;;
		}
		else if (iTimeleft > TIMEOUTCOUTS)
		{
			dReValue = 0;
			break;;
		}
		iTimeleft++;
		Sleep(500);
	} while (1);

	CoUninitialize();
	return dReValue;

// 	CWebDlg *p = (CWebDlg*)lp;
// 	return p->CheckHtmlLoadComplete(lp);

}
bool CWebDlg::CheckHtmlLoadComplete(LPVOID lp)
{
	CWebDlg *p = (CWebDlg*)lp;

	BOOL bBusy;
	long lReadyState;
	int iTimeleft = 0;
	do
	{
		bBusy = p->m_WebBrowser.get_Busy();
		lReadyState = p->m_WebBrowser.get_ReadyState();

		if (!bBusy && lReadyState >= READYSTATE_COMPLETE)
		{
			return true;
		}
		else if (iTimeleft > TIMEOUTCOUTS)
		{
			return false;
		}
		iTimeleft++;
		Sleep(500);
	} while (1);
}


LRESULT CWebDlg::AnalysisHtmlData(WPARAM wParam, LPARAM lParam)
{
	LRESULT dwResult = OPEN_PAGE_FAILED;
	HRESULT hret;
	int iPaiMingResult = OPEN_PAGE_FAILED;
	CComPtr<IHTMLDocument2> pDocument2;
	CComPtr<IHTMLElement> pElement;

	CComPtr<IDispatch> pDispath;
	CComPtr<IDispatch> pDispathTemp;

	CComQIPtr<IHTMLElementCollection> pAllColls;
	CComQIPtr<IHTMLElementCollection> pTags;

	psthreadParam pParam = (psthreadParam)lParam;

	KeyWordDataInfo  *pSdata = pParam->pData;

	try
	{
		do
		{
			pDispath = m_WebBrowser.get_Document();

			if (pDispath == NULL)
			{
				break;
			}

			hret = pDispath->QueryInterface(IID_IHTMLDocument2, (void**)&pDocument2);
			if (FAILED(hret))
			{
				break;
			}

			hret = pDocument2->get_all(&pAllColls);
			if (FAILED(hret))
			{
				break;
			}

			hret = pAllColls->tags(COleVariant(g_mapElemCfg[pSdata->iIndex].strCollTags), &pDispathTemp);

			if (FAILED(hret))
			{
				break;
			}


// 			CComQIPtr<IHTMLElement> pElementTemp = nullptr;
// 			//定位下一页
// 			if (GetTargetElement(pAllColls, g_mapElemCfg[pSdata->iIndex].tagNextPage, pElementTemp))
// 			{
// 				pParam->pElement = pElementTemp;
// 			}

			pTags = pDispathTemp;
			std::vector<long> vHeight;
			CString strFileName(_T(""));

			//IsADWord(pSdata->iFlag, pSdata->iIndex, pAllColls);
			BOOL bCrash = FALSE;
			if (pSdata->iFlag == SEARCH_BAIDU)
			{
				iPaiMingResult = GetBaiduPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_PHONEBAIDU)
			{
				iPaiMingResult = GetPhoneBaiduPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_PHONESOGOU)
			{
				iPaiMingResult = GetPhoneSougouPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_PHONE360)
			{
				iPaiMingResult = GetPhone360Photo(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_SOGOU)
			{
				iPaiMingResult = GetSoGouPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_BING
				|| pSdata->iFlag == SEARCH_360
				|| pSdata->iFlag == SEARCH_YOUDAO)
			{
				iPaiMingResult = GetILStylePhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_WEIXIN)
			{
				iPaiMingResult = GetWeixinPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg);
			}
			else if (pSdata->iFlag == SEARCH_PHONESHENMA)
			{
				iPaiMingResult = GetPhoneShenMaPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}

			if (iPaiMingResult == NO_PAIMING || iPaiMingResult == OPEN_PAGE_FAILED)
			{
				int iResult = JudgeHtml(pAllColls, pSdata);

				if (iResult == OPEN_PAGE_NORMAL)
				{
					dwResult = NO_PAIMING;  //页面正常打开但无排名
				}
				else if (iResult == ENGINE_APPEAR_VERIFYCODE)
				{
					dwResult = ENGINE_APPEAR_VERIFYCODE;  //页面出现验证码
				}
				break;
			}
			else if (PAGE_OFFICIAL_OPENFAILED == iPaiMingResult)//有官网没打开
			{
				// 				if (!strFileName.IsEmpty())
				// 				{
				// 					ChangeButtonStyle(pAllColls, pSdata);
				// 					SaveToHtml(pAllColls, strFileName, pSdata);
				// 				}
				dwResult = PAGE_OFFICIAL_OPENFAILED;  //
				break;
			}
			if (_T("1") != g_mapElemCfg[pSdata->iIndex].sKWSearchMethod)
			{
				ChangeButtonStyle(pAllColls, pSdata);
			}

			dwResult = PAIMING_EXIST;
			SaveToHtml(pAllColls, strFileName, pSdata);


			//压力测试（打印出每一个有快照的搜索引擎页面）
			// 			CString strpathExist;
			// 			strpathExist.Format(_T("%s\\log\\%s\\%s_%s_%dExist_%s"), GetInstallPath(), pSdata->strComany, pSdata->strKeyWordName, pSdata->strComany, (pSdata->iCurPage + 1), g_mapElemCfg[pSdata->iIndex].strHtmlName);
			// 			SaveToHtml(pAllColls, strpathExist, pSdata);

		} while (0);
	}
	catch (...)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("处理html时，出现异常"));
	}


	return dwResult;
}

/*
@brief  取得百度谷歌快照
@param
@param
@return 0表示正常打开无快照  1表示有快照  2表示页面无法打开   3表示出现验证码

*/
LRESULT CWebDlg::GetSearchData(WPARAM wParam, LPARAM lParam)
{
	LRESULT dwResult = OPEN_PAGE_FAILED;
	HRESULT hret;
	int iPaiMingResult = OPEN_PAGE_FAILED;
	CComPtr<IHTMLDocument2> pDocument2;
	CComPtr<IHTMLElement> pElement;

	CComPtr<IDispatch> pDispath;
	CComPtr<IDispatch> pDispathTemp;

	CComQIPtr<IHTMLElementCollection> pAllColls;
	CComQIPtr<IHTMLElementCollection> pTags;

	psthreadParam pParam = (psthreadParam)lParam;

	KeyWordDataInfo  *pSdata = pParam->pData;

	try
	{
		do
		{
			pDispath = m_WebBrowser.get_Document();

			if (pDispath == NULL)
			{
				break;
			}

			hret = pDispath->QueryInterface(IID_IHTMLDocument2, (void**)&pDocument2);
			if (FAILED(hret))
			{
				break;
			}

			hret = pDocument2->get_all(&pAllColls);
			if (FAILED(hret))
			{
				break;
			}


			//临时调试
			//((_T("深圳市贵喜钣金有限公司") == pSdata->strComany || _T("东莞市吉诺塑胶制品有限公司") == pSdata->strComany) || _T("东莞市吉诺塑胶制品有限公司") == pSdata->strComany)
			// 			if (BADIDU_INDEX == pSdata->iIndex && (_T("深圳市贵喜钣金有限公司") == pSdata->strComany || _T("东莞市吉诺塑胶制品有限公司") == pSdata->strComany))
			// 			{
			// 				g_debugLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("保存html"));
			// 				CString strpath;
			// 				strpath.Format(_T("%s\\log\\bd1125%s_%d.htm"), GetInstallPath(), pSdata->strKeyWordName, (pSdata->iCurPage + 1));
			// 				SaveToHtml(pAllColls, strpath, pSdata);
			// 			}
			//压力测试（打印出每一个搜索引擎页面）

			// 			CString strFolderName;
			// 			strFolderName.Format(_T("%s\\log\\%s"), GetInstallPath(), pSdata->strComany);
			// 			if (-1 == GetFileAttributes(strFolderName))
			// 			{
			// 				SHCreateDirectoryEx(NULL, strFolderName, NULL);
			// 				strFolderName.Format(_T("%s\\log\\%s\\1"), GetInstallPath(), pSdata->strComany);
			// 				SHCreateDirectoryEx(NULL, strFolderName, NULL);
			// 				strFolderName.Format(_T("%s\\log\\%s\\2"), GetInstallPath(), pSdata->strComany);
			// 				SHCreateDirectoryEx(NULL, strFolderName, NULL);
			// 				strFolderName.Format(_T("%s\\log\\%s\\3"), GetInstallPath(), pSdata->strComany);
			// 				SHCreateDirectoryEx(NULL, strFolderName, NULL);
			// 			}
			// 			CString strpath;
			// 			strpath.Format(_T("%s\\log\\%s\\%s_%s_%d_%s"), GetInstallPath(), pSdata->strComany, pSdata->strKeyWordName, pSdata->strComany, (pSdata->iCurPage + 1), g_mapElemCfg[pSdata->iIndex].strHtmlName);
			// 			SaveToHtml(pAllColls, strpath, pSdata);
			//g_TaskRecord.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("Run*****保存原始html****公司名:%s 关键词:%s  搜索引擎:%s"), pSdata->strComany, pSdata->strKeyWordName, g_mapElemCfg[pSdata->iIndex].strHtmlName);


			hret = pAllColls->tags(COleVariant(g_mapElemCfg[pSdata->iIndex].strCollTags), &pDispathTemp);

			if (FAILED(hret))
			{
				break;
			}

			pTags = pDispathTemp;

			std::vector<long> vHeight;
			CString strFileName(_T(""));

			//IsADWord(pSdata->iFlag, pSdata->iIndex, pAllColls);
			BOOL bCrash = FALSE;
			if (pSdata->iFlag == SEARCH_BAIDU)
			{
				iPaiMingResult = GetBaiduPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_PHONEBAIDU)
			{
				iPaiMingResult = GetPhoneBaiduPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_PHONESOGOU)
			{
				iPaiMingResult = GetPhoneSougouPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_PHONE360)
			{
				iPaiMingResult = GetPhone360Photo(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_SOGOU)
			{
				iPaiMingResult = GetSoGouPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_BING
				|| pSdata->iFlag == SEARCH_360
				|| pSdata->iFlag == SEARCH_YOUDAO)
			{
				iPaiMingResult = GetILStylePhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}
			else if (pSdata->iFlag == SEARCH_WEIXIN)
			{
				iPaiMingResult = GetWeixinPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg);
			}
			else if (pSdata->iFlag == SEARCH_PHONESHENMA)
			{
				iPaiMingResult = GetPhoneShenMaPhoto(pTags, *pSdata, strFileName, bCrash, pParam->pPhotoDlg, pAllColls);
			}

			if (iPaiMingResult == NO_PAIMING || iPaiMingResult == OPEN_PAGE_FAILED)
			{
				int iResult = JudgeHtml(pAllColls, pSdata);

				if (iResult == OPEN_PAGE_NORMAL)
				{
					dwResult = NO_PAIMING;  //页面正常打开但无排名
				}
				else if (iResult == ENGINE_APPEAR_VERIFYCODE)
				{
					dwResult = ENGINE_APPEAR_VERIFYCODE;  //页面出现验证码
				}
				break;
			}
			else if (PAGE_OFFICIAL_OPENFAILED == iPaiMingResult)//有官网没打开
			{
				// 				if (!strFileName.IsEmpty())
				// 				{
				// 					ChangeButtonStyle(pAllColls, pSdata);
				// 					SaveToHtml(pAllColls, strFileName, pSdata);
				// 				}
				dwResult = PAGE_OFFICIAL_OPENFAILED;  //
				break;
			}
			ChangeButtonStyle(pAllColls, pSdata);

			dwResult = PAIMING_EXIST;
			SaveToHtml(pAllColls, strFileName, pSdata);
			
			
			//压力测试（打印出每一个有快照的搜索引擎页面）
			// 			CString strpathExist;
			// 			strpathExist.Format(_T("%s\\log\\%s\\%s_%s_%dExist_%s"), GetInstallPath(), pSdata->strComany, pSdata->strKeyWordName, pSdata->strComany, (pSdata->iCurPage + 1), g_mapElemCfg[pSdata->iIndex].strHtmlName);
			// 			SaveToHtml(pAllColls, strpathExist, pSdata);

		} while (0);
	}
	catch (...)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("处理html时，出现异常"));
	}


	return dwResult;
}

int CWebDlg::IsADWord(int _SearchEngineId, int _index, IHTMLElementCollection *pAllColls)
{
	//<item key="LocationAd" value="div;id;3001,4001,5001,6001,7001,8001" />
	vector<CString> vsAD;

	int iADWord = 0;
	if (pAllColls == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("IHTMLElementCollection is NULL, SearchEngineId = %d"), _SearchEngineId);
		return iADWord;
	}

	SplitCString(g_mapElemCfg[_index].strLocationAd, _T(";"), vsAD, false);
	if (vsAD.size() != 3)
	{
		return iADWord;
	}
	HRESULT hret;
	CComPtr<IDispatch> pDispathTemp;
	CComQIPtr<IHTMLElementCollection> pTags;
	hret = pAllColls->tags(COleVariant(vsAD[0]), &pDispathTemp);
	if (FAILED(hret) || NULL == pDispathTemp)
	{
		return iADWord;
	}
	pTags = pDispathTemp;
	if (SEARCH_BAIDU == _SearchEngineId)
	{
		std::vector<CString> vsADid;
		SplitCString(vsAD[2], _T(","), vsADid, false);
		CComQIPtr<IHTMLElement> pElement;
		HRESULT hr;
		std::vector<CString>::iterator it = vsADid.begin();
		while (it != vsADid.end())
		{
			CComDispatchDriver spElem;
			VARIANT index = { 0 };
			V_VT(&index) = VT_I4;
			V_I4(&index) = 0;

			VARIANT varID = { 0 };
			VariantInit(&varID);
			varID.vt = VT_BSTR;
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, *it);
			varID.bstrVal = (*it).AllocSysString();
			hr = pTags->item(varID, index, &spElem);
			SysFreeString(varID.bstrVal);
			it++;
			if (FAILED(hr) || spElem == NULL)
				continue;

			Sleep(30);
			HRESULT hr = spElem.QueryInterface(&pElement);
			spElem.Release();
			if (FAILED(hr) || pElement == NULL)
				continue;

			CComBSTR bstr;
			CString strPage;
			pElement->get_innerHTML(&bstr);
			strPage = bstr;
			if (strPage.Find(_T("广告")) != -1)
			{
				iADWord = 1;
				break;
			}
		}
	}
	else
	{
		CComQIPtr<IHTMLElement> pElement;
		HRESULT hr;
		long lDivCounts = 0;
		pTags->get_length(&lDivCounts);

		for (long l = 0; l <= lDivCounts; l++)
		{
			CComDispatchDriver spElem;
			VARIANT index = { 0 };
			V_VT(&index) = VT_I4;
			V_I4(&index) = l;
			hr = pTags->item(COleVariant(l), index, &spElem);
			if (FAILED(hr) || spElem == NULL)
				continue;
			CComQIPtr<IHTMLElementCollection> pChild;

			Sleep(30);
			HRESULT hr = spElem.QueryInterface(&pElement);
			spElem.Release();

			if (FAILED(hr) || pElement == NULL)
				continue;

			CComBSTR bstr;
			if (vsAD[1] == _T("id"))
			{
				pElement->get_id(&bstr);
			}
			else if (vsAD[1] == _T("class"))
			{
				pElement->get_className(&bstr);
			}
			else
			{
				continue;
			}

			CString strClass(bstr);
			strClass.Trim();
			if (strClass.Find(vsAD[2]) == -1)
			{
				continue;
			}
			pElement->get_innerHTML(&bstr);
			CString strPage = bstr;
			if (strPage.Find(_T("广告")) != -1)
			{
				iADWord = 1;
				break;
			}
		}
	}
	if (1 == iADWord)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("该关键词的快照页面含有广告"));
	}
	return iADWord;
}


/*
@brief  判断百度搜索的网址是否在   收录的网站
@param  strHtml  某个条目的html代码
@return TRUE表存在
*/
BOOL  CWebDlg::FindIsOwnWebInBaidu(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficial, DWORD& dwWebId, CString& sSiteName)
{
	int iPos1, iPos2;
	BOOL bReturn = FALSE;
	strHtml.MakeLower();

	iPos1 = iPos2 = 0;
	//预防网址中有加粗
	strHtml.Replace(_T("<b>"), _T(""));
	strHtml.Replace(_T("</b>"), _T(""));
	iPos1 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag1, iPos2);

	while (true)
	{
		Sleep(10);
		if (iPos1 != -1)
		{
			iPos2 = m_IntelligFind.GetFindPos(strHtml, g_mapElemCfg[sData.iIndex].strFindUrlFlag2, g_mapElemCfg[sData.iIndex].strFindUrlFlag3, iPos1);
			/*iPos1 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag2, iPos1);
			iPos2 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag3, iPos1);*/
			if (iPos2 != -1)
			{
				//截取网址  14表示<SPAN class=g>这个字符串长度
				CString strUrl = strHtml.Mid(iPos1 + 1, iPos2 - iPos1 - 1);

				strUrl = m_IntelligFind.GetFindData(strUrl, g_mapElemCfg[sData.iIndex].strUniversalFst
					, _T("..."), _T("&nbsp;"));
				/*
				int iPos3 = strUrl.Find(g_mapElemCfg[sData.iIndex].strUniversalFst);

				if (iPos3 == -1)
				iPos3 = strUrl.Find(_T("..."));

				if (iPos3 == -1)
				iPos3 = strUrl.Find(_T("&nbsp;"));

				if (iPos3 != -1)
				{
				strUrl = strUrl.Left(iPos3);
				}*/

				//遇到网址超长的直接先处理shenqijiu0724.cn.b2b16...
				if (strUrl.Find(_T(".cn.b2b")) != -1)
				{
					dwWebId = 40043478;
					bReturn = TRUE;
					return bReturn;
				}

				bOwnOfficial = GetMainDomain(strUrl, sData.vOfficialList);

				CStringA pChar = WideToChar(strUrl.GetString());

				bReturn = GetWebId(sData, dwWebId, pChar);
				if (strUrl.GetLength() > 0)
				{
					if (strUrl.GetAt(0) == '.' && strUrl.GetAt(strUrl.GetLength() - 1) == '.')
					{
						if (bReturn)
						{
							sSiteName = strUrl;
						}
						break;
					}
				}

			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}

		if (bReturn)
			break;


		iPos1 = strHtml.Find(_T("<span"), iPos2);
	}


	return bReturn;

}


/*
@brief  判断手机百度搜索的网址是否在   收录的网站
@param  strHtml  某个条目的html代码
@return TRUE表存在
add by zhoulin
*/
BOOL CWebDlg::FindIsOwnWebInPhoneBaidu(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName)
{
	int iPos1, iPos2, iPos3;
	BOOL bReturn = FALSE;
	strHtml.MakeLower();

	iPos1 = iPos2 = iPos3 = 0;
	strHtml.Replace(_T("<span>"), _T(""));
	strHtml.Replace(_T("</span>"), _T(""));
	iPos1 = strHtml.Find(_T("<div class=\"c-showurl\">"), iPos2);

	if (iPos1 == -1)
	{
		iPos1 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag1, iPos2);//第一次定位“c-showurl”
		if (-1 != iPos1)
		{
			iPos1 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag1, (iPos1 + 1));//第二次定位“c-showurl”
		}
	}

	if (iPos1 != -1)
	{
		iPos3 = iPos1 + 1;
		iPos2 = m_IntelligFind.GetFindPos(strHtml, g_mapElemCfg[sData.iIndex].strFindUrlFlag2, g_mapElemCfg[sData.iIndex].strFindUrlFlag3, iPos3);
		/*iPos3 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag2, iPos1 + 1);
		iPos2 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag3, iPos3);*/

		if (iPos2 != -1)
		{
			CString strUrl = strHtml.Mid(iPos3 + 1, iPos2 - iPos3 - 1);

			bOwnOfficia = GetMainDomain(strUrl, sData.vOfficialList);
			CStringA pChar = WideToChar(strUrl.GetString());

			bReturn = GetWebId(sData, dwWebId, pChar);
			if (bReturn)
			{
				sSiteName = strUrl;
			}
		}
	}

	return bReturn;
}

/*
@brief  判断手机搜狗搜索的网址是否在   收录的网站
@param  strHtml  某个条目的html代码
@return TRUE表存在
add by zhoulin
*/
BOOL CWebDlg::FindIsOwnWebInPhoneSougou(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName)
{
	int iPos1, iPos2, iPos3;
	BOOL bReturn = FALSE;
	strHtml.MakeLower();

	iPos1 = iPos2 = iPos3 = 0;
	/*iPos1 = strHtml.Find(_T("<span class=\"siteurl\">"), iPos2);*/

	/*if (iPos1 == -1)*/
	iPos1 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag1, iPos2);

	if (iPos1 != -1)
	{
		iPos2 = m_IntelligFind.GetFindPos(strHtml, g_mapElemCfg[sData.iIndex].strFindUrlFlag2, g_mapElemCfg[sData.iIndex].strFindUrlFlag3, iPos1);
		/*
		iPos1 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag2, iPos1);
		iPos2 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag3, iPos1);*/

		if (iPos2 != -1)
		{
			CString strUrl = strHtml.Mid(iPos1 + 1, iPos2 - iPos1 - 1);

			bOwnOfficia = GetMainDomain(strUrl, sData.vOfficialList);

			CStringA pChar = WideToChar(strUrl.GetString());

			bReturn = GetWebId(sData, dwWebId, pChar);
			if (bReturn)
			{
				sSiteName = strUrl;
			}
		}
	}

	return bReturn;
}

/*
@brief  判断360搜索的网址是否在   收录的网站
@param  strHtml  某个条目的html代码
@return TRUE表存在
add by zhoulin
*/
BOOL  CWebDlg::FindIsOwnWebIn360(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName)
{
	int iPos1, iPos2, iPos3;
	BOOL bReturn = FALSE;
	strHtml.MakeLower();

	//预防网址中有加粗
	strHtml.Replace(_T("<b>"), _T(""));
	strHtml.Replace(_T("</b>"), _T(""));
	iPos1 = iPos2 = iPos3 = 0;
	iPos1 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag1, iPos2);

	if (iPos1 != -1)
	{
		iPos2 = m_IntelligFind.GetFindPos(strHtml, g_mapElemCfg[sData.iIndex].strFindUrlFlag2, g_mapElemCfg[sData.iIndex].strFindUrlFlag3, iPos1);
		/*iPos1 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag2, iPos1);
		iPos2 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag3, iPos1);*/

		if (iPos2 != -1)
		{
			CString strUrl = strHtml.Mid(iPos1 + 1, iPos2 - iPos1 - 1);

			bOwnOfficia = GetMainDomain(strUrl, sData.vOfficialList);

			CStringA pChar = WideToChar(strUrl.GetString());

			bReturn = GetWebId(sData/*.strWebList*/, dwWebId, pChar);
			if (bReturn)
			{
				sSiteName = strUrl;
			}
		}
	}

	return bReturn;
}

/*
@brief  判断谷歌搜索的网址是否在   收录的网站
@param  strHtml  某个条目的html代码
@return TRUE表存在
*/
BOOL  CWebDlg::FindIsOwnWebInGoogle(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName)
{
	int iPos1, iPos2, iPos3;
	BOOL bReturn = FALSE;

	//预防网址中有加粗
	strHtml.Replace(_T("<b>"), _T(""));
	strHtml.Replace(_T("</b>"), _T(""));
	iPos1 = iPos2 = iPos3 = 0;

	//截取网址  14表示<SPAN class=g>这个字符串长度
	CString strUrl = GetUrlFormContent(strHtml, sData.iIndex);

	bOwnOfficia = GetMainDomain(strUrl, sData.vOfficialList);

	CStringA pChar = WideToChar(strUrl.GetString());

	bReturn = GetWebId(sData, dwWebId, pChar);
	if (bReturn)
	{
		sSiteName = strUrl;
	}
	return bReturn;

}

LRESULT CWebDlg::RedirectURL(WPARAM wParam, LPARAM lParam)
{
	//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("ENTER REDICURL"));
	BOOL bBusy;
	long lReadyState;
	KeyWordDataInfo *pData = (KeyWordDataInfo*)lParam;

	try
	{
		if (wParam == 0)
		{
			if (pData != NULL)
			{
				VARIANT vFlags;

				VariantInit(&vFlags);
				vFlags.vt = VT_I4;
				vFlags.lVal = navNoHistory | navNoReadFromCache | navNoWriteToCache;
				//Sleep(10);

				//pData->strUrl = _T("E:\\深圳红酒课程_深圳问萄文化发展有限公司_BAIDU.html");
				m_WebBrowser.Navigate(pData->strUrl, &vFlags, NULL, NULL, NULL);
				//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("Navigate...%s"), pData->strUrl);


				//VariantClear(&vFlags);

				return 1;
			}
		}
		else if (wParam == 1)
		{
			bBusy = m_WebBrowser.get_Busy();
			lReadyState = m_WebBrowser.get_ReadyState();

			if (!bBusy && lReadyState >= READYSTATE_COMPLETE)
			{
				return 1;
			}
		}
		else if (wParam == 2)
		{
			m_WebBrowser.Stop();
		}
	}
	catch (...)
	{
		return 1;  //出现异常，完成
	}



	return 0;
}

LRESULT CWebDlg::RedirectURLNew(WPARAM wParam, LPARAM lParam)
{
	BOOL bBusy;
	long lReadyState;

	try
	{
		if (wParam == 0)
		{
			bBusy = m_WebBrowser.get_Busy();
			lReadyState = m_WebBrowser.get_ReadyState();

			if (!bBusy && lReadyState >= READYSTATE_COMPLETE)
			{
				return 1;
			}
		}
		else if (wParam == 1)
		{
			m_WebBrowser.Stop();
		}
	}
	catch (...)
	{
		return 1;  //出现异常，完成
	}



	return 0;
}


/*
@brief  取得il标签条目高度，及反馈数据
@param  过滤标签内容
@param  sData  关键词相关数据
@param  strFileName  要保存的图片路径   输出
@return	0：有排名	1：无排名	2：打开网页错误
*/
int CWebDlg::GetILStylePhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *_pAllColls)
{
	int iRetValue = OPEN_PAGE_FAILED;
	int iRankCnt = 0;   //抓取到的排名数量
	std::vector<long> vHeights;  //返回高度，可能有多条数据
	CString strWebRecord = _T("");  //网站排名记录

	if (pLis == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("IHTMLElementCollection is NULL"));
		return iRetValue;
	}

	CComQIPtr<IHTMLElement> pElement;
	BackDataInfo *pBdata = NULL;
	HRESULT hr;

	int iRank = 0;
	long lLiCounts = 0;
	pLis->get_length(&lLiCounts);
	CString strAllRedirectUrl(_T(""));

	for (long l = 0; l <= lLiCounts; l++)
	{
		IDispatch  *pDisp = NULL;
		BOOL	bIsB2BCatch = FALSE;
		CString strTemp;
		CComDispatchDriver spElem;

		VARIANT index = { 0 };
		V_VT(&index) = VT_I4;
		V_I4(&index) = l;

		hr = pLis->item(COleVariant(l), index, &spElem);
		if (FAILED(hr) || spElem == NULL)
			continue;

		Sleep(30);
		HRESULT hr = spElem.QueryInterface(&pElement);
		spElem.Release();

		CComBSTR bstr;
		pElement->get_outerHTML(&bstr);
		CString strPage(bstr);
		strPage.MakeLower();

		if (strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag1) == -1)
			continue;


		bstr.~CComBSTR();
		pElement->get_innerHTML(&bstr);
		strPage = bstr;

		CString strSnapUrl(_T(""));
		CString strSnapShotUrl(_T(""));
		if (!getSnapUrlAndSnapShotUrl(pElement, sData, strSnapUrl, strSnapShotUrl))
		{
			continue;
		}
		int ipause = 0;

		iRank++;
		BOOL bIsAce = FALSE;
		CatchInfo cInfo;
		DWORD dwWebId = 0;		//网站ID;
		bool bIsOfficialWebFlag = false;	//当前网站是否为官网网址;
		CString strUserfulDomain(_T(""));
		BOOL bFind = LinkUrlIsInOurSitelist(strSnapUrl, strSnapShotUrl, sData, cInfo.bIsOfficialFlag, dwWebId, strUserfulDomain, strPage, bIsOfficialWebFlag);

		if (bFind || cInfo.bIsOfficialFlag)
		{
			int iAllOpenState = -1;
			CString strAllErrorInfo(_T(""));
			int iResult = HasCompanyInHtml(strSnapUrl, sData, cInfo, dwWebId, lp, strSnapShotUrl, strAllRedirectUrl, strUserfulDomain, bIsOfficialWebFlag, iAllOpenState, strAllErrorInfo);
			changeOpenUrlState(iAllOpenState);
			setErrorWebId(strAllErrorInfo);
			if (iResult != FOUND_COMPANY || dwWebId <= 0)  //剔除官网中包含查找的标记的情况
			{
				if (iResult == NOT_FOUND_COMPANY || iResult == PAGE_NOT_FOUND)
				{
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("%d ：没有在目标网页中找到公司名"), sData.iIndex);
				}
				// 				else if (GET_WEBDATA_EXCEPTION == iResult)
				// 				{
				// 					setWebOpenFlag(true);
				// 				}

				if (!cInfo.bIsOfficialFlag)
				{
					continue;
				}
			}
			else
			{
				bIsB2BCatch = TRUE;
				iRankCnt++;
			}

			//如果找到递增1
			if (cInfo.bIsOfficialFlag)
			{
				iRankCnt++;
			}

			addPageTags(pElement, cInfo, dwWebId, sData, bIsAce);

			if (vHeights.size() == 0)
			{
				pBdata = new BackDataInfo();
				ASSERT(pBdata != NULL);

				pBdata->iFlag = sData.iFlag;
				pBdata->iRank = sData.iCurPage * 10 + iRank;
				if (0 == sData.iCurPage)
				{
					if (pBdata->iRank > 10)
					{
						pBdata->iRank = 10;
					}
				}
				else if (1 == sData.iCurPage)
				{
					if (pBdata->iRank > 20)
					{
						pBdata->iRank = 20;
					}
				}
				else if (2 == sData.iCurPage)
				{
					if (pBdata->iRank > 30)
					{
						pBdata->iRank = 30;
					}
				}

				pBdata->strKeyWordName = sData.strKeyWordName;
				pBdata->strKey = sData.strKey;
				pBdata->strCompanyName = sData.strComany;
			}
			vHeights.push_back(sData.iCurPage * 10 + iRank);

			//如果抓到包含官网的，设置标记为1
			if (cInfo.bIsOfficialFlag)
			{
				pBdata->iOfficialFlag = 1;
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);
				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			//如果抓到包含其他的,设置标记为1
			else if (bIsB2BCatch)
			{
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);

				//是否为2代网站
				if (dwWebId < 40000000 && dwWebId != 4000065)
					pBdata->iB2BFlag = 1;

				//是否为王牌网站
				if (cInfo.bIsAceFlag && bIsAce)
					pBdata->iAceFlag = 1;

				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}

			}

			if (pBdata != NULL && 0 == pBdata->iKeywordType)
			{
				pBdata->iKeywordType = IsADWord(sData.iFlag, sData.iIndex, _pAllColls);
			}
		}
	}
	addBackData(pBdata, sData, iRetValue, strFileName, iRankCnt, strWebRecord, strAllRedirectUrl);
	return iRetValue;
}

/*
@brief  在有道页面上抓取排名
@param  过滤标签内容
@param  sData  关键词相关数据
@param  strFileName  要保存的图片路径   输出
@return	0：有排名	1：无排名	2：打开网页错误
*/
/*
int CWebDlg::GetYouDaoPhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult)
{
int iRetValue = OPEN_PAGE_FAILED;
int iRankCnt = 0;   //抓取到的排名数量
std::vector<long> vHeights;  //返回高度，可能有多条数据
CString strWebRecord = _T("");  //网站排名记录

if (pLis == NULL)
{
g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("IHTMLElementCollection YOUDAO is NULL"));
return iRetValue;
}

g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("YOUDAO抓取"));

CComQIPtr<IHTMLElement> pElement;
BackDataInfo *pBdata = NULL;
HRESULT hr;

int iRank = 0;
long lLiCounts = 0;
pLis->get_length(&lLiCounts);

for (long l = 0; l <= lLiCounts; l++)
{
IDispatch  *pDisp = NULL;
BOOL	bIsB2BCatch = FALSE;
CString strTemp;

VARIANT index = { 0 };
V_VT(&index) = VT_I4;
V_I4(&index) = l;

hr = pLis->item(COleVariant(l), index, &pDisp);
if (FAILED(hr) || pDisp == NULL)
continue;

Sleep(30);
pElement = pDisp;
pDisp->Release();

CComBSTR bstr;
pElement->get_innerHTML(&bstr);
CString strPage(bstr);

strPage.MakeLower();

if (strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag1) == -1)
{
continue;
}

if (strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag2) == -1)
{
continue;
}

iRank++;

BOOL bOwnOfficial = FALSE;   //包含官网
DWORD dwWebId = 0;		//网站ID;
BOOL bFind = FindIsOwnWebInGoogle(strPage, sData,bOwnOfficial,dwWebId);

if (bFind || bOwnOfficial)
{
CString strUrl = GetUrlFormContent(strPage,sData.iIndex);
if (strUrl.Find(_T("http:")) == -1)
strUrl.Insert(0, _T("http://"));
int iResult = HasCompanyInHtml(strUrl, sData, bOwnOfficial);
if (iResult != FOUND_COMPANY)
{
if (iResult == NOT_FOUND_COMPANY || iResult == PAGE_NOT_FOUND)
g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("YOUDAO ：没有在目标网页中找到公司名"));

if (!bOwnOfficial)
{
continue;
}
}
else
{
bIsB2BCatch = TRUE;
iRankCnt++;
}

//如果找到递增1
if (bOwnOfficial)
{
iRankCnt++;
}

//加红框
strPage.Insert(0, _T("<div style=\"padding: 5px; border: 2px solid rgb(223, 0, 1); border-image: none; margin-bottom: 5px; box-shadow: 1px 1px 3px 0px rgba(0,0,0,0.6);\">"));
strPage.Append(_T("</div>"));
hr = pElement->put_innerHTML((BSTR)strPage.GetString());

if (FAILED(hr))
{
g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("YOUDAO ：加红框失败"));
}

if (vHeights.size() == 0)
{
pBdata = new BackDataInfo();
ASSERT(pBdata != NULL);

pBdata->iFlag = SEARCH_YOUDAO;
pBdata->iRank = sData.iCurPage * 10 + iRank;
pBdata->strKeyWordName = sData.strKeyWordName;
pBdata->strKey = sData.strKey;
pBdata->strCompanyName = sData.strComany;
}
vHeights.push_back(sData.iCurPage * 10 + iRank);
//如果抓到包含官网的，设置标记为1
if (bOwnOfficial)
{
pBdata->iOfficialFlag = 1;
}
//如果抓到包含其他的,设置标记为1
if (bIsB2BCatch)
{
//记录下网站ID对应排名
strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);
if (dwWebId < 40000000)
pBdata->iB2BFlag = 1;
if (strWebRecord.GetLength() <= 0)
{
strWebRecord = strTemp;
}
else
{
strWebRecord += _T("(;2)") + strTemp;
}
}
//}
}
}

if (pBdata != NULL)
{
CString strCompany = sData.strComany;
iRetValue = PAIMING_EXIST;
GetImageFilePath(sData.strKeyWordName, strCompany, strFileName, SEARCH_YOUDAO);

pBdata->strPagePath = strFileName;
pBdata->iRankCnt = iRankCnt;
pBdata->strRecordInfo = strWebRecord;
m_vBackData.push_back(pBdata);
}

return iRetValue;
}*/


/*
@brief  根据url取得
@param  [in/out]strUrl  网址
@param  [in/out]vHostlist 官网地址
*/
BOOL CWebDlg::GetMainDomain(CString &strUrl, const std::vector<CString> &vHostlist)
{
	BOOL bResult = FALSE;
	int iPos1 = -1;
	int iPos2 = -1;
	CString strHostUrl;

	strHostUrl = strUrl;

	TCHAR  szUrl[8][10] = { _T(".com"), _T(".cn"), _T(".net"), _T(".cc"), _T(".org"), _T(".biz"), _T(".info"), _T(".mobi") };

	for (int i = 0; i< 8; i++)
	{
		iPos2 = strUrl.Find(szUrl[i]);

		if (iPos2 != -1)
		{
			if (i == 1) //避免有几个cn情况
			{
				if (strUrl.Find(szUrl[i], iPos2 + 1) != -1)
				{
					iPos2 = strUrl.Find(szUrl[i], iPos2 + 1);
				}
			}

			strUrl = strUrl.Left(iPos2);
			//怕一些特殊的网址，比如m.cncaid.net类型的
			if (strUrl.GetLength() <= 1
				|| strUrl.Right(3) == _T("www"))
				continue;

			//查看当前网址是否包含在官网中
			if (vHostlist.size() > 0)
			{
				for (int i = 0; i < vHostlist.size(); ++i)
				{
					CString strLow = vHostlist[i];
					strLow.MakeLower();
					iPos2 = strHostUrl.Find(strLow);
					if (iPos2 != -1)
					{
						g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("包含官网排名：%s"), vHostlist[i]);
						bResult = TRUE;
						break;
					}
				}

			}

			iPos1 = strUrl.ReverseFind(_T('.'));
			break;
		}
	}
	if (iPos1 != -1)
	{
		strUrl = strUrl.Right(strUrl.GetLength() - iPos1);
		strUrl.Append(_T("."));
	}

	return bResult;
}
/*
@brief  请求url 判断里面的html源码是否包含公司名
@param  strUrl  网址
@param  strCompay  公司名
@param  bOwnOfficial 是否包含官网
@返回值：-1：打开网页失败 0：未找到公司名 1：找到公司名 2：找不到网页（404）
*/
int CWebDlg::HasCompanyInHtml(CString &strUrl, const KeyWordDataInfo &sData, CatchInfo &sCatch, DWORD &dwWebID, LPVOID lp, CString& sSiteName, CString& _strAllRedirectUrl, CString& _strUserfulDomain, bool _bIsOfficialWebFlag, int& _iAllOpenState, CString& _strAllErrorWebId)
{
	CWebDlg *pPhotoDlg = (CWebDlg*)lp;
	int iResult = OPEN_PAGE_FAILED;
	int iErrorCode = OPEN_PAGE_FAILED;
	sCatch.bIsOfficialFlag = FALSE;
	CStringA cstrHtmlSrcA;
	bool bGetSrcSuccess = false;
	CString sRedirectUrl(_T(""));
	CStringA cstrHtmlSrcTem;
	CString strContentType = _T("");
	long iHttpStatusCode = 0;	//http返回的异常状态码（除200）

	//iResult = GetSrcByHttp(strUrl, cstrHtmlSrcTem, strContentType, sData, sRedirectUrl/*, true*/);
	iResult = GetSrcByLibcurl(strUrl, cstrHtmlSrcTem, strContentType, sData, sRedirectUrl, iHttpStatusCode, _bIsOfficialWebFlag);
	if (GET_WEBDATA_NORMAL == iResult)
	{
		bGetSrcSuccess = true;
		cstrHtmlSrcA = cstrHtmlSrcTem;
		CString strJsUrl;
		if (GetSingleUrl(sData, cstrHtmlSrcA, strJsUrl, sSiteName))
		{
			long iHttpStatusCodeTemp = iHttpStatusCode;
			//GetSrcByHttp(strJsUrl, cstrHtmlSrcTem, strContentType, sData, sRedirectUrl);
			GetSrcByLibcurl(strJsUrl, cstrHtmlSrcTem, strContentType, sData, sRedirectUrl, iHttpStatusCode, _bIsOfficialWebFlag);
			iHttpStatusCode = iHttpStatusCodeTemp;
			cstrHtmlSrcA += cstrHtmlSrcTem;
		}
	}

	if (bGetSrcSuccess)
	{
		// 		{
		// 			//压力测试用，保存数据到txt文件
		// 			CString strlogPath;
		// 			strlogPath.Format(_T("%s\\log\\%s\\1\\%s_%s_0_%s"), GetInstallPath(), sData.strComany, sData.strKeyWordName, _strUserfulDomain, g_mapElemCfg[sData.iIndex].strHtmlName);
		// 			//strlogPath.Replace(_T(".html"), _T(".txt"));
		// 			for (int i = 0; i < 100; i++)
		// 			{
		// 				if ((-1 != GetFileAttributes(strlogPath)))
		// 				{
		// 					strlogPath.Format(_T("%s\\log\\%s\\1\\%s_%s_%d_%s"), GetInstallPath(), sData.strComany, sData.strKeyWordName, _strUserfulDomain, i, g_mapElemCfg[sData.iIndex].strHtmlName);
		// 				}
		// 				else
		// 				{
		// 					break;
		// 				}
		// 			}
		// 			try
		// 			{
		// 				DeleteFile(strlogPath);
		// 				CFile file;
		// 				file.Open(strlogPath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate, NULL);
		// 				WORD unicode = 0xFEFF;  //
		// 				file.Write(&unicode, 2);  //
		// 				file.Write(cstrHtmlSrcA, cstrHtmlSrcA.GetLength()*sizeof(TCHAR));
		// 				//Write( const void* lpBuf, UINT nCount )  lpBuf是写入数据的Buf指针，nCount是Buf里需要写入文件的字节数
		// 				file.Close();
		// 			}
		// 			catch (CException* e)
		// 			{
		// 			}
		// 		}

		//		CString strContentType;
		CString strData;
		strData.Empty();

		//获取嵌入url数据，及检查是否包含官网（商铺）
		sCatch.bIsOfficialFlag = GetPageData(cstrHtmlSrcA, strContentType, strData, sData.vAllCompanys, sRedirectUrl, _strUserfulDomain, iErrorCode, iHttpStatusCode);

		// 		{
		// 			//压力测试用，保存数据到txt文件
		// 			CString strlogPath;
		// 			strlogPath.Format(_T("%s\\log\\%s\\3\\%s_%s_0_%s"), GetInstallPath(), sData.strComany, sData.strKeyWordName, _strUserfulDomain, g_mapElemCfg[sData.iIndex].strHtmlName);
		// 			//strlogPath.Replace(_T(".html"), _T(".txt"));
		// 			for (int i = 0; i < 100; i++)
		// 			{
		// 				if ((-1 != GetFileAttributes(strlogPath)))
		// 				{
		// 					strlogPath.Format(_T("%s\\log\\%s\\3\\%s_%s_%d_%s"), GetInstallPath(), sData.strComany, sData.strKeyWordName, _strUserfulDomain, i, g_mapElemCfg[sData.iIndex].strHtmlName);
		// 				}
		// 				else
		// 				{
		// 					break;
		// 				}
		// 			}
		// 			try
		// 			{
		// 				DeleteFile(strlogPath);
		// 				CFile file;
		// 				file.Open(strlogPath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate, NULL);
		// 				WORD unicode = 0xFEFF;  //
		// 				file.Write(&unicode, 2);  //
		// 				file.Write(strData, strData.GetLength()*sizeof(TCHAR));
		// 				//Write( const void* lpBuf, UINT nCount )  lpBuf是写入数据的Buf指针，nCount是Buf里需要写入文件的字节数
		// 				file.Close();
		// 			}
		// 			catch (CException* e)
		// 			{
		// 			}
		// 		}
		// 

		//iResult = CheckWebFlag(sData.vCompanys, strData, sData.iIndex, sData.vAllCompanys);
		iResult = CheckWebFlag(sData.vCompanys, strData, sData.iIndex, sData.vCompanysTag, sData.bOnlyRareWord);
		//判断是否抓取到快照标记，如果是王牌的的话获取ID;
		if (iResult == FOUND_COMPANY)
		{
			sCatch.bIsAceFlag = CheckAceWeb(strData, dwWebID, sData.iIndex);
		}
		//只有找不到公司名才去判断是否要遍历包含网站
		if (iResult == NOT_FOUND_COMPANY)
		{
			//检查嵌入网站问题
			InspectEmbedWeb(strData, strContentType, sData, iResult, sRedirectUrl, _strUserfulDomain, iErrorCode, iHttpStatusCode);
		}
	}

	if (FOUND_COMPANY == iResult || sCatch.bIsOfficialFlag)
	{
		//添加需要返回的网址链接
		GetRedirectUrl(sData, strUrl, _strUserfulDomain, _strAllRedirectUrl, sRedirectUrl);
	}
	if (PAGE_NOT_FOUND == iResult
		|| GET_WEBDATA_EXCEPTION == iResult
		|| PAGE_FUNTION_ERROR == iResult
		|| PAGE_NOT_FOUND == iErrorCode
		|| GET_WEBDATA_EXCEPTION == iErrorCode)
	{
		addErrorWebId(_strAllErrorWebId, dwWebID, iHttpStatusCode);
	}
	if (FOUND_COMPANY == iResult || sCatch.bIsOfficialFlag)
	{
		if (_bIsOfficialWebFlag)
		{
			_iAllOpenState = E_CURRENTURLOPEN_STATE0;
		}
		else
		{
			_iAllOpenState = E_CURRENTURLOPEN_STATE1;
		}
	}
	else if (GET_WEBDATA_NORMAL != iResult && !bGetSrcSuccess)
	{
		if (_bIsOfficialWebFlag)
		{
			_iAllOpenState = E_CURRENTURLOPEN_STATE2;
		}
		else
		{
			_iAllOpenState = E_CURRENTURLOPEN_STATE3;
		}
	}
	else if (PAGE_NOT_FOUND == iErrorCode || GET_WEBDATA_EXCEPTION == iErrorCode)
	{
		if (_bIsOfficialWebFlag)
		{
			_iAllOpenState = E_CURRENTURLOPEN_STATE2;
		}
		else
		{
			_iAllOpenState = E_CURRENTURLOPEN_STATE3;
		}
	}

	return iResult;
}
void CWebDlg::addErrorWebId(CString& strAllErrorWebId, DWORD dwWebID, long _iHttpStatusCode)
{
	CString sErrorInfo(_T(""));
	sErrorInfo.Format(_T("%d,%d"), dwWebID, _iHttpStatusCode);
	strAllErrorWebId = sErrorInfo;

	//	strAllErrorWebId += sErrorInfo;
}
void CWebDlg::changeOpenUrlState(int _iCurrentState)
{
	if (E_ALLURLOPEN_STATE0 == m_iPageOpenState)
	{
		if (E_CURRENTURLOPEN_STATE0 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE1;
		}
		else if (E_CURRENTURLOPEN_STATE1 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE2;
		}
		else if (E_CURRENTURLOPEN_STATE2 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE5;
		}
		else if (E_CURRENTURLOPEN_STATE3 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE4;
		}
	}
	else if (E_ALLURLOPEN_STATE2 == m_iPageOpenState)
	{
		if (E_CURRENTURLOPEN_STATE0 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE1;
		}
		else if (E_CURRENTURLOPEN_STATE2 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE3;
		}
	}
	else if (E_ALLURLOPEN_STATE3 == m_iPageOpenState)
	{
		if (E_CURRENTURLOPEN_STATE0 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE1;
		}
	}
	else if (E_ALLURLOPEN_STATE4 == m_iPageOpenState)
	{
		if (E_CURRENTURLOPEN_STATE0 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE1;
		}
		else if (E_CURRENTURLOPEN_STATE1 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE2;
		}
		else if (E_CURRENTURLOPEN_STATE2 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE5;
		}
	}
	else if (E_ALLURLOPEN_STATE5 == m_iPageOpenState)
	{
		if (E_CURRENTURLOPEN_STATE0 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE1;
		}
		else if (E_CURRENTURLOPEN_STATE1 == _iCurrentState)
		{
			m_iPageOpenState = E_ALLURLOPEN_STATE3;
		}
	}
}

//对于打开目标页(官网内页)，结果产生了跳转的，无法抓到指纹，直接视为王牌网站（需要在排除 目标pc站/目标wap站/搜索引擎站/转码站之外的站）
bool CWebDlg::compareInsideAndOutSiteUrl(const CString& strUserfulDomain, const CString& _sRedirectUrl, bool _bIsOfficialWebFlag)
{
	//cutDomain(strDomain, strUsefulDomain)

	//搜索引擎站/转码站
	const int iCount = 5;
	TCHAR  sEngineDomain[iCount][15] =
	{
		_T("baidu.com"), _T("haosou.com"), _T("sogou.com"), _T("bing.com"), _T("so.com")
	};

	if (!_bIsOfficialWebFlag || strUserfulDomain.IsEmpty() || _sRedirectUrl.IsEmpty())
	{
		return false;
	}
	else
	{
		if (-1 != _sRedirectUrl.Find(strUserfulDomain))
		{
			//快照网址与链接进去后的内页网址一致，排除
			return false;
		}
		else
		{
			for (int i = 0; i < iCount; i++)
			{
				if (-1 != _sRedirectUrl.Find(sEngineDomain[i]))
				{
					//搜索引擎网址，排除
					return false;
				}
			}
		}
	}
	return true;
}

void CWebDlg::addRedirectUrl(CString &strRedirectUrl, CString _sRedirectUrl)
{
	if (!_sRedirectUrl.IsEmpty())
	{
		if (strRedirectUrl.IsEmpty())
		{
			strRedirectUrl += _sRedirectUrl;
		}
		else
		{
			strRedirectUrl += _T("(;a)");
			strRedirectUrl += _sRedirectUrl;
		}
	}
}
bool CWebDlg::GetSingleUrl(const KeyWordDataInfo &sData, const CStringA &_cstrHtmlSrcA, CString& _strJsUrl, const CString &sSiteName)
{
	//CString strJsUrl = _T("http://wsdetail.b2b.hc360.com/XssFilter?callback=jQuery17106749556216588805_1504752957807&bcid=411225166&_=1504752957856");
	//http://wsdetail.b2b.hc360.com/XssFilter?bcid=411225166
	//var supplyBcId = "411225166";
	SpecialUrlFlag arrUrlFlag[] =
	{
		{ _T(".hc360."), "hc360.com/supplyself/", ".html", _T("http://wsdetail.b2b.hc360.com/XssFilter?bcid=") },
	};
	bool bEngFlag = false;
	try
	{
		int iArrID = 0;
		if (SEARCH_BAIDU == sData.iFlag)
		{
			iArrID = 0;
			bEngFlag = true;
		}
		else if (SEARCH_360 == sData.iFlag)
		{
			iArrID = 1;
			bEngFlag = true;
		}
		else if (SEARCH_SOGOU == sData.iFlag)
		{
			iArrID = 2;
			bEngFlag = true;
		}
		else if (SEARCH_BING == sData.iFlag)
		{
			iArrID = 3;
			bEngFlag = true;
		}
		else if (SEARCH_YOUDAO == sData.iFlag)
		{
			iArrID = 4;
			bEngFlag = true;
		}
		else if (SEARCH_PHONEBAIDU == sData.iFlag)
		{
			iArrID = 5;
		}
		else if (SEARCH_PHONESOGOU == sData.iFlag)
		{
			iArrID = 6;
		}
		else if (SEARCH_PHONE360 == sData.iFlag)
		{
			iArrID = 7;
		}
		else if (SEARCH_WEIXIN == sData.iFlag)
		{
			iArrID = 8;
		}
		else if (SEARCH_PHONESHENMA == sData.iFlag)
		{
			iArrID = 9;
		}

		if (bEngFlag)
		{
			int iArrLength = sizeof(arrUrlFlag) / sizeof(arrUrlFlag[0]);
			for (int i = 0; i < iArrLength; i++)
			{
				CString strUrl = arrUrlFlag[i].strSiteName;
				// 				vector<CStringA>::iterator itor;
				// 				//for (auto str : g_vSingleUrl[iArrID].vEngUrl)
				// 				for (itor = g_vSingleUrl[iArrID].vEngUrl.begin(); itor != g_vSingleUrl[iArrID].vEngUrl.end(); itor++)
				// 				{
				//if (strUrl == sSiteName)//匹配网站名
				if (-1 != sSiteName.Find(strUrl))//匹配网站名
				{
					int iFlag1Pos = _cstrHtmlSrcA.Find(arrUrlFlag[i].strMarkerFlag1);
					if (-1 != iFlag1Pos)
					{
						int iFlag2Pos = _cstrHtmlSrcA.Find(arrUrlFlag[i].strMarkerFlag2, iFlag1Pos);
						if (-1 != iFlag2Pos)
						{
							int iStartPos = iFlag1Pos + arrUrlFlag[i].strMarkerFlag1.GetLength();
							int iStrLen = iFlag2Pos - iStartPos;
							CStringA strKeyUrlA = _cstrHtmlSrcA.Mid(iStartPos, iStrLen);
							CString strKeyUrl = CStrA2CStrT(strKeyUrlA);
							CString strUrl;
							strUrl.Format(_T("%s%s"), arrUrlFlag[i].strJsUrl, strKeyUrl);
							_strJsUrl = strUrl;

							//add by pj
							if (SEARCH_BAIDU == sData.iFlag)
							{
								g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("_strJsUrl = %s\r\n"), _strJsUrl);
							}
							//itor = g_vSingleUrl[iArrID].vEngUrl.erase(itor);
							return true;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				//				}
			}
		}
		else
		{
			return false;
		}
	}
	catch (CException* e)
	{
		return false;
	}
	return false;
}
void CWebDlg::GetRedirectUrl(const KeyWordDataInfo &sData, CString& _strUrl, CString& _strRedirectUrl)
{
	if (_strUrl.IsEmpty())
	{
		return;
	}
	else
	{
		CString strServer;
		CString strObject;
		DWORD dwServiceType;
		INTERNET_PORT nPort;
		AfxParseURL(_strUrl, dwServiceType, strServer, strObject, nPort);
		if (AFX_INET_SERVICE_HTTP != dwServiceType && AFX_INET_SERVICE_HTTPS != dwServiceType)
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("获取重定向Url,解析目标网址:%s失败"), _strUrl.GetString());
			return;
		}

		if (g_iDalayForSite != -1)
		{
			Sleep(g_iDalayForSite);
		}

		//bool bRequestState = false;
		for (int i = 0; i < IRETRYTIMES; i++)
		{
			CInternetSession m_session(IE_AGENT);
			CHttpFile *pFile = NULL;
			CHttpConnection *pConnection = NULL;
			CString strData;
			strData.Empty();

			try
			{
				//原来为5秒,现在改为10秒
				m_session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 10 * 1000);
				m_session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 10 * 1000);
				m_session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 10 * 1000);
				m_session.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 10 * 1000);
				m_session.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 10 * 1000);
				//m_session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);          // 1次重试

				pConnection = m_session.GetHttpConnection(strServer, nPort);

				if (pConnection == NULL)
				{
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("获取重定向Url,打开目标网址:%s连接失败"), _strUrl.GetString());
					return;
				}

				pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject.GetString(), 0, 1, 0, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_AUTO_REDIRECT);
				if (pFile != NULL)
				{
					DWORD dwRet = 0;
					//原来为5秒,现在改为10秒
					pFile->SetOption(INTERNET_OPTION_SEND_TIMEOUT, 10 * 1000, 0);
					pFile->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 10 * 1000, 0);

					pFile->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 10 * 1000, 0);
					pFile->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 1 * 1000, 0);


					pFile->AddRequestHeaders(_T("Accept: text/html"));
					pFile->AddRequestHeaders(_T("Accept-Language: zh-CN"));
					pFile->AddRequestHeaders(_T("Content-Type:text/html; charset=utf-8"));  //统一用utf-8防止乱码
					if (SEARCH_PHONEBAIDU == sData.iFlag || SEARCH_PHONE360 == sData.iFlag || SEARCH_PHONESOGOU == sData.iFlag)
					{
						pFile->AddRequestHeaders(_T("User-Agent: Mozilla/5.0 (Linux; U; Android 2.2; en-us; Nexus One Build/FRF91) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1"));
					}

					pFile->SendRequest();
					TCHAR tszLocation[1024 * 2] = { 0 };
					DWORD dwBufSize = 1024 * 2 * sizeof(TCHAR);

					pFile->QueryInfo(HTTP_QUERY_LOCATION, tszLocation, &dwBufSize);

					_strRedirectUrl.Format(_T("%s"), tszLocation);
					if (_strRedirectUrl.IsEmpty())
					{
						_strRedirectUrl = _strUrl;
					}

					pFile->Close();
					delete pFile;
				}

				pConnection->Close();
				delete pConnection;

				m_session.Close();

				//bRequestState = true;
				break;
			}
			catch (...)
			{
				//60无特殊含义，仅代表url长度，之所以定为60，是经验值
				if ((IRETRYTIMES - 1) == i)
				{
					if (_strUrl.GetLength() < 60)
					{
						_strRedirectUrl = _strUrl;
					}
					else
					{
						_strRedirectUrl = _T("error");
					}
					return;
				}
				else
				{
					if (pFile != NULL)
					{
						pFile->Close();
						delete pFile;
						pConnection->Close();
						delete pConnection;
						m_session.Close();
						g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("获取重定向_strUrl,请求读取数据异常,错误码：%d，目标网址:%s"), GetLastError(), _strUrl.GetString());
						Sleep(200);
					}
				}
			}
		}
	}
}

void CWebDlg::GetRedirectUrl(const KeyWordDataInfo &_sData, CString &strUrl, CString _strUserfulDomain, CString &strRedirectUrl, CString _sRedirectUrl)
{
#if 1
	//g_debugLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("重定向url:开始_strUrl = %s, _sRedirectUrl = %s, strRedirectUrl = %s"), strUrl, _sRedirectUrl, strRedirectUrl);
	//strUrl = _T("http://m.sogou.com/web/uID=haHM1H0_8WM7lwCe/v=5/type=1/sp=1/ct=180122130742/keyword=%E6%A2%AF%E5%BD%A2%E6%A2%AF%E5%AD%90%E7%AD%8B%E5%8E%82%E5%AE%B6/id=a9f1d228-b990-4ff0-8160-ef71f2583a93/sec=bQ-G0lkbNawIr9ZlZeEPkw../dp=1/vr=30000909/tc?userGroupId=1&dp=1&key=%E6%A2%AF%E5%BD%A2%E6%A2%AF%E5%AD%90%E7%AD%8B%E5%8E%82%E5%AE%B6&pno=1&g_ut=3&is_per=0&pg=webz&clk=7&url=http%3A%2F%2Fm.ic98.com%2Fsupply%2F1419%2F14195600.html&f=0&vrid=30000909&linkid=0&wml=3&w=1281&needType=M3uB-LO8tQA.&adPattern=ID_Y8zDND2XhhlfnvIdmgXNfATFzZk_DRGTW7qdDp6aHkvHQ7mc7yXjTlnZMvBT_MR3ea0Or9wk.&ftype=readermodel");
	if (_sRedirectUrl.IsEmpty())
	{
		if (!_strUserfulDomain.IsEmpty() && (-1 != strUrl.Find(_strUserfulDomain))
			&& (S360_INDEX == _sData.iIndex || SOGOU_INDEX == _sData.iIndex || BING_INDEX == _sData.iIndex))
		{
			//目前 360、搜狗、必应 会进入该条件
			_sRedirectUrl = strUrl;
		}
		else
		{
			CString strServer;
			CString strObject;
			DWORD dwServiceType;
			INTERNET_PORT nPort;
			if (g_bIsDebug)
			{
				strUrl = g_strDebugUrl;
				CString strMsg = _T("开始调试单个Url: ") + strUrl;
				AfxMessageBox(strMsg);
			}
			if (strUrl.Compare(_T("")) == 0)
			{
				g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("获取重定向Url,解析目标网址:%s为空"), strUrl.GetString());
				return;
			}
			//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("获取重定向Url,解析的目标网址：%s"), strUrl.GetString());

			AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort);
			if (AFX_INET_SERVICE_HTTP != dwServiceType && AFX_INET_SERVICE_HTTPS != dwServiceType)
			{
				g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("获取重定向Url,解析目标网址:%s失败"), strUrl.GetString());
				return;
			}

			if (g_iDalayForSite != -1)
			{
				Sleep(g_iDalayForSite);
			}

			bool bHttpRequest = false;
			bool bRequestState = false;
			for (int i = 0; i < IRETRYTIMES; i++)
			{
				CInternetSession m_session(IE_AGENT);
				CHttpFile *pFile = NULL;
				CHttpConnection *pConnection = NULL;
				CString strData;
				strData.Empty();

				try
				{
					//原来为5秒,现在改为10秒
					m_session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 20 * 1000);
					m_session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 20 * 1000);
					m_session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 20 * 1000);
					m_session.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 20 * 1000);
					m_session.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 20 * 1000);
					//m_session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);          // 1次重试

					pConnection = m_session.GetHttpConnection(strServer, nPort);

					if (pConnection == NULL)
					{
						g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("获取重定向Url,打开目标网址:%s连接失败"), strUrl.GetString());
						m_session.Close();
						continue;
					}

					//先用head方法请求，head方法不支持，再用Get方法请求
					if (!bHttpRequest)
					{
						pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_HEAD/*HTTP_VERB_GET*/, strObject.GetString(), 0, 1, 0, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_AUTO_REDIRECT);
					}
					else
					{
						pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject.GetString(), 0, 1, 0, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_AUTO_REDIRECT);
					}
					if (pFile != NULL)
					{
						DWORD dwRet = 0;
						//原来为5秒,现在改为10秒
						pFile->SetOption(INTERNET_OPTION_SEND_TIMEOUT, 20 * 1000, 0);
						pFile->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 20 * 1000, 0);

						pFile->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 20 * 1000, 0);
						pFile->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 20 * 1000, 0);


						pFile->AddRequestHeaders(_T("Accept: text/html"));
						pFile->AddRequestHeaders(_T("Accept-Language: zh-CN"));
						pFile->AddRequestHeaders(_T("Content-Type:text/html; charset=utf-8"));  //统一用utf-8防止乱码

						if (SEARCH_PHONEBAIDU == _sData.iFlag || SEARCH_PHONE360 == _sData.iFlag || SEARCH_PHONESOGOU == _sData.iFlag)
						{
							pFile->AddRequestHeaders(_T("User-Agent: Mozilla/5.0 (Linux; U; Android 2.2; en-us; Nexus One Build/FRF91) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1"));
						}
						pFile->SendRequest();
						pFile->QueryInfoStatusCode(dwRet);
						if (((dwRet >= HTTP_STATUS_OK) && (dwRet <= HTTP_STATUS_PARTIAL_CONTENT)) || ((dwRet >= HTTP_STATUS_AMBIGUOUS) && (dwRet <= HTTP_STATUS_REDIRECT_KEEP_VERB)))
						{
							TCHAR tszLocation[1024 * 2] = { 0 };
							DWORD dwBufSize = 1024 * 2 * sizeof(TCHAR);
							pFile->QueryInfo(HTTP_QUERY_LOCATION, tszLocation, &dwBufSize);

							_sRedirectUrl.Format(_T("%s"), tszLocation);
							bRequestState = true;
							if (_sRedirectUrl.IsEmpty() || (_sRedirectUrl.GetLength() < 4))
							{
								if (!bHttpRequest)
								{
									bHttpRequest = true;
									bRequestState = false;
								}
								else
								{
									if ((IRETRYTIMES - 1) == i)
									{
										_sRedirectUrl = strUrl;
									}
									else
									{
										bRequestState = false;
									}
								}
							}

							pFile->Close();
							delete pFile;
						}
						else
						{
							pFile->Close();
							delete pFile;
							bHttpRequest = true;
						}
					}
					else
					{
						bHttpRequest = true;
					}


					pConnection->Close();
					delete pConnection;

					m_session.Close();

					if (bRequestState)
					{
						break;
					}
					Sleep(200);
				}
				catch (...)
				{
					//60无特殊含义，仅代表url长度，之所以定为60，是经验值
					if ((IRETRYTIMES - 1) == i)
					{
						if (strUrl.GetLength() < 60)
						{
							_sRedirectUrl = strUrl;
						}
						else
						{
							_sRedirectUrl = _T("error");
						}
						return;
					}
					else
					{
						if (pFile != NULL)
						{
							pFile->Close();
							delete pFile;
							pConnection->Close();
							delete pConnection;
							m_session.Close();
							g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("获取重定向Url,请求读取数据异常,错误码：%d，目标网址:%s"), GetLastError(), strUrl.GetString());
							Sleep(200);
						}
					}
				}
			}
		}
	}
#endif

	//判断请求的目标网址是否合法
	if (_sRedirectUrl.GetLength() > 4)
	{
		CString shttp = _sRedirectUrl.Left(4);
		if (_T("http") != shttp)
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("提取的目标网址非法，请求url=%s"), strUrl.GetString());
		}
	}

	if (strRedirectUrl.IsEmpty())
	{
		strRedirectUrl += _sRedirectUrl;
	}
	else
	{
		strRedirectUrl += _T("(;a)");
		strRedirectUrl += _sRedirectUrl;
	}
}
static size_t downloadCallback(void *buffer, size_t sz, size_t nmemb, void *writer)
{
	string* psResponse = (string*)writer;
	size_t size = sz * nmemb;
	psResponse->append((char*)buffer, size);

	return sz * nmemb;
}

void CWebDlg::GetAddrByUrl(CString& _strUrl, CString& _strRedirectUrl)
{
	string strUrl = CT2A(_strUrl.GetBuffer());
	string strTmpStr;
	int iResult = OPEN_PAGE_FAILED;

	CURL *curl = curl_easy_init();
	if (NULL == curl)
	{
		return;
	}
	struct curl_slist *headers = NULL; /* init to NULL is important */
	headers = curl_slist_append(headers, "Accept: text/html");
	headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
	headers = curl_slist_append(headers, "Accept-Language: zh-CN");
	headers = curl_slist_append(headers, "Content-Type:text/html; charset=utf-8");
	headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Linux; U; Android 2.2; en-us; Nexus One Build/FRF91) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1");
	try
	{
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloadCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strTmpStr);
		curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
		//curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
		//CURLOPT_FOLLOWLOCATION指明：
		//让curl递归的抓取http头中Location中指明的url。
		//当抓取次数超过CURLOPT_MAXREDIRS时，递归将终止
		//解决重定向的问题
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		CURLcode res = curl_easy_perform(curl);
		string strRsp;
		if (res == CURLE_OK)
		{
			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			if (HTTP_STATUS_OK == http_code)
			{
				CString strUrl;
				CStringA strUrlA;
				CStringA strSrc;
				CStringA strTempSrc;
				bool bFind = false;
				strTempSrc.Format("%s", strTmpStr);
				strSrc = strTempSrc;
				try
				{
					strTempSrc.MakeLower();
					if (strTempSrc.Find("window.location.replace") != -1)
					{
						int iTargetChar = strTempSrc.Find("window.location.replace");
						strUrlA = strTempSrc.Mid(iTargetChar);
						strSrc = strSrc.Mid(iTargetChar);
						bFind = true;
					}
					else if (strTempSrc.Find("<meta http-equiv") != -1)
					{
						int iTargetChar = strTempSrc.Find("<meta http-equiv");
						strUrlA = strTempSrc.Mid(iTargetChar);
						strSrc = strSrc.Mid(iTargetChar);
						bFind = true;
					}
					else if (strTempSrc.Find("http:") != -1 || strTempSrc.Find("https:") != -1)
					{
						bFind = true;
						strUrlA = strTempSrc;
					}

					if (bFind)
					{
						int iPosUrl = strUrlA.Find("http:");
						if (-1 == iPosUrl)
						{
							iPosUrl = strUrlA.Find("https:");
						}
						if (-1 != iPosUrl)
						{
							strUrlA = strUrlA.Mid(iPosUrl);
							strSrc = strSrc.Mid(iPosUrl);

							for (int index = 0; index < strUrlA.GetLength(); index++)
							{
								TCHAR cChar = strUrlA.GetAt(index);
								if (_T(' ') == cChar || _T('>') == cChar || _T('\"') == cChar || _T('\'') == cChar || _T('\r') == cChar || _T('\n') == cChar)
								{
									strSrc = strSrc.Left(index);
									strUrl = (CString)strSrc;
									_strRedirectUrl = strUrl;
									break;
								}
							}
						}
					}

				}
				catch (...)
				{
					curl_easy_cleanup(curl);
					return;
				}
			}
		}
	}
	catch (...)
	{
		curl_easy_cleanup(curl);
		return;
	}
	curl_easy_cleanup(curl);
}

int CWebDlg::GetSrcByLibcurl(CString& _strUrl, CStringA& _strWebSrc, CString& _strContentType, const KeyWordDataInfo& _sData, CString& _sRedirectUrl, long& _iHttpStatusCode, bool _bIsOfficialWebFlag)
{
	string strUrl = CT2A(_strUrl.GetBuffer());
	string strTmpStr;
	int iResult = OPEN_PAGE_FAILED;

	CURL *curl = curl_easy_init();
	if (NULL == curl)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("curl_easy_init初始化失败"));
		iResult = PAGE_FUNTION_ERROR;
		return iResult;
	}
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("**解析的目标网址：%s"), _strUrl.GetString());

	struct curl_slist *headers = NULL; /* init to NULL is important */
	headers = curl_slist_append(headers, "Accept: text/html");
	headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
	headers = curl_slist_append(headers, "Accept-Language: zh-CN");
	headers = curl_slist_append(headers, "Content-Type:text/html; charset=utf-8");
	if (SEARCH_PHONEBAIDU == _sData.iFlag || SEARCH_PHONE360 == _sData.iFlag || SEARCH_PHONESOGOU == _sData.iFlag || SEARCH_PHONESHENMA == _sData.iFlag)
	{
		if (_bIsOfficialWebFlag)
		{
			headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Linux; U; Android 2.2; en-us; Nexus One Build/FRF91) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1  UpdateRankQ");
		}
		else
		{
			headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Linux; U; Android 2.2; en-us; Nexus One Build/FRF91) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1");
		}
	}
	else
	{
		if (_bIsOfficialWebFlag)
		{
			headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.110 Safari/537.36  UpdateRankQ");
		}
		else
		{
			headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.110 Safari/537.36");
		}
	}

	for (int i = 0; i < IRETRYTIMES; i++)
	{
		try
		{
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloadCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strTmpStr);
			curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
			//curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
			//CURLOPT_FOLLOWLOCATION指明：
			//让curl递归的抓取http头中Location中指明的url。
			//当抓取次数超过CURLOPT_MAXREDIRS时，递归将终止
			//解决重定向的问题
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
			CURLcode res = curl_easy_perform(curl);
			string strRsp;
			if (res != CURLE_OK)
			{
				iResult = PAGE_FUNTION_ERROR;
				if ((IRETRYTIMES - 1) == i)
				{
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("http_code：0, ErrorCode=%d"), res);
				}
				Sleep(500);
				continue;
			}
			else
			{
				long http_code = 0;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
				g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("http_code：%d"), http_code);
				if (HTTP_STATUS_OK == http_code)
				{
					char *contentType = { 0 };
					CURLcode return_code = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
					if ((CURLE_OK == return_code) && contentType)
					{
						CStringA sContentTypeTemp;
						sContentTypeTemp.Format("%s", contentType);
						if (sContentTypeTemp.Find("utf"))
						{
							_strContentType = ToWideCharString(sContentTypeTemp.GetBuffer(), CP_UTF8);
						}
						else
						{
							_strContentType = ToWideCharString(sContentTypeTemp.GetBuffer(), CP_ACP);
						}
					}

					char* redirectUrl = { 0 };
					return_code = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &redirectUrl);
					if ((CURLE_OK == return_code) && redirectUrl)
					{
						CStringA sRedirectUrlTemp;
						sRedirectUrlTemp.Format("%s", redirectUrl);
						if (_strContentType.Find(_T("utf")))
						{
							_sRedirectUrl = ToWideCharString(sRedirectUrlTemp.GetBuffer(), CP_UTF8);
						}
						else
						{
							_sRedirectUrl = ToWideCharString(sRedirectUrlTemp.GetBuffer(), CP_ACP);
						}
					}

					strRsp = strTmpStr;
					CStringA strWebSrcTemp;
					strWebSrcTemp = strRsp.c_str();
					//如果数据为空，则继续
					if (strWebSrcTemp.GetLength() <= 0)
					{
						_strWebSrc = "null_data";
						iResult = GET_WEBDATA_EXCEPTION;
						g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("请求目标网址:%s失败,当前次数:%d,搜索引擎网址：%s"), _strUrl.GetString(), i + 1, _sData.strUrl.GetString());
						Sleep(500);
						continue;
					}
					else
					{
						_strWebSrc = strWebSrcTemp;
						iResult = GET_WEBDATA_NORMAL;
						break;
					}
				}
				else
				{
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("打开网页失败，状态码：%d, 网址：%s"), http_code, _strUrl.GetString());
					_iHttpStatusCode = http_code;
					if (http_code >= HTTP_STATUS_BAD_REQUEST && http_code <= HTTP_STATUS_RETRY_WITH)
					{
						//i = IRETRYTIMES;//  这里不重试
						iResult = PAGE_NOT_FOUND;
					}
					else if (http_code >= HTTP_STATUS_SERVER_ERROR && http_code <= HTTP_STATUS_VERSION_NOT_SUP)
					{
						iResult = GET_WEBDATA_EXCEPTION;
					}
				}
			}
		}
		catch (...)
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("http_code：0, 请求读取数据异常,错误码：%d，目标网址:%s"), GetLastError(), _strUrl.GetString());
			curl_easy_cleanup(curl);
			return PAGE_FUNTION_ERROR;
		}

		//如果是没有找到，则退出
		if (iResult != OPEN_PAGE_FAILED)
		{
			//Sleep(500);
			break;
		}
	}

	curl_easy_cleanup(curl);

	return iResult;
}

int CWebDlg::GetSrcByHttp(CString &strUrl, CStringA &strWebSrc, CString& _strContentType, const KeyWordDataInfo &sData, CString& _sRedirectUrl/*, bool _bFlag*/)
{
	CString strServer;
	CString strObject;
	DWORD dwServiceType;
	INTERNET_PORT nPort;
	int iResult = OPEN_PAGE_FAILED;

	//strUrl = _T("http://www.boliwang.com.cn/wap/index.php");
	if (g_bIsDebug)
	{
		strUrl = g_strDebugUrl;
		CString strMsg = _T("开始调试单个Url: ") + strUrl;
		AfxMessageBox(strMsg);
	}
	if (strUrl.Compare(_T("")) == 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("解析目标网址:%s为空"), strUrl.GetString());
		return iResult;
	}
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("解析的目标网址：%s"), strUrl.GetString());

	AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort);
	if (AFX_INET_SERVICE_HTTP != dwServiceType && AFX_INET_SERVICE_HTTPS != dwServiceType)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("解析目标网址:%s失败"), strUrl.GetString());
		return iResult;
	}

	if (g_iDalayForSite != -1)
	{
		Sleep(g_iDalayForSite);
	}

	for (int i = 0; i < IRETRYTIMES; i++)
	{
		CInternetSession m_session(IE_AGENT);
		CHttpFile *pFile = NULL;
		CHttpConnection *pConnection = NULL;
		CString strData;
		strData.Empty();

		try
		{
			//原来为5秒,现在改为30秒
			m_session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 20 * 1000);
			m_session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 20 * 1000);
			m_session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 20 * 1000);
			m_session.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 20 * 1000);
			m_session.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 20 * 1000);
			//m_session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);          // 1次重试

			pConnection = m_session.GetHttpConnection(strServer, nPort);

			if (pConnection == NULL)
			{
				g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("打开目标网址:%s连接失败"), strUrl.GetString());
				return iResult;
			}

			pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject.GetString(), 0, 1, 0, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD);
			if (pFile != NULL)
			{
				DWORD dwRet = 0;
				//原来为5秒,现在改为10秒
				pFile->SetOption(INTERNET_OPTION_SEND_TIMEOUT, 20 * 1000, 0);
				pFile->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 20 * 1000, 0);

				pFile->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 20 * 1000, 0);
				pFile->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 20 * 1000, 0);


				pFile->AddRequestHeaders(_T("Accept: text/html"));
				pFile->AddRequestHeaders(_T("Accept-Language: zh-CN"));
				pFile->AddRequestHeaders(_T("Content-Type:text/html; charset=utf-8"));  //统一用utf-8防止乱码

				if (SEARCH_PHONEBAIDU == sData.iFlag || SEARCH_PHONE360 == sData.iFlag || SEARCH_PHONESOGOU == sData.iFlag)
				{
					//https://blog.csdn.net/tianjinjianzhan/article/details/51702232
					pFile->AddRequestHeaders(_T("User-Agent: Mozilla/5.0 (Linux; U; Android 2.2; en-us; Nexus One Build/FRF91) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1 UpdateRankQ"));
				}
				else
				{
					pFile->AddRequestHeaders(_T("User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.89 Safari/537.36 UpdateRankQ"));
				}
				pFile->SendRequest();
				pFile->QueryInfoStatusCode(dwRet);
				if (dwRet == HTTP_STATUS_OK)
				{
					//					CString strContentType = _T("");
					if (!pFile->QueryInfo(HTTP_QUERY_CONTENT_TYPE, _strContentType))
					{
						g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("从HTTP头中获取ContentType失败err：%d，网址：%s"), GetLastError(), strUrl.GetString());
					}
					TCHAR tszLocation[1024 * 2] = { 0 };
					DWORD dwBufSize = 1024 * 2 * sizeof(TCHAR);
					BOOL bResult = pFile->QueryOption(INTERNET_OPTION_URL, tszLocation, &dwBufSize);
					_sRedirectUrl.Format(_T("%s"), tszLocation);

					DWORD dwRead = 0;
					char  szBufa[MAXSIZE] = { 0 };

					DWORD dwTotalRecievedLen = 0;
					DWORD dwOldLen = 0;

					CStringA strTemp;
					do
					{
						dwRead = pFile->Read(szBufa, sizeof(szBufa)-1);

						dwTotalRecievedLen += dwRead;
						strTemp += szBufa;
						memset(szBufa, 0, sizeof(szBufa));

					} while (dwRead > 0);

					//如果数据为空，则继续
					if (dwTotalRecievedLen <= 0)
					{
						iResult = GET_WEBDATA_EXCEPTION;
						g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("请求目标网址:%s失败,当前次数:%d,搜索引擎网址：%s"), strUrl.GetString(), i + 1, sData.strUrl.GetString());
						Sleep(200);
						continue;
					}
					else
					{
						strWebSrc = strTemp;
						iResult = GET_WEBDATA_NORMAL;
						break;
					}
				}
				else
				{
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("打开网页失败，状态码：%d, 网址：%s"), dwRet, strUrl.GetString());
					if (dwRet >= HTTP_STATUS_BAD_REQUEST && dwRet <= HTTP_STATUS_RETRY_WITH)
					{
						i = IRETRYTIMES;//  这里不重试
						iResult = PAGE_NOT_FOUND;
					}
					else if (dwRet >= HTTP_STATUS_SERVER_ERROR && dwRet <= HTTP_STATUS_VERSION_NOT_SUP)
					{
						iResult = GET_WEBDATA_EXCEPTION;
					}
				}
				pFile->Close();
				delete pFile;
			}

			pConnection->Close();
			delete pConnection;

			m_session.Close();
		}
		catch (...)
		{
			// 			if (BADIDU_INDEX == sData.iIndex && (_T("深圳市贵喜钣金有限公司") == sData.strComany || _T("东莞市吉诺塑胶制品有限公司") == sData.strComany))
			// 			{
			// 				g_debugLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("请求读取数据异常,错误码：%d，目标网址:%s"), GetLastError(), strUrl.GetString());
			// 			}
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("请求读取数据异常,错误码：%d，目标网址:%s"), GetLastError(), strUrl.GetString());
			return OPEN_PAGE_FAILED;
		}

		//如果是没有找到，则退出
		if (iResult != OPEN_PAGE_FAILED)
		{
			Sleep(200);
			break;
		}
	}
	return iResult;
}
/*
@brief  将网页数据转化为指定的编码格式并返回
@param  pszRawData  接收到的网页源数据
@param  strContentType 包含编码类型的字符串
@param  strRetData 指定编码后的数据
@return 是否包含在官网中
*/
#define ENCODELENGTH 30
BOOL CWebDlg::GetPageData(CStringA strRawData, CString &strContentType, CString &strRetData, const std::vector<CStdString> &vCompanys, CString& _sRedirectUrl, CString _UserfulDomain, int& _ErrorCode, long& _iHttpStatusCode)
{
	BOOL bRes = FALSE;
	bool bValue = false;
	bRes = GetEmbedUrlData(strRawData, _sRedirectUrl, bValue, _ErrorCode, _iHttpStatusCode);

	//查看是utf8格式还是gbk格式
	int iPos1 = strRawData.Find(("charset="));

	if (-1 != strContentType.Find(_T("charset")) && !bValue)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("使用http协议指定的编码格式，值：%s "), strContentType.GetString());
	}
	else if (iPos1 != -1)
	{
		strContentType = strRawData.Mid(iPos1, ENCODELENGTH);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("在网页中找到charset=字样，值：%s "), strContentType.GetString());
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("网页及http协议头中都找不到charset=字样，将分别用gbk及UTF8编码解析."));
	}

	strContentType.MakeLower();

	if (strContentType.Find(_T("gb2312")) != -1
		|| strContentType.Find(_T("gbk")) != -1)
	{
		strRetData = ToWideCharString(strRawData.GetBuffer(), CP_ACP);
	}
	else if (strContentType.Find(_T("utf-8")) != -1
		|| strContentType.Find(_T("utf8")) != -1)
	{
		strRetData = ToWideCharString(strRawData.GetBuffer(), CP_UTF8);
	}
	else
	{
		strRetData = ToWideCharString(strRawData.GetBuffer(), CP_UTF8);
		strRetData += ToWideCharString(strRawData.GetBuffer(), CP_ACP);
	}

	PageDataPreproccess(strRetData, _UserfulDomain);

	//如果当前官网(商铺)不包含公司名，则不算该公司官网
	if (bRes)
	{
		for (int i = 0; i < vCompanys.size(); ++i)
		{
			if (strRetData.Find((CString)(vCompanys[i])) != -1)
			{
				bRes = TRUE;
				break;
			}

			bRes = FALSE;
		}
	}

	return bRes;
}

BOOL CWebDlg::PageDataPreproccess(CString &strRetData, CString _UserfulDomain)
{
	//特殊网站数据替换
	pageDataPre arrPageData[] =
	{
		{ _T("78gq.com"), _T("<a href=\"http://www.78gq.com/company/\" target=\"_blank\"><strong class=\"keylink\">公司</strong></a>"), _T("公司") },
	};
	int iArrLength = sizeof(arrPageData) / sizeof(arrPageData[0]);
	for (int i = 0; i < iArrLength; i++)
	{
		if (_UserfulDomain == arrPageData[i].strDomain)
		{
			strRetData.Replace(arrPageData[i].strBeforeData, arrPageData[i].strAfterData);
			break;
		}
	}

	return true;
}

/*
@brief  将网页数据转化为指定的编码格式并返回
@param  pszRawData  要转换的数据
@param  dwRawDataLen 数据长度
@param  codePage 转换代码
@return 指定编码后的数据
*/
CString CWebDlg::ToWideCharString(LPSTR str, DWORD codePage)
{
	CString strResult;
	_ASSERT(str);
	USES_CONVERSION;
	WCHAR *buf;
	int length = MultiByteToWideChar(codePage, 0, str, -1, NULL, 0);
	buf = new WCHAR[length + 1];
	ZeroMemory(buf, (length + 1) * sizeof(WCHAR));
	MultiByteToWideChar(codePage, 0, str, -1, buf, length);

	strResult = (CString(W2T(buf)));
	if (buf != NULL)
	{
		delete[]buf;
		buf = NULL;
	}
	return strResult;
}

CStringA CWebDlg::CStrW2CStrA(const CStringW &cstrSrcW)
{
	int len = WideCharToMultiByte(CP_ACP, 0, LPCWSTR(cstrSrcW), -1, NULL, 0, NULL, NULL);
	char *str = new char[len];
	memset(str, 0, len);
	WideCharToMultiByte(CP_ACP, 0, LPCWSTR(cstrSrcW), -1, str, len, NULL, NULL);
	CStringA cstrDestA = str;
	delete[] str;

	return cstrDestA;
}
CStringW CWebDlg::CStrA2CStrW(const CStringA &cstrSrcA)
{
	int len = MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len];
	memset(wstr, 0, len*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, wstr, len);
	CStringW cstrDestW = wstr;
	delete[] wstr;

	return cstrDestW;
}

/*
@brief  根据百度条目html源码取得对应链接url
@param  strPage  源码
@param  Index	 引擎索引
*/
CString CWebDlg::GetUrlFormContent(const CString &strPage, int Index)
{
	CString strUrl;
	int iPos1 = strPage.Find(g_mapElemCfg[Index].strGetUrlFlag1);
	int iPos2 = -1;
	int length = 0;
	//查找数据的长度+1，取后面数据
	length = g_mapElemCfg[Index].strGetUrlFlag1.GetLength() + 1;
	// 	if (iPos1 != -1)
	// 		iPos1 = strPage.Find(_T("href="), iPos1 + 6);

	if (iPos1 != -1)
	{
		iPos2 = strPage.Find(g_mapElemCfg[Index].strGetUrlFlag2, iPos1 + length);  //加6  href="  包含6个字符
		if (iPos2 != -1 && iPos2 > iPos1 + length)
		{
			strUrl = strPage.Mid(iPos1 + length, iPos2 - iPos1 - length);
		}
	}

	return strUrl;
}



void CWebDlg::exceptionHandling(const KeyWordDataInfo &sData)
{
	if (m_vBackData.size() == 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("%s引擎打开遇到验证码或引擎页面打开失败，未抓取到快照，添加关键词排名个数为-1的信息到容器"), GetSearchFlag(sData.iFlag));
		BackDataInfo* pBack = new BackDataInfo();
		pBack->strKeyWordName = sData.strKeyWordName;
		pBack->iFlag = sData.iFlag;
		pBack->strKey = sData.strKey;
		pBack->strCompanyName = sData.strComany;
		pBack->iRankCnt = -1;
		if (E_ALLURLOPEN_STATE3 == getPageOpenState() || E_ALLURLOPEN_STATE4 == getPageOpenState() || E_ALLURLOPEN_STATE5 == getPageOpenState())
		{
			pBack->strRecordErrorInfo = getErrorWebId();
			if (!pBack->strRecordErrorInfo.IsEmpty())
			{
				pBack->iRankCnt = -2;
			}
		}

		//g_TaskRecord.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("Run*****遇到验证码或打开引擎失败****公司名:%s 关键词:%s  搜索引擎:%s"), sData.strComany, sData.strKeyWordName, g_mapElemCfg[sData.iIndex].strHtmlName);

		m_vBackData.push_back(pBack);
	}
	// 	bool bExceptionFlag = false;
	// 	switch (sData.iFlag)
	// 	{
	// 		case SEARCH_BAIDU:       //百度
	// 		{
	// 			if (!bSearchFlag[BADIDU_INDEX])
	// 			{
	// 				bExceptionFlag = true;
	// 			}
	// 			break;
	// 		}
	// 		case SEARCH_360:       //360搜索
	// 		{
	// 			if (!bSearchFlag[S360_INDEX])
	// 			{
	// 				bExceptionFlag = true;
	// 			}
	// 			break;
	// 		}
	// 		case SEARCH_SOGOU:       //搜狗
	// 		{
	// 			if (!bSearchFlag[SOGOU_INDEX])
	// 			{
	// 				bExceptionFlag = true;
	// 			}
	// 			break;
	// 		}
	// 		case SEARCH_BING:       //必应
	// 		{
	// 			if (!bSearchFlag[BING_INDEX])
	// 			{
	// 				bExceptionFlag = true;
	// 			}
	// 			break;
	// 		}
	// 		case SEARCH_PHONEBAIDU:
	// 		{
	// 			if (!bSearchFlag[PHONEBAIDU_INDEX])
	// 			{
	// 				bExceptionFlag = true;
	// 			}
	// 			break;
	// 		}
	// 		case SEARCH_PHONESOGOU:
	// 		{
	// 			if (!bSearchFlag[PHONESOGOU_INDEX])
	// 			{
	// 				bExceptionFlag = true;
	// 			}
	// 			break;
	// 		}
	// 		case SEARCH_YOUDAO:
	// 		{
	// 			if (!bSearchFlag[YOUDAO_INDEX])
	// 			{
	// 				bExceptionFlag = true;
	// 			}
	// 			break;
	// 		}
	// 		case SEARCH_PHONE360:
	// 		{
	// 			if (!bSearchFlag[PHONE360_INDEX])
	// 			{
	// 				bExceptionFlag = true;
	// 			}
	// 			break;
	// 		}
	// 		case SEARCH_WEIXIN:
	// 		{
	// 			if (!bSearchFlag[WEIXIN_INDEX])
	// 			{
	// 				bExceptionFlag = true;
	// 			}
	// 			break;
	// 		}
	// 		default:
	// 			break;
	// 	}
	// 	if (bExceptionFlag)
	// 	{
	// 		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("%s引擎打开遇到验证码，未抓取到快照，添加关键词排名个数为-1的信息到容器"), GetSearchFlag(sData.iFlag));
	// 		BackDataInfo* pBack = new BackDataInfo();
	// 		pBack->strKeyWordName = sData.strKeyWordName;
	// 		pBack->iFlag = sData.iFlag;
	// 		pBack->strKey = sData.strKey;
	// 		pBack->strCompanyName = sData.strComany;
	// 		pBack->iRankCnt = -1;
	// 		m_vBackData.push_back(pBack);
	// 	}
}

/*
@brief  把抓取的数据返回给上层
*/
void CWebDlg::BackMsg()
{

	if (m_vBackData.size() > 0)
	{
		int i = 0;
		BOOL bHasData = FALSE;
		CStdString strResult = _T("");
		for (i = 0; i < m_vBackData.size(); i++)
		{
			Sleep(1);
			BackDataInfo *pBackData = m_vBackData[i];
			if (pBackData != NULL)
			{
				if (!pBackData->strPagePath.IsEmpty())
				{
					CString strFileName = PathFindFileName(pBackData->strPagePath);
					CString strConName = pBackData->strCompanyName;
					ReplaceHtmlChar(strConName);
					pBackData->strPagePath = g_shttpOss + g_sKWPath + _T("/") + strConName + _T("/") + strFileName;
				}
				if (pBackData->strPagePath.IsEmpty() && pBackData->iRank > 0)
				{
					g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("数据出错!关键词为：%s,搜索引擎为:%s"), pBackData->strKeyWordName, GetSearchFlag(pBackData->iFlag));
					delete pBackData;
					continue;
				}
				strResult.Format(_T("%s%s(;0)%s(;0)%s(;0)%d(;0)%s(;0)%d(;0)%s(;0)%d(;0)%d(;0)%d(;0)%s(;0)%s(;0)%d(;1)")
					, strResult.c_str()
					, pBackData->strKey
					, pBackData->strKeyWordName
					, GetSearchFlag(pBackData->iFlag)
					, pBackData->iRank
					, pBackData->strPagePath
					, pBackData->iRankCnt
					, pBackData->strRecordInfo
					, pBackData->iB2BFlag
					, pBackData->iOfficialFlag
					, pBackData->iAceFlag
					, pBackData->strRedirectUrl
					, pBackData->strRecordErrorInfo
					, pBackData->iKeywordType);
				//g_TaskRecord.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("return*********公司名:%s 关键词:%s  搜索引擎:%s Rankings:%d rankCount:%d"), pBackData->strCompanyName, pBackData->strKeyWordName, GetSearchFlag(pBackData->iFlag), pBackData->iRank, pBackData->iRankCnt);

				bHasData = TRUE;
				delete pBackData;
				pBackData = NULL;
			}
		}
		if (bHasData)
		{
			//			strResult = strResult.Left(strResult.length()-4);
			BYTE * pByData = (BYTE *)strResult.c_str();

			EnterCriticalSection(&critSendMsg);
			g_server->SendData(strResult.size() * 2, E_GET_EXCUTE_TASK_RESULT, (char*)pByData);
			LeaveCriticalSection(&critSendMsg);

			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("返回快照数据：%s"), strResult.c_str());
		}

		m_vBackData.clear();
	}
}

/*
@brief  在搜狗页面上抓取排名
@param  过滤标签内容
@param  sData  关键词相关数据
@param  strFileName  要保存的图片路径   输出
@return	0：有排名	1：无排名	2：打开网页错误
*/
int CWebDlg::GetSoGouPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *_pAllColls)
{
	int iRetValue = OPEN_PAGE_FAILED;
	int iRankCnt = 0;   //抓取到的排名数量
	std::vector<long> vHeights;  //返回高度，可能有多条数据
	CString strWebRecord = _T("");  //网站排名记录

	if (pTables == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("IHTMLElementCollection sogou is NULL"));
		return iRetValue;
	}

	CComQIPtr<IHTMLElement> pElement;
	BackDataInfo *pBdata = NULL;
	HRESULT hr;

	int iRank = 0;
	long lDivCnts = 0;
	pTables->get_length(&lDivCnts);
	CString strAllRedirectUrl(_T(""));

	for (long l = 0; l < lDivCnts; l++)
	{
		IDispatch *pDisp = NULL;
		CString strTemp;
		BOOL	bIsB2BCatch = FALSE;

		VARIANT index = { 0 };
		V_VT(&index) = VT_I4;
		V_I4(&index) = l;

		hr = pTables->item(COleVariant(l), index, &pDisp);
		if (FAILED(hr) || pDisp == NULL)
			continue;

		Sleep(30);
		pElement = pDisp;
		pDisp->Release();

		CComBSTR bstr;
		pElement->get_innerHTML(&bstr);
		CString strPage(bstr);

		if (strPage.GetLength() <= 0)
			continue;

		if (m_IntelligFind.FindResult(strPage, g_mapElemCfg[sData.iIndex].strItemFlag1
			, g_mapElemCfg[sData.iIndex].strItemFlag2, 0, 1)
			|| m_IntelligFind.FindResult(strPage, _T("class=\"rb\""), _T("class=\"vrwrap\""), 0, 1))
			continue;

		CString strSnapUrl(_T(""));
		CString strSnapShotUrl(_T(""));
		if (!getSnapUrlAndSnapShotUrl(pElement, sData, strSnapUrl, strSnapShotUrl))
		{
			continue;
		}

		iRank++;
		BOOL bIsAce = FALSE;
		CatchInfo cInfo;
		DWORD dwWebId = 0;		//网站ID;
		bool bIsOfficialWebFlag = false;	//当前网站是否为官网网址;

		CString strUserfulDomain(_T(""));
		BOOL bFind = LinkUrlIsInOurSitelist(strSnapUrl, strSnapShotUrl, sData, cInfo.bIsOfficialFlag, dwWebId, strUserfulDomain, strPage, bIsOfficialWebFlag);

		if (bFind || cInfo.bIsOfficialFlag)
		{
			int iAllOpenState = -1;
			CString strAllErrorInfo(_T(""));
			int iResult = HasCompanyInHtml(strSnapUrl, sData, cInfo, dwWebId, lp, strSnapShotUrl, strAllRedirectUrl, strUserfulDomain, bIsOfficialWebFlag, iAllOpenState, strAllErrorInfo);

			changeOpenUrlState(iAllOpenState);
			setErrorWebId(strAllErrorInfo);

			if (iResult != FOUND_COMPANY || dwWebId <= 0)
			{
				if (iResult == NOT_FOUND_COMPANY || iResult == PAGE_NOT_FOUND)
				{
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("sogou ：没有在目标网页中找到公司名"));
				}
				//continue;
				if (!cInfo.bIsOfficialFlag)
				{
					continue;
				}
			}
			else
			{
				bIsB2BCatch = TRUE;
				iRankCnt++;
			}

			//如果找到递增1
			if (cInfo.bIsOfficialFlag)
			{
				iRankCnt++;
			}

			if (addPageTags(pElement, cInfo, dwWebId, sData, bIsAce))
			{
				l++;
				lDivCnts++;
			}

			if (vHeights.size() == 0)
			{
				pBdata = new BackDataInfo();
				ASSERT(pBdata != NULL);

				pBdata->iFlag = SEARCH_SOGOU;
				pBdata->iRank = sData.iCurPage * 10 + iRank;

				if (0 == sData.iCurPage)
				{
					if (pBdata->iRank > 10)
					{
						pBdata->iRank = 10;
					}
				}
				else if (1 == sData.iCurPage)
				{
					if (pBdata->iRank > 20)
					{
						pBdata->iRank = 20;
					}
				}
				else if (2 == sData.iCurPage)
				{
					if (pBdata->iRank > 30)
					{
						pBdata->iRank = 30;
					}
				}

				pBdata->strKeyWordName = sData.strKeyWordName;
				pBdata->strKey = sData.strKey;
				pBdata->strCompanyName = sData.strComany;
			}
			vHeights.push_back(iRank);
			//如果抓到包含官网的，设置标记为1
			if (cInfo.bIsOfficialFlag)
			{
				pBdata->iOfficialFlag = 1;
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);
				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			//如果抓到包含其他的,设置标记为1
			else if (bIsB2BCatch)
			{
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);

				//是否为2代网站
				if (dwWebId < 40000000 && dwWebId != 4000065)
					pBdata->iB2BFlag = 1;

				//是否为王牌网站
				if (cInfo.bIsAceFlag && bIsAce)
					pBdata->iAceFlag = 1;

				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}

			if (pBdata != NULL && 0 == pBdata->iKeywordType)
			{
				pBdata->iKeywordType = IsADWord(sData.iFlag, sData.iIndex, _pAllColls);
			}
		}
	}
	addBackData(pBdata, sData, iRetValue, strFileName, iRankCnt, strWebRecord, strAllRedirectUrl);
	return iRetValue;
}


/*
@brief  判断搜狗的网址是否在   收录的网站
@param  strHtml  某个条目的html代码
@return TRUE表存在
*/
BOOL CWebDlg::FindIsOwnWebInSoGou(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId, CString& sSiteName)
{
	return FindIsOwnWebInGoogle(strHtml, sData, bOwnOfficia, dwWebId, sSiteName);
}

/*
@brief 根据搜索标志得到搜索引擎名称
@param iFlag  搜索标志
@return 搜索引擎名称
*/
CString CWebDlg::GetSearchFlag(int iFlag)
{
	CString strSearch;

	switch (iFlag)
	{
	case SEARCH_BAIDU:       //百度
		strSearch = _T("百度");
		break;
	case SEARCH_360:       //360搜索
		strSearch = _T("360搜索");
		break;
	case SEARCH_SOGOU:       //搜狗
		strSearch = _T("搜狗");
		break;
	case SEARCH_BING:       //必应
		strSearch = _T("必应");
		break;
	case SEARCH_PHONEBAIDU:
		strSearch = _T("手机百度");
		break;
	case SEARCH_PHONESOGOU:
		strSearch = _T("手机搜狗");
		break;
	case SEARCH_YOUDAO:
		strSearch = _T("有道");
		break;
	case SEARCH_PHONE360:
		strSearch = _T("手机360");
		break;
	case SEARCH_WEIXIN:
		strSearch = _T("微信");
		break;
	case SEARCH_PHONESHENMA:
		strSearch = _T("手机神马");
		break;
	}

	return strSearch;
}

/*
@brief  360改版后返回的URL不对360搜索页面返回的URL处理找到真正的URL
@param  strURL  360搜索页面得到的URL
@return 返回真正的URL(URL解码后)
CString CWebDlg::GetRealURL(CString strURL)
{
CString strTemp = strURL;
int iPos1, iPos2;
iPos1 = iPos2 = 0;
iPos1 = strTemp.Find(g_mapElemCfg[S360_INDEX].strUniversalFst, iPos2);

if (iPos1 != -1)
{
iPos2 = strTemp.Find(g_mapElemCfg[S360_INDEX].strUniversalSnd, iPos1);

if (iPos2 != -1)
{
strTemp = strTemp.Mid(iPos1 + 4, iPos2 - iPos1 - 4);
strTemp = UrlDecode(strTemp.GetBuffer());
}

}

return strTemp;
}*/
/*

/ *
@brief  在必应页面上抓取排名
@param  过滤标签内容
@param  sData  关键词相关数据
@param  strFileName  要保存的图片路径   输出
@return	0：有排名	1：无排名	2：打开网页错误
* /
int CWebDlg::GetBingPhoto(IHTMLElementCollection *pLis, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult)
{
int iRetValue = OPEN_PAGE_FAILED;
int iRankCnt = 0;   //抓取到的排名数量
std::vector<long> vHeights;  //返回高度，可能有多条数据
CString strWebRecord = _T("");  //网站排名记录

if (pLis == NULL)
{
g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("IHTMLElementCollection bing is NULL"));
return iRetValue;
}

CComQIPtr<IHTMLElement> pElement;
BackDataInfo *pBdata = NULL;
HRESULT hr;

int iRank = 0;
long lLiCounts = 0;
pLis->get_length(&lLiCounts);

for (long l = 0; l <= lLiCounts; l++)
{
IDispatch  *pDisp = NULL;
BOOL	bIsB2BCatch = FALSE;
CString strTemp;

VARIANT index = { 0 };
V_VT(&index) = VT_I4;
V_I4(&index) = l;

hr = pLis->item(COleVariant(l), index, &pDisp);
if (FAILED(hr) || pDisp == NULL)
continue;

Sleep(30);
pElement = pDisp;
pDisp->Release();

CComBSTR bstr;
pElement->get_outerHTML(&bstr);
CString strPage(bstr);

strPage.MakeLower();

if (strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag1) == -1)
{
continue;
}

bstr.~CComBSTR();
pElement->get_innerHTML(&bstr);

strPage = bstr;
iRank++;

BOOL bOwnOfficial = FALSE;
DWORD dwWebId = 0;		//网站ID;
BOOL bFind = FindIsOwnWebInGoogle(strPage, sData, bOwnOfficial, dwWebId);

if (bFind || bOwnOfficial)
{
int iResult = HasCompanyInHtml(GetUrlFormContent(strPage,sData.iIndex), sData,bOwnOfficial);
if (iResult != FOUND_COMPANY)
{
if (iResult == NOT_FOUND_COMPANY || iResult == PAGE_NOT_FOUND)
g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("baidu ：没有在目标网页中找到公司名"));

if (!bOwnOfficial)
{
continue;
}
}
else
{
bIsB2BCatch = TRUE;
iRankCnt++;
}

//如果找到递增1
if (bOwnOfficial)
{
iRankCnt++;
}

//加红框
strPage.Insert(0, _T("<div style=\"padding: 5px; border: 2px solid rgb(223, 0, 1); border-image: none; margin-bottom: 5px; box-shadow: 1px 1px 3px 0px rgba(0,0,0,0.6);\">"));
strPage.Append(_T("</div>"));
hr = pElement->put_innerHTML((BSTR)strPage.GetString());

if (FAILED(hr))
{
g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("bing ：加红框失败"));
}

if (vHeights.size() == 0)
{
pBdata = new BackDataInfo();
ASSERT(pBdata != NULL);

pBdata->iFlag = SEARCH_BING;
pBdata->iRank = sData.iCurPage * 10 + iRank;
pBdata->strKeyWordName = sData.strKeyWordName;
pBdata->strKey = sData.strKey;
pBdata->strCompanyName = sData.strComany;
}
vHeights.push_back(sData.iCurPage*10 + iRank);
//如果抓到包含官网的，设置标记为1
if (bOwnOfficial)
{
pBdata->iOfficialFlag = 1;
}
//如果抓到包含其他的,设置标记为1
if (bIsB2BCatch)
{
//记录下网站ID对应排名
strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);
if (dwWebId < 40000000)
pBdata->iB2BFlag = 1;
if (strWebRecord.GetLength() <= 0)
{
strWebRecord = strTemp;
}
else
{
strWebRecord += _T("(;2)") + strTemp;
}
}
//}
}
}

if (pBdata != NULL)
{
CString strCompany = sData.strComany;
iRetValue = PAIMING_EXIST;
GetImageFilePath(sData.strKeyWordName, strCompany, strFileName, SEARCH_BING);

pBdata->strPagePath = strFileName;
pBdata->iRankCnt = iRankCnt;
pBdata->strRecordInfo = strWebRecord;
m_vBackData.push_back(pBdata);
}

return iRetValue;
}*/

/*
@brief  判断Bing搜索的网址是否在   收录的网站
@param  strHtml  某个条目的html代码
@return TRUE表存在
BOOL CWebDlg::FindIsOwnWebInBing(CString strHtml, const KeyWordDataInfo &sData, BOOL& bOwnOfficia, DWORD& dwWebId)
{
return FindIsOwnWebInGoogle(strHtml, sData, bOwnOfficia,dwWebId);
}
*/


/*
@brief 将html保存到一个文件
@param pHtml  html对象
@param strFileName  html文件路径
*/
void CWebDlg::SaveToHtml(IHTMLElementCollection *pHtml, const CString &strFileName, const KeyWordDataInfo *pSearchData)
{

	if (pSearchData->iFlag == SEARCH_YOUDAO)
	{
		SaveToHtmlYouDao(pHtml, strFileName, pSearchData);
		return;
	}
	CComQIPtr<IDispatch> pDispatHtml;
	CComQIPtr<IDispatch> pDispatTemp;
	CComQIPtr<IHTMLElement> pElement;
	CComQIPtr<IHTMLElementCollection> pTags;
	CString strBaseUrl = _T("");

	//满足所有链接能够正常点击
	int iPos = pSearchData->strUrl.Find(_T(".com"));
	if (iPos != -1)
	{
		strBaseUrl = pSearchData->strUrl.Mid(0, iPos + 5);
		strBaseUrl.Format(_T("<base href=\"%s\">"), strBaseUrl);
	}
	else
	{
		iPos = pSearchData->strUrl.Find(_T(".cn"));
		if (iPos != -1)
		{
			strBaseUrl = pSearchData->strUrl.Mid(0, iPos + 4);
			strBaseUrl.Format(_T("<base href=\"%s\">"), strBaseUrl);
		}
	}


	pHtml->tags(COleVariant(_T("html")), &pDispatHtml);
	pTags = pDispatHtml;

	VARIANT index = { 0 };
	V_VT(&index) = VT_I4;
	V_I4(&index) = 0;

	::DeleteFile(strFileName);

	if (pTags != NULL)
	{
		pTags->item(index, index, &pDispatTemp);

		pElement = pDispatTemp;
		if (pElement != NULL)
		{
			CComBSTR bstr;

			pElement->get_outerHTML(&bstr);

			CString strTmp;
			strTmp = bstr;
			//插入Base
			//strTmp.Insert(6, strBaseUrl);
			strTmp.Insert(0, strBaseUrl);
			bstr = strTmp.AllocSysString();



			{
				//关键词写入到输入框
				CString strTemp = bstr;
				CString sSource;
				sSource.Format(_T("%s=\"%s\""), g_mapElemCfg[pSearchData->iIndex].tagKWEdit.skey, g_mapElemCfg[pSearchData->iIndex].tagKWEdit.sValue);
				CString sReplace;
				sReplace.Format(_T("%s=\"%s\" value=\"%s\""), g_mapElemCfg[pSearchData->iIndex].tagKWEdit.skey, g_mapElemCfg[pSearchData->iIndex].tagKWEdit.sValue, pSearchData->strKeyWordName);
				strTemp.Replace(sSource, sReplace);
				bstr = strTemp;
			}
			if (_T("1") != g_mapElemCfg[pSearchData->iIndex].sKWSearchMethod && (pSearchData->iFlag == SEARCH_PHONE360
				|| pSearchData->iFlag == SEARCH_WEIXIN
				|| pSearchData->iFlag == SEARCH_PHONESOGOU))
			{
				CString strBtn;
				strBtn = bstr;
				ChangeSpecialButtonStyle(strBtn, pSearchData);
				bstr = strBtn.AllocSysString();
			}
			FILE *pFile = _wfopen(strFileName, _T("w+"));

			if (pFile != NULL)
			{
				CStringA strA = WideToChar(bstr, CP_UTF8);

				if (IsNeedChange(pSearchData->iFlag))
				{
					string strHTML = strA.GetBuffer(0);
					string strCss = "";

					ChangeData(pSearchData->iFlag, strCss, pSearchData->iIndex);
					RemoveAllScriptTag(strHTML);
					ReplaceStyleToCss(strHTML, strCss.c_str());
					RemoveSpecialTag(strHTML, pSearchData->iFlag);
					fwrite(strHTML.c_str(), 1, strHTML.length(), pFile);
				}
				else
				{
					CStringA strHtml = strA;
					if (pSearchData->iFlag == SEARCH_WEIXIN)
					{
						strHtml.Replace("/wechat/css", "http://weixin.sogou.com/wechat/css");
					}
					fwrite(strHtml.GetBuffer(), 1, strHtml.GetLength(), pFile);
				}

				fclose(pFile);
			}
		}
	}

}

void CWebDlg::SaveToHtmlYouDao(IHTMLElementCollection *pHtml, const CString &strFileName, const KeyWordDataInfo *pSearchData)
{
	CComQIPtr<IDispatch> pDispatHtml;
	CComQIPtr<IDispatch> pDispatTemp;
	CComQIPtr<IHTMLElement> pElement;
	CComQIPtr<IHTMLElementCollection> pTags;
	CComBSTR bstr;
	CComBSTR bstrdIV;
	CStringA strHtml = "<html>";
	pHtml->tags(COleVariant(g_mapElemCfg[pSearchData->iIndex].strUniversalFst), &pDispatHtml);
	pTags = pDispatHtml;

	VARIANT index = { 0 };
	V_VT(&index) = VT_I4;
	V_I4(&index) = 0;
	long icount = 0;

	::DeleteFile(strFileName);

	if (pTags != NULL)
	{
		pTags->item(index, index, &pDispatTemp);

		pElement = pDispatTemp;
		if (pElement != NULL)
		{
			pElement->get_outerHTML(&bstr);
		}
	}
	pHtml->tags(COleVariant(g_mapElemCfg[pSearchData->iIndex].strUniversalSnd), &pDispatHtml);
	pTags = pDispatHtml;
	if (pTags != NULL)
	{
		pTags->item(index, index, &pDispatTemp);

		pElement = pDispatTemp;
		if (pElement != NULL)
		{
			pElement->get_outerHTML(&bstrdIV);
		}

		CString strBtn;
		strBtn = bstrdIV;
		ChangeSpecialButtonStyle(strBtn, pSearchData);
		bstrdIV = strBtn.AllocSysString();
	}
	CStringA strA = WideToChar(bstr, CP_UTF8);
	CStringA strDiv = WideToChar(bstrdIV, CP_UTF8);
	strHtml += strA;
	strHtml.Append("<body>");
	strHtml += strDiv;
	strHtml.Append("</body>");
	strHtml.Append("</html>");

	FILE *pFile = _wfopen(strFileName, _T("w+"));

	if (pFile != NULL)
	{
		strHtml.Replace("/css/tgl.css", "../../css/tgl.css");
		strHtml.Replace("/css/left_nav.css", "../../css/left_nav.css");
		fwrite(strHtml.GetBuffer(), 1, strHtml.GetLength(), pFile);
	}
	fclose(pFile);
}

void CWebDlg::ChangeButtonStyle(IHTMLElementCollection *pHtml, const KeyWordDataInfo *pKeyWordDataInfo)
{
	CComQIPtr<IDispatch> pDispatTemp;
	CComQIPtr<IHTMLElement> pElement;
	CString strUrl;
	CString strBtn;
	CComBSTR bstr;

	VARIANT index = { 0 };
	VARIANT varID = { 0 };

	//用于查找的按钮链接
	strUrl.Format(_T(" onclick=\"javascript:location.href='%s'\" "), pKeyWordDataInfo->strUrl);

	V_VT(&index) = VT_I4;
	V_I4(&index) = 0;

	VariantInit(&varID);
	varID.vt = VT_BSTR;

	varID.bstrVal = g_mapElemCfg[pKeyWordDataInfo->iIndex].strBtnId.AllocSysString();

	pHtml->item(varID, index, &pDispatTemp);
	pElement = pDispatTemp;

	if (pElement != NULL)
	{
		pElement->get_outerHTML(&bstr);

		int iPos = 6;  //插入strUrl位置，默认都是input之后
		strBtn = bstr;
		strBtn.MakeLower();
		if (strBtn.Find(_T("<button")) != -1)
		{
			iPos = 7;
		}
		strBtn.Replace(_T("submit"), _T("button"));
		strBtn.Insert(iPos, strUrl);

		BSTR pStr = strBtn.AllocSysString();

		if (FAILED(pElement->put_outerHTML(pStr)))
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("修改按钮样式失败  type:%d"), pKeyWordDataInfo->iFlag);

		SysFreeString(pStr);
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("取不到相应按钮元素  type:%d"), pKeyWordDataInfo->iFlag);
	}

}


void CWebDlg::SetInternetFeature(void)
{
	LRESULT lr = 0;
	INTERNETFEATURELIST featureToEnable = FEATURE_RESTRICT_FILEDOWNLOAD;

	//禁止下载文件
	if (SUCCEEDED(CoInternetSetFeatureEnabled(featureToEnable, SET_FEATURE_ON_PROCESS, true)))
	{
		//Check to make sure that the API worked as expected
		if (FAILED(CoInternetIsFeatureEnabled(featureToEnable, SET_FEATURE_ON_PROCESS)))
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("设置FEATURE_RESTRICT_FILEDOWNLOAD失败,err:%d"), GetLastError());
		}
	}
	else
	{
		//The API returned an error while enabling pop-up management
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("设置FEATURE_RESTRICT_FILEDOWNLOAD失败,err:%d"), GetLastError());
	}

	//禁止网页导航及提交操作时发出声音
	featureToEnable = FEATURE_DISABLE_NAVIGATION_SOUNDS;
	if (SUCCEEDED(CoInternetSetFeatureEnabled(featureToEnable, SET_FEATURE_ON_PROCESS, true)))
	{
		//Check to make sure that the API worked as expected
		if (FAILED(CoInternetIsFeatureEnabled(featureToEnable, SET_FEATURE_ON_PROCESS)))
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("设置FEATURE_DISABLE_NAVIGATION_SOUNDS失败,err:%d"), GetLastError());
		}
	}
	else
	{
		//The API returned an error while enabling pop-up management
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("设置FEATURE_DISABLE_NAVIGATION_SOUNDS失败,err:%d"), GetLastError());
	}
}

#define HTMLLENGTH  12000
int CWebDlg::JudgeHtml(IHTMLElementCollection *pHtml, const KeyWordDataInfo *pKeyWordDataInfo)
{
	int iResult = OPEN_PAGE_FAILED;
	CString strPage;
	CComQIPtr<IDispatch> pDispatHtml;
	CComQIPtr<IDispatch> pDispatTemp;
	CComQIPtr<IHTMLElement> pElement;
	CComQIPtr<IHTMLElementCollection> pTags;

	pHtml->tags(COleVariant(g_mapElemCfg[pKeyWordDataInfo->iIndex].strJudgeHtmlTag), &pDispatHtml);
	pTags = pDispatHtml;

	VARIANT index = { 0 };
	V_VT(&index) = VT_I4;
	V_I4(&index) = 0;

	Sleep(50);

	if (pTags != NULL)
	{
		pTags->item(index, index, &pDispatTemp);

		pElement = pDispatTemp;
		if (pElement != NULL)
		{
			CComBSTR bstr;

			pElement->get_outerHTML(&bstr);

			strPage = bstr;
			int iContentLen = strPage.GetLength();
			if (iContentLen < HTMLLENGTH)
			{
				g_pLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("打开页面的html字符串长度为：%d, 小于%d, 非正常页面, 网址为：%s"), iContentLen, HTMLLENGTH, pKeyWordDataInfo->strUrl.GetString());
				g_pLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("网页HTML内容为(后10000个字符）：%s"), strPage.Right(10000).GetString());

				if (strPage.Find(_T("验证码")) != -1)
				{
					iResult = ENGINE_APPEAR_VERIFYCODE;
					if (pKeyWordDataInfo->iFlag == SEARCH_SOGOU)
					{
						g_pLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("搜狗发现验证码字样,当前搜索引擎撤消搜索, 网址为:%s,当前完成搜索关键字个数为:%d"), pKeyWordDataInfo->strUrl.GetString(), g_iSogou);
					}
					if (pKeyWordDataInfo->iFlag == SEARCH_BAIDU)
					{
						g_pLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("百度发现验证码字样,当前搜索引擎撤消搜索, 网址为:%s,当前完成搜索关键字个数为:%d"), pKeyWordDataInfo->strUrl.GetString(), g_iBaidu);
					}
					if (pKeyWordDataInfo->iFlag == SEARCH_360)
					{
						g_pLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("360发现验证码字样,当前搜索引擎撤消搜索, 网址为:%s,当前完成搜索关键字个数为:%d"), pKeyWordDataInfo->strUrl.GetString(), g_i360);
					}
					if (pKeyWordDataInfo->iFlag == SEARCH_BING)
					{
						g_pLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("必应发现验证码字样,当前搜索引擎撤消搜索, 网址为:%s,当前完成搜索关键字个数为:%d"), pKeyWordDataInfo->strUrl.GetString(), g_iBing);
					}
					if (pKeyWordDataInfo->iFlag == SEARCH_YOUDAO)
					{
						g_pLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("有道发现验证码字样,当前搜索引擎撤消搜索, 网址为:%s,当前完成搜索关键字个数为:%d"), pKeyWordDataInfo->strUrl.GetString(), g_iYouDao);
					}
					if (pKeyWordDataInfo->iFlag == SEARCH_WEIXIN)
					{
						g_pLog.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("搜狗微信发现验证码字样,当前搜索引擎撤消搜索, 网址为:%s"), pKeyWordDataInfo->strUrl.GetString());
					}
				}
				else
				{
					g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("页面不能正常打开！"));
				}

				DeleteSearchCache();
			}
			else
				iResult = OPEN_PAGE_NORMAL;
		}
	}


	return iResult;
}

/*
Add By 周林
@brief  移除要保存HTML里的所有script标签
@param  string strHTMLData HTML内容
@return void
*/
void CWebDlg::RemoveAllScriptTag(string &strHTMLData)
{
	int ipos = -10;
	int iposL = strHTMLData.find("<script");
	int iposU = strHTMLData.find("<SCRIPT");
	if ((iposL != strHTMLData.npos) && (iposU == strHTMLData.npos))
	{
		ipos = iposL;
	}
	else if ((iposL == strHTMLData.npos) && (iposU != strHTMLData.npos))
	{
		ipos = iposU;
	}
	else
	{
		ipos = iposU <= iposL ? iposU : iposL;
	}

	while (ipos != strHTMLData.npos && ipos != -10)
	{
		Sleep(5);
		int ipos1 = -10;
		int iposL1 = strHTMLData.find("</script>");
		int iposU1 = strHTMLData.find("</SCRIPT>");
		if ((iposL1 != strHTMLData.npos) && (iposU1 == strHTMLData.npos))
		{
			ipos1 = iposL1;
		}
		else if ((iposL1 == strHTMLData.npos) && (iposU1 != strHTMLData.npos))
		{
			ipos1 = iposU1;
		}
		else
		{
			ipos1 = iposU1 <= iposL1 ? iposU1 : iposL1;
		}

		if (ipos1 != strHTMLData.npos && ipos1 != -10)
		{
			strHTMLData = strHTMLData.replace(ipos, ipos1 - ipos + 9, "");
		}
		else
		{
			return;
		}

		iposL1 = strHTMLData.find("<script");
		iposU1 = strHTMLData.find("<SCRIPT");
		if ((iposL1 != strHTMLData.npos) && (iposU1 == strHTMLData.npos))
		{
			ipos = iposL1;
		}
		else if ((iposL1 == strHTMLData.npos) && (iposU1 != strHTMLData.npos))
		{
			ipos = iposU1;
		}
		else
		{
			ipos = iposU1 <= iposL1 ? iposU1 : iposL1;
		}
	}
}

void CWebDlg::RemoveSpecialTag(string &strHTMLData, int iFlag)
{
	if (iFlag == SEARCH_PHONEBAIDU)
	{
		int iPos = strHTMLData.find("http-equiv=\"refresh\"");
		if (-1 != iPos)
		{
			//<meta http-equiv="refresh" content="0; URL=http://wap.baidu.com/s?word=%E6%BB%A8%E5%B7%9E%E9%9B%95%E5%88%BB%E5%8A%A0%E5%B7%A5&amp;pn=0&amp;pu=sz%401321_480&amp;t_noscript=jump">
			strHTMLData = strHTMLData.replace(iPos, 20, "http-equiv=\"\"");
		}
	}
}

/*
Add By 周林
@brief  把HTML里style标签内容替换成css引用
@param  string strHTMLData HTML内容
@param  char* strData 要替换的内容
@return void
*/
void CWebDlg::ReplaceStyleToCss(string &strHTMLData, const char* strData)
{
	bool bCss = false;
	int ipos = -10;
	int iposL = strHTMLData.find("<style");
	int iposU = strHTMLData.find("<STYLE");
	if ((iposL != strHTMLData.npos) && (iposU == strHTMLData.npos))
	{
		ipos = iposL;
	}
	else if ((iposL == strHTMLData.npos) && (iposU != strHTMLData.npos))
	{
		ipos = iposU;
	}
	else
	{
		ipos = iposU <= iposL ? iposU : iposL;
	}

	if (ipos != strHTMLData.npos && ipos != -10)
	{
		int ipos1 = -10;
		int iposL1 = strHTMLData.find("</style>");
		int iposU1 = strHTMLData.find("</STYLE>");
		if ((iposL1 != strHTMLData.npos) && (iposU1 == strHTMLData.npos))
		{
			ipos1 = iposL1;
		}
		else if ((iposL1 == strHTMLData.npos) && (iposU1 != strHTMLData.npos))
		{
			ipos1 = iposU1;
		}
		else
		{
			ipos1 = iposU1 <= iposL1 ? iposU1 : iposL1;
		}

		if (ipos1 != strHTMLData.npos && ipos1 != -10)
		{
			strHTMLData = strHTMLData.replace(ipos, ipos1 - ipos + 8, strData);
			bCss = true;
		}
	}

	//在</head>前面插入JS
	if (!bCss)
	{
		int ipos = -10;
		int iposL = strHTMLData.find("</head>");
		int iposU = strHTMLData.find("</HEAD>");
		if ((iposL != strHTMLData.npos) && (iposU == strHTMLData.npos))
		{
			ipos = iposL;
		}
		else if ((iposL == strHTMLData.npos) && (iposU != strHTMLData.npos))
		{
			ipos = iposU;
		}
		else
		{
			ipos = iposU <= iposL ? iposU : iposL;
		}
		if (ipos != strHTMLData.npos && ipos != -10)
		{
			strHTMLData = strHTMLData.insert(ipos, strData);
			bCss = true;
		}
	}
}

/*
Add By 周林
@brief  判断HTML是否需要更改
@param  CString strFileName 快照文件名
@return bool
*/
bool CWebDlg::IsNeedChange(int iFlag)
{
	if (iFlag == SEARCH_BAIDU || iFlag == SEARCH_360 || iFlag == SEARCH_BING
		|| iFlag == SEARCH_SOGOU || iFlag == SEARCH_PHONEBAIDU || iFlag == SEARCH_PHONE360 || iFlag == SEARCH_PHONESOGOU || iFlag == SEARCH_PHONESHENMA)
	{
		return true;
	}
	return false;
}

/*
Add By 周林
@brief  根据搜索标志获取style标签需要替换的内容
@param  int iFlag 搜索标志
@param  string& szCss 替换内容【输出】
@return void
*/
void CWebDlg::ChangeData(int iFlag, string& szCss, int index)
{
	CStringA str = ("");
	str = (CStringA)g_mapElemCfg[index].strCssPath;
	szCss = "<link rel=\"stylesheet\" type=\"text/css\" href=\"";
	szCss.append(str.GetBuffer());

	string strJs = "\r\n<script src=\"";
	strJs.append(str.GetBuffer());
	strJs.append("/js/parameter.js\" type = \"text/javascript\" ></script>");


	switch (iFlag)
	{
	case SEARCH_BAIDU:
		szCss.append("/css/baidu.css\">");
		szCss.append(strJs);
		//szCss.append("\r\n<script src=\"http://192.168.1.16:1211/parameter.js\" type=\"text/javascript\" ></script>");
		break;
	case SEARCH_360:
		szCss.append("/css/360.css\">");
		szCss.append(strJs);
		break;
	case SEARCH_BING:
		szCss.append("/css/bing.css\">");
		szCss.append(strJs);
		break;
	case SEARCH_SOGOU:
		szCss.append("/css/sogou.css\">");
		szCss.append(strJs);
		break;
	case SEARCH_PHONEBAIDU:
		szCss.append("/css/phonebaidu.css\">");
		szCss.append(strJs);
		break;
	case SEARCH_PHONE360:
		szCss.append("/css/phone360.css\">");
		szCss.append(strJs);
		break;
	case SEARCH_PHONESOGOU:
		szCss.append("/css/phonesogou.css\">");
		szCss.append(strJs);
		break;
	case SEARCH_PHONESHENMA:
		szCss.append("/css/phoneshenma.css\">");
		szCss.append(strJs);
		break;

	default:
		break;
	}
}

/*
@brief 获取当前url的源文件；
@param url 地址
@return 源文本
*/
CStringA CWebDlg::urlopen(CString url, int& _ErrorCode, long& _iHttpStatusCode)
{
	CStringA strData = "";

	if (url.GetLength() < 3)
	{
		return strData;
	}
	// 	url = _T("http://info.b2b168.com/s168-03134000.html");
	// 	url = _T("http://www.atobo.com.cn/HotOffs/Detail/8515031.html");
	// 	url = _T("2b168.com/s168-03134000.html");
	//  	url = _T("http://info.b2b16");
	// 	url = _T("b2b16");
	// 	url = _T("http://info.b2b168.com/s168-03134000");
	// 	url = _T("http://info.b2b168.com/s168-83134000.html123");
	// 	url = _T("http://info.b2b168.com/s168-83134000.html'>");
	// 	url = _T("http://info.b2b168.co");
	// 	url = _T("http://info.b2b168.com/s168-83134000.htm");

	//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("UpdateRankQ：aaa..."));

	HINTERNET hSession = InternetOpen(_T("UpdateRankQ"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("UpdateRankQ：bbb..."));
	if (hSession != NULL)
	{
		//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("UpdateRankQ：ccc..."));
		HINTERNET hHttp = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
		//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("UpdateRankQ：ddd..."));

		TCHAR retBuf[10] = { 0 };
		DWORD bufLen = sizeof(retBuf);
		HttpQueryInfo(hHttp, HTTP_QUERY_STATUS_CODE, retBuf, &bufLen, 0);
		DWORD dwRet = (DWORD)_wtol(retBuf);
		_iHttpStatusCode = dwRet;
		if (dwRet >= HTTP_STATUS_BAD_REQUEST && dwRet <= HTTP_STATUS_RETRY_WITH)
		{
			_ErrorCode = PAGE_NOT_FOUND;
		}
		else if (dwRet >= HTTP_STATUS_SERVER_ERROR && dwRet <= HTTP_STATUS_VERSION_NOT_SUP)
		{
			_ErrorCode = GET_WEBDATA_EXCEPTION;
		}
		if (PAGE_NOT_FOUND == _ErrorCode || GET_WEBDATA_EXCEPTION == _ErrorCode)
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("打开网页失败，状态码ErrorCode：%d, 网址：%s"), dwRet, url.GetString());
		}

		if (hHttp != NULL)
		{
			char Temp[MAXSIZE] = { 0 };
			ULONG Number = 1;
			//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("UpdateRankQ：eee..."));
			while (Number > 0)
			{
				InternetReadFile(hHttp, Temp, MAXSIZE - 1, &Number);
				Temp[Number] = '\0';
				strData += Temp;
			}
			//g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("UpdateRankQ：fff..."));
			InternetCloseHandle(hHttp);
			hHttp = NULL;
		}
		InternetCloseHandle(hSession);

		hSession = NULL;

		return strData;
	}

	return strData;
}

/*
@brief 获取嵌入网址的源文件，返回是否包含在官网中
@param strData c待检测的数据，
@return TRUE 包含在官网中
*/
BOOL CWebDlg::GetEmbedUrlData(CStringA& strData, CString& _sRedirectUrl, bool& _boolValue, int& _ErrorCode, long& _iHttpStatusCode)
{
	//	strData数据样例
	// 	<!DOCTYPE html>
	// 	<html>
	// 	<head>
	// 		<meta content = "always" name = "referrer">
	// 		<script>
	// 		if (window.parent != window) {
	// 			window.top.location.replace("http://www.cn5135.com/ProductDetail-4940745.html");
	// 		}
	// 		else {
	// 			window.location.replace("http://www.cn5135.com/ProductDetail-4940745.html");
	// 		}
	// 		</script>
	// 		<noscript>
	// 			<META http - equiv = "refresh" content = "0;URL='http://www.cn5135.com/ProductDetail-4940745.html'">
	// 		</noscript>
	// 	</head>
	// 	<body>
	// 	</body>
	// 	</html>

	// 	strData = "<body style=\"display:none\"><a href=\"http://www.16898.cc/s62610/Detail?pid=14846328\" id=\"redirect_url\"/><script>document.getElementById(\"redirect_url\").click();</script></body> <noscript><META http-equiv=\"refresh\" content=\"0;URL='http://www.16898.cc/s62610/Detail?pid=14846328.html'\"></noscript> ";
	// 	strData = "<body style=\"display:none\"><a href=\"http://www.16898.cc/s62610/Detail?pid=14846328\" id=\"redirect_url\"/><script>document.getElementById(\"redirect_url\").click();</script></body> <noscript><META http-equiv=\"refresh\" content=\"0;URL='http://www.16898.cc/s62610/Detail?pid=14846328'\"></noscript> ";
	// 	strData = "<body style=\"display:none\"><a href=\"http://www.16898.cc/s62610/Detail?pid=14846328\" id=\"redirect_url\"/><script>document.getElementById(\"redirect_url\").click();</script></body> <noscript><META http-equiv=\"refresh\" content=\"URL=http://www.16898.cc/s62610/Detail?pid=14846328\"></noscript> ";
	// 	strData = "<body style=\"display:none\"><a href=\"http://www.16898.cc/s62610/Detail?pid=14846328\" id=\"redirect_url\"/><script>document.getElementById(\"redirect_url\").click();</script></body> <noscript><META http-equiv=\"refresh\" content=\"0;URL='http://www.16898.cc/s62610/Detail?pid=14846328></noscript> ";
	// 	strData = "<body style=\"display:none\"><a href=\"http://www.16898.cc/s62610/Detail?pid=14846328\" id=\"redirect_url\"/>";
	// 	strData = "<body style=\"display:none\"><a href=\"http://www.16898.cc/s62610/Detail?pid=14846328\" id=\"redirect_url\"/>";
	// 	strData = "<body style=\"display:none\"><a href=\"//www.16898.cc/s62610/Detail?pid=14846328\" id=\"redirect_url\"/>";


	CStringA strTempSrc = strData;
	if (strTempSrc.GetLength() < 5)
	{
		return FALSE;
	}
	//查看当前网站是否嵌套（手机百度包含嵌套的网站，首商网，勤加缘）
	//edit by pj
	int iDataLen = strTempSrc.GetLength();
	if (iDataLen < 1000)
	{
		bool bWebSite = false;
		CString strUrl;
		CStringA strUrlA;
		bool bFind = false;

		try
		{
			strTempSrc.MakeLower();
		}
		catch (CException* e)
		{
			strTempSrc = strData;
		}
		if (strTempSrc.Find("window.location.replace") != -1)
		{
			int iTargetChar = strTempSrc.Find("window.location.replace");
			strUrlA = strTempSrc.Mid(iTargetChar);
			strData = strData.Mid(iTargetChar);
			bFind = true;


			// 			int iPosUrl = strData.Find("http:");
			// 			if (-1 == iPosUrl)
			// 			{
			// 				iPosUrl = strData.Find("https:");
			// 			}
			// 
			// 			if (-1 != iPosUrl)
			// 			{
			// 				int iPos2 = strData.Find("\")", iPosUrl);
			// 				//iqjy168Pos = iPos2;
			// 				strUrl = (CString)strData.Mid(iPosUrl, iPos2 - iPosUrl);
			// 				bWebSite = true;
			// 			}
		}
		else if (strTempSrc.Find("<meta http-equiv") != -1)
		{
			int iTargetChar = strTempSrc.Find("<meta http-equiv");
			strUrlA = strTempSrc.Mid(iTargetChar);
			strData = strData.Mid(iTargetChar);
			bFind = true;
		}
		else if (strTempSrc.Find("http:") != -1 || strTempSrc.Find("https:") != -1)
		{
			bFind = true;
			strUrlA = strTempSrc;
		}

		if (bFind)
		{
			int iPosUrl = strUrlA.Find("http:");
			if (-1 == iPosUrl)
			{
				iPosUrl = strUrlA.Find("https:");
			}
			if (-1 != iPosUrl)
			{
				strUrlA = strUrlA.Mid(iPosUrl);
				strData = strData.Mid(iPosUrl);

				for (int index = 0; index < strUrlA.GetLength(); index++)
				{
					TCHAR cChar = strUrlA.GetAt(index);
					if (_T(' ') == cChar || _T('>') == cChar || _T('\"') == cChar || _T('\'') == cChar || _T('\r') == cChar || _T('\n') == cChar)
					{
						//						CStringA strTemp = strUrlA.Left(index);
						// 						strUrl = (CString)strTemp;
						strData = strData.Left(index);
						strUrl = (CString)strData;
						bWebSite = true;
						break;
					}
				}
			}
			else
			{
				g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("查找嵌入网址失败，源数据:%s"), strTempSrc.GetString());
			}
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("查找嵌入网址失败，源数据:%s"), strTempSrc.GetString());

			CString strRetData;
			strRetData = ToWideCharString(strTempSrc.GetBuffer(), CP_UTF8);
			strRetData += ToWideCharString(strTempSrc.GetBuffer(), CP_ACP);
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("查找嵌入网址失败，打印数据:%s"), strRetData);
		}

		if (bWebSite)
		{
			//手机百度勤加缘
			if (strUrl.Find(_T("m.qjy168.com")) != -1)
			{
				int iqjy168Pos = strUrl.ReverseFind(_T('/'));
				CString strUid = strUrl.Mid(iqjy168Pos + 1);
				if (strUrl.Find(_T("https:")) != -1)
				{
					strUrl.Format(_T("https://m.qjy168.com/provide.php?provide_id=%s&remain=all&content_page=1"), strUid);
				}
				else
				{
					strUrl.Format(_T("http://m.qjy168.com/provide.php?provide_id=%s&remain=all&content_page=1"), strUid);
				}
			}
			//手机百度供应商
			if (strUrl.Find(_T("site.china.cn")) != -1)
			{
				strUrl += _T("?minute=1");
			}
			//手机百度际通宝
			//http://wap.gtobal.com/trade/detail--1-4750436202.html
			if (strUrl.Find(_T("wap.gtobal.com")) != -1)
			{
				int iPos = strUrl.ReverseFind(_T('-'));
				CString strUid = strUrl.Mid(iPos + 1);
				if (strUrl.Find(_T("https:")) != -1)
				{
					strUrl.Format(_T("https://www.gtobal.com/sell/detail-%s"), strUid);
				}
				else
				{
					strUrl.Format(_T("http://www.gtobal.com/sell/detail-%s"), strUid);
				}
			}
			_sRedirectUrl = strUrl;
			strData = urlopen(strUrl, _ErrorCode, _iHttpStatusCode);

			if (!strData.IsEmpty())
			{
				_boolValue = true;
			}
		}
	}

	//查找是否包含官网(商铺)
	if (strData.Find("id=\"hide_FLAG_for_qkphoto\"") != -1
		|| strData.Find("id=hide_FLAG_for_qkphoto") != -1)
		return TRUE;

	return FALSE;
}

/*
@brief 判断数据中是否包含公司名标记
@param vComp 所有公司名标记， strData 要查找的字符串；
@return iResult 结果
*/
int CWebDlg::CheckWebFlag(const std::vector<CString>& vComps, const CString& strData, int iIndex, const std::vector<CStdString>& vCompName, const bool& _bOnlyRareWord)
{
	CString strBakData = strData;
	int iResult = OPEN_PAGE_FAILED;
	if (strData.GetLength() <= 0)
	{
		return iResult;
	}

	//替换掉所有的A标签
	for (int i = 0; i < g_mapElemCfg[iIndex].vReplaceTagWebs.size(); ++i)
	{
		if (strBakData.Find(g_mapElemCfg[iIndex].vReplaceTagWebs[i]) != -1)
		{
			ReplaceTag(strBakData, _T("<a>"), _T("</a>"));
		}
	}

	int iSize = vComps.size();
	iResult = NOT_FOUND_COMPANY;        //默认未找到

	iResult = keyFingerSearch(strBakData, g_sFingerRecogn, vCompName);
	if (!iResult && !_bOnlyRareWord)
	{
		for (int i = 0; i < iSize; i++)
		{
			if (strBakData.Find(vComps[i].GetString()) != -1)
			{
				iResult = FOUND_COMPANY;  //找到
				break;
			}
		}
	}
	return iResult;
}

/*
@breif 取得手机360反馈数据
@param  过滤标签内容
@param  sData  关键词相关数据
@param  strFileName  要保存的图片路径   输出
@return 执行结果
*/
int CWebDlg::GetPhone360Photo(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp, IHTMLElementCollection *_pAllColls)
{
	int iRetValue = OPEN_PAGE_FAILED;
	int iRankCnt = 0;   //抓取到的排名数量
	std::vector<long> vHeights;  //返回高度，可能有多条数据
	CString strWebRecord = _T("");  //网站排名记录

	if (pTables == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("IHTMLElementCollection phone360 is NULL"));
		return iRetValue;
	}

	CComQIPtr<IHTMLElement> pElement;
	BackDataInfo *pBdata = NULL;
	HRESULT hr;

	int iRank = 0;
	long lDivCnts = 0;
	pTables->get_length(&lDivCnts);
	CString strAllRedirectUrl(_T(""));

	for (long l = 0; l < lDivCnts; l++)
	{
		IDispatch *pDisp = NULL;
		CString strTemp;
		BOOL	bIsB2BCatch = FALSE;

		VARIANT index = { 0 };
		V_VT(&index) = VT_I4;
		V_I4(&index) = l;

		hr = pTables->item(COleVariant(l), index, &pDisp);
		if (FAILED(hr) || pDisp == NULL)
			continue;

		Sleep(30);
		pElement = pDisp;
		pDisp->Release();

		CString sLocation = g_mapElemCfg[sData.iIndex].strFindUrlFlag1;
		CComBSTR bstr;
		CString strPage;
		if (sLocation == _T("old"))
		{
			pElement->get_innerHTML(&bstr);
			strPage = bstr;

			if (strPage.GetLength() <= 0)
				continue;

			if ((strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag1) != -1) || (strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag2) == -1))
			{
				continue;
			}
		}
		else
		{
			pElement->get_className(&bstr);
			CString strClass(bstr);
			strClass.Trim();
			if (!(strClass.Find(g_mapElemCfg[sData.iIndex].strItemFlag1) != -1 && strClass.Find(g_mapElemCfg[sData.iIndex].strItemFlag2 != -1)))
			{
				continue;
			}
			pElement->get_innerHTML(&bstr);
			strPage = bstr;
		}
		// #if 0
		// 		CComBSTR bstr;
		// 		pElement->get_innerHTML(&bstr);
		// 		CString strPage(bstr);
		// 
		// 		if (strPage.GetLength() <= 0)
		// 			continue;
		// 
		// 		if ((strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag1) != -1) || (strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag2) == -1))
		// 		{
		// 			continue;
		// 		}
		// 
		// #else
		// 		CComBSTR bstr;
		// 		pElement->get_className(&bstr);
		// 		CString strClass(bstr);
		// 		strClass.Trim();
		// 		if (!(strClass.Find(g_mapElemCfg[sData.iIndex].strItemFlag1) != -1 && strClass.Find(g_mapElemCfg[sData.iIndex].strItemFlag2 != -1)))
		// 		{
		// 			continue;
		// 		}
		// 		pElement->get_innerHTML(&bstr);
		// 		CString strPage(bstr);
		// #endif

		CString strSnapUrl(_T(""));
		CString strSnapShotUrl(_T(""));
		if (!getSnapUrlAndSnapShotUrl(pElement, sData, strSnapUrl, strSnapShotUrl))
		{
			continue;
		}

		iRank++;
		BOOL bIsAce = FALSE;
		CatchInfo cInfo;
		DWORD dwWebId = 0;		//网站ID;
		bool bIsOfficialWebFlag = false;	//当前网站是否为官网网址;
		CString strUserfulDomain(_T(""));
		BOOL bFind = LinkUrlIsInOurSitelist(strSnapUrl, strSnapShotUrl, sData, cInfo.bIsOfficialFlag, dwWebId, strUserfulDomain, strPage, bIsOfficialWebFlag);

		if (bFind || cInfo.bIsOfficialFlag)
		{
			int iAllOpenState = -1;
			CString strAllErrorInfo(_T(""));
			int iResult = HasCompanyInHtml(strSnapUrl, sData, cInfo, dwWebId, lp, strSnapShotUrl, strAllRedirectUrl, strUserfulDomain, bIsOfficialWebFlag, iAllOpenState, strAllErrorInfo);
			changeOpenUrlState(iAllOpenState);
			setErrorWebId(strAllErrorInfo);
			if (iResult != FOUND_COMPANY || dwWebId <= 0)
			{
				if (iResult == NOT_FOUND_COMPANY || iResult == PAGE_NOT_FOUND)
				{
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("phone360 ：没有在目标网页中找到公司名"));
				}
				// 				else if (GET_WEBDATA_EXCEPTION == iResult)
				// 				{
				// 					setWebOpenFlag(true);
				// 				}


				if (!cInfo.bIsOfficialFlag)
				{
					continue;
				}
			}
			else
			{
				bIsB2BCatch = TRUE;
				iRankCnt++;
			}

			//如果找到递增1
			if (cInfo.bIsOfficialFlag)
			{
				iRankCnt++;
			}

			if (addPageTags(pElement, cInfo, dwWebId, sData, bIsAce))
			{
				l++;
				lDivCnts++;
			}

			if (vHeights.size() == 0)
			{
				pBdata = new BackDataInfo();
				ASSERT(pBdata != NULL);

				pBdata->iFlag = sData.iFlag;
				pBdata->iRank = sData.iCurPage * 10 + iRank;
				if (0 == sData.iCurPage)
				{
					if (pBdata->iRank > 10)
					{
						pBdata->iRank = 10;
					}
				}
				else if (1 == sData.iCurPage)
				{
					if (pBdata->iRank > 20)
					{
						pBdata->iRank = 20;
					}
				}
				else if (2 == sData.iCurPage)
				{
					if (pBdata->iRank > 30)
					{
						pBdata->iRank = 30;
					}
				}

				pBdata->strKeyWordName = sData.strKeyWordName;
				pBdata->strKey = sData.strKey;
				pBdata->strCompanyName = sData.strComany;
			}
			vHeights.push_back(iRank);
			//如果抓到包含官网的，设置标记为1
			if (cInfo.bIsOfficialFlag)
			{
				pBdata->iOfficialFlag = 1;
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);

				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			//如果抓到包含其他的,设置标记为1
			else if (bIsB2BCatch)
			{
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);

				//是否为2代网站
				if (dwWebId < 40000000 && dwWebId != 4000065)
					pBdata->iB2BFlag = 1;

				//是否为王牌网站
				if (cInfo.bIsAceFlag && bIsAce)
					pBdata->iAceFlag = 1;

				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
			if (pBdata != NULL && 0 == pBdata->iKeywordType)
			{
				pBdata->iKeywordType = IsADWord(sData.iFlag, sData.iIndex, _pAllColls);
			}
		}
	}
	addBackData(pBdata, sData, iRetValue, strFileName, iRankCnt, strWebRecord, strAllRedirectUrl);
	return iRetValue;
}

//修改网页搜索按钮不包含ID的样式
void CWebDlg::ChangeSpecialButtonStyle(CString& strData, const KeyWordDataInfo *pSearchData)
{
	CString strPage;
	CString strUrl;
	strUrl.Format(_T(" onclick=\"javascript:location.href='%s'\" "), pSearchData->strUrl);

	CString strTemp = strData;
	strTemp.MakeLower();
	int iPos1 = strTemp.Find(g_mapElemCfg[pSearchData->iIndex].strBtnId);

	int iPos2 = -1;

	if (iPos1 > 0)
	{
		//如果是button按钮则要将submit 替换成button		
		strData.Replace(_T("<button"), _T("<input"));
		strData.Replace(_T("type=submit"), _T("type=button"));
		strPage = strData.Left(iPos1);
		iPos2 = strPage.ReverseFind(_T('<'));
		strData.Insert(iPos2 + 6, strUrl);
	}

	return;
}

/*
@brief 检测当前网站嵌套网址内是否包含公司标记
@param sData 关键词信息；
@param strData 网站数据；
@return
*/
int CWebDlg::InspectEmbedWeb(CString &strData, CString& _strContentType, const KeyWordDataInfo &sData, int &iResult, CString& _sRedirectUrl, CString _userfulDomain, int& _ErrorCode, long& _iHttpStatusCode)
{

	std::vector<CStdString> vTmp;
	vTmp = g_mapElemCfg[sData.iIndex].vTargetMarkWeb;
	//遍历所有过滤网站
	for (int nWeb = 0; nWeb < vTmp.size(); ++nWeb)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("检测到有嵌入网站需要检测，正在执行:%s"), vTmp[nWeb].c_str());
		if (strData.Find(vTmp[nWeb]) != -1)
		{
			int iGetPreLen = 0;
			int iGetLastLen = 0;
			if (g_mapElemCfg[sData.iIndex].vTargetPreLen.size() > 0)
				iGetPreLen = _ttoi(g_mapElemCfg[sData.iIndex].vTargetPreLen[nWeb]);
			if (g_mapElemCfg[sData.iIndex].vTargetLstLen.size() > 0)
				iGetLastLen = _ttoi(g_mapElemCfg[sData.iIndex].vTargetLstLen[nWeb]);
			int iPos1 = strData.Find(g_mapElemCfg[sData.iIndex].vTargetKey[nWeb]);
			if (iPos1 != -1)
			{
				CString strTmp = _T("");
				//根据给定的关键词查找前面或后面一段数据；
				if (iGetPreLen > 0)
				{
					strTmp = strData.Mid(iPos1 - iGetPreLen, iGetPreLen);
				}
				else
				{
					strTmp = strData.Mid(iPos1, iGetLastLen);
				}
				if (g_mapElemCfg[sData.iIndex].vTargetStart.size() <= 0 ||
					g_mapElemCfg[sData.iIndex].vTargetEnd.size() <= 0)
				{
					g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("读取到嵌入查找数据为空，退出！"));
					break;
				}
				int Ipos2 = strTmp.Find(g_mapElemCfg[sData.iIndex].vTargetStart[nWeb]);
				int iPos3 = strTmp.Find(g_mapElemCfg[sData.iIndex].vTargetEnd[nWeb], Ipos2 + g_mapElemCfg[sData.iIndex].vTargetStart[nWeb].GetLength() + 2);
				if (Ipos2 != -1 && iPos3 != -1 && strTmp.GetLength() > 0)
				{
					CString strTmpUrl = strTmp.Mid(Ipos2 + g_mapElemCfg[sData.iIndex].vTargetStart[nWeb].GetLength() + 1, iPos3 - Ipos2 - g_mapElemCfg[sData.iIndex].vTargetStart[nWeb].GetLength() - 1);
					//如果存在需要替换的连接字符
					strTmpUrl.Replace(_T("&amp;"), _T("&"));
					CStringA strTemp = urlopen(strTmpUrl, _ErrorCode, _iHttpStatusCode);
					//重新将源码转换成对应的字符集，并判断是否包含在官网；
					//					CString strContentType = _T("");
					GetPageData(strTemp, _strContentType, strData, sData.vAllCompanys, _sRedirectUrl, _userfulDomain, _ErrorCode, _iHttpStatusCode);
					iResult = CheckWebFlag(sData.vCompanys, strData, sData.iIndex, sData.vCompanysTag, sData.bOnlyRareWord);
					//原来是没有找到公司名；
					if (iResult == OPEN_PAGE_FAILED)
					{
						iResult == NOT_FOUND_COMPANY;
					}

					break;
				}
			}
		}
	}

	return 0;
}

/*
@breif 查找是否是商贸网站，并近路网站网址；
@param strWebList  网站列表；
@param WebId 查找到的网站ID;
@return True 查找成功；
*/
int CWebDlg::GetWebId(const KeyWordDataInfo &sData, DWORD &dwWebId, const CStringA &pChar)
{
	int iPos1 = 0;
	int iPos2 = 0;
	int iPos3 = 0;
	BOOL bReturn = FALSE;

	CStringA strList = sData.strWebList;
	strList.MakeLower();

	if (pChar.GetLength() > 0)
	{
		iPos1 = strList.Find(pChar);
		if (iPos1 != -1)
		{
			//取当前网址的ID;
			CStringA strFindID = strList.Left(iPos1);
			iPos2 = strFindID.ReverseFind(',');
			iPos3 = strFindID.ReverseFind('\r');
			dwWebId = atol(strFindID.Mid(iPos3 + 1, iPos2 - iPos3 - 1));
			bReturn = TRUE;
		}
	}

	return bReturn;
}

//微信搜索
BOOL CWebDlg::FindIsOwnWebInWeixin(CString strHtml, const KeyWordDataInfo &sData, CString& sSiteName)
{
	int iPos1, iPos2, iLen;
	strHtml.MakeLower();
	CString strTmp = sData.strWeixinName;
	strTmp.MakeLower();
	if (strTmp.GetLength() <= 0)
	{
		return TRUE;
	}

	//预防网址中有加粗
	strHtml.Replace(_T("<b>"), _T(""));
	strHtml.Replace(_T("</b>"), _T(""));
	iPos1 = iPos2 = iLen = 0;
	iPos1 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag1, iPos2);
	iLen = g_mapElemCfg[sData.iIndex].strFindUrlFlag1.GetLength() + 1;
	if (iPos1 != -1)
	{
		iPos2 = strHtml.Find(g_mapElemCfg[sData.iIndex].strFindUrlFlag2, iPos1 + iLen);
		if (iPos2 != -1)
		{
			CString strRes = strHtml.Mid(iPos1 + iLen, iPos2 - iPos1 - iLen);

			if (strTmp == strRes)
			{
				return TRUE;
			}
		}

	}

	return FALSE;

}

/*
@breif 取得微信抓取结果
@param  sData  关键词相关数据
@param  strFileName  要保存的图片路径   输出
@return 执行结果
*/
int CWebDlg::GetWeixinPhoto(IHTMLElementCollection *pTables, const KeyWordDataInfo &sData, CString &strFileName, BOOL &bResult, LPVOID lp)
{
	int iRetValue = OPEN_PAGE_FAILED;
	int iRankCnt = 0;   //抓取到的排名数量
	std::vector<long> vHeights;  //返回高度，可能有多条数据
	CString strWebRecord = _T("");  //网站排名记录

	if (pTables == NULL)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("IHTMLElementCollection weixin is NULL"));
		return iRetValue;
	}

	CComQIPtr<IHTMLElement> pElement;
	BackDataInfo *pBdata = NULL;
	HRESULT hr;

	int iRank = 0;
	long lDivCnts = 0;
	pTables->get_length(&lDivCnts);
	CString strAllRedirectUrl(_T(""));

	for (long l = 0; l < lDivCnts; l++)
	{
		IDispatch *pDisp = NULL;
		CString strTemp;
		BOOL	bIsB2BCatch = FALSE;

		VARIANT index = { 0 };
		V_VT(&index) = VT_I4;
		V_I4(&index) = l;

		hr = pTables->item(COleVariant(l), index, &pDisp);
		if (FAILED(hr) || pDisp == NULL)
			continue;

		Sleep(30);
		pElement = pDisp;
		pDisp->Release();

		CComBSTR bstr;
		pElement->get_innerHTML(&bstr);
		CString strPage(bstr);
		strTemp = strPage;
		if (strPage.GetLength() <= 0)
			continue;
		if ((strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag2) != -1) || (strPage.Find(g_mapElemCfg[sData.iIndex].strItemFlag1) == -1))
		{
			continue;
		}

		iRank++;

		//替换掉所有href标签
		int iPosHref2 = -1;
		int iPosHref = strPage.Find(_T("href=\""));
		while (iPosHref != -1)
		{
			iPosHref2 = strPage.Find(_T("\""), iPosHref + 7);
			CString strTmpHref = strPage.Mid(iPosHref + 6, iPosHref2 - iPosHref - 6);
			strPage.Replace(strTmpHref, _T("javascript:void(0)"));
			iPosHref = strPage.Find(_T("href=\""), iPosHref2);
		}
		strPage.Replace(_T("_blank"), _T(""));

		hr = pElement->put_innerHTML((BSTR)strPage.GetString());

		strPage = strTemp;
		DWORD dwWebId = 40000000;		//网站ID;
		CatchInfo cInfo;
		CString sSiteName = _T("");
		BOOL bFind = FindIsOwnWebInWeixin(strPage, sData, sSiteName);

		if (bFind)
		{
			CString strUrl = GetUrlFormContent(strPage, sData.iIndex);
			strUrl.Replace(_T("&amp;"), _T("&"));
			if (strUrl.Find(_T("http:")) == -1)
				strUrl.Insert(0, _T("http://"));

			CString strUserfulDomain(_T(""));
			bool bIsOfficialWebFlag = false;
			int iAllOpenState = -1;
			CString strAllErrorInfo(_T(""));
			int iResult = HasCompanyInHtml(strUrl, sData, cInfo, dwWebId, lp, sSiteName, strAllRedirectUrl, strUserfulDomain, bIsOfficialWebFlag, iAllOpenState, strAllErrorInfo);
			changeOpenUrlState(iAllOpenState);
			setErrorWebId(strAllErrorInfo);
			if (iResult != FOUND_COMPANY)
			{
				if (iResult == NOT_FOUND_COMPANY || iResult == PAGE_NOT_FOUND)
					g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("微信 ：没有在目标网页中找到公司名"));

				continue;
			}
			else
			{
				bIsB2BCatch = TRUE;
				iRankCnt++;
			}

			BOOL bIsAce;
			if (addPageTags(pElement, cInfo, dwWebId, sData, bIsAce))
			{
				l++;
				lDivCnts++;
			}

			if (vHeights.size() == 0)
			{
				pBdata = new BackDataInfo();
				ASSERT(pBdata != NULL);

				pBdata->iFlag = sData.iFlag;
				pBdata->iRank = sData.iCurPage * 10 + iRank;
				if (0 == sData.iCurPage)
				{
					if (pBdata->iRank > 10)
					{
						pBdata->iRank = 10;
					}
				}
				else if (1 == sData.iCurPage)
				{
					if (pBdata->iRank > 20)
					{
						pBdata->iRank = 20;
					}
				}
				else if (2 == sData.iCurPage)
				{
					if (pBdata->iRank > 30)
					{
						pBdata->iRank = 30;
					}
				}

				pBdata->strKeyWordName = sData.strKeyWordName;
				pBdata->strKey = sData.strKey;
				pBdata->strCompanyName = sData.strComany;
			}
			vHeights.push_back(iRank);

			//如果抓到包含其他的,设置标记为1
			if (bIsB2BCatch)
			{
				//记录下网站ID对应排名
				strTemp.Format(_T("%d,%d"), dwWebId, sData.iCurPage * 10 + iRank);
				if (strWebRecord.GetLength() <= 0)
				{
					strWebRecord = strTemp;
				}
				else
				{
					strWebRecord += _T("(;2)") + strTemp;
				}
			}
		}
	}
	addBackData(pBdata, sData, iRetValue, strFileName, iRankCnt, strWebRecord, strAllRedirectUrl);
	return iRetValue;
}

/*
@brief 去除整个网页源码中的<a>标签
@param strData 网页源数据
@param strTag 要替换的开始标签
@param strTag 要替换的结束标签
*/
int CWebDlg::ReplaceTag(CString &strData, const CString& strStartTag, const CString& strEndTag)
{
	CString strTag1;
	CString strTag2;
	CString strItem1;
	int iPos = -1;
	int iPos1 = -1;
	if (strData.GetLength() < 1)
	{
		return 0;
	}

	strTag1 = strStartTag;
	strTag1 = strTag1.Trim();

	strTag2 = strTag1.Right(1);
	strTag1 = strTag1.Left(strTag1.GetLength() - 1);
	//先替换标签头 <a href ……>
	iPos = strData.Find(strTag1);
	while (iPos != -1)
	{
		iPos1 = strData.Find(strTag2, iPos);
		strItem1 = strData.Mid(iPos, iPos1 - iPos + 1);
		strData.Replace(strItem1, _T(""));
		iPos = strData.Find(strTag1);
	}

	//最后将所有的标签结束符替换掉</a>

	strData.Replace(strEndTag, _T(""));

	return 1;

}

BOOL CWebDlg::CheckAceWeb(const CString& strData, DWORD &dwWebId, int dwIndex)
{
	//查找是否包含王牌网站
	if (strData.Find(_T("id=\"hide_web_for_qkphoto\"")) == -1
		&& strData.Find(_T("id=hide_web_for_qkphoto")) == -1)
		return FALSE;
	//查找ID是多少
	for (auto web : g_mapElemCfg[dwIndex].mapAceWebToID)
	{
		if (strData.Find(web.first) != -1)
		{
			dwWebId = web.second;
			break;
		}
	}

	return TRUE;
}

bool CWebDlg::keyFingerSearch(CString _str, sFingerRecogn _Finger, const std::vector<CStdString> &vCompanys)
{
	CString strFind = _str;
	bool bHasFind = false;
	int iFoundNum = 0;

	//查找生僻字
	for (int i = 0; i < _Finger.iCount; i++)
	{
		if (!_Finger.vsCharData[i].IsEmpty() && (-1 != strFind.Find(_Finger.vsCharData[i])))
		{
			iFoundNum++;
			if (iFoundNum >= _Finger.iMatchNum)
			{
				bHasFind = true;
				break;
			}
		}
	}
	//查找公司名
	if (bHasFind)
	{
		for (int i = 0; i < vCompanys.size(); ++i)
		{
			if (strFind.Find((CString)(vCompanys[i])) != -1)
			{
				return true;
			}
		}
	}

	return false;
}

/*
@brief 对Win8以下系统进行提权
*/
BOOL CWebDlg::RaisePrivileges()
{
	HANDLE TokenHandle;
	TOKEN_PRIVILEGES t_privileges = { 0 };

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))
	{
		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &t_privileges.Privileges[0].Luid))
	{
		return TRUE;
	}

	t_privileges.PrivilegeCount = 1;
	t_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(TokenHandle, FALSE, &t_privileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		CloseHandle(TokenHandle);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*
@brief 设置IE运行模式
@param eVer IE版本
*/
void CWebDlg::SetIECoreVersion(E_IeLimit eVer)
{
	HKEY hMainKey = HKEY_CURRENT_USER;
	if (IsWOW64())
	{
		hMainKey = HKEY_LOCAL_MACHINE;
	}
	TCHAR* path = _T("SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION");
	TCHAR* valueName = _T("UpdateRank.exe");
	long version = 8888;
	HKEY hKey;
	DWORD dwDisposition;
	//获取IE版本的值
	switch (eVer)
	{
	case IE6:
		version = 6000;
		break;
	case IE7:
		version = 7000;
		break;
	case IE8:
		version = 8888;
		break;
	case IE9:
		version = 9999;
		break;
	case IE10:
		version = 10001;
		break;
	case IE11:
		version = 11001;
		break;
	default:
		version = 8888;
		break;
	}
	long ret = RegOpenKeyEx(hMainKey, path, 0, REG_LEGAL_OPTION, &hKey);
	if (ret != ERROR_SUCCESS)
	{

		ret = RegCreateKeyEx(hMainKey, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition);
		if (ret != ERROR_SUCCESS)
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("写入IE模式失败，IE版本：%d"), version);
			return;
		}
	}

	g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("写入IE版本：%d"), version);

	ret = RegSetValueEx(hKey, valueName, NULL, REG_DWORD, (BYTE*)&version, sizeof(version));

	RegCloseKey(hKey);
	if (ret != ERROR_SUCCESS)
		return;
}

/*
@breif 判断是否64位系统
*/
BOOL CWebDlg::IsWOW64()
{
	BOOL bRetVal = FALSE;
	SYSTEM_INFO si = { 0 };
	LPFN_PGNSI pGNSI = (LPFN_PGNSI)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetNativeSystemInfo");
	if (pGNSI == NULL)
	{
		return FALSE;
	}
	pGNSI(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
	{
		bRetVal = TRUE;
	}
	return bRetVal;
}


// int CWebDlg::GetIEVersion()
// {
// 	TCHAR szFilename[] = _T("mshtml.dll");
// 
// 	int	  iVer = -1;
// 	DWORD dwMajorVersion = 0;
// 	DWORD dwHandle = 0;
// 
// 	DWORD dwVerInfoSize = GetFileVersionInfoSize(szFilename, &dwHandle);
// 
// 	if (dwVerInfoSize)
// 	{
// 		LPVOID lpBuffer = LocalAlloc(LPTR, dwVerInfoSize);
// 		if (lpBuffer)
// 		{
// 			if (GetFileVersionInfo(szFilename, dwHandle, dwVerInfoSize, lpBuffer))
// 			{
// 				VS_FIXEDFILEINFO * lpFixedFileInfo = NULL;
// 				UINT nFixedFileInfoSize = 0;
// 				if (VerQueryValue(lpBuffer, TEXT("\\"), (LPVOID*)&lpFixedFileInfo, &nFixedFileInfoSize) && (nFixedFileInfoSize))
// 				{
// 					dwMajorVersion = HIWORD(lpFixedFileInfo->dwFileVersionMS);
// 					g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("当前IE版本为：%d"), dwMajorVersion);
// 				}
// 			}
// 			LocalFree(lpBuffer);
// 		}
// 	}
// 
// 	switch (dwMajorVersion)
// 	{
// 	case 6:
// 		iVer = 0;
// 		break;
// 	case 7:
// 		iVer = 1;
// 		break;
// 	case 8:
// 		iVer = 2;
// 		break;
// 	case 9:
// 		iVer = 3;
// 		break;
// 	case 10:
// 		iVer = 4;
// 		break;
// 	case 11:
// 		iVer = 5;
// 		break;
// 	default:
// 		iVer = 2;
// 		g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("获取当前IE版本失败，默认为IE8"));
// 		break;
// 	}
// 
// 	return iVer;
// }


/*
@breif 设置IE模式
*/
void CWebDlg::SetIEMode(E_IeLimit eVer)
{
	RaisePrivileges();
	SetIECoreVersion(eVer);
}
