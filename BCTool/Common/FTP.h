#pragma once
#include <afxinet.h>
#include <map>
#include <list>

//服务器配置
struct T_CONN_INFO
{
	CString strServer;
	int nPort;
	CString strUser;
	CString strPwd;
	BOOL bPassive;
};

class CFTP
{
public:
	CFTP(void);
	~CFTP(void);

	/*
	@brief 连接FTP服务器
	@param  ftp用户信息结构体
	*/
	BOOL Connect(const T_CONN_INFO &tInfo);

	/*
	@brief 从FTP下载文件到本地，此处为文件级的下载
	@param  strRFile  ftp文件名
	@param  strLFile  本地文件全路径
	@return  返回错误码 1表示下载成功其他表示失败
	*/
	BOOL GetFile(CString strRFile, CString strLFile);

	/*
	@brief 从本地上传文件到ftp服务器，此处为文件级的上传
	@param  strRFile	ftp文件全路径
	@param  strLFile  本地文件全路径
	@return  返回错误码 1表示上传成功其他表示失败
	*/
	BOOL UpLoadFile(CString strRFile,CString strLFile);

	/*
	@brief 从FTP下载文件到本地，此方法支持断点续传
	@param  strRFile  ftp文件名全路径
	@param  strLFile  本地文件全路径
	@param  IsNeedRest  True 表示需要断点续传 False表示不需要断点续传
	@param  dwRetryCount  失败之后的重试次数
	@return  True 表示下载成功 False表示下载失败
	*/
	BOOL GetFileByte(CString strRFile, CString strLFile,DWORD dwRetryCount = 3,BOOL IsNeedRest = TRUE);


	/*
	@brief 从本地上传文件到ftp服务器，此方法支持断点续传
	@param  strRFile	ftp文件全路径
	@param  strLFile  本地文件全路径
	@param  IsNeedRest  True 表示需要断点续传 False表示不需要断点续传
	@param  dwRetryCount  失败之后的重试次数
	@param  IsDelTempFile  上传文件之前是否要先删掉服务器上未上传完整的文件
	@return  True 表示上传成功 False表示上传失败
	*/
	BOOL UpLoadFileByte(CString strRFile,CString strLFile,DWORD dwRetryCount = 10,BOOL IsNeedRest = TRUE,BOOL IsDelTempFile = TRUE);

	/*
	@brief 断开与FTP连接
	*/
	void DisConnect() ;

	/*
	@brief 获取FTP文件大小及最后修改时间
	@param  strRFile  ftp文件全路径
	@param  strLastModefyTime ftp文件最后修改时间
	*/
	ULONGLONG GetServerFileLength(CString strRFile,CString&strLastModefyTime);

	/*
	@brief 判断FTP文件是否存在
	@param  strRFile  ftp文件全路径
	*/
	BOOL IsFtpFileExist(CString strRFile);

	/*
	@brief 断开当前连接重新连接ftp服务器
	*/
	BOOL ReConnect();

	/*
	@brief 断开当前连接重新连接ftp服务器
	*/
	BOOL IsConnect();

	/*
	@brief 获取出错信息
	*/
	CString GetErrorMessage();

	/*
	@brief 删除ftp服务器文件
	*/
	BOOL DeleteFtpFile(CString strRFilePath);

	/*
	@brief 重命名ftp服务器文件
	*/
	BOOL ReNameFtpFile(CString strOld,CString strNew);

	/*
	@brief 根据服务器返回的响应获取文件重置(REST的位置)
	*/
	ULONGLONG GetServerRestPos();

	/*
	@宽字节CString转Char*
	*/
	char* CStringToMutilChar(CString& str,int& chLength,WORD wPage=CP_ACP);

private:
	T_CONN_INFO m_tConnInfo;

	CInternetSession m_session;
	CFtpConnection *m_pConnect;
	CInternetFile * m_pRemoteFile;
	CString  m_strErrorMsg;
	ULONGLONG m_lRestPos;				//rest大小值
};

