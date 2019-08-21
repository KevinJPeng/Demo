/******************************************************
  客户端在线升级模块代码
*******************************************************/

#ifndef _UPDATE_OPERATE_H_
#define _UPDATE_OPERATE_H_
#include "..\..\threadmodel\ithreadunit.h"
#include "define.h"

class CUpdateOperate : public IThreadUnit
{
public:
	CUpdateOperate(void);
	~CUpdateOperate(void);

	//线程消息处理
	DWORD DispatchMessage(T_Message *pMsg);

	//升级消息处理
	DWORD OnHandleUpdateMsg(T_Message *pMsg);

	//升级成功校验
	DWORD OnHandleSuccessCheck(T_Message *pMsg);

	//取消升级处理
	DWORD OnHandleCancelUpdate(T_Message *pMsg);

	//下载安装包消息
	DWORD OnHandleDownLoadInstallExe(T_Message *pMsg);

	//下载安装包且静默安装
	DWORD OnDownLoadExeAndSlientInstall(T_Message *pMsg);

	//安全退出
	DWORD OnHandleSafeExit(T_Message *pMsg);

private:
	//开始升级接口
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);	

	//下载安装包文件接口
	static DWORD WINAPI DownLoadInstallExe(LPVOID lpParameter);	

	//下载静默推送安装包接口
	static DWORD WINAPI DownLoadSilentInstallExe(LPVOID lpParameter);

	//初始化相关变量，不能放在构造函数中，如果重复几次操作会出现问题
	void InitalParam();	

	//获取本地存放的升级配置文件信息
	BOOL ReadUpdateOLConfig(void);	

	//获取校验失败次数
	void ReadCheckUpdateFailCount();	

	//写入升级校验次数到配置文件中
	void WriteCheckUpdateFailCount();

	//选择http服务器
	BOOL ChooseHttpServer(const CString& strAddr);	

	//选取可用的升级服务器，做升级服务器切换
	BOOL SelAvailableHost(const vector<CString>& vecHost,DWORD strHostPort,const CString& strAddr);  

	//unicode CString转多字节
	char* CStringToMutilChar(CString str,int& chLength);

	//判断URL是否可用
	BOOL IsUrlAvailable(const CString& strURL);

	//获取服务端的更新信息
	bool GetNewUpdateInfo(CString strXMLData, T_UPDATE_INFO *Info);	

	//获取需要所有需要进行下载的文件大小及URL列表
	BOOL GetDownLoadFileList(T_UPDATE_INFO &newInfo,std::vector<CString>&vecUrl,std::vector<CString>&vecTempPath);	

	//将简写目录转换为实际的目录
	CString GetDirectory(CString strShortDir);	

	//判断是否为系统目录
	bool IsSysFileAndExist(CString szTarDir, T_FILE_ITEM &fileItem);

	//取md5值
	BOOL GetFileMD5(const CString &strFile, CString &strMd5);	

	//开始下载文件
	BOOL StartDownLoadFile(BOOL& b_IsCancel);	

	//下载安装包文件-----20141124
	BOOL DownLoadFileByURL(CString& strUrl);

	//下载静默安装包
	BOOL DownLoadFileByURL(CString& strUrl,CString& strFileName);

	//是否需要下载服务Deamon.exe
	BOOL IsNeedUpdateServers();

	//是否需要下载服务AutoRunService.exe/测试程序AutomaticSearch.exe
	void IsNeedUpdateClickPro(BOOL &bNeedUpdateService, BOOL &bNeedUpdateSerchPro);

	//生成GUID
	CString GetGuidCode();
