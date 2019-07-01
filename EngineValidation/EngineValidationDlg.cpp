
// EngineValidationDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EngineValidation.h"
#include "EngineValidationDlg.h"
#include "afxdialogex.h"
#include "WebDlg.h"
#include "data.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
	EnableActiveAccessibility();
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CEngineValidationDlg �Ի���



CEngineValidationDlg::CEngineValidationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEngineValidationDlg::IDD, pParent)
{
	EnableActiveAccessibility();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	g_notifyUiEvent = ::CreateEvent(NULL, false, false, NULL);
}

void CEngineValidationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_11, m_s11);
	DDX_Control(pDX, IDC_STATIC_12, m_s12);
	DDX_Control(pDX, IDC_STATIC_21, m_s21);
	DDX_Control(pDX, IDC_STATIC_22, m_s22);
	DDX_Control(pDX, IDC_STATIC_31, m_s31);
	DDX_Control(pDX, IDC_STATIC_32, m_s32);
	DDX_Control(pDX, IDC_STATIC_41, m_s41);
	DDX_Control(pDX, IDC_STATIC_42, m_s42);
	DDX_Control(pDX, IDC_STATIC_51, m_s51);
	DDX_Control(pDX, IDC_STATIC_52, m_s52);
	DDX_Control(pDX, IDC_STATIC_61, m_s61);
	DDX_Control(pDX, IDC_STATIC_62, m_s62);
	DDX_Control(pDX, IDC_STATIC_71, m_s71);
	DDX_Control(pDX, IDC_STATIC_72, m_s72);
	DDX_Control(pDX, IDC_STATIC_81, m_s81);
	DDX_Control(pDX, IDC_STATIC_82, m_s82);
	DDX_Control(pDX, IDC_COMBO_ENGINENAME, m_comboEngineType);
	DDX_Control(pDX, IDC_EDIT_MARK1, m_editMark1);
	//  DDX_Control(pDX, IDC_EDIT_MARK2, m_editMark1);
	//  DDX_Control(pDX, IDC_EDIT_MARK3, m_editMark2);
	DDX_Control(pDX, IDC_EDIT_MARK2, m_editMark2);
	DDX_Control(pDX, IDC_EDIT_MARK3, m_editMark3);
	DDX_Control(pDX, IDC_EDIT_MARK4, m_editMark4);
	DDX_Control(pDX, IDC_EDIT_MARK5, m_editMark5);
	DDX_Control(pDX, IDC_EDIT_MARK6, m_editMark6);
	DDX_Control(pDX, IDC_EDIT_MARK7, m_editMark7);
	DDX_Control(pDX, IDC_EDIT_MARK8, m_editMark8);
}

BEGIN_MESSAGE_MAP(CEngineValidationDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_GO, &CEngineValidationDlg::OnBnClickedButtonGo)
//	ON_CBN_SELCHANGE(IDC_COMBO1, &CEngineValidationDlg::OnCbnSelchangeCombo1)
ON_CBN_SELCHANGE(IDC_COMBO_ENGINENAME, &CEngineValidationDlg::OnCbnSelchangeComboEnginename)
END_MESSAGE_MAP()


// CEngineValidationDlg ��Ϣ�������

BOOL CEngineValidationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	m_comboEngineType.AddString(_T("ȫ��"));
	m_comboEngineType.AddString(_T("�ٶ�"));
	m_comboEngineType.AddString(_T("360"));
	m_comboEngineType.AddString(_T("�ѹ�"));
	m_comboEngineType.AddString(_T("��Ӧ"));
	m_comboEngineType.AddString(_T("�ֻ��ٶ�"));
	m_comboEngineType.AddString(_T("�ֻ�360"));
	m_comboEngineType.AddString(_T("�ֻ��ѹ�"));
	m_comboEngineType.AddString(_T("�ֻ�����"));
	m_comboEngineType.SetCurSel(0);


	m_editMark1.SetWindowTextW(_T("��֤��"));
	m_editMark2.SetWindowTextW(_T("��֤��"));
	m_editMark3.SetWindowTextW(_T("��֤��"));
	m_editMark4.SetWindowTextW(_T("��֤��"));
	m_editMark5.SetWindowTextW(_T("��֤��"));
	m_editMark6.SetWindowTextW(_T("��֤��"));
	m_editMark7.SetWindowTextW(_T("��֤��"));
	m_editMark8.SetWindowTextW(_T("��֤��"));

	CreateThread(NULL, NULL, ReDrawUIThread, this, 0, 0);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CEngineValidationDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CEngineValidationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CEngineValidationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


