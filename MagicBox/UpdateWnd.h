#ifndef _UPDATEWND_H_
#define _UPDATEWND_H_
#include "duilib/UIlib.h"
using namespace DuiLib;

// #ifdef _DEBUG
// #   ifdef _UNICODE
// #       pragma comment(lib, "DuiLib_ud.lib")
// #   else
// #       pragma comment(lib, "DuiLib_d.lib")
// #   endif
// #else
// #   ifdef _UNICODE
// #       pragma comment(lib, "DuiLib_u.lib")
// #   else
// #       pragma comment(lib, "DuiLib.lib")
// #   endif
// #endif

class CUpdateWnd :public WindowImplBase
{
public:
	CUpdateWnd(void);
	~CUpdateWnd(void);
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	virtual void InitWindow();
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);

	virtual LPCTSTR GetResourceID() const;
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	void Notify(TNotifyUI& msg);
	void SetBATPath(CStdString _sBATPath);
private:
private:
	CStdString m_sBATPath;
};

#endif