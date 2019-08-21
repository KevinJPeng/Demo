/*

 GetData.cpp
 author: larry
 从服务器取得发布数据类


*/


#include "StdAfx.h"
#include "GetData.h"
#include "json/json.h"
#include "SQLiteOperate.h"
#include "Reg.h"
#include "Base64.h"
#include "UrlOperate.h"

#include <time.h>
#include <string>
#include <afxinet.h>
#include <Iphlpapi.h>

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "..\\..\\lib\\tinyxml.lib")

//
//#ifdef _DEBUG
//#pragma comment(lib, "..\\..\\lib\\json_vc71_libmtd.lib")
//#else
//#pragma comment(lib, "..\\..\\lib\\json_vc71_libmt.lib")
//#endif

#define GETPWDSTRING  _T("/api/DesktopAutoLogin/GetData")
#define GETDATASTRING  _T("/api/Desktop")
#define GETTUISONGMSG _T("/api/Client/SendMsg")
#define GETJZMSG _T("/api/Order/GetPurchasedProduct")
#define BUYPRODUCT _T("/api/Client/GetBuyProductUrl")
#define DASHIZHENCIOFF _T("/api/Member/GetClentAnnualTips")



CGetData::CGetData(void)
:IThreadUnit(E_THREAD_DATASTATISTICS, 0xFFFF)
	, iProduceId(0)
	, iWeiXinId(0)
	, iExressVersionId(0)
	, iJzId(0)
	, iIsBuyWeixin(0)
	, iIsBuyJz(0)
	, bBack(FALSE)
{
	m_strZCApi = _T("");
	CString strInitPath = _T("");
	strInitPath.Format(_T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());

	CString szService = _T("");
	IXMLRW xml;
	xml.init(strInitPath);
	xml.ReadString(_T("MC"), _T("DASHIZHENCI"), _T("apiAddress"), szService);

	m_strZCApi.Format(_T("%s"), szService);

	HANDLE hThread = CreateThread(NULL, 0, MessageThread, this, 0, &dwThreadId);
	if (hThread)
		CloseHandle(hThread);

	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
}


CGetData::~CGetData(void)
{
	if (hEvent != NULL)
		CloseHandle(hEvent);
}

DWORD CGetData::DispatchMessage(T_Message *pMsg)
{
	

	switch (pMsg->dwMsg)
	{
	case MSG_TIMER:
		{
			//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到Timer消息！"));
		}
		break;
	case MSG_SEVEN_DATA:
	case MSG_POST_DETAIL_DATA:
	case MSG_LOGIN_SHANGZHOU:
	case MSG_LOGIN_CLIENT:
	case MSG_PRODUCT_POST:
	case MSG_LOGIN_WEIXIN:
	case MSG_WEIXIN_DATA:
	case MSG_KEYWORD_PRODUCT_DATA:
	case MSG_KEYWORD_RESULT_DATA:
	case MSG_UPDATE_CONFIG_TO_LOCAL:
	case MSG_GET_DELAYMSG:
	case MSG_GET_JZ_INFORMATION:
	case MSG_BUY_PRODUCT:
	case MSG_SUBMIT_USERINFO:
	case MSG_ZHENCI_INFO:
	case MSG_START_PAIMING:
		{
			T_Message *tMsg = IMsgQueue::New_Message();
			/*PassInfo *passInfo = new PassInfo;

			if (pMsg->lParam != NULL && passInfo != NULL)
				*passInfo = *(PassInfo*)pMsg->lParam;*/

			tMsg->dwDestWork = pMsg->dwDestWork;
			tMsg->dwMsg = pMsg->dwMsg;
			tMsg->dwMsgType = pMsg->dwMsgType;
			tMsg->dwSourWork = pMsg->dwSourWork;
			tMsg->lParam = pMsg->lParam;
			tMsg->wParam = pMsg->wParam;

			//g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到非定时消息"));


			if (PostThreadMessage(dwThreadId, pMsg->dwMsg, 0, (LPARAM)tMsg))
			{
				if (pMsg->dwMsg == MSG_LOGIN_CLIENT)
				{
					if (WaitForSingleObject(hEvent, 5000) == WAIT_OBJECT_0)
						g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("登录等待事件对象成功"));
					else
						g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("登录等待事件对象失败"));
				}
			}
			else
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("PostThreadMessage失败的"));
			}
		}
		break;

	//case MSG_SUBMIT_USERINFO:
	//	{
	//		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到提交数据消息"));
	//		PostThreadMessage(dwThreadId, pMsg->dwMsg, 0, 0);
	//	}
	//	break;
	case MSG_SALF_EXIT:
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到退出消息，转发"));
			PostThreadMessage(dwThreadId, pMsg->dwMsg, 0, 0);
		}
		break;

	case MSG_LOGIN_CANCEL:
		{
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("收到用户取消登录消息"));

			if (WaitForSingleObject(hEvent, 5000) == WAIT_OBJECT_0)   //这里保证取消登录时， 返回消息标志已经被之前登录消息处理过
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("取消登录等待事件对象成功"));
			else
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("取消登录等待事件对象失败"));
			

			SetEvent(hEvent);

			m_Lock.Lock();
			bBack = FALSE;
			m_Lock.Unlock();
		}
		
		break;
	default:
		break;
	}

	return 0;
}

/*
	@brief  从服务器取得数据
	@param  pMsg   消息
	@param  iType  类型  1:获取用户最近七天发布成功统计数据，2：获取历史推广数据，3：登录商舟网，4：登录客户端，5 : 商务快车快捷推广，6 : 商舟微信通登录，7 : 商舟微信通运营图表
	@return 返回数据
	*/
CString CGetData::GetJsonFromServer(T_Message *pMsg, int iType)
{
	CString strUrl;

	PassInfo  *pInfo = (PassInfo*)pMsg->lParam;
	if (pInfo != NULL)
	{
		
		//登录客户端后面加一个MAC地址
		if (iType == TYPE_LOGINCLIENT)
		{
			CString strMac = GetPhysicalAddress();
			//if (strMac.IsEmpty())
			//{
			//	strMac = _T("00-00-00-00-00-00");
			//}
			strUrl.Format(_T("%s%s?%s&Type=%d&Mac=%s"), pInfo->strUrl, GETDATASTRING, pInfo->strParam, iType,strMac);
		}
		else
		{	
		
			strUrl.Format(_T("%s%s?%s&Type=%d"), pInfo->strUrl, GETDATASTRING, pInfo->strParam, iType);
		}
		
		if ((iType >= TYPE_SEVERNDATA && iType <= TYPE_QUICKPOST)
			|| (iType >= TYPE_KEYWORD_RESULT && iType <= TYPE_KEYWORD_PRODUCT))
		{
			if (iProduceId != 0)
			{
				strUrl.AppendFormat(_T("&ProductId=%d"), iProduceId);
			}
		}
		else if (iType >= TYPE_LOGINWEIXIN && iType <= TYPE_WEIXININFO)
		{
			if (iWeiXinId != 0)
			{
				strUrl.AppendFormat(_T("&ProductId=%d"), iWeiXinId);
			}
		}
		/*else if (iType >= TYPE_KEYWORD_RESULT && iType <= TYPE_KEYWORD_ONLINE)
		{

		}*/
		
	}

	return GetUrlRespones(strUrl);
}

 //CString CGetData::GetJsonFromServer(T_Message *pMsg, int iType)
 //{
 //	CInternetSession m_session(_T("HttpClient"));
 //	CHttpFile *pFile = NULL;
 //	CHttpConnection *pConnection = NULL;
 //	CString strData;
 //	CString strUrl;
 //	CString strServer;
 //	CString strObject;
 //	DWORD dwServiceType;
 //	INTERNET_PORT nPort;
 //
 //
 //	strUrl.Format(_T("%s?%s&Type=%d"), (TCHAR*)pMsg->lParam, (TCHAR*)pMsg->wParam, iType);
 //	strUrl.Replace(_T("+"), _T("%2B"));
	//strUrl = _T("http://seo.chinaz.com/?host=www.sumsz.com");
 //	AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort);
 //	if(AFX_INET_SERVICE_HTTP != dwServiceType && AFX_INET_SERVICE_HTTPS != dwServiceType)
 //	{
 //		
 //		return strData;
 //	}
 //
 //	try
 //	{
 //		pConnection = m_session.GetHttpConnection(strServer, nPort);
 //
 //		if (pConnection == NULL)
 //		{
 //			
 //			return strData;
 //		}
 //
 //		pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject.GetString());
 //
 //		if (pFile != NULL)
 //		{
 //			DWORD dwRet = 0;
 //			pFile->AddRequestHeaders(_T("Accept: application/xhtml+xml"));
 //			pFile->AddRequestHeaders(_T("Accept-Language: zh-CN"));
 //			pFile->AddRequestHeaders(_T("Content-Type:application/xhtml+xml; charset=utf-8"));  //统一用utf-8防止乱码
 //
 //			//bSucce = pFile->AddRequestHeaders(_T("User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)"));
 //			//bSucce = pFile->AddRequestHeaders(_T("Accept-Encoding: gzip, deflate"));
 //
 //			pFile->SendRequest();
 //			pFile->QueryInfoStatusCode(dwRet);

 //
 //			pFile->SetOption(INTERNET_OPTION_SEND_TIMEOUT, 30*1000, 0);
 //			pFile->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 30*1000, 0);
 //			if (dwRet == HTTP_STATUS_OK)
 //			{
 //				DWORD dwRead = 0;
 //				char  szBufa[8192] = {0};
 //				wchar_t szBufw[8192] = {0};
 //				DWORD dwCode = CP_UTF8;
 //
 //				CString strTemp;
 //				strData.Empty();
 //				do 
 //				{
 //					dwRead = pFile->Read(szBufa, sizeof(szBufa)-1);
 //
 //					if (strData.IsEmpty())
 //					{
 //						strTemp= szBufa;
 //						strTemp.MakeLower();
 //						if (strTemp.Find(_T("utf-8")) != -1)
 //							dwCode = CP_UTF8;
 //					}
 //
 //					MultiByteToWideChar( dwCode, 0, szBufa,
 //						strlen(szBufa)+1, szBufw, sizeof(szBufw)/sizeof(szBufw[0]) );
 //
 //					strData.Append(szBufw);
 //					memset(szBufa, 0, sizeof(szBufa));
 //					memset(szBufw, 0, sizeof(szBufw));
 //
 //				} while (dwRead > 0);
 //			}
 //
 //			
 //
 //			pFile->Close();
 //		}
 //		pConnection->Close();
 //		m_session.Close();
 //	}
 //	catch (...)
 //	{
 //		
 //		return strData;
 //	}
 //
 //
 //
 //	return strData;
 //}


