#pragma once
#include <vector>
#include <map>
#define REG_USER_ROOT               HKEY_CURRENT_USER
#define REG_KEY_VERSION             _T("version")           

#ifndef _LIVE_UPDATE
#define _LIVE_UPDATE
#endif

#define IsHexNum(c) ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))  

struct T_FILE_ITEM
{
	CString szLocalName;    //本地文件名（相对路径，可包含目录）
	CString szMD5;
	CString szRemoteName;   //文件在服务器上的位置
	CString szOnlyAbsentUpdate;  //仅文件不存在时升级
};

struct T_EXEC_FILE_ITEM
{
	CString szFileName;
	CString szParam;
	CString szAction;     //kill:结束以szFileName为名称的进程  run:运行
	CString szRunTime;    //pre:升级前执行  post:升级后执行
	CString szWaitEnd;    //0:不等待  1：等待运行结束
};

struct T_EXEC_DIRECTORY
{
	CString szTargetDir;                       //命令文件所在目录
	std::vector<T_EXEC_FILE_ITEM> vExecItem;   //命令信息
};

typedef std::map<CString, T_FILE_ITEM> FILE_ITEM_MAP;   //文件项的MAP，文件名为Key
typedef std::map<CString, FILE_ITEM_MAP> DIR_ITEM_MAP;  //每个DIR对应的文件列表，目录名为Key

struct T_UPDATE_INFO
{
	CString szVersion;
	CString szCmpLessVersion;							//新增小于某个版本号不升级
	std::vector<CString>vCmpEquealVersion;				//等于某个版本

	std::map<CString, FILE_ITEM_MAP> mapDirList;     //文件列表的Map，目录名为Key
	std::vector<T_EXEC_FILE_ITEM> vRunList;          //依次执行的命令列表

	T_UPDATE_INFO()
	{
		szVersion = "";
		szCmpLessVersion = _T("");
		vCmpEquealVersion.clear();
		mapDirList.clear();
		vRunList.clear();
	}
	void clear()
	{
		szVersion = "";
		szCmpLessVersion = _T("");
		vCmpEquealVersion.clear();
		mapDirList.clear();
		vRunList.clear();
	}
};


struct PRODUCT_SETTING
{
	CString strName;					//产品名称
	CString strRegKey;				//产品注册路径
	int iUpdateSuccFlag;				//升级成功的产品标识 4表示新商务快车普通版 6表示旗舰版 8表示主控Dat文件 10表示六度
	CString strUpdateRequestPage;	//升级时请求的页面
	CString strUpdateDownLoadpage;	//下载页面
	CString strClientTool;			//客户端程序名
	CString strDlgToRecvMsg;			//接收升级结果的对话框名称

	PRODUCT_SETTING()
	{
		strName = _T("");
		strRegKey = _T("");
		iUpdateSuccFlag = 0;
		strUpdateRequestPage = _T("");
		strUpdateDownLoadpage = _T("");
		strClientTool = _T("");
		strDlgToRecvMsg = _T("");
	}
//添加清空函数，每次升级初始化参数时清空
	void clear()
	{
		strName = _T("");
		strRegKey = _T("");
		iUpdateSuccFlag = 0;
		strUpdateRequestPage = _T("");
		strClientTool = _T("");
		strDlgToRecvMsg = _T("");
	}
};

//add by zhumingxing 20141211---添加配置文件结构

struct T_FILE
{
	CString strFileName;
	CString strTargetDir;
	CString strVersion;
	CString strData;

	T_FILE()
	{
		strFileName = _T("");
		strTargetDir = _T("");
		strVersion   = _T("");
		strData = _T("");
	}
};

//从服务器返回的数据
struct T_DATA_FROM_SERVER
{	
	CString strVersion;           //版本号
	std::vector<T_FILE> vMcconfig;       //配置文件

	T_DATA_FROM_SERVER()
	{
		strVersion = _T("");
		vMcconfig.clear();
	}
};

//定义结构体将服务器返回的安装包下载信息封装
struct T_TUISONG
{
	CString m_strMd5;
	vector<CString>m_vecDownLoadUrl;

	T_TUISONG()
	{
		m_strMd5 = _T("");
		m_vecDownLoadUrl.clear();
	}
	void clear()
	{
		m_strMd5 = _T("");
		m_vecDownLoadUrl.clear();
	}
};

//end

