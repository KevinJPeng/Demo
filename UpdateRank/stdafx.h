// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

// #if !defined _DEBUG
#pragma comment(linker, "/subsystem:\"Windows\" /entry:\"mainCRTStartup\"")//ȥ��console����
// #endif

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS         // �Ƴ��Ի����е� MFC �ؼ�֧��

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
#endif

#include <afx.h>
#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>
#include <afxdisp.h>
#include <afxcontrolbars.h>



// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
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
*  һЩ���ݶ���
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

// ������������
#define SEARCHFLAGCOUNT  10

#define DECRYPTCFG					_T("szw^1*3&cfg")

// �������
enum
{
	SEARCH_BAIDU = 0x01,               // �ٶ�        
	SEARCH_360 = 0x02,               // 360����
	SEARCH_SOGOU = 0x04,               // �ѹ�
	SEARCH_BING = 0x08,               // ��Ӧ
	SEARCH_YOUDAO = 0x10,               // �е�
	SEARCH_PHONEBAIDU = 0x20,               // �ֻ��ٶ�
	SEARCH_PHONESOGOU = 0x40,               // �ֻ��ѹ�
	SEARCH_PHONE360 = 0x80,				//�ֻ�360
	SEARCH_WEIXIN = 0xF0,				//΢��
	SEARCH_PHONESHENMA = 0x0100,		//�ֻ�����
	SEARCH_Count = 10,					//��������
};


#define OPEN_PAGE_NORMAL           -2
#define OPEN_PAGE_FAILED           -1
#define NOT_FOUND_COMPANY          0
#define FOUND_COMPANY              1
#define PAGE_NOT_FOUND             2
#define PAIMING_EXIST              3
#define NO_PAIMING                 4
#define ENGINE_APPEAR_VERIFYCODE   5

#define GET_WEBDATA_NORMAL			6		//������ȡ��ҳ����
#define GET_WEBDATA_EXCEPTION		7		//��ҳ���쳣������5XX,�������򿪵�����Ϊ�գ�
#define PAGE_OFFICIAL_OPENFAILED    8		//������ʧ��
#define PAGE_FUNTION_ERROR			9		//����ִ��ʧ��

//��λitem
enum
{
	LOCATION_ITEM_TAG_NULL = 0,					// ���Ա�ǩ��λ        
	LOCATION_ITEM_TAG_ID = 1,						// ͨ����ǩ��ID��λ        
	LOCATION_ITEM_TAG_CLASS = 2,					// ͨ����ǩ��������λ        
	LOCATION_ITEM_TAG_ATTRIBUTE = 3,				// ͨ����ǩ�����Զ�λ        
	LOCATION_ITEM_TAG_INDEX = 4,					// ͨ����ǩ�ͱ��ģ����λ        
};

typedef struct _locationStruct
{
	int iLocationType;		//��λ����
	CString sTagType;			//��ǩ����
	long lFuzzyIndex;			//ģ����λid
	CString sKey;				//��λ��
	CString sValue;			//��λֵ
}locationStruct, *plocationStruct;



//��ҳ�ܵĴ�״̬
enum
{
	E_ALLURLOPEN_STATE0 = 0,		//��ʼ״̬
	E_ALLURLOPEN_STATE1,			//�й�������
	E_ALLURLOPEN_STATE2,			//����ͨ����
	E_ALLURLOPEN_STATE3,			//����ͨ����������û�򿪣� ����-2
	E_ALLURLOPEN_STATE4,			//û����������ͨ��վû�򿪣� ����-2
	E_ALLURLOPEN_STATE5			//û�������й���û�򿪣� ����-2
};
//��ҳ��ǰ��״̬
enum
{
	E_CURRENTURLOPEN_STATE0 = 0,		//�й�������
	E_CURRENTURLOPEN_STATE1,			//����ͨ������
	E_CURRENTURLOPEN_STATE2,			//����û��
	E_CURRENTURLOPEN_STATE3,			//��ͨ��û��
};

enum
{
	ELEMENT_MARK_INNERTEXT = 0,			//�ڵ�����
	ELEMENT_MARK_HREF = 1,					//ͨ��href��ȡֵ
	ELEMENT_MARK_ATTRIBUTE = 2,			//ͨ�����Ի�ȡֵ
};
typedef struct _targetAddressSign
{
	int iUrlGetType;			//URL��ȡ����
	CString sHrefKey;			//��
}targetAddressSign, *ptargetAddressSign;

