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

//cryptopp
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "cryptopp6.0.0/include/md5.h"
#include "cryptopp6.0.0/include/modes.h"
#include "cryptopp6.0.0/include/files.h"
#include "cryptopp6.0.0/include/hex.h"
using namespace CryptoPP;
#ifdef _DEBUG
#pragma comment(lib, "cryptopp6.0.0/lib/MDd/cryptlib.lib")
#else
#pragma comment(lib, "cryptopp6.0.0/lib/MD/cryptlib.lib")
#endif

using namespace std;

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


void CUtility::JsonTest()
{
	JsonObjectBuilder builder;
	builder.Append("boolean1", true);
	builder.Append("boolean2", false);
	builder.Append("number1", 100);
	builder.Append("number2", 1.1);
	builder.Append("string", "我的JsonLite");
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
void CUtility::HexPrint(std::string datas)
{
	cout.fill('0');//设置有效位空白字符填充字符'0'，只对有效位空白字符有效，一般结合cout.width()一起使用，对\t和\n无效
	int datasize = datas.size();
	for (int i = 0; i < datasize; i++)
	{
		cout.width(2);	//显示固定2位数十六进制数，不足前面补零
		std::cout << std::hex << std::uppercase << (0xFF & static_cast<uint8_t>(datas.at(i)));
	}
	std::cout << std::dec << std::endl;
	return;
}

//======================== MD5算法====================
std::string CUtility::MD5En(std::string datas)
{
	int len = datas.length();
	uint8_t *message = new uint8_t[len + 1];
	memcpy(message, datas.c_str(), len + 1);
	message[len] = 0;
	//MD5生成的信息摘要固定长度位（16个字节）
	const size_t size = CryptoPP::Weak::MD5::DIGESTSIZE;
	uint8_t digest[size] = { 0 };//128 bits=16bytes
	CryptoPP::Weak::MD5 md5;
	md5.CalculateDigest(digest, message, len);
		return std::string(reinterpret_cast<const char*>(digest), size);
}
std::string CUtility::MD5Source()
{
	CryptoPP::Weak::MD5 md;
	const size_t size = CryptoPP::Weak1::MD5::DIGESTSIZE * 2;//32
	uint8_t buf[size] = { 0 };
	std::string strPath = "d:\\a.dat";
	CryptoPP::FileSource(strPath.c_str(), true,
		new CryptoPP::HashFilter(md,
		new CryptoPP::HexEncoder(
		new CryptoPP::ArraySink(buf, size))));
	return std::string(reinterpret_cast<const char*>(buf), size);
}
void CUtility::CryptoTest()
{
	byte message[] = "HelloWorld!";
	byte mres[16];//MD5 128 bits=16bytes
	Weak::MD5 md5;
	md5.Update(message, 11);//strlen=11
	md5.Final(mres);
 	string smd5;
	for (int i = 0; i < 16; i++)
		printf("%02X", mres[i]);
	printf("\r\n");

	string sdatas = "HelloWorld!";
	std::string md5str = MD5En(sdatas);
	HexPrint(md5str);

// 	string encoded("");
// 	CryptoPP::StringSource ss(bin, length, true, new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)));
}

