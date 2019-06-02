/**
*  StringConvert.hpp
*
*/

#ifndef STRING_CONVERT_HPP
#define STRING_CONVERT_HPP

#include <string>
#include <locale>
#include <codecvt>

//https://blog.csdn.net/Deft_MKJing/article/details/79460485
//https://baijiahao.baidu.com/s?id=1619013475460656422&wfr=spider&for=pc
class StringConvert final
{
public :

  // Delete destructor
  ~StringConvert() = delete;

  // WBCS to MBCS
  static std::string wideStringToAnsiString(const std::wstring& wstring, const std::locale& locale = std::locale{""});
  // MBCS to WBCS
  static std::wstring ansiStringToWideString(const std::string& string, const std::locale& locale = std::locale{""});
  // UTF-8 -> UTF-16
  static std::u16string u8StringToU16String(const std::string& u8String);
  // UTF-8 -> UTF-32
  static std::u32string u8StringToU32String(const std::string& u8String);
  // UTF-16 -> UTF-8
  static std::string u16StringToU8String(const std::u16string& u16String);
  // UTF-16 -> UTF-32
  static std::u32string u16StringToU32String(const std::u16string& u16String);
  // UTF-32 -> UTF-8
  static std::string u32StringToU8String(const std::u32string& u32String);
  // UTF-32 -> UTF-16
  static std::u16string u32StringToU16String(const std::u32string& u32String);


  std::wstring utf8ToUcs2(const std::string& utf8Str);
  std::string ucs2ToUtf8(const std::wstring& ucs2Str);
  bool utf8CharToUcs2Char(const char* utf8Tok, wchar_t* ucs2Char, uint32_t* utf8TokLen);
  void ucs2CharToUtf8Char(const wchar_t ucs2Char, char* utf8Tok);


#ifdef WIN32

  //转换过程：先将utf8转双字节Unicode编码，再通过WideCharToMultiByte将宽字符转换为多字节
  std::string UTF8_To_string(const std::string & str);
  std::string string_To_UTF8(const std::string & str);



  std::wstring Utf8ToUnicode(const std::string &strUTF8);
  std::string UnicodeToUtf8(const std::wstring &strUnicode);
  std::wstring StringToWString(const std::string &str);
  std::string WStringToString(const std::wstring &wstr);
#endif

private :

#if defined(_MSC_VER) && _MSC_VER >= 1900
  // https://social.msdn.microsoft.com/Forums/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
  #define _MSVC_CONVERT_WORKAROUND
#endif
  // Member data
#if defined(_MSVC_CONVERT_WORKAROUND)
  static std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> s_u32Converter;
  static std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> s_u16Converter;
#else
  static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> s_u32Converter;
  static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> s_u16Converter;
#endif
};

#endif // STRING_CONVERT_HPP
