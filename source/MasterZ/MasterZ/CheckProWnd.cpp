#include "stdafx.h"
#include "CheckProWnd.h"
#include "controls_ex.h"

//滚动条位置
#define  WINDOW_SCROLL_POS 482

CCheckProWnd::CCheckProWnd(void)
{	
	m_bIsCancelFlag = FALSE;
	m_bIsAllItemCheckOk = TRUE;
	m_bIsAllItemRepairOK = TRUE;
}


CCheckProWnd::~CCheckProWnd(void)
{
}

void CCheckProWnd::SetParentWnd(HWND hWnd)
{	
	m_hParentWnd = hWnd;
}

void CCheckProWnd::InitWindow()
{	
	InitialProListCtrl();
}

CDuiString CCheckProWnd::GetSkinFolder()
{
	return _T("skin");
}

CDuiString CCheckProWnd::GetSkinFile()
{
	return _T("CheckPro.xml");
}

LPCTSTR CCheckProWnd::GetWindowClassName(void) const
{
	return _T("Client_CheckWnd");
}

CControlUI* CCheckProWnd::CreateControl(LPCTSTR pstrClass)
{
	if (_tcscmp(pstrClass,_T("ButtonGif")) == 0)
	{
		return  new CButtonGifUI;
	}
}

LRESULT CCheckProWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//屏蔽对标题栏的双击操作
	switch (uMsg)
	{
		//屏蔽鼠标右键消息
	case WM_LBUTTONDOWN:
		{	
			int yPos = GET_Y_LPARAM(lParam);
			int xpos = GET_X_LPARAM(lParam);
			ChangeItemColor(xpos,yPos);
		}
		break;
	default:
		break;
	}

	if(WM_NCLBUTTONDBLCLK != uMsg)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return 0;
}

void CCheckProWnd::Notify(TNotifyUI& msg)
{	
	if (msg.sType == _T("click"))
	{	
		//用户手机点击关闭按钮
		if (msg.pSender->GetName() == _T("btn_cancle"))
		{	
			m_bIsCancelFlag = TRUE;
			::SendMessage(m_hParentWnd,MSG_DIAGNOSIS_CANCEL,0,0);
			Close();
			return;
		}
		//点击开始诊断按钮
		if (msg.pSender->GetName() == _T("btn_check") && (msg.pSender->GetText() == _T("开始诊断") ||msg.pSender->GetText() == _T("重新诊断")))
		{	
			m_bIsAllItemCheckOk = TRUE;
			m_bIsCancelFlag = FALSE;
			InitialStartCheckUI();
			::SendMessage(m_hParentWnd,MSG_SELF_DIAGNOSIS,0,LPARAM_CHILD_TO_MIAN);
			return;
		}
		//用户点击取消诊断按钮
		if (msg.pSender->GetName() == _T("btn_check") && msg.pSender->GetText() == _T("取消"))
		{	
			m_bIsCancelFlag = TRUE;
			SetCancelCheckUI();
			::SendMessage(m_hParentWnd,MSG_DIAGNOSIS_CANCEL,0,0);
			return;
		}
		//一键修复功能
		if (msg.pSender->GetName() == _T("btn_repairall") )
		{	
			m_bIsAllItemRepairOK = TRUE;

			if (msg.pSender->GetText() == _T("重新修复"))
			{
				m_vecNeedRepair = m_vecRepairFail;
				m_vecRepairFail.clear();
			}
			SetRepairRunningUI();
			m_dwCurrentRepairIndex =  _ttoi(m_vecNeedRepair[0]);
			m_vecNeedRepair.erase(m_vecNeedRepair.begin());
			::SendMessage(m_hParentWnd,MSG_REPAIR_FAULT,m_dwCurrentRepairIndex,LPARAM_CHILD_TO_MIAN);
		}
	} 
	__super::Notify(msg);
}

