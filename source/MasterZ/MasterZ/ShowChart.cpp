#include "stdafx.h"
#include "ShowChart.h"

//static  DWORD iMsCount = 0;
//static  DWORD iAreaCount = 0;

CShowChart::CShowChart()
{	
// 	//此处创建datas不存在创建datas目录
// 	CString strDirDatas =(CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\datas");
// 
// 	//表示不存在
// 	if (_taccess(strDirDatas,0) != 0)
// 	{	
// 		g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("图表目录不存在需要创建！"));
// 		if (!CreateDirectory(strDirDatas,NULL))
// 		{
// 			g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("创建图表数据目录失败,会导致图表显示失败,目录路径%s,错误码%d"),strDirDatas,GetLastError());
// 		}
// 	}
	m_PaintManager = NULL;
	m_pActiveX = NULL;
	m_pActiveXExposure = NULL;

// 	//xml
// 	m_strXmlUserCare = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\datas\\usercaredata.xml");
// 	m_strXmlUserRequest = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\datas\\userrequestdata.xml");
// 	m_strXmlKeyWordCount = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\datas\\userkeywordcountdata.xml");
// 	m_strXmlExposureCount = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\datas\\userexposurecountdata.xml");

	//html
	m_strHtmlUserCare = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\usercarechart.html");
	m_strHtmlUserRequest = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\userrequestchart.html");
	m_strHtmlKeyWordCount = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\userkeywordchart.html");
	m_strHtmlExposureCount = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\userexposurechart.html");
	m_strHtmlEmptyKeyWordPath = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\nokeyworddata.html");
	m_strHtmlEmptyExposurePath = (CString)GetInstallPath() + _T("\\bin\\skin\\FusionCharts\\noexposuredata.html");
}

CShowChart::~CShowChart()
{


}

void CShowChart::SetPaintManager(CPaintManagerUI*pPaintMgr, HWND hwnd, CWebBrowserUI* pWeb)
{
	m_PaintManager = pPaintMgr;
	m_hwnd = hwnd;
//	m_pActiveX = pWeb;
}

void CShowChart::SetWebPoint(CWebBrowserUI* pWeb)
{
	m_pActiveXExposure = pWeb;
}

BOOL CShowChart::LoadUserCareChart(WEIXINTONG_DATA& pdata)
{
	if (!GenUserCareXml(pdata))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("创建XML失败,文件路径%s"), m_strHtmlUserCare);
		return FALSE;
	}

	CWebBrowserUI* pActiveX_UserRequest = static_cast<CWebBrowserUI*>(m_PaintManager->FindControl(_T("flash_userrequest")));

	if (NULL == pActiveX_UserRequest)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("用户请求数据flash Activex控件指针为空！"));
		return FALSE;
	}
	pActiveX_UserRequest->SetVisible(false);

	CWebBrowserUI* pActiveX = static_cast<CWebBrowserUI*>(m_PaintManager->FindControl(_T("flash_usercare")));
	if (NULL == pActiveX)
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("用户关注数据flash Activex控件指针为空！"));
		return FALSE;
	}
	pActiveX->SetVisible(true);


	pActiveX->SetDelayCreate(false);
	CCustomWebEventHandler *pWebHandle = new CCustomWebEventHandler;
	pWebHandle->SetMainHwnd(m_hwnd);

	pActiveX->SetWebBrowserEventHandler(pWebHandle);
	pActiveX->Navigate2(_T("about:blank"));
	pActiveX->Navigate2(m_strHtmlUserCare.GetBuffer());

	return TRUE;
}

