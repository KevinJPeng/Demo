/*

 GetData.cpp
 author: larry
 从服务器取得发布数据类


*/

#pragma once
#include "..\..\threadmodel\ithreadunit.h"
#include "ServerData.h"
#include "Lock.h"
#include "tinyxml.h"
#include "InternetHttp.h"
#include "CommFunc.h"
#include "IniFile.h"

enum
{
	//1:获取用户最近七天发布成功统计数据，2：获取历史推广数据，3：登录商舟网，4：登录客户端，5 : 商务快车快捷推广，6 : 商舟微信通登录，7 : 商舟微信通运营图表
	//8：获取推广效果数据（关键词），9：获取上线关键词变化图表数据，10：获取产品曝光量变化图表数据  11产品和关键词统计数据
	TYPE_SEVERNDATA = 1,
	TYPE_HISTORY = 2,
	TYPE_LOGINSHANGZHOU = 3,
	TYPE_LOGINCLIENT = 4,
	TYPE_QUICKPOST = 5,
	TYPE_LOGINWEIXIN = 6,
	TYPE_WEIXININFO = 7,

	TYPE_KEYWORD_RESULT = 8,
	TYPE_KEYWORD_PRODUCT = 9,
	TYPE_ZHENCI_RESULT = 10,  // 大师甄词
// 	TYPE_PRODUCT_SHOW = 10,
// 	TYPE_PRODUCT_KEYWORD = 11,
};

