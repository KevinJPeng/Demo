#ifndef _UTILITY_H_
#define _UTILITY_H_
#include "CurlAPI.h"
#include "StdString.h"

//CStdString WideToChar(const wchar_t *pWide, DWORD dwCode = CP_ACP);
class CUtility
{
public:
	CUtility(IObserver* _obs);
	~CUtility();
	int DLAndInstallClient();
	bool clientAlreadyExists(CStdString _sClientName);
	int startClient(CStdString _sClientName);
	CStdString SelectLocalPath(CStdString _strShortDir);
	TCHAR* GetProgPath(void);
	CStdString SelectServer();
	bool InstallClient(CStdString _sLocalPath);
	void SetParentWnd(HWND hWnd);
	string GetUpdateServerUrl(string _serverAddr);
	int UpgradeSelf(CStdString& _sLocalPath, string _pRecv);
	bool SplitCString(const string & input, const string & delimiter, std::vector<string >& results, bool includeEmpties);
	bool SetServerInfo(string pRecv);
	BOOL CloseProcess(const char* pProcessName);
	char* wchar2char(const wchar_t* wchar);

private:
	typedef struct _serverInfo 
	{
		string sClientVersion;	//客户端版本号
		string sClientMD5;		//客户端MD5
		string sAPKMD5;			//安装包MD5
		_serverInfo()
		{
			sClientVersion = "";
			sClientMD5 = "";
			sAPKMD5 = "";
		}
	}serverInfo;
public:
private:
	IObserver* m_pobs;
	CCurlAPI m_Curl;
	HWND m_hParentWnd;
	serverInfo m_serverInfo;
};

#endif