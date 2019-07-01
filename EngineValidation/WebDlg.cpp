// WebDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EngineValidation.h"
#include "WebDlg.h"
#include "afxdialogex.h"

#include <atlbase.h>
#include <mshtml.h>   
#include <afxinet.h>
#include <afxhtml.h>
#import <mshtml.tlb>


#define  TIMEOUTCOUTS   8      //��ʱʱ��

#define   GETSEARCHDATA     WM_USER + 0x01
#define   GETREDIRECTURL    WM_USER + 0x02

// CWebDlg �Ի���

IMPLEMENT_DYNAMIC(CWebDlg, CDialogEx)

CWebDlg::CWebDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWebDlg::IDD, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif
	m_ValidationConfirm = ::CreateEvent(NULL, false, false, NULL);
}

CWebDlg::~CWebDlg()
{
}

void CWebDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPLORER2, m_webBrowser);
	DDX_Control(pDX, IDC_BUTTON_YES, m_btnOK);
	DDX_Control(pDX, IDC_BUTTON_NO, m_btnCancle);
}


BEGIN_MESSAGE_MAP(CWebDlg, CDialogEx)
 	ON_MESSAGE(GETSEARCHDATA, &GetSearchData)
 	ON_MESSAGE(GETREDIRECTURL, &RedirectURL)
	ON_BN_CLICKED(IDOK, &CWebDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_YES, &CWebDlg::OnBnClickedButtonYes)
	ON_BN_CLICKED(IDC_BUTTON_NO, &CWebDlg::OnBnClickedButtonNo)
END_MESSAGE_MAP()


// CWebDlg ��Ϣ�������


BOOL CWebDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	m_webBrowser.put_Silent(TRUE);
	//m_webBrowser.Navigate(_T("www.baidu.com"), NULL, NULL, NULL, NULL);
	CreateThread(NULL, NULL, OpenSearchEngineThread, this, 0, NULL);

	m_btnOK.EnableWindow(FALSE);
	m_btnCancle.EnableWindow(FALSE);


	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣:  OCX ����ҳӦ���� FALSE
}
DWORD WINAPI CWebDlg::OpenSearchEngineThread(LPVOID lp)
{
	CWebDlg *pThis = (CWebDlg*)lp;
	pThis->OpenSearchEngine();
	return 0;
}


#if 1


LRESULT  CWebDlg::RedirectURL(WPARAM wParam, LPARAM lParam)
{
	BOOL bBusy;
	long lReadyState;
	pEngineInfo p = (pEngineInfo)lParam;

	try
	{
		if (wParam == 0)
		{
				VARIANT vFlags;

				VariantInit(&vFlags);
				vFlags.vt = VT_I4;
				vFlags.lVal = navNoHistory | navNoReadFromCache | navNoWriteToCache;
				m_webBrowser.Navigate(p->sUrl, &vFlags, NULL, NULL, NULL);
				return 1;
		}
		else if (wParam == 1)
		{
			bBusy = m_webBrowser.get_Busy();
			lReadyState = m_webBrowser.get_ReadyState();

			if (!bBusy && lReadyState >= READYSTATE_COMPLETE)
			{
				return 1;
			}
		}
		else if (wParam == 2)
		{
			m_webBrowser.Stop();
		}
	}
	catch (...)
	{
		return 1;  //�����쳣�����
	}

	return 0;
}

