// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

// #if !defined _DEBUG
#pragma comment(linker, "/subsystem:\"Windows\" /entry:\"mainCRTStartup\"")//去除console窗口
// #endif

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS         // 移除对话框中的 MFC 控件支持

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            //  从 Windows 头文件中排除极少使用的信息
#endif

#include <afx.h>
#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>
#include <afxdisp.h>
#include <afxcontrolbars.h>



// TODO:  在此处引用程序需要的其他头文件
#include <vector>
#include "stringutils.h"
#include "IServerSocket.h"
#include "ServerSocket.h"
#include "trace.h"
#include "FileReadAndSave.h"
#include "StdString.h"
#include <afxinet.h>
#include <afxhtml.h>
#include "IXMLRW.h"
#include <afxdisp.h>

#define IsHexNum(c) ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) 
#define IE_AGENT  _T("Mozilla/5.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727)")

/**************************************
*  一些数据定义
**************************************/

#define BADIDU_INDEX     0
#define S360_INDEX       1
#define SOGOU_INDEX      2
#define BING_INDEX       3
#define YOUDAO_INDEX     4
#define PHONEBAIDU_INDEX 5
#define PHONESOGOU_INDEX 6
#define PHONE360_INDEX	 7
#define WEIXIN_INDEX	 8
#define PHONESHENMA_INDEX	 9

// 搜索引擎数量
#define SEARCHFLAGCOUNT  10

#define DECRYPTCFG					_T("szw^1*3&cfg")

// 搜索标记
enum
{
	SEARCH_BAIDU = 0x01,               // 百度        
	SEARCH_360 = 0x02,               // 360搜索
	SEARCH_SOGOU = 0x04,               // 搜狗
	SEARCH_BING = 0x08,               // 必应
	SEARCH_YOUDAO = 0x10,               // 有道
	SEARCH_PHONEBAIDU = 0x20,               // 手机百度
	SEARCH_PHONESOGOU = 0x40,               // 手机搜狗
	SEARCH_PHONE360 = 0x80,				//手机360
	SEARCH_WEIXIN = 0xF0,				//微信
	SEARCH_PHONESHENMA = 0x0100,		//手机神马
	SEARCH_Count = 10,					//引擎数量
};


#define OPEN_PAGE_NORMAL           -2
#define OPEN_PAGE_FAILED           -1
#define NOT_FOUND_COMPANY          0
#define FOUND_COMPANY              1
#define PAGE_NOT_FOUND             2
#define PAIMING_EXIST              3
#define NO_PAIMING                 4
#define ENGINE_APPEAR_VERIFYCODE   5

#define GET_WEBDATA_NORMAL			6		//正常获取网页数据
#define GET_WEBDATA_EXCEPTION		7		//网页打开异常（返回5XX,或正常打开但数据为空）
#define PAGE_OFFICIAL_OPENFAILED    8		//官网打开失败
#define PAGE_FUNTION_ERROR			9		//函数执行失败

//定位item
enum
{
	LOCATION_ITEM_TAG_NULL = 0,					// 仅以标签定位        
	LOCATION_ITEM_TAG_ID = 1,						// 通过标签和ID定位        
	LOCATION_ITEM_TAG_CLASS = 2,					// 通过标签和类名定位        
	LOCATION_ITEM_TAG_ATTRIBUTE = 3,				// 通过标签和属性定位        
	LOCATION_ITEM_TAG_INDEX = 4,					// 通过标签和编号模糊定位        
};

typedef struct _locationStruct
{
	int iLocationType;		//定位类型
	CString sTagType;			//标签类型
	long lFuzzyIndex;			//模糊定位id
	CString sKey;				//定位键
	CString sValue;			//定位值
}locationStruct, *plocationStruct;



