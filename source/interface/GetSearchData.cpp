#include "stdafx.h"
#include "GetSearchData.h"
#include "..\common\StdString.h"
#include "..\common\StdStrUtils.h"

CGetSearchData::CGetSearchData(void)
{
	m_Domain = _T("");
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

	/*m_strKeyword = _T("");*/
	m_strDifficultyLevel = _T(""); 
	m_strTop3Rate = _T(""); 
	m_strHeatLevel = _T("");
	m_dwBaiduSearchNums = 0; 
	m_dwBaiduIndex = 0; 
	m_dwB2bWebsites = 0; 
	m_dwPPCNums = 0; 
	m_dwDomains = 0; 
	m_dwBaiduProduct = 0; 
	m_dwBaiduSearchNumScore = 0; 
	m_dwBaiduIndexScore = 0; 
	m_dwB2bWebsiteScore = 0;
	m_dwPPCNumsScore = 0; 
	m_dwDomainScore = 0; 
	m_dwBaiduProductScore = 0; 
	m_dwTotalScore = 0;
}


CGetSearchData::~CGetSearchData(void)
{
}



bool CGetSearchData::GetSearchKeywordData(CString strData, Keyword &keyword)
{
	CStdStrUtils utl;
	std::vector<CStdString> vRes;
	utl.SplitString(CStdString(strData.GetBuffer()), _T("(;0)"), vRes);

	/*m_strKeyword = vRes[0];*/
	std::vector <CStdString>::size_type res = vRes.size();
	if (8 == res)
	{
		vRes[1].Replace(_T("\,"), NULL);
		m_dwBaiduSearchNums = _ttol(vRes[1]);
		m_dwBaiduIndex = _ttol(vRes[2]);
		m_dwB2bWebsites = _ttol(vRes[3]);
		m_dwPPCNums = _ttol(vRes[4]);
		m_dwDomains = _ttol(vRes[5]);
		m_dwBaiduProduct = _ttol(vRes[6]);
	}
	else
	{
		return false;
	}

	ExecuteTotalScore(m_dwBaiduSearchNums, m_dwBaiduIndex, m_dwB2bWebsites, m_dwPPCNums, m_dwDomains, m_dwBaiduProduct);
	ExecuteDifficultyLevel(m_dwTotalScore);

	//计算搜索热度 = 商贸网站数 + 竞价广告数 +一级域名数 + 百度产品数
	DWORD dwHeatLevel = m_dwB2bWebsites + m_dwPPCNums + m_dwDomains + m_dwBaiduProduct + 15;
	ExecuteHeatLevel(dwHeatLevel);

	keyword.strKeyword = vRes[0];
	keyword.strBaiduSearchNums = vRes[1];
	keyword.strBaiduIndex = vRes[2];
	keyword.strHeatLevel = m_strHeatLevel;
	keyword.strDifficultyLevel = m_strDifficultyLevel;
	return true;
}


void CGetSearchData::ExecuteTotalScore(DWORD dwBaiduSearchNums, DWORD dwBaiduIndex, DWORD dwB2bWebsites, DWORD dwPPCNumScore
	                                     , DWORD dwDomains, DWORD dwBaiduProduct)
{
	ExecuteBaiduSearchScore(m_dwBaiduSearchNums);

	//计算百度指数分值
	m_dwBaiduIndex == 0 ? m_dwBaiduIndexScore = 1 : m_dwBaiduIndexScore = 10;

	ExecuteB2bWebsiteScore(m_dwB2bWebsites);
	ExecutePPCNumScore(m_dwPPCNums);
	ExecuteDomainScore(m_dwDomains);
	ExecuteBaiduProductScore(m_dwBaiduProduct);

	m_dwTotalScore = m_dwBaiduSearchNumScore + m_dwBaiduIndexScore/* + m_dwB2bWebsiteScore*/
		        +m_dwPPCNumsScore + m_dwDomainScore + m_dwBaiduProductScore;
}


