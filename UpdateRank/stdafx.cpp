// stdafx.cpp : 只包括标准包含文件的源文件
// UpdateRank.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO:  在 STDAFX.H 中
// 引用任何所需的附加头文件，而不是在此文件中引用
CString	 g_strDebugUrl = _T("");
CStringA  g_strWeb;
std::map<CString, int> g_mapSearch;
std::map<int, _Elem_MatchFlag> g_mapElemCfg;
std::vector<pKeyWordDataInfo> g_vAllKeyWords;
std::vector<CString> g_vDiffCompany;
//IServerSocket *g_Socket = NULL;
bool g_bManualRefresh = false;

CServerSocket *g_server;
CLogTrace g_log(_T("UpdateRank2.log"), NULL);
CLogTrace g_pLog(_T("VerifyCodeInfo.log"), NULL);
sFingerRecogn g_sFingerRecogn;
//IXMLRW *g_DatMc = NULL;


volatile BOOL bSearchFlag[SEARCHFLAGCOUNT];

CRITICAL_SECTION critSection;
CRITICAL_SECTION critSendMsg;
CRITICAL_SECTION critSearchFlag;
CRITICAL_SECTION critCount;

int g_iBaidu = 0;
int g_iSogou = 0;
int g_iBing = 0;
int g_i360 = 0;
int g_iYouDao = 0;

int g_iDelayForKeyWord = -1;
int g_iDalayForSite = -1;
int g_iThreadCount = -1;
CString g_shttpOss = _T("");
CString g_sKWPath = _T("9999");
BOOL g_bIsDebug = false;

TCHAR tSpecialChar[HtmlCharNum][CHAR_LEN] = { _T("\t"), _T(" "), _T("+"), _T("/"), _T("?"), _T("%"), _T("#"), _T("&"), _T("="), _T("."), _T("\\"), _T(":"), _T("*"), _T("\""), _T("<"), _T(">"), _T("|") };
// 初始化map
void InitMap()
{
	g_mapSearch[_T("百度")] = SEARCH_BAIDU;
	g_mapSearch[_T("360搜索")] = SEARCH_360;
	g_mapSearch[_T("搜狗")] = SEARCH_SOGOU;
	g_mapSearch[_T("必应")] = SEARCH_BING;
	g_mapSearch[_T("有道")] = SEARCH_YOUDAO;
	g_mapSearch[_T("手机百度")] = SEARCH_PHONEBAIDU;
	g_mapSearch[_T("手机搜狗")] = SEARCH_PHONESOGOU;
	g_mapSearch[_T("手机360")] = SEARCH_PHONE360;
	g_mapSearch[_T("微信")] = SEARCH_WEIXIN;
	g_mapSearch[_T("手机神马")] = SEARCH_PHONESHENMA;
}

// 在keyword目录下创建公司名目录保存快照
void CreateCompanyDir(CString strDirName)
{
	CString strComName = strDirName;
	ReplaceHtmlChar(strComName);
	TCHAR szFile[MAX_PATH] = { 0 };
	CString strFile;
	::GetModuleFileName(NULL, szFile, MAX_PATH);
	PathAppend(szFile, _T("..\\..\\"));

	strFile = szFile;
	if (g_bIsDebug)
	{
		strFile.Append(_T("image\\"));
		strFile.Append(_T("DEBUG"));

		if (!::CreateDirectory(strFile.GetBuffer(), NULL))
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("创建DEBUG目录失败, 路径:%s, ERROR:%d"), strFile, GetLastError());
		}
		strFile.Append(_T("\\"));
	}
	else
	{
		if (g_bManualRefresh)
		{
			strFile.Append(_T("image\\keyword2\\"));
		}
		else
		{
			strFile.Append(_T("image\\keyword\\"));
		}
		strFile.Append(g_sKWPath);
		strFile.Append(_T("\\"));
	}
	strFile.Append(strComName);

	if (!::CreateDirectory(strFile.GetBuffer(), NULL))
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("创建目录失败, 路径:%s, ERROR:%d"), strFile, GetLastError());
	}

}
void CreateDateDirDir(CString strDirName)
{
	CString strDateName = strDirName;
	ReplaceHtmlChar(strDateName);
	TCHAR szFile[MAX_PATH] = { 0 };
	CString strFile;
	::GetModuleFileName(NULL, szFile, MAX_PATH);
	PathAppend(szFile, _T("..\\..\\"));

	strFile = szFile;
	if (g_bManualRefresh)
	{
		strFile.Append(_T("image\\keyword2\\"));
	}
	else
	{
		strFile.Append(_T("image\\keyword\\"));
	}
	strFile.Append(strDateName);

	if (!::CreateDirectory(strFile.GetBuffer(), NULL))
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("创建目录失败, 路径:%s, ERROR:%d"), strFile, GetLastError());
	}

}