//内页总的打开状态
enum
{
	E_ALLURLOPEN_STATE0 = 0,		//初始状态
	E_ALLURLOPEN_STATE1,			//有官网排名
	E_ALLURLOPEN_STATE2,			//有普通排名
	E_ALLURLOPEN_STATE3,			//有普通排名（官网没打开） 返回-2
	E_ALLURLOPEN_STATE4,			//没排名（有普通网站没打开） 返回-2
	E_ALLURLOPEN_STATE5			//没排名（有官网没打开） 返回-2
};
//内页当前打开状态
enum
{
	E_CURRENTURLOPEN_STATE0 = 0,		//有官网排名
	E_CURRENTURLOPEN_STATE1,			//有普通网排名
	E_CURRENTURLOPEN_STATE2,			//官网没打开
	E_CURRENTURLOPEN_STATE3,			//普通网没打开
};

enum
{
	ELEMENT_MARK_INNERTEXT = 0,			//节点内文
	ELEMENT_MARK_HREF = 1,					//通过href获取值
	ELEMENT_MARK_ATTRIBUTE = 2,			//通过属性获取值
};
typedef struct _targetAddressSign
{
	int iUrlGetType;			//URL获取类型
	CString sHrefKey;			//键
}targetAddressSign, *ptargetAddressSign;

#define ADD_SEPARATOR_1 _T(",")
#define ADD_SEPARATOR_2 _T(";")
//分隔符类型
enum
{
	ELEMENT_SEPARATOR_1 = 0,			//分隔符1(",")
	ELEMENT_SEPARATOR_2 = 1,			//分隔符2(";")
};
//抓取页面返回数据标记
typedef struct _Flag_CatchInfo
{
	BOOL	bIsOfficialFlag;		//官网标记
	BOOL	bIsAceFlag;			//2代标记
	BOOL	bSpecialOfficialFlag;	//特殊官网标记（对于打开目标页，结果产生了跳转的，需要在排除 目标pc站/目标wap站/搜索引擎站/转码站之外的站，都视为王牌站，不用进去抓取）

	_Flag_CatchInfo()
	{
		bIsOfficialFlag = FALSE;
		bIsAceFlag = FALSE;
		bSpecialOfficialFlag = FALSE;
	}
}CatchInfo, *pCatchInfo;

// 关键词数据信息
typedef struct _Tag_KeyWordDataInfo
{
	bool					bOnlyRareWord;
	int                   iFlag;            // 搜索标志
	int                   iCurPage;         // 当前第几页
	int					  iIndex;			//搜索引起索引
	int					  iPublishType;		//推广类型（1：普通推广；2：轻舟推广；3:1和2；4：优排；5:1和4；6:2和4）
	CString				  strClientType;	//客户类型（属于什么客户）
	CString               strKey;           // 关键词Key
	CString               strKeyWordName;   // 搜索关键词名
	CString               strKeyHex;        // 搜索关键词16进制码
	CString               strUrl;           // 目标网址
	CStringA              strWebList;       // 舟大师发布的网站列表
	CString               strWebFlag;       // 网站标记0：表示忽略网站列表，只抓取舟大师收录的其它网站
	//         1：表示抓取网站列表及所有舟大师收录的网站
	//         2：表示仅抓取网站列表中指定的网站
	//		   3：表示排除舟大师收录的网站排除网站列表
	//		   4：表示3+ 官网列表；
	CString               strComany;        // 公司名
	CString				  strWeixinName;	//微信公众号名称
	std::vector<CString>  vOfficialList;	//官网列表
	std::vector<CString>  vCompanys;        // 公司名(包含发布标记)
	std::vector<CStdString>  vAllCompanys;		//包括公司简称和全称；

	std::vector<CStdString>  vCompanysTag;        // 生僻字判断公司(,简称, 或  ，简称，)

	_Tag_KeyWordDataInfo()
	{
		bOnlyRareWord = false;
		iCurPage = 0;
		iFlag = 0;
		iIndex = -1;
		iPublishType = -1;
		strClientType.Empty();
		strWeixinName.Empty();
		vCompanys.clear();
		vAllCompanys.clear();
		vOfficialList.clear();
		vCompanysTag.clear();
	}

	~_Tag_KeyWordDataInfo()
	{
		vCompanys.clear();
		vAllCompanys.clear();
		vOfficialList.clear();
		vCompanysTag.clear();
	}

}KeyWordDataInfo, *pKeyWordDataInfo;