/*
	@brief  取得近7天用户所有数据
	@param  pMsg 消息
	*/
void CGetData::HandleSevenDataMessage(T_Message *pMsg)
{
	CString strData = GetJsonFromServer(pMsg, TYPE_SEVERNDATA);
	PassInfo *pInfo = (PassInfo*)pMsg->lParam;
	char *pUserName = NULL;

	SEVEEN_DATA sevenData_server;
	char *pBuf = WideToMulti(strData.GetString());

	ParseData(pBuf, sevenData_server);
	if (pInfo != NULL)
		pUserName = WideToMulti(pInfo->strUserName.GetString());

	if (sevenData_server.iSuccessFlag == 1)
	{
		IsSaveToLocal(pUserName, pBuf, TYPE_SEVERNDATA);
	}
	else
	{
		//从服务器取数据失败
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("从服务器取近七天数据失败"));
		GetSevenDataFromDb(pUserName, sevenData_server);
	}

	T_Message *pBackMsg = IMsgQueue::New_Message();
	pBackMsg->dwDestWork = pMsg->dwSourWork;
	pBackMsg->dwSourWork = pMsg->dwDestWork;
	pBackMsg->dwMsg = pMsg->dwMsg;
	pBackMsg->wParam = 0;
	pBackMsg->lParam = (LPARAM)&sevenData_server;

	SendMessage(pBackMsg);

	if (pUserName)
		delete pUserName;
		
	if (pBuf)
		delete pBuf;
	
}

/*
	@brief  登录客户端
	@param  pMsg 消息
	*/
void CGetData::LoginClient( T_Message *pMsg )
{
	m_Lock.Lock();
	bBack = TRUE;       //默认是返回消息
	m_Lock.Unlock();

	SetEvent(hEvent);  //如遇到取消登录时，确保返回消息标志已经处理
	iProduceId = 0;    //将产品id置为0
	iWeiXinId = 0;
	
	CString strData = GetJsonFromServer(pMsg, TYPE_LOGINCLIENT);
	char *pBuf = WideToMulti(strData.GetString());

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("登录需要返回的json:%s"), strData.GetString());

	int iResult = 0;
	if (pBuf != NULL)
	{
		Json::Reader reader;
		Json::Value json_object;

		if (reader.parse(pBuf, json_object))
		{
			iResult = json_object["ExtensionData"]["CallResult"].asInt();
			iProduceId = json_object["Data"]["NewExressProductId"].asInt();
			iWeiXinId = json_object["Data"]["WeiXinProductId"].asInt();
			iExressVersionId = json_object["Data"]["NewExressVersionId"].asInt();
			iJzId = json_object["Data"]["JzProductId"].asInt();

			iIsBuyWeixin = json_object["Data"]["IsBuyWexin"].asInt();
			iIsBuyJz = json_object["Data"]["IsBuyJz"].asInt();
		}
		delete pBuf;
	}

	if (bBack)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("登录需要返回消息"));

		PassInfo *pInfo = (PassInfo*)pMsg->lParam;

		pInfo->iLoginType = pMsg->wParam;
		pInfo->iExressVersionId = iExressVersionId;
		pInfo->iWeiXinId = iWeiXinId;
		pInfo->iJzId = iJzId;
		pInfo->iProductId = iProduceId;

		T_Message *pBackMsg = IMsgQueue::New_Message();
		pBackMsg->dwDestWork = pMsg->dwSourWork;
		pBackMsg->dwSourWork = pMsg->dwDestWork;
		pBackMsg->dwMsg = pMsg->dwMsg;
		pBackMsg->wParam = iResult;
		pBackMsg->lParam = pMsg->lParam;
		PostMessage(pBackMsg);
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("登录遇到取消消息"));
	}
	
}


/*
	@brief  取得产品推广详情
	@param  pMsg 消息
	*/
void CGetData::HandleProductPostDetail( T_Message *pMsg )
{
	CString strData = GetJsonFromServer(pMsg, TYPE_HISTORY);
	PassInfo *pInfo = (PassInfo*)pMsg->lParam;
	char *pUserName = NULL;

	ProductDetail productData_server;
	char *pBuf = WideToMulti(strData.GetString());

	ParseData(pBuf, productData_server);
	if (pInfo != NULL)
		pUserName = WideToMulti(pInfo->strUserName.GetString());

	if (productData_server.iSuccessFlag == 1)
	{
		IsSaveToLocal(pUserName, pBuf, TYPE_HISTORY);
	}
	else
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("从服务器取产品推广详情数据失败"));
		GetSevenDataFromDb(pUserName, productData_server);
	}
	
	T_Message *pBackMsg = IMsgQueue::New_Message();
	pBackMsg->dwDestWork = pMsg->dwSourWork;
	pBackMsg->dwSourWork = pMsg->dwDestWork;
	pBackMsg->dwMsg = pMsg->dwMsg;
	pBackMsg->wParam = 0;
	pBackMsg->lParam = (LPARAM)&productData_server;
	SendMessage(pBackMsg);


	if (pUserName)
		delete pUserName;

	if (pBuf)
		delete pBuf;
}


/*
	@brief  登录商舟网  或开通微信通   开通建站系统
	@param  pMsg 消息
	*/
void CGetData::LoginShangzhou( T_Message *pMsg )
{
	CString strUrl;
	PassInfo *pInfo = (PassInfo*)pMsg->lParam;

	if (pInfo != NULL)
	{
		if (pMsg->dwMsg == MSG_LOGIN_SHANGZHOU)
		{
			strUrl.Format(_T("%s%s?%s&Type=%d"), pInfo->strUrl, GETDATASTRING, pInfo->strParam, TYPE_LOGINSHANGZHOU);


			/*if (iProduceId != 0)
				strUrl.AppendFormat(_T("&ProductId=%d"), iProduceId);*/
		}
		else
		{
			strUrl.Format(_T("%s%s?%s&Type=%d"), pInfo->strUrl, GETDATASTRING, pInfo->strParam, TYPE_LOGINWEIXIN);
			if (iWeiXinId != 0)
				strUrl.AppendFormat(_T("&ProductId=%d"), iWeiXinId);
		}	
	}

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("登录商舟网:%s"), strUrl);

	ShellExecute(NULL, _T("open"), strUrl.GetString(), NULL, NULL, SW_NORMAL);
}

bool CGetData::GetURL(CStdString& strURL)
{	
	CString strURLTemp = _T("");
	CString strCfgFile = _T("");
	strCfgFile.Format(_T("%s\\data2\\mc.dat"), GetInstallPath());
	
	IXMLRW iniFile;
	iniFile.init(strCfgFile);

	iniFile.ReadString(_T("MC"), _T("LOGININFO"), _T("url"), strURLTemp);
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("获取到配置文件的API线上地址为%s！"), strURLTemp);

	//如果为空，则取[SERVICE] Current 
	if (strURLTemp.IsEmpty())
	{
		strURL = _T("http://api.sumszw.com");
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("API配置为空,使用默认API上线地址：http://api.sumszw.com"));
	}
	return true;
}

/*
	@brief  将服务器取得的数据插入数据库
	@param  pUserName  用户名  数据与用户名关联
	@param  pJson  要插入的数据字符串
	@param  iType  1表示近七天数据，2表示产品推广详情 7为微信通数据
	*/
