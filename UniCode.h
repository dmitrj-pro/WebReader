#pragma once
#include <string>
#include <locale>
#include <codecvt>
#include <DPLib.conf.h>
	
using __DP_LIB_NAMESPACE__::String;	


inline std::u16string wstrFroStr(const std::string & str) {
	//std::u16string wstr(str.begin(), str.end());
	//return wstr;
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.from_bytes(str);
}

inline std::string strFromWStr(const std::u16string & str) {
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.to_bytes(str);
	//const std::string s( str.begin(), str.end() );
	//return s;
}

inline std::wstring wwstrFroStr(const std::string & str) {
	std::wstring wstr(str.begin(), str.end());
	return wstr;
	//std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	//return converter.from_bytes(str);
}

inline std::string strFromWwStr(const std::wstring & str) {
	//std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	//return converter.to_bytes(str);
	const std::string s( str.begin(), str.end() );
	return s;
}

inline String normalizeString(const String & str) {
	return strFromWwStr(wwstrFroStr(str));
}

String toBase64(const String & str0);

String fromBase64(const String & str);
