#pragma once
#include "..\..\threadmodel\ithreadunit.h"
#include "..\..\common\ICommCallback.h"
#include "CommClient.h"
#include "..\..\common\FileReadAndSave.h"

#include "UtilityThread.h"
class CUtilityThread;

#define BStrAlloc(AA) SysAllocString(AA);
#define BStrFree(AA) if(AA!=NULL){SysFreeString(AA);AA=NULL;}

class CKeyword :
	public ICommCallback
{
public:
	CKeyword(CUtilityThread *pUtility);
	~CKeyword(void);

	//初始化变量
	void initVariable(void);

	//另起的线程入口
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);

	//打开主控并与其建立连接
	void OpenAndConnectProc(void);

	//将处理好的数据发给主控
	void SendCombineDataToProc(CString strData);

	//处理收到的结果数据
	void OnReceive(CString strData);

	//将处理的消息或者失败的消息发给ui,flag表示获取数据成功或失败或完成
	void ReturnDataToUI(const CString &strData, DWORD flag);

	//处理ui发过来的数据
	void HandleData(CString strKeyword, CString strCompany, CString strProvince, 
							CString strCity, CString strBrand, CString strTexture);

	//数据的具体处理
	void DataFormat(CString strKeyword);

	//合并最终发给主控的数据
	CString CombineData();

	//加密发给主控的数据
	CString EncryptData(CString strCombineData);


	//将多字节转化为unicode
	//wstring ANSIToUnicode(const string& str);

	//将unicode转化为多字节
	//string UnicodeToANSI(const wstring& str);

	//将关键词进行编码转化为url
	//CString KeywordToUrl(CString strKeyword);

	CString GetUTF8String(IN CString sour);

	//关键词url转换utf-8
	BOOL ConvertUrlToUTF8(IN BSTR sour,OUT BSTR *dest);

	//转化为utf8多字节
	INT ToMultiByteUTF8(IN BSTR pszSource,OUT PCHAR *pszOut);
	
	//转化特殊字符
	BSTR FindTranChar(WCHAR Param);

	void KeywordAnalysis(const CString &strMsg);
	void SafeExit(void);
	void CancelKeywordAnalysis(void);

private:
	CCommClient m_comm;
	CString m_strKeyword;
	CString m_strCompany;
	CString m_strProvince;
	CString m_strCity;
	CString m_strBrand;
	CString m_strTexture;
	std::vector<CString> m_vKeyword;
	//CZBase64 m_base64;
	CFileReadAndSave m_encryptCode;
	bool m_bIsCompleted;  //表示关键词搜索已经完成
	bool m_bFlat;   //表示部分搜索完成标记
	bool m_bCancel;  //取消标记
	HANDLE m_thread;	//线程句柄
	HANDLE m_hNewExe;   //打开的SearchKeyword句柄
	CString m_strMsg;  //保存ui发过来的消息
	
	CUtilityThread *m_pUtility;
};

