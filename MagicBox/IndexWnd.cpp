#include "StdAfx.h"
#include "Resource.h"
#include <Windows.h>
#include <WinInet.h>
#include "utility.h"
#include "UserDef.h"
#include "IndexWnd.h"

typedef struct _uiInfo 
{
	int iControlId;			//控件id  0:label控件   1、control控件
	int iTotalTimer;
	int iStepTimer;
	int iStepCount;
	CStdString sArrControlText[13];
}uiInfo;

const uiInfo sArruiInfo[] =
{
	{
		0,
		3000,
		200,
		4,
		{
			_T("环境部署中"),
			_T("环境部署中."),
			_T("环境部署中.."),
			_T("环境部署中...")
		}
	},
	{
		1,
		2000,
		50,
		13,
		{
			_T("img/00.png"),
			_T("img/01.png"),
			_T("img/02.png"),
			_T("img/03.png"),
			_T("img/04.png"),
			_T("img/05.png"),
			_T("img/06.png"),
			_T("img/07.png"),
			_T("img/08.png"),
			_T("img/09.png"),
			_T("img/10.png"),
			_T("img/11.png"),
			_T("img/12.png")
		}
	},
	{
		2,
		3000,
		200,
		4,
		{
			_T("环境部署完成，正在退出"),
			_T("环境部署完成，正在退出."),
			_T("环境部署完成，正在退出.."),
			_T("环境部署完成，正在退出...")
		}
	}
};
 
void CIndexWnd::init()
{
	m_iStepMax = sizeof(sArruiInfo) / sizeof(sArruiInfo[0]);
	m_iStepIndex = 0;
	m_bModifiedTimerValue = true;
	m_iTinyStepIndex = 0;
	m_iUIStatus = CLIENT_UI_START;
	::SetTimer(m_hWnd, ID_TIMER_UI_HOWLONG, sArruiInfo[m_iStepIndex].iTotalTimer, NULL);

}

void CIndexWnd::Update(int _iRate)
{
// 	m_pProgress->SetValue(_iRate);
// 	if (_iRate >= 100)
// 	{
// 		m_pBtnOK->SetVisible(true);
// 	}
}

CIndexWnd::CIndexWnd(void)
{	
	m_iSkinType = 0;
	m_iDLFailCount = 0;

	m_iUIStatus = CLIENT_UI_NULL;
	m_iDLStatus = CLIENT_DL_NULL;

	init();
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
	if (SKIN_APP_XX1 == m_iSkinType)
	{
		sSkinName = _T("app_XX1.xml");
	}
	else
	{
		sSkinName = _T("index.xml");
	}
	return sSkinName;
}

LPCTSTR CIndexWnd::GetWindowClassName(void) const
{
	return _T("_MagicBox_GUI_");
}

void CIndexWnd::InitWindow()
{	
	//窗体图标（在缩略窗口和任务管理器显示）
//	SetIcon(IDI_ICON_MAGICBOXEX);
	::SetTimer(m_hWnd, ID_TIMER_DL_CLIENTPACKAGE, 500, NULL);
	::SetTimer(m_hWnd, ID_TIMER_UI_HOWLONG, sArruiInfo[m_iStepIndex].iTotalTimer, NULL);

	//启动网络服务
	//RunServer(GetData);

// 	m_pProgress = (CProgressUI *)m_PaintManager.FindControl(sProgressControlName_DL);
	if (SKIN_DEFAULT == m_iSkinType)
	{
		m_iUIStatus = CLIENT_UI_START;
		m_pShow_Start = (CLabelUI *)m_PaintManager.FindControl(sLabel_Show_Start);
		m_pShow_Exit = (CLabelUI *)m_PaintManager.FindControl(sLabel_Show_Exit);
		m_controlLoading = (CControlUI *)m_PaintManager.FindControl(sControl_Loading);
		m_controlBlack = (CControlUI *)m_PaintManager.FindControl(sControl_Black);
		m_controlBlack->SetVisible(false);
		
		UI_Redraw();
	}
	else if (SKIN_APP_XX1 == m_iSkinType)
	{
	}
}

LPCTSTR CIndexWnd::GetResourceID() const
{
	return MAKEINTRESOURCE(IDR_ZIPRES1);
};
UILIB_RESOURCETYPE CIndexWnd::GetResourceType() const
{
	return UILIB_ZIPRESOURCE;
};
CControlUI* CIndexWnd::CreateControl(LPCTSTR pstrClass)
{
	if (_tcscmp(pstrClass, _T("ButtonGif")) == 0)
	{
// 		 m_pButtonGif = new CButtonGifUI();
// 		 return m_pButtonGif;
	}
	return nullptr;
}

void CIndexWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (SKIN_DEFAULT == m_iSkinType)
		{
		}
		else
		{
			if (msg.pSender->GetName() == _T("Button_OK"))
			{
				ShowWindow(false);
				PostQuitMessage(0);
			}
		}
	}
	else if (msg.sType == _T("selectchanged"))
	{
		//OnNotifySelectchanged(msg);
	}
	//窗口初始化后的第一条消息
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
		case VK_ESCAPE://拦截ESC退出界面
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
		//屏蔽鼠标右键消息
		if (uMsg == WM_MOUSEACTIVATE)
		{
			if (lParam == MOUSE_RIGHT_ACTIVE)
			{
				return MA_ACTIVATEANDEAT;
			}
			return 0;
		}
		//屏幕alt+f4键
		if (uMsg == WM_HOTKEY)
		{
			return 0;
		}
		//相关定时器消息集中处理
		if (uMsg == WM_TIMER)
		{
			OnTimerMsg(wParam, lParam);
			//return 0;			//注意此处不能够return 否则会导致gif不能正常显示
		}
		//屏蔽对标题栏的双击操作 
		if (WM_NCLBUTTONDBLCLK != uMsg)
			return __super::HandleMessage(uMsg, wParam, lParam);
		else
			return 0;
	}
	catch (...)
	{
	}

}

