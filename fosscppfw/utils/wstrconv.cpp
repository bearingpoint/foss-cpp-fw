#include "wstrconv.h"
#ifdef __WIN32__
#	define UNICODE
#	include <windows.h>
#endif
#include <locale>
#include <codecvt>

std::wstring str2Wstr(std::string const& input) {
#ifdef __WIN32__
	std::wstring ret;
	int len = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), input.size(), NULL, 0);
	if (len > 0) {
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, input.c_str(), input.size(), &ret[0], len);
	}
	return ret;
#else
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8Converter;
	return utf8Converter.from_bytes( input );
#endif
}