void CCheckProWnd::SetProblemList( const T_DATA_FROM_XML& tProList )
{	
	m_tProblemList.vProblem.clear();
	m_tProblemList = tProList;
}

//初始化界面状态
void CCheckProWnd::InitialProListCtrl()
{	
	CVerticalLayoutUI* pVMianLayout = (CVerticalLayoutUI*)m_PaintManager.FindControl(_T("mianlist"));
	if (NULL != pVMianLayout)
	{
		pVMianLayout->RemoveAll();
	}

	CString strFormatStr = _T("");
	for (int iindex = 0; iindex < m_tProblemList.vProblem.size(); ++ iindex)
	{
		//动态添加检测项
		CTextUI* ptextCheckItem = new CTextUI;
		strFormatStr.Format(_T("client_checkitem%s"),m_tProblemList.vProblem[iindex].strIndex);
		ptextCheckItem->SetAttribute(_T("name"),strFormatStr);
		ptextCheckItem->SetAttribute(_T("text"),m_tProblemList.vProblem[iindex].strPromble);
		ptextCheckItem->SetAttribute(_T("textcolor"),_T("#ffa4a4a4"));
		ptextCheckItem->SetAttribute(_T("width"),_T("300"));
		ptextCheckItem->SetAttribute(_T("endellipsis"),_T("true"));
		ptextCheckItem->SetAttribute(_T("textpadding"),_T("15,15,0,0"));
		ptextCheckItem->SetAttribute(_T("font"),_T("0"));

		//动态添加建议项----初始化隐藏
		CTextUI* pTextSuggestItem = new CTextUI;
		strFormatStr.Format(_T("pro_suggest%s"),m_tProblemList.vProblem[iindex].strIndex);
		pTextSuggestItem->SetAttribute(_T("name"),strFormatStr);
		pTextSuggestItem->SetAttribute(_T("text"),m_tProblemList.vProblem[iindex].strSuggestion);
		pTextSuggestItem->SetAttribute(_T("textcolor"),_T("#ff000000"));
		pTextSuggestItem->SetAttribute(_T("width"),_T("410"));
		pTextSuggestItem->SetAttribute(_T("endellipsis"),_T("true"));
		pTextSuggestItem->SetAttribute(_T("height"),_T("30"));
		pTextSuggestItem->SetAttribute(_T("textpadding"),_T("15,7,0,0"));
		pTextSuggestItem->SetAttribute(_T("visible"),_T("false"));

		//垂直布局
		CVerticalLayoutUI* pVLayout_Pro1 = new CVerticalLayoutUI;
		pVLayout_Pro1->SetAttribute(_T("height"),_T("65"));
		pVLayout_Pro1->SetAttribute(_T("width"),_T("500"));
		pVLayout_Pro1->SetAttribute(_T("childpadding"),_T("3"));

		//添加进两个控件到垂直布局中
		pVLayout_Pro1->Add(ptextCheckItem);
		pVLayout_Pro1->Add(pTextSuggestItem);

		//动态添加右侧检测状态按钮
		CTextUI* pButtonCheck = new CTextUI;
		strFormatStr.Format(_T("check_state%s"),m_tProblemList.vProblem[iindex].strIndex);
		pButtonCheck->SetAttribute(_T("name"),strFormatStr);

		pButtonCheck->SetAttribute(_T("text"),_T("未诊断"));
		pButtonCheck->SetAttribute(_T("textcolor"),_T("#ffa4a4a4"));
		pButtonCheck->SetAttribute(_T("float"),_T("true"));
		pButtonCheck->SetAttribute(_T("pos"),_T("425,15,500,40"));
		pButtonCheck->SetAttribute(_T("align"),_T("left"));
		pButtonCheck->SetAttribute(_T("font"),_T("0"));

		//动态添加右侧修复按钮状态----默认隐藏
		CButtonUI* pButtonRepair = new CButtonUI;
		strFormatStr.Format(_T("repair_problem%s"),m_tProblemList.vProblem[iindex].strIndex);
		pButtonRepair->SetAttribute(_T("name"),strFormatStr);
		pButtonRepair->SetAttribute(_T("text"),_T("修复"));
		pButtonRepair->SetAttribute(_T("textcolor"),_T("#ff44A4D3"));
		pButtonRepair->SetAttribute(_T("disabledtextcolor"),_T("#ff44A4D3"));
		pButtonRepair->SetAttribute(_T("float"),_T("true"));
		pButtonRepair->SetAttribute(_T("pos"),_T("428,45,500,65"));
		pButtonRepair->SetAttribute(_T("align"),_T("left"));
		pButtonRepair->SetAttribute(_T("visible"),_T("false"));
		pButtonRepair->SetAttribute(_T("font"),_T("0"));
		pButtonRepair->SetEnabled(FALSE);
		//水平布局项
		CHorizontalLayoutUI* pHLayoutMian = new CHorizontalLayoutUI;
		strFormatStr.Format(_T("HlayOut%s"),m_tProblemList.vProblem[iindex].strIndex);

		pHLayoutMian->SetAttribute(_T("name"),strFormatStr);
		pHLayoutMian->SetAttribute(_T("width"),_T("500"));
		pHLayoutMian->SetAttribute(_T("height"),_T("66"));
		pHLayoutMian->SetAttribute(_T("bordersize"),_T("1"));
		pHLayoutMian->SetAttribute(_T("bordercolor"),_T("#ffdddddd"));
		pHLayoutMian->SetAttribute(_T("padding"),_T("0,-1,0,0"));

		pHLayoutMian->Add(pVLayout_Pro1);
		pHLayoutMian->Add(pButtonCheck);
		pHLayoutMian->Add(pButtonRepair);

		CVerticalLayoutUI* pVLayout = (CVerticalLayoutUI*)m_PaintManager.FindControl(_T("mianlist"));
		if (NULL != pVLayout)
		{
			pVLayout->Add(pHLayoutMian);
		}
	}
}

