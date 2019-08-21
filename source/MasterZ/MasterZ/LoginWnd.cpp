#include "StdAfx.h"
#include "LoginWnd.h"

const TCHAR* const kEditUserName = _T("text_user");
const TCHAR* const kEditPassWord = _T("text_psw");
const TCHAR* const kOptionRememberControlName = _T("option_remember");
const TCHAR* const kOptionAutoLoginControlName = _T("option_auto");
const TCHAR* const kButtonLoginControlName = _T("btn_login");
const TCHAR* const kButtonLoginCancelControlName = _T("btn_login_cancel");
const TCHAR* const kLableLoginErrorControlName = _T("label_false");


CLoginWnd::CLoginWnd(void)
{	
	m_strURL = _T("");
	m_strUserName = _T("");
}


CLoginWnd::~CLoginWnd(void)
{
}

void CLoginWnd::SetParentWnd(HWND hWnd)
{
	m_hParentWnd = hWnd;
}

void CLoginWnd::InitWindow()
{	
	m_pOptRember = (COptionUI *)m_PaintManager.FindControl(kOptionRememberControlName);
	m_pOptAutoLogin = (COptionUI *)m_PaintManager.FindControl(kOptionAutoLoginControlName);
	m_PUserName = (CEditUI*)m_PaintManager.FindControl(kEditUserName);
	m_PPassWord = (CEditUI*)m_PaintManager.FindControl(kEditPassWord);
	m_PButtonLogin = (CButtonUI*)m_PaintManager.FindControl(kButtonLoginControlName);
	m_PButtonLoginCancel = (CButtonUI*)m_PaintManager.FindControl(kButtonLoginCancelControlName);
	m_PLableError =(CLabelUI*)m_PaintManager.FindControl(kLableLoginErrorControlName);

	m_PUserName->SetFocus();
	InitialUserInfo();
}

CDuiString CLoginWnd::GetSkinFolder()
{
	 return _T("skin");
}

CDuiString CLoginWnd::GetSkinFile()
{
	return _T("login.xml");
}

LPCTSTR CLoginWnd::GetWindowClassName(void) const
{
	return _T("MasterZ_loginwnd");
}

void CLoginWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{	
		//登陆 用户验证
		if (msg.pSender->GetName() == kButtonLoginControlName)
		{	
			if (m_PUserName->GetText().IsEmpty())
			{
				m_PLableError->SetText(_T("用户名不能为空!"));
				return;
			}
			else
			{
				m_strUserName = m_PUserName->GetText();
				SendLoginMessageToMianUI();

				//add by zhumingxing 20140523----显示取消按钮
				m_PButtonLogin->SetVisible(false);
				m_PButtonLoginCancel->SetVisible(true);
			}
		}
		//modefy by zhumingxing 20140523
		if (msg.pSender->GetName() == _T("closebtn"))
		{
			::SendMessage(m_hParentWnd,MSG_LOGIN_CANCEL,0,0);	
			 Close();
		}
		if (msg.pSender->GetName() == _T("btn_login_cancel"))
		{
			::SendMessage(m_hParentWnd,MSG_LOGIN_CANCEL,0,0);

			//add by zhumingxing 20140523----设回登录状态是否需要收回已经取消消息再切换
			m_PLableError->SetText(_T(""));
			m_PButtonLogin->SetVisible(true);
			m_PButtonLoginCancel->SetVisible(false);
			
		}
		//注册
		if (msg.pSender->GetName() == _T("btnRegister"))
		{
			ShellExecute(NULL, _T("open"), URL_SZ_REGISTE, NULL, NULL, SW_NORMAL);
		}
		//忘记密码
		if (msg.pSender->GetName() == _T("btnPassword"))
		{
			ShellExecute(NULL, _T("open"), URL_PASSWORD_FIND, NULL, NULL, SW_NORMAL);
		}
	}
	if (msg.sType == _T("selectchanged"))
	{	
		if (msg.pSender->GetName() == _T("option_auto"))
		{	
			bool bSelected = m_pOptAutoLogin->IsSelected();
			//之前为未选中状态
			if (bSelected)
			{	
				//选中自动登录，记住密码也要被选中
				m_pOptRember->Selected(true);
			}
		}
		if (msg.pSender->GetName() ==_T("option_remember"))
		{
			bool bSelected = m_pOptRember->IsSelected();

			//之前为选中状态
			if (!bSelected)
			{	
				//选中自动登录，记住密码也要被选中
				m_pOptAutoLogin->Selected(false);
			}
		}
		m_PUserName->SetFocus();
	}
	if (msg.sType == _T("textchanged") )
	{
		m_PLableError->SetText(_T(""));
	}
	__super::Notify(msg);
}