void CGetData::InsertData(const char *pUserName, const char *pJson, int iType)
{
	try
	{
		if (pJson != NULL)
		{
			CStringA strDate;
			CStringA strSql;
			time_t timep;
			struct tm *p;

			time(&timep);
			p = localtime(&timep); /*取得当地时间*/
			strDate.Format("%d%02d%02d", p->tm_year+1900, p->tm_mon + 1, p->tm_mday);

			int iLen = strlen(pJson);
			strSql.Format("insert into ServerData(szData\
						  ,type\
						  ,UserName\
						  ,insertDate) values ('%s', %d, '%s', '%s')"
						  , pJson
						  , iType
						  , pUserName
						  , strDate);

			g_pGlobalData->sqlIte.DirectStatement(strSql.GetString());
		}
		

	}
	catch (...)
	{

		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("插入数据出现异常"));
	}
}

/*
	@brief  从本地数据库取数据
	@param  pUserName 用户名
	@param  [in out]sevenData  返回数据
	*/
void CGetData::GetSevenDataFromDb(const char *pUserName, SEVEEN_DATA &sevenData)
{
	CStringA strData;

	sevenData.iSuccessFlag = 0;
	GetDataFromDb(TYPE_SEVERNDATA, pUserName, strData);

	ParseData(strData.GetString(), sevenData);
}

void CGetData::GetSevenDataFromDb(const char *pUserName, ProductDetail &productData )
{
	CStringA strData;

	productData.iSuccessFlag = 0;
	GetDataFromDb(TYPE_HISTORY, pUserName, strData);

	typeid(productData).name();

	ParseData(strData.GetString(), productData);
}

void CGetData::GetSevenDataFromDb( const char *pUserName, WEIXINTONG_DATA &weixinData )
{
	CStringA strData;

	weixinData.iSuccessFlag = 0;
	GetDataFromDb(TYPE_WEIXININFO, pUserName, strData);

	ParseData(strData.GetString(), weixinData);
}

void CGetData::GetSevenDataFromDb( const char *pUserName, KEYWORDSDETAILRESPONSELIST &keyResult )
{
	CStringA strData;

	keyResult.iSuccessFlag = 0;
	GetDataFromDb(TYPE_KEYWORD_RESULT, pUserName, strData);

	ParseData(strData.GetString(), keyResult);
}

void CGetData::GetSevenDataFromDb( const char *pUserName, KEYWORD_PRODUCT_LIST &keyOnline )
{
	CStringA strData;

	keyOnline.iSuccessFlag = 0;
	GetDataFromDb(TYPE_KEYWORD_PRODUCT, pUserName, strData);

	ParseData(strData.GetString(), keyOnline);
}

/*
@brief  取得数据库表格中某数据最新一次插入数据的日期
@param  表格名
@param  用户名
@param  类型 1为近七天数据，2为推广产品详情 7为微信通数据
@return 返回日期，如果为空表示数据库还没有数据
changed by zhoulin
*/
CStringA CGetData::GetRecentDateInTable(const char *pTableName, const char *pUserName, int iType)
{
	CStringA strDate;

	try
	{
		CStringA  strSql;
		ResultTable resTable;
		ResultRecord *resRecord = NULL;

		strSql.Format("SELECT insertDate  FROM %s where type=%d and UserName='%s' order by id desc", pTableName, iType, pUserName);

		if (g_pGlobalData->sqlIte.SelectStmt(strSql.GetString(), resTable))
		{

			if ((resRecord = resTable.next()) != NULL)
			{
				strDate = resRecord->fields_[0].c_str();
			}
		}

	}
	catch (...)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("取得表最新插入时间出现异常"));
	}

	return strDate;
}

/*
	@brief  取得数据库表格中某数据最新一次插入数据的日期
	@param  表格名
	@param  类型 1为近七天数据，2为推广产品详情 7为微信通数据
	@return 返回日期，如果为空表示数据库还没有数据
	*/
//CStringA CGetData::GetRecentDateInTable(const char *pTableName, int iType)
//{
//	CStringA strDate;
//
//	try
//	{
//		CStringA  strSql;
//		ResultTable resTable;
//		ResultRecord *resRecord = NULL;
//
//		strSql.Format("SELECT insertDate  FROM %s where type=%d order by id desc", pTableName, iType);
//
//		if (g_pGlobalData->sqlIte.SelectStmt(strSql.GetString(), resTable))
//		{
//
//			if ((resRecord = resTable.next()) != NULL)
//			{
//				strDate = resRecord->fields_[0].c_str();
//			}
//		}		
//		
//	}
//	catch (...)
//	{
//		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("取得表最新插入时间出现异常"));
//	}
//
//	return strDate;
//}

/*
	@brief  解析json数据
	@param  pJson 要解析的json字符串
	@param  [in out]sevenData 近七天数据
	*/
void CGetData::ParseData( const char *pJson, SEVEEN_DATA &sevenData )
{
	sevenData.iSuccessFlag = 0;
	if (pJson != NULL)
	{
		Json::Reader reader;
		Json::Value json_object;

		if (reader.parse(pJson, json_object))
		{
			int iResult = json_object["ExtensionData"]["CallResult"].asInt();

			if (iResult == 1)  //请求成功
			{
				Json::Value json_Data = json_object["Data"];

				int iSize = json_Data["PublishSuccessList"].size();
				for(int i=0; i<iSize; i++)
				{
					PublishSuccess postSuccess;
					postSuccess.iAllCount = json_Data["PublishSuccessList"][i]["PublishAllCount"].asInt();
					strncpy(postSuccess.szDate, json_Data["PublishSuccessList"][i]["Daytime"].asCString(), DATETIMELEN-1);
					sevenData.postSuccessList.push_back(postSuccess);
				}

				sevenData.postDetail.iCompanySubjectCount = json_Data["PublishDetail"]["CompanySubjectCount"].asInt();
				sevenData.postDetail.iCompanySuccCount = json_Data["PublishDetail"]["CompanySuccCount"].asInt();
				sevenData.postDetail.iProcurementSubjectCount = json_Data["PublishDetail"]["ProcurementSubjectCount"].asInt();
				sevenData.postDetail.iProcurementSuccCount = json_Data["PublishDetail"]["ProcurementSuccCount"].asInt();
				sevenData.postDetail.iProductSubjectCount = json_Data["PublishDetail"]["ProductSubjectCount"].asInt();
				sevenData.postDetail.iProductSuccCount = json_Data["PublishDetail"]["ProductSuccCount"].asInt();
				sevenData.postDetail.iSearchEngineSubjectCount = json_Data["PublishDetail"]["SearchEngineSubjectCount"].asInt();
				sevenData.postDetail.iSearchEngineSuccCount = json_Data["PublishDetail"]["SearchEngineSuccCount"].asInt();
				strncpy(sevenData.postDetail.szDate, json_Data["PublishDetail"]["Daytime"].asCString(), DATETIMELEN-1);


				sevenData.postYesterDay.iCompanyRegisterSuccCount = json_Data["YesterDayPublishInfo"]["CompanyRegisterSuccCount"].asInt();
				sevenData.postYesterDay.iPublishAllCount = json_Data["YesterDayPublishInfo"]["PublishAllCount"].asInt();
				sevenData.postYesterDay.iSubjectCount = json_Data["YesterDayPublishInfo"]["SubjectCount"].asInt();
			}

			sevenData.iSuccessFlag = iResult;     //结果
		}
	}
}

/*
	@brief  解析json数据
	@param  pJson 要解析的json字符串
	@param  [in out]proDuct 产品推广详情
	*/
void CGetData::ParseData( const char *pJson, ProductDetail &proDuct )
{
	proDuct.iSuccessFlag = 0;
	if (pJson != NULL)
	{
		Json::Reader reader;
		Json::Value json_object;

		if (reader.parse(pJson, json_object))
		{
			int iResult = json_object["ExtensionData"]["CallResult"].asInt();

			if (iResult == 1)  //请求成功
			{
				Json::Value json_Data = json_object["Data"];


				proDuct.hisInfo.iCompanyRegisterSuccTotalCount = json_Data["HistoryPublishInfo"]["CompanyRegisterSuccTotalCount"].asInt();
				proDuct.hisInfo.iPublishSubjectTotalCount = json_Data["HistoryPublishInfo"]["PublishSubjectTotalCount"].asInt();
				proDuct.hisInfo.iPublishSuccTotalCount = json_Data["HistoryPublishInfo"]["PublishSuccTotalCount"].asInt();


				proDuct.difInfo.iCompanySubjectTotalCount = json_Data["DifferentPublishHistoryInfo"]["CompanySubjectTotalCount"].asInt();
				proDuct.difInfo.iCompanySuccTotalCount = json_Data["DifferentPublishHistoryInfo"]["CompanySuccTotalCount"].asInt();
				proDuct.difInfo.iProcurementSubjectTotalCount = json_Data["DifferentPublishHistoryInfo"]["ProcurementSubjectTotalCount"].asInt();
				proDuct.difInfo.iProcurementSuccTotalCount = json_Data["DifferentPublishHistoryInfo"]["ProcurementSuccTotalCount"].asInt();
				proDuct.difInfo.iProductSubjectTotalCount = json_Data["DifferentPublishHistoryInfo"]["ProductSubjectTotalCount"].asInt();
				proDuct.difInfo.iProductSuccTotalCount = json_Data["DifferentPublishHistoryInfo"]["ProductSuccTotalCount"].asInt();
				proDuct.difInfo.iSearchEngineSubjectTotalCount = json_Data["DifferentPublishHistoryInfo"]["SearchEngineSubjectTotalCount"].asInt();
				proDuct.difInfo.iSearchEngineSuccTotalCount = json_Data["DifferentPublishHistoryInfo"]["SearchEngineSuccTotalCount"].asInt();
			}

			proDuct.iSuccessFlag = iResult;
		}
	}
}