DWORD WINAPI CEngineValidationDlg::WorkThread(LPVOID _pThis)
{
	CEngineValidationDlg* pThis = (CEngineValidationDlg*)_pThis;
	CoInitialize(NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	AfxEnableControlContainer();
	CWebDlg  photoDlg;
	photoDlg.DoModal();
	CoUninitialize();
	return 0;
}



//void CEngineValidationDlg::OnCbnSelchangeCombo1()
//{
//	// TODO:  �ڴ���ӿؼ�֪ͨ����������
//}

void CEngineValidationDlg::staticControlInit(bool _bFlag, int _index)
{
	m_s11.EnableWindow(_bFlag);
	m_s12.EnableWindow(_bFlag);
	m_s21.EnableWindow(_bFlag);
	m_s22.EnableWindow(_bFlag);
	m_s31.EnableWindow(_bFlag);
	m_s32.EnableWindow(_bFlag);
	m_s41.EnableWindow(_bFlag);
	m_s42.EnableWindow(_bFlag);
	m_s51.EnableWindow(_bFlag);
	m_s52.EnableWindow(_bFlag);
	m_s61.EnableWindow(_bFlag);
	m_s62.EnableWindow(_bFlag);
	m_s71.EnableWindow(_bFlag);
	m_s72.EnableWindow(_bFlag);
	m_s81.EnableWindow(_bFlag);
	m_s82.EnableWindow(_bFlag);
	m_editMark1.EnableWindow(_bFlag);
	m_editMark2.EnableWindow(_bFlag);
	m_editMark3.EnableWindow(_bFlag);
	m_editMark4.EnableWindow(_bFlag);
	m_editMark5.EnableWindow(_bFlag);
	m_editMark6.EnableWindow(_bFlag);
	m_editMark7.EnableWindow(_bFlag);
	m_editMark8.EnableWindow(_bFlag);
	if (0 == _index)
	{
	}
	else if (1 == _index)
	{
		m_s11.EnableWindow(!_bFlag);
		m_s12.EnableWindow(!_bFlag);
		m_editMark1.EnableWindow(!_bFlag);
	}
	else if (2 == _index)
	{
		m_s21.EnableWindow(!_bFlag);
		m_s22.EnableWindow(!_bFlag);
		m_editMark2.EnableWindow(!_bFlag);
	}
	else if (3 == _index)
	{
		m_s31.EnableWindow(!_bFlag);
		m_s32.EnableWindow(!_bFlag);
		m_editMark3.EnableWindow(!_bFlag);
	}
	else if (4 == _index)
	{
		m_s41.EnableWindow(!_bFlag);
		m_s42.EnableWindow(!_bFlag);
		m_editMark4.EnableWindow(!_bFlag);
	}
	else if (5 == _index)
	{
		m_s51.EnableWindow(!_bFlag);
		m_s52.EnableWindow(!_bFlag);
		m_editMark5.EnableWindow(!_bFlag);
	}
	else if (6 == _index)
	{
		m_s61.EnableWindow(!_bFlag);
		m_s62.EnableWindow(!_bFlag);
		m_editMark6.EnableWindow(!_bFlag);
	}
	else if (7 == _index)
	{
		m_s71.EnableWindow(!_bFlag);
		m_s72.EnableWindow(!_bFlag);
		m_editMark7.EnableWindow(!_bFlag);
	}
	else if (8 == _index)
	{
		m_s81.EnableWindow(!_bFlag);
		m_s82.EnableWindow(!_bFlag);
		m_editMark8.EnableWindow(!_bFlag);
	}

}
vector<CString> CEngineValidationDlg::GetKeyWords()
{
	vector<CString> v_keyWords;
	v_keyWords.clear();
	CString sTemp = _T("");
	int iArrLen = sizeof(ac_KeyWord) / sizeof(ac_KeyWord[0]);
	for (int i = 0; i < iArrLen; i++)
	{
		sTemp = ac_KeyWord[i];
		for (int j = i; j < iArrLen; j++)
		{
			CString sTemp2 = _T("");
			sTemp2 = sTemp;
			sTemp2 += ac_KeyWord[j];
			v_keyWords.push_back(sTemp2);
		}
	}

	return v_keyWords;
}

void CEngineValidationDlg::OnCbnSelchangeComboEnginename()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	int index = m_comboEngineType.GetCurSel();
	if (0 == index)
	{
		staticControlInit(true, index);
	}
	else
	{
		staticControlInit(false, index);
	}

}

