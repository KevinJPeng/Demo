#pragma once

class CDirectory
{
public:
	CDirectory(void);
	~CDirectory(void);

	//返回安装目录
	TCHAR *GetInstallDir(void);

	//返回当前目录
	TCHAR *GetCurrentDir(void);

	//返回客户端程序路径（包括文件名）
	TCHAR *GetClientFilePath(void);

	//返回关键词工具的程序路径（包括文件名）
	TCHAR *GetKeyWordProcPath(void);

	//获取主控的程序路径（包括文件名）
	TCHAR *GetMcProcPath(void);

private:
	TCHAR m_tszInstallDir[MAX_PATH];
	TCHAR m_tszCurrentDir[MAX_PATH];
	TCHAR m_tMasterZFilePath[MAX_PATH];
	TCHAR m_tszKeyWordProcPath[MAX_PATH];
	TCHAR m_tszMcProcPath[MAX_PATH];
};