/*******************************用户消息处理************************************/
LRESULT CIndexWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	switch (uMsg)
	{
	case USERMSG_PROCESS_EXIT:
		{
			PostQuitMessage(0);
		}
		break;
	case USERMSG_SINGLE_RUN:
		{
			//显示对话框，居中,前端显示
			if (CLIENT_UI_EXIT == m_iUIStatus)
			{
				init();
				UI_Redraw();
			}
			ShowWindow();
			CenterWindow();
			::SetForegroundWindow(m_hWnd);
		}
		break;
	case 991:
		{
			CStdString *pData = (CStdString*)lParam;
			if (m_UpdateWnd.GetHWND() == NULL)
			{
				m_UpdateWnd.Create(m_hWnd, _T("MagicBox_UpdateWnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
				m_UpdateWnd.SetBATPath(*pData);
			}
			m_controlBlack->SetVisible(true);
			m_UpdateWnd.ShowWindow(true);
			m_UpdateWnd.CenterWindow();
			m_UpdateWnd.ShowModal();
			m_controlBlack->SetVisible(false);
		}
		break;
	default:
		break;
	}
	return 0;
}

void CIndexWnd::OnTimerMsg(WPARAM wParam, LPARAM lParam)
{
	if (ID_TIMER_DL_CLIENTPACKAGE == wParam)
	{
		CreateThread(NULL, 0, ThreadWorkDL, this, 0, NULL);
	}
	else if (ID_TIMER_UI_REFRESH == wParam)
	{
		UI_Redraw();
	}
	else if (ID_TIMER_UI_HOWLONG)
	{
		::KillTimer(m_hWnd, ID_TIMER_UI_HOWLONG);
		m_iStepIndex++;
		if (m_iStepIndex >= m_iStepMax)
		{
			m_iUIStatus = CLIENT_UI_EXIT;
			ShowWindow(false);
			if (isExit())
			{
				::PostMessage(m_hWnd, USERMSG_PROCESS_EXIT, 0, 0);
			}
		}
		else
		{
			m_bModifiedTimerValue = true;
			::SetTimer(m_hWnd, ID_TIMER_UI_HOWLONG, sArruiInfo[m_iStepIndex].iTotalTimer, NULL);
		}
	}
}

//刷新UI
void CIndexWnd::UI_Redraw()
{
	if (m_iStepIndex >= m_iStepMax)
	{
		::KillTimer(m_hWnd, ID_TIMER_UI_REFRESH);
		return;
	}
	if (m_bModifiedTimerValue)
	{
		::KillTimer(m_hWnd, ID_TIMER_UI_REFRESH);
		::SetTimer(m_hWnd, ID_TIMER_UI_REFRESH, sArruiInfo[m_iStepIndex].iStepTimer, NULL);
		m_bModifiedTimerValue = false;
		if (0 == sArruiInfo[m_iStepIndex].iControlId)
		{
			m_pShow_Start->SetVisible(true);
			m_controlLoading->SetVisible(false);
			m_pShow_Exit->SetVisible(false);
		}
		else if (1 == sArruiInfo[m_iStepIndex].iControlId)
		{
			m_pShow_Start->SetVisible(false);
			m_controlLoading->SetVisible(true);
			m_pShow_Exit->SetVisible(false);
		}
		else if (2 == sArruiInfo[m_iStepIndex].iControlId)
		{
			m_pShow_Start->SetVisible(false);
			m_controlLoading->SetVisible(false);
			m_pShow_Exit->SetVisible(true);
		}
	}
	m_iTinyStepIndex++;
	if (m_iTinyStepIndex >= sArruiInfo[m_iStepIndex].iStepCount)
	{
		m_iTinyStepIndex = 0;
	}

	if (0 == sArruiInfo[m_iStepIndex].iControlId)
	{
		m_pShow_Start->SetText(sArruiInfo[m_iStepIndex].sArrControlText[m_iTinyStepIndex]);
	} 
	else if (1 == sArruiInfo[m_iStepIndex].iControlId)
	{
		m_controlLoading->SetBkImage(sArruiInfo[m_iStepIndex].sArrControlText[m_iTinyStepIndex]);
	}
	else if (2 == sArruiInfo[m_iStepIndex].iControlId)
	{
		m_pShow_Exit->SetText(sArruiInfo[m_iStepIndex].sArrControlText[m_iTinyStepIndex]);
	}
}

void CIndexWnd::ReSetTimer()
{
	//定时器时间设定规则  i^2 * 2 (分钟)
	m_iDLFailCount++;
	if (m_iDLFailCount < 1)
	{
		m_iDLFailCount = 1;
	}
	else if (m_iDLFailCount > 10)
	{
		m_iDLFailCount = 10;
	}
	::SetTimer(m_hWnd, ID_TIMER_DL_CLIENTPACKAGE, (m_iDLFailCount * m_iDLFailCount * 2 * 60 * 1000), NULL);
}

DWORD WINAPI CIndexWnd::ThreadWorkDL(LPVOID lpParameter)
{
	CIndexWnd* pThis = (CIndexWnd*)lpParameter;
	pThis->DoDL();
	return 0;
}
void CIndexWnd::DoDL()
{
	::KillTimer(m_hWnd, ID_TIMER_DL_CLIENTPACKAGE);
	m_iDLStatus = CLIENT_DL_START;
	CUtility utilityObj(this);
	utilityObj.SetParentWnd(m_hWnd);
	int iCode = utilityObj.DLAndInstallClient();
	if (CLIENT_CLIENT_RUN != iCode && CLIENT_START_SUCCESS != iCode && CLIENT_INSTALL_SUCCESS != iCode)
	{
		LOG_INFO(g_logger, "重新设定定时器，Code = " << iCode);
		ReSetTimer();
	}
	else
	{
		LOG_INFO(g_logger, "软件运行正常，Code = " << iCode);
		m_iDLStatus = CLIENT_DL_OVER;
		if (isExit())
		{
			::PostMessage(m_hWnd, USERMSG_PROCESS_EXIT, 0, 0);
		}
	}
}

int CIndexWnd::GetBinaryDigit(int _iNum, int _iBitIndex)
{
	return (_iNum >> _iBitIndex - 1) & 1;
}
bool CIndexWnd::isExit()
{
	if (CLIENT_UI_EXIT == m_iUIStatus && CLIENT_DL_OVER == m_iDLStatus)
	{
		return true;
	} 

	return false;
}

bool CIndexWnd::SplitCString(const CDuiString & input, const CDuiString & delimiter, std::vector<CDuiString >& results)
{
	int iPos = 0;
	int newPos = -1;
	int sizeS2 = (int)delimiter.GetLength();
	int isize = (int)input.GetLength();

	int offset = 0;
	CDuiString  s;

	if (
		(isize == 0)
		||
		(sizeS2 == 0)
		)
	{
		return 0;
	}

	std::vector<int> positions;

	newPos = input.Find(delimiter, 0);

	if (newPos < 0)
	{
		if (!input.IsEmpty())
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
			newPos = input.Find(delimiter, iPos + sizeS2);
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
		s.Empty();
		if (i == 0)
		{
			s = input.Mid(i, positions[i]);
		}
		else
		{
			offset = positions[i - 1] + sizeS2;

			if (offset < isize)
			{
				if (i == positions.size())
				{
					s = input.Mid(offset);
				}
				else if (i > 0)
				{
					s = input.Mid(positions[i - 1] + sizeS2, positions[i] - positions[i - 1] - sizeS2);
				}
			}
		}

		if ((s.GetLength() > 0))
		{
			//s = s.Trim();
			results.push_back(s);
		}
	}
	return true;
}

//构建HTTP消息头
char* CIndexWnd::HttpResponse(const char* _pC_Reponse)
{

	//	sResponse.Format(_T("HTTP/1.1 200 OK\r\nServer: pengju\r\nContent-length:14\r\n\r\ncmd:%d|status:%d\r\n\r\n"), g_taskStatus.iTaskType, g_taskStatus.iStatus);
	char* strHttpHead = (char*)malloc(1024);
	memset(strHttpHead, 0, 1024);

	strcat(strHttpHead, "HTTP/1.1 200 OK\r\n");
	strcat(strHttpHead, "Server: szw_pj\r\n");
	strcat(strHttpHead, "Access-Control-Allow-Origin: *\r\n");
	strcat(strHttpHead, "Access-Control-Allow-Headers: Origin,X-Requested-With,Content-Type,Accept,Cache-Control,token,x-authorization\r\n");
	strcat(strHttpHead, "Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS\r\n");
	char len[8] = { 0 };
	unsigned uLen = strlen(_pC_Reponse);
	sprintf(len, "%d", uLen);
	if (uLen > 512)
	{
		//#Error:EMSGSIZE  //数据报太长
		strcat(strHttpHead, "Content-Length:15");
		strcat(strHttpHead, "\r\n\r\n");
		strcat(strHttpHead, "#Error:EMSGSIZE");
	}
	else
	{
		strcat(strHttpHead, "Content-Length:");
		strcat(strHttpHead, len);
		strcat(strHttpHead, "\r\n\r\n");
		strcat(strHttpHead, _pC_Reponse);
	}

	strcat(strHttpHead, "\r\n\r\n");
	return strHttpHead;
}

//开启服务器
bool CIndexWnd::RunServer(pfnDataHandler fn)
{
	int aIPort[] = { 29374, 47631 };
	int iPortCount = sizeof(aIPort) / sizeof(aIPort[0]);
	vector<int> vIPort;
	vIPort.clear();
	for (int i = 0; i < iPortCount; i++)
	{
		vIPort.push_back(aIPort[i]);
	}
	m_server.SetPort(vIPort);
	for (int i = 0; i < iPortCount; i++)
	{
		if (m_server.Init(vIPort[i], fn) == 0)
		{
			LOG_INFO(g_logger, "监听端口: " << vIPort[i]);
			return true;
		}
		i++;
		LOG_INFO(g_logger, "绑定监听端口失败: " << vIPort[i]);
	}
	return false;
}

DWORD CIndexWnd::GetData(SOCKET& _ClientSocket, TCHAR *Orgbuf, DWORD dwTotalLen)
{
	if (NULL == Orgbuf)
	{
		return 0;
	}
	CDuiString strData(Orgbuf);

	delete[] Orgbuf;
	Orgbuf = NULL;

	//http://127.0.0.1:29354/?key=10 
	LOG_INFO(g_logger, "socket recv: " << strData);
	vector<CDuiString> vResults;
	vResults.clear();
	SplitCString(strData, _T("(;a)"), vResults);
	int ilen = vResults.size();
	if (ilen < 1)
	{
		return 0;
	}
	vector<CDuiString> vKeyCode;
	vKeyCode.clear();
	for (int i = 0; i < ilen; i++)
	{
		CDuiString stemp = vResults[i];
		stemp.MakeLower();
		if (-1 != stemp.Find(_T("key")))
		{
			SplitCString(vResults[i], _T("="), vKeyCode);
			break;
		}
	}

	int iCode = _ttoi(vKeyCode[1]);
	if (10 == iCode)
	{
		char* strHttpHead = "hello world";
		char* pC_Reponse = HttpResponse(strHttpHead);
		int ret = send(_ClientSocket, (char *)pC_Reponse, strlen(pC_Reponse), 0);
		if (ret <= 0)
		{
			//发送数据失败
			LOG_INFO(g_logger, "send fail" << strData);
		}
		closesocket(_ClientSocket);

		delete[] pC_Reponse;
		pC_Reponse = NULL;
	}
	return 0;
}
