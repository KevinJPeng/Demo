#include "StdAfx.h"
#include "ClearFiles.h"
#include "..\..\common\Reg.h"

#include "atltime.h"
#include <algorithm>


CClearFiles::CClearFiles(void)
{
	InitVariable();
}


CClearFiles::~CClearFiles(void)
{
}

void CClearFiles::ClearQuickPhotos(void)
{
	InitVariable();
	m_threadClearFile = CreateThread(NULL, 0, &CClearFiles::ThreadProcClearFile, this, 0, NULL);
}

void CClearFiles::ClearCancel(void)
{
	m_bIsCancel = true;
	//TerminateThread(m_thread,0);
	WaitForSingleObject(m_threadClearFile, INFINITE);
	CloseHandle(m_threadClearFile);
	g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("取消清理快照！"));
}

/*
void CClearFiles::ClearCodeImg(void)
{
	m_threadClearCode = CreateThread(NULL, 0, &CClearFiles::ThreadProcClearCode, this, 0, NULL);
}*/

void CClearFiles::InitVariable(void)
{
	m_bFlag = false;
	m_bIsCancel = false;
	m_nFileOfYear = 0;
	m_nFileOfMonth = 0;
	m_nFileOfDay = 0;
	m_nFileOfHour = 0;
	m_nFileOfMinute = 0;
	m_nThisYear = 0;
	m_nThisMonth = 0;
	m_nToday = 0;
	m_nThisHour = 0;
	m_nThisMinute = 0;
	m_nInitYear = 0;
	m_nInitMonth = 0;
	m_nInitDay = 0;
	m_SizeOfFile = 0;
	m_iDelDmp = 0;
	m_listDir.clear();
	m_vFileNameOfYearMonDay.clear();
	m_vHowMinutesOfFileToNow.clear();
	m_threadClearFile = NULL;
	m_threadClearCode = NULL;
	m_nLogSaveDays = 60;
}


DWORD WINAPI CClearFiles::ThreadProcClearFile(LPVOID lpParameter)
{
	CClearFiles* pThis = (CClearFiles*)lpParameter;

 	TCHAR szPath[MAX_PATH] = {0};

	PathAppend(szPath, g_pGlobalData->dir.GetInstallDir());
	PathAppend(szPath, _T("\\image\\result"));

	pThis->m_SizeOfFile = 0;
	//清理结果数据；
	pThis->RemoveDirByTime(szPath);

	//清理验证码和图片数据数据；
	pThis->ClearCode();

	return 0;
}


/*
DWORD WINAPI CClearFiles::ThreadProcClearCode(LPVOID lpParameter)
{
	CClearFiles* pThis = (CClearFiles*)lpParameter;
	if (!pThis->ClearCode())
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("清理code失败！"));
		return -1;
	}
	return 0;
}
*/


