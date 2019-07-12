// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// UpdateRank.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

// TODO:  �� STDAFX.H ��
// �����κ�����ĸ���ͷ�ļ����������ڴ��ļ�������
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
// ��ʼ��map
void InitMap()
{
	g_mapSearch[_T("�ٶ�")] = SEARCH_BAIDU;
	g_mapSearch[_T("360����")] = SEARCH_360;
	g_mapSearch[_T("�ѹ�")] = SEARCH_SOGOU;
	g_mapSearch[_T("��Ӧ")] = SEARCH_BING;
	g_mapSearch[_T("�е�")] = SEARCH_YOUDAO;
	g_mapSearch[_T("�ֻ��ٶ�")] = SEARCH_PHONEBAIDU;
	g_mapSearch[_T("�ֻ��ѹ�")] = SEARCH_PHONESOGOU;
	g_mapSearch[_T("�ֻ�360")] = SEARCH_PHONE360;
	g_mapSearch[_T("΢��")] = SEARCH_WEIXIN;
	g_mapSearch[_T("�ֻ�����")] = SEARCH_PHONESHENMA;
}

// ��keywordĿ¼�´�����˾��Ŀ¼�������
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
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("����DEBUGĿ¼ʧ��, ·��:%s, ERROR:%d"), strFile, GetLastError());
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
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("����Ŀ¼ʧ��, ·��:%s, ERROR:%d"), strFile, GetLastError());
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
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("����Ŀ¼ʧ��, ·��:%s, ERROR:%d"), strFile, GetLastError());
	}

}

// ��ȡCPU������
int GetCPUCoreNums()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}


// ���治ͬ�û���˾��
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

// �������йؼ�������
void ParseAllKeyWordData(const CStdString &strWord)
{
	// ��ʼ��
	InitMap();

	// ����Ĭ����վ
	GetDefaultWeb();

	// �洢���йؼ���������Ϣ
	std::vector<CStdString> vecKeyWordInfo;
	StringUtils StringUtil;

	g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("��ʼ��������"));

	// �ؼ���1����(;8) �ؼ���2����(;8)����.(;8)�ؼ���N����
	// ���������йؼ�����Ϣ�洢��vecKeyWordInfo��
	StringUtil.SplitString(strWord, _T("(;8)"), vecKeyWordInfo, true);
	if (vecKeyWordInfo.size() == 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("û������"));
	}

	// �����йؼ�����Ϣ������ȫ�ֱ���g_vAllKeyWords��
	for (int iCount = 0; iCount < vecKeyWordInfo.size(); iCount++)
	{
		Sleep(5);
		if (!vecKeyWordInfo[iCount].IsEmpty())
		{
			ParseKeyWordData(vecKeyWordInfo[iCount]);
		}
	}

	CreateDateDirDir(g_sKWPath);
	// ������ͬ��˾��Ŀ¼
	for (int i = 0; i < g_vDiffCompany.size(); i++)
	{
		Sleep(5);
		CreateCompanyDir(g_vDiffCompany[i]);
	}

	g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("��ͬ��˾������:%d"), g_vDiffCompany.size());

	g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("�����������,�յ�%d���ؼ�������"), vecKeyWordInfo.size());

	vecKeyWordInfo.clear();
}