BOOL CShowChart::LoadUserRequestChart(WEIXINTONG_DATA& pdata)
{
	if (!GenUserRequestXml(pdata))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("创建XML失败,文件路径%s"), m_strHtmlUserRequest);
		return FALSE;
	}

	CWebBrowserUI* pActiveX_UserCare = static_cast<CWebBrowserUI*>(m_PaintManager->FindControl(_T("flash_usercare")));
	if (NULL == pActiveX_UserCare)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("用户关注数据flash Activex控件指针为空！"));
		return FALSE;
	}
	pActiveX_UserCare->SetVisible(false);

	CWebBrowserUI* pActiveX = static_cast<CWebBrowserUI*>(m_PaintManager->FindControl(_T("flash_userrequest")));
	if (NULL == pActiveX)
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("用户请求数据flash Activex控件指针为空！"));
		return FALSE;
	}
	pActiveX->SetVisible(true);

	pActiveX->SetDelayCreate(false);
	CCustomWebEventHandler *pWebHandle = new CCustomWebEventHandler;
	pWebHandle->SetMainHwnd(m_hwnd);

	pActiveX->SetWebBrowserEventHandler(pWebHandle);
	pActiveX->Navigate2(_T("about:blank"));
	pActiveX->Navigate2(m_strHtmlUserRequest.GetBuffer());

	return TRUE;
}


//企业版2.0
//显示企业版2.0上线关键词数据统计
BOOL CShowChart::LoadKeyWordCountChart(std::vector<KEYWORDSTATISTICCHAR>& pdata)
{
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Enter LoadKeyWordCountChart Function!!!!"));

	if(!GenKeyWordCountXml(pdata))
	{
		g_log.Trace(LOGL_TOP, LOGT_ERROR, __TFILE__, __LINE__, _T("创建XML失败,文件路径%s"), m_strHtmlKeyWordCount);
		return FALSE;
	}

	if (m_pActiveX != NULL)
	{
		if (pdata.size() <= 0)
		{
			m_pActiveX->Navigate2(m_strHtmlEmptyKeyWordPath.GetBuffer());
		}
		else
		{
			m_pActiveX->Navigate2(m_strHtmlKeyWordCount.GetBuffer());
		}
		
	}

	return TRUE;
}

//显示企业版2.0产品曝光量统计
BOOL CShowChart::LoadExposureCountChart(std::vector<PRODUCTSTATISTICSCHART>& pdata)
{
	g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("Enter LoadExposureCountChart Function!!!!"));

	if (!GenExposureCountXml(pdata))
	{
		g_log.Trace(LOGL_TOP,LOGT_ERROR, __TFILE__,__LINE__, _T("创建XML失败,文件路径%s"),m_strHtmlExposureCount);
		return FALSE;
	}

	if (m_pActiveXExposure != NULL)
	{
		if (pdata.size() <= 0)
		{
			m_pActiveXExposure->Navigate2(m_strHtmlEmptyExposurePath.GetBuffer());
		}
		else
		{
			m_pActiveXExposure->Navigate2(m_strHtmlExposureCount.GetBuffer());
		}

	}

	return TRUE;
}

