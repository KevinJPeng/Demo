#pragma once
#include <vector>
#define DATETIMELEN 11
#define MAX_USER_PATH 256

#define MID_LENGTH  50



//用户发布成功统计数据
typedef struct PublishSuccess
{
	int  iAllCount;               //总推广成功数量
	char  szDate[DATETIMELEN];   //统计日期
	PublishSuccess()
	{
		iAllCount = 0;
		ZeroMemory(szDate, sizeof(szDate));
	}
	PublishSuccess(const PublishSuccess &publishSuccess)
	{
		iAllCount = publishSuccess.iAllCount;

		ZeroMemory(szDate, sizeof(szDate));
		strncpy(szDate, publishSuccess.szDate, DATETIMELEN-1);
	}


}PublishSuccess;

//用户最近七天发布详情数据
typedef struct PublishDetail
{
	int  iSearchEngineSuccCount;         // 引擎推广成功数量  
	int  iSearchEngineSubjectCount;      // 引擎推广主题数量  
	int  iCompanySuccCount;	             // 公司推广成功数量  
	int  iCompanySubjectCount;           // 公司推广主题数量  
	int  iProcurementSuccCount;          // 采购推广成功数量  
	int  iProcurementSubjectCount;       // 采购推广主题数量  
	int  iProductSuccCount;              // 优势产品推广成功数量  
	int  iProductSubjectCount;           // 产品推广主题数量  

	char   szDate[DATETIMELEN];

	PublishDetail()
	{
		iSearchEngineSuccCount = 0;
		iSearchEngineSubjectCount = 0;
		iCompanySuccCount = 0;
		iCompanySubjectCount = 0;
		iProcurementSuccCount = 0;
		iProcurementSubjectCount = 0;
		iProductSuccCount = 0;
		iProductSubjectCount = 0;
		ZeroMemory(szDate, sizeof(szDate));
	}

}PublishDetail;


//昨天操作统计数据
typedef struct YesterDayPublishInfo
{
	int  iCompanyRegisterSuccCount;  // 公司注册成功数量  
	int  iPublishAllCount;           // 总推广成功数量  
	int  iSubjectCount;              // 总主题数量 
	YesterDayPublishInfo()
	{
		iCompanyRegisterSuccCount = 0;
		iPublishAllCount = 0;
		iSubjectCount = 0;
	}
}YesterDayPublishInfo;


typedef struct SEVEEN_DATA
{
	std::vector<PublishSuccess> postSuccessList;     //用户近7天发布成功统计数据集合
	PublishDetail               postDetail;          //用户最近七天发布详情数据
	YesterDayPublishInfo        postYesterDay;       //昨天操作统计数据
	int iSuccessFlag;           //成功失败标志  1:请求成功,2:参数验证失败,3:参数错误,4：身份验证未通过,5：账户信息错误,6.请求失败  0  失败

	SEVEEN_DATA()
	{
		iSuccessFlag = 0;
	}
}SEVEEN_DATA;







//历史累计总推广主题，发布、注册成功数
typedef struct HistoryPublishInfo
{
	int  iPublishSubjectTotalCount;       // 推广发布主题累计总次数  
	int  iPublishSuccTotalCount;          // 发布网站成功的累计总次数  
	int  iCompanyRegisterSuccTotalCount;  // 公司注册成功网站的累计总次数 
	HistoryPublishInfo()
	{
		iPublishSubjectTotalCount = 0;
		iPublishSuccTotalCount = 0;
		iCompanyRegisterSuccTotalCount = 0;
	}
}HistoryPublishInfo;