void CCheckProWnd::ChangeItemColor(DWORD dwMousePosx, DWORD dwMousePosy)
{
	//排除滚动条
	if (dwMousePosx > WINDOW_SCROLL_POS)
	{
		return;
	}

	for (int m = 0; m < m_tProblemList.vProblem.size(); ++m)
	{	

		CString str;
		str.Format(_T("HlayOut%s"),m_tProblemList.vProblem[m].strIndex);
		CHorizontalLayoutUI* p = (CHorizontalLayoutUI*)m_PaintManager.FindControl(str);
		RECT retSize = p->GetPos();

		if ( retSize.top <= dwMousePosy && dwMousePosy <= retSize.bottom)
		{	
			for (int n = 0; n < m_tProblemList.vProblem.size(); ++n)
			{	
				CString str;
				str.Format(_T("HlayOut%s"),m_tProblemList.vProblem[n].strIndex);
				CHorizontalLayoutUI* p = (CHorizontalLayoutUI*)m_PaintManager.FindControl(str);
				p->SetAttribute(_T("bkcolor"),_T("#FFFFFFFF"));
			}
			p->SetAttribute(_T("bkcolor"),_T("#FFEBF6FF"));
			return;
		}
	}
}

//开始诊断
void CCheckProWnd::InitialStartCheckUI()
{	
	m_vecCheckBak.clear();
	m_vecNeedRepair.clear();
	m_vecRepairFail.clear();

	//隐藏图标
	CButtonUI* pbutInitial = (CButtonGifUI*)m_PaintManager.FindControl(_T("repairimage"));
	pbutInitial->SetVisible(false);

	//隐藏初始化文字
	CTextUI* ptextIntial = (CTextUI*)m_PaintManager.FindControl(_T("text_initial"));
	ptextIntial->SetVisible(false);

	//显示运行状态
	CTextUI* ptextRunning =  (CTextUI*)m_PaintManager.FindControl(_T("text_running"));
	ptextRunning->SetText(_T("舟大师正在为您进行问题诊断，请稍后！"));
	ptextRunning->SetVisible(true);

	//显示运行时的gif图片
	CButtonGifUI* pbuttonRunning = (CButtonGifUI*)m_PaintManager.FindControl(_T("btn_checkrunning"));
	pbuttonRunning->SetVisible(true);

	//将检测按钮文字设置为取消
	CButtonUI* pSatrt = (CButtonUI*)m_PaintManager.FindControl(_T("btn_check"));
	pSatrt->SetText(_T("取消"));

	//隐藏掉一键修复按钮
	CButtonUI* pRepairAll = (CButtonUI*)m_PaintManager.FindControl(_T("btn_repairall"));
	pRepairAll->SetVisible(false);

	CString strCtrlName = _T("");
	for (int m = 0; m < m_tProblemList.vProblem.size(); ++m)
	{	
		//重设检测点字体颜色为灰色
		strCtrlName.Format(_T("client_checkitem%s"),m_tProblemList.vProblem[m].strIndex);
		CTextUI* ptextCheckItem = (CTextUI*)m_PaintManager.FindControl(strCtrlName);
		ptextCheckItem->SetAttribute(_T("textcolor"),_T("#ffa4a4a4"));

		//隐藏建议
		strCtrlName.Format(_T("pro_suggest%s"),m_tProblemList.vProblem[m].strIndex);
		CTextUI* pSuggest = (CTextUI*)m_PaintManager.FindControl(strCtrlName);
		pSuggest->SetVisible(false);

		//隐藏修复按钮
		strCtrlName.Format(_T("repair_problem%s"),m_tProblemList.vProblem[m].strIndex);
		CButtonUI* pButtonRepair = (CButtonUI*)m_PaintManager.FindControl(strCtrlName);
		pButtonRepair->SetAttribute(_T("disabledtextcolor"),_T("#ff44A4D3"));
		pButtonRepair->SetVisible(false);

		//重置检测状态
		strCtrlName.Format(_T("check_state%s"),m_tProblemList.vProblem[m].strIndex);
		CTextUI* pstate = (CTextUI*)m_PaintManager.FindControl(strCtrlName);
		pstate->SetText(_T("诊断中"));
		pstate->SetAttribute(_T("textcolor"),_T("#ff0000CD"));
	}
}

