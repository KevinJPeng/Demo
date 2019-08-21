#include "stdafx.h"
#include "CheckNewVersion.h"


CCheckNewVersion::CCheckNewVersion(WPARAM nVersion)
{
	m_nNewVerFlag = nVersion;
}

CCheckNewVersion::~CCheckNewVersion(void)
{

};


BOOL CCheckNewVersion::CheckPro(void)
{
	return TRUE;
}

BOOL CCheckNewVersion::RepairPro(void)
{
	return TRUE;
}

void CCheckNewVersion::GetBackCheckStruct(T_PROBLEM_DATA &tData)
{
	tData.strIndex = _T("4");
	//等于0不是最新版本
	if (m_nNewVerFlag == 0)
	{
		tData.strSuggestion = ProblemCheckNewVersionSuggest;
		tData.bIsRepair = true;
	}
	else
	{
		tData.bCheckFlag = true;
	}
}

void CCheckNewVersion::GetBackRepairStruct(T_PROBLEM_DATA &tData)
{
	tData.strIndex = _T("4");
}