#pragma once

#include <tchar.h>
#include <AtlBase.h>
#include <comutil.h>
#include <stdlib.h>
//#include <afxstr.h>

#define OID_802_3_PERMANENT_ADDRESS        0x01010101
//#define OID_802_3_CURRENT_ADDRESS        0x01010102
#define IOCTL_NDIS_QUERY_GLOBAL_STATS      0x00170002


TCHAR *GetCurPath(void);
TCHAR *GetProgPath(void);
TCHAR *GetInstallPath(void);
LPWSTR MByteToWChar( LPSTR lpcszStr, LPWSTR lpwszStr, DWORD *dwSize, UINT codePage = CP_ACP);
LPSTR WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD *dwSize, UINT codePage = CP_OEMCP);
bool WaitProcEnd(TCHAR *pProgName, DWORD dwTimeOut, bool bForceEnd);

// 通过DriverIO获取MAC地址
int GetMac(TCHAR *szMac, int nBuffSize);

CString GetMacByAPI();

CString GetMacByCongigAll();

int GetRandomNum();

void GetRandomMac(TCHAR *szMac, int nBufferSize);

BOOL GetMacFromRegedit(HKEY hKey, const TCHAR *const pszKeyPath, const TCHAR *const pszKeyName, TCHAR* szMac);
BOOL WriteMacToRegedit(HKEY hKey, const TCHAR *const pszKeyPath,
	const TCHAR *const pszKeyName,
	const TCHAR *const pszKeyValue);
// int CompareBSTR(_bstr_t bstr1, _bstr_t bstr2);
// int CompareBSTRNoCase(_bstr_t bstr1, _bstr_t bstr2);

//返回一个全局递增的数字（原子操作）
LONG GetGlobalIndexNum(void);

// 获取MAC地址
CString GetPhysicalAddress();

// 获取操作系统的位数
void GetSystemBits(__out TCHAR* szBits);
void SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo);

// 获取操作系统名称
void GetSystemName(__out TCHAR* szSystemName);

// 通过Kernel32.dll版本信息跟GetVersionEx配合获取,因为只用GetVersionEx在操作系统程序设置成兼容模式下无法正确判断
void GetSystemNameByKernel32(__out TCHAR* szSystemName);

// 获取操作系统版本名称
typedef BOOL(WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
void GetSystemMark(__out TCHAR* szSystemMark);

// 获取操作系统服务包版本
void GetSystemServicePack(__out TCHAR* szSystemSP);

// 获取操作系统的详细信息
void GetSystemDetailedInfo(__out TCHAR* szSystemInfo);