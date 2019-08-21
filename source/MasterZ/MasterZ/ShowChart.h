#ifndef _SHOW_CHART_H_
#define _SHOW_CHART_H_
#include "UIlib.h"
#include "../../duilib/Utils/flash11.tlh"
#include "ClientInterface.h"
//显示相关图表类
class CShowChart
{
public:
	CShowChart();
	~CShowChart();
//下面接口用来显示出图表
public:
	//界面管理指针
	void SetPaintManager(CPaintManagerUI*pPaintMgr, HWND hwnd, CWebBrowserUI* pWeb);

	//曝光量控件指针
	void SetWebPoint(CWebBrowserUI* pWeb);

	//显示用户关注数据
	BOOL LoadUserCareChart(WEIXINTONG_DATA& pdata);
	//显示用户请求数据
	BOOL LoadUserRequestChart(WEIXINTONG_DATA& pdata);

	//企业版2.0
	//显示企业版2.0上线关键词数据统计
	BOOL LoadKeyWordCountChart(std::vector<KEYWORDSTATISTICCHAR>& pdata);
	//显示企业版2.0产品曝光量统计
	BOOL LoadExposureCountChart(std::vector<PRODUCTSTATISTICSCHART>& pdata);

private:
	//生成用户关注数据数据源
	BOOL GenUserCareXml(WEIXINTONG_DATA& pdata);
	//生成用户请求数据数据源
	BOOL GenUserRequestXml(WEIXINTONG_DATA& pdata);

	//企业版2.0上线关键词统计数据源
	BOOL GenKeyWordCountXml(std::vector<KEYWORDSTATISTICCHAR>& pdata);
	//企业版2.0曝光量数据统计
	BOOL GenExposureCountXml(std::vector<PRODUCTSTATISTICSCHART>& pdata);

private:
	CPaintManagerUI *m_PaintManager;
	HWND m_hwnd;						//主窗口句柄
	CWebBrowserUI	*m_pActiveX;		//第一个关键词数量的窗口
	CWebBrowserUI	*m_pActiveXExposure;		//第二个曝光量数量的窗口
// 	CString m_strXmlUserCare;
// 	CString m_strXmlUserRequest;
// 	CString m_strXmlKeyWordCount;
// 	CString m_strXmlExposureCount;
	CString m_strHtmlUserCare;
	CString m_strHtmlUserRequest;
	CString m_strHtmlKeyWordCount;
	CString m_strHtmlExposureCount;
	CString m_strHtmlEmptyKeyWordPath;					//没有关键词数据的网页路径
	CString m_strHtmlEmptyExposurePath;					//没有曝光量数据的网页路径
};
#endif