/*
	@brief  解析json数据
	@param  pJson 要解析的json字符串
	@param  [in out]weixinData 微信通数据
	*/
void CGetData::ParseData( const char *pJson, WEIXINTONG_DATA &weixinData )
{
	weixinData.iSuccessFlag = 0;
	if (pJson != NULL)
	{
		Json::Reader reader;
		Json::Value json_object;

		if (reader.parse(pJson, json_object))
		{
			int iResult = json_object["ExtensionData"]["CallResult"].asInt();

			if (iResult == 1)  //请求成功
			{
				Json::Value json_Data = json_object["Data"];

				int iSize = json_Data["RequestChartList"].size();
				for(int i=0; i<iSize; i++)
				{
					RequestChart requestData;

					requestData.iTextReply = json_Data["RequestChartList"][i]["TextReply"].asInt();
					requestData.iMenuClick = json_Data["RequestChartList"][i]["MenuClick"].asInt();
					requestData.iLocationRequest = json_Data["RequestChartList"][i]["LocationRequest"].asInt();
					requestData.iTotalRequest = json_Data["RequestChartList"][i]["TotalRequest"].asInt();
					
					strncpy(requestData.szDate, json_Data["RequestChartList"][i]["Date"].asCString(), DATETIMELEN-1);
					weixinData.requestChartList.push_back(requestData);
				}

				iSize = json_Data["SubscribeChartList"].size();
				for(int i=0; i<iSize; i++)
				{
					SubscribeChart subscribeData;

					subscribeData.iSubscribNum = json_Data["SubscribeChartList"][i]["SubscribNum"].asInt();
					subscribeData.iUnSubscribNum = json_Data["SubscribeChartList"][i]["UnSubscribNum"].asInt();
					subscribeData.iAddNum = json_Data["SubscribeChartList"][i]["AddNum"].asInt();

					strncpy(subscribeData.szDate, json_Data["SubscribeChartList"][i]["Date"].asCString(), DATETIMELEN-1);
					weixinData.subscribeChartList.push_back(subscribeData);
				}
			}

			weixinData.iSuccessFlag = iResult;     //结果
		}
	}
}

/*
	@brief  解析json数据
	@param  pJson 要解析的json字符串
	@param  [in out]keyResult 关键词排名数据
	*/
void CGetData::ParseData( const char *pJson, KEYWORDSDETAILRESPONSELIST &keyResult )
{
	keyResult.iSuccessFlag = 0;
	if (pJson != NULL)
	{
		Json::Reader reader;
		Json::Value json_object;

		if (reader.parse(pJson, json_object))
		{
			int iResult = json_object["ExtensionData"]["CallResult"].asInt();

			if (iResult == 1)  //请求成功
			{
				Json::Value json_Data = json_object["Data"];
				int iSize = json_Data["NewExpressKeyWordsDetailList"].size();
				
				if (json_Data["SubmitTimes"].isString())
				{
					strncpy(keyResult.szKeyWordLastTime, json_Data["SubmitTimes"].asCString(), MID_LENGTH-1);
				}

				for (int i=0; i<iSize; i++)
				{
					KEYWORDSDETAILRESPONSE  keyOne;

					keyOne.iRankings = json_Data["NewExpressKeyWordsDetailList"][i]["Rankings"].asInt();
					
					if (json_Data["NewExpressKeyWordsDetailList"][i]["KeyWordName"].isString())
						strncpy(keyOne.szKeyWordName, json_Data["NewExpressKeyWordsDetailList"][i]["KeyWordName"].asCString(), MID_LENGTH-1);

					if (json_Data["NewExpressKeyWordsDetailList"][i]["SearchEngineName"].isString())
						strncpy(keyOne.szSearchEngineName, json_Data["NewExpressKeyWordsDetailList"][i]["SearchEngineName"].asCString(), MID_LENGTH-1);

					if (json_Data["NewExpressKeyWordsDetailList"][i]["LocalFile"].isString())
						strncpy(keyOne.szLocalFile, json_Data["NewExpressKeyWordsDetailList"][i]["LocalFile"].asCString(), MAX_USER_PATH-1);

					if (json_Data["NewExpressKeyWordsDetailList"][i]["StatisticTime"].isString())
						strncpy(keyOne.szDate, json_Data["NewExpressKeyWordsDetailList"][i]["StatisticTime"].asCString(), DATETIMELEN-1);
					
					keyResult.keyList.push_back(keyOne);
				}
			}

			keyResult.iSuccessFlag = iResult;
		}
	}
}


/*
	@brief  解析json数据
	@param  pJson 要解析的json字符串
	@param  [in out]keyOnline 获取上线关键词变化图表数据
	*/
void CGetData::ParseData( const char *pJson, KEYWORD_PRODUCT_LIST &keyOnline )
{
	keyOnline.iSuccessFlag = 0;
	if (pJson != NULL)
	{
		Json::Reader reader;
		Json::Value json_object;

		if (reader.parse(pJson, json_object))
		{
			int iResult = json_object["ExtensionData"]["CallResult"].asInt();

			if (iResult == 1)  //请求成功
			{
				Json::Value json_Data = json_object["Data"];
				int iSize = json_Data["KeywordStatisticList"].size();
				int i = 0;

				for (i=0; i<iSize; i++)
				{
					KEYWORDSTATISTICCHAR  keyOne;

					keyOne.iUserId = json_Data["KeywordStatisticList"][i]["Id"].asInt();
					keyOne.iAllSEHasRankingCount = json_Data["KeywordStatisticList"][i]["AllSEHasRankingCount"].asInt();

					strncpy(keyOne.szDate, json_Data["KeywordStatisticList"][i]["StatisticsTimes"].asCString(), DATETIMELEN-1);

					keyOnline.keyWordList.push_back(keyOne);
				}

				iSize = json_Data["ProductStatisticsInfoList"].size();
				for (i = 0; i<iSize; i++)
				{
					PRODUCTSTATISTICSCHART  proOne;

					proOne.iUserId = json_Data["ProductStatisticsInfoList"][i]["Id"].asInt();
					proOne.iProductExposureCount = json_Data["ProductStatisticsInfoList"][i]["ProductExposureCount"].asInt();
					proOne.iProductCoverCount = json_Data["ProductStatisticsInfoList"][i]["ProductCoverCount"].asInt();

					strncpy(proOne.szDate, json_Data["ProductStatisticsInfoList"][i]["StatisticsTimes"].asCString(), DATETIMELEN-1);

					keyOnline.productList.push_back(proOne);
				}

				keyOnline.pro_key.iKeyWordCount = json_Data["KeyWordCount"].asInt();
				keyOnline.pro_key.iKeyWordHasRankCount = json_Data["KeyWordHasRankCount"].asInt();
				keyOnline.pro_key.iProductExposureCount = json_Data["ProductExposureCount"].asInt();
				keyOnline.pro_key.iProductCount = json_Data["ProductCount"].asInt();
				keyOnline.pro_key.iProductDownCount = json_Data["ProductDownCount"].asInt();
				keyOnline.pro_key.iProductCoverCount = json_Data["ProductCoverCount"].asInt();
			}

			keyOnline.iSuccessFlag = iResult;
		}
	}
}


/*
	@brief  登录商舟网 进行推广
	@param  pMsg 消息
	*/
void CGetData::LoginProductPost( T_Message *pMsg )
{
	CString strUrl;
	PassInfo *pInfo = (PassInfo*)pMsg->lParam;

	if (pInfo != NULL)
		strUrl.Format(_T("%s%s?%s&Type=%d&ProductId=%d&PublishType=%d"), pInfo->strUrl, GETDATASTRING, pInfo->strParam, TYPE_QUICKPOST, iProduceId, pMsg->wParam);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("直接推广:%s"), strUrl);

	ShellExecute(NULL, _T("open"), strUrl.GetString(), NULL, NULL, SW_NORMAL);
}


/*
	@brief 消息处理线程
	*/
