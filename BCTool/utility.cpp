#include "stdafx.h"
#include "utility.h"
#include "Directory.h"
#include "FileOperation.h"
#include "CurlAPI.h"
#include "UserDef.h"
#include <shlobj.h>
#include <shellapi.h>
#include "StringUtils.h"
#include "base64.h"
#include "md5.h"
#include<cstdio>
// #include <windows.h>
#include <Tlhelp32.h>
// #include <stdio.h>
//#include <winnt.h>

class fileTest
{
public:
	fileTest(){ _file = NULL; }
	~fileTest(){ close(); }
	inline bool isOpen(){ return _file != NULL; }
	inline long open(const char *path, const char * mod)
	{
		if (_file != NULL){ fclose(_file); _file = NULL; }
		_file = fopen(path, mod);
		if (_file)
		{
			long tel = 0;
			long cur = ftell(_file);
			fseek(_file, 0L, SEEK_END);
			tel = ftell(_file);
			fseek(_file, cur, SEEK_SET);
			return tel;
		}
		return -1;
	}
	inline void clean(int index, int len)
	{
#if !defined(__APPLE__) && !defined(WIN32) 
		if (_file != NULL)
		{
			int fd = fileno(_file);
			fsync(fd);
			posix_fadvise(fd, index, len, POSIX_FADV_DONTNEED);
			fsync(fd);
		}
#endif
	}
	inline void close()
	{
		if (_file != NULL){ clean(0, 0); fclose(_file); _file = NULL; }
	}
	inline void write(const char * data, size_t len)
	{
		if (_file && len > 0)
		{
			if (fwrite(data, 1, len, _file) != len)
			{
				close();
			}
		}
	}
	inline void flush(){ if (_file) fflush(_file); }

	inline std::string readLine()
	{
		char buf[500] = { 0 };
		if (_file && fgets(buf, 500, _file) != NULL)
		{
			return std::string(buf);
		}
		return std::string();
	}
	inline const std::string readContent();
	inline bool removeFile(const std::string & path) { return ::remove(path.c_str()) == 0; }
public:
	FILE *_file;
};

CUtility::CUtility()
{
}
CUtility::~CUtility()
{

}

string WideToChar_string(const wchar_t *pWide, DWORD dwCode = CP_ACP)
{
	string strChar = "";
	char *pChar = NULL;
	int  iWlen = 0;

	if (pWide == NULL
		|| (iWlen = wcslen(pWide)) == 0)
	{
		return pChar;
	}

	int iLen = WideCharToMultiByte(dwCode, 0, pWide, iWlen, NULL, NULL, NULL, NULL);
	if (iLen > 0)
	{
		pChar = new char[iLen + 1];
		if (pChar != NULL)
		{
			memset(pChar, 0, iLen + 1);
			WideCharToMultiByte(dwCode, 0, pWide, iWlen, pChar, iLen, NULL, NULL);
		}

		strChar = pChar;
	}

	if (pChar != NULL)
	{
		delete[] pChar;
		pChar = NULL;
	}

	return strChar;
}

CStdStringA WideToChar(const wchar_t *pWide, DWORD dwCode = CP_ACP)
{
	CStdStringA strChar = "";
	char *pChar = NULL;
	int  iWlen = 0;

	if (pWide == NULL
		|| (iWlen = wcslen(pWide)) == 0)
	{
		return pChar;
	}

	int iLen = WideCharToMultiByte(dwCode, 0, pWide, iWlen, NULL, NULL, NULL, NULL);
	if (iLen > 0)
	{
		pChar = new char[iLen + 1];
		if (pChar != NULL)
		{
			memset(pChar, 0, iLen + 1);
			WideCharToMultiByte(dwCode, 0, pWide, iWlen, pChar, iLen, NULL, NULL);
		}

		strChar = pChar;
	}

	if (pChar != NULL)
	{
		delete[] pChar;
		pChar = NULL;
	}

	return strChar;
}
string CUtility::GetUpdateServerUrl(string _serverAddr)
{
	CStdString sCurVersion = g_sVersion;
	//进行一次异或操作
	TCHAR ch = _T('①');
	for (int i = 0; i < sCurVersion.GetLength(); ++i)
	{
		DWORD Temp = sCurVersion[i];
		Temp = Temp^ch;
		sCurVersion.SetAt(i, Temp);
	}
	string sEncode = WideToChar_string(sCurVersion, CP_UTF8);
	std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(sEncode.c_str()), sEncode.length());

	char buf[500] = { 0 };
	sprintf(buf, "%s?msg=%s", _serverAddr.c_str(), encoded.c_str());

	string sBuf = buf;
	return sBuf;
}