LRESULT  CWebDlg::GetSearchData(WPARAM wParam, LPARAM lParam)
{
	pEngineInfo p = (pEngineInfo)lParam;
	HRESULT hret;
	int iPaiMingResult = 0;
	CComPtr<IHTMLDocument2> pDocument2;
	CComPtr<IHTMLElement> pElement;

	CComPtr<IDispatch> pDispath;
	CComPtr<IDispatch> pDispathTemp;

	CComQIPtr<IHTMLElementCollection> pAllColls;
	CComQIPtr<IHTMLElementCollection> pTags;

	g_mapEngineOpenCount[p->iEngineId]++;
	::SetEvent(g_notifyUiEvent);
	try
	{
		do
		{
			pDispath = m_webBrowser.get_Document();

			if (pDispath == NULL)
			{
				break;
			}

			hret = pDispath->QueryInterface(IID_IHTMLDocument2, (void**)&pDocument2);
			if (FAILED(hret))
			{
				break;
			}

			hret = pDocument2->get_body(&pElement);
			if (FAILED(hret))
			{
				break;
			}

			CComBSTR bstr;
			CString strPage;
			pElement->get_outerHTML(&bstr);
			strPage = bstr;
			if (-1 != strPage.Find(g_ValidationText[p->iEngineId]))
			{
				m_btnOK.EnableWindow(TRUE);
				m_btnCancle.EnableWindow(TRUE);

				return 1;
			}

		} while (0);
	}
	catch (...)
	{
	}

	return 0;
}
pEngineInfo CWebDlg::GetEngineInfo()
{
	pEngineInfo pSData = NULL;
	CLocalLock local(&g_critSection);

	while (1)
	{
		if (g_EngineInfos.size() > 0)
		{
			pSData = (pEngineInfo)g_EngineInfos.top();
			g_EngineInfos.pop();
			if (g_bEngineGoing[pSData->iEngineId])
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	return pSData;
}

DWORD CWebDlg::OpenSearchEngine()
{
	bool bReady = false;
	int iTimeleft;
//	pEngineInfo pBrowserInfoTemp;
//	engineInfo sBrowserInfoTemp;
	while (true)
	{
		Sleep(200);

		long lMessageBack = -1;
		pEngineInfo pDataInfo;
		pDataInfo = GetEngineInfo();
		if (NULL == pDataInfo)
		{
			break;
		}
//		if (g_EngineInfos.size() <= 0)
//		{
//			break;
//		}
//		pBrowserInfoTemp = new engineInfo();
//		sBrowserInfoTemp = g_EngineInfos.top();
//		g_EngineInfos.pop();
//		pBrowserInfoTemp->iEngineId = sBrowserInfoTemp.iEngineId;
//		pBrowserInfoTemp->sUrl = sBrowserInfoTemp.sUrl;

		for (int i = 0; i < 3; i++)  //���ٱȽ����򲻿���ҳ���������
		{
			iTimeleft = 0;
			int iSend = -2;

			iSend = (BOOL)SendMessage(GETREDIRECTURL, 0, (LPARAM)pDataInfo);

			//g_pageLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("׼��ץȡ��ҳ����bbb...%d"), iSend);
			//������Ҫͬ������Ϣ�����߳�ȥ��������ֹ��ͬ�̲߳������ֱ�������,��Ҫ�����Ч��
			if (iSend == 1)
			{
				do
				{
					//g_pageLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("׼��ץȡ��ҳ����ccc..."));
					bReady = (BOOL)SendMessage(GETREDIRECTURL, 1, (LPARAM)pDataInfo);

					if (bReady || (iTimeleft > TIMEOUTCOUTS))
					{
						if (iTimeleft > TIMEOUTCOUTS)
						{
							SendMessage(GETREDIRECTURL, 2, (LPARAM)pDataInfo);
							break;
						}

						//g_pageLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("׼��ץȡ��ҳ���ݣ��ؼ���:%s  ������־Ϊ��%d"), sData.strKeyWordName, sData.iFlag);
						lMessageBack = SendMessage(GETSEARCHDATA, 0, (LPARAM)pDataInfo);
						//g_pageLog.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("����ץȡ��ҳ���ݣ��ؼ���:%s  ������־Ϊ��%d"), sData.strKeyWordName, sData.iFlag);
						break;
					}

					iTimeleft++;
					Sleep(500);

				} while (1);
			}

			if (1 == lMessageBack)
			{
				//����ֵΪ1��ʾ������֤��
				m_iEngineId = pDataInfo->iEngineId;
				WaitForSingleObject(m_ValidationConfirm, INFINITE);
			}

			if (-1 != lMessageBack)
			{
				break;
			}
		}

		if (pDataInfo != NULL)
		{
			delete pDataInfo;
			pDataInfo = NULL;
		}
	}
	OnBnClickedOk();
	return 0;
}
#endif

void CWebDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnOK();
}


void CWebDlg::OnBnClickedButtonYes()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	::SetEvent(m_ValidationConfirm);
	g_bEngineGoing[m_iEngineId] = FALSE;
	m_btnOK.EnableWindow(false);
	m_btnCancle.EnableWindow(false);

}


void CWebDlg::OnBnClickedButtonNo()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	::SetEvent(m_ValidationConfirm);
	m_btnOK.EnableWindow(false);
	m_btnCancle.EnableWindow(false);
}
