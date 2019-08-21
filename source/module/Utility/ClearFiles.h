#pragma once

#include "..\..\threadmodel\ithreadunit.h"
#include <windows.h>
#include <list>
#include "../../common/IniFile.h"

typedef struct OPER_FLAG		//文件夹操作标记（删除/不删除）
{
	CString m_strDirName;			//文件夹完整路径
	bool m_bDelete;				//文件夹是否被删除

	OPER_FLAG()					//结构体初始化成员变量
	{
		m_strDirName = _T("");
		m_bDelete = false;
	}

} OperFlag;


typedef struct OPER_SELECT		//选择要删除的文件夹
{
	CString m_strItem;			//文件夹名称
	int m_nHowMinuteOfFileToNow;			//文件夹离今天的实际分钟数

	OPER_SELECT()					//结构体初始化成员变量
	{
		m_strItem = _T("");
		m_nHowMinuteOfFileToNow = 0;
	}

} OperSelect;


class CClearFiles 
{
public:
	CClearFiles(void);
	~CClearFiles(void);


public:
	void InitVariable(void);    //初始化变量
	bool RemoveDirByTime(LPTSTR pDir);
	void ToGetFileName(CString strPath, CString strFileNameOfYearMonDay);       //得到文件夹的时间
	void GetCurrentTime(void);                                    //得到当前时间
	void GetFolderTime(CString strGetItem);	                    //得到文件夹名的时间
	bool CheckValidPath(LPTSTR pDir, CString &szNewPath);	 //检验路径
	void DayOfYear();		         //表示一年第几天
	void JudgeWhichFolderToDelete(CString strNewDir);         //判断要删除的文件夹
	void AddFolderToDelete(CString pAddSzItem, CString strNewDirectory);        //添加要删除的文件夹
	void RemoveDir();                //移除文件夹
	bool DeleteDirectory(CString strDir, int flag = 0, CString FileType = _T("\\*.*"));   //递归删除
	void GetFileSize(CString Dir);        //获得文件大小
	int GetDays(void);      //获得离今天的天数
	bool ClearCode(void);      //删除推广时产生的code文件
	bool ReadMcconfigIni(void);  //读配置文件mcconfig获得上次程序退出的配置
	bool writeMcconfigIni(void);  //将本次清理code的时间写入mcconfig配置文件中
	bool DelOutDateLogZip(); //清理过期日志压缩包
	bool DelOutDateCodeImg(); //清理过期验证码图片（拖动验证码、截图）
	static DWORD WINAPI ThreadProcClearFile(LPVOID lpParameter);
	/*static DWORD WINAPI ThreadProcClearCode(LPVOID lpParameter);*/
	
	void ClearQuickPhotos(void);
	void ClearCancel(void);
/*	void ClearCodeImg(void);*/

private:
	list<OperFlag> m_listDir;				//用一个list容器来存放
	vector<OperSelect> m_vHowMinutesOfFileToNow;     //用来统计文件夹离今天的时间
	vector<CString> m_vFileNameOfYearMonDay;    //用来存放文件夹的名称
	HANDLE m_threadClearFile;     //清理快照的线程句柄
	HANDLE m_threadClearCode;     //清理code文件夹的线程句柄
	bool m_bIsCancel;    //判断是否取消
	IXMLRW m_iniFile;
	
private:
	//文件夹的时间
	int m_nFileOfYear;
	int m_nFileOfMonth;
	int m_nFileOfDay;
	int m_nFileOfHour;
	int m_nFileOfMinute;
	//本地现在时间
	int m_nThisYear;
	int m_nThisMonth;
	int m_nToday;
	int m_nThisHour;
	int m_nThisMinute;
	bool m_bFlag;      //判断是否是第一次调用ToGetFileName

	//配置文件的时间
	int m_nInitYear;
	int m_nInitMonth;
	int m_nInitDay;

	//配置日志压缩文件保存天数
	int m_nLogSaveDays;

	//获取是否要删除dmp文件
	int m_iDelDmp;

public:
	ULONGLONG m_SizeOfFile;
	
};