//取消诊断
void CCheckProWnd::SetCancelCheckUI()
{	
	//显示图标
	CButtonUI* pbutInitial = (CButtonGifUI*)m_PaintManager.FindControl(_T("repairimage"));

	//如果所有检测项都成功了，则显示成功的图片
	if (m_bIsAllItemCheckOk)
	{
		pbutInitial->SetAttribute(_T("bkimage"),_T("file=\'images\\checkpro_ok.png\'"));
	}
	else
	{
		pbutInitial->SetAttribute(_T("bkimage"),_T("file=\'images\\checkpro_fail.png\'"));
	}
	pbutInitial->SetVisible(true);

	//显示初始化文字
	CTextUI* ptextIntial = (CTextUI*)m_PaintManager.FindControl(_T("text_initial"));

	if (m_bIsAllItemCheckOk)
	{
		ptextIntial->SetAttribute(_T("text"),_T("诊断完成，未发现异常！"));
	}
	else
	{
		ptextIntial->SetAttribute(_T("text"),_T("诊断完成，发现异常建议立即修复！"));
	}
	ptextIntial->SetAttribute(_T("pos"),_T("100,28,400,75"));
	ptextIntial->SetVisible(true);


	//隐藏运行状态
	CTextUI* ptextRunning =  (CTextUI*)m_PaintManager.FindControl(_T("text_running"));
	ptextRunning->SetVisible(false);

	//隐藏运行时的gif图片
	CButtonGifUI* pbuttonRunning = (CButtonGifUI*)m_PaintManager.FindControl(_T("btn_checkrunning"));
	pbuttonRunning->SetVisible(false);

	CButtonUI* pSatrt = (CButtonUI*)m_PaintManager.FindControl(_T("btn_check"));
	pSatrt->SetText(_T("重新诊断"));

	//对于还没有返回检测结果的项取消时应该恢复至初始化状态
	for (int iindex = 0; iindex < m_tProblemList.vProblem.size(); ++ iindex)
	{	
		bool bFindFlag = false;

		for (int iindex1 = 0; iindex1 < m_vecCheckBak.size(); ++iindex1)
		{
			if (m_tProblemList.vProblem[iindex].strIndex != m_vecCheckBak[iindex1])
			{
				continue;
			}
			else
			{
				bFindFlag = TRUE;
				break;
			}
		}

		if (!bFindFlag)
		{
			//检测点颜色也要变过来
			CString strCheckButtonName;

			strCheckButtonName.Format(_T("client_checkitem%s"),m_tProblemList.vProblem[iindex].strIndex);
			CTextUI* ptextCheckItem = (CTextUI*)m_PaintManager.FindControl(strCheckButtonName);
			ptextCheckItem->SetAttribute(_T("textcolor"),_T("#ffa4a4a4"));

			strCheckButtonName.Format(_T("check_state%s"),m_tProblemList.vProblem[iindex].strIndex);
			CTextUI* pButtonCheck = (CTextUI*)m_PaintManager.FindControl(strCheckButtonName);
			pButtonCheck->SetText(_T("未诊断"));
			pButtonCheck->SetAttribute(_T("textcolor"),_T("#ffa4a4a4"));
		}

	}

	CButtonUI* pRepairAll = (CButtonUI*)m_PaintManager.FindControl(_T("btn_repairall"));
	pRepairAll->SetText(_T("一键修复"));
	if (m_vecNeedRepair.size() != 0)
	{
		pRepairAll->SetVisible(true);
	}
	else
	{
		pRepairAll->SetVisible(false);
	}
}