//���ֽ�ת���ֽ�
char* CEngineValidationDlg::CStringToMutilChar(CString& str, int& chLength, WORD wPage)
{
	char* pszMultiByte;
	int iSize = WideCharToMultiByte(wPage, 0, str, -1, NULL, 0, NULL, NULL);
	pszMultiByte = (char*)malloc((iSize + 1)/**sizeof(char)*/);
	memset(pszMultiByte, 0, iSize + 1);
	WideCharToMultiByte(wPage, 0, str, -1, pszMultiByte, iSize, NULL, NULL);
	chLength = iSize;
	return pszMultiByte;
}
// Unicode CString URLEncode
BYTE CEngineValidationDlg::toHex(const BYTE &x)
{
	return x > 9 ? x + 55 : x + 48;
}

CString CEngineValidationDlg::UrlEncode(CString sIn)
{

	int ilength = -1;
	char* pUrl = CStringToMutilChar(sIn, ilength, CP_UTF8);
	CStringA strSrc(pUrl);

	CStringA sOut;
	const int nLen = strSrc.GetLength() + 1;

	register LPBYTE pOutTmp = NULL;
	LPBYTE pOutBuf = NULL;
	register LPBYTE pInTmp = NULL;
	LPBYTE pInBuf = (LPBYTE)strSrc.GetBuffer(nLen);
	BYTE b = 0;

	//alloc out buffer
	pOutBuf = (LPBYTE)sOut.GetBuffer(nLen * 3 - 2);//new BYTE [nLen  * 3];

	if (pOutBuf)
	{
		pInTmp = pInBuf;
		pOutTmp = pOutBuf;

		// do encoding
		while (*pInTmp)
		{
			if (isalnum(*pInTmp))
				*pOutTmp++ = *pInTmp;
			else
			if (isspace(*pInTmp))
				*pOutTmp++ = '+';
			else
			{
				*pOutTmp++ = '%';
				*pOutTmp++ = toHex(*pInTmp >> 4);
				*pOutTmp++ = toHex(*pInTmp % 16);
			}
			pInTmp++;
		}
		*pOutTmp = '\0';
		//sOut=pOutBuf;
		//delete [] pOutBuf;
		sOut.ReleaseBuffer();
	}
	strSrc.ReleaseBuffer();
	if (pUrl != NULL)
	{
		delete pUrl;
		pUrl = NULL;
	}
	return CString(sOut);
}