LRESULT CLoginWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	switch (uMsg)
	{
	case MSG_LOGIN_CLIENT:
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("正在处理登录消息wparam= %d"),wParam);
			//登录成功
			if (wParam == 1)
			{	
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("登录成功！"));
				int iAutoFlag = 0;
				int iSavePassWordFlag = 0;
				tuserInfotodb tuserinfo = *((tuserInfotodb*)(lParam));

				//根据当前用户选择写配置文件[加密之后写入配置文件]
				if (m_pOptRember->IsSelected())
				{	
					iSavePassWordFlag = 1;
					
					if (m_pOptAutoLogin->IsSelected())
					{
						iAutoFlag = 1;
					}			
				}
				//此处修改为无论用户是否勾选密码都将先关信息存放到数据库，开机启动刷新关键词排名需要用到
				g_globalData.sqlIte.DeleteUserInfo();
				WriteUserInfo(tuserinfo.strUserName,tuserinfo.strPassWord,iAutoFlag,iSavePassWordFlag,tuserinfo.iProductID);
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户信息写入数据库成功！处理用户登录消息完成！"));

				//判断当前登录成功用户名与支持版本是否一致，如果不一样则需要将新的用户信息写入注册表----20150312
				UpdateRegUserInfo(tuserinfo.strUserName,tuserinfo.iProductID);

				this->Close();
			}
			else
			{				
				if (wParam == WPARAM_USERINFO_ERROR)
				{
					m_PLableError->SetText( _T("用户名或密码错误!"));
				}
				else if (wParam == WPARAM_CONNECTSERVER_ERROR)
				{
					m_PLableError->SetText( _T("连接服务器失败！"));
				}
				else
				{	
					CString strErrorInfo;
					strErrorInfo.Format(_T("请求失败,请稍后再试！错误码:%d"),wParam);
					m_PLableError->SetText(strErrorInfo);
				}
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("处理用户登录消息完成！"));
			}

			//返回之后设回登录状态 add by zhumingxing 20140523
			m_PButtonLogin->SetVisible(true);
			m_PButtonLoginCancel->SetVisible(false);
		}
		break;
	//对tab键的支持
	case WM_TAB_KEY_PRESS:
		{
			if (m_PUserName->IsFocused())
			{
				m_PPassWord->SetFocus();
			}
			else if (m_PPassWord->IsFocused())
			{	
				m_PUserName->SetFocus();
			}
		}
		break;
	case  WM_ENTER_KEY_PRESS:
		{	
			m_PButtonLogin->Activate();
			return 0;
		}
	default:
		break;
	}
	//屏蔽对标题栏的双击操作
	if(WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}

void CLoginWnd::SendLoginMessageToMianUI()
{
	PTUSERINTO puserInfo = new _TUSERINFO();
	puserInfo->strURL = m_strURL;
	puserInfo->strUserName = m_PUserName->GetText();
	puserInfo->strPassWord = m_PPassWord->GetText();
	if (m_pOptRember->IsSelected())
	{
		puserInfo->bIsSavePassWord = 1;
	}
	if (m_pOptAutoLogin->IsSelected())
	{
		puserInfo->bIsAutoPassWord = 1;
	}

	m_PLableError->SetText(_T("正在登录..."));
	::SendMessage(m_hParentWnd,MSG_SERVEREX_USER_VALIDATE,(WPARAM)puserInfo,0);
}

void CLoginWnd::InitialUserInfo()
{	
	//记录URL
	CStdString strURL = _T("");
	GetURL(strURL);
	m_strURL = strURL;

	CString strUserName = _T("");
	CString strPassWord = _T("");
	int iAutoFlag = -1;
	int iSavePassWord = -1;

	ReadUserInfo(strUserName,strPassWord,iAutoFlag,iSavePassWord);
	if (iSavePassWord == 1)
	{
		
		m_PUserName->SetText(strUserName);	
		m_PPassWord->SetText(strPassWord);
	}
	
	/*默认都是记住密码和自动登录，此流程和之前的企业版存在区别，原因是因为2.0只要登录成功都会写数据库，而
	  企业版则是记住密码了才会写
	*/

	m_pOptRember->Selected(true);
	m_pOptAutoLogin->Selected(true);
}