private:
	//执行更新之前的操作
	bool RunUpdateCommand(T_UPDATE_INFO &newInfo, CString strRunFlag);	

	//删除文件
	void DelFiles(CString strFolder);		

	//超时等待进程退出（单位ms)
	bool WaitProcEnd(TCHAR *pProgName, DWORD dwTimeOut = INFINITE,
					 bool bForceEnd = false);	

	//停止相应的进程准备升级 
	void PrepareForCopyFiles();		

	//判断IECtrl是否可以复制
	bool CheckIEctrlCopy();												
	void SaveFileListToLocal(CString strData);
	bool MyCopyFile(CString strSrc, CString strDest, BOOL bFailIfExists);

	//删除Temp目录 0:删除 1:不删除
	BOOL DeleteDownLoadDir(WORD type = 0);	

	//处理用户重启客户端消息
	void HanleRebootMsg();												
	void PackagMessage(DWORD dwDestThread,DWORD dwSourceThread,DWORD dwMessageType,WPARAM wParam, LPARAM lParam,BOOL bIsSync = FALSE);
	
	//add by zhumingxing 20141119获取filelist_new文件信息用来进行升级成功校验
	bool GetCheckFileInfo(T_UPDATE_INFO *pInfo,vector<CString>&vec_ExcludeCheckFile);
	BOOL CheckUpdateSuccess();
	BOOL IsFileExcludeCRC(CString szTarDir, T_FILE_ITEM &fileItem);
	
	//判断某个文件路径是否存在，存在返回其MD5值
	BOOL GetTempFileMd5(const CString& strTempFilePath,CString& strMd5);
	
	//新增更新本地配置文件功能
	CString GetAPIURL();	
	BOOL UpdateConfigToLocal();
	int GetConfigData(T_DATA_FROM_SERVER &tData, CString strUrl);
	bool WriteConfigStr(T_DATA_FROM_SERVER &tConfigData);

	//宽字节CString转Char*
	BYTE toHex(const BYTE &x);
	char* CStringToMutilChar(CString& str,int& chLength,WORD wPage=CP_ACP);
	CString  URLEncode(CString sIn);
	//URLDecode---UTF-8解码:要换解码方式只需要改变Page页
	CString Utf8ToStringT(LPSTR str);
	CString UrlDecode(LPCTSTR url);
	CStdString DecodeString( CString& strDest);
	BOOL GetDownLoadEncodeData(CString& strData,CString strFileName);

	CString GetUserRequestVersion();
	//是否匹配到filelist中配置的不升级版本
	BOOL IsMatchLimitEquelVersion(CString& strLocalVersion);				
	BOOL GetTuiSongFromStr(const CString& strTuiSong);

private:
	DELAY_MESSAGE m_tPostMessage;					 //强制升级结构体，安装包下载完成之后返回给主界面
	PRODUCT_SETTING m_tPtdSetting;	
	CString m_szHttpServer;
	T_UPDATE_INFO m_newUpdate;						 //服务器返回的列表信息
	CIniFile m_iniFile;						         //操作ini文件
	IXMLRW   m_XmlCfg;
	vector<CString>m_vec_ExcludeCheckFile;			 //排除校验列表
	CString strSerResponse;
	ZipEncode m_zip;
	std::vector<CString> m_vecDownLoadUrl;			 //需要下载的URL列表
	std::vector<CString> m_vecTempPath;				 //下载文件存放的临时目录
	std::vector<CString>m_vecmd5;					 //存放服务器上面需要下载的文件的MD5值，之后可以与下载之后的文件的MD5值进行对比是否加入finish	
	DWORD m_dwProgeress;							 //已经下载文件大小进度
	CString m_strbatFilePath;						 //bat文件的路径
	HANDLE m_thread;								 //升级线程句柄
	HANDLE m_theadDownLoad;						//下载安装包进程
	HANDLE m_threadSilent;							 //下载静默安装包线程句柄

	BOOL m_bIsCancelDownLoad;						 //取消下载线程
	BOOL m_bIsCancelSilentInstall;					 //是否取消静默推送安装包下载
	CFile m_file;
	WORD m_wUpdateFlag;								 //记录当前升级类型
	bool m_bIsCancelUpdate;							 //是否取消升级
	T_Message *m_tMsg;	
	WORD m_wRbootType;								//重启类型
	int m_dwUpdateFailCount;						 //用户校验失败统计
	CString m_strEncodedata;						 //存放客户端发送过来的加密字符串
	T_TUISONG m_tTuiSong;							 //对推送信息进行处理
};
#endif