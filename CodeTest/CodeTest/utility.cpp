#include "pch.h"
#include <iostream>
#include "utility.h"

#include "log4z/log4z.h"
using namespace zsummer::log4z;

#include "tinyxml2/tinyxml2.h"
using namespace tinyxml2;

#include "stringConvert/StringConvert.hpp"


CUtility::CUtility()
{

}
CUtility::~CUtility()
{
	
}

void CUtility::log4zTest()
{
	ILog4zManager::getRef().enableLogger(LOG4Z_MAIN_LOGGER_ID, false);
	m_logId = ILog4zManager::getRef().createLogger("log4z");
	ILog4zManager::getRef().setLoggerPath(m_logId, "./log");
	ILog4zManager::getRef().setLoggerLimitsize(m_logId, 3);	//限制单个日志文件的大小，单位M
	ILog4zManager::getRef().prePushLog(m_logId, LOG_LEVEL_DEBUG);
	ILog4zManager::getRef().start();

	LOG_INFO(m_logId, "*** *** " << "this is log4z test" << " *** ***");

	ILog4zManager::getRef().stop();
}
void CUtility::tinyxml2Test()
{
	{
		static const char* xml =
			"<information>"
			"	<attributeApproach v='2' />"
			"	<textApproach>"
			"		<v>2</v>"
			"	</textApproach>"
			"</information>";

		XMLDocument doc;
		doc.Parse(xml);

		int v0 = 0;
		int v1 = 0;

		XMLElement* attributeApproachElement = doc.FirstChildElement()->FirstChildElement("attributeApproach");
		attributeApproachElement->QueryIntAttribute("v", &v0);

		XMLElement* textApproachElement = doc.FirstChildElement()->FirstChildElement("textApproach");
		textApproachElement->FirstChildElement("v")->QueryIntText(&v1);

		int iErrorCode = doc.ErrorID();
		printf("Both values are the same: %d and %d\n", v0, v1);
	}

	{
		// This test is pre-test for the next one
		// (where Element1 is inserted "after itself".
		// This code didn't use to crash.
		XMLDocument doc;
		XMLElement* element1 = doc.NewElement("Element1");
		XMLElement* element2 = doc.NewElement("Element2");
		doc.InsertEndChild(element1);
		doc.InsertEndChild(element2);
		doc.InsertAfterChild(element2, element2);
		doc.InsertAfterChild(element2, element2);
	}

	{
		XMLDocument doc;
		doc.LoadFile("../Common/tinyxml2/resources/dream.xml");
		doc.SaveFile("../Common/tinyxml2/resources/out/dreamout.xml");
	}
}

void CUtility::StringConvertTest()
{
	const TCHAR* pwcTest(L"This is a wChar");
	std::string sTest = StringConvert::wideStringToAnsiString(pwcTest);
	std::cout << sTest << std::endl;

	//C++ std::string存储的是单字节字符，对于中文编码，编码的时候一般是将中文字变成2个字节的gb2312后存储到std::string里面
	std::string sTest01 = "hello世界";
	int ilen = sTest01.length();
	std::cout << "hello世界" << "  是" << ilen << "个字节" << std::endl;
//	ucs2ToUtf8(const std::wstring& ucs2Str)
}