int CUtility::UpgradeSelf(CStdString& _sLocalPath, string _pRecv)
{
	CCurlAPI curlAPIObj;

	CStdString sRecvData = _pRecv;

	std::vector<CStdString> vecVersionS;
	StringUtils StringUtil;
	StringUtil.SplitString((CStdString)sRecvData, _T("."), vecVersionS, true);

	int ivLenS = vecVersionS.size();
	if (ivLenS == 3)
	{
		int iFirstS = _ttoi(vecVersionS[0]);
		int iSecondS = _ttoi(vecVersionS[1]);
		int iThreeS = _ttoi(vecVersionS[2]);

		std::vector<CStdString> vecVersionC;
		StringUtils StringUtil;
		StringUtil.SplitString((CStdString)g_sVersion, _T("."), vecVersionC, true);
		int ivLenC = vecVersionC.size();
		if (3 == ivLenC)
		{
			int iFirstC = _ttoi(vecVersionC[0]);
			int iSecondC = _ttoi(vecVersionC[1]);
			int iThreeC = _ttoi(vecVersionC[2]);

			//第一位表示产品号，应该相同
			if (iFirstS == iFirstC)
			{
				//服务器的版本高于当前版本
				if ((iSecondS > iSecondC) || (iSecondS == iSecondC && iThreeS > iThreeC))
				{
					//获取当前程序路径（exe的路径，包含文件名）
					TCHAR szFullPath[MAX_PATH];
					memset(szFullPath, 0, MAX_PATH);
					::GetModuleFileName(NULL, szFullPath, MAX_PATH);

					//文件下载路径
					if (_sLocalPath.IsEmpty())
					{
						_sLocalPath = SelectLocalPath(_T("%temp%"));
					}
					CStdString sLocalPathName = _sLocalPath + _T("\\BCTool.exe");
					CStdString sServerUrl = g_sUpdateSelfUrl; //SelectServer();
					if (sServerUrl.IsEmpty())
					{
						return CLIENT_SELECTSERVERFAIL;
					}

					//下载程序自身
					if (0 == curlAPIObj.DLFileByHttp(WideToChar(sServerUrl.c_str()), WideToChar(sLocalPathName.c_str())))
					{
						CFileOperation fileOpeObj;

						//判断程序是否存在
						if (fileOpeObj.IsPathExistEx(sLocalPathName))
						{
							string sMD5 = md5file(WideToChar(sLocalPathName.c_str()));
							if (sMD5 == m_serverInfo.sClientMD5)
							{
								char buf[500] = { 0 };
								int iLen = 0;
								CStdString sLocalBatName = _sLocalPath + _T("\\killProcessTemp_BCTool.bat");

								fileTest    file;        //file handle.
								file.open(WideToChar(sLocalBatName.c_str()), "wb+");

								sprintf(buf, "taskkill /F /IM BCTool.exe\r\n");
								iLen = strlen(buf);
								file.write(buf, iLen);

								memset(buf, 0, 500);
								sprintf(buf, "xcopy /Y \"%s\" \"%s\"\r\n", WideToChar(sLocalPathName.c_str()).c_str(), WideToChar(szFullPath).c_str());
								iLen = strlen(buf);
								file.write(buf, iLen);

								memset(buf, 0, 500);
								sprintf(buf, "start \"\" \"%s\"\r\n", WideToChar(szFullPath).c_str());
								iLen = strlen(buf);
								file.write(buf, iLen);

								file.flush();
								file.close();


								//判断批处理文件是否存在
								if (fileOpeObj.IsPathExistEx(sLocalBatName))
								{
									LOG_INFO(g_utilityVar.loggerId, "发送升级消息给主窗口");
									SendMessage(m_hParentWnd, 991, 0, (LPARAM)&sLocalBatName);
									return 0;
								}
							}
							else
							{
								LOG_INFO(g_utilityVar.loggerId, "MD5比对失败，下载的文件与服务器上versioncfg.xml的MD5不一致");
							}
						}
					}
					else
					{
						LOG_INFO(g_utilityVar.loggerId, "软件升级失败");
					}
				}
			}
		}
	}

	return 1;
}
bool CUtility::SplitCString(const string & input, const string & delimiter, std::vector<string >& results, bool includeEmpties)
{
	int iPos = 0;
	int newPos = -1;
	int sizeS2 = (int)delimiter.size();
	int isize = (int)input.size();

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

		if (/*includeEmpties || */(s.size() > 0))
		{
			results.push_back(s);
		}
	}
	return true;
}
bool CUtility::SetServerInfo(string pRecv)
{
	std::vector<string> vecVersionS;
	SplitCString(pRecv, "&", vecVersionS, false);
	if (3 == vecVersionS.size())
	{
		m_serverInfo.sClientVersion = vecVersionS[0];
		m_serverInfo.sClientMD5 = vecVersionS[1];
		m_serverInfo.sAPKMD5 = vecVersionS[2];
	}
	else
	{
		LOG_INFO(g_utilityVar.loggerId, "服务器信息解析失败");
	}

	if (m_serverInfo.sClientVersion.empty() || m_serverInfo.sClientMD5.empty() || m_serverInfo.sAPKMD5.empty())
	{
		return false;
	}
	return true;
}
char* CUtility::wchar2char(const wchar_t* wchar)
{
	char * pChar;
	int len = WideCharToMultiByte(CP_ACP, 0, wchar, wcslen(wchar), NULL, 0, NULL, NULL);
	pChar = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, wchar, wcslen(wchar), pChar, len, NULL, NULL);
	pChar[len] = '\0';
	return pChar;
}