//四种推广历史累计总推广主题、发布成功数
typedef struct DifferentPublishHistoryInfo
{
	int iSearchEngineSubjectTotalCount;      // 引擎推广发布主题累计总次数  
	int iSearchEngineSuccTotalCount;         // 引擎推广成功发布网站的累计总次数  
	int iCompanySubjectTotalCount;           // 公司推广发布主题累计总次数  
	int iCompanySuccTotalCount;              // 公司推广成功发布网站的累计总次数  
	int iProcurementSubjectTotalCount;       // 采购推广发布主题累计总次数  
	int iProcurementSuccTotalCount;          // 采购推广成功发布网站的累计总次数  
	int iProductSubjectTotalCount;           // 优势产品推广发布主题累计总次数  
	int iProductSuccTotalCount;               // 优势产品成功发布网站的累计总次数 

	DifferentPublishHistoryInfo()
	{
		iSearchEngineSubjectTotalCount = 0;
		iSearchEngineSuccTotalCount = 0;
		iCompanySubjectTotalCount = 0;
		iCompanySuccTotalCount = 0;
		iProcurementSubjectTotalCount = 0;
		iProcurementSuccTotalCount = 0;
		iProductSubjectTotalCount = 0;
		iProductSuccTotalCount = 0;
	}
}DifferentPublishHistoryInfo;


typedef struct ProductDetail
{
	HistoryPublishInfo            hisInfo;
	DifferentPublishHistoryInfo   difInfo;
	int                           iSuccessFlag;  // 1:请求成功,2:参数验证失败,3:参数错误,4：身份验证未通过,5：账户信息错误,6.请求失败  0 失败

	ProductDetail()
	{
		iSuccessFlag = 0;
	}
}ProductDetail;



typedef struct UserInfo
{
	char szUName[MAX_USER_PATH];       //用户名
	char szPwd[MAX_USER_PATH];         //用户密码
	int iSavePwd;                      //是否保存账号密码      0不保存  1保存
	int iAutoLogin;                    //是否自动登录          0不自动登录 1自动登录
	int iProduceId;                    //产品id

	UserInfo()
	{
		memset(szUName, 0, sizeof(szUName));
		memset(szPwd, 0, sizeof(szPwd));
		iSavePwd = 1;
		iAutoLogin = 1;
		iProduceId = 0;
	}
}UserInfo;

typedef struct PassInfo  //传输数据
{
	CString strUrl;       //网址
	CString strParam;     //参数 EncryptedStr=用户名|时间戳[|用户登录密码]加密字符串&TimeStamp=yyyy-MM-dd HH:mm:ss
	//加密规则，先自行定义一个字符或字符串异或，然后再base64加密         //取密码时为空
	CString strUserName;  //用户名 加密过的用户名   //取密码的时候不用加密

	int  iLoginType;       //取密码时，保存wparam值
	
	int  iProductId;
	int  iWeiXinId;         //微信通id
	int  iExressVersionId;  //版本id
	int  iJzId;             //建站系统id

	PassInfo()
	{
		iProductId = 0;
		iWeiXinId = 0;
		iExressVersionId = 0;
		iJzId = 0;
		iLoginType = 0;
	}

}PassInfo;

//甑词信息数据
typedef struct ZhenciInfo
{
	int  iType;           //publishtype
	CString strPostUrl;   //请求的Url;
	CString strUserName;  //用户名
	CString strTitle;   //标题
	CString strContent; //内容
	CString strRequestUrl; //请求详细地址URL;
	CString strParam;     //参数 EncryptedStr=用户名|时间戳[|用户登录密码]加密字符串&TimeStamp=yyyy-MM-dd HH:mm:ss
	//加密规则，先自行定义一个字符或字符串异或，然后再base64加密         //取密码时为空

	ZhenciInfo()
	{
		iType = 3;
		strPostUrl = _T("");
		strUserName = _T("");
		strTitle = _T("");
		strContent = _T("");
		strRequestUrl = _T("");
		strParam = _T("");
	}
}ZhenciInfo;


//用户请求数变化图表
typedef struct RequestChart
{
	int  iTextReply;               //文本回复数
	int  iMenuClick;               //菜单点击数
	int  iLocationRequest;         //位置请求数
	int  iTotalRequest;            //总请求数
	char  szDate[DATETIMELEN];     //统计日期

	RequestChart()
	{
		iTextReply = 0;
		iMenuClick = 0;
		iLocationRequest = 0;
		iTotalRequest = 0;
		ZeroMemory(szDate, sizeof(szDate));
	}

	RequestChart(const RequestChart &requestChart)
	{
		iTextReply = requestChart.iTextReply;
		iMenuClick = requestChart.iMenuClick;
		iLocationRequest = requestChart.iLocationRequest;
		iTotalRequest = requestChart.iTotalRequest;

		ZeroMemory(szDate, sizeof(szDate));
		strncpy(szDate, requestChart.szDate, DATETIMELEN-1);
	}

}RequestChart;


