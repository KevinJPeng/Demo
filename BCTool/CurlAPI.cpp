#include "StdAfx.h"
#include "CurlAPI.h"

int g_iProgressLine = 0;

size_t my_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fwrite(ptr, size, nmemb, stream);
}

int my_progress_func(char *progress_data,
	double t, /* dltotal */
	double d, /* dlnow */
	double ultotal,
	double ulnow)
{
	g_iProgressLine = d*100.0 / t;
	printf("%s %g / %g (%g %%)\n", progress_data, d, t, d*100.0 / t);
	return 0;
}

CCurlAPI::CCurlAPI()
{
	m_bDLOver = false;
	curl_global_init(CURL_GLOBAL_ALL);
}
CCurlAPI::~CCurlAPI()
{
	curl_global_cleanup();
}

int CCurlAPI::DLFileByHttp(const char* _sRemotePath, const char* _sLocalPath)
{
	//char *url = "http://112.74.102.50:8075/MasterZ_93.exe";
	char *progress_data = "* ";

	if (NULL == _sRemotePath || NULL == _sLocalPath)
	{
		return 1000;
	}

	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return 1001;
	}
	HANDLE hThread = NULL;
	try
	{
		FILE* pOutfile = fopen(_sLocalPath, "wb");
		if (NULL == pOutfile)
		{
			curl_easy_cleanup(curl);
			return 1002;
		}
		curl_easy_setopt(curl, CURLOPT_URL, _sRemotePath);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pOutfile);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, my_progress_func);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data);
		g_iProgressLine = 0;
//		hThread = CreateThread(NULL, 0, ThreadNotify, this, 0, NULL);
		CURLcode res = curl_easy_perform(curl);
		if (CURLE_OK != res)
		{
			m_bDLOver = true;
			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			fclose(pOutfile);
			curl_easy_cleanup(curl);
//			WaitForSingleObject(hThread, INFINITE); // 等待，直到线程被激发
			return 1003;
		}
		fclose(pOutfile);
		curl_easy_cleanup(curl);
	}
	catch (...)
	{
		curl_easy_cleanup(curl);
	}
	m_bDLOver = true;
	WaitForSingleObject(hThread, INFINITE); // 等待，直到线程被激发
	return 0;
}

DWORD WINAPI CCurlAPI::ThreadNotify(LPVOID lpParameter)
{
	CCurlAPI* pThis = (CCurlAPI*)lpParameter;
	pThis->EventNotify();
	return 0;
}
void CCurlAPI::EventNotify()
{
// 	while (!m_bDLOver)
// 	{
// 		//WaitForSingleObject();
// 		Sleep(50);
// 		Notify();
// 	}
}

size_t recvData(char* _data, size_t _size, size_t _nmemb, string* writedata)
{
	size_t sizes = _size*_nmemb;
	if (NULL == _data)
	{
		return 0;
	}
	writedata->append(_data);
	return sizes;
}

int CCurlAPI::GetDataByHttp(const char* _sUrl, string& _sRecvOut)
{
	CURL* curl;
	//string RevBuff;
	char errbuf[CURL_ERROR_SIZE] = { 0 };
	curl = curl_easy_init();//初始化curl库

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, _sUrl); //设置url
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf); //设置错误缓冲
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1); //设置GET, POST方式
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.1 (KHTML, like Gecko) Chrome/21.0.1180.89 Safari/537.1 SumTest"); //设置代理


		//设置接收数据的缓冲区，此时一定要设置回调函数来对数据进行处理
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&_sRecvOut);
		//设置回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, recvData);

		CURLcode ret = curl_easy_perform(curl);//执行

		//将返回码设置给retcode，比如404错误之类的l
		long lErrorCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &lErrorCode);

		curl_easy_cleanup(curl);
	}
	//_sRecvOut = RevBuff.c_str();
	return 0;
}

