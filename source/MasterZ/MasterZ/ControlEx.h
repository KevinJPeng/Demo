//////////////////////////////////////////////////////////////////////////////////////
//Description: 二级（含二级）以上的自定义控件的创建回调实现
//             一级自定义控件的创建是在WindowImplBase继承类的CreateControl中实现
//
//二级自定义控件是指：在主XML中包含自定义控件，该自定义控件对应的XML中又包括自定义控件。
//以此类推，也可以有三级、四级等自定义控件，其中每一级对应一个创建回调。
//////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "controls_ex.h"

//二级自定义控件的创建回调实现
class CDialogBuilderCallbackEx2 : public IDialogBuilderCallback
{
public:
	CDialogBuilderCallbackEx2(CPaintManagerUI *pPaintMgr)
	{
		m_PaintManager = pPaintMgr;
	}

	CControlUI* CreateControl(LPCTSTR pstrClass) 
	{
		if( _tcscmp(pstrClass, _T("ChartWnd")) == 0 )
		{
			//CWndUI  *pUI  = new CWndUI;      
			//HWND    hWnd  = CreateWindow(_T("BUTTON"), _T(""), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, m_PaintManager->GetPaintWindow(), NULL, NULL, NULL);
			//pUI->Attach(hWnd);  
			////pUI->AddPoint();
			//m_HwndButton = hWnd;
			//m_pwndui = pUI;
			//return pUI;
		}
		if (_tcscmp(pstrClass,_T("ButtonGif")) == 0)
		{
			return  new CButtonGifUI;
		}

		return NULL;
	}

private:
	CPaintManagerUI *m_PaintManager;
};


////三级自定义控件的创建回调实现
//class CDialogBuilderCallbackEx3 : public IDialogBuilderCallback
//{
//public:
//	CDialogBuilderCallbackEx3()
//	{
//	}
//
//	CControlUI* CreateControl(LPCTSTR pstrClass) 
//	{
//		if( _tcscmp(pstrClass, _T("ChartWnd")) == 0 )
//		{
//			//...
//		}
//
//		return NULL;
//	}
//};