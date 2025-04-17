/*
 * strManip.h
 *
 *  Created on: Nov 13, 2015
 *	  Author: bog
 */

#include <string>
#include <vector>
#include <regex>

/** counts the number of separate words (using ' ' as separator)*/
unsigned getNumberOfWords(std::string const& str);
/** counts how many times the given regex matches within the string */
int regexCount(std::string const& str, std::regex const& rx);

/** splits a string given a separator */
std::vector<std::string> strSplit(std::string const& text, char sep);
/** splits a string at all positions where the given regex matches (the regex is used as separator). */
std::vector<std::string> strSplitByRegex(std::string str, std::regex r);

/** Joins a list of strings adding the "delimiter" between them */
template<typename T>
std::string strJoin(std::vector<T> const& parts, std::string const& delimiter);

/**
 * Splits a string by spaces but keeping all quoted parts intact.
 * For example: "a string with \"quoted parts within\" is split into":
 * {"a", "string", "with", "quoted parts within", "is", "split", "into" }
 *                          ^^^^^^^^^^^^^^^^^^^
*/
std::vector<std::string> strSplitPreserveQuotes(std::string const& text, std::vector<char> const& sep);
/** Transforms a string into lowercase */
std::string strLower(std::string const& str);
/** Transforms a string into uppercase */
std::string strUpper(std::string const& str);

/** Converts all strings in the list to lowercase, in place */
void makeLowercase(std::vector<std::string>& strings);

/** Replaces all occurrences of [what] with [replacement] directly into the target string. */
void replaceAllSubstr(std::string &str, std::string const& what, std::string const& replacement);
std::string trim(const std::string& str);

/**
 * @file strManip.h
 * @brief Decode a base64 encoded string
 * @version 0.1
 * @date 2024-05-27
 *
 */
std::string base64_decode(const std::string &encoded_string);


/**
 * @brief Generates a random hexadecimal string
 * @date 2025-04-17
 */
std::string generateRandomHexString(size_t numBytes);

#include "_strManip_private.h"