//因为问题项是并行执行所以接收到检测完成之后需要进行是否全部执行的判断
BOOL CCheckProWnd::IsProItemCheckFinish()
{
	//对于还没有返回检测结果的项取消时应该恢复至初始化状态
	for (int iindex = 0; iindex < m_tProblemList.vProblem.size(); ++ iindex)
	{	
		CString str1 = m_tProblemList.vProblem[iindex].strIndex;

		bool bFindFlag = false;
		for (int iindex1 = 0; iindex1 < m_vecCheckBak.size(); ++iindex1)
		{
			if (m_tProblemList.vProblem[iindex].strIndex != m_vecCheckBak[iindex1])
			{
				continue;
			}
			else
			{
				bFindFlag = TRUE;
				break;
			}
		}

		if (!bFindFlag)
		{
			return FALSE;
		}

	}

	return TRUE;
}

void CCheckProWnd::SetFinishCheckUI()
{
	//显示图标
	CButtonUI* pbutInitial = (CButtonGifUI*)m_PaintManager.FindControl(_T("repairimage"));

	//如果所有检测项都成功了，则显示成功的图片
	if (m_bIsAllItemCheckOk)
	{
		pbutInitial->SetAttribute(_T("bkimage"),_T("file=\'images\\checkpro_ok.png\'"));
	}
	else
	{
		pbutInitial->SetAttribute(_T("bkimage"),_T("file=\'images\\checkpro_fail.png\'"));
	}
	pbutInitial->SetVisible(true);

	//显示初始化文字
	CTextUI* ptextIntial = (CTextUI*)m_PaintManager.FindControl(_T("text_initial"));

	if (m_bIsAllItemCheckOk)
	{
		ptextIntial->SetAttribute(_T("text"),_T("诊断完成，未发现异常！"));
	}
	else
	{
		ptextIntial->SetAttribute(_T("text"),_T("诊断完成，发现异常建议立即修复！"));
	}
	ptextIntial->SetAttribute(_T("pos"),_T("100,28,400,75"));
	ptextIntial->SetVisible(true);

	//隐藏运行状态
	CTextUI* ptextRunning =  (CTextUI*)m_PaintManager.FindControl(_T("text_running"));
	ptextRunning->SetVisible(false);

	//隐藏运行时的gif图片
	CButtonGifUI* pbuttonRunning = (CButtonGifUI*)m_PaintManager.FindControl(_T("btn_checkrunning"));
	pbuttonRunning->SetVisible(false);

	//重新设置为开始诊断状态
	CButtonUI* pSatrt = (CButtonUI*)m_PaintManager.FindControl(_T("btn_check"));
	pSatrt->SetText(_T("重新诊断"));

	CButtonUI* pRepairAll = (CButtonUI*)m_PaintManager.FindControl(_T("btn_repairall"));
	pRepairAll->SetText(_T("一键修复"));
	if (m_vecNeedRepair.size() != 0)
	{
		pRepairAll->SetVisible(true);
	}
	else
	{
		pRepairAll->SetVisible(false);
	}
}