// 获取CPU核心数
int GetCPUCoreNums()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}


// 保存不同用户公司名
void SaveCompanyName(CString strComName)
{
	size_t iSize = g_vDiffCompany.size();
	if (iSize == 0)
	{
		g_vDiffCompany.push_back(strComName);
		return;
	}
	for (int i = 0; i < iSize; i++)
	{
		Sleep(5);
		if (g_vDiffCompany[i] == strComName)
		{
			return;
		}
	}
	g_vDiffCompany.push_back(strComName);
}

// 解析所有关键词数据
void ParseAllKeyWordData(const CStdString &strWord)
{
	// 初始化
	InitMap();

	// 加载默认网站
	GetDefaultWeb();

	// 存储所有关键词数据信息
	std::vector<CStdString> vecKeyWordInfo;
	StringUtils StringUtil;

	g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("开始解释数据"));

	// 关键词1数据(;8) 关键词2数据(;8)…….(;8)关键词N数据
	// 解析出所有关键词信息存储在vecKeyWordInfo中
	StringUtil.SplitString(strWord, _T("(;8)"), vecKeyWordInfo, true);
	if (vecKeyWordInfo.size() == 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("没有数据"));
	}

	// 把所有关键词信息解析到全局变量g_vAllKeyWords中
	for (int iCount = 0; iCount < vecKeyWordInfo.size(); iCount++)
	{
		Sleep(5);
		if (!vecKeyWordInfo[iCount].IsEmpty())
		{
			ParseKeyWordData(vecKeyWordInfo[iCount]);
		}
	}

	CreateDateDirDir(g_sKWPath);
	// 创建不同公司名目录
	for (int i = 0; i < g_vDiffCompany.size(); i++)
	{
		Sleep(5);
		CreateCompanyDir(g_vDiffCompany[i]);
	}

	g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("不同公司名个数:%d"), g_vDiffCompany.size());

	g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("解析数据完成,收到%d条关键词数据"), vecKeyWordInfo.size());

	vecKeyWordInfo.clear();
}

