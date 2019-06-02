/**
*  StringConvert.cpp
*
*/
#include "pch.h"
#include "StringConvert.hpp"
#include <vector>
#ifdef WIN32
#include <windows.h>
#endif

using codecvt_t = std::codecvt<wchar_t, char, std::mbstate_t>;

#if defined(_MSVC_CONVERT_WORKAROUND)
  std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> StringConvert::s_u32Converter;
  std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> StringConvert::s_u16Converter;
#else
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> StringConvert::s_u32Converter;
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> StringConvert::s_u16Converter;
#endif

std::string StringConvert::wideStringToAnsiString(const std::wstring& wstring, const std::locale& locale)
{
  const codecvt_t& codecvt(std::use_facet<codecvt_t>(locale));
  std::mbstate_t state;
  std::vector<char> buffer((wstring.size() + 1) * codecvt.max_length());
  const wchar_t* inNext{wstring.c_str()};
  char* outNext{&buffer[0]};

  std::codecvt_base::result result{codecvt.out(state, wstring.c_str(), wstring.c_str() + wstring.size(), inNext, &buffer[0], &buffer[0] + buffer.size(), outNext)};

  if (result == std::codecvt_base::error)
    throw std::runtime_error("can not convert wstring to string");

  return &buffer[0];
}

std::wstring StringConvert::ansiStringToWideString(const std::string& string, const std::locale& locale)
{
  const codecvt_t& codecvt(std::use_facet<codecvt_t>(locale));
  std::mbstate_t state{};
  std::vector<wchar_t> buffer(string.size() + 1);
  const char* inNext{string.c_str()};
  wchar_t* outNext{&buffer[0]};

  std::codecvt_base::result result{codecvt.in(state, string.c_str(), string.c_str() + string.size(), inNext, &buffer[0], &buffer[0] + buffer.size(), outNext)};

  if (result == std::codecvt_base::error)
    throw std::runtime_error("can not convert string to wstring");

  return &buffer[0];
}

std::u16string StringConvert::u8StringToU16String(const std::string& u8String)
{
#if defined(_MSVC_CONVERT_WORKAROUND)
  auto&& workaround = s_u16Converter.from_bytes(u8String);
  return std::u16string(reinterpret_cast<const char16_t*>(workaround.c_str()));
#else
  return s_u16Converter.from_bytes(u8String);
#endif
}

std::u32string StringConvert::u8StringToU32String(const std::string& u8String)
{
#if defined(_MSVC_CONVERT_WORKAROUND)
  auto&& workaround = s_u32Converter.from_bytes(u8String);
  return std::u32string(reinterpret_cast<const char32_t*>(workaround.c_str()));
#else
  return s_u32Converter.from_bytes(u8String);
#endif
}

std::string StringConvert::u16StringToU8String(const std::u16string& u16String)
{
#if defined(_MSVC_CONVERT_WORKAROUND)
  using workaround_str = std::basic_string<int16_t, std::char_traits<int16_t>, std::allocator<int16_t> >;
  workaround_str workaround(reinterpret_cast<const int16_t*>(u16String.c_str()));
  return s_u16Converter.to_bytes(workaround);
#else
  return s_u16Converter.to_bytes(u16String);
#endif
}

std::u32string StringConvert::u16StringToU32String(const std::u16string& u16String)
{
  std::string temp{u16StringToU8String(u16String)};

  return u8StringToU32String(temp);
}

std::string StringConvert::u32StringToU8String(const std::u32string& u32String)
{
#if defined(_MSVC_CONVERT_WORKAROUND)
  using workaround_str = std::basic_string<int32_t, std::char_traits<int32_t>, std::allocator<int32_t> >;
  workaround_str workaround(reinterpret_cast<const int32_t*>(u32String.c_str()));
  return s_u32Converter.to_bytes(workaround);
#else
  return s_u32Converter.to_bytes(u32String);
#endif
}

std::u16string StringConvert::u32StringToU16String(const std::u32string& u32String)
{
  std::string temp{u32StringToU8String(u32String)};

  return u8StringToU16String(temp);
}


bool StringConvert::utf8CharToUcs2Char(const char* utf8Tok, wchar_t* ucs2Char, uint32_t* utf8TokLen)
{
  //We do math, that relies on unsigned data types
  const unsigned char* utf8TokUs = reinterpret_cast<const unsigned char*>(utf8Tok);

  //Initialize return values for 'return false' cases.
  *ucs2Char = L'?';
  *utf8TokLen = 1;

  //Decode
  if (0x80 > utf8TokUs[0])
  {
    //Tokensize: 1 byte
    *ucs2Char = static_cast<const wchar_t>(utf8TokUs[0]);
  }
  else if (0xC0 == (utf8TokUs[0] & 0xE0))
  {
    //Tokensize: 2 bytes
    if (0x80 != (utf8TokUs[1] & 0xC0))
    {
      return false;
    }
    *utf8TokLen = 2;
    *ucs2Char = static_cast<const wchar_t>(
      (utf8TokUs[0] & 0x1F) << 6
      | (utf8TokUs[1] & 0x3F)
      );
  }
  else if (0xE0 == (utf8TokUs[0] & 0xF0))
  {
    //Tokensize: 3 bytes
    if ((0x80 != (utf8TokUs[1] & 0xC0))
      || (0x80 != (utf8TokUs[2] & 0xC0))
      )
    {
      return false;
    }
    *utf8TokLen = 3;
    *ucs2Char = static_cast<const wchar_t>(
      (utf8TokUs[0] & 0x0F) << 12
      | (utf8TokUs[1] & 0x3F) << 6
      | (utf8TokUs[2] & 0x3F)
      );
  }
  else if (0xF0 == (utf8TokUs[0] & 0xF8))
  {
    //Tokensize: 4 bytes
    *utf8TokLen = 4;
    return false;                        //Character exceeds the UCS-2 range (UCS-4 would be necessary)
  }
  else if ((0xF8 == utf8TokUs[0] & 0xFC))
  {
    //Tokensize: 5 bytes
    *utf8TokLen = 5;
    return false;                        //Character exceeds the UCS-2 range (UCS-4 would be necessary)
  }
  else if (0xFC == (utf8TokUs[0] & 0xFE))
  {
    //Tokensize: 6 bytes
    *utf8TokLen = 6;
    return false;                        //Character exceeds the UCS-2 range (UCS-4 would be necessary)
  }
  else
  {
    return false;
  }

  return true;
}