bool CClearFiles::ClearCode(void)
{
	TCHAR szPath[MAX_PATH] = {0};   //验证码图片路径
	TCHAR szPicPath[MAX_PATH] = { 0 }; //用户产品图片路径
	TCHAR szDmpPath[MAX_PATH] = { 0 };  //dmp路径
	TCHAR szErrCodePath[MAX_PATH] = { 0 };

	PathAppend(szPath, g_pGlobalData->dir.GetInstallDir());
	PathAppend(szPath, _T("\\image\\code"));

	PathAppend(szErrCodePath, g_pGlobalData->dir.GetInstallDir());
	PathAppend(szErrCodePath, _T("\\image\\errorcode"));

	PathAppend(szPicPath, g_pGlobalData->dir.GetInstallDir());
	PathAppend(szPicPath, _T("\\image\\user"));	

	PathAppend(szDmpPath, g_pGlobalData->dir.GetInstallDir());
	PathAppend(szDmpPath, _T("\\bin"));

	DelOutDateCodeImg();	//清除过期验证码图片

	GetCurrentTime();
	if (ReadMcconfigIni())
	{
		if (m_nInitYear == m_nThisYear && m_nInitMonth == m_nThisMonth && m_nInitDay == m_nToday)
		{
			return true;
		}
		else
		{
			if (!DeleteDirectory(szPath, 1) || !DeleteDirectory(szPicPath,1)
				||!DeleteDirectory(szErrCodePath,0))
			{
				//删除的文件夹为空也在删除文件失败之中
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("删除验证码或用户图片中的文件失败！"));
				return false;
			}

			if (m_iDelDmp != 1)
			{
				//删除dmp
				DeleteDirectory(szDmpPath, 1, _T("\\*.dmp"));
			}
			
		}
	}
	else
	{
		if (!DeleteDirectory(szPath, 1) || !DeleteDirectory(szPicPath, 1)
			|| !DeleteDirectory(szErrCodePath, 0))
		{
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("删除验证码或用户图片中中的文件失败！"));
			return false;
		}

		if (m_iDelDmp != 1)
		{
			//删除dmp
			DeleteDirectory(szDmpPath, 1, _T("\\*.dmp"));
		}
	}

	//清除日志压缩包
	DelOutDateLogZip();

	if (!writeMcconfigIni())
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("将清理code时间写入mcconfig文件失败！"));
	}

	return true;
}


void CClearFiles::GetCurrentTime()
{
	SYSTEMTIME tm;
	GetLocalTime(&tm);          //得到当前本地时间

	m_nThisYear = tm.wYear;
	m_nThisMonth = tm.wMonth;
	m_nToday = tm.wDay;
	m_nThisHour = tm.wHour;
	m_nThisMinute = tm.wMinute;
}


bool CClearFiles::ReadMcconfigIni()
{
/*
	TCHAR szCfgINI[MAX_PATH] = {0};

	_sntprintf(szCfgINI, _TRUNCATE, _T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());
	m_iniFile.init(szCfgINI);

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("mc.dat:%s！"), szCfgINI);

	m_iniFile.ReadInt(_T("MC"),_T("CLEARCODETIME"), _T("year"), m_nInitYear, -1);
	m_iniFile.ReadInt(_T("MC"), _T("CLEARCODETIME"), _T("month"), m_nInitMonth, -1);
	m_iniFile.ReadInt(_T("MC"), _T("CLEARCODETIME"), _T("day"), m_nInitDay, -1);
	m_iniFile.ReadInt(_T("MC"), _T("CLEARZIPLOG"), _T("Days"), m_nLogSaveDays, 60);*/

	CString strCfgFile = _T("");

	strCfgFile.Format(_T("%s\\data2\\version.dat"), g_pGlobalData->dir.GetInstallDir());

	CIniFile iniFile;
	iniFile.SetFilePath((CStdString)strCfgFile);

	iniFile.ReadInteger(_T("CLEARCODETIME"), _T("year"), m_nInitYear, -1);
	iniFile.ReadInteger(_T("CLEARCODETIME"), _T("month"), m_nInitMonth, -1);
	iniFile.ReadInteger(_T("CLEARCODETIME"), _T("day"), m_nInitDay, -1);
	iniFile.ReadInteger(_T("CLEARZIPLOG"), _T("Days"), m_nLogSaveDays, 60);

	if (-1 == m_nInitYear || -1 == m_nInitMonth || -1 == m_nInitDay)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("配置文件未读到配置时间！"));
		return false;	
	}

	CReg reg;

	//获取最后修改时间，默认为空
	TCHAR *pYunType = (TCHAR*)reg.ReadValueOfKey(HKEY_CURRENT_USER, _T("Software\\szw\\MasterZ"), _T("Yuntask"));
	if (pYunType != NULL)	
	{
		m_iDelDmp = _ttoi(pYunType);
	}
	
	return true;
}


