#pragma once
#include "CWebBrowser2.h"
#include <vector>
#include "afxwin.h"
using namespace std;

// CWebDlg 对话框

class CWebDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWebDlg)

public:
	CWebDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWebDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_WEB };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	static DWORD WINAPI OpenSearchEngineThread(LPVOID lp);
	DWORD OpenSearchEngine();
	afx_msg LRESULT  GetSearchData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT  RedirectURL(WPARAM wParam, LPARAM lParam);
	pEngineInfo GetEngineInfo();

public:
	CWebBrowser2 m_webBrowser;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonYes();
	afx_msg void OnBnClickedButtonNo();


private:
	int m_iEngineId;
	HANDLE m_ValidationConfirm;
public:
	CButton m_btnOK;
	CButton m_btnCancle;
};
