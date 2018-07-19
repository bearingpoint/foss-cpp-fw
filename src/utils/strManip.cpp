/*
 * strManip.cpp
 *
 *  Created on: Nov 13, 2015
 *      Author: bog
 */

#include <boglfw/utils/strManip.h>

#include <algorithm>
#include <locale>

std::vector<std::string> strSplit(std::string const& text, char sep) {
	return strSplit(text, std::vector<char>{sep});
}

std::vector<std::string> strSplit(std::string const& text, std::vector<char> const& sep) {
	std::vector<std::string> tokens;
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

std::vector<std::string> strSplitPreserveQuotes(std::string const& text, std::vector<char> const& sep) {
	auto q = strSplit(text, '"');
	std::vector<std::string> res;
	for (int i=0; i<q.size(); i++) {
		if (i%2)	// odd parts are between quotes, we push them as-is
			res.push_back(q[i]);
		else {
			auto qc = strSplit(q[i], sep);	// even parts must be split by usual separators
			for (auto w : qc)
				res.push_back(w);	// and pushed word by word
		}
	}
	return res;
}

std::string strLower(std::string const& str) {
	std::locale loc("en_US.UTF8");
	auto &f = std::use_facet<std::ctype<char> >(loc);
	std::string res(str.c_str());
	f.tolower(const_cast<char*>(res.c_str()), res.c_str() + res.length());
	return res;
}

std::string strUpper(std::string const& str) {
	std::locale loc("en_US.UTF8");
	auto &f = std::use_facet<std::ctype<char> >(loc);
	std::string res(str.c_str());
	f.toupper(const_cast<char*>(res.c_str()), res.c_str() + res.length());
	return res;
}

void replaceAllSubstr(std::string &str, std::string const& what, std::string const& replacement) {
	size_t pos = 0;
	while ((pos = str.find(what, pos)) != str.npos) {
		str.replace(str.begin()+pos, str.begin()+pos+what.length(), replacement);
		pos += replacement.length();
	}
}

unsigned getNumberOfWords(std::string const& str) {
	return std::count(str.begin(), str.end(), ' ');
}
