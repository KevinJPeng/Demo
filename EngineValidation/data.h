#ifndef _DATA_H_
#define _DATA_H_

#include "stdafx.h"
#include <iostream>

const TCHAR ac_KeyWord[] =
{
	'a', 'b', 'c', 'd'//, 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 
//	'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
//	'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
//	'+', '-', '*', '/', '!', '@', '#', '$', '%', '^', '&', '<'
};


const CString as_EngineUrl[] =
{
	_T("http://www.baidu.com/s?wd=%s&pn=%d"),
	_T("http://www.so.com/s?q=%s&pn=%d"),
	_T("http://www.sogou.com/web?query=%s&amp;page=%d"),
	_T("http://cn.bing.com/search?q=%s&first=%d"),
	_T("http://wap.baidu.com/s?word=%s&pn=%d"),
	_T("http://m.so.com/s?q=%s&amp;pn=%d&mode=jisu"),
	_T("http://wap.sogou.com/web/searchList.jsp?keyword=%s&p=%d"),
	_T("http://m.sm.cn/s?q=%s&pn=%d")
};

#endif



#if 0
const CString as_EngineUrl[] =
{
	_T("http://www.baidu.com/s?wd=%s&amp;pn=%d"),
	_T("http://www.so.com/s?q=%s&amp;pn=%d"),
	_T("http://www.sogou.com/web?query=%s&amp;page=%d"),
	_T("http://cn.bing.com/search?q=%s&amp;first=%d"),
	_T("http://wap.baidu.com/s?word=%s&amp;pn=%d"),
	_T("http://m.so.com/s?q=%s&amp;pn=%d&amp;mode=jisu"),
	_T("http://wap.sogou.com/web/searchList.jsp?keyword=%s&amp;p=%d"),
	_T("http://m.sm.cn/s?q=%s&amp;pn=%d")
};
#endif