BOOL CShowChart::GenUserCareXml(WEIXINTONG_DATA& pdata)
{	
	int ilength = -1;
	CString strHtmlStart = _T("<html><head><meta charset=\"utf-8\"></head><body bgcolor=\"#ffffff\" scroll=\"no\"><div id=\"main\" style=\"width:680px; height:300px; padding:1px;\">");
	strHtmlStart += _T(" </div><script src = \"js/echarts.js\"></script><script type=\"text/javascript\">require.config({paths:{echarts: './js'}});");
	strHtmlStart += _T(" require(['echarts','echarts/chart/line'],function(ec){var myChart= ec.init(document.getElementById('main'),'macarons');");
	strHtmlStart += _T(" var option={title:{text:'用户关注曲线',x:'center'},tooltip:{trigger:'axis',axisPointer:{ type:'none'}},legend:{show:true, data : ['关注数', '取消关注数', '净增长数'], x : 'center', y : 'bottom'}, ");
	strHtmlStart += _T(" xAxis:[{name:'日期',type:'category',boundaryGap:false,axisLabel:{show:true,interval:1,textStyle:{fontSize:12,fontWeight:'bold'}},data:[");
	CString strHtmlMid0 = _T(" ]}],yAxis:[{name:'数量',type:'value',splitArea: {show: true}}],series : [{name:'关注数',type:'line',data:[");
	CString strHtmlMid1 = _T(" ]},{name:'取消关注数',type:'line',	data:[");
	CString strHtmlMid2 = _T(" ]},{name:'净增长数',type:'line',	data:[");
	CString strHtmlEnd = _T("]}]}; myChart.setOption(option); });</script></body></html>");
	/*	int ilength = -1;*/
	CString strXmlDefault = strHtmlStart + _T("0") + strHtmlMid0 + _T("'0'") + strHtmlMid1 + strHtmlMid2 + strHtmlEnd;

	CStdioFile xmlfile;
	if (!xmlfile.Open(m_strHtmlUserCare, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("创建XML失败,文件路径%s"), m_strHtmlUserCare);
		return FALSE;
	}
	if (pdata.iSuccessFlag == 0)
	{	
		g_log.Trace(LOGL_TOP,LOGT_WARNING, __TFILE__,__LINE__, _T("用户关注数据返回成功标志为0，设置为默认图表！"));

		char* pszMultiByte = CStringToMutilChar(strXmlDefault, ilength, CP_UTF8);

		xmlfile.Write(pszMultiByte, ilength);
		xmlfile.Close();
		free(pszMultiByte);
		pszMultiByte = NULL;
// 		xmlfile.WriteString(strXmlDefault.GetBuffer(strXmlDefault.GetLength()));
// 		xmlfile.Close();

		return TRUE;
	}
	if (pdata.iSuccessFlag == 1)
	{
		if (pdata.subscribeChartList.size() == 0)
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户关注数据返回成功标志为1，但是列表数据为空，设置为默认图表！"));

			char* pszMultiByte = CStringToMutilChar(strXmlDefault, ilength, CP_UTF8);

			xmlfile.Write(pszMultiByte, ilength);
			xmlfile.Close();
			free(pszMultiByte);
			pszMultiByte = NULL;
// 			xmlfile.WriteString(strXmlDefault.GetBuffer(strXmlDefault.GetLength()));
// 			xmlfile.Close();

			return TRUE;
		}
	}

	//下面就是对正常数据来进行处理
	CString strTempContent;
	CString strCategories = _T("");
	CString strdataset1 = _T("");
	CString strdataset2 = _T("");
	CString strdataset3 = _T("");
	for (int i = 0; i < pdata.subscribeChartList.size(); ++i)
	{
		strCategories += _T("'") + GetDay(pdata.subscribeChartList[i].szDate) + _T("',");
		strTempContent.Format(_T("%d,"), pdata.subscribeChartList[i].iSubscribNum);
		strdataset1 += strTempContent;
		strTempContent.Format(_T("%d,"), pdata.subscribeChartList[i].iUnSubscribNum);
		strdataset2 += strTempContent;
		strTempContent.Format(_T("%d,"), pdata.subscribeChartList[i].iAddNum);
		strdataset3 += strTempContent;
	}
	strCategories = strCategories.Left(strCategories.GetLength() - 1);
	strdataset1 = strdataset1.Left(strdataset1.GetLength() - 1);
	strdataset2 = strdataset2.Left(strdataset2.GetLength() - 1);
	strdataset3 = strdataset3.Left(strdataset3.GetLength() - 1);

	strXmlDefault = strHtmlStart + strCategories + strHtmlMid0 + strdataset1 + strHtmlMid1;
	strXmlDefault += strdataset2 + strHtmlMid2 + strdataset3 + strHtmlEnd;

	char* pszMultiByte = CStringToMutilChar(strXmlDefault, ilength, CP_UTF8);

	xmlfile.Write(pszMultiByte, ilength);
	xmlfile.Close();
	free(pszMultiByte);
	pszMultiByte = NULL;
// 	xmlfile.WriteString(strXmlDefault.GetBuffer(strXmlDefault.GetLength()));
// 	xmlfile.Close();

	return TRUE;
}