#define ADD_SEPARATOR_1 _T(",")
#define ADD_SEPARATOR_2 _T(";")
//�ָ�������
enum
{
	ELEMENT_SEPARATOR_1 = 0,			//�ָ���1(",")
	ELEMENT_SEPARATOR_2 = 1,			//�ָ���2(";")
};
//ץȡҳ�淵�����ݱ��
typedef struct _Flag_CatchInfo
{
	BOOL	bIsOfficialFlag;		//�������
	BOOL	bIsAceFlag;			//2�����
	BOOL	bSpecialOfficialFlag;	//���������ǣ����ڴ�Ŀ��ҳ�������������ת�ģ���Ҫ���ų� Ŀ��pcվ/Ŀ��wapվ/��������վ/ת��վ֮���վ������Ϊ����վ�����ý�ȥץȡ��

	_Flag_CatchInfo()
	{
		bIsOfficialFlag = FALSE;
		bIsAceFlag = FALSE;
		bSpecialOfficialFlag = FALSE;
	}
}CatchInfo, *pCatchInfo;

// �ؼ���������Ϣ
typedef struct _Tag_KeyWordDataInfo
{
	bool					bOnlyRareWord;
	int                   iFlag;            // ������־
	int                   iCurPage;         // ��ǰ�ڼ�ҳ
	int					  iIndex;			//������������
	int					  iPublishType;		//�ƹ����ͣ�1����ͨ�ƹ㣻2�������ƹ㣻3:1��2��4�����ţ�5:1��4��6:2��4��
	CString				  strClientType;	//�ͻ����ͣ�����ʲô�ͻ���
	CString               strKey;           // �ؼ���Key
	CString               strKeyWordName;   // �����ؼ�����
	CString               strKeyHex;        // �����ؼ���16������
	CString               strUrl;           // Ŀ����ַ
	CStringA              strWebList;       // �۴�ʦ��������վ�б�
	CString               strWebFlag;       // ��վ���0����ʾ������վ�б�ֻץȡ�۴�ʦ��¼��������վ
	//         1����ʾץȡ��վ�б������۴�ʦ��¼����վ
	//         2����ʾ��ץȡ��վ�б���ָ������վ
	//		   3����ʾ�ų��۴�ʦ��¼����վ�ų���վ�б�
	//		   4����ʾ3+ �����б�
	CString               strComany;        // ��˾��
	CString				  strWeixinName;	//΢�Ź��ں�����
	std::vector<CString>  vOfficialList;	//�����б�
	std::vector<CString>  vCompanys;        // ��˾��(�����������)
	std::vector<CStdString>  vAllCompanys;		//������˾��ƺ�ȫ�ƣ�

	std::vector<CStdString>  vCompanysTag;        // ��Ƨ���жϹ�˾(,���, ��  ����ƣ�)

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


// ������������
typedef struct _Tag_BackDataInfo
{
	int         iRank;                      // �ؼ�������(1~30)
	int         iFlag;                      // ���������־
	int			iRankCnt;					//�ؼ�����������
	int			iB2BFlag;					//B2B��־
	int			iOfficialFlag;				//������־
	int			iAceFlag;					//���Ʊ��
	int			iKeywordType;				//�ؼ������ͣ�0���ùؼ��ʲ������	1���ùؼ��ʴ���棩
	CString     strKeyWordName;             // �ؼ�������
	CString     strKey;                     // �ؼ���Key
	CString     strPagePath;                // ����·��
	CString     strCompanyName;             // ��˾��
	CString		strRecordInfo;			 //������¼  ��վID,����λ��(;2)��վID,����λ��
	CString		strRedirectUrl;			 //�ض�����Url
	CString		strRecordErrorInfo;		//��¼��ʧ�ܵ���վ  ��վID,-2(;2)��վID,-2


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

//������վ���⴦��ı��
typedef struct _SpecialUrlFlag
{
	//int iEngID;
	CString	strSiteName;			//��վ��
	CStringA	strMarkerFlag1;		//��ȡ���1
	CStringA	strMarkerFlag2;		//��ȡ���2
	CString strJsUrl;				//�̶���URL·��
}SpecialUrlFlag;

//������վ����Ԥ����
typedef struct _pageDataPre
{
	//int iEngID;
	CString	strDomain;			//����
	CString	strBeforeData;		//ԭʼ����
	CString	strAfterData;			//�滻�������
}pageDataPre;

typedef struct _ItemLabel
{
	CString strlable;				//��ǩ��
	CString strAttributeName;		//������
	CString strAttributeValue;		//����ֵ
}ItemLabel, *pItemLabel;

typedef struct _KeyData
{
	CString strUrlTitle;		//
	CString strUrlLink;		//
}KeyData, *pKeyData;

typedef struct _TagNode
{
	CString sTag;			//��ǩ
	CString skey;			//���ԣ�����
	CString sValue;		//���ԣ�ֵ��
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

//��ҳԪ�ر��ƥ���������ʽ
typedef struct _Elem_MatchFlag
{
	CString		strJumpUrl;							//��ת��ַ
	CString		strCollTags;						//Ԫ�ؼ���
	CString		strItemFlag1;						//���˲�����Ҫ���ҵ�ITEM��ǩ
	CString		strItemFlag2;						//....
	CString     strFindUrlFlag1;					//����URL���
	CString		strFindUrlFlag2;					//���ұ��
	CString		strFindUrlFlag3;					//....
	CString		strBtnId;							//��ťID��ʶ����
	CString		strHtmlName;						//�ļ����ƺ�׺
	//���ñ��
	CString		strGetUrlFlag1;						//����Ŀ����ַ�ı�ǣ�
	CString		strGetUrlFlag2;						//...
	CString		strJudgeHtmlTag;					//�ж�
	CString		strLocationAd;						//�жϸùؼ����Ƿ���й��

	//���ܱ�ǣ�����ÿ���������Ҫ��Ϊֵ��
	CString		strUniversalFst;					//��һ�����ܱ��  �ٶȣ����ҽ�����ǣ�360����Ŀ���ַ��ȡ��ʵurl��ǣ��е�������ʱ����ұ��
	CString		strUniversalSnd;					//�ڶ������ܱ��
	CString		strCssPath;							//Css�ļ�·��

	int			iSearchPageNum;					//����ҳ�����
	CString		sKWSearchMethod;					//�ؼ��ʴ򿪷�ʽ��0���ؼ���ƴ�������Ӻ���   1���ؼ������뵽���������ٶ�һ�°�ť��
	CString		sHomePage;						//����������ҳ
	TagNode		tagKWEdit;						//�ؼ��������
	TagNode		tagSearchBtn;						//�ٶ�һ�°�ť
	TagNode		tagNextPage;						//��һҳ
	TagNode		tagMainBody;						//

	//����ץȡ��ǣ�
	std::vector<locationStruct>		vstrLocationTitle;
	targetAddressSign			strTargetTitle;
	std::vector<locationStruct>		vstrLocationLink;
	targetAddressSign			strTargetLink;

	std::vector<CStdString>	vTargetMarkWeb;			//���Ұ���Ŀ����ַ����վ
	std::vector<CStdString> vTargetKey;				//���Ұ���Ŀ����ַ�Ĺؼ��֣�	
	std::vector<CStdString>	vTargetStart;			//��λĿ���ǰ��ؼ��ʣ�
	std::vector<CStdString>	vTargetEnd;				//��λĿ�����Ĺؼ���
	std::vector<CStdString>	vTargetPreLen;			//Ҫ����ǰ��ĳ��ȣ�
	std::vector<CStdString> vTargetLstLen;			//���Һ���ĳ��ȣ�
	std::vector<CStdString> vReplaceTagWebs;		//�滻��ǩ����վ

	std::map<CStdString, DWORD> mapAceWebToID;		//������ַ��Ӧ��վID;

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

//��������iFlag��ǵ������õ�ӳ��
extern std::map<int, _Elem_MatchFlag> g_mapElemCfg;

extern sFingerRecogn g_sFingerRecogn;

// ������������iFlag��ӳ��
extern std::map<CString, int> g_mapSearch;

// �洢���йؼ���
extern std::vector<pKeyWordDataInfo> g_vAllKeyWords;

// ���治ͬ�û��Ĺ�˾��
extern std::vector<CString> g_vDiffCompany;

// ��������(�Ƿ�����)
extern volatile BOOL bSearchFlag[];

// ���ش�������socket�ӿ������������ݵ�����
extern CServerSocket *g_server;

// �ж����û���ˢ������ˢ
extern bool g_bManualRefresh;

// ץȡ������־
extern CLogTrace g_log;

// extern CLogTrace g_debugLog;
// extern CLogTrace g_TaskRecord;

// ������֤�����ҳ�쳣��־
extern CLogTrace g_pLog;

// �ٽ������
extern CRITICAL_SECTION critSection;
extern CRITICAL_SECTION critSendMsg;
extern CRITICAL_SECTION critSearchFlag;
extern CRITICAL_SECTION critCount;

// ��¼ץȡ�ؼ��ʸ���
extern int g_iBaidu;
extern int g_iSogou;
extern int g_iBing;
extern int g_i360;
extern int g_iYouDao;

// ץȡ��վ�б�
extern CStringA  g_strWeb;

// �ؼ���֮�����ʱ
extern int g_iDelayForKeyWord;

// ó��վ֮�����ʱ
extern int g_iDalayForSite;

extern int g_iThreadCount;

//����Ҫץȡ��URL
extern CString g_strDebugUrl;
extern BOOL g_bIsDebug;

const int HtmlCharNum = 17;
const int CHAR_LEN = 2;
extern TCHAR tSpecialChar[HtmlCharNum][CHAR_LEN];


extern CString g_shttpOss;			//���ڿ���·��ǰ��(http:oss)
extern CString g_sKWPath;			//OSS�Ͽ����ļ��������������� ����һ���㷨��ɣ�


//extern IXMLRW *g_DatMc;

/**************************************
* ���ݶ������
**************************************/


char* CStringToMutilChar(CString& str, int& chLength, WORD wPage);

// ��ʼ��map
void InitMap();

/*
@brief �����ϲ㴫���������ݣ����ֽ�����
@param strWordҪ�������ַ���
*/
void ParseAllKeyWordData(const CStdString &strWord);

/*
@brief ��keywordĿ¼�´��������ļ���
@param strDirName Ŀ¼����
*/
void CreateDateDir(CString strDirName);

/*
@brief ��keywordĿ¼�´�����˾��Ŀ¼�������
@param strDirName Ŀ¼����
*/
void CreateCompanyDir(CString strDirName);

/*
@brief �Ѳ�ͬ�û��Ĺ�˾��װ��vector��
@param strComName ��˾����
*/
void SaveCompanyName(CString strComName);

/*
@brief ����ÿ���ؼ�������
@param strKeyWordҪ�����Ĺؼ��������ַ���
*/
void ParseKeyWordData(const CStdString &strKeyWord);

/*
@brief  ȡ����
@return ������Ϣָ��
*/
pKeyWordDataInfo GetSearKeyWordInfo();

/*
@brief  ����SiteList.txt�ļ����ݵ�strWeb
@return
*/
void GetDefaultWeb();

/*
@brief  ���ַ�ת���ɶ��ֽ�
@param  pChar  ���ַ�
@return ���ض��ֽ�
*/
CStringA WideToChar(const wchar_t *pWide, DWORD dwCode = CP_ACP);

//����ٶȡ����ѡ�360���������滺��
void DeleteSearchCache();

//URLEncode---UTF-8����
BYTE toHex(const BYTE &x);
CString UrlEncode(CString sIn);

//URLDecode---UTF-8����
CString Utf8ToStringT(LPSTR str);
CString UrlDecode(LPCTSTR url);

// ��ȡCPU������
int GetCPUCoreNums();

//�滻��ҳ�����ַ�
void ReplaceHtmlChar(CString &strData);

CStringA CStrW2CStrA(const CStringW &cstrSrcW);
CStringW CStrA2CStrW(const CStringA &cstrSrcA);
bool SplitCString(const CString & input, const CString & delimiter, std::vector<CString >& results, bool includeEmpties);

//ɾ���ض������ļ�
void Recurse(LPCTSTR _pstr, LPCTSTR _fileType, bool _bRecurse);