BOOL CUtility::CloseProcess(const char* pProcessName)
{
	int i = 0;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	//获取进程快照句柄
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	vector<DWORD> vHandle;
	vHandle.clear();
	while (bMore)
	{
		char* pcExeName = wchar2char(pe32.szExeFile);
		//stricmp不区分大小写  
		if (_stricmp(pProcessName, pcExeName) == 0)
		{
			//通过进程ID获取进程句柄
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, vHandle[i]);
			if (hProcess)
			{
				//通过进程句柄关闭进程
				::TerminateProcess(hProcess, NULL);
				CloseHandle(hProcess);
			}
		}
		delete[] pcExeName;
		pcExeName = NULL;
		bMore = ::Process32Next(hProcessSnap, &pe32);
	}
	CloseHandle(hProcessSnap);
	return TRUE;
}

int CUtility::DLAndInstallClient()
{
	//选择下载位置（文件临时保存路径）
	CStdString sLocalPath = _T("");

	//检测软件是否需要升级
	CCurlAPI curlAPIObj;

	string pRecv;
	curlAPIObj.GetDataByHttp(GetUpdateServerUrl(g_sServerUrl).c_str(), pRecv);

	LOG_INFO(g_utilityVar.loggerId, "当前版本:" << WideToChar(g_sVersion) << "&" << "服务器信息:" << pRecv);

	if (SetServerInfo(pRecv))
	{
		int iErrorCode = UpgradeSelf(sLocalPath, pRecv);
	}

	CDirectory directory;
	CStdString sClientName = directory.GetClientFilePath();
	//判断客户端是否存在
	if (clientAlreadyExists(sClientName))
	{
		LOG_INFO(g_utilityVar.loggerId, "检测到用户已安装客户端");
		//启动客户端
		int iErrorCode = startClient(sClientName);
		LOG_INFO(g_utilityVar.loggerId, "启动客户端,Code = " << iErrorCode);
		return iErrorCode;
	}

	//选择下载位置
	if (sLocalPath.IsEmpty())
	{
		sLocalPath = SelectLocalPath(_T("%temp%"));
	}
	CStdString sLocalPathName = sLocalPath + _T("\\MasterZ_Custom_93.exe");


	//判断下载位置是否有安装包
	bool bDL_APK = true;
	CFileOperation fileOpeObj;
	if (fileOpeObj.IsPathExistEx(sLocalPathName))
	{
		LOG_TRACE(g_utilityVar.loggerId, "检测到安装包已存在：" << WideToChar(sLocalPathName.c_str()));
		//先关闭进程（进程若在运行中，下载会失败）
		CloseProcess("MasterZ_Custom_93.exe");
		string sMD5 = md5file(WideToChar(sLocalPathName.c_str()));
		if (sMD5 != m_serverInfo.sAPKMD5)
		{
			LOG_TRACE(g_utilityVar.loggerId, "本地安装包和服务器安装包MD5不一致：LocalMD5:" << sMD5 << "_ServerMD5:" << m_serverInfo.sAPKMD5);
			//删除已有安装包
			remove(WideToChar(sLocalPathName.c_str()));
		}
		else
		{
			LOG_TRACE(g_utilityVar.loggerId, "本地安装包和服务器安装包MD5一致,直接安装");
			bDL_APK = false;
		}
	}

	if (bDL_APK)
	{
		//选择服务器
		CStdString sServerUrl = SelectServer();
		if (sServerUrl.IsEmpty())
		{
			return CLIENT_SELECTSERVERFAIL;
		}

		//下载客户端安装包
		//char* pRemotePath = "http://112.74.102.50:8075/MasterZ_Custom_93.exe";
		CCurlAPI curlAPIObj;
		//curlAPIObj.Attach(m_pobs);
		if (0 != curlAPIObj.DLFileByHttp(WideToChar(sServerUrl.c_str()), WideToChar(sLocalPathName.c_str())))
		{
			//curlAPIObj.Detach(m_pobs);
			LOG_INFO(g_utilityVar.loggerId, "安装包下载失败");
			return CLIENT_DL_FAIL;
		}
		LOG_INFO(g_utilityVar.loggerId, "安装包下载成功");
		//curlAPIObj.Detach(m_pobs);
	}

	//安装客户端
	if (!InstallClient(sLocalPathName))
	{
		LOG_INFO(g_utilityVar.loggerId, "安装包安装失败");
		return CLIENT_INSTALL_FAIL;
	} 

	LOG_INFO(g_utilityVar.loggerId, "安装包安装成功");
	return CLIENT_INSTALL_SUCCESS;
}

