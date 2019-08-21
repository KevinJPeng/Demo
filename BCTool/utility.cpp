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
	//����һ��������
	TCHAR ch = _T('��');
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

			//��һλ��ʾ��Ʒ�ţ�Ӧ����ͬ
			if (iFirstS == iFirstC)
			{
				//�������İ汾���ڵ�ǰ�汾
				if ((iSecondS > iSecondC) || (iSecondS == iSecondC && iThreeS > iThreeC))
				{
					//��ȡ��ǰ����·����exe��·���������ļ�����
					TCHAR szFullPath[MAX_PATH];
					memset(szFullPath, 0, MAX_PATH);
					::GetModuleFileName(NULL, szFullPath, MAX_PATH);

					//�ļ�����·��
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

					//���س�������
					if (0 == curlAPIObj.DLFileByHttp(WideToChar(sServerUrl.c_str()), WideToChar(sLocalPathName.c_str())))
					{
						CFileOperation fileOpeObj;

						//�жϳ����Ƿ����
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


								//�ж��������ļ��Ƿ����
								if (fileOpeObj.IsPathExistEx(sLocalBatName))
								{
									LOG_INFO(g_utilityVar.loggerId, "����������Ϣ��������");
									SendMessage(m_hParentWnd, 991, 0, (LPARAM)&sLocalBatName);
									return 0;
								}
							}
							else
							{
								LOG_INFO(g_utilityVar.loggerId, "MD5�ȶ�ʧ�ܣ����ص��ļ����������versioncfg.xml��MD5��һ��");
							}
						}
					}
					else
					{
						LOG_INFO(g_utilityVar.loggerId, "�������ʧ��");
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
		LOG_INFO(g_utilityVar.loggerId, "��������Ϣ����ʧ��");
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
	//��ȡ���̿��վ��
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
		//stricmp�����ִ�Сд  
		if (_stricmp(pProcessName, pcExeName) == 0)
		{
			//ͨ������ID��ȡ���̾��
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, vHandle[i]);
			if (hProcess)
			{
				//ͨ�����̾���رս���
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
	//ѡ������λ�ã��ļ���ʱ����·����
	CStdString sLocalPath = _T("");

	//�������Ƿ���Ҫ����
	CCurlAPI curlAPIObj;

	string pRecv;
	curlAPIObj.GetDataByHttp(GetUpdateServerUrl(g_sServerUrl).c_str(), pRecv);

	LOG_INFO(g_utilityVar.loggerId, "��ǰ�汾:" << WideToChar(g_sVersion) << "&" << "��������Ϣ:" << pRecv);

	if (SetServerInfo(pRecv))
	{
		int iErrorCode = UpgradeSelf(sLocalPath, pRecv);
	}

	CDirectory directory;
	CStdString sClientName = directory.GetClientFilePath();
	//�жϿͻ����Ƿ����
	if (clientAlreadyExists(sClientName))
	{
		LOG_INFO(g_utilityVar.loggerId, "��⵽�û��Ѱ�װ�ͻ���");
		//�����ͻ���
		int iErrorCode = startClient(sClientName);
		LOG_INFO(g_utilityVar.loggerId, "�����ͻ���,Code = " << iErrorCode);
		return iErrorCode;
	}

	//ѡ������λ��
	if (sLocalPath.IsEmpty())
	{
		sLocalPath = SelectLocalPath(_T("%temp%"));
	}
	CStdString sLocalPathName = sLocalPath + _T("\\MasterZ_Custom_93.exe");


	//�ж�����λ���Ƿ��а�װ��
	bool bDL_APK = true;
	CFileOperation fileOpeObj;
	if (fileOpeObj.IsPathExistEx(sLocalPathName))
	{
		LOG_TRACE(g_utilityVar.loggerId, "��⵽��װ���Ѵ��ڣ�" << WideToChar(sLocalPathName.c_str()));
		//�ȹرս��̣��������������У����ػ�ʧ�ܣ�
		CloseProcess("MasterZ_Custom_93.exe");
		string sMD5 = md5file(WideToChar(sLocalPathName.c_str()));
		if (sMD5 != m_serverInfo.sAPKMD5)
		{
			LOG_TRACE(g_utilityVar.loggerId, "���ذ�װ���ͷ�������װ��MD5��һ�£�LocalMD5:" << sMD5 << "_ServerMD5:" << m_serverInfo.sAPKMD5);
			//ɾ�����а�װ��
			remove(WideToChar(sLocalPathName.c_str()));
		}
		else
		{
			LOG_TRACE(g_utilityVar.loggerId, "���ذ�װ���ͷ�������װ��MD5һ��,ֱ�Ӱ�װ");
			bDL_APK = false;
		}
	}

	if (bDL_APK)
	{
		//ѡ�������
		CStdString sServerUrl = SelectServer();
		if (sServerUrl.IsEmpty())
		{
			return CLIENT_SELECTSERVERFAIL;
		}

		//���ؿͻ��˰�װ��
		//char* pRemotePath = "http://112.74.102.50:8075/MasterZ_Custom_93.exe";
		CCurlAPI curlAPIObj;
		//curlAPIObj.Attach(m_pobs);
		if (0 != curlAPIObj.DLFileByHttp(WideToChar(sServerUrl.c_str()), WideToChar(sLocalPathName.c_str())))
		{
			//curlAPIObj.Detach(m_pobs);
			LOG_INFO(g_utilityVar.loggerId, "��װ������ʧ��");
			return CLIENT_DL_FAIL;
		}
		LOG_INFO(g_utilityVar.loggerId, "��װ�����سɹ�");
		//curlAPIObj.Detach(m_pobs);
	}

	//��װ�ͻ���
	if (!InstallClient(sLocalPathName))
	{
		LOG_INFO(g_utilityVar.loggerId, "��װ����װʧ��");
		return CLIENT_INSTALL_FAIL;
	} 

	LOG_INFO(g_utilityVar.loggerId, "��װ����װ�ɹ�");
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
	//�ж�MasterZ�ͻ����Ƿ�������
	HANDLE hMutex = CreateMutex(NULL, FALSE, _T("_MASTERZ_CLIENT_GUI_"));
	if (hMutex)
	{
		if (ERROR_ALREADY_EXISTS != GetLastError())
		{
			ReleaseMutex(hMutex);
			CloseHandle(hMutex);
			//			HANDLE                hProcess;             //ִ������Ľ��̾��
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
	CStdString strTotalFlag = _T("");      //����ͨ�������#SETUP#

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

		//�Ҳ���#XXX#ʱ����%XXX%
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

		//�Ҳ���ͨ���ԭ������
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
				//MessageBox(_T("δ�ҵ�����������"));
				//g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("δ�ҵ��������� %s,Ŀ¼δ�滻��"), strTotalFlag);
			}
			strRet.ReleaseBuffer();
		}


		//���滻���Ŀ¼����Ŀ¼����֮���·��
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
	//��Ĭ��װ
	HINSTANCE hNewExe = ShellExecute(NULL, _T("Open"), _sLocalPath, _T("/S"), NULL, SW_HIDE);
	if ((DWORD)hNewExe <= 32)
	{
		//g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("��װʧ�ܣ�������:%d��"), (DWORD)hNewExe);
		return false;
	}
	return true;
}
void CUtility::SetParentWnd(HWND hWnd)
{
	m_hParentWnd = hWnd;
}