void CEngineValidationDlg::OnBnClickedButtonGo()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	g_EngineInfos.empty();
	g_mapEngineOpenCount.clear();


	CString sText(_T(""));
	m_editMark1.GetWindowTextW(sText);
	g_ValidationText.push_back(sText);
	m_editMark2.GetWindowTextW(sText);
	g_ValidationText.push_back(sText);
	m_editMark3.GetWindowTextW(sText);
	g_ValidationText.push_back(sText);
	m_editMark4.GetWindowTextW(sText);
	g_ValidationText.push_back(sText);
	m_editMark5.GetWindowTextW(sText);
	g_ValidationText.push_back(sText);
	m_editMark6.GetWindowTextW(sText);
	g_ValidationText.push_back(sText);
	m_editMark7.GetWindowTextW(sText);
	g_ValidationText.push_back(sText);
	m_editMark8.GetWindowTextW(sText);
	g_ValidationText.push_back(sText);

	for (int i = 0; i < 8; i++)
	{
		g_bEngineGoing.push_back(TRUE);
	}

	vector<CString> v_keyWords;
	v_keyWords = GetKeyWords();
	int iLen = v_keyWords.size();
	int iSize = sizeof(as_EngineUrl) / sizeof(as_EngineUrl[0]);

	int iEngineIndex = m_comboEngineType.GetCurSel();
	CString skeywordHex;
	CString sUrl;

	if (0 == iEngineIndex)
	{

		for (int i = 0; i < iLen; i++)
		{
			skeywordHex = UrlEncode(v_keyWords[i]);

			for (int j = 0; j < iSize; j++)
			{
				sUrl.Format(as_EngineUrl[j], skeywordHex, 1);

				pEngineInfo engineInfoTemp = new engineInfo();
				engineInfoTemp->iEngineId = j;
				engineInfoTemp->sUrl = sUrl;
				
				g_EngineInfos.push(engineInfoTemp);
				g_mapEngineOpenCount[j] = 0;
			}
		}
	}
	else
	{
		for (int i = 0; i < iLen; i++)
		{
			skeywordHex = UrlEncode(v_keyWords[i]);

			sUrl.Format(as_EngineUrl[iEngineIndex - 1], skeywordHex, 1);

			pEngineInfo engineInfoTemp = new engineInfo();
			engineInfoTemp->iEngineId = iEngineIndex - 1;
			engineInfoTemp->sUrl = sUrl;
			g_mapEngineOpenCount[iEngineIndex - 1] = 0;
			g_EngineInfos.push(engineInfoTemp);
		}
	}

	const int iThreadNum = 3;
	HANDLE *hThreads = new HANDLE[iThreadNum];
	for (int i = 0; i < iThreadNum; i++)
	{
		DWORD dwThreadId;
		hThreads[i] = CreateThread(NULL, NULL, WorkThread, this, 0, &dwThreadId);
	}
	for (int i = 0; i < iThreadNum; i++)
	{
		CloseHandle(hThreads[i]);
	}
	delete[] hThreads;
}
DWORD WINAPI CEngineValidationDlg::ReDrawUIThread(LPVOID _pThis)
{
	CEngineValidationDlg* pThis = (CEngineValidationDlg*)_pThis;
	pThis->ReDrawUI();
	return 0;
}
void CEngineValidationDlg::ReDrawUI()
{
	while (true)
	{
		WaitForSingleObject(g_notifyUiEvent, INFINITE);
		int index = m_comboEngineType.GetCurSel();

		CString str;
		if (0 == index)
		{
			str.Format(_T("%d"), g_mapEngineOpenCount[0]);
			m_s12.SetWindowTextW(str);
			str.Format(_T("%d"), g_mapEngineOpenCount[1]);
			m_s22.SetWindowTextW(str);
			str.Format(_T("%d"), g_mapEngineOpenCount[2]);
			m_s32.SetWindowTextW(str);
			str.Format(_T("%d"), g_mapEngineOpenCount[3]);
			m_s42.SetWindowTextW(str);
			str.Format(_T("%d"), g_mapEngineOpenCount[4]);
			m_s52.SetWindowTextW(str);
			str.Format(_T("%d"), g_mapEngineOpenCount[5]);
			m_s62.SetWindowTextW(str);
			str.Format(_T("%d"), g_mapEngineOpenCount[6]);
			m_s72.SetWindowTextW(str);
			str.Format(_T("%d"), g_mapEngineOpenCount[7]);
			m_s82.SetWindowTextW(str);

		}
		else if (1 == index)
		{
			str.Format(_T("%d"), g_mapEngineOpenCount[0]);
			m_s12.SetWindowTextW(str);
		}
		else if (2 == index)
		{
			str.Format(_T("%d"), g_mapEngineOpenCount[1]);
			m_s22.SetWindowTextW(str);
		}
		else if (3 == index)
		{
			str.Format(_T("%d"), g_mapEngineOpenCount[2]);
			m_s32.SetWindowTextW(str);
		}
		else if (4 == index)
		{
			str.Format(_T("%d"), g_mapEngineOpenCount[3]);
			m_s42.SetWindowTextW(str);
		}
		else if (5 == index)
		{
			str.Format(_T("%d"), g_mapEngineOpenCount[4]);
			m_s52.SetWindowTextW(str);
		}
		else if (6 == index)
		{
			str.Format(_T("%d"), g_mapEngineOpenCount[5]);
			m_s62.SetWindowTextW(str);
		}
		else if (7 == index)
		{
			str.Format(_T("%d"), g_mapEngineOpenCount[6]);
			m_s72.SetWindowTextW(str);
		}
		else if (8 == index)
		{
			str.Format(_T("%d"), g_mapEngineOpenCount[7]);
			m_s82.SetWindowTextW(str);
		}
	}
}
