/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Variant.hpp
 */

#ifndef VARIANT_H_
#define VARIANT_H_

#include <sys/types.h>
#include <string>
#include "SecureString.hpp"

namespace OpenAB {
/**
 * @defgroup  libOpenAB  OpenAB Library
 *
 */

/*!
 * @brief Variant class that can contains different types of data
 */
class Variant
{
  public:
    /**
     *  @brief Constructor.
     *  Creates invalid Variant.
     */
    Variant();

    /**
     *  @brief Constructor.
     *  @param [in] value char value to be stored in Variant
     */
    Variant(char value);

    /**
     *  @brief Constructor.
     *  @param [in] value bool value to be stored in Variant
     */
    Variant(bool value);

    /**
     *  @brief Constructor.
     *  @param [in] value 4-bytes unsigned integer value to be stored in Variant
     */
    Variant(u_int32_t value);

    /**
     *  @brief Constructor.
     *  @param [in] value 4-byte signed integer value to be stored in Variant
     */
    Variant(int32_t value);

    /**
     *  @brief Constructor.
     *  @param [in] value double value to be stored in Variant
     */
    Variant(double value);

    /**
     *  @brief Constructor.
     *  @param [in] value std::string value to be stored in Variant
     */
    Variant(const std::string& value);

    /**
     *  @brief Constructor.
     *  @param [in] value pointer value to be stored in Variant
     */
    Variant(void* value);

    /**
     *  @brief Constructor.
     *  @param [in] value const pointer value to be stored in Variant
     */
    Variant(const char* value);

    /**
     *  @brief Constructor.
     *  @param [in] value OpenAB::SecureString value to be stored in Variant
     */
    Variant(const OpenAB::SecureString& value);

    /**
     *  @brief Copy constructor.
     */
    Variant(Variant const &other);

    /**
     *  @brief Destructor, virtual by default.
     */
    virtual ~Variant();

    /*!
     *  @brief Assignment operator.
     */
    Variant& operator=(Variant const &other);

    /*!
     *  @brief Compare operator.
     */
    bool operator==(Variant const &other);

    /** @enum
     * Data types that can be stored by Variant
     */
    enum DataType
    {
      INVALID,        /**< invalid type, used to indicate that Variant is not initialized */
      CHAR,           /**< char type */
      BOOL,           /**< bool type */
      INTEGER,        /**< 4-bytes signed integer type */
      DOUBLE,         /**< double type */
      STRING,         /**< string type */
      SECURE_STRING,  /**< secure string eg. holding password */
      POINTER,        /**< pointer type */
    };

    /**
     * Sets char value to be stored in Variant.
     * @param [in] value char value to be stored in Variant.
     */
    void set(char value);

    /**
     * Sets bool value to be stored in Variant.
     * @param [in] value bool value to be stored in Variant.
     */
    void set(bool value);


    /**
     * Sets 4-bytes signed integer value to be stored in Variant.
     * @param [in] value 4-bytes signed integer value to be stored in Variant.
     */
    void set(int32_t value);

    /**
     * Sets 4-bytes unsigned integer value to be stored in Variant.
     * @param [in] value 4-bytes unsigned integer value to be stored in Variant.
     */
    void set(u_int32_t value);

    /**
     * Sets double value to be stored in Variant.
     * @param [in] value double value to be stored in Variant.
     */
    void set(double value);

    /**
     * Sets string value to be stored in Variant.
     * @param [in] value string value to be stored in Variant.
     */
    void set(const std::string& value);

    /**
     * Sets OpenAB::SecureString value to be stored in Variant.
     * @param [in] value OpenAB::SecureString value to be stored in Variant.
     */
    void set(const OpenAB::SecureString& value);

    /**
     * Sets pointer value to be stored in Variant.
     * @param [in] value pointer value to be stored in Variant.
     */
    void set(void* value);

    /**
     * Sets pointer value to be stored in Variant.
     * @param [in] value pointer value to be stored in Variant.
     */
    void set(const char* value);

    /**
     * Returns Variant::CHAR value stored in Variant
     * @return value stored in Variant, or 0 if Variant does not contain Variant::CHAR value
     */
    char getChar() const;

    /**
     * Returns Variant::BOOL value stored in Variant
     * @return value stored in Variant, or false if Variant does not contain Variant::BOOL value
     */
    bool getBool() const;

    /**
     * Returns Variant::INTEGER value stored in Variant
     * @return value stored in Variant, or 0 if Variant does not contain Variant::INTEGER value
     */
    int32_t getInt() const;

    /**
     * Returns Variant::DOUBLE value stored in Variant
     * @return value stored in Variant, or 0.0 if Variant does not contain Variant::DOUBLE value
     */
    double getDouble() const;

    /**
     * Returns Variant::STRING value stored in Variant
     * @return value stored in Variant, or empty string if Variant does not contain Variant::STRING value
     */
    std::string getString() const;

    /**
     * Returns Variant::SECURE_STRING value stored in Variant
     * @return value stored in Variant, or empty OpenAB::SecureString if Variant does not contain Variant::SECURE_STRING value
     */
    OpenAB::SecureString getSecureString() const;

    /**
     * Returns Variant::POINTER value stored in Variant
     * @return value stored in Variant, or NULL if Variant does not contain Variant::POINTER value
     */
    void* getPointer() const;

    /**
     * Returns data type stored by Variant
     * @return data type stored by Variant
     */
    DataType getType() const;

    /**
     * Checks if Variant was not initialized
     * @return true if Variant is not initialized, false otherwise
     */
    bool invalid() const;

  private:
    void clear();

    DataType dataType;

    union
    {
        char c;
        bool b;
        int32_t i;
        double d;
        void* p;
    } data;

    std::string stringData;
    OpenAB::SecureString secureStringData;
};

} // namespace OpenAB

#endif // VARIANT_H_
