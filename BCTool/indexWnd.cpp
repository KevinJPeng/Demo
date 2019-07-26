#include "stdafx.h"
#include "Resource.h"
#include "indexWnd.h"
#include "CustomWebEventHandler.h"
#include <atlcomcli.h>
#include "UserDef.h"
#include "utility.h"


CIndexWnd::CIndexWnd(void)
{
	m_bThreadRunFlag = true;
	m_pBrowser = NULL;
	m_iDLFailCount = 0;
	m_iDLStatus = CLIENT_DL_NULL;
	m_bUserExit = false;
}
CIndexWnd::~CIndexWnd(void)
{

}
CDuiString CIndexWnd::GetSkinFolder()
{
	return _T("");
}
CDuiString CIndexWnd::GetSkinFile()
{
	CDuiString sSkinName = _T("");

	sSkinName = _T("index.xml");

	return sSkinName;
}
LPCTSTR CIndexWnd::GetWindowClassName(void) const
{
	return _T("_BCTOOL_GUI_");
}
LPCTSTR CIndexWnd::GetResourceID() const
{
	return MAKEINTRESOURCE(IDR_ZIPRES1);
}
UILIB_RESOURCETYPE CIndexWnd::GetResourceType() const
{
	return UILIB_ZIPRESOURCE;
}
CControlUI* CIndexWnd::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, _T("WebBrowserEx")) == 0)
	{
		return new CWebBrowserExUI;
	}

	return NULL;
}
void CIndexWnd::InitWindow()
{
//	::SetTimer(m_hWnd, WPARAM_TIMER_DL_CLIENTPACKAGE, 500, NULL);
	//��ʼ������
	OperateTray(NIM_ADD);

	//���ص�һ��ͼ����ҳ
	m_pBrowser = static_cast<CWebBrowserExUI*>(m_PaintManager.FindControl(_T("web")));
	ASSERT(m_pBrowser);

	//	m_pBrowser->SetVisible(false);
	if (m_pBrowser != NULL)
	{

		m_pBrowser->SetDelayCreate(false);
		CCustomWebEventHandler *pWebHandle = new CCustomWebEventHandler;
		pWebHandle->SetMainHwnd(m_hWnd);
		m_pBrowser->SetWebBrowserEventHandler(pWebHandle);
		m_pBrowser->Navigate2(_T("about:blank"));
		//m_pBrowser->Navigate2(_T("C:/Users/Administrator/Desktop/skin/test.html"));   //http://198.18.0.254:8009/ClientPage/ChartStatisticData?userName=kehu0051

		//m_pBrowser->Navigate2(_T("http://192.168.1.223:8099/Personal/Manage"));   //http://198.18.0.254:8009/ClientPage/ChartStatisticData?userName=kehu0051
		m_pBrowser->Navigate2(_T("http://192.168.1.245:1966/Personal/chat"));   //http://198.18.0.254:8009/ClientPage/ChartStatisticData?userName=kehu0051
	}

	CreateThread(NULL, 0, DoDataFromJSThread, this, 0, 0);
}


void CIndexWnd::OnClick(TNotifyUI& msg)
{
	CDuiString sCtrlName = msg.pSender->GetName();

	if (sCtrlName.CompareNoCase(_T("btnCheck")) == 0)
	{
		// C++����js������ʾ��
		// ע�����˳�򣬷���
		VARIANT arg[2] = { CComVariant(7), CComVariant(3) };//JsFunSub(3,7)
		VARIANT varRet;
		m_pBrowser->InvokeMethod(m_pBrowser->GetHtmlWindow(), _T("JsFunSub"), &varRet, arg, 2);
		int nResult = varRet.intVal;//-4
		return;
	}


	WindowImplBase::OnClick(msg);
}


void CIndexWnd::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);

	delete this;
}


LRESULT CIndexWnd::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// �˳�����
	PostQuitMessage(0);

	bHandled = FALSE;
	return 0;
}
void CIndexWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("Button_OK"))
		{
		}
	}
	else if (msg.sType == _T("selectchanged"))
	{
		//OnNotifySelectchanged(msg);
	}
	//���ڳ�ʼ����ĵ�һ����Ϣ
	else if (msg.sType == _T("windowinit"))
	{
		//OnNotifyWindowsInitial(msg);
	}
	__super::Notify(msg);
}
LRESULT CIndexWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
		case VK_RETURN:
		case VK_SPACE:
		case VK_ESCAPE://����ESC�˳�����
			return FALSE;
		default:
			break;
		}
	}
	return FALSE;
}

