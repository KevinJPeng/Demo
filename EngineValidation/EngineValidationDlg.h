
// EngineValidationDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include <vector>
using namespace std;


// CEngineValidationDlg 对话框
class CEngineValidationDlg : public CDialogEx
{
// 构造
public:
	CEngineValidationDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_ENGINEVALIDATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	static DWORD WINAPI WorkThread(LPVOID pThis);
	void staticControlInit(bool _bFlag, int _index);
	vector<CString> GetKeyWords();
	//URL编码
	CString  UrlEncode(CString sIn);
	//宽字节转多字节
	char* CStringToMutilChar(CString& str, int& chLength, WORD wPage);
	BYTE toHex(const BYTE &x);

	static DWORD WINAPI ReDrawUIThread(LPVOID pThis);
	void ReDrawUI();
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonGo();
	CStatic m_s11;
	CStatic m_s12;
	CStatic m_s21;
	CStatic m_s22;
	CStatic m_s31;
	CStatic m_s32;
	CStatic m_s41;
	CStatic m_s42;
	CStatic m_s51;
	CStatic m_s52;
	CStatic m_s61;
	CStatic m_s62;
	CStatic m_s71;
	CStatic m_s72;
	CStatic m_s81;
	CStatic m_s82;
	CComboBox m_comboEngineType;
//	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnSelchangeComboEnginename();
//	CEdit m_editMark1;
	CEdit m_editMark1;
//	CEdit m_editMark2;
	CEdit m_editMark2;
	CEdit m_editMark3;
	CEdit m_editMark4;
	CEdit m_editMark5;
	CEdit m_editMark6;
	CEdit m_editMark7;
	CEdit m_editMark8;
};