//用户关注数变化图表
typedef struct SubscribeChart
{
	int  iSubscribNum;               //关注数
	int  iUnSubscribNum;             //取消关注数
	int  iAddNum;                    //净增长数
	char  szDate[DATETIMELEN];       //统计日期

	SubscribeChart()
	{
		iSubscribNum = 0;
		iUnSubscribNum = 0;
		iAddNum = 0;
		ZeroMemory(szDate, sizeof(szDate));
	}

	SubscribeChart(const SubscribeChart &subscribeChart)
	{
		iSubscribNum = subscribeChart.iSubscribNum;
		iUnSubscribNum = subscribeChart.iUnSubscribNum;
		iAddNum = subscribeChart.iAddNum;
		
		ZeroMemory(szDate, sizeof(szDate));
		strncpy(szDate, subscribeChart.szDate, DATETIMELEN-1);
	}

}SubscribeChart;


//用户当月微信通运营图表统计数据
typedef struct WEIXINTONG_DATA
{
	std::vector<RequestChart> requestChartList;     //用户请求数变化图表集合
	std::vector<SubscribeChart> subscribeChartList;     //用户关注数变化图表集合
	
	int iSuccessFlag;           //成功失败标志 -1 未开通微信通 0  失败 1:请求成功,2:参数验证失败,3:参数错误,4：身份验证未通过,5：账户信息错误,6.请求失败 

	WEIXINTONG_DATA()
	{
		iSuccessFlag = -1;
	}
}WEIXINTONG_DATA;


//关键词推广效果数据
typedef struct KEYWORDSDETAILRESPONSE
{
	int  iRankings;					  //关键词排名

	char szKeyWordName[MID_LENGTH];           //关键词名称
	char szSearchEngineName[MID_LENGTH];           //搜索引擎名称
	char szLocalFile[MAX_USER_PATH];		        //快照地址
	char szDate[DATETIMELEN];                   //统计日期

	KEYWORDSDETAILRESPONSE()
	{
		
		iRankings = 0;

		memset(szKeyWordName, 0, sizeof(szKeyWordName));
		memset(szSearchEngineName, 0, sizeof(szSearchEngineName));
		memset(szLocalFile, 0, sizeof(szLocalFile));
		memset(szDate, 0, sizeof(szDate));
	}

	KEYWORDSDETAILRESPONSE(const KEYWORDSDETAILRESPONSE &temp)
	{
		
		iRankings = temp.iRankings;

		memset(szKeyWordName, 0, sizeof(szKeyWordName));
		memset(szSearchEngineName, 0, sizeof(szSearchEngineName));
		memset(szLocalFile, 0, sizeof(szLocalFile));
		memset(szDate, 0, sizeof(szDate));

		strncpy(szKeyWordName, temp.szKeyWordName, MID_LENGTH-1);
		strncpy(szSearchEngineName, temp.szSearchEngineName, MID_LENGTH-1);
		strncpy(szLocalFile, temp.szLocalFile, MAX_USER_PATH-1);
		strncpy(szDate, temp.szDate, DATETIMELEN-1);
	}


}KEYWORDSDETAILRESPONSE;

//关键词推广效果数据集合
typedef struct KEYWORDSDETAILRESPONSELIST
{
	std::vector<KEYWORDSDETAILRESPONSE>  keyList;

	int iSuccessFlag;
	char szKeyWordLastTime[MID_LENGTH];           //关键词排名最新更新时间

	KEYWORDSDETAILRESPONSELIST()
	{
		iSuccessFlag = 0;
		memset(szKeyWordLastTime, 0, sizeof(szKeyWordLastTime));
	}
	KEYWORDSDETAILRESPONSELIST(const KEYWORDSDETAILRESPONSELIST &temp)
	{

		iSuccessFlag = temp.iSuccessFlag;
		memset(szKeyWordLastTime, 0, sizeof(szKeyWordLastTime));
		strncpy(szKeyWordLastTime, temp.szKeyWordLastTime, MID_LENGTH-1);
		keyList = temp.keyList;
	}

}KEYWORDSDETAILRESPONSELIST;