BOOL CShowChart::GenUserRequestXml(WEIXINTONG_DATA& pdata)
{
	int ilength = -1;
	CString strHtmlStart = _T("<html><head><meta charset=\"utf-8\"></head><body bgcolor=\"#ffffff\" scroll=\"no\"><div id=\"main\" style=\"width:680px; height:300px; padding:1px;\">");
	strHtmlStart += _T(" </div><script src = \"js/echarts.js\"></script><script type=\"text/javascript\">require.config({paths:{echarts: './js'}});");
	strHtmlStart += _T(" require(['echarts','echarts/chart/line'],function(ec){var myChart= ec.init(document.getElementById('main'),'macarons');");
	strHtmlStart += _T(" var option={title:{text:'用户请求曲线',x:'center'},tooltip:{trigger:'axis',axisPointer:{ type:'none'}},legend:{show:true, data : ['文本回复', '菜单点击', '总请求数'], x : 'center', y : 'bottom'}, ");
	strHtmlStart += _T(" xAxis:[{name:'日期',type:'category',boundaryGap:false,axisLabel:{show:true,interval:1,textStyle:{fontSize:12,fontWeight:'bold'}},data:[");
	CString strHtmlMid0 = _T(" ]}],yAxis:[{name:'数量',type:'value',splitArea: {show: true}}],series : [{name:'文本回复',type:'line',data:[");
	CString strHtmlMid1 = _T(" ]},{name:'菜单点击',type:'line',	data:[");
	CString strHtmlMid2 = _T(" ]},{name:'总请求数',type:'line',	data:[");
	CString strHtmlEnd = _T("]}]}; myChart.setOption(option); });</script></body></html>");

	CString strXmlDefault = strHtmlStart + _T("0") + strHtmlMid0 + _T("0") + strHtmlMid1 + strHtmlMid2 + strHtmlEnd;

	CStdioFile xmlfile;
	if (!xmlfile.Open(m_strHtmlUserRequest, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("创建XML失败,文件路径%s"), m_strHtmlUserRequest);
		return FALSE;
	}
	if (pdata.iSuccessFlag == 0)
	{	
		g_log.Trace(LOGL_TOP,LOGT_WARNING, __TFILE__,__LINE__, _T("用户请求数据返回成功标志为0，设置为默认图表！"));

		char* pszMultiByte = CStringToMutilChar(strXmlDefault, ilength, CP_UTF8);

		xmlfile.Write(pszMultiByte, ilength);
		xmlfile.Close();
		free(pszMultiByte);
		pszMultiByte = NULL;
// 		xmlfile.WriteString(strXmlDefault.GetBuffer(strXmlDefault.GetLength()));
// 		xmlfile.Close();

		return TRUE;
	}
	if (pdata.iSuccessFlag == 1)
	{
		if (pdata.requestChartList.size() == 0)
		{	
			g_log.Trace(LOGL_TOP,LOGT_PROMPT, __TFILE__,__LINE__, _T("用户请求数据返回成功标志为1，但是列表数据为空，设置为默认图表！"));

			char* pszMultiByte = CStringToMutilChar(strXmlDefault, ilength, CP_UTF8);

			xmlfile.Write(pszMultiByte, ilength);
			xmlfile.Close();
			free(pszMultiByte);
			pszMultiByte = NULL;
// 			xmlfile.WriteString(strXmlDefault.GetBuffer(strXmlDefault.GetLength()));
// 			xmlfile.Close();

			return TRUE;
		}
	}
	//下面就是对正常数据来进行处理
	CString strTempContent;
	CString strCategories = _T("");
	CString strdataset1 = _T("");
	CString strdataset2 = _T("");
	CString strdataset3 = _T("");
	for (int i = 0; i < pdata.requestChartList.size(); ++i)
	{
		strCategories += _T("'") + GetDay(pdata.requestChartList[i].szDate) + _T("',");
		strTempContent.Format(_T("%d,"), pdata.requestChartList[i].iTextReply);
		strdataset1 += strTempContent;
		strTempContent.Format(_T("%d,"), pdata.requestChartList[i].iMenuClick);
		strdataset2 += strTempContent;
		strTempContent.Format(_T("%d,"), pdata.requestChartList[i].iTotalRequest);
		strdataset3 += strTempContent;
	}
	strCategories = strCategories.Left(strCategories.GetLength() - 1);
	strdataset1 = strdataset1.Left(strdataset1.GetLength() - 1);
	strdataset2 = strdataset2.Left(strdataset2.GetLength() - 1);
	strdataset3 = strdataset3.Left(strdataset3.GetLength() - 1);

	strXmlDefault = strHtmlStart + strCategories + strHtmlMid0 + strdataset1 + strHtmlMid1;
	strXmlDefault += strdataset2 + strHtmlMid2 + strdataset3 + strHtmlEnd;

	char* pszMultiByte = CStringToMutilChar(strXmlDefault, ilength, CP_UTF8);

	xmlfile.Write(pszMultiByte, ilength);
	xmlfile.Close();
	free(pszMultiByte);
	pszMultiByte = NULL;
// 	xmlfile.WriteString(strXmlDefault.GetBuffer(strXmlDefault.GetLength()));
// 	xmlfile.Close();

	return TRUE;
}

