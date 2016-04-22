/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file SecureString.hpp
 */

#ifndef SECURESTRING_HPP_
#define SECURESTRING_HPP_

#include <string>
#include <vector>

/*!
 * @brief namespace OpenAB
 */
namespace OpenAB {

/*!
 * @brief SecureString class, used to store strings like passwords etc.
 * Current implementation encodes string into two strings using simple XOR encoding.
 * @todo Currently this class in not secure at all, but in future may be extended to keep strings in memory in more secured way.
 */
class SecureString
{
  public:
    /**
     * @brief Default constructor.
     */
    SecureString();

    /**
     * @brief Constructor.
     * @param [in] str string to be managed by this object.
     */
    SecureString(const char* str);

    /**
     * @brief Constructor.
     * @param [in] str string to be managed by this object.
     */
    SecureString(const std::string& str);

    /*!
     *  @brief Copy constructor.
     */
    SecureString(const SecureString& other);

    /**
     * @brief Assignment operator.
     */
    SecureString& operator= (const SecureString &other);

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~SecureString();

    /*!
     * @brief Returns managed string.
     * When returned string is no longer needed, clearStr should be called.
     * @return managed string.
     */
    char* str() const;

    /*!
     * @brief Clears internal state after calling str().
     * To be used when string returned by str() is no longer needed.
     */
    void clearStr() const;

    /*!
     * @brief Clears managed string.
     */
    void clear();

    /**
     * @brief Comparison operator.
     */
    bool operator== (const SecureString &other) const;

  private:
    void generateKey(unsigned int size);

    std::vector<unsigned char> key;
    std::vector<unsigned char> content;
    mutable char *decoded;
};

} // namespace OpenAB

#endif // SECURESTRING_HPP_