void CCheckProWnd::HandleCheckRetrunMsg(T_PROBLEM_DATA& tCheckBak)
{

	CString strName = _T("");
	m_tCheckBackData = tCheckBak;

	if (m_tCheckBackData.bCheckFlag == TRUE)
	{	
		strName.Format(_T("client_checkitem%s"),m_tCheckBackData.strIndex);
		CTextUI* ptextCheckItem = (CTextUI*)m_PaintManager.FindControl(strName);
		ptextCheckItem->SetAttribute(_T("textcolor"),_T("#FF3c8942"));

		strName.Format(_T("check_state%s"),m_tCheckBackData.strIndex);
		CTextUI* pButtonCheck =  (CTextUI*)m_PaintManager.FindControl(strName);
		pButtonCheck->SetAttribute(_T("text"),_T("正常"));
		pButtonCheck->SetAttribute(_T("textcolor"),_T("#FF3c8942"));
	}
	else
	{	
		m_bIsAllItemCheckOk = FALSE;
		strName.Format(_T("client_checkitem%s"),m_tCheckBackData.strIndex);
		CTextUI* ptextCheckItem = (CTextUI*)m_PaintManager.FindControl(strName);
		ptextCheckItem->SetAttribute(_T("textcolor"),_T("#FFc92605"));

		strName.Format(_T("check_state%s"),m_tCheckBackData.strIndex);
		CTextUI* pButtonCheck =  (CTextUI*)m_PaintManager.FindControl(strName);
		pButtonCheck->SetAttribute(_T("text"),_T("异常"));
		pButtonCheck->SetAttribute(_T("textcolor"),_T("#FFc92605"));

		strName.Format(_T("pro_suggest%s"),m_tCheckBackData.strIndex);
		CTextUI* pTextSuggestItem = (CTextUI*)m_PaintManager.FindControl(strName);
		pTextSuggestItem->SetAttribute(_T("text"),m_tCheckBackData.strSuggestion);
		pTextSuggestItem->SetAttribute(_T("textcolor"),_T("#ff000000"));
		pTextSuggestItem->SetToolTip(m_tCheckBackData.strSuggestion);
		pTextSuggestItem->SetVisible(true);

		if (m_tCheckBackData.bIsRepair)
		{
			m_vecNeedRepair.push_back(m_tCheckBackData.strIndex);
		}

	}
	m_vecCheckBak.push_back(m_tCheckBackData.strIndex);

	if (IsProItemCheckFinish())
	{
		SetFinishCheckUI();
	}
}

BOOL CCheckProWnd::GetCancelFlag()
{
	return m_bIsCancelFlag;
}