// 解析每条具体的关键词信息
void ParseKeyWordData(const CStdString &strKeyWord)
{
	if (strKeyWord.Compare(_T("")) == 0)
	{
		return;
	}

	std::vector<CStdString> VecResult;
	StringUtils StringUtil;

	// 关键词数据:
	// 关键词信息(;9)分割符列表(;9)搜索引擎名称(;9)网站列表(;9)目标网站标记(;9)公司名(;3)公司简称(;9)官网列表(;9)发布标记前缀(;9)微信公众号名称(;9)是否特殊客户
	// 分解关键词信息
	StringUtil.SplitString(strKeyWord, _T("(;9)"), VecResult, true);

	int iCountInfo = VecResult.size();
	if (iCountInfo < 6)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("分解单条关键词数据失败，(;9)组数:%d"), iCountInfo);
		return;
	}


	// 根据分解的字符串填充关键词信息结构
	// 网站标志
	pKeyWordDataInfo pKeyWordInfo = new KeyWordDataInfo();
	pKeyWordInfo->strWebFlag = VecResult[4];
	if (pKeyWordInfo->strWebFlag.IsEmpty())
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("目标网站标记为空"));
		if (pKeyWordInfo != NULL)
		{
			delete pKeyWordInfo;
			pKeyWordInfo = NULL;
		}
		return;
	}

	//  	Sleep(8000);
	// // 	// 公司名
	// 	pKeyWordInfo->vAllCompanys.clear();
	// 	VecResult[5] = _T("商舟网科技有限公司");
	// 	VecResult[5] = _T("商舟网科技有限公司(;3)商舟网科技");
	// 	VecResult[5] = _T("商舟网科技有限公司(;3)商舟网科技|");
	// 	VecResult[5] = _T("商舟网科技有限公司(;3)商舟网科技|商舟网");
	// 	VecResult[5] = _T("商舟网科技有限公司(;3)");
	CStdString strCompanys;
	StringUtil.SplitString(VecResult[5], _T("(;3)"), pKeyWordInfo->vAllCompanys, false);
	std::vector<CStdString>  vShortCompanys;		//分解简称
	//	pKeyWordInfo->vAllCompanys[1] = _T("");
	if (pKeyWordInfo->vAllCompanys.size() > 1)
	{
		StringUtil.SplitString(pKeyWordInfo->vAllCompanys[1], _T("|"), vShortCompanys, false);
		if (vShortCompanys.size() > 0)
		{
			CStdString strMainCompanys = pKeyWordInfo->vAllCompanys[0];
			pKeyWordInfo->vAllCompanys.clear();
			pKeyWordInfo->vAllCompanys.push_back(strMainCompanys);
			for (int i = 0; i < vShortCompanys.size(); i++)
			{
				pKeyWordInfo->vAllCompanys.push_back(vShortCompanys[i]);

				//有多个公司简称
				CString sComName = vShortCompanys[i];
				CStdString sComTag;
				sComTag.Format(_T(",%s,"), sComName);
				pKeyWordInfo->vCompanysTag.push_back(sComTag);
				sComTag.Format(_T("，%s，"), sComName);
				pKeyWordInfo->vCompanysTag.push_back(sComTag);
				sComTag.Format(_T(",%s，"), sComName);
				pKeyWordInfo->vCompanysTag.push_back(sComTag);
				sComTag.Format(_T("，%s,"), sComName);
				pKeyWordInfo->vCompanysTag.push_back(sComTag);
			}
		}
		else
		{
			//只有一个公司简称
			CString sComName = pKeyWordInfo->vAllCompanys[1];
			CStdString sComTag;
			sComTag.Format(_T(",%s,"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T("，%s，"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T(",%s，"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T("，%s,"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
		}
	}
	else if (1 == pKeyWordInfo->vAllCompanys.size())
	{
		//没有公司简称，用公司全称
		//_T("商舟网科技有限公司(;3)");
		CString sComName = pKeyWordInfo->vAllCompanys[0];
		CStdString sComTag;
		sComTag.Format(_T(",%s,"), sComName);
		pKeyWordInfo->vCompanysTag.push_back(sComTag);
		sComTag.Format(_T("，%s，"), sComName);
		pKeyWordInfo->vCompanysTag.push_back(sComTag);
		sComTag.Format(_T(",%s，"), sComName);
		pKeyWordInfo->vCompanysTag.push_back(sComTag);
		sComTag.Format(_T("，%s,"), sComName);
		pKeyWordInfo->vCompanysTag.push_back(sComTag);
	}

	if (pKeyWordInfo->vAllCompanys.size() <= 0)
	{
		if (VecResult[5].GetLength() <= 0)
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("公司名和简称为空"));
			if (pKeyWordInfo != NULL)
			{
				delete pKeyWordInfo;
				pKeyWordInfo = NULL;
			}
			return;
		}
		else
		{
			pKeyWordInfo->vAllCompanys.push_back(VecResult[5]);

			//没有公司简称，用公司全称
			//_T("商舟网科技有限公司");
			CString sComName = VecResult[5];
			CStdString sComTag;
			sComTag.Format(_T(",%s,"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T("，%s，"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T(",%s，"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T("，%s,"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
		}
	}

	pKeyWordInfo->strComany = pKeyWordInfo->vAllCompanys[0];

	// 保存公司名
	CString strComName = pKeyWordInfo->strComany;
	SaveCompanyName(strComName);

	// 解析网站列表
	// 网站列表信息:
	// 网站1(;1) 网站2(;1) 网站3…(;1) 网站N
	std::vector<CStdString> vecWebList;
	const CStdString &strWebInfo = VecResult[3];
	const CStdString &strWebFlag = VecResult[4];
	const CStdString &strWebHost = VecResult[6];

	if (strWebFlag == _T("2"))  //仅网站列表
	{
		CStringA strList = WideToChar(strWebInfo.c_str());
		if (strList.GetLength() > 0)
		{
			pKeyWordInfo->strWebList = strList;
		}
		//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("仅网站,网站为:%s"), strWebInfo.c_str());
	}
	else if (strWebFlag == _T("1"))  //网站列表、加旗舰版网站
	{
		CStringA strList = WideToChar(strWebInfo.c_str());
		if (strList.GetLength() > 0)
		{
			pKeyWordInfo->strWebList = g_strWeb + strList;
		}

		//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("列表加网站,网站为:%s"), strWebInfo.c_str());
	}
	else if (strWebFlag == _T("0"))  //仅默认列表
	{
		pKeyWordInfo->strWebList = g_strWeb;
		//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("仅列表,网站为:%s"), strWebInfo.c_str());
	}
	else if ((strWebFlag == _T("3")) || (strWebFlag == _T("4")))  //增加官网和排除列表（3）  //从默认列表排除制定网站(4)
	{
		StringUtil.SplitString(strWebInfo, _T("(;1)"), vecWebList, false);
		if (vecWebList.size() <= 0)
		{
			if (strWebInfo.GetLength() > 0)
			{
				vecWebList.push_back(strWebInfo);
			}
		}

		if (vecWebList.size() > 0)
		{
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("排除指定网站个数：%d"), vecWebList.size());
		}

		CStringA strTmp = NULL;
		CStdString strLow;
		for (int i = 0; i < vecWebList.size(); i++)
		{
			//转换成小写
			strLow = vecWebList[i].ToLower();
			strTmp = WideToChar(strLow.c_str());
			g_strWeb.Replace(strTmp, "");
		}
		pKeyWordInfo->strWebList = g_strWeb;

		if (strWebFlag == _T("3"))
		{
			//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("收到的官网列表：%s"), strWebHost.c_str());
			vecWebList.clear();
			StringUtil.SplitString(strWebHost, _T("(;3)"), vecWebList, false);
			if (vecWebList.size() <= 0 && strWebHost.GetLength() > 1)
			{
				pKeyWordInfo->vOfficialList.push_back((CString)strWebHost);
			}
			for (int i = 0; i < vecWebList.size(); ++i)
			{
				if (vecWebList[i].GetLength()>1)
				{
					pKeyWordInfo->vOfficialList.push_back((CString)vecWebList[i]);
				}
			}
		}

	}

	// 搜索引擎名称
	CString strSearchName = VecResult[2];
	pKeyWordInfo->iFlag = g_mapSearch[strSearchName];
	if (pKeyWordInfo->iFlag == 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("解析搜索引擎名失败"));
		if (pKeyWordInfo != NULL)
		{
			delete pKeyWordInfo;
			pKeyWordInfo = NULL;
		}
		return;
	}

	// 关键词信息:
	// KEY(;2)关键词名称
	// 解析关键词信息
	std::vector<CStdString> vecKeyWordInfo;
	const CStdString &strWordInfo = VecResult[0];
	StringUtil.SplitString(strWordInfo, _T("(;2)"), vecKeyWordInfo, true);

	if (vecKeyWordInfo.size() < 2)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("解析关键词信息失败"));
		if (pKeyWordInfo != NULL)
		{
			delete pKeyWordInfo;
			pKeyWordInfo = NULL;
		}
		return;
	}

	pKeyWordInfo->strKey = vecKeyWordInfo[0];
	pKeyWordInfo->strKeyWordName = vecKeyWordInfo[1];
	pKeyWordInfo->strKeyHex = UrlEncode(pKeyWordInfo->strKeyWordName);

	// 解析分割符列表
	// 分割符列表：
	// 分割符1(;3)分割符2(;3)分割符3…(;3)分割符N
	std::vector<CStdString> vecConpNameSlip;
	const CStdString &strConpNameInfo = VecResult[1];
	CString strPreInfo = _T("");
	if (VecResult.size() > 7)
	{
		strPreInfo = VecResult[7];
	}
	StringUtil.SplitString(strConpNameInfo, _T("(;3)"), vecConpNameSlip, true);

	if (vecConpNameSlip.size() == 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("解析公司名组合失败"));
		if (pKeyWordInfo != NULL)
		{
			delete pKeyWordInfo;
			pKeyWordInfo = NULL;
		}
		return;
	}


	//新增开关，判断是否需要对抓取指纹进行放宽2017/01/09
	//(;9)___(;3)|||(;3)///(;3)---(;9)
	//(;9)___s(;3)|||(;3)///(;3)---(;9)在第一个拼接符的尾部增加一个字符s,表示是否严抓的开关
	bool bSwitch = true;
	// 	vecConpNameSlip[0] = _T("__");
	// 	vecConpNameSlip[0] = _T("___");
	// 	vecConpNameSlip[0] = _T("___s");
	// 	vecConpNameSlip[0] = _T("___ss");
	// 	vecConpNameSlip[0] = _T("___sqe");
	if (vecConpNameSlip.size() > 0 && vecConpNameSlip[0].GetLength() > 1)
	{
		int ilen = vecConpNameSlip[0].GetLength();
		TCHAR ch = vecConpNameSlip[0].GetAt(ilen - 1);//GetAt编号从0开始
		if (_T('s') == ch || _T('S') == ch)
		{
			bSwitch = false;
			vecConpNameSlip[0] = vecConpNameSlip[0].Left(ilen - 1);
		}
		else if (_T('r') == ch || _T('R') == ch)
		{
			pKeyWordInfo->bOnlyRareWord = true;
		}
	}

	for (int j = 0; j < vecConpNameSlip.size(); j++)
	{
		if (vecConpNameSlip[j].size() > 0)
		{
			for (int i = 0; i < pKeyWordInfo->vAllCompanys.size(); ++i)
			{
				CString strComSlip = vecConpNameSlip[j];
				CString strComnyCom = strPreInfo + (CString)pKeyWordInfo->vAllCompanys[i] + strComSlip;
				pKeyWordInfo->vCompanys.push_back(strComnyCom);
			}
		}
	}

	//部分网站会对我们的特殊符号进行改写，加入以下代码进行兼容（eg：|||被修改为|）2017/09/19
	if (bSwitch)
	{
		CString  sCompanyMark[5] = { _T("|"), _T("_"), _T("/"), _T("-"), _T(",") };
		for (int index = 0; index < sizeof(sCompanyMark) / sizeof(sCompanyMark[0]); index++)
		{
			for (int i = 0; i < pKeyWordInfo->vAllCompanys.size(); ++i)
			{
				CString strComnyCom = strPreInfo + (CString)pKeyWordInfo->vAllCompanys[i] + sCompanyMark[index];
				pKeyWordInfo->vCompanys.push_back(strComnyCom);
			}
		}
	}
	//部分网站的抓取指纹与发布指纹不一致，主要表现在发布标记前缀是否存在“点”号，现加入以下代码进行兼容2017/11/28
	if (pKeyWordInfo->vCompanys.size() > 0)
	{
		std::vector<CString> strVCompany = pKeyWordInfo->vCompanys;
		for (int i = 0; i < strVCompany.size(); i++)
		{
			CString strNewName(_T("."));
			if (_T('.') == strVCompany[i].GetAt(0))
			{
				strNewName = strVCompany[i].Mid(1);
			}
			else
			{
				strNewName += strVCompany[i];
			}
			pKeyWordInfo->vCompanys.push_back(strNewName);
		}
	}

	if (pKeyWordInfo->vCompanys.size() == 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("解析公司名组合失败"));
		if (pKeyWordInfo != NULL)
		{
			delete pKeyWordInfo;
			pKeyWordInfo = NULL;
		}
		return;
	}

	//微信名
	if (VecResult.size() > 8)
	{
		pKeyWordInfo->strWeixinName = VecResult[8];
	}

	//客户类型
	if (VecResult.size() > 9)
	{
		pKeyWordInfo->strClientType = VecResult[9];
	}

	//推广类型
	if (VecResult.size() > 10)
	{
		CStdString strPublishType = VecResult[10];
		if (!strPublishType.IsEmpty())
			pKeyWordInfo->iPublishType = _ttoi(strPublishType);

		if (pKeyWordInfo->iPublishType == 2)
		{
			CStringA strListA = pKeyWordInfo->strWebList;
			CString strListW;
			strListW = CStrA2CStrW(strListA);

			vector<CString > vsSiteList;
			SplitCString(strListW, _T("\r\n"), vsSiteList, true);

			for (int i = 0; i < vsSiteList.size(); i++)
			{
				vector<CString > vSingleSite;
				SplitCString(vsSiteList[i], _T(","), vSingleSite, true);
				CStdString strFilter = vsSiteList[i] + _T("\r\n");
				CStringA strTmp = WideToChar(strFilter.c_str());
				if (3 == vSingleSite.size())
				{
					//推广类型为2（只推轻舟站的客户），只抓官网，需要过滤非官网网站
					if ((_T("4") != vSingleSite[2].Mid(11, 1)))
					{						
						pKeyWordInfo->strWebList.Replace(strTmp, "");
					}
				}
				else if (2 == vSingleSite.size())
				{
					int iPos = -1;
					iPos = pKeyWordInfo->strWebList.Find(strTmp);
					if (iPos != -1)
					{
						CStringA strList = pKeyWordInfo->strWebList;
						pKeyWordInfo->strWebList = strList.Left(iPos);
					}
					break;
				}
			}			
		}
	}

	if (pKeyWordInfo != NULL)
	{
		g_vAllKeyWords.push_back(pKeyWordInfo);
	}

	return;
}



char* CStringToMutilChar(CString& str, int& chLength, WORD wPage)
{
	char* pszMultiByte;
	int iSize = WideCharToMultiByte(wPage, 0, str, -1, NULL, 0, NULL, NULL);
	pszMultiByte = (char*)malloc((iSize + 1)/**sizeof(char)*/);
	memset(pszMultiByte, 0, iSize + 1);
	WideCharToMultiByte(wPage, 0, str, -1, pszMultiByte, iSize, NULL, NULL);
	chLength = iSize;
	return pszMultiByte;
}

// URL编码
CString  UrlEncode(CString sIn)
{
	int ilength = -1;
	char* pUrl = CStringToMutilChar(sIn, ilength, CP_UTF8);
	CStringA strSrc(pUrl);

	CStringA sOut;
	const int nLen = strSrc.GetLength() + 1;

	register LPBYTE pOutTmp = NULL;
	LPBYTE pOutBuf = NULL;
	register LPBYTE pInTmp = NULL;
	LPBYTE pInBuf = (LPBYTE)strSrc.GetBuffer(nLen);
	BYTE b = 0;

	//alloc out buffer
	pOutBuf = (LPBYTE)sOut.GetBuffer(nLen * 3 - 2);//new BYTE [nLen  * 3];

	if (pOutBuf)
	{
		pInTmp = pInBuf;
		pOutTmp = pOutBuf;

		// do encoding
		while (*pInTmp)
		{
			if (isalnum(*pInTmp))
				*pOutTmp++ = *pInTmp;
			else
			if (isspace(*pInTmp))
				*pOutTmp++ = '+';
			else
			{
				*pOutTmp++ = '%';
				*pOutTmp++ = toHex(*pInTmp >> 4);
				*pOutTmp++ = toHex(*pInTmp % 16);
			}
			pInTmp++;
		}
		*pOutTmp = '\0';
		//sOut=pOutBuf;
		//delete [] pOutBuf;
		sOut.ReleaseBuffer();
	}
	strSrc.ReleaseBuffer();
	if (pUrl != NULL)
	{
		delete pUrl;
		pUrl = NULL;
	}
	return CString(sOut);
}


void GetDefaultWeb()
{
	TCHAR szFile[MAX_PATH] = { 0 };
	CString strFilePath;

	if (GetModuleFileName(NULL, szFile, MAX_PATH))
	{
		PathAppend(szFile, _T("..\\..\\"));

		HANDLE hFile = INVALID_HANDLE_VALUE;
		if (g_strWeb.IsEmpty())
		{
			strFilePath.Format(_T("%s6\\SiteList.txt"), szFile);
			hFile = CreateFile(strFilePath.GetString(),               // file to open
				GENERIC_READ,          // open for reading
				FILE_SHARE_READ,       // share for reading
				NULL,                  // default security
				OPEN_EXISTING,         // existing file only
				FILE_ATTRIBUTE_NORMAL, // normal file
				NULL);                 // no attr. template

			if (hFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwSize = GetFileSize(hFile, NULL);
				if (INVALID_FILE_SIZE != dwSize
					&& dwSize > 0)
				{
					LPSTR pBuf = g_strWeb.GetBuffer(dwSize + 1);

					DWORD dwFact = 0;
					do
					{
						DWORD dwByte;
						if (!ReadFile(hFile, pBuf + dwFact, dwSize, &dwByte, NULL))
							break;

						dwFact += dwByte;

					} while (dwFact < dwSize);

					g_strWeb.ReleaseBuffer();
				}

				CloseHandle(hFile);
			}
		}
	}
}

/*
@brief  宽字符转换成多字节
@param  pChar  宽字符
@return 返回多字节
*/
CStringA WideToChar(const wchar_t *pWide, DWORD dwCode)
{
	CStringA strChar = "";
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


//清理百度、搜搜、360等搜索引擎缓存
void DeleteSearchCache()
{
	INTERNET_CACHE_ENTRY_INFO *cache_entry_info;
	HANDLE hcache;
	DWORD size = sizeof(INTERNET_CACHE_ENTRY_INFO);

	FindFirstUrlCacheEntry(NULL, NULL, &size);
	cache_entry_info = (INTERNET_CACHE_ENTRY_INFO*)new char[size];
	hcache = FindFirstUrlCacheEntry(NULL, cache_entry_info, &size);

	if (hcache)
	{
		BOOL flag = TRUE;
		while (flag)
		{
			//cout<<cache_entry_info->lpszSourceUrlName<<" "<<cache_entry_info->lpszSourceUrlName<<endl<<endl;
			CString strUrl = cache_entry_info->lpszSourceUrlName;
			if (strUrl.Find(_T(".baidu.")) != -1
				|| strUrl.Find(_T(".so.")) != -1
				|| strUrl.Find(_T(".soso.")) != -1
				|| strUrl.Find(_T(".sogou.")) != -1
				|| strUrl.Find(_T(".bing.")) != -1
				|| strUrl.Find(_T(".youdao.")) != -1
				|| strUrl.Find(_T(".360.")) != -1)
			{
				DeleteUrlCacheEntry(cache_entry_info->lpszSourceUrlName);
			}

			delete[]cache_entry_info;
			size = 0;

			FindNextUrlCacheEntry(hcache, NULL, &size);
			cache_entry_info = (INTERNET_CACHE_ENTRY_INFO*)new char[size];
			flag = FindNextUrlCacheEntry(hcache, cache_entry_info, &size);
		}
	}
	if (cache_entry_info)
	{
		delete[]cache_entry_info;
	}
}

// Unicode CString URLEncode
BYTE toHex(const BYTE &x)
{
	return x > 9 ? x + 55 : x + 48;
}


//URLDecode
CString Utf8ToStringT(LPSTR str)
{
	CString strResult;
	_ASSERT(str);
	USES_CONVERSION;
	WCHAR *buf;
	int length = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	buf = new WCHAR[length + 1];
	ZeroMemory(buf, (length + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, length);

	if (str != NULL)
	{
		delete str;
		str = NULL;
	}
	strResult = (CString(W2T(buf)));
	if (buf != NULL)
	{
		delete[]buf;
		buf = NULL;
	}
	return strResult;
}

CString UrlDecode(LPCTSTR url)
{
	_ASSERT(url);
	USES_CONVERSION;
	LPSTR _url = T2A(const_cast<LPTSTR>(url));

	int i = 0;
	int length = (int)strlen(_url);
	CHAR *buf = new CHAR[length + 1];
	ZeroMemory(buf, length + 1);
	LPSTR p = buf;
	while (i < length)
	{
		if (i <= length - 3 && _url[i] == '%' && IsHexNum(_url[i + 1]) && IsHexNum(_url[i + 2]))
		{
			sscanf(_url + i + 1, "%2x", p++);

			i += 3;
		}
		else
		{
			*(p++) = _url[i++];
		}
	}

	return Utf8ToStringT(buf);
}


// 取任务
pKeyWordDataInfo GetSearKeyWordInfo()
{
	pKeyWordDataInfo pSData = NULL;
	EnterCriticalSection(&critSection);
	if (g_vAllKeyWords.size() > 0)
	{
		pSData = (pKeyWordDataInfo)g_vAllKeyWords.at(g_vAllKeyWords.size() - 1);
		g_vAllKeyWords.pop_back();
	}
	LeaveCriticalSection(&critSection);
	return pSData;
}

//替换网页特殊字符
void ReplaceHtmlChar(CString &strData)
{
	for (int i = 0; i < HtmlCharNum; i++)
	{
		strData.Replace(tSpecialChar[i], _T(""));
	}
}

CStringA CStrW2CStrA(const CStringW &cstrSrcW)
{
	int len = WideCharToMultiByte(CP_ACP, 0, LPCWSTR(cstrSrcW), -1, NULL, 0, NULL, NULL);
	char *str = new char[len];
	memset(str, 0, len);
	WideCharToMultiByte(CP_ACP, 0, LPCWSTR(cstrSrcW), -1, str, len, NULL, NULL);
	CStringA cstrDestA = str;
	delete[] str;

	return cstrDestA;
}
CStringW CStrA2CStrW(const CStringA &cstrSrcA)
{
	int len = MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len];
	memset(wstr, 0, len*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, wstr, len);
	CStringW cstrDestW = wstr;
	delete[] wstr;

	return cstrDestW;
}
bool SplitCString(const CString & input, const CString & delimiter, std::vector<CString >& results, bool includeEmpties)
{
	int iPos = 0;
	int newPos = -1;
	int sizeS2 = (int)delimiter.GetLength();
	int isize = (int)input.GetLength();

	int offset = 0;
	CString  s;

	if (
		(isize == 0)
		||
		(sizeS2 == 0)
		)
	{
		return 0;
	}

	std::vector<int> positions;

	newPos = input.Find(delimiter, 0);

	if (newPos < 0)
	{
		if (!input.IsEmpty())
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
			newPos = input.Find(delimiter, iPos + sizeS2);
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
		s.Empty();
		if (i == 0)
		{
			s = input.Mid(i, positions[i]);
		}
		else
		{
			offset = positions[i - 1] + sizeS2;

			if (offset < isize)
			{
				if (i == positions.size())
				{
					s = input.Mid(offset);
				}
				else if (i > 0)
				{
					s = input.Mid(positions[i - 1] + sizeS2, positions[i] - positions[i - 1] - sizeS2);
				}
			}
		}

		if (/*includeEmpties || */(s.GetLength() > 0))
		{
			results.push_back(s);
		}
	}
	return true;
}


void Recurse(LPCTSTR _pstr, LPCTSTR _fileType, bool _bRecurse)
{
	CFileFind finder;

	// build a string with wildcards
	CString strWildcard(_pstr);
	strWildcard += _T("\\*.*");

	// start working for files
	BOOL bWorking = finder.FindFile(strWildcard);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// skip . and .. files; otherwise, we'd
		// recur infinitely!

		if (finder.IsDots())
			continue;

		// if it's a directory, recursively search it

		if (finder.IsDirectory())
		{
			if (_bRecurse)
			{
				CString str = finder.GetFilePath();
				Recurse(str, _fileType, _bRecurse);
			}
			else
			{
				continue;
			}
		}
		else
		{
			CString strFilePath = finder.GetFilePath();
			int flag = strFilePath.ReverseFind(_T('.'));
			if (-1 != flag)
			{
				if (0 == strFilePath.Mid(flag).CompareNoCase(_fileType))
				{
					DeleteFile(strFilePath);
				}
			}
		}
	}

	finder.Close();
}