bool CClearFiles::writeMcconfigIni(void)
{
/*
	TCHAR szCfgINI[MAX_PATH] = {0};

	_sntprintf(szCfgINI, _TRUNCATE, _T("%s\\data2\\mc.dat"), g_pGlobalData->dir.GetInstallDir());
	m_iniFile.init(szCfgINI);

	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("mc.dat:%s！"), szCfgINI);

	m_iniFile.WriteInt(_T("MC"),_T("CLEARCODETIME"), _T("year"), m_nThisYear);
	m_iniFile.WriteInt(_T("MC"),_T("CLEARCODETIME"), _T("month"), m_nThisMonth);
	m_iniFile.WriteInt(_T("MC"),_T("CLEARCODETIME"), _T("day"), m_nToday);*/

	CString strCfgFile = _T("");

	strCfgFile.Format(_T("%s\\data2\\version.dat"),g_pGlobalData->dir.GetInstallDir());

	CIniFile iniFile;
	iniFile.SetFilePath((CStdString)strCfgFile);

	iniFile.WriteInteger(_T("CLEARCODETIME"), _T("year"), m_nThisYear);
	iniFile.WriteInteger(_T("CLEARCODETIME"), _T("month"), m_nThisMonth);
	iniFile.WriteInteger(_T("CLEARCODETIME"), _T("day"), m_nToday);

	return true;
}


bool CClearFiles::CheckValidPath(LPTSTR pDir, CString &strNewPath)
{
	CString strPath = pDir;

	if (pDir == NULL)	          //判断指针是否为空
	{
		return false;
	}

	if (!PathIsDirectory(pDir))   //判断是否为有效路径
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("清理快照的路径为无效路径！Dir：%s"), pDir);
		return false;
	}

	if (strPath.Right(1) == _T("\\")) //判断文件路径是否以反斜杠结尾
	{
		strNewPath = strPath.Left(strPath.GetLength()-1);
	}
	else 
	{
		strNewPath = strPath;
	}
	return true;
}


void CClearFiles::GetFolderTime(CString strGetItem)
{
	m_nFileOfYear = _ttoi((strGetItem.Left(4)).GetBuffer());    //获取文件名整数年

	m_nFileOfMonth = _ttoi((strGetItem.Mid(4, 2)).GetBuffer());		//获取文件名整数月

	m_nFileOfDay = _ttoi((strGetItem.Mid(6, 2)).GetBuffer());		//获取文件名整数天

	m_nFileOfHour = _ttoi((strGetItem.Mid(8, 2)).GetBuffer());        //获取文件名小时数

	m_nFileOfMinute = _ttoi((strGetItem.Right(2)).GetBuffer());        //获取文件名分钟数
}


void CClearFiles::DayOfYear()
{
	int leap, leap1, k, j;
	int a[2][13] = {{0,31,28,31,30,31,30,31,31,30,31,30,31}, {0,31,29,31,30,31,30,31,31,30,31,30,31}};
	leap = ((m_nFileOfYear%4 == 0) && (m_nFileOfYear%100 != 0)) || (m_nFileOfYear%400 == 0);  //判断是否是闰年

	for (k = 1; k < m_nFileOfMonth; ++k)
	{
		m_nFileOfDay +=a[leap][k];   //表示各文件是今年第几天
	}

	leap1 = ((m_nThisYear%4 == 0) && (m_nThisYear%100 != 0)) || (m_nThisYear%400 == 0);

	SYSTEMTIME tm;
	GetLocalTime(&tm); 
	m_nToday = tm.wDay;
	for (j = 1; j < m_nThisMonth; ++j)
	{
		m_nToday += a[leap1][j];    //表示今天是今年第几天
	}
}


