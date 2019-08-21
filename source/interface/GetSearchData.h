#pragma once



struct Keyword
{
	CStdString strKeyword;
	CStdString strBaiduSearchNums;
	CStdString strBaiduIndex;
	CStdString strHeatLevel;
	CStdString strDifficultyLevel;
};

class CGetSearchData
{
public:
	CGetSearchData(void);
	~CGetSearchData(void);

	//关键字查询数据接口
	bool GetSearchKeywordData(CString strData, Keyword &keyword);

	//网站综合查询数据接口
	CString GetSearchWebData(const CString strData, int flag);

	//处理数据找到需要的值
	CString FindValue(const CString strData, CString flag);

	//计算搜索条数分值
	void ExecuteBaiduSearchScore(DWORD dwBaiduSearchNums);

	//计算商贸网站分值
	void ExecuteB2bWebsiteScore(DWORD dwB2bWebsites);

	//计算竞价分值
	void ExecutePPCNumScore(DWORD dwPPCNumScore);

	//计算一级域名分值
	void ExecuteDomainScore(DWORD dwDomains);

	//计算百度产品分值
	void ExecuteBaiduProductScore(DWORD dwBaiduProduct);

	//计算总得分
	void ExecuteTotalScore(DWORD dwBaiduSearchNums, DWORD dwBaiduIndex, DWORD dwB2bWebsites, DWORD dwPPCNumScore
		                , DWORD dwDomains, DWORD dwBaiduProduct);

	//计算难易程度及百度前3页几率
    void ExecuteDifficultyLevel(DWORD dwTotalScore);

	//计算热度
	void ExecuteHeatLevel(DWORD dwHeatLevel);



private:
	//网站综合查询的数据
	//CString m_HandleUrl;       //返回的结果值
	//CString m_strResult;
	CString m_Domain;
	CString m_BaiduPR;
	CString m_GooglePR;
	CString m_AntiChain;
	CString m_InnerChain;
	CString m_DomainAge;
	CString m_Num;
	CString m_CompanyName;
	CString m_AuditTime;
	CString m_BaiduIncluded;
	CString m_GoogleIncluded;
	CString m_360Included;
	CString m_SougouIncluded;
	CString m_BaiduAntiChain;
	CString m_GoogleAntiChain;
	CString m_360AntiChain;
	CString m_SougoAntiChain;
	
	//关键词分析的数据
	/*CString m_strKeyword;*/ //关键词
	DWORD m_dwBaiduSearchNums; //百度搜索条目数
	DWORD m_dwBaiduIndex; //百度指数
	DWORD m_dwB2bWebsites; //商贸网站数
	DWORD m_dwPPCNums; //竞价数量
	DWORD m_dwDomains; //一级域名数量
	DWORD m_dwBaiduProduct; //百度产品
	DWORD m_dwBaiduSearchNumScore; //百度搜索条目分值
	DWORD m_dwBaiduIndexScore; //百度指数分值
	DWORD m_dwB2bWebsiteScore; //商贸网站数分值
	DWORD m_dwPPCNumsScore; //竞价数量分值
	DWORD m_dwDomainScore; //一级域名分值
	DWORD m_dwBaiduProductScore; //百度产品分值
	DWORD m_dwTotalScore;

	CString m_strDifficultyLevel; //难易程度
	CString m_strHeatLevel;     //热度
	CString m_strTop3Rate; //出现百度前3页几率

};

