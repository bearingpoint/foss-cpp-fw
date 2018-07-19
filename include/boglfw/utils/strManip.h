/*
 * strManip.h
 *
 *  Created on: Nov 13, 2015
 *      Author: bog
 */

#include <string>
#include <vector>

unsigned getNumberOfWords(std::string const& str);
std::vector<std::string> strSplit(std::string const& text, char sep);
std::vector<std::string> strSplit(std::string const& text, std::vector<char> const& sep);
std::vector<std::string> strSplitPreserveQuotes(std::string const& text, std::vector<char> const& sep);
std::string strLower(std::string const& str);
std::string strUpper(std::string const& str);
void replaceAllSubstr(std::string &str, std::string const& what, std::string const& replacement);