void CGetSearchData::ExecuteHeatLevel(DWORD dwHeatLevel)
{
	double dHeatLevel = ((double)dwHeatLevel)/((double)40)*100;
	if (dHeatLevel < 20) 
	{
		m_strHeatLevel = _T("冷");
	}
	else if (dHeatLevel >= 20 && dHeatLevel < 40) 
	{
		m_strHeatLevel = _T("较冷");
	}
	else if (dHeatLevel >= 40 && dHeatLevel < 60)
	{
		m_strHeatLevel = _T("较热");
	}
	else if (dHeatLevel >= 60 && dHeatLevel < 80) 
	{
		m_strHeatLevel = _T("热");
	}
	else if (dHeatLevel >= 80) 
	{
		m_strHeatLevel = _T("最热");
	}
}


void CGetSearchData::ExecuteBaiduSearchScore(DWORD dwBaiduSearchNums)
{
	if (dwBaiduSearchNums >= 500000 && dwBaiduSearchNums < 1000000) 
	{
		m_dwBaiduSearchNumScore = 8;
	}
	else if (dwBaiduSearchNums >= 1000000 && dwBaiduSearchNums < 3000000) 
	{
		m_dwBaiduSearchNumScore = 15;
	}
	else if (dwBaiduSearchNums >= 3000000 && dwBaiduSearchNums < 5000000) 
	{
		m_dwBaiduSearchNumScore = 25;
	}
	else if (dwBaiduSearchNums >= 5000000) 
	{
		m_dwBaiduSearchNumScore = 35;
	}
}


void CGetSearchData::ExecutePPCNumScore(DWORD dwPPCNums)
{
	if (dwPPCNums == 0) 
	{
		m_dwPPCNumsScore = 1;

	}
	else if (dwPPCNums > 0 && dwPPCNums < 4) 
	{
		m_dwPPCNumsScore = 3;
	}
	else if (dwPPCNums >= 4 && dwPPCNums < 7) 
	{
		m_dwPPCNumsScore = 7;
	}
	else if (dwPPCNums >= 7 && dwPPCNums < 9) 
	{
		m_dwPPCNumsScore = 11;
	}
	else if (dwPPCNums >= 9 && dwPPCNums < 11) 
	{
		m_dwPPCNumsScore = 15;
	}

}


void CGetSearchData::ExecuteB2bWebsiteScore(DWORD dwB2bWebsites)
{
	if (dwB2bWebsites == 0) 
	{
		m_dwB2bWebsiteScore = 20;
		//SearchHot = 63;
	}
	else if (dwB2bWebsites >= 0 && dwB2bWebsites < 5) 
	{
		m_dwB2bWebsiteScore = 15;
		//SearchHot = 50.4;
	}
	else if (dwB2bWebsites >= 5 && dwB2bWebsites < 9) 
	{
		m_dwB2bWebsiteScore = 10;
		//SearchHot = 37.8;
	}
	else if (dwB2bWebsites >= 9 && dwB2bWebsites < 13)
	{
		m_dwB2bWebsiteScore = 5;
		//SearchHot = 25.2;
	}
	else if (dwB2bWebsites >= 13) 
	{
		m_dwB2bWebsiteScore = 1;
		//SearchHot = 12.6;
	}

}


void CGetSearchData::ExecuteDomainScore(DWORD dwDomains)
{
	if (dwDomains >= 0 && dwDomains < 4) 
	{
		m_dwDomainScore = 1;
	}
	else if (dwDomains >= 4 && dwDomains < 9) 
	{
		m_dwDomainScore = 8;
	}
	else if (dwDomains >= 9 && dwDomains < 14) 
	{
		m_dwDomainScore = 15;
	}
	else if (dwDomains >= 14 && dwDomains < 19) 
	{
		m_dwDomainScore = 25;
	}
	else if (dwDomains >= 19) 
	{
		m_dwDomainScore = 35;
	}
}


void CGetSearchData::ExecuteBaiduProductScore(DWORD dwBaiduProduct)
{
	if (dwBaiduProduct == 1 || dwBaiduProduct == 0) 
	{
		m_dwBaiduProductScore = 1;
	}
	else if (dwBaiduProduct == 2) 
	{
		m_dwBaiduProductScore = 2;
	}
	else if (dwBaiduProduct == 3) 
	{
		m_dwBaiduProductScore = 3;
	}
	else if (dwBaiduProduct == 4) 
	{
		m_dwBaiduProductScore = 4;
	}
	else if (dwBaiduProduct > 4) 
	{
		m_dwBaiduProductScore = 5;
	}

}