//add byzhumingxing 20150721
inline int SplitStringWithSeparator(const CString &strSource, const CString &strSeparator, std::vector<CString> &strVector, BOOL bIsIncludeEmpty = FALSE)
{
	CString strSourceTemp = strSource;
	CString strTemp = _T("");
	strVector.clear();
	if (strSource == _T(""))
	{
		return 0;
	}

	DWORD dwSeparatorLen = strSeparator.GetLength();

	int nPostion = strSourceTemp.Find(strSeparator);
	if (nPostion == -1)
	{
		strVector.push_back(strSourceTemp);
		return strVector.size();
	}

	while (nPostion != -1)
	{
		strTemp = strSourceTemp.Left(nPostion);
		if (!strTemp.IsEmpty() || bIsIncludeEmpty)
		{
			strVector.push_back(strTemp);
		}
		strSourceTemp.Delete(0, strTemp.GetLength() + dwSeparatorLen);
		strTemp.Empty();

		nPostion = strSourceTemp.Find(strSeparator);
	}

	if (!strSourceTemp.IsEmpty() || bIsIncludeEmpty)
	{
		strVector.push_back(strSourceTemp);
	}


	return strVector.size();
}
//end

inline CString GetDirectory(CString strShortDir)
{
	CString strRet = _T("");
	CString strVar = _T("");
	CString strSingleFlag = _T("");
	CString strTotalFlag = _T("");      //整个通配符，如#SETUP#

FindFlag:
	int iFirstFlagPos = strShortDir.Find(_T('#'));
	int iEndFlagPos = 0;
	if (-1 != iFirstFlagPos)
	{
		iEndFlagPos = strShortDir.Find(_T('#'), iFirstFlagPos + 1);
		strVar = strShortDir.Mid(iFirstFlagPos + 1, iEndFlagPos - iFirstFlagPos - 1);
		strSingleFlag = _T("#");
		strTotalFlag = strShortDir.Mid(iFirstFlagPos, iEndFlagPos - iFirstFlagPos + 1);
	}

	//找不到#XXX#时再找%XXX%
	if (-1 == iFirstFlagPos || -1 == iEndFlagPos)
	{
		iFirstFlagPos = strShortDir.Find(_T('%'));
		if (-1 != iFirstFlagPos)
		{
			iEndFlagPos = strShortDir.Find(_T('%'), iFirstFlagPos + 1);
			strVar = strShortDir.Mid(iFirstFlagPos + 1, iEndFlagPos - iFirstFlagPos - 1);
			strSingleFlag = _T("%");
			strTotalFlag = strShortDir.Mid(iFirstFlagPos, iEndFlagPos - iFirstFlagPos + 1);
		}
	}

	//找不到通配符原串返回
	if (-1 == iFirstFlagPos || -1 == iEndFlagPos || strVar.IsEmpty())
		return strShortDir;

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
			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("未找到环境变量 %s,目录未替换！"), strTotalFlag);
		}
		strRet.ReleaseBuffer();
	}


	//将替换后的目录加上目录变量之后的路径
	strShortDir.Replace(strTotalFlag, strRet);
	goto FindFlag;           //继续查找下一个通配符
}

