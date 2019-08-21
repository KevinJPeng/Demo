#pragma once
#include "..\..\duilib\utils\winimplbase.h"
#include <algorithm>

class CCheckProWnd :
	public WindowImplBase
{
public:
	CCheckProWnd(void);
	~CCheckProWnd(void);

	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
public:
	void SetParentWnd(HWND hWnd);
	void SetProblemList(const T_DATA_FROM_XML& tProList);
	void HandleCheckRetrunMsg(T_PROBLEM_DATA& tCheckBak);			//处理检测返回消息
	void HandleRepairRetrunMsg(T_PROBLEM_DATA& tRepairBak);			//处理修复返回消息
	BOOL GetCancelFlag();
	void ActiveCheckButton();										//激活诊断项
private:
	void InitialProListCtrl();										//根据检测模块返回的数据显示出问题列表
	void ChangeItemColor(DWORD dwMousePosx, DWORD dwMousePosy);
	void InitialStartCheckUI();										//设置开始诊断状态
	void SetCancelCheckUI();										//设置取消诊断状态
	void SetFinishCheckUI();										//设置完成检测状态
	void SetFinishRepairUI();										//设置完成修复状态
	void SetRepairRunningUI();										//设置用户正在修复界面状态
	BOOL IsProItemCheckFinish();									//是否所有的检测项均已检测完毕

private:
	HWND m_hParentWnd;
	T_DATA_FROM_XML m_tProblemList;		//记录问题列表由检测模块返回
	vector<CString>m_vecCheckBak;		//记录用户点击检测后已经返回的item下标
	vector<CString>m_vecNeedRepair;		//记录需要进行修复的项
	vector<CString>m_vecRepairFail;		//记录修复失败的项
	T_PROBLEM_DATA m_tCheckBackData;	//检测返回的结果结构体
	T_PROBLEM_DATA m_tRepairBackData;	//修复返回的结果结构体
	BOOL m_bIsCancelFlag;
	DWORD m_dwCurrentRepairIndex;		//记录当前用户点击的修复的项
	BOOL m_bIsAllItemCheckOk;			//是否所有的检测项都检测oK
	BOOL m_bIsAllItemRepairOK;			//是否所有的项都修复成功
};

