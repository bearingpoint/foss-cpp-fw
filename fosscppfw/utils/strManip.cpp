/*
 * strManip.cpp
 *
 *  Created on: Nov 13, 2015
 *	  Author: bog
 */

#include "strManip.h"

#include <algorithm>
#include <cctype>

template <class TCHAR, class strType = std::basic_string<TCHAR>>
std::vector<strType> strSplitImpl(strType const& text, std::vector<TCHAR> const& sep) {
	std::vector<strType> tokens;
	size_t start = 0, end = 0;
	do {
		end = text.npos;
		for (auto c : sep) {
			auto end1 = text.find(c, start);
			if (end1 < end)
				end = end1;
		}
		if (end != text.npos) {
			tokens.push_back(text.substr(start, end - start));
			start = end + 1;
		}
	} while (end != text.npos);

	tokens.push_back(text.substr(start));
	return tokens;
}

std::vector<std::string> strSplit(std::string const& text, char sep) {
	return strSplitImpl<char>(text, {sep});
}

std::vector<std::wstring> strSplit(std::wstring const& text, wchar_t sep) {
	return strSplitImpl<wchar_t>(text, {sep});
}

std::vector<std::string> strSplitPreserveQuotes(std::string const& text, std::vector<char> const& sep) {
	auto q = strSplit(text, '"');
	std::vector<std::string> res;
	for (uint i=0; i<q.size(); i++) {
		if (i%2)	// odd parts are between quotes, we push them as-is
			res.push_back(q[i]);
		else {
			auto qc = strSplitImpl<char>(q[i], sep);	// even parts must be split by usual separators
			for (auto &w : qc)
				res.push_back(w);	// and pushed word by word
		}
	}
	return res;
}

std::vector<std::string> strSplitByRegex(std::string str, std::regex r) {
	std::vector<std::string> parts;
	std::sregex_token_iterator iter(str.begin(), str.end(), r, -1);
	std::sregex_token_iterator end;

	while (iter != end) {
		if (iter->length()) {
			parts.push_back(*iter);
		}
		++iter;
	}
	if (parts.empty()) {
		parts.push_back(str); // separator did not match
	}
	return parts;
}

std::string strLower(std::string const& str) {
	std::string result;
	std::transform(str.begin(), str.end(), std::back_inserter(result), [] (char c) {
		return std::tolower(c);
	});
	return result;
}

std::string strUpper(std::string const& str) {
	std::string result;
	std::transform(str.begin(), str.end(), std::back_inserter(result), [] (char c) {
		return std::toupper(c);
	});
	return result;
}

template <class TCHAR, class strType = std::basic_string<TCHAR>>
void replaceAllSubstrImpl(strType &str, strType const& what, strType const& replacement) {
	size_t pos = 0;
	while ((pos = str.find(what, pos)) != str.npos) {
		str.replace(str.begin()+pos, str.begin()+pos+what.length(), replacement);
		pos += replacement.length();
	}
}

void replaceAllSubstr(std::string &str, std::string const& what, std::string const& replacement) {
	return replaceAllSubstrImpl<char>(str, what, replacement);
}
void replaceAllSubstr(std::wstring &str, std::wstring const& what, std::wstring const& replacement) {
	return replaceAllSubstrImpl<wchar_t>(str, what, replacement);
}


unsigned getNumberOfWords(std::string const& str) {
	return std::count(str.begin(), str.end(), ' ');
}

int regexCount(std::string const& str, std::regex const& rx) {
	std::sregex_iterator itEnd;
	std::sregex_iterator it(str.begin(), str.end(), rx);
	int count = 0;
	while (it != itEnd) {
		count++, ++it;
	}
	return count;
}

std::string trim(const std::string& str) {
	size_t first = str.find_first_not_of(" \t\n\r");
	size_t last = str.find_last_not_of(" \t\n\r");
	if (first == std::string::npos || last == std::string::npos) {
		return "";
	}
	return str.substr(first, last - first + 1);
}

std::string base64_decode(const std::string &encoded_string) {
	const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string decoded_string;
	int val = 0, bits = -8;

	for (char c : encoded_string) {
		if (c == '=')
			break;
		if (std::isspace(c))
			continue;

		val = (val << 6) + base64_chars.find(c);
		bits += 6;

		if (bits >= 0) {
			decoded_string.push_back(char((val >> bits) & 0xFF));
			bits -= 8;
		}
	}
	return decoded_string;
}