void CCheckProWnd::HandleRepairRetrunMsg( T_PROBLEM_DATA& tRepairBak )
{	
	CString strName = _T("");
	m_tRepairBackData = tRepairBak;

	//此处需要对修复按钮状态进行处理
	if (m_tRepairBackData.bRepairFlag)
	{	
		strName.Format(_T("repair_problem%s"),m_tRepairBackData.strIndex);
		CButtonUI* pButtonRepair = (CButtonUI*)m_PaintManager.FindControl(strName);
		pButtonRepair->SetVisible(false);

		//隐藏掉建议及设置正常的状态
		strName.Format(_T("pro_suggest%s"),m_tRepairBackData.strIndex);
		CTextUI* pTextSuggestItem = (CTextUI*)m_PaintManager.FindControl(strName);
		pTextSuggestItem->SetVisible(false);

		strName.Format(_T("client_checkitem%s"),m_tRepairBackData.strIndex);
		CTextUI* ptextCheckItem = (CTextUI*)m_PaintManager.FindControl(strName);
		ptextCheckItem->SetAttribute(_T("textcolor"),_T("#FF3c8942"));

		strName.Format(_T("check_state%s"),m_tRepairBackData.strIndex);
		CTextUI* pButtonCheck =  (CTextUI*)m_PaintManager.FindControl(strName);
		pButtonCheck->SetAttribute(_T("text"),_T("修复成功"));
		pButtonCheck->SetAttribute(_T("textcolor"),_T("#FF3c8942"));

	}	
	else
	{	
		m_bIsAllItemRepairOK = FALSE;
		m_vecRepairFail.push_back(m_tRepairBackData.strIndex);

		strName.Format(_T("repair_problem%s"),m_tRepairBackData.strIndex);
		CButtonUI* pButtonRepair = (CButtonUI*)m_PaintManager.FindControl(strName);
		pButtonRepair->SetAttribute(_T("disabledtextcolor"),_T("#FFc92605"));
		pButtonRepair->SetAttribute(_T("text"),_T("修复失败"));

		//如果是时间修复失败，则提示用户已管理员身份运行软件修复或手动修改
		if (!m_tRepairBackData.strIndex.Compare(_T("3")))
		{
			strName.Format(_T("pro_suggest%s"),m_tRepairBackData.strIndex);
			CTextUI* pTextSuggestItem = (CTextUI*)m_PaintManager.FindControl(strName);
			pTextSuggestItem->SetAttribute(_T("text"),m_tRepairBackData.strSuggestion);
			pTextSuggestItem->SetAttribute(_T("textcolor"),_T("#FFc92605"));
			pTextSuggestItem->SetToolTip(m_tRepairBackData.strSuggestion);
		}
	}
	//此处处理完一项的修复结果,然后再发送下一项的结果,只有当全部结果都返回完毕时
	if (m_vecNeedRepair.size() == 0)
	{
		SetFinishRepairUI();
	}
	else
	{
		m_dwCurrentRepairIndex = _ttoi(m_vecNeedRepair[0]);
		m_vecNeedRepair.erase(m_vecNeedRepair.begin());
		::SendMessage(m_hParentWnd,MSG_REPAIR_FAULT,m_dwCurrentRepairIndex,LPARAM_CHILD_TO_MIAN);
	}
}