//企业版2.0上线关键词统计数据源
BOOL CShowChart::GenKeyWordCountXml(std::vector<KEYWORDSTATISTICCHAR>& pdata)
{
	int ilength = -1;
	CString strHtmlStart = _T("<html><head><meta charset=\"utf-8\"></head><body bgcolor=\"#ffffff\" scroll=\"no\"><div id=\"main\" style=\"width:620px; height:300px; padding:1px;\">");
	strHtmlStart += _T(" </div><script src = \"js/echarts.js\"></script><script type=\"text/javascript\">require.config({paths:{echarts: './js'}});");
	strHtmlStart += _T(" require(['echarts','echarts/chart/line'],function(ec){var myChart= ec.init(document.getElementById('main'),'macarons');");
	strHtmlStart += _T(" var option={title:{text:'近一个月上线关键词统计',x:'center'},tooltip:{trigger:'axis',axisPointer:{ type:'none'}},");
	strHtmlStart += _T(" xAxis:[{name:'日期',type:'category',boundaryGap:false,axisLabel:{show:true,rotate:90,interval:1,textStyle:{fontSize:12,fontWeight:'bold'}},data:[");
	CString strHtmlMid = _T("]}],yAxis:[{name:'数量',type:'value',splitArea: {show: true}}],series:[{name:'关键词',type:'line',data:["); 
	CString strHtmlEnd = _T("]}]}; myChart.setOption(option); });</script></body></html>");
	
	CString strXmlDefault = _T("");
	CStdioFile xmlfile;

	if (!xmlfile.Open(m_strHtmlKeyWordCount,CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("创建XML失败,文件路径%s"), m_strHtmlKeyWordCount);
		return FALSE;
	}

	if (pdata.size() == 0)
	{	
		g_log.Trace(LOGL_TOP,LOGT_WARNING, __TFILE__,__LINE__, _T("用户上线关键词统计数据列表数据为空，设置为默认图表！"));
		return TRUE;
	}
	//接下来就是取正常数据
	CString strXContent =_T("");
	CString strYContent = _T("");
	CString strSetContent = _T("");
	for(int m = 0; m < pdata.size(); ++m)
	{	
		strXContent += _T("'") + FormatDate(pdata[m].szDate) + _T("',");
		strSetContent.Format(_T("%d,"),pdata[m].iAllSEHasRankingCount);
		strYContent += strSetContent;
	}
	strXContent = strXContent.Left(strXContent.GetLength() - 1);
	strYContent = strYContent.Left(strYContent.GetLength() - 1);
	strXmlDefault = strHtmlStart+ strXContent+strHtmlMid+strYContent+strHtmlEnd;

	char* pszMultiByte = CStringToMutilChar(strXmlDefault, ilength,CP_UTF8);

	xmlfile.Write(pszMultiByte, ilength);
	xmlfile.Close();
	free(pszMultiByte);
	pszMultiByte = NULL;
//	xmlfile.WriteString(strXmlDefault.GetBuffer(strXmlDefault.GetLength()));
//	xmlfile.Close();

	return TRUE;

}