void CClearFiles::JudgeWhichFolderToDelete(CString strNewDir)
{
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("文件装进容器的个数！Count: %d"), m_vHowMinutesOfFileToNow.size());
	if (m_bIsCancel)
	{
		return;
	}
	if (m_vHowMinutesOfFileToNow.size() > 5)
	{
		vector<int> vDays;
		vector<OperSelect>::iterator iter;
		//vDays = m_vHowMinutesOfFileToNow.m_nLaterDay;
		for (iter = m_vHowMinutesOfFileToNow.begin(); iter != m_vHowMinutesOfFileToNow.end(); ++iter)
		{
			if (m_bIsCancel)
			{
				return;
			}
			vDays.push_back(iter->m_nHowMinuteOfFileToNow);
		}
		make_heap(vDays.begin(), vDays.end());
		sort_heap(vDays.begin(), vDays.end());

		int nFlat = vDays[4];  //要删除的文件夹和不删除文件夹的分界处
		for (iter = m_vHowMinutesOfFileToNow.begin(); iter != m_vHowMinutesOfFileToNow.end(); ++iter)
		{
			if (iter->m_nHowMinuteOfFileToNow > nFlat)
			{
				if (m_bIsCancel)
				{
					return;
				}
				AddFolderToDelete(iter->m_strItem, strNewDir);
			}
		}
		vDays.clear();
	}

	m_vHowMinutesOfFileToNow.clear();
}


void CClearFiles::AddFolderToDelete(CString pAddSzItem, CString strNewDirectory)
{
	CString strScr = strNewDirectory + _T("\\") + pAddSzItem;   //完整路径

	OperFlag dir;
	dir.m_strDirName = strScr;
	dir.m_bDelete = true;
	m_listDir.push_back (dir);
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("删除文件的最终路径！Path: %s"), strScr);
}


void CClearFiles::RemoveDir()
{
	if (m_bIsCancel)
	{
		return;
	}
	for (list<OperFlag>::iterator it = m_listDir.begin(); it != m_listDir.end(); ++it)
	{
		if (m_bIsCancel)
		{
			return;
		}
		if (it->m_bDelete)
		{
			if (m_bIsCancel)
			{
				return;
			}
			CString strName = it->m_strDirName;
			DeleteDirectory(strName);
		}
	}
}


bool CClearFiles::DeleteDirectory(CString strDir, int flag,CString strFileType) 
{ 
	if(strDir.IsEmpty()) 
		return true; 

	// 首先删除文件及子文件夹 
	CFileFind ff; 
	BOOL bFound = ff.FindFile(strDir + strFileType, 0); 
	while(bFound) 
	{ 
		if (m_bIsCancel && flag == 0)
		{
			return false;
		}
		bFound = ff.FindNextFile(); 
		if(ff.GetFileName() == _T(".") || ff.GetFileName()==_T("..")) 
		{
			continue;
		}
		// 去掉文件(夹)只读等属性 
		SetFileAttributes(ff.GetFilePath(), FILE_ATTRIBUTE_NORMAL); 
		if(ff.IsDirectory()) 
		{ 
			// 递归删除子文件夹 
			if (m_bIsCancel && flag == 0)
			{
				return false;
			}
			DeleteDirectory(ff.GetFilePath()); 
			RemoveDirectory(ff.GetFilePath()); 
		} 
		else 
		{ 
			// 删除文件 
			GetFileSize(ff.GetFilePath());
			DeleteFile(ff.GetFilePath()); 
		} 
	} 
	ff.Close(); 

	// 然后删除该文件夹 
	if (0 == flag)
	{
		RemoveDirectory(strDir);
	} 

	return true;
}


void CClearFiles::GetFileSize(CString Dir)
{
	CFileStatus status;
	CFile::GetStatus(Dir, status);
	m_SizeOfFile += status.m_size;
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("删除文件的大小！Size: %d"), m_SizeOfFile/(1024*1024));
}