class CGetData :
	public IThreadUnit
{
public:
	CGetData(void);
	virtual ~CGetData(void);

	//线程消息处理
	DWORD DispatchMessage(T_Message *pMsg);


	/*
	@brief  从服务器取得数据
	@param  pMsg   消息
	@param  iType   类型   1:获取用户最近七天发布成功统计数据，2：获取历史推广数据，3：登录商舟网，4：登录客户端，5 : 商务快车快捷推广，6 : 商舟微信通登录，7 : 商舟微信通运营图表
	@return 返回数据
	*/
	CString  GetJsonFromServer(T_Message *pMsg, int iType);

	/*
	@brief  取得近7天用户所有数据
	@param  pMsg 消息
	*/
	void HandleSevenDataMessage(T_Message *pMsg);

	/*
	@brief  登录客户端
	@param  pMsg 消息
	*/
	void LoginClient(T_Message *pMsg);


	/*
	@brief  登录商舟网  或开通微信通   开通建站系统
	@param  pMsg 消息
	*/
	void LoginShangzhou(T_Message *pMsg);


	/*
	@brief  购买微信通或建站系统
	@param  pMsg 消息
	*/
	void BuyProduct(T_Message *pMsg);

	/*
	@brief  登录商舟网 进行推广
	@param  pMsg 消息
	*/
	void LoginProductPost(T_Message *pMsg);


	/*
	@brief  取得产品推广详情
	@param  pMsg 消息
	*/
	void HandleProductPostDetail(T_Message *pMsg);


	/*
	@brief  大师甄词消息
	@param  pMsg 消息
	*/
	void HandleZhenCiDetail(T_Message *pMsg);

	/*
	@brief 开始刷排名消息
	@param pMsg 消息
	*/
	void HandleStartPaiming(T_Message *pMsg);

	/*
	@brief  取得微信通数据
	@param  pMsg 消息
	*/
	void HandleWeiXinData(T_Message *pMsg);


	/*
	@brief  获取推广效果数据（关键词）
	@param  pMsg 消息
	*/
	void HandleKeyWordResult(T_Message *pMsg);

	/*
	@brief  获取上线关键词变化图表数据
	@param  pMsg 消息
	*/
	void HandleKeyWordOnLine( T_Message *pMsg );

	/*
	@brief  获取消息推送数据
	@param  pMsg 消息
	*/
	void HandleTuiSongMsg( T_Message *pMsg );


	/*
	@brief  获取建站系统信息
	@param  pMsg 消息
	*/
	void HandleJzInformMsg(T_Message *pMsg);

	/*
	@brief  将服务器取得的数据插入数据库
	@param  pUserName  用户名  数据与用户名关联
	@param  pJson  要插入的数据字符串
	@param  iType  1表示近七天数据，2表示产品推广详情 7为微信通数据
	*/
	void  InsertData(const char *pUserName, const char *pJson, int iType);


	/*
	@brief  解析json数据
	@param  pJson 要解析的json字符串
	@param  [in out]sevenData 近七天数据
	*/
	void  ParseData(const char *pJson, SEVEEN_DATA &sevenData);

	/*
	@brief  解析json数据
	@param  pJson 要解析的json字符串
	@param  [in out]proDuct 产品推广详情
	*/
	void  ParseData(const char *pJson, ProductDetail &proDuct);

	/*
	@brief  解析json数据
	@param  pJson 要解析的json字符串
	@param  [in out]weixinData 微信通数据
	*/
	void  ParseData(const char *pJson, WEIXINTONG_DATA &weixinData);


	/*
	@brief  解析json数据
	@param  pJson 要解析的json字符串
	@param  [in out]keyResult 关键词排名数据
	*/
	void  ParseData(const char *pJson, KEYWORDSDETAILRESPONSELIST &keyResult);


	/*
	@brief  解析json数据
	@param  pJson 要解析的json字符串
	@param  [in out]keyOnline 获取上线关键词变化图表数据
	*/
	void ParseData( const char *pJson, KEYWORD_PRODUCT_LIST &keyOnline );



	/*
	@brief  从本地数据库取数据
	@param  pUserName 用户名
	@param  [in out]productData  返回数据
	*/
	void  GetSevenDataFromDb(const char *pUserName, ProductDetail &productData);
	void  GetSevenDataFromDb(const char *pUserName, SEVEEN_DATA &sevenData);
	void  GetSevenDataFromDb(const char *pUserName, WEIXINTONG_DATA &weixinData);
	void  GetSevenDataFromDb(const char *pUserName, KEYWORDSDETAILRESPONSELIST &keyResult);
	void  GetSevenDataFromDb(const char *pUserName, KEYWORD_PRODUCT_LIST &keyOnline);

	/*
	@brief  取得数据库表格中某数据最新一次插入数据的日期
	@param  表格名
	@param  用户名
	@param  类型 1为近七天数据，2为推广产品详情 7为微信通数据
	@return 返回日期，如果为空表示数据库还没有数据
	add by zhoulin
	*/
	CStringA GetRecentDateInTable(const char *pTableName , const char *pUserName, int iType);

	/*
	@brief  取得数据库表格中某数据最新一次插入数据的日期
	@param  表格名
	@param  类型 1为近七天数据，2为推广产品详情 7为微信通数据
	@return 返回日期，如果为空表示数据库还没有数据
	*/
	/*CStringA  GetRecentDateInTable(const char *pTableName = "ServerData", int iType = 1);*/

	/*
	add by zhoulin 
	@brief  通过网卡驱动IO取得mac地址
	@param szMac Mac字符串空间, nBuffSize 空间大小
	@return int状态码
	*/
//	int GetMac(TCHAR *szMac, int nBuffSize);


	/*
	@brief  取得url返回信息
	@parm   url
	@return 返回响应信息
	*/
	CString GetUrlRespones(const CString &strUrl);


	/*
	@brief 消息处理线程
	*/
	static DWORD WINAPI MessageThread(LPVOID);


	/*
	@brief 提交用户信息到服务器
	@param pMsg 消息
	@return  成功为true 
	*/
	BOOL SubmitClientInfo(T_Message *pMsg);


	/*
	@brief 判断是否需要保存到本地数据库
	@param pUserName 用户名
	@param pBuf 用户数据
	@param iType 数据类型  1 七天数据，2 历史推广数据 7为微信通数据
	*/
	void IsSaveToLocal(const char *pUserName, const char *pBuf, int iType);


	/*
	@brief 从本地数据库提取数据
	@param iType 类型  1为七天数据  2为历史数据  7为微信通数据 8：获取推广效果数据（关键词），9：获取上线关键词变化图表数据，10：获取产品曝光量变化图表数据
	@param pUserName 用户名
	@param strData [out] 要返回的数据
	*/
	void GetDataFromDb(int iType, const char *pUserName, CStringA &strData);

	/*
	@brief 根据类型删除今天数据
	@param  iType 类型
	changed by zhoulin
	*/
	void DeleteDataInToday(const char *pUserName, int iType);


	/*
	@brief 存储数据到本地
	@param  iType 类型
	@param  pUserName 用户名
	@param  pBuf存储的数据
	changed by zhoulin
	*/
	void SaveToLocal(const char *pUserName, const char *pBuf, int iType);

	/*
	@brief 根据类型删除今天数据
	@param  iType 类型
	*/
	/*void DeleteDataInToday(int iType);*/


	void GetPwdFromUser(T_Message *pMsg);

	bool GetURL(CStdString& strURL);
private:
	int  iProduceId;         //产品id
	int  iWeiXinId;         //微信通id
	int  iExressVersionId;  //版本id
	int  iJzId;             //建站系统id

	int  iIsBuyWeixin;     //是否购买微信通0未购买
	int  iIsBuyJz;     //是否购买建站系统0未购买

	DWORD dwThreadId;        //线程id

	BOOL  bBack;            //登录是否返回消息

	CLock  m_Lock;          //锁

	HANDLE  hEvent;         //事件对象

	IXMLRW m_iniFile;      //操作ini文件

	CString m_strZCApi;       //大师甄词服务器
};