// 返回主控数据
typedef struct _Tag_BackDataInfo
{
	int         iRank;                      // 关键词排名(1~30)
	int         iFlag;                      // 搜索引擎标志
	int			iRankCnt;					//关键词排名个数
	int			iB2BFlag;					//B2B标志
	int			iOfficialFlag;				//官网标志
	int			iAceFlag;					//王牌标记
	int			iKeywordType;				//关键词类型（0：该关键词不带广告	1：该关键词带广告）
	CString     strKeyWordName;             // 关键词名称
	CString     strKey;                     // 关键词Key
	CString     strPagePath;                // 快照路径
	CString     strCompanyName;             // 公司名
	CString		strRecordInfo;			 //排名记录  网站ID,排名位置(;2)网站ID,排名位置
	CString		strRedirectUrl;			 //重定向后的Url
	CString		strRecordErrorInfo;		//记录打开失败的网站  网站ID,-2(;2)网站ID,-2


	_Tag_BackDataInfo()
	{
		iRank = 0;
		iFlag = 0;
		iRankCnt = 0;
		iB2BFlag = 0;
		iOfficialFlag = 0;
		iAceFlag = 0;
		iKeywordType = 0;
		strPagePath = _T("");
		strKey = _T("");
		strKeyWordName = _T("");
		strCompanyName = _T("");
		strRecordInfo = _T("");
		strRedirectUrl = _T("");
		strRecordErrorInfo = _T("");
	}

}BackDataInfo, *pBackDataInfo;

class CWebDlg;
typedef struct _sthreadParam
{
	CWebDlg *pPhotoDlg;
	pKeyWordDataInfo pData;
//	CComQIPtr<IHTMLElement> pElement;
}sthreadParam, *psthreadParam;

//特殊网站特殊处理的标记
typedef struct _SpecialUrlFlag
{
	//int iEngID;
	CString	strSiteName;			//网站名
	CStringA	strMarkerFlag1;		//提取标记1
	CStringA	strMarkerFlag2;		//提取标记2
	CString strJsUrl;				//固定的URL路径
}SpecialUrlFlag;

//特殊网站进行预处理
typedef struct _pageDataPre
{
	//int iEngID;
	CString	strDomain;			//域名
	CString	strBeforeData;		//原始数据
	CString	strAfterData;			//替换后的数据
}pageDataPre;

typedef struct _ItemLabel
{
	CString strlable;				//标签名
	CString strAttributeName;		//属性名
	CString strAttributeValue;		//属性值
}ItemLabel, *pItemLabel;

typedef struct _KeyData
{
	CString strUrlTitle;		//
	CString strUrlLink;		//
}KeyData, *pKeyData;

typedef struct _TagNode
{
	CString sTag;			//标签
	CString skey;			//属性（键）
	CString sValue;		//属性（值）
	_TagNode()
	{
		sTag = _T("");
		skey = _T("");
		sValue = _T("");
	}
	void clear()
	{
		sTag = _T("");
		skey = _T("");
		sValue = _T("");
	}
}TagNode, *pTagNode;