int CClearFiles::GetDays(void)
{
	int nDays = 0;           //计算的是离今年第一天负多少天
	int nYear = m_nThisYear;
	while (nYear != m_nFileOfYear)
	{
		bool leap = ((((m_nThisYear-1)%4 == 0) && ((m_nThisYear-1)%100 != 0)) || ((m_nThisYear-1)%400 == 0));       //判断是否是闰年
		if (leap)
		{
			nDays -= 366;
		} 
		else
		{
			nDays -= 365;
		}

		--nYear;
	}

	int nFileToToday = 0;
	nFileToToday = m_nToday - m_nFileOfDay - nDays;   //判断文件夹离今天是多少天

	return nFileToToday;   
}


bool CClearFiles::RemoveDirByTime(LPTSTR pDir)
{
	CString strNewPath = _T("");
	GetCurrentTime();
	if(!CheckValidPath(pDir, strNewPath))
	{
		return false;
	}

	ToGetFileName(strNewPath, _T(""));
	if (m_bIsCancel)
	{
		return false;
	}

	vector<CString>::iterator iter;
	CString strFileNameOfYearMonDay = _T("");
	for (iter = m_vFileNameOfYearMonDay.begin(); iter != m_vFileNameOfYearMonDay.end(); ++iter)
	{
		strFileNameOfYearMonDay = *iter;
		ToGetFileName(strNewPath +_T("\\") + *iter, strFileNameOfYearMonDay);
		if (m_bIsCancel)
		{
			return false;
		}
	}

	JudgeWhichFolderToDelete(strNewPath);
	if (m_bIsCancel)
	{
		return false;
	}
	RemoveDir();
	if (m_bIsCancel)
	{
		return false;
	}
	//删除空文件夹
	for (iter = m_vFileNameOfYearMonDay.begin(); iter != m_vFileNameOfYearMonDay.end(); ++iter)
	{
		if (m_bIsCancel)
		{
			return false;
		}
		RemoveDirectory(strNewPath + _T("\\") + *iter);
	}
	
	m_listDir.clear();
	m_vFileNameOfYearMonDay.clear();

	return true;
}


void CClearFiles::ToGetFileName(CString strPath, CString strFileNameOfYearMonDay)
{
	CFileFind Find;   
	BOOL bFound;           //判断是否成功找到文件 
	bFound = Find.FindFile(strPath  + _T("\\*.*"));   //修改" "内内容给限定查找文件类型 

	while(bFound)      //遍历所有文件  
	{   
		if (m_bIsCancel)
		{
			return;
		}
		bFound = Find.FindNextFile();

		//如果找到的是返回上层的目录 则结束本次查找  
		if (Find.IsDots())
		{
			continue;
		}

		//找到的是文件夹，则遍历该文件夹下的文件
		if(Find.IsDirectory())     
		{   
			if (m_bIsCancel)
			{
				return;
			}
			//判断是不是第一次调用该函数
			if (m_bFlag == false)
			{
				if (m_bIsCancel)
				{
					return;
				}
				CString strFileName = Find.GetFileTitle();
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("现将年月日的文件夹遍历放进vector容器！FileName: %s"), strFileName);
				m_vFileNameOfYearMonDay.push_back(strFileName);
			}
			else
			{
				if (m_bIsCancel)
				{
					return;
				}
				int nHowDaysOfFileToToday = 0;
				int nMinutes = 0;
				CString strFileNameOfHourMin = Find.GetFileTitle();
				CString strCombineItem = strFileNameOfYearMonDay + strFileNameOfHourMin;
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("现将年月日时分的文件夹名组合起来！CombineFileName: %s"), strCombineItem);

				GetFolderTime(strCombineItem);
				if (m_bIsCancel)
				{
					return;
				}
				DayOfYear();      //判断文件或者今天分别是一年的第几天
				if (m_bIsCancel)
				{
					return;
				}
				nHowDaysOfFileToToday = GetDays();      //得到了文件夹离今天的确切天数
				nMinutes = nHowDaysOfFileToToday*24*60 + (m_nThisHour*60+m_nThisMinute) - (m_nFileOfHour*60+m_nFileOfMinute);  //计算实际的分钟数

				OperSelect sort;
				sort.m_nHowMinuteOfFileToNow = nMinutes;
				sort.m_strItem = strFileNameOfYearMonDay + _T("\\") + strFileNameOfHourMin;
				m_vHowMinutesOfFileToNow.push_back(sort);    //将各个文件夹离今天的分钟数放进容器	
				g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("将文件夹时间换算成离现在的分钟数，放进vector容器中！Minutes: %d"), nMinutes);
			}					
		}   
	}   
	Find.Close(); 
	m_bFlag = true;
}

