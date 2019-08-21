#pragma once;

class CCheckNewVersion:public CDiagnoseBase
{
public:
	CCheckNewVersion(WPARAM nVersion);
	virtual ~CCheckNewVersion(void);
public:
	virtual BOOL CheckPro(void);
	virtual BOOL RepairPro(void);
	virtual void GetBackCheckStruct(T_PROBLEM_DATA &tData);
	virtual void GetBackRepairStruct(T_PROBLEM_DATA &tData);

private:
	DWORD m_nNewVerFlag;         //新版本标记
};