//网页元素标记匹配成配置样式
typedef struct _Elem_MatchFlag
{
	CString		strJumpUrl;							//跳转网址
	CString		strCollTags;						//元素集合
	CString		strItemFlag1;						//过滤不属于要查找的ITEM标签
	CString		strItemFlag2;						//....
	CString     strFindUrlFlag1;					//查找URL标记
	CString		strFindUrlFlag2;					//查找标记
	CString		strFindUrlFlag3;					//....
	CString		strBtnId;							//按钮ID标识符；
	CString		strHtmlName;						//文件名称后缀
	//共用标记
	CString		strGetUrlFlag1;						//查找目标网址的标记；
	CString		strGetUrlFlag2;						//...
	CString		strJudgeHtmlTag;					//判断
	CString		strLocationAd;						//判断该关键词是否带有广告

	//万能标记，根据每个引擎的需要作为值；
	CString		strUniversalFst;					//第一个万能标记  百度：查找结束标记；360：从目标地址获取真实url标记；有道：保存时候查找标记
	CString		strUniversalSnd;					//第二个万能标记
	CString		strCssPath;							//Css文件路径

	int			iSearchPageNum;					//搜索页面个数
	CString		sKWSearchMethod;					//关键词打开方式（0：关键词拼接在链接后面   1：关键词输入到输入框后点击百度一下按钮）
	CString		sHomePage;						//搜索引擎主页
	TagNode		tagKWEdit;						//关键词输入框
	TagNode		tagSearchBtn;						//百度一下按钮
	TagNode		tagNextPage;						//下一页
	TagNode		tagMainBody;						//

	//引擎抓取标记；
	std::vector<locationStruct>		vstrLocationTitle;
	targetAddressSign			strTargetTitle;
	std::vector<locationStruct>		vstrLocationLink;
	targetAddressSign			strTargetLink;

	std::vector<CStdString>	vTargetMarkWeb;			//查找包含目标网址的网站
	std::vector<CStdString> vTargetKey;				//查找包含目标网址的关键字；	
	std::vector<CStdString>	vTargetStart;			//定位目标的前面关键词；
	std::vector<CStdString>	vTargetEnd;				//定位目标后面的关键词
	std::vector<CStdString>	vTargetPreLen;			//要查找前面的长度；
	std::vector<CStdString> vTargetLstLen;			//查找后面的长度；
	std::vector<CStdString> vReplaceTagWebs;		//替换标签的网站

	std::map<CStdString, DWORD> mapAceWebToID;		//王牌网址对应网站ID;

	_Elem_MatchFlag()
	{
		strJumpUrl = _T("");
		strCollTags = _T("");
		strItemFlag2 = _T("");
		strItemFlag1 = _T("");
		strFindUrlFlag1 = _T("");
		strFindUrlFlag2 = _T("");
		strFindUrlFlag3 = _T("");
		strBtnId = _T("");

		strGetUrlFlag1 = _T("");
		strGetUrlFlag2 = _T("");
		strJudgeHtmlTag = _T("");

		strUniversalFst = _T("");
		strUniversalSnd = _T("");
		strHtmlName = _T("");
		strCssPath = _T("");
		strLocationAd = _T("");
		sKWSearchMethod = _T("");
		sHomePage = _T("");
		vTargetMarkWeb.clear();
		vTargetKey.clear();
		vTargetEnd.clear();
		vTargetPreLen.clear();
		vTargetStart.clear();
		vTargetLstLen.clear();
		vReplaceTagWebs.clear();
		mapAceWebToID.clear();
		vstrLocationTitle.clear();
		vstrLocationLink.clear();
	}

}MatchElemFlag;

typedef struct _sFingerRecogn
{
	int				iMatchNum;
	int				iCount;
	CString			sCharData;
	std::vector<CString>	vsCharData;

	_sFingerRecogn()
	{
		iMatchNum = 2;
		iCount = 0;
		sCharData = _T("");
		vsCharData.clear();
	}
}sFingerRecogn;

//搜索引擎iFlag标记到其配置的映射
extern std::map<int, _Elem_MatchFlag> g_mapElemCfg;

extern sFingerRecogn g_sFingerRecogn;

// 搜索引擎名到iFlag的映射
extern std::map<CString, int> g_mapSearch;

// 存储所有关键词
extern std::vector<pKeyWordDataInfo> g_vAllKeyWords;