void CCheckProWnd::SetRepairRunningUI()
{
	//隐藏图标
	CButtonUI* pbutInitial = (CButtonGifUI*)m_PaintManager.FindControl(_T("repairimage"));
	pbutInitial->SetVisible(false);

	//隐藏初始化文字
	CTextUI* ptextIntial = (CTextUI*)m_PaintManager.FindControl(_T("text_initial"));
	ptextIntial->SetVisible(false);

	//显示运行状态
	CTextUI* ptextRunning =  (CTextUI*)m_PaintManager.FindControl(_T("text_running"));
	ptextRunning->SetText(_T("舟大师正在为您进行问题修复，请稍后！"));
	ptextRunning->SetVisible(true);

	//显示运行时的gif图片
	CButtonGifUI* pbuttonRunning = (CButtonGifUI*)m_PaintManager.FindControl(_T("btn_checkrunning"));
	pbuttonRunning->SetVisible(true);

	//隐藏检测和修复按钮

	CButtonUI* pStartCheck = (CButtonUI*)m_PaintManager.FindControl(_T("btn_check"));
	pStartCheck->SetVisible(FALSE);

	CButtonUI* pRepairAll = (CButtonUI*)m_PaintManager.FindControl(_T("btn_repairall"));
	pRepairAll->SetVisible(FALSE);

	CString strCtrlName = _T("");
	for (int iindex = 0; iindex < m_vecNeedRepair.size(); ++ iindex)
	{	
		strCtrlName.Format(_T("repair_problem%s"),m_vecNeedRepair[iindex]);
		CButtonUI* pButtonRepair = (CButtonUI*)m_PaintManager.FindControl(strCtrlName);
		pButtonRepair->SetAttribute(_T("text"),_T("正在修复"));
		pButtonRepair->SetAttribute(_T("textcolor"),_T("#ff44A4D3"));
		pButtonRepair->SetAttribute(_T("disabledtextcolor"),_T("#ff44A4D3"));
		pButtonRepair->SetVisible(true);
		pButtonRepair->SetEnabled(false);
	}
	sort(m_vecNeedRepair.begin(),m_vecNeedRepair.end());
}

void CCheckProWnd::SetFinishRepairUI()
{	
	//显示图标
	CButtonUI* pbutInitial = (CButtonGifUI*)m_PaintManager.FindControl(_T("repairimage"));

	//如果所有检测项都成功了，则显示成功的图片
	if (m_bIsAllItemRepairOK)
	{
		pbutInitial->SetAttribute(_T("bkimage"),_T("file=\'images\\checkpro_ok.png\'"));
	}
	else
	{
		pbutInitial->SetAttribute(_T("bkimage"),_T("file=\'images\\checkpro_fail.png\'"));
	}
	pbutInitial->SetVisible(true);

	//显示初始化文字
	CTextUI* ptextIntial = (CTextUI*)m_PaintManager.FindControl(_T("text_initial"));

	if (m_bIsAllItemRepairOK)
	{
		ptextIntial->SetAttribute(_T("text"),_T("修复成功，请重新诊断！"));
	}
	else
	{
		ptextIntial->SetAttribute(_T("text"),_T("修复失败，建议重新修复！"));
		CButtonUI* pRepairAll = (CButtonUI*)m_PaintManager.FindControl(_T("btn_repairall"));
		pRepairAll->SetText(_T("重新修复"));
		pRepairAll->SetVisible(true);

	}
	ptextIntial->SetAttribute(_T("pos"),_T("100,28,400,75"));
	ptextIntial->SetVisible(true);

	//隐藏运行状态
	CTextUI* ptextRunning =  (CTextUI*)m_PaintManager.FindControl(_T("text_running"));
	ptextRunning->SetVisible(false);

	//隐藏运行时的gif图片
	CButtonGifUI* pbuttonRunning = (CButtonGifUI*)m_PaintManager.FindControl(_T("btn_checkrunning"));
	pbuttonRunning->SetVisible(false);

	//重新设置为开始诊断状态
	CButtonUI* pSatrt = (CButtonUI*)m_PaintManager.FindControl(_T("btn_check"));
	pSatrt->SetText(_T("重新诊断"));
	pSatrt->SetVisible(true);
}

void CCheckProWnd::ActiveCheckButton()
{
	CButtonUI* pSatrt = (CButtonUI*)m_PaintManager.FindControl(_T("btn_check"));
	pSatrt->Activate();
}