DWORD WINAPI CGetData::MessageThread( LPVOID lp)
{
	CGetData *pThis = (CGetData*)lp;
	MSG   msg;
	
	while (GetMessage(&msg, 0, 0, 0))
	{
		T_Message *pMsg = (T_Message *)msg.lParam;
		//ASSERT(pMsg != NULL);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到消息ID=%d"),msg.message);

		switch (msg.message)
		{
		case MSG_SEVEN_DATA:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到提取近七天数据消息"));
				pThis->HandleSevenDataMessage(pMsg);
			}
			break;

		case MSG_POST_DETAIL_DATA:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到提取产品推广详情数据消息"));
				pThis->HandleProductPostDetail(pMsg);
			}
			break;

		case MSG_ZHENCI_INFO:
		   {
		//		::MessageBox(NULL, _T("收到大师甄词消息"), _T("OK"), MB_OK);
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到大师甄词消息"));
				pThis->HandleZhenCiDetail(pMsg);
		   }
			break;
		
		case MSG_START_PAIMING:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到开始刷5*20排名消息"));
				pThis->HandleStartPaiming(pMsg);
			}

		case MSG_WEIXIN_DATA:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到提取微信通数据消息"));
				pThis->HandleWeiXinData(pMsg);
			}
			break;

		case MSG_KEYWORD_RESULT_DATA:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到获取关键词排名消息"));
				pThis->HandleKeyWordResult(pMsg);
			}
			break;

		case MSG_KEYWORD_PRODUCT_DATA:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到获取上线关键词变化消息"));
				pThis->HandleKeyWordOnLine(pMsg);
			}
			break;
		case MSG_GET_DELAYMSG:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到推送消息_thread"));
				pThis->HandleTuiSongMsg(pMsg);
			}
			break;


		case MSG_LOGIN_WEIXIN:
		case MSG_LOGIN_SHANGZHOU:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到登录商舟网或微信通消息"));
				pThis->LoginShangzhou(pMsg);
			}
			break;

		case MSG_BUY_PRODUCT:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到购买产品消息"));
				pThis->BuyProduct(pMsg);

			}
			break;

		case MSG_LOGIN_CLIENT:
			{
				if (pMsg->wParam == 0)
				{
					g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到登录客户端消息"));
					pThis->LoginClient(pMsg);
				}
				else if (pMsg->wParam == 1 || pMsg->wParam == 2 || pMsg->wParam == 3)
				{
					g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("取密码消息"));
					pThis->GetPwdFromUser(pMsg);
				}
				
			}
			break;
		case MSG_PRODUCT_POST:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到直接推广消息"));
				pThis->LoginProductPost(pMsg);
			}
			break;

		case MSG_SUBMIT_USERINFO:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到提交信息消息"));
				pThis->SubmitClientInfo(pMsg);
			}
			break;
		case MSG_GET_JZ_INFORMATION:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到获取建站系统数据"));
				pThis->HandleJzInformMsg(pMsg);

			}
			break;

		case MSG_SALF_EXIT:
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("退出消息，退出线程"));
				goto end;
			}
			break;
		default:
			break;
		}

		if (pMsg != NULL)
		{
			//PassInfo *pTem = (PassInfo *)pMsg->lParam;

			//if (pTem /*&& msg.message != MSG_LOGIN_CLIENT*/)
			//	delete pTem;

			IMsgQueue::Free_Message(pMsg);
		}

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("消息处理完毕ID=%d"),msg.message);

	}

end:
	return 0;
}

/*
	@brief  取得url返回信息
	@parm   url
	@return 返回响应信息
	*/
CString CGetData::GetUrlRespones(const CString &strUrl )
{
	int iTries = 0;         //重试机制  出现异常时重试
	CString strData;
start:
	try
	{
		try
		{
			CInternetSession m_Session(_T("HttpClient"));
			CHttpFile *pFile = NULL;

			m_Session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT , 30*1000);
			m_Session.SetOption(INTERNET_OPTION_SEND_TIMEOUT , 30*1000);
			m_Session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT , 30*1000);
			m_Session.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT , 30*1000);
			m_Session.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT , 30*1000);
			m_Session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);          // 1次重试

			pFile = (CHttpFile*)m_Session.OpenURL(strUrl);

			if (pFile != NULL)
			{
				DWORD dwRet = 0;
				pFile->QueryInfoStatusCode(dwRet);
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("QueryInfoStatusCode"), strUrl.GetString(), GetLastError());

				if (dwRet == HTTP_STATUS_OK)
				{
					CStringA strTemp;
					char szBufa[8192] = {0};
					strData.Empty();

					while(pFile->ReadString((LPTSTR)szBufa, ((sizeof(szBufa)/2) -2 )) != NULL)
					{
						strTemp.Append(szBufa);
						memset(szBufa, 0, sizeof(szBufa));
					}

					TCHAR *pBufW = MultitoWide(strTemp.GetString(), CP_UTF8);
					if (pBufW != NULL)
					{
						strData.Append(pBufW);

						delete pBufW;
					}
				}

				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("状态码为:%d目标网址:%s"), dwRet, strUrl.GetString());

				pFile->Close();

				delete pFile;
			}
			else
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("geturl失败,目标网址:%s %d"), strUrl.GetString(), GetLastError());
			}
			m_Session.Close();
		}
		catch (CException* e)
		{
			TCHAR szBuf[500] = {0};
			if(e->GetErrorMessage(szBuf, 500))
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("打开地址异常:%s URL:%s, error:%d"), szBuf, strUrl.GetString(), GetLastError());
			}

			iTries++;
			if (iTries <= 1) //出现异常时重试一次
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("出现异常重试,次数为%d"), iTries);
				goto start;
			}
		}
	}
	catch (...)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("出现异常，请求读取数据失败，目标网址:%s %d"), strUrl.GetString(), GetLastError());

		iTries++;
		if (iTries <= 1) //出现异常时重试一次
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("出现异常重试,次数为%d"), iTries);
			goto start;
		}
	}

	return strData;
}


/*
@brief  大师甄词消息
@param  pMsg 消息
*/
void CGetData::HandleZhenCiDetail(T_Message *pMsg)
{
	ZhenciInfo* pInfo = (ZhenciInfo*)pMsg->lParam;
	CString strUserName = pInfo->strUserName;
	CString strPostUrl = _T("");
	char *pBuf = NULL;

	strPostUrl.Format(_T("%s%s?userName=%s"), m_strZCApi, DASHIZHENCIOFF, strUserName);


	pInfo->strRequestUrl.Format(_T("%s%s?%s&Type=%d&ProductId=%d&PublishType=%d"), pInfo->strPostUrl, GETDATASTRING, pInfo->strParam, TYPE_QUICKPOST, iProduceId, pInfo->iType);

	g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("请求大师甄词的登录地址为:%s"), pInfo->strRequestUrl);

	g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("请求大师甄词的API地址为:%s"), strPostUrl);

	// 从服务器获取数据
	CString strJsonData = GetUrlRespones(strPostUrl);

	g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("请求大师甄词的数据为:%s"), strJsonData);

	pBuf = WideToMulti(strJsonData.GetString());

	if (pBuf != NULL)
	{
		Json::Reader reader;
		Json::Value json_object;

		if (reader.parse(pBuf, json_object))
		{
			wchar_t *pParamW = NULL;
			const char *pParam = NULL;

			Json::Value json_Data = json_object["Data"];

			if (!json_Data.isNull())
			{
				if (!json_Data["Title"].isNull())
				{
					pParam = json_Data["Title"].asCString();
					pParamW = MultitoWide(pParam);

					if (pParamW)
					{
						pInfo->strTitle.Format(_T("%s"), pParamW);
						delete[]pParamW;
					}

				}

				if (!json_Data["Content"].isNull())
				{
					pParam = json_Data["Content"].asCString();
					pParamW = MultitoWide(pParam);

					if (pParamW)
					{
						pInfo->strContent.Format(_T("%s"), pParamW);
						delete[]pParamW;
					}
				}
			}
			else
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("请求大师甄词的数据失败"));
			}
		}
		delete[]pBuf;
	}

	// 返回数据
	T_Message *pBackMsg = IMsgQueue::New_Message();
	pBackMsg->dwDestWork = pMsg->dwSourWork;
	pBackMsg->dwSourWork = pMsg->dwDestWork;
	pBackMsg->dwMsg = pMsg->dwMsg;
	pBackMsg->wParam = 0;
	pBackMsg->lParam = (LPARAM)pInfo;
	SendMessage(pBackMsg);

}


/*
	@brief 提交用户信息到服务器
	@return  成功为true 
	*/