// 保存不同用户的公司名
extern std::vector<CString> g_vDiffCompany;

// 搜索开关(是否搜索)
extern volatile BOOL bSearchFlag[];

// 主控传过来的socket接口用来发送数据到主控
extern CServerSocket *g_server;

// 判断是用户手刷还是云刷
extern bool g_bManualRefresh;

// 抓取快照日志
extern CLogTrace g_log;

// extern CLogTrace g_debugLog;
// extern CLogTrace g_TaskRecord;

// 出现验证码或网页异常日志
extern CLogTrace g_pLog;

// 临界区相关
extern CRITICAL_SECTION critSection;
extern CRITICAL_SECTION critSendMsg;
extern CRITICAL_SECTION critSearchFlag;
extern CRITICAL_SECTION critCount;

// 记录抓取关键词个数
extern int g_iBaidu;
extern int g_iSogou;
extern int g_iBing;
extern int g_i360;
extern int g_iYouDao;

// 抓取网站列表
extern CStringA  g_strWeb;

// 关键词之间的延时
extern int g_iDelayForKeyWord;

// 贸网站之间的延时
extern int g_iDalayForSite;

extern int g_iThreadCount;

//调试要抓取的URL
extern CString g_strDebugUrl;
extern BOOL g_bIsDebug;

const int HtmlCharNum = 17;
const int CHAR_LEN = 2;
extern TCHAR tSpecialChar[HtmlCharNum][CHAR_LEN];


extern CString g_shttpOss;			//加在快照路径前面(http:oss)
extern CString g_sKWPath;			//OSS上快照文件夹名（由年月日 经过一定算法组成）


//extern IXMLRW *g_DatMc;

/**************************************
* 数据定义结束
**************************************/


char* CStringToMutilChar(CString& str, int& chLength, WORD wPage);

// 初始化map
void InitMap();

/*
@brief 解析上层传过来的数据，并分解任务
@param strWord要解析的字符串
*/
void ParseAllKeyWordData(const CStdString &strWord);

/*
@brief 在keyword目录下创建日期文件夹
@param strDirName 目录名称
*/
void CreateDateDir(CString strDirName);

/*
@brief 在keyword目录下创建公司名目录保存快照
@param strDirName 目录名称
*/
void CreateCompanyDir(CString strDirName);

/*
@brief 把不同用户的公司名装进vector中
@param strComName 公司名称
*/
void SaveCompanyName(CString strComName);

/*
@brief 解析每条关键词数据
@param strKeyWord要解析的关键词数据字符串
*/
void ParseKeyWordData(const CStdString &strKeyWord);

/*
@brief  取任务
@return 任务信息指针
*/
pKeyWordDataInfo GetSearKeyWordInfo();

/*
@brief  加载SiteList.txt文件内容到strWeb
@return
*/
void GetDefaultWeb();

/*
@brief  宽字符转换成多字节
@param  pChar  宽字符
@return 返回多字节
*/
CStringA WideToChar(const wchar_t *pWide, DWORD dwCode = CP_ACP);

//清理百度、搜搜、360等搜索引擎缓存
void DeleteSearchCache();

//URLEncode---UTF-8编码
BYTE toHex(const BYTE &x);
CString UrlEncode(CString sIn);

//URLDecode---UTF-8解码
CString Utf8ToStringT(LPSTR str);
CString UrlDecode(LPCTSTR url);

// 获取CPU核心数
int GetCPUCoreNums();

//替换网页特殊字符
void ReplaceHtmlChar(CString &strData);

CStringA CStrW2CStrA(const CStringW &cstrSrcW);
CStringW CStrA2CStrW(const CStringA &cstrSrcA);
bool SplitCString(const CString & input, const CString & delimiter, std::vector<CString >& results, bool includeEmpties);

//删除特定类型文件
void Recurse(LPCTSTR _pstr, LPCTSTR _fileType, bool _bRecurse);