static bool ParseXML(TiXmlDocument *pDoc, T_UPDATE_INFO *pInfo)
{
	TiXmlHandle hDoc(pDoc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	hRoot = hDoc.FirstChildElement();

	pElem = hRoot.FirstChild("software").Element();
	if (!pElem) return false;

	pInfo->szVersion = pElem->Attribute("version");

	//add begin by zhumingxing 20150731 新增版本过滤规则解析
	pInfo->szCmpLessVersion = pElem->Attribute("limit_less");
	CString strTemp = CString(pElem->Attribute("limit_equal"));
	
	SplitStringWithSeparator(strTemp, _T("|"), pInfo->vCmpEquealVersion, TRUE);
	//end add

	//获取文件列表
	pElem = hRoot.FirstChild("filelist").FirstChild("directory").Element();
	if (!pElem) return false;

	for (; pElem != NULL; pElem = pElem->NextSiblingElement())
	{
		std::map<CString, T_FILE_ITEM> filemap;
		TiXmlElement *pFileItem = pElem->FirstChild("file")->ToElement();

		//遍历读取文件列表
		for (; pFileItem; pFileItem = pFileItem->NextSiblingElement())
		{
			T_FILE_ITEM fileItem;
			fileItem.szLocalName = pFileItem->Attribute("name");
			fileItem.szMD5 = pFileItem->Attribute("md5");
			fileItem.szOnlyAbsentUpdate = pFileItem->Attribute("onlyAbsentUpdate");
			fileItem.szRemoteName = pFileItem->Attribute("src");

			filemap[fileItem.szLocalName] = fileItem;
		}

		pInfo->mapDirList[CString(pElem->Attribute("value"))] = filemap;
	}

	//获取命令列表
	TiXmlElement *pFileItem = hRoot.FirstChild("runlist").FirstChild("file").Element();

	//遍历读取命令列表
	for (; pFileItem; pFileItem = pFileItem->NextSiblingElement())
	{
		T_EXEC_FILE_ITEM fileItem;
		fileItem.szFileName = pFileItem->Attribute("name");
		fileItem.szParam = pFileItem->Attribute("param");
		fileItem.szAction = pFileItem->Attribute("action");
		fileItem.szRunTime = pFileItem->Attribute("runtime");
		fileItem.szWaitEnd = pFileItem->Attribute("wait");

		pInfo->vRunList.push_back(fileItem);
	}


	return true;
}

static bool ParseNewXMLNew(TiXmlDocument *pDoc, T_UPDATE_INFO *pInfo,vector<CString>& vec_ExcludeCheckFile)
{
	TiXmlHandle hDoc(pDoc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	hRoot = hDoc.FirstChildElement();

	pElem = hRoot.FirstChild("software").Element();
	if (!pElem) return false;

	pInfo->szVersion = pElem->Attribute("version");

	//获取文件列表
	pElem = hRoot.FirstChild("filelist").FirstChild("directory").Element();
	if (!pElem) return false;

	for (; pElem != NULL; pElem = pElem->NextSiblingElement())
	{
		std::map<CString, T_FILE_ITEM> filemap;
		TiXmlElement *pFileItem = pElem->FirstChild("file")->ToElement();

		//遍历读取文件列表
		for (; pFileItem; pFileItem = pFileItem->NextSiblingElement())
		{
			T_FILE_ITEM fileItem;
			fileItem.szLocalName = pFileItem->Attribute("name");
			fileItem.szMD5 = pFileItem->Attribute("md5");
			fileItem.szOnlyAbsentUpdate = pFileItem->Attribute("onlyAbsentUpdate");
			fileItem.szRemoteName = pFileItem->Attribute("src");

			filemap[fileItem.szLocalName] = fileItem;
		}

		pInfo->mapDirList[CString(pElem->Attribute("value"))] = filemap;
	}

	//获取命令列表
	TiXmlElement *pFileItem = hRoot.FirstChild("runlist").FirstChild("file").Element();

	//遍历读取命令列表
	for (; pFileItem; pFileItem = pFileItem->NextSiblingElement())
	{
		T_EXEC_FILE_ITEM fileItem;
		fileItem.szFileName = pFileItem->Attribute("name");
		fileItem.szParam = pFileItem->Attribute("param");
		fileItem.szAction = pFileItem->Attribute("action");
		fileItem.szRunTime = pFileItem->Attribute("runtime");
		fileItem.szWaitEnd = pFileItem->Attribute("wait");

		pInfo->vRunList.push_back(fileItem);
	}

	//add by zhumingxing 20141119
	pFileItem = hRoot.FirstChild("excludeCRC").FirstChild("file").Element();
	//遍历读取命令列表
	for (; pFileItem; pFileItem = pFileItem->NextSiblingElement())
	{
		//T_EXEC_FILE_ITEM fileItem;
		CString strExcludeCheckFile = CString(pFileItem->Attribute("name"));
		//fileItem.szParam = pFileItem->Attribute("param");
		//fileItem.szAction = pFileItem->Attribute("action");
		//fileItem.szRunTime = pFileItem->Attribute("runtime");
		//fileItem.szWaitEnd = pFileItem->Attribute("wait");

		vec_ExcludeCheckFile.push_back(GetDirectory(strExcludeCheckFile));
	}

	return true;
}

static void BuildXMLFile(T_UPDATE_INFO &Info, CString xmlPath)
{
	TiXmlDocument doc;
	TiXmlElement *pRoot;
	CString strSerPath = _T("");

	DWORD dwSize = MAX_PATH;
	char szBuf[MAX_PATH] = {0};

	pRoot = new TiXmlElement("root");
	doc.LinkEndChild(pRoot);

	TiXmlElement *pVer = new TiXmlElement("software");
	dwSize = MAX_PATH;
	WCharToMByte(Info.szVersion, szBuf, &dwSize);
	pVer->SetAttribute("version", szBuf);
	pRoot->LinkEndChild(pVer);

	TiXmlElement *pFileList = new TiXmlElement("filelist");
	pRoot->LinkEndChild(pFileList);

	DIR_ITEM_MAP::iterator itDir = Info.mapDirList.begin();
	for (; itDir != Info.mapDirList.end(); ++itDir)
	{
		TiXmlElement *pDir = new TiXmlElement("directory");
		dwSize = MAX_PATH;
		WCharToMByte(itDir->first, szBuf, &dwSize);
		pDir->SetAttribute("value", szBuf);

		FILE_ITEM_MAP::iterator itFile = itDir->second.begin();
		for (; itFile != itDir->second.end(); ++itFile)
		{
			TiXmlElement *pFile = new TiXmlElement("file");

			dwSize = MAX_PATH;
			WCharToMByte(itFile->second.szLocalName, szBuf, &dwSize);
			pFile->SetAttribute("name", szBuf);

			dwSize = MAX_PATH;
			WCharToMByte(itFile->second.szMD5, szBuf, &dwSize);
			pFile->SetAttribute("md5", szBuf);

			if (itFile->second.szRemoteName.Left(1) != _T('/'))
			{
				strSerPath.Format(_T("/%s"), itFile->second.szRemoteName);
			}
			else
			{
				strSerPath = itFile->second.szRemoteName;
			}
			strSerPath.Replace(_T("\\"), _T("/"));

			dwSize = MAX_PATH;
			WCharToMByte(strSerPath, szBuf, &dwSize);
			pFile->SetAttribute("src", szBuf);

			dwSize = MAX_PATH;
			WCharToMByte(itFile->second.szOnlyAbsentUpdate, szBuf, &dwSize);
			pFile->SetAttribute("onlyAbsentUpdate", szBuf);


			pDir->LinkEndChild(pFile);
		}

		pFileList->LinkEndChild(pDir);
	}

	TiXmlElement *pRunList = new TiXmlElement("runlist");
	pRoot->LinkEndChild(pRunList);

	std::vector<T_EXEC_FILE_ITEM>::iterator itExecItem = Info.vRunList.begin();
	for(; itExecItem != Info.vRunList.end(); ++itExecItem)
	{
		TiXmlElement *pExecItem = new TiXmlElement("file");

		dwSize = MAX_PATH;
		WCharToMByte(itExecItem->szFileName, szBuf, &dwSize);
		pExecItem->SetAttribute("name", szBuf);

		dwSize = MAX_PATH;
		WCharToMByte(itExecItem->szParam, szBuf, &dwSize);
		pExecItem->SetAttribute("param", szBuf);

		dwSize = MAX_PATH;
		WCharToMByte(itExecItem->szAction, szBuf, &dwSize);
		pExecItem->SetAttribute("action", szBuf);

		dwSize = MAX_PATH;
		WCharToMByte(itExecItem->szRunTime, szBuf, &dwSize);
		pExecItem->SetAttribute("runtime", szBuf);

		dwSize = MAX_PATH;
		WCharToMByte(itExecItem->szWaitEnd, szBuf, &dwSize);
		pExecItem->SetAttribute("wait", szBuf);

		pRunList->LinkEndChild(pExecItem);
	}

	dwSize = MAX_PATH;
	WCharToMByte(xmlPath, szBuf, &dwSize);

	doc.SaveFile(szBuf);
}

/*
@brief 分解从服务端请求到的自动任务数据
@param pDoc 获取的配置信息xml文件的指针
@param [in out] tConfigInfo 从服务器获取的配置信息
@return 成功为true
*/
static bool ParseServerResponse(TiXmlDocument *pDoc, T_DATA_FROM_SERVER &tConfigInfo)
{
	if (!pDoc)
	{
		return false;
	}


	TiXmlElement *pRoot = pDoc->FirstChildElement("root");

	if (!pRoot)
	{
		return false;
	}

	TiXmlElement *pVersion = pRoot->FirstChildElement("info");
	if (pVersion)
	{
		//版本号
		tConfigInfo.strVersion = pVersion->Attribute("version");
	}
	else
	{	
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("未获得版本号！"));
		return false;
	}

	TiXmlElement *pFile = pRoot->FirstChildElement("file");
	if (pFile)
	{
		//遍历读取file
		for (; pFile; pFile = pFile->NextSiblingElement())
		{
			//T_SECTION section;
			T_FILE file;
			file.strFileName = pFile->Attribute("name");
			file.strTargetDir = pFile->Attribute("targetdir");
			file.strVersion = pFile->Attribute("version");

			TiXmlElement *pSection = pFile->FirstChildElement("data");
			if (pSection)
			{	
				file.strData = pSection->Attribute("value");
			}
			else
			{
				g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("未获得data！"));
				return false;
			}

			tConfigInfo.vMcconfig.push_back(file);
		}	
	}
	else
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__, __LINE__, _T("未获得file！"));
		return false;
	}

	return true;
}