BOOL CGetData::SubmitClientInfo(T_Message *pMsg)
{
	CStdString strServer;
	GetURL(strServer);

	CReg reg;
	CString	strSupportVersion;
	CString strNewVersion;
	CString strURL;
	CString strUserName;
	CString strMac = GetPhysicalAddress();
	TCHAR szOSInfo[300] = { 0 };
	int  unInstallFlag = 0;
	GetSystemDetailedInfo(szOSInfo);

	//if (g_pGlobalData->sqlIte.GetUserInfo(userInfo))
	//{
	//	strUserName = stringcoding::StringBase64Decode((CString)(userInfo.szUName));

	//	//进行一次异或操作
	//	TCHAR ch = _T('①');
	//	for (int i =0; i < strUserName.GetLength(); ++i)
	//	{	
	//		DWORD Temp = strUserName[i];
	//		Temp = Temp^ch;
	//		strUserName.SetAt(i,Temp);
	//	}
	//}
	//else
	//{
	//	strUserName = _T("NoUserName");
	//}

	strSupportVersion	= (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("SupportProduct"));
	if (strSupportVersion.IsEmpty())
	{
		strUserName = _T("NoUserName");
		strSupportVersion = _T("ZDS_");
	}
	else
	{
		strUserName = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), strSupportVersion);
	}

	if (strUserName.IsEmpty())
	{
		strUserName = _T("NoUserName");
	}

	strNewVersion = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ\\Setup"), _T("version"));
	if (strNewVersion.IsEmpty())
	{
		strNewVersion = _T("0.0.0.0");
	}

	BYTE* szUnFlag = reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ\\Setup"), _T("uninstall"));
	
	if (szUnFlag != NULL)
	{
		unInstallFlag = *((int*)(szUnFlag));
	}
	
	strURL.Format(_T("%s/api/UserSetUpStatistics/AddUserSetUp?UserName=%s&MacAddress=%s&PdtVer=%s,Ver:%s&SysVer=%s&uninstall=%d"), \
		strServer.GetBuffer(), strUserName, strMac,strSupportVersion,strNewVersion, URLEncode(szOSInfo),unInstallFlag);

	CString strServerRes = GetUrlRespones(strURL);


	g_log.Trace(LOGL_LOW, LOGT_PROMPT, __TFILE__, __LINE__, _T("Submit MacAddr URL:%s, Result:%s"), strURL, strServerRes);

	if (!strServerRes.CompareNoCase(_T("success")))
	{
		return TRUE;
	}
	else 
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("提交用户信息失败!"));
		//发送消息给ui重新提交
		//T_Message *pBackMsg = IMsgQueue::New_Message();
		//pBackMsg->dwDestWork = pMsg->dwSourWork;
		//pBackMsg->dwSourWork = pMsg->dwDestWork;
		//pBackMsg->dwMsg = pMsg->dwMsg;
		//PostMessage(pBackMsg);
		return FALSE;
	}
}

/*
   @brief 存储数据到本地
   @param  iType 类型
   @param  pUserName 用户名
   @param  pBuf存储的数据
   changed by zhoulin
*/
void CGetData::SaveToLocal(const char *pUserName, const char *pBuf, int iType)
{
	if (pUserName == NULL
		|| pBuf == NULL)
		return;
	InsertData(pUserName, pBuf, iType);
}

/*
	@brief 判断是否需要保存到本地数据库
	@param pUserName 用户名
	@param pBuf 用户数据
	@param iType 数据类型  1 七天数据，2 历史推广数据 7为微信通数据
	*/
void CGetData::IsSaveToLocal( const char *pUserName, const char *pBuf, int iType )
{
	if (pUserName == NULL
		|| pBuf == NULL)
		return;

	CStringA strDate;
	time_t timep;
	struct tm *p;

	time(&timep);
	p = localtime(&timep); /*取得当地时间*/

	if (p != NULL)
	{
		strDate.Format("%d%02d%02d", p->tm_year+1900, p->tm_mon + 1, p->tm_mday);
	}

	//判断是否需要插入本地数据库
	if (strDate.Compare(GetRecentDateInTable("ServerData", pUserName, iType)) > 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("数据非今天更新,类型：%d"), iType);
		InsertData(pUserName, pBuf, iType);
	}
}


/*
	@brief 从本地数据库提取数据
	@param iType 类型  1为七天数据  2为历史数据  7为微信通数据
	@param pUserName 用户名
	@param strData [out] 要返回的数据
	*/
void CGetData::GetDataFromDb( int iType, const char *pUserName, CStringA &strData )
{
	try
	{
		CStringA strDate;
		CStringA strSql;

		strDate = GetRecentDateInTable("ServerData", pUserName, iType);
		if (!strDate.IsEmpty())
		{
			ResultTable resTable;
			ResultRecord *resRecord = NULL;

			strSql.Format("select szData from  ServerData where type=%d and insertDate='%s' and UserName='%s' order by id desc", iType, strDate, pUserName);

			if (g_pGlobalData->sqlIte.SelectStmt(strSql.GetString(), resTable))
			{
				if ((resRecord = resTable.next()) != NULL)
					strData = resRecord->fields_[0].c_str();
			}
		}

	}
	catch (...)
	{

	}
}

/*
	@brief  取得微信通数据
	@param  pMsg 消息
	*/
void CGetData::HandleWeiXinData( T_Message *pMsg )
{
	
	PassInfo *pInfo = (PassInfo*)pMsg->lParam;
	WEIXINTONG_DATA weixin_server;
	char *pUserName = NULL;
	char *pBuf = NULL;
	

	if (iIsBuyWeixin == 1)
	{
		weixin_server.iSuccessFlag = 0;
		CString strData = GetJsonFromServer(pMsg, TYPE_WEIXININFO);
		pBuf = WideToMulti(strData.GetString());

		ParseData(pBuf, weixin_server);
		if (pInfo != NULL)
			pUserName = WideToMulti(pInfo->strUserName.GetString());

		if (weixin_server.iSuccessFlag == 1)
		{
			IsSaveToLocal(pUserName, pBuf, TYPE_WEIXININFO);
		}
		else
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("从服务器取微信通数据失败"));
			GetSevenDataFromDb(pUserName, weixin_server);
		}
	}
	else
	{
		weixin_server.iSuccessFlag = -1;   //未开通微信通

	}


	T_Message *pBackMsg = IMsgQueue::New_Message();
	pBackMsg->dwDestWork = pMsg->dwSourWork;
	pBackMsg->dwSourWork = pMsg->dwDestWork;
	pBackMsg->dwMsg = pMsg->dwMsg;
	pBackMsg->wParam = 0;
	pBackMsg->lParam = (LPARAM)&weixin_server;
	SendMessage(pBackMsg);


	if (pUserName)
		delete pUserName;

	if (pBuf)
		delete pBuf;
}

/*
@brief  获取推广效果数据（关键词）
@param  pMsg 消息
changed by zhoulin
*/
void CGetData::HandleKeyWordResult(T_Message *pMsg)
{
	KEYWORDSDETAILRESPONSELIST keyResult;
	PassInfo *pInfo = (PassInfo*)pMsg->lParam;
	int iOnLineOrOffLine = (int)pMsg->wParam;
	char *pUserName = NULL;

	CString strData = _T("");
	char *pBuf = NULL;

	if (pInfo != NULL)
		pUserName = WideToMulti(pInfo->strUserName.GetString());

	switch (iOnLineOrOffLine)
	{
	case 0:
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("收到wParam为0从本地取推广效果数据"));
		GetSevenDataFromDb(pUserName, keyResult);

		if (keyResult.iSuccessFlag == 0)
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("从本地取推广效果数据失败,从线上取"));
			strData = GetJsonFromServer(pMsg, TYPE_KEYWORD_RESULT);
			pBuf = WideToMulti(strData.GetString());
			ParseData(pBuf, keyResult);

			if (keyResult.iSuccessFlag == 1)
			{
				DeleteDataInToday(pUserName, TYPE_KEYWORD_RESULT);
				SaveToLocal(pUserName, pBuf, TYPE_KEYWORD_RESULT);
				if (keyResult.keyList.size() == 0)
				{
					g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("关键词数据为空,返回的json为：%s"), strData.GetString());
				}
			}
		}

		break;

	case 1:
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("收到wParam为1从线上取推广效果数据"));

		strData = GetJsonFromServer(pMsg, TYPE_KEYWORD_RESULT);
		pBuf = WideToMulti(strData.GetString());
		ParseData(pBuf, keyResult);

		if (keyResult.iSuccessFlag == 1)
		{
			DeleteDataInToday(pUserName, TYPE_KEYWORD_RESULT);
			SaveToLocal(pUserName, pBuf, TYPE_KEYWORD_RESULT);
			if (keyResult.keyList.size() == 0)
			{
				g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("关键词数据为空,返回的json为：%s"), strData.GetString());
			}
		}

		break;
	default:
		break;
	}

	T_Message *pBackMsg = IMsgQueue::New_Message();
	pBackMsg->dwDestWork = pMsg->dwSourWork;
	pBackMsg->dwSourWork = pMsg->dwDestWork;
	pBackMsg->dwMsg = pMsg->dwMsg;
	pBackMsg->wParam = pMsg->wParam;
	pBackMsg->lParam = (LPARAM)&keyResult;
	SendMessage(pBackMsg);

	if (pUserName)
		delete pUserName;

	if (pBuf)
		delete pBuf;
}


/*
	@brief  获取推广效果数据（关键词）
	@param  pMsg 消息
	*/