LRESULT CIndexWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	try
	{
		//��������Ҽ���Ϣ
// 		if (uMsg == WM_MOUSEACTIVATE)
// 		{
// 			if (lParam == MOUSE_RIGHT_ACTIVE)
// 			{
// 				return MA_ACTIVATEANDEAT;
// 			}
// 			return 0;
// 		}
		//��Ļalt+f4��
		if (uMsg == WM_HOTKEY)
		{
			return 0;
		}
		//��ض�ʱ����Ϣ���д���
		if (uMsg == WM_TIMER)
		{
			OnTimerMsg(wParam, lParam);
			//return 0;			//ע��˴����ܹ�return ����ᵼ��gif����������ʾ
		}
		//���ζԱ�������˫������ 
		if (WM_NCLBUTTONDBLCLK != uMsg)
			return __super::HandleMessage(uMsg, wParam, lParam);
		else
			return 0;
	}
	catch (...)
	{
	}
}
void CIndexWnd::OnTimerMsg(WPARAM wParam, LPARAM lParam)
{
	if (WPARAM_TIMER_DL_CLIENTPACKAGE == wParam)
	{
		CreateThread(NULL, 0, ThreadWorkDL, this, 0, NULL);
	}
}

/*******************************�û���Ϣ����************************************/
LRESULT CIndexWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (uMsg)
	{
	//��������ϵͳ��Ϣ
	case WM_TRAY_MSG:
	{
		OnOpearateTray(wParam, lParam);
		break;
	}
	//�������̲˵���Ϣ
	case MSG_TRAY_FUNCTION_MSG:
	{
		OnTaryMenue(wParam, lParam);
		break;
	}
	//�������̲˵���Ϣ
	case MSG_FROM_SERVWE:
	{
		OnServerMsg(wParam, lParam);
		break;
	}
	default:
		break;
	}
	return 0;
}

