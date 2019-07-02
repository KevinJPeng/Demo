#include "pch.h"
#include <iostream>
#include "utility.h"

//log4z
#include "log4z/log4z.h"
using namespace zsummer::log4z;

//tinyXml
#include "tinyxml2/tinyxml2.h"
using namespace tinyxml2;

//StringConvert
#include "stringConvert/StringConvert.hpp"

//Json
#include "Json/JSON.h"


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
	ILog4zManager::getRef().setLoggerLimitsize(m_logId, 3);	//���Ƶ�����־�ļ��Ĵ�С����λM
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

	//C++ std::string�洢���ǵ��ֽ��ַ����������ı��룬�����ʱ��һ���ǽ������ֱ��2���ֽڵ�gb2312��洢��std::string����
	std::string sTest01 = "hello����";
	int ilen = sTest01.length();
	std::cout << "hello����" << "  ��" << ilen << "���ֽ�" << std::endl;
//	ucs2ToUtf8(const std::wstring& ucs2Str)
}


void CUtility::JsonTest()
{
	JsonObjectBuilder builder;
	builder.Append("boolean1", true);
	builder.Append("boolean2", false);
	builder.Append("number1", 100);
	builder.Append("number2", 1.1);
	builder.Append("string", "�ҵ�JsonLite");
	builder.Append("string1", "123\"456");
	builder.Append("string2", "123\\456");
	builder.Append("string3", "123/456");
	//builder.Append("string4",  "123\b456");
	//builder.Append("string5",  "123\f456");
	builder.Append("string6", "123\n456");
	//builder.Append("string7",  "123\r456");
	builder.Append("string8", "123\t456");
	builder.Append("string9", "\\u1234\\u4567");
	builder.Append("object",
		JsonObjectBuilder()
		.Append("boolean1", true)
		.Append("boolean2", false)
		.Build()
		);
	builder.Append("array",
		JsonArrayBuilder()
		.Append(true)
		.Append(false)
		.Append(100)
		.Append(1.1)
		.Append("xyz")
		.Build()
		);
	JString str0 = builder.Build()["string"].GetString();
	JString text0 = JSON::Format(builder.Build(), 2);

	//JsonValue json = JsonParser().Parse("{ \"name\": \"100\", \"boolean\": true }");
	JsonValue json = JSON::Parse(text0);
	JString str1 = json["string1"].GetString();
	JString str2 = json["string2"].GetString();
	JString str3 = json["string3"].GetString();
	//JString str4  = json["string4"].GetString();
	//JString str5  = json["string5"].GetString();
	JString str6 = json["string6"].GetString();
	//JString str7  = json["string7"].GetString();
	JString str8 = json["string8"].GetString();
	JString str9 = json["string9"].GetString();
	JString text1 = json.ToString();
	JString text2 = JSON::Format(json, 4);

	//puts(str0);
	puts(text0);

	printf("string1: %s\n", (char*)str1);
	printf("string2: %s\n", (char*)str2);
	printf("string3: %s\n", (char*)str3);
	//printf("string4: %s\n", (char*)str4);
	//printf("string5: %s\n", (char*)str5);
	printf("string6: %s\n", (char*)str6);
	//printf("string7: %s\n", (char*)str7);
	printf("string8: %s\n", (char*)str8);
	printf("string9: %s\n", (char*)str9);

	//puts(text1);
	puts(text2);

//	_getch();
}