//void CGetData::HandleKeyWordResult( T_Message *pMsg )
//{
//	CString strData = GetJsonFromServer(pMsg, TYPE_KEYWORD_RESULT);
//	PassInfo *pInfo = (PassInfo*)pMsg->lParam;
//	char *pUserName = NULL;
//
//	KEYWORDSDETAILRESPONSELIST keyResult;
//	char *pBuf = WideToMulti(strData.GetString());
//
//	ParseData(pBuf, keyResult);
//	if (pInfo != NULL)
//		pUserName = WideToMulti(pInfo->strUserName.GetString());
//
//	if (keyResult.iSuccessFlag == 1)
//	{
//		DeleteDataInToday(TYPE_KEYWORD_RESULT);
//		IsSaveToLocal(pUserName, pBuf, TYPE_KEYWORD_RESULT);
//		if (keyResult.keyList.size() == 0)
//		{
//			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("关键词数据为空,返回的json为：%s"), strData.GetString());
//		}
//	}
//	else
//	{
//		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("从服务器取获取推广效果数据失败"));
//		GetSevenDataFromDb(pUserName, keyResult);
//	}
//
//	
//
//	T_Message *pBackMsg = IMsgQueue::New_Message();
//	pBackMsg->dwDestWork = pMsg->dwSourWork;
//	pBackMsg->dwSourWork = pMsg->dwDestWork;
//	pBackMsg->dwMsg = pMsg->dwMsg;
//	pBackMsg->wParam = pMsg->wParam;
//	pBackMsg->lParam = (LPARAM)&keyResult;
//	SendMessage(pBackMsg);
//
//	if (pUserName)
//		delete pUserName;
//
//	if (pBuf)
//		delete pBuf;
//}

/*
@brief  获取上线关键词变化图表数据
@param  pMsg 消息
changed by zhoulin
*/

void CGetData::HandleKeyWordOnLine(T_Message *pMsg)
{
	KEYWORD_PRODUCT_LIST key_product;
	PassInfo *pInfo = (PassInfo*)pMsg->lParam;
	int iOnLineOrOffLine = (int)pMsg->wParam;
	char *pUserName = NULL;

	CString strData = _T("");
	char *pBuf = NULL;

	if (pInfo != NULL)
		pUserName = WideToMulti(pInfo->strUserName.GetString());

	switch (iOnLineOrOffLine)
	{
	case 0:
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("收到wParam为0从本地取上线关键词变化图表数据"));
		GetSevenDataFromDb(pUserName, key_product);

		if (key_product.iSuccessFlag == 0)
		{
			g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("从本地取上线关键词变化图表数据失败,从线上取"));
			strData = GetJsonFromServer(pMsg, TYPE_KEYWORD_PRODUCT);
			pBuf = WideToMulti(strData.GetString());
			ParseData(pBuf, key_product);

			if (key_product.iSuccessFlag == 1)
			{
				DeleteDataInToday(pUserName, TYPE_KEYWORD_PRODUCT);
				SaveToLocal(pUserName, pBuf, TYPE_KEYWORD_PRODUCT);
			}
		}

		break;

	case 1:
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("收到wParam为1从线上取上线关键词变化图表数据数据"));

		strData = GetJsonFromServer(pMsg, TYPE_KEYWORD_PRODUCT);
		pBuf = WideToMulti(strData.GetString());
		ParseData(pBuf, key_product);

		if (key_product.iSuccessFlag == 1)
		{
			DeleteDataInToday(pUserName, TYPE_KEYWORD_PRODUCT);
			SaveToLocal(pUserName, pBuf, TYPE_KEYWORD_PRODUCT);
		}

		break;
	default:
		break;
	}

	T_Message *pBackMsg = IMsgQueue::New_Message();
	pBackMsg->dwDestWork = pMsg->dwSourWork;
	pBackMsg->dwSourWork = pMsg->dwDestWork;
	pBackMsg->dwMsg = pMsg->dwMsg;
	pBackMsg->wParam = pMsg->wParam;
	pBackMsg->lParam = (LPARAM)&key_product;
	SendMessage(pBackMsg);

	if (pUserName)
		delete pUserName;

	if (pBuf)
		delete pBuf;
}

/*
	@brief  获取上线关键词变化图表数据
	@param  pMsg 消息
	*/
//void CGetData::HandleKeyWordOnLine( T_Message *pMsg )
//{
//	CString strData = GetJsonFromServer(pMsg, TYPE_KEYWORD_PRODUCT);
//	PassInfo *pInfo = (PassInfo*)pMsg->lParam;
//	char *pUserName = NULL;
//
//	KEYWORD_PRODUCT_LIST key_product;
//	char *pBuf = WideToMulti(strData.GetString());
//
//	ParseData(pBuf, key_product);
//	if (pInfo != NULL)
//		pUserName = WideToMulti(pInfo->strUserName.GetString());
//
//	if (key_product.iSuccessFlag == 1)
//	{
//		DeleteDataInToday(TYPE_KEYWORD_PRODUCT);
//		IsSaveToLocal(pUserName, pBuf, TYPE_KEYWORD_PRODUCT);
//	}
//	else
//	{
//		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("从服务器取上线关键词变化图表数据失败"));
//		GetSevenDataFromDb(pUserName, key_product);
//	}
//
//	
//
//	T_Message *pBackMsg = IMsgQueue::New_Message();
//	pBackMsg->dwDestWork = pMsg->dwSourWork;
//	pBackMsg->dwSourWork = pMsg->dwDestWork;
//	pBackMsg->dwMsg = pMsg->dwMsg;
//	pBackMsg->wParam = pMsg->wParam;
//	pBackMsg->lParam = (LPARAM)&key_product;
//	SendMessage(pBackMsg);
//
//	if (pUserName)
//		delete pUserName;
//
//	if (pBuf)
//		delete pBuf;
//}

/*
@brief 根据类型删除数据只保留一份最新数据
@param  pUserName 用户名
@param  iType 类型
changed by zhoulin
*/
void CGetData::DeleteDataInToday(const char *pUserName, int iType)
{
	try
	{

		{
			CStringA strSql;
			strSql.Format("delete from ServerData where UserName = '%s' and type=%d", pUserName, iType);

			g_pGlobalData->sqlIte.DirectStatement(strSql.GetString());
		}


	}
	catch (...)
	{

		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("删除数据出现异常，类型:%d"), iType);
	}
}


/*
	@brief 根据类型删除今天数据
	@param  iType 类型
	*/
//void CGetData::DeleteDataInToday( int iType )
//{
//	try
//	{
//		
//		{
//			CStringA strDate;
//			CStringA strSql;
//			time_t timep;
//			struct tm *p;
//
//			time(&timep);
//			p = localtime(&timep); /*取得当地时间*/
//			strDate.Format("%d%02d%02d", p->tm_year+1900, p->tm_mon + 1, p->tm_mday);
//
//
//			strSql.Format("delete from ServerData where insertDate='%s' and type=%d", strDate.GetString(), iType);
//
//			g_pGlobalData->sqlIte.DirectStatement(strSql.GetString());
//		}
//
//
//	}
//	catch (...)
//	{
//
//		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("删除数据出现异常，类型:%d"), iType);
//	}
//}

void CGetData::GetPwdFromUser( T_Message *pMsg )
{
	CString strUrl;
	CString strData;
	int  iResult = 0;
	PassInfo  *pInfo = NULL;
	char *pBuf = NULL;


	pInfo = (PassInfo*)pMsg->lParam;
	strUrl.Format(_T("%s%s?UserName=%s"), pInfo->strUrl, GETPWDSTRING, URLEncode(DecodeString(pInfo->strUserName)));

	strData = GetUrlRespones(strUrl);
	pBuf = WideToMulti(strData.GetString());
	pInfo->iLoginType = pMsg->wParam;
	
	if (pBuf != NULL)
	{
		Json::Reader reader;
		Json::Value json_object;

		if (reader.parse(pBuf, json_object))
		{
			iResult = json_object["ExtensionData"]["CallResult"].asInt();

			if (iResult == 1)
			{
				if (!json_object["Data"].isNull())
				{
					const char *pParam = json_object["Data"].asCString();
					wchar_t *pParamW = MultitoWide(pParam);

					if (pParamW)
					{
						pInfo->strParam = pParamW;

						delete []pParamW;
					}
					
				}
				
			}
		}
		delete []pBuf;
	}

	if (iResult == 1 && (!pInfo->strParam.IsEmpty()))  //取密码成功  登录
	{
		pInfo->strParam = EncodeUrlParam(pInfo->strParam);
		LoginClient(pMsg);
	}
	else
	{
		T_Message *pBackMsg = IMsgQueue::New_Message();
		pBackMsg->dwDestWork = pMsg->dwSourWork;
		pBackMsg->dwSourWork = pMsg->dwDestWork;
		pBackMsg->dwMsg = pMsg->dwMsg;
		pBackMsg->wParam = 0;
		pBackMsg->lParam = pMsg->lParam;
		PostMessage(pBackMsg);
	}
}

/*
	@brief  获取消息推送数据
	@param  pMsg 消息
	*/