//企业版2.0曝光量数据统计
BOOL CShowChart::GenExposureCountXml(std::vector<PRODUCTSTATISTICSCHART>& pdata)
{	
	int ilength = -1;
	CString strHtmlStart = _T("<html><head><meta charset=\"utf-8\"></head><body bgcolor=\"#ffffff\" scroll=\"no\"><div id=\"main\" style=\"width:620px; height:300px; padding:1px;\">");
	strHtmlStart += _T(" </div><script src = \"js/echarts.js\"></script><script type=\"text/javascript\">require.config({paths:{echarts: './js'}});");
	strHtmlStart += _T(" require(['echarts','echarts/chart/line'],function(ec){var myChart= ec.init(document.getElementById('main'),'macarons');");
	strHtmlStart += _T(" var option={title:{text:'近一个月产品曝光量统计',x:'center'},tooltip:{trigger:'axis',axisPointer:{ type:'none'}},");
	strHtmlStart += _T(" xAxis:[{name:'日期',type:'category',boundaryGap:false,axisLabel:{show:true,rotate:90,interval:1,textStyle:{fontSize:12,fontWeight:'bold'}},data:[");
	CString strHtmlMid = _T("]}],yAxis:[{name:'数量',type:'value',splitArea: {show: true}}],series:[{name:'曝光量',type:'line',data:[");
	CString strHtmlEnd = _T("]}]}; myChart.setOption(option); });</script></body></html>");
/*	int ilength = -1;*/
	CString strXmlDefault = strHtmlStart+_T("'0'")+strHtmlMid+_T("0")+strHtmlEnd;
	CStdioFile xmlfile;
	if (!xmlfile.Open(m_strHtmlExposureCount,CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("创建XML失败,文件路径%s"), m_strHtmlExposureCount);
		return FALSE;
	}
	
	if (pdata.size() == 0)
	{	
			g_log.Trace(LOGL_TOP,LOGT_WARNING, __TFILE__,__LINE__, _T("用户产品曝光量量统计数据列表数据为空，设置为默认图表！"));

			char* pszMultiByte = CStringToMutilChar(strXmlDefault, ilength, CP_UTF8);

			xmlfile.Write(pszMultiByte, ilength);
			xmlfile.Close();
			free(pszMultiByte);
			pszMultiByte = NULL;
// 			xmlfile.WriteString(strXmlDefault.GetBuffer(strXmlDefault.GetLength()));
// 			xmlfile.Close();

			return TRUE;
	}
	//接下来就是取正常数据

	CString strXContent = _T("");
	CString strYContent = _T("");
	CString strSetContent = _T("");
	for (int m = 0; m < pdata.size(); ++m)
	{
		strXContent += _T("'") + FormatDate(pdata[m].szDate) + _T("',");
		strSetContent.Format(_T("%d,"), pdata[m].iProductCoverCount);
		strYContent += strSetContent;
	}
	strXContent = strXContent.Left(strXContent.GetLength() - 1);
	strYContent = strYContent.Left(strYContent.GetLength() - 1);
	strXmlDefault = strHtmlStart + strXContent + strHtmlMid + strYContent + strHtmlEnd;

	char* pszMultiByte = CStringToMutilChar(strXmlDefault, ilength, CP_UTF8);

	xmlfile.Write(pszMultiByte, ilength);
	xmlfile.Close();
	free(pszMultiByte);
	pszMultiByte = NULL;
// 	xmlfile.WriteString(strXmlDefault.GetBuffer(strXmlDefault.GetLength()));
// 	xmlfile.Close();

	return TRUE;
}



