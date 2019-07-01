#ifndef _CURLAPI_H_
#define _CURLAPI_H_
#include "curl/curl.h"
#include <list>
#include "observer.h"
#include "subject.h"
using namespace std;
#ifdef _DEBUG
#   ifdef _UNICODE
#       pragma comment(lib, "libcurld.lib")
#   else
#       pragma comment(lib, "libcurld.lib")
#   endif
#else
#   ifdef _UNICODE
#       pragma comment(lib, "libcurl.lib")
#   else
#       pragma comment(lib, "libcurl.lib")
#   endif
#endif

extern int g_iProgressLine;


class CCurlAPI :public ISubject
{
public:
	CCurlAPI();
	~CCurlAPI();
	int DLFileByHttp(const char* _sRemotePath, const char* _sLocalPath);
	int GetDataByHttp(const char* _sUrl, string& _sRecvOut);

	virtual void Attach(IObserver *);
	virtual void Detach(IObserver *);
	virtual void Notify();

	//工作线程
	static DWORD WINAPI ThreadNotify(LPVOID lpParameter);
	void EventNotify();

protected:
private:
	bool m_bDLOver;
	list<IObserver *> m_observers;  // 观察者列表
};

#endif