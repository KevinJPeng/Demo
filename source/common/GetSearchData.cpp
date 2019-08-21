#include "stdafx.h"
#include "GetSearchData.h"


CGetSearchData::CGetSearchData(void)
{
	m_BaiduPR = _T("");
	m_GooglePR = _T("");
	m_AntiChain = _T("");
	m_InnerChain = _T("");
	m_DomainAge = _T("");
	m_Num = _T("");
	m_CompanyName = _T("");
	m_AuditTime = _T("");
	m_BaiduIncluded = _T("");
	m_GoogleIncluded = _T("");
	m_360Included = _T("");
	m_SougouIncluded = _T("");
	m_BaiduAntiChain = _T("");
	m_GoogleAntiChain = _T("");
	m_360AntiChain = _T("");
	m_SougoAntiChain = _T("");
}


CGetSearchData::~CGetSearchData(void)
{
}


CString CGetSearchData::GetSearchKeywordData(CString strData)
{
	return strData;
}


CString CGetSearchData::GetSearchWebData(const CString strData, int flag)
{
	int res = strData.Find(_T("999"));
	if (-1 == res)
	{
		
	}
	else
	{
		switch (flag)
		{
		case 1:
			m_BaiduPR = FindValue(strData, _T("%SEO-BaiduPR%"));
			return m_BaiduPR;
		case 2:
			m_GooglePR = FindValue(strData, _T("%SEO-GooglePR%"));
			return m_GooglePR;
		case 3:
			m_AntiChain = FindValue(strData, _T("%SEO-AntiChain%"));
			return m_AntiChain;
		case 4:
			m_InnerChain = FindValue(strData, _T("%SEO-InnerChain%"));
			return m_InnerChain;
		case 5:
			m_DomainAge = FindValue(strData, _T("%SEO-DomainAge%"));
			return m_DomainAge;
		case 6:
			m_Num = FindValue(strData, _T("%ICP-Num%"));
			return m_Num;
		case 7:
			m_CompanyName = FindValue(strData, _T("%ICP-CompanyName%"));
			return m_CompanyName;
		case 8:
			m_AuditTime = FindValue(strData, _T("%ICP-AuditTime%"));
			return m_AuditTime;
		case 9:
			m_BaiduIncluded = FindValue(strData, _T("%Baidu-Included%"));
			return m_BaiduIncluded;
		case 10:
			m_GoogleIncluded = FindValue(strData, _T("%Google-Included%"));
			return m_GoogleIncluded;
		case 11:
			m_360Included = FindValue(strData, _T("%360-Included%"));
			return m_360Included;
		case 12:
			m_SougouIncluded = FindValue(strData, _T("%Sougou-Included%"));
			return m_SougouIncluded;
		case 13:
			m_BaiduAntiChain = FindValue(strData, _T("%Baidu-AntiChain%"));
			return m_BaiduAntiChain;
		case 14:
			m_GoogleAntiChain = FindValue(strData, _T("%Google-AntiChain%"));
			return m_GoogleAntiChain;
		case 15:
			m_360Included = FindValue(strData, _T("%360-AntiChain%"));
			return m_360AntiChain;
		case 16:
			m_SougoAntiChain = FindValue(strData, _T("%Sougou-AntiChain%"));
			return m_SougoAntiChain;

		default:
			break;
		}
	}
	
}


CString CGetSearchData::FindValue(const CString strData, CString flag)
{
	m_strResult = _T("");
	CString strTemp = _T("");
	int res = strData.Find(flag);
	int nLength = flag.GetLength();
	strTemp = strData.Right(strData.GetLength() - nLength - res - 1);
	res = strTemp.Find(_T("\,"));
	m_strResult = strTemp.Left(res);
	return m_strResult;
}