void CGetData::HandleTuiSongMsg( T_Message *pMsg )
{
	CString strUrl;
	CString strData;
	BOOL bGet = FALSE;
	DELAY_MESSAGE  delayMsg;
	char *pBuf = NULL;
	PassInfo  *pInfo = (PassInfo*)pMsg->lParam;

	if (pInfo != NULL)
	{
		strUrl.Format(_T("%s%s?%s"), pInfo->strUrl, GETTUISONGMSG, pInfo->strParam);

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("推送消息网址：%s"), strUrl);
		strData = GetUrlRespones(strUrl);

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("推送消息返回数据格式：%s"), strData);
		pBuf = WideToMulti(strData.GetString());

		if (pBuf != NULL)
		{
			Json::Reader reader;
			Json::Value json_object;

			if (reader.parse(pBuf, json_object))
			{
				wchar_t *pParamW = NULL;
				const char *pParam = NULL;

				if (!json_object["Title"].isNull())
				{
					pParam = json_object["Title"].asCString();
					pParamW = MultitoWide(pParam);

					if (pParamW)
					{
						delayMsg.strTitle = pParamW;
						delete []pParamW;
					}
				}

				if (!json_object["TitleBoxColor"].isNull())
				{
					pParam = json_object["TitleBoxColor"].asCString();
					pParamW = MultitoWide(pParam);

					if (pParamW)
					{
						delayMsg.strColor = pParamW;
						delete []pParamW;
					}
				}

				if (!json_object["IntroUrl"].isNull())
				{
					pParam = json_object["IntroUrl"].asCString();
					pParamW = MultitoWide(pParam);

					if (pParamW)
					{
						delayMsg.strIntroUrl = pParamW;
						delete []pParamW;
					}
				}

				if (!json_object["DetailUrl"].isNull())
				{
					pParam = json_object["DetailUrl"].asCString();
					pParamW = MultitoWide(pParam);

					if (pParamW)
					{
						delayMsg.strDetailUrl = pParamW;
						delete []pParamW;
					}
				}

				if (!json_object["ButtonText"].isNull())
				{
					pParam = json_object["ButtonText"].asCString();
					pParamW = MultitoWide(pParam);

					if (pParamW)
					{
						delayMsg.strBtnText = pParamW;
						delete []pParamW;
					}
				}

				if (!json_object["SoftwareMd5"].isNull())
				{
					pParam = json_object["SoftwareMd5"].asCString();
					pParamW = MultitoWide(pParam);

					if (pParamW)
					{
						delayMsg.strMd5 = pParamW;
						delete []pParamW;
					}
				}
				//bGet = TRUE;
				delayMsg.iShowTime = json_object["ShowTime"].asInt();
				delayMsg.iType = json_object["PushType"].asInt();

				if (strData.Find(_T("{")) != -1 && strData.Find(_T("}")) != -1)  //正常json格式
				{
					bGet = TRUE;
				}
				
			}
			delete []pBuf;
		}
	}

	T_Message *pBackMsg = IMsgQueue::New_Message();
	pBackMsg->dwDestWork = pMsg->dwSourWork;
	pBackMsg->dwSourWork = pMsg->dwDestWork;
	pBackMsg->dwMsg = pMsg->dwMsg;
	pBackMsg->wParam = bGet;
	pBackMsg->lParam = (LPARAM)&delayMsg;
	SendMessage(pBackMsg);
}


/*
	@brief  获取建站系统信息
	@param  pMsg 消息
	*/
void CGetData::HandleJzInformMsg( T_Message *pMsg )
{
	CString strUrl;
	CString strData;
	BOOL bGet = FALSE;
	JZ_MESSAGE  jzMsg;
	char *pBuf = NULL;
	int iResult = 0;
	PassInfo  *pInfo = (PassInfo*)pMsg->lParam;


	if (iIsBuyJz == 1)
	{
		jzMsg.iSuccessFlag = 0;
		if (pInfo != NULL)
		{
			strUrl.Format(_T("%s%s?%s&productId=%d"), pInfo->strUrl, GETJZMSG, pInfo->strParam, iJzId);

			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("取建站数据网址：%s"), strUrl);
			strData = GetUrlRespones(strUrl);

			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("建站数据返回数据格式：%s"), strData);
			pBuf = WideToMulti(strData.GetString());

			if (pBuf != NULL)
			{
				Json::Reader reader;
				Json::Value json_object;

				if (reader.parse(pBuf, json_object))
				{
					wchar_t *pParamW = NULL;
					const char *pParam = NULL;

					jzMsg.iSuccessFlag = json_object["ExtensionData"]["CallResult"].asInt();

					Json::Value json_Data = json_object["Data"];

					if (jzMsg.iSuccessFlag == 1)
					{
						if (!json_Data["ExpirationDate"].isNull())
						{
							pParam = json_Data["ExpirationDate"].asCString();
							pParamW = MultitoWide(pParam);

							if (pParamW)
							{
								jzMsg.strExpirationDate = pParamW;
								delete []pParamW;
							}
						}

						if (!json_Data["AccessUrl"].isNull())
						{
							pParam = json_Data["AccessUrl"].asCString();
							pParamW = MultitoWide(pParam);

							if (pParamW)
							{
								jzMsg.strAccessUrl = pParamW;
								delete []pParamW;
							}
						}

						if (!json_Data["BindUrl"].isNull())
						{
							pParam = json_Data["BindUrl"].asCString();
							pParamW = MultitoWide(pParam);

							if (pParamW)
							{
								jzMsg.strBindUrl = pParamW;
								delete []pParamW;
							}
						}

						if (!json_Data["BindDomain"].isNull())
						{
							pParam = json_Data["BindDomain"].asCString();
							pParamW = MultitoWide(pParam);

							if (pParamW)
							{
								jzMsg.strBindDomain = pParamW;
								delete []pParamW;
							}
						}

						if (!json_Data["WebSiteUrl"].isNull())
						{
							pParam = json_Data["WebSiteUrl"].asCString();
							pParamW = MultitoWide(pParam);

							if (pParamW)
							{
								jzMsg.strWebSiteUrl = pParamW;
								delete []pParamW;
							}
						}

						jzMsg.iIsBind = json_Data["IsBind"].asInt();


					}
				}
				delete []pBuf;
			}
		}
	}
	else
	{
		jzMsg.iSuccessFlag = -1;
	}

	

	T_Message *pBackMsg = IMsgQueue::New_Message();
	pBackMsg->dwDestWork = pMsg->dwSourWork;
	pBackMsg->dwSourWork = pMsg->dwDestWork;
	pBackMsg->dwMsg = pMsg->dwMsg;
	pBackMsg->wParam = 0;
	pBackMsg->lParam = (LPARAM)&jzMsg;
	SendMessage(pBackMsg);
}


/*
	@brief  购买微信通或建站系统
	@param  pMsg 消息
	*/
void CGetData::BuyProduct( T_Message *pMsg )
{
	CString strUrl;
	CString strTargetUrl;
	CString strData;
	PassInfo *pInfo = (PassInfo*)pMsg->lParam;

	if (pInfo != NULL)
	{
		strUrl.Format(_T("%s%s?%s"), pInfo->strUrl, BUYPRODUCT, pInfo->strParam);
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("获取购买产品数据url:%s"), strUrl.GetString());

		strData = GetUrlRespones(strUrl);

		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("购买产品返回json:%s"), strData.GetString());

		char *pBuf = WideToMulti(strData.GetString());

		if (pBuf != NULL)
		{
			Json::Reader reader;
			Json::Value json_object;

			if (reader.parse(pBuf, json_object))
			{
				if (!json_object["ExtensionData"]["RetMsg"].isNull())
				{
					const char *pParam = json_object["ExtensionData"]["RetMsg"].asCString();
					wchar_t *pParamW = MultitoWide(pParam);

					if (pParamW)
					{
						strTargetUrl = pParamW;
						delete[]pParamW;
					}
				}
			}
			delete []pBuf;
		}

	}

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("购买产品url:%s"), strTargetUrl.GetString());

	if (!strTargetUrl.IsEmpty())
		ShellExecute(NULL, _T("open"), strTargetUrl.GetString(), NULL, NULL, SW_NORMAL);
}

/*
@brief 开始刷排名消息
@param pMsg 消息
*/
void CGetData::HandleStartPaiming(T_Message *pMsg)
{
	CString strUser =_T("");
	CString strInitPath = _T("");
	CString strServer = _T("");
	CString strURL = _T("");
	CString strData = _T("");

	strInitPath.Format(_T("%s\\data2\\Yuntask.dat"), g_pGlobalData->dir.GetInstallDir());
	strUser = *(CString*)pMsg->lParam;
	
	IXMLRW xml;
	xml.init(strInitPath);
	
	xml.ReadString(_T("YunTask"), _T("AutoTask"), _T("KeyWordServer"), strServer);

	strURL.Format(_T("%s/api/KeywordCacheService/GenerateUserKeywordInfo?userName=%s"), strServer, strUser);
	
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("开始启动刷排名url:%s"), strURL.GetString());

	strData = GetUrlRespones(strURL);

	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("启动刷排名后返回数据:%s"),strData.GetString());

	T_Message *pBackMsg = IMsgQueue::New_Message();
	pBackMsg->dwDestWork = pMsg->dwSourWork;
	pBackMsg->dwSourWork = pMsg->dwDestWork;
	pBackMsg->dwMsg = pMsg->dwMsg;
	pBackMsg->wParam = 0;
	pBackMsg->lParam = (LPARAM)&strData;
	SendMessage(pBackMsg);
}


