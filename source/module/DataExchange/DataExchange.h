// DataExchange.h : main header file for the DataExchange DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CDataExchangeApp
// See DataExchange.cpp for the implementation of this class
//

class CDataExchangeApp : public CWinApp
{
public:
	CDataExchangeApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