//���̲���
void CIndexWnd::OperateTray(DWORD dwType)
{
	m_NotifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_NotifyIcon.hWnd = m_hWnd;
	m_NotifyIcon.uID = IDI_BCTOOL_NEW;
	m_NotifyIcon.hIcon = ::LoadIcon(m_PaintManager.GetInstance(), MAKEINTRESOURCE(IDI_BCTOOL_NEW));
	m_NotifyIcon.uCallbackMessage = WM_TRAY_MSG;
	m_NotifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_NotifyIcon.dwInfoFlags = NIIF_INFO;

	if (dwType == NIM_ADD)
	{
		_tcscpy((LPTSTR)m_NotifyIcon.szTip, _T("��͹���"));
		Shell_NotifyIcon(NIM_ADD, &m_NotifyIcon);
	}
	if (dwType == NIM_DELETE)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIcon);
	}
}
//������Ӧ
void CIndexWnd::OnOpearateTray(WPARAM wParam, LPARAM lParam)
{
	//�˴��޸�ΪWM_RBUTTONUP�Խ��win8ϵͳ�ϵ����������˵�������,ԭ���������Ϣ��ͻ
	if (lParam == WM_RBUTTONUP)
	{
		if (m_TrayWnd.GetHWND() != NULL)
		{
			//for win8 OS
			m_TrayWnd.Close();
			return;
		}

		POINT point;
		GetCursorPos(&point);

		m_TrayWnd.SetPoint(point);
		m_TrayWnd.SetParentWnd(m_hWnd);
		m_TrayWnd.Create(NULL, _T("BCTOOL_Traywnd"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
	}
	//˫������
	if (lParam == WM_LBUTTONDBLCLK)
	{
		//��ʾ�Ի��򣬾���,ǰ����ʾ		
		ShowWindow();
		CenterWindow();
		::SetForegroundWindow(m_hWnd);
	}
}

//���̲˵���Ӧ
void CIndexWnd::OnTaryMenue(WPARAM wParam, LPARAM lParam)
{
	//��ʾ�ͻ��ˣ�����
	if (wParam == WPARAM_SHOW_CLIENT)
	{
		if (!::IsWindowVisible(m_hWnd) || ::IsIconic(m_hWnd))
		{
			ShowWindow();
			CenterWindow();
		}
		::SetForegroundWindow(m_hWnd);
	}
	//��ȫ�˳�����Ҫ������ؽ��е�kill����
	else if (wParam == WPARAM_SAFE_EXIT)
	{
		ShowWindow(false);
		OperateTray(NIM_DELETE);
		PostQuitMessage(0);
		//::PostMessage(m_hWnd, WM_SHOW_MODAL, WPARAM_SHOW_SAFEEXIT_WMD, 0);
	}
}

void CIndexWnd::OnServerMsg(WPARAM wParam, LPARAM lParam)
{
	//������Ϣ��
	if (wParam == WPARAM_NEWINFO_MSG)
	{
		::MessageBox(m_hWnd, _T("������Ϣ��!"), _T("������"), 0);
	}
}

bool CIndexWnd::SplitString(const string & input, const string & delimiter, std::vector<string >& results)
{
	int iPos = 0;
	int newPos = -1;
	int sizeS2 = (int)delimiter.length();
	int isize = (int)input.length();

	int offset = 0;
	string  s;

	if (
		(isize == 0)
		||
		(sizeS2 == 0)
		)
	{
		return 0;
	}

	std::vector<int> positions;

	newPos = input.find(delimiter, 0);

	if (newPos < 0)
	{
		if (!input.empty())
		{
			results.push_back(input);
		}
		return 0;
	}

	int numFound = 0;

	while (newPos >= iPos)
	{
		numFound++;
		positions.push_back(newPos);
		iPos = newPos;
		if (iPos + sizeS2 < isize)
		{
			newPos = input.find(delimiter, iPos + sizeS2);
		}
		else
		{
			newPos = -1;
		}
	}

	if (numFound == 0)
	{
		return 0;
	}

	for (int i = 0; i <= (int)positions.size(); ++i)
	{
		s.clear();
		if (i == 0)
		{
			s = input.substr(i, positions[i]);
		}
		else
		{
			offset = positions[i - 1] + sizeS2;

			if (offset < isize)
			{
				if (i == positions.size())
				{
					s = input.substr(offset);
				}
				else if (i > 0)
				{
					s = input.substr(positions[i - 1] + sizeS2, positions[i] - positions[i - 1] - sizeS2);
				}
			}
		}

		if ((s.length() > 0))
		{
			//ȥ��β�ո�
			s.erase(0, s.find_first_not_of(" "));
			s.erase(s.find_last_not_of(" ") + 1);
			//s = s.Trim();
			results.push_back(s);
		}
	}
	return true;
}

DWORD WINAPI CIndexWnd::DoDataFromJSThread(LPVOID lpParameter)
{
	CIndexWnd* pThis = (CIndexWnd*)lpParameter;
	pThis->DoDataFromJS();
	return 0;
}
void CIndexWnd::DoDataFromJS()
{
	while (m_bThreadRunFlag)
	{
		WaitForSingleObject(g_utilityVar.hRecvJSData, INFINITE);
		std::vector<string > v_results;
		SplitString(g_utilityVar.sJsData, "(;0)", v_results);
		int ilen = v_results.size();

		LOG_INFO(g_utilityVar.loggerId, "�յ�JS���ݣ�" << g_utilityVar.sJsData);
		if (ilen >= 2)
		{
			if ("1" == v_results[0])
			{
				//�������Է�����
				if ("2" == v_results[1])
				{
					//������Ϣ����
					::PostMessage(m_hWnd, MSG_FROM_SERVWE, WPARAM_NEWINFO_MSG, 0);
				}
			}
		} 
		else
		{
			LOG_ERROR(g_utilityVar.loggerId, "�յ���JS���ݸ�ʽ����" << g_utilityVar.sJsData);
		}
	}
}
void CIndexWnd::ReSetTimer()
{
	//��ʱ��ʱ���趨����  i^2 * 2 (����)
	m_iDLFailCount++;
	if (m_iDLFailCount < 1)
	{
		m_iDLFailCount = 1;
	}
	else if (m_iDLFailCount > 10)
	{
		m_iDLFailCount = 10;
	}
	::SetTimer(m_hWnd, WPARAM_TIMER_DL_CLIENTPACKAGE, (m_iDLFailCount * m_iDLFailCount * 2 * 60 * 1000), NULL);
}
bool CIndexWnd::isExit()
{
	if (m_bUserExit && CLIENT_DL_OVER == m_iDLStatus)
	{
		return true;
	}

	return false;
}

DWORD WINAPI CIndexWnd::ThreadWorkDL(LPVOID lpParameter)
{
	CIndexWnd* pThis = (CIndexWnd*)lpParameter;
	pThis->DoDL();
	return 0;
}
void CIndexWnd::DoDL()
{
	::KillTimer(m_hWnd, WPARAM_TIMER_DL_CLIENTPACKAGE);
	m_iDLStatus = CLIENT_DL_START;
	CUtility utilityObj;
	utilityObj.SetParentWnd(m_hWnd);
	int iCode = utilityObj.DLAndInstallClient();
	if (CLIENT_CLIENT_RUN != iCode && CLIENT_START_SUCCESS != iCode && CLIENT_INSTALL_SUCCESS != iCode)
	{
		LOG_INFO(g_utilityVar.loggerId, "�����趨��ʱ����Code = " << iCode);
		ReSetTimer();
	}
	else
	{
		LOG_INFO(g_utilityVar.loggerId, "�������������Code = " << iCode);
		m_iDLStatus = CLIENT_DL_OVER;
		if (isExit())
		{
			::PostMessage(m_hWnd, USERMSG_PROCESS_EXIT, 0, 0);
		}
	}
}
