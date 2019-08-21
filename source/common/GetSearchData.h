#pragma once
class CGetSearchData
{
public:
	CGetSearchData(void);
	~CGetSearchData(void);

	//关键字查询数据接口
	CString GetSearchKeywordData(CString strData);

	//网站综合查询数据接口
	CString GetSearchWebData(const CString strData, int flag);

	//处理数据找到需要的值
	CString FindValue(const CString strData, CString flag);


private:
	//网站综合查询的数据
	CString m_strResult;   //返回的结果值
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
};