void CGetSearchData::ExecuteDifficultyLevel(DWORD dwTotalScore)
{
	if (dwTotalScore < 20) 
	{
		m_strDifficultyLevel = "易";
// 		Top3Rate = "70%-90%";
// 		DifficultLevelValue = 1;
	}
	else if (dwTotalScore >= 20 && dwTotalScore < 40) 
	{
		m_strDifficultyLevel = "较易";
// 		Top3Rate = "50%-70%";
// 		DifficultLevelValue = 2;
	}
	else if (dwTotalScore >= 40 && dwTotalScore < 60)
	{
		m_strDifficultyLevel = "较难";
// 		Top3Rate = "30%-50%";
// 		DifficultLevelValue = 3;
	}
	else if (dwTotalScore >= 60 && dwTotalScore < 80) 
	{
		m_strDifficultyLevel = "难";
// 		Top3Rate = "10%-30%";
// 		DifficultLevelValue = 4;
	}
	else if (dwTotalScore >= 80) 
	{
		m_strDifficultyLevel = "最难";
// 		Top3Rate = "0%-10%";
// 		DifficultLevelValue = 5;
	}
}


CString CGetSearchData::GetSearchWebData(const CString strData, int flag)
{
	switch (flag)
	{
	case 0:
		{
			m_Domain = FindValue(strData, _T("HandleUrl"));
			return m_Domain;
		}
		break;
	case 1:
		{
			m_BaiduPR = FindValue(strData, _T("SEO-BaiduPR"));
			return m_BaiduPR;
		}
		break;
	case 2:
		{
			int res = 0;
			m_GooglePR = FindValue(strData, _T("SEO-GooglePR"));
			res = m_GooglePR.Find(_T("-"));
			if (res == 0)
			{
				return _T("0");
			}
			else
			{
				return m_GooglePR;
			}	
		}
		break;
	case 3:
		{
			m_AntiChain = FindValue(strData, _T("SEO-AntiChain"));
			return m_AntiChain;
		}
		break;
	case 4:
		{
			m_InnerChain = FindValue(strData, _T("SEO-InnerChain"));
			return m_InnerChain;
		}
		break;
	case 5:
		{
			m_DomainAge = FindValue(strData, _T("SEO-DomainAge"));
			return m_DomainAge;
		}
		break;
	case 6:
		{
			m_Num = FindValue(strData, _T("ICP-Num"));
			return m_Num;
		}
		break;
	case 7:
		{
			m_CompanyName = FindValue(strData, _T("ICP-CompanyName"));
			return m_CompanyName;
		}
		break;
	case 8:
		{
			m_AuditTime = FindValue(strData, _T("ICP-AuditTime"));
			return m_AuditTime;
		}
		break;
	case 9:
		{
			m_BaiduIncluded = FindValue(strData, _T("Baidu-Included"));
			return m_BaiduIncluded;
		}
		break;
	case 10:
		{
			m_GoogleIncluded = FindValue(strData, _T("Google-Included"));
			return m_GoogleIncluded;
		}
		break;
	case 11:
		{
			m_360Included = FindValue(strData, _T("360-Included"));
			return m_360Included;
		}
		break;
	case 12:
		{
			m_SougouIncluded = FindValue(strData, _T("Sougou-Included"));
			return m_SougouIncluded;
		}
		break;
	case 13:
		{
			m_BaiduAntiChain = FindValue(strData, _T("Baidu-AntiChain"));
			return m_BaiduAntiChain;
		}
		break;
	case 14:
		{
			m_GoogleAntiChain = FindValue(strData, _T("Google-AntiChain"));
			return m_GoogleAntiChain;
		}
		break;
	case 15:
		{
			m_360AntiChain = FindValue(strData, _T("360-AntiChain"));
			return m_360AntiChain;
		}
		break;
	case 16:
		{
			m_SougoAntiChain = FindValue(strData, _T("Sougou-AntiChain"));
			return m_SougoAntiChain;
		}
		break;

	default:
		break;
	}		
}


CString CGetSearchData::FindValue(const CString strData, CString flag)
{
	CString strResult = _T("");
	int res = strData.Find(flag);
	if (res == -1)
	{
		return strResult;    //没有找到对应标记的直接返回空
	}
	int nLength = flag.GetLength();
	CString strTemp = strData.Right(strData.GetLength() - nLength - res - 2);
	res = strTemp.Find(_T("\,"));
	strResult = strTemp.Left(res);
	return strResult;
}