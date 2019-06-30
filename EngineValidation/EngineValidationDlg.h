
// EngineValidationDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include <vector>
using namespace std;


// CEngineValidationDlg �Ի���
class CEngineValidationDlg : public CDialogEx
{
// ����
public:
	CEngineValidationDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_ENGINEVALIDATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	static DWORD WINAPI WorkThread(LPVOID pThis);
	void staticControlInit(bool _bFlag, int _index);
	vector<CString> GetKeyWords();
	//URL����
	CString  UrlEncode(CString sIn);
	//���ֽ�ת���ֽ�
	char* CStringToMutilChar(CString& str, int& chLength, WORD wPage);
	BYTE toHex(const BYTE &x);

	static DWORD WINAPI ReDrawUIThread(LPVOID pThis);
	void ReDrawUI();
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
