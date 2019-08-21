#include "StdAfx.h"
#include "Directory.h"
#include "Reg.h"

#define REG_INSTALL_ROOT  HKEY_CURRENT_USER
#define REG_INSTALL_PATH  _T("Software\\szw\\MasterZ\\Setup")

CDirectory::CDirectory(void)
{
};

CDirectory::~CDirectory(void)
{
}

//返回安装目录
TCHAR *CDirectory::GetInstallDir(void)
{
	CReg reg;
	BYTE *pbValue = reg.ReadValueOfKey(REG_INSTALL_ROOT, REG_INSTALL_PATH, _T("Install Path"));
 	if (pbValue)
 	{
		memset(m_tszInstallDir, 0, sizeof(m_tszInstallDir));
		_tcscpy(m_tszInstallDir, (TCHAR*)pbValue);
	}

	return m_tszInstallDir;
}

//返回当前目录
TCHAR *CDirectory::GetCurrentDir(void)
{
	GetModuleFileName(NULL, m_tszCurrentDir, MAX_PATH);

	TCHAR *pEndBacklash = _tcsrchr(m_tszCurrentDir, _T('\\'));

	if (pEndBacklash != NULL)
		*pEndBacklash = 0;

	return m_tszCurrentDir;
}

//返回客户端程序路径（包括文件名）
TCHAR *CDirectory::GetClientFilePath(void)
{
	CReg reg;
	BYTE *pbValue = reg.ReadValueOfKey(REG_INSTALL_ROOT, REG_INSTALL_PATH, _T("Client"));
	if (pbValue)
	{
		memset(m_tMasterZFilePath, 0, sizeof(m_tMasterZFilePath));
		_tcscpy(m_tMasterZFilePath, (TCHAR*)pbValue);
	}

	return m_tMasterZFilePath;
}

//返回关键词工具的程序路径（包括文件名）
TCHAR *CDirectory::GetKeyWordProcPath(void)
{
	CReg reg;
	BYTE *pbValue = reg.ReadValueOfKey(REG_INSTALL_ROOT, REG_INSTALL_PATH, _T("KWProc"));
	if (pbValue)
	{
		memset(m_tszKeyWordProcPath, 0, sizeof(m_tszKeyWordProcPath));
		_tcscpy(m_tszKeyWordProcPath, (TCHAR *)pbValue);
	}

	return m_tszKeyWordProcPath;
}

//获取主控的程序路径（包括文件名）
TCHAR *CDirectory::GetMcProcPath(void)
{
	CReg reg;
	BYTE *pbValue = reg.ReadValueOfKey(REG_INSTALL_ROOT, REG_INSTALL_PATH, _T("MCProc"));
	if (pbValue)
	{
		memset(m_tszMcProcPath, 0, sizeof(m_tszMcProcPath));
		_tcscpy(m_tszMcProcPath, (TCHAR *)pbValue);
	}

	return m_tszMcProcPath;
}