bool CUtility::clientAlreadyExists(CStdString _sClientName)
{
	CStdString sLowerClientName = _sClientName;
	sLowerClientName.MakeLower();
	if (!sLowerClientName.IsEmpty() && (-1 != sLowerClientName.Find(_T("masterz"))))
	{
		CFileOperation fileOpeObj;
		if (fileOpeObj.IsPathExistEx(_sClientName) || fileOpeObj.IsPathExistEx(sLowerClientName))
		{
			return TRUE;
		}
	}
	return FALSE;
}
int CUtility::startClient(CStdString _sClientName)
{
	//判断MasterZ客户端是否已启动
	HANDLE hMutex = CreateMutex(NULL, FALSE, _T("_MASTERZ_CLIENT_GUI_"));
	if (hMutex)
	{
		if (ERROR_ALREADY_EXISTS != GetLastError())
		{
			ReleaseMutex(hMutex);
			CloseHandle(hMutex);
			//			HANDLE                hProcess;             //执行任务的进程句柄
			TCHAR strDir[MAX_PATH] = { _T('\0') };
			TCHAR strExePath[MAX_PATH] = { 0 };
			_tcscpy_s(strDir, _sClientName);
			TCHAR *pch = _tcsrchr(strDir, _T('\\'));
			if (pch == NULL)
			{
				return CLIENT_START_FAIL;
			}
			*pch = 0;
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			si.dwFlags = STARTF_FORCEOFFFEEDBACK;

			if (CreateProcess(_sClientName, 0, NULL,
				NULL, FALSE, 0, NULL, strDir, &si, &pi) == TRUE)
			{
// 				if (NULL != phProcess)
// 					*phProcess = pi.hProcess;
				return CLIENT_START_SUCCESS;
			}
			return CLIENT_START_FAIL;
		}
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return CLIENT_CLIENT_RUN;
	}
	return CLIENT_OPEN_MUTEX_FAIL;
}

CStdString CUtility::SelectLocalPath(CStdString _strShortDir)
{
	CStdString strRet = _T("");
	CStdString strVar = _T("");
	CStdString strSingleFlag = _T("");
	CStdString strTotalFlag = _T("");      //整个通配符，如#SETUP#

	while (true)
	{
		int iFirstFlagPos = _strShortDir.Find(_T('#'));
		int iEndFlagPos = 0;
		if (-1 != iFirstFlagPos)
		{
			iEndFlagPos = _strShortDir.Find(_T('#'), iFirstFlagPos + 1);
			strVar = _strShortDir.Mid(iFirstFlagPos + 1, iEndFlagPos - iFirstFlagPos - 1);
			strSingleFlag = _T("#");
			strTotalFlag = _strShortDir.Mid(iFirstFlagPos, iEndFlagPos - iFirstFlagPos + 1);
		}

		//找不到#XXX#时再找%XXX%
		if (-1 == iFirstFlagPos || -1 == iEndFlagPos)
		{
			iFirstFlagPos = _strShortDir.Find(_T('%'));
			if (-1 != iFirstFlagPos)
			{
				iEndFlagPos = _strShortDir.Find(_T('%'), iFirstFlagPos + 1);
				strVar = _strShortDir.Mid(iFirstFlagPos + 1, iEndFlagPos - iFirstFlagPos - 1);
				strSingleFlag = _T("%");
				strTotalFlag = _strShortDir.Mid(iFirstFlagPos, iEndFlagPos - iFirstFlagPos + 1);
			}
		}

		//找不到通配符原串返回
		if (-1 == iFirstFlagPos || -1 == iEndFlagPos || strVar.IsEmpty())
			return _strShortDir;

		if (strSingleFlag == _T("#"))
		{
			if (!strVar.CompareNoCase(_T("SETUP")))
			{
				strRet = GetProgPath();
			}
			else if (!strVar.CompareNoCase(_T("NULL")))
			{
				strRet = _T("");
			}
		}
		else if (strSingleFlag == _T("%"))
		{
			if (!GetEnvironmentVariable(strVar, strRet.GetBuffer(MAX_PATH), MAX_PATH))
			{
				//MessageBox(_T("未找到环境变量！"));
				//g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("未找到环境变量 %s,目录未替换！"), strTotalFlag);
			}
			strRet.ReleaseBuffer();
		}


		//将替换后的目录加上目录变量之后的路径
		_strShortDir.Replace(strTotalFlag, strRet);
	}
}
TCHAR* CUtility::GetProgPath(void)
{
	static TCHAR s_szProgPath[MAX_PATH] = { 0 };
	SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 1, s_szProgPath);

	return s_szProgPath;
}

CStdString CUtility::SelectServer()
{
	return g_sAPKDLUrl;
}

bool CUtility::InstallClient(CStdString _sLocalPath)
{
	//静默安装
	HINSTANCE hNewExe = ShellExecute(NULL, _T("Open"), _sLocalPath, _T("/S"), NULL, SW_HIDE);
	if ((DWORD)hNewExe <= 32)
	{
		//g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("安装失败，错误码:%d！"), (DWORD)hNewExe);
		return false;
	}
	return true;
}
void CUtility::SetParentWnd(HWND hWnd)
{
	m_hParentWnd = hWnd;
}
