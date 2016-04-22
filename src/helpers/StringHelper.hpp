/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file StringHelper.hpp
 */

#ifndef STRINGHELPER_HPP_
#define STRINGHELPER_HPP_
#include <string>
#include <vector>

/*!
 * @brief namespace OpenAB
 */
namespace OpenAB {

/*!
 * @brief Removes whitespaces from beginning and end of given string
 * @param [in,out] str string from which whitespaces should be removed.
 */
void trimWhitespaces (std::string& str);

/*!
 * @brief Removes whitespaces and spaces from beginning and end of given string
 * @param [in,out] str string from which spaces should be removed.
 */
void trimSpaces (std::string& str);

/*!
 * @brief Tokenizes string.
 * Uses provided delimiter for tokens, process can be controlled by behavior in case of duplicated and empty tokens.
 * @param [in] str string to be tokenized.
 * @param [in] delimiter delimiter of tokens.
 * @param [in] unique should duplicated tokens be removed.
 * @param [in] leaveEmptyTokens should empty tokens be keep.
 * @return vector of tokens.
 */
std::vector<std::string> tokenize(const std::string& str, char delimiter, bool unique = false, bool leaveEmptyTokens = true);

/*!
 * @brief Checks if given string is contained in vector of strings.
 * @param [in] vec vector of string to be checked.
 * @param [in] str string to be searched in vector of strings.
 * @return true if string was found in vector, false otherwise.
 */
bool contains(const std::vector<std::string>& vec, const std::string& str);

/*!
 * @brief Checks if given string begins with given substring.
 * @param [in] str string to be checked.
 * @param [in] substr substring to be checked.
 * @return true if string begins with given substring, false otherwise.
 */
bool beginsWith (const std::string& str, const std::string& substr);

/*!
 * @brief Checks if given string ends with given substring.
 * @param [in] str string to be checked.
 * @param [in] substr substring to be checked.
 * @return true if string ends with given substring, false otherwise.
 */
bool endsWith (const std::string& str, const std::string& substr );

/*!
 * @brief Removes all occurences of given character from string.
 * @param [in,out] str string from which all occurences of character should be removed.
 * @param [in] toRemove character to be removed.
 */
void eraseAllOccurences(std::string& str, char toRemove);

/*!
 * @brief Substitutes all occurrences of given substring.
 * @param [in,out] str string in which substring should be substituted.
 * @param [in] from substring that should be substituted.
 * @param [in] to string to which substring should be substituted.
 * @todo Performance for large strings is not good.
 */
void substituteAll (std::string& str, const std::string& from, const std::string& to);

/*!
 * @brief Cuts part of string contained between two substrings.
 * @param [in] str string from which part should be cut.
 * @param [in] begin beginning substring
 * @param [in] end ending substring
 * @param [in,out] pos position from which beginning substring should be searched,
 * after cutting it will point to position after end substring. If no beginning/end substring will be found it will be set to std::string::npos.
 * @return cut substring or empty string.
 */
std::string cut(const std::string& str, const std::string& begin, const std::string& end, std::string::size_type& pos);

/*!
 * @brief Parses URL into parts.
 * URL is parsed into four parts: scheme (http, https, ftp, etc.), host, path and query,
 * eg. http://www.google.com/search?q=test will be parsed into:
 * - http
 * - www.google.com
 * - search
 * - q=test
 * @param [in] url URL to be parsed
 * @return vector of four strings containing parts of parsed URL (scheme, host, path, query).
 * Returned vector will always have size of four, some of them may be empty if provided URL does not contain all parts described above.
 */
std::vector<std::string> parserURL(const std::string& url);

/*!
 * @brief Parses host part out of URL.
 * @param [in] url url to be parsed.
 * @return concatenated scheme and host parts of original URL.
 */
std::string parseURLHostPart(const std::string& url);

/*!
 * @biref Reads one line from stream and unflods it using line folding rule defined in RFC2425 -5.8.1
 * @param [in] is input stream from which line should be read
 * @param [oun] str output of unfolded line
 * @return false if input stream has ended, true otherwise
 */
bool getUnfoldedLine(std::istream& is, std::string& str);

/*!
 * @brief Unfolds all lines in provided string. check if it does not duplicates functionality from getUnfoldedLine
 * @param [in, out] string to be unfolded
 */
void linearize(std::string& str);

/*!
 * @brief Removes quotation of special characters e.g. converts "\," into ","
 * @param [in, out] string from which special characters should be unquoted
 */
void unquoteSpecialCharacters(std::string& str);

std::string percentDecode(const std::string& uri);


} // namespace OpenAB

#endif // STRINGHELPER_HPP_