// ����ÿ������Ĺؼ�����Ϣ
void ParseKeyWordData(const CStdString &strKeyWord)
{
	if (strKeyWord.Compare(_T("")) == 0)
	{
		return;
	}

	std::vector<CStdString> VecResult;
	StringUtils StringUtil;

	// �ؼ�������:
	// �ؼ�����Ϣ(;9)�ָ���б�(;9)������������(;9)��վ�б�(;9)Ŀ����վ���(;9)��˾��(;3)��˾���(;9)�����б�(;9)�������ǰ׺(;9)΢�Ź��ں�����(;9)�Ƿ�����ͻ�
	// �ֽ�ؼ�����Ϣ
	StringUtil.SplitString(strKeyWord, _T("(;9)"), VecResult, true);

	int iCountInfo = VecResult.size();
	if (iCountInfo < 6)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("�ֽⵥ���ؼ�������ʧ�ܣ�(;9)����:%d"), iCountInfo);
		return;
	}


	// ���ݷֽ���ַ������ؼ�����Ϣ�ṹ
	// ��վ��־
	pKeyWordDataInfo pKeyWordInfo = new KeyWordDataInfo();
	pKeyWordInfo->strWebFlag = VecResult[4];
	if (pKeyWordInfo->strWebFlag.IsEmpty())
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("Ŀ����վ���Ϊ��"));
		if (pKeyWordInfo != NULL)
		{
			delete pKeyWordInfo;
			pKeyWordInfo = NULL;
		}
		return;
	}

	//  	Sleep(8000);
	// // 	// ��˾��
	// 	pKeyWordInfo->vAllCompanys.clear();
	// 	VecResult[5] = _T("�������Ƽ����޹�˾");
	// 	VecResult[5] = _T("�������Ƽ����޹�˾(;3)�������Ƽ�");
	// 	VecResult[5] = _T("�������Ƽ����޹�˾(;3)�������Ƽ�|");
	// 	VecResult[5] = _T("�������Ƽ����޹�˾(;3)�������Ƽ�|������");
	// 	VecResult[5] = _T("�������Ƽ����޹�˾(;3)");
	CStdString strCompanys;
	StringUtil.SplitString(VecResult[5], _T("(;3)"), pKeyWordInfo->vAllCompanys, false);
	std::vector<CStdString>  vShortCompanys;		//�ֽ���
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

				//�ж����˾���
				CString sComName = vShortCompanys[i];
				CStdString sComTag;
				sComTag.Format(_T(",%s,"), sComName);
				pKeyWordInfo->vCompanysTag.push_back(sComTag);
				sComTag.Format(_T("��%s��"), sComName);
				pKeyWordInfo->vCompanysTag.push_back(sComTag);
				sComTag.Format(_T(",%s��"), sComName);
				pKeyWordInfo->vCompanysTag.push_back(sComTag);
				sComTag.Format(_T("��%s,"), sComName);
				pKeyWordInfo->vCompanysTag.push_back(sComTag);
			}
		}
		else
		{
			//ֻ��һ����˾���
			CString sComName = pKeyWordInfo->vAllCompanys[1];
			CStdString sComTag;
			sComTag.Format(_T(",%s,"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T("��%s��"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T(",%s��"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T("��%s,"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
		}
	}
	else if (1 == pKeyWordInfo->vAllCompanys.size())
	{
		//û�й�˾��ƣ��ù�˾ȫ��
		//_T("�������Ƽ����޹�˾(;3)");
		CString sComName = pKeyWordInfo->vAllCompanys[0];
		CStdString sComTag;
		sComTag.Format(_T(",%s,"), sComName);
		pKeyWordInfo->vCompanysTag.push_back(sComTag);
		sComTag.Format(_T("��%s��"), sComName);
		pKeyWordInfo->vCompanysTag.push_back(sComTag);
		sComTag.Format(_T(",%s��"), sComName);
		pKeyWordInfo->vCompanysTag.push_back(sComTag);
		sComTag.Format(_T("��%s,"), sComName);
		pKeyWordInfo->vCompanysTag.push_back(sComTag);
	}

	if (pKeyWordInfo->vAllCompanys.size() <= 0)
	{
		if (VecResult[5].GetLength() <= 0)
		{
			g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("��˾���ͼ��Ϊ��"));
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

			//û�й�˾��ƣ��ù�˾ȫ��
			//_T("�������Ƽ����޹�˾");
			CString sComName = VecResult[5];
			CStdString sComTag;
			sComTag.Format(_T(",%s,"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T("��%s��"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T(",%s��"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
			sComTag.Format(_T("��%s,"), sComName);
			pKeyWordInfo->vCompanysTag.push_back(sComTag);
		}
	}

	pKeyWordInfo->strComany = pKeyWordInfo->vAllCompanys[0];

	// ���湫˾��
	CString strComName = pKeyWordInfo->strComany;
	SaveCompanyName(strComName);

	// ������վ�б�
	// ��վ�б���Ϣ:
	// ��վ1(;1) ��վ2(;1) ��վ3��(;1) ��վN
	std::vector<CStdString> vecWebList;
	const CStdString &strWebInfo = VecResult[3];
	const CStdString &strWebFlag = VecResult[4];
	const CStdString &strWebHost = VecResult[6];

	if (strWebFlag == _T("2"))  //����վ�б�
	{
		CStringA strList = WideToChar(strWebInfo.c_str());
		if (strList.GetLength() > 0)
		{
			pKeyWordInfo->strWebList = strList;
		}
		//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("����վ,��վΪ:%s"), strWebInfo.c_str());
	}
	else if (strWebFlag == _T("1"))  //��վ�б����콢����վ
	{
		CStringA strList = WideToChar(strWebInfo.c_str());
		if (strList.GetLength() > 0)
		{
			pKeyWordInfo->strWebList = g_strWeb + strList;
		}

		//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("�б����վ,��վΪ:%s"), strWebInfo.c_str());
	}
	else if (strWebFlag == _T("0"))  //��Ĭ���б�
	{
		pKeyWordInfo->strWebList = g_strWeb;
		//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("���б�,��վΪ:%s"), strWebInfo.c_str());
	}
	else if ((strWebFlag == _T("3")) || (strWebFlag == _T("4")))  //���ӹ������ų��б�3��  //��Ĭ���б��ų��ƶ���վ(4)
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
			g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("�ų�ָ����վ������%d"), vecWebList.size());
		}

		CStringA strTmp = NULL;
		CStdString strLow;
		for (int i = 0; i < vecWebList.size(); i++)
		{
			//ת����Сд
			strLow = vecWebList[i].ToLower();
			strTmp = WideToChar(strLow.c_str());
			g_strWeb.Replace(strTmp, "");
		}
		pKeyWordInfo->strWebList = g_strWeb;

		if (strWebFlag == _T("3"))
		{
			//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("�յ��Ĺ����б�%s"), strWebHost.c_str());
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

	// ������������
	CString strSearchName = VecResult[2];
	pKeyWordInfo->iFlag = g_mapSearch[strSearchName];
	if (pKeyWordInfo->iFlag == 0)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("��������������ʧ��"));
		if (pKeyWordInfo != NULL)
		{
			delete pKeyWordInfo;
			pKeyWordInfo = NULL;
		}
		return;
	}

	// �ؼ�����Ϣ:
	// KEY(;2)�ؼ�������
	// �����ؼ�����Ϣ
	std::vector<CStdString> vecKeyWordInfo;
	const CStdString &strWordInfo = VecResult[0];
	StringUtil.SplitString(strWordInfo, _T("(;2)"), vecKeyWordInfo, true);

	if (vecKeyWordInfo.size() < 2)
	{
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("�����ؼ�����Ϣʧ��"));
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

	// �����ָ���б�
	// �ָ���б�
	// �ָ��1(;3)�ָ��2(;3)�ָ��3��(;3)�ָ��N
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
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("������˾�����ʧ��"));
		if (pKeyWordInfo != NULL)
		{
			delete pKeyWordInfo;
			pKeyWordInfo = NULL;
		}
		return;
	}


	//�������أ��ж��Ƿ���Ҫ��ץȡָ�ƽ��зſ�2017/01/09
	//(;9)___(;3)|||(;3)///(;3)---(;9)
	//(;9)___s(;3)|||(;3)///(;3)---(;9)�ڵ�һ��ƴ�ӷ���β������һ���ַ�s,��ʾ�Ƿ���ץ�Ŀ���
	bool bSwitch = true;
	// 	vecConpNameSlip[0] = _T("__");
	// 	vecConpNameSlip[0] = _T("___");
	// 	vecConpNameSlip[0] = _T("___s");
	// 	vecConpNameSlip[0] = _T("___ss");
	// 	vecConpNameSlip[0] = _T("___sqe");
	if (vecConpNameSlip.size() > 0 && vecConpNameSlip[0].GetLength() > 1)
	{
		int ilen = vecConpNameSlip[0].GetLength();
		TCHAR ch = vecConpNameSlip[0].GetAt(ilen - 1);//GetAt��Ŵ�0��ʼ
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

	//������վ������ǵ�������Ž��и�д���������´�����м��ݣ�eg��|||���޸�Ϊ|��2017/09/19
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
	//������վ��ץȡָ���뷢��ָ�Ʋ�һ�£���Ҫ�����ڷ������ǰ׺�Ƿ���ڡ��㡱�ţ��ּ������´�����м���2017/11/28
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
		g_log.Trace(LOGL_TOP, LOGT_WARNING, __TFILE__, __LINE__, _T("������˾�����ʧ��"));
		if (pKeyWordInfo != NULL)
		{
			delete pKeyWordInfo;
			pKeyWordInfo = NULL;
		}
		return;
	}

	//΢����
	if (VecResult.size() > 8)
	{
		pKeyWordInfo->strWeixinName = VecResult[8];
	}

	//�ͻ�����
	if (VecResult.size() > 9)
	{
		pKeyWordInfo->strClientType = VecResult[9];
	}

	//�ƹ�����
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
					//�ƹ�����Ϊ2��ֻ������վ�Ŀͻ�����ֻץ��������Ҫ���˷ǹ�����վ
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

// URL����
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
@brief  ���ַ�ת���ɶ��ֽ�
@param  pChar  ���ַ�
@return ���ض��ֽ�
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


//����ٶȡ����ѡ�360���������滺��
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


// ȡ����
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

//�滻��ҳ�����ַ�
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