void StringConvert::ucs2CharToUtf8Char(const wchar_t ucs2Char, char* utf8Tok)
{
  //We do math, that relies on unsigned data types
  uint32_t ucs2CharValue = static_cast<uint32_t>(ucs2Char);   //The standard doesn't specify the signed/unsignedness of wchar_t
  unsigned char* utf8TokUs = reinterpret_cast<unsigned char*>(utf8Tok);

  //Decode
  if (0x80 > ucs2CharValue)
  {
    //Tokensize: 1 byte
    utf8TokUs[0] = static_cast<unsigned char>(ucs2CharValue);
    utf8TokUs[1] = '\0';
  }
  else if (0x800 > ucs2CharValue)
  {
    //Tokensize: 2 bytes
    utf8TokUs[2] = '\0';
    utf8TokUs[1] = static_cast<unsigned char>(0x80 | (ucs2CharValue & 0x3F));
    ucs2CharValue = (ucs2CharValue >> 6);
    utf8TokUs[0] = static_cast<unsigned char>(0xC0 | ucs2CharValue);
  }
  else
  {
    //Tokensize: 3 bytes
    utf8TokUs[3] = '\0';
    utf8TokUs[2] = static_cast<unsigned char>(0x80 | (ucs2CharValue & 0x3F));
    ucs2CharValue = (ucs2CharValue >> 6);
    utf8TokUs[1] = static_cast<unsigned char>(0x80 | (ucs2CharValue & 0x3F));
    ucs2CharValue = (ucs2CharValue >> 6);
    utf8TokUs[0] = static_cast<unsigned char>(0xE0 | ucs2CharValue);
  }
}

std::wstring StringConvert::utf8ToUcs2(const std::string& utf8Str)
{
  std::wstring ucs2Result;
  wchar_t ucs2CharToStrBuf[] = { 0, 0 };
  const char* cursor = utf8Str.c_str();
  const char* const end = utf8Str.c_str() + utf8Str.length();

  while (end > cursor)
  {
    uint32_t utf8TokLen = 1;
    utf8CharToUcs2Char(cursor, &ucs2CharToStrBuf[0], &utf8TokLen);
    ucs2Result.append(ucs2CharToStrBuf);
    cursor += utf8TokLen;
  }

  return ucs2Result;
}

std::string StringConvert::ucs2ToUtf8(const std::wstring& ucs2Str)
{
  std::string utf8Result;
  char utf8Sequence[] = { 0, 0, 0, 0, 0 };
  const wchar_t* cursor = ucs2Str.c_str();
  const wchar_t* const end = ucs2Str.c_str() + ucs2Str.length();

  while (end > cursor)
  {
    const wchar_t ucs2Char = *cursor;
    ucs2CharToUtf8Char(ucs2Char, utf8Sequence);
    utf8Result.append(utf8Sequence);
    cursor++;
  }

  return utf8Result;
}

#ifdef WIN32
std::string StringConvert::UTF8_To_string(const std::string & str)
{
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴 
	memset(pwBuf, 0, nwLen * 2 + 2);

	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	memset(pBuf, 0, nLen + 1);

	WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr = pBuf;

	delete[]pBuf;
	delete[]pwBuf;

	pBuf = NULL;
	pwBuf = NULL;

	return retStr;
}

//------------------------------------------------------------------------ 
std::string StringConvert::string_To_UTF8(const std::string & str)
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴 
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}



std::wstring StringConvert::Utf8ToUnicode(const std::string &strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
	if (len == 0)
	{
		return L"";
	}

	wchar_t *pRes = new wchar_t[len];
	if (pRes == NULL)
	{
		return L"";
	}

	MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, pRes, len);
	pRes[len - 1] = L'\0';
	std::wstring result = pRes;
	delete[] pRes;

	return result;
}

std::string StringConvert::UnicodeToUtf8(const std::wstring &strUnicode)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, strUnicode.c_str(), -1, NULL, 0, NULL, NULL);
	if (len == 0)
	{
		return "";
	}

	char *pRes = new char[len];
	if (pRes == NULL)
	{
		return "";
	}

	WideCharToMultiByte(CP_UTF8, 0, strUnicode.c_str(), -1, pRes, len, NULL, NULL);
	pRes[len - 1] = '\0';
	std::string result = pRes;
	delete[] pRes;

	return result;
}

std::wstring StringConvert::StringToWString(const std::string &str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	if (len == 0)
	{
		return L"";
	}

	wchar_t *pRes = new wchar_t[len];
	if (pRes == NULL)
	{
		return L"";
	}

	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, pRes, len);
	pRes[len - 1] = L'\0';
	std::wstring result = pRes;
	delete[] pRes;

	return result;
}

std::string StringConvert::WStringToString(const std::wstring &wstr)
{
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	if (len == 0)
	{
		return "";
	}

	char *pRes = new char[len];
	if (pRes == NULL)
	{
		return "";
	}

	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, pRes, len, NULL, NULL);
	pRes[len - 1] = '\0';
	std::string result = pRes;
	delete[] pRes;

	return result;
}
#endif