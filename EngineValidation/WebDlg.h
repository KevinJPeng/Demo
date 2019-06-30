#pragma once
#include "CWebBrowser2.h"
#include <vector>
using namespace std;

// CWebDlg �Ի���

class CWebDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWebDlg)

public:
	CWebDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CWebDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG_WEB };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
};