//清理过期日志压缩包
bool CClearFiles::DelOutDateLogZip()
{
	CFileFind fFind;
	TCHAR szZipLogPath[MAX_PATH] = { 0 };  
	BOOL bFound;
	CTime tFileModifyTime;
	CTime tCurrTime;

	PathAppend(szZipLogPath, g_pGlobalData->dir.GetInstallDir());
	PathAppend(szZipLogPath, _T("..\\bak\\masterz\\*.*"));

	tCurrTime = CTime::GetCurrentTime();
	//当前时间转换；
	int iCurYear = tCurrTime.GetYear();
	int iCurMonth = tCurrTime.GetMonth();
	int iCurDay = tCurrTime.GetDay();

	bFound = fFind.FindFile(szZipLogPath); //查找所有文件
	while (bFound)
	{
		bFound = fFind.FindNextFile();
		//如果是..或者目录类型，则继续下一个
		if (fFind.IsDots() || fFind.IsDirectory())
		{
			continue;
		}
		//文件最后更新时间
		fFind.GetLastWriteTime(tFileModifyTime);
		int iFileYear = tFileModifyTime.GetYear();
		int iFileMonth = tFileModifyTime.GetMonth();
		int iFileDay = tFileModifyTime.GetDay();

		//时间间隔
		int iDiffDays = (iCurYear - iFileYear) * 360 + (iCurMonth - iFileMonth) * 30 + (iCurDay-iFileDay);

		if (iDiffDays > m_nLogSaveDays)
		{
			CString strDelName = fFind.GetFilePath();
			if (!DeleteFile(strDelName))
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("删除文件失败,%s"), strDelName);
			}
		}
	}

	fFind.Close();

	return true;
}

bool CClearFiles::DelOutDateCodeImg()
{
	CFileFind fFind;
	TCHAR szCodePath[MAX_PATH] = { 0 };
	BOOL bFound;
	CTime tFileModifyTime;
	CTime tCurrTime;

	PathAppend(szCodePath, g_pGlobalData->dir.GetInstallDir());
	PathAppend(szCodePath, _T("\\image\\result\\*.*"));

	tCurrTime = CTime::GetCurrentTime();
	//当前时间转换；
	int iCurYear = tCurrTime.GetYear();
	int iCurMonth = tCurrTime.GetMonth();
	int iCurDay = tCurrTime.GetDay();

	bFound = fFind.FindFile(szCodePath); //查找所有文件
	while (bFound)
	{
		bFound = fFind.FindNextFile();
		//如果是..或者目录类型，则继续下一个
		if (fFind.IsDots() || fFind.IsDirectory())
		{
			continue;
		}
		//文件创建时间
		fFind.GetLastWriteTime(tFileModifyTime);
		int iFileYear = tFileModifyTime.GetYear();
		int iFileMonth = tFileModifyTime.GetMonth();
		int iFileDay = tFileModifyTime.GetDay();

		//时间间隔
		int iDiffDays = (iCurYear - iFileYear) * 360 + (iCurMonth - iFileMonth) * 30 + (iCurDay - iFileDay);

		static int iSaveDays = 2;
		if (iDiffDays > iSaveDays)
		{
			CString strDelName = fFind.GetFilePath();
			if (!DeleteFile(strDelName))
			{
				g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("删除文件失败,%s"), strDelName);
			}
		}
	}

	fFind.Close();

	return true;
}