//上线关键词变化数据
typedef struct KEYWORDSTATISTICCHAR
{
	int iUserId;                             //用户id
	int iAllSEHasRankingCount;              //所有搜索引擎都有排名的关键词个数
	char szDate[DATETIMELEN];               //日期


	KEYWORDSTATISTICCHAR()
	{
		iUserId = 0;
		iAllSEHasRankingCount = 0;
		memset(szDate, 0, sizeof(szDate));
	}

	KEYWORDSTATISTICCHAR(const KEYWORDSTATISTICCHAR &temp)
	{
		iUserId = temp.iUserId;
		iAllSEHasRankingCount = temp.iAllSEHasRankingCount;

		memset(szDate, 0, sizeof(szDate));
		strncpy(szDate, temp.szDate, DATETIMELEN-1);
	}
}KEYWORDSTATISTICCHAR;


//产品曝光量
typedef struct PRODUCTSTATISTICSCHART
{
	int iUserId;                           //用户id
	int iProductExposureCount;             //产品曝光量
	int iProductCoverCount;                //产品覆盖量
	char szDate[DATETIMELEN];              //日期

	PRODUCTSTATISTICSCHART()
	{
		iUserId = 0;
		iProductExposureCount = 0;
		iProductCoverCount = 0;
		memset(szDate, 0, sizeof(szDate));
	}

	PRODUCTSTATISTICSCHART(const PRODUCTSTATISTICSCHART &temp)
	{
		iUserId = temp.iUserId;
		iProductExposureCount = temp.iProductExposureCount;
		iProductCoverCount = temp.iProductCoverCount;

		memset(szDate, 0, sizeof(szDate));
		strncpy(szDate, temp.szDate, DATETIMELEN-1);
	}

}PRODUCTSTATISTICSCHART;


//产品和关键词统计数据
typedef struct PRODUCT_KEYWORDSTATISTICS
{
	int iKeyWordCount;			 // 关键词总数 关键词总数 
	int iKeyWordHasRankCount;	// 有排名关键词总数 有排名关键词总数 
	int iProductExposureCount;	// 产品曝光量 产品曝光量 
	int iProductCount;			// 产品总数 产品总数 
	int iProductDownCount;		// 有下架风险的产品数 有下架风险的产品数 
	int iProductCoverCount;		// 产品信息覆盖量 产品信息覆盖量 


	PRODUCT_KEYWORDSTATISTICS()
	{
		iKeyWordCount = 0;
		iKeyWordHasRankCount = 0;
		iProductExposureCount = 0;
		iProductCount = 0;
		iProductDownCount = 0;
		iProductCoverCount = 0;
	}

}PRODUCT_KEYWORDSTATISTICS;



//产品曝光量   上线关键词变化数据   产品和关键词统计数据 集合
typedef struct KEYWORD_PRODUCT_LIST
{
	std::vector<KEYWORDSTATISTICCHAR>   keyWordList;
	std::vector<PRODUCTSTATISTICSCHART> productList;
	PRODUCT_KEYWORDSTATISTICS         pro_key;

	int iSuccessFlag;

	KEYWORD_PRODUCT_LIST()
	{
		iSuccessFlag = 0;
	}
}KEYWORD_PRODUCT_LIST;



//消息推送数据
typedef struct DELAY_MESSAGE
{
	CString strTitle;
	CString strIntroUrl;
	CString strDetailUrl;
	CString strColor;
	int     iShowTime;
	int     iType;
	CString strBtnText;
	CString strMd5;

	DELAY_MESSAGE()
	{
		iShowTime = 5;
		iType = 0;
	}
}DELAY_MESSAGE;


//建站系统信息
typedef struct JZ_MESSAGE
{
	CString strExpirationDate;   //到期时间
	CString strAccessUrl;        //应用网站访问URL 
	CString strBindUrl;          //建站系统绑定的URurl
	CString strBindDomain;       //建站系统绑定的域
	CString strWebSiteUrl;       //网站地址
	int    iIsBind;              //是否绑定

	int iSuccessFlag;

	JZ_MESSAGE()
	{
		iIsBind = 0;
		iSuccessFlag = -1;
	}
}JZ_MESSAGE;
