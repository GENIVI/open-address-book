/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file GDataContactsConverter.hpp
 */

#ifndef GDATACONTACTSCONVERTER_HPP_
#define GDATACONTACTSCONVERTER_HPP_

#include <gdata/gdata.h>
#include <string>
#include <vector>

/*!
 * @brief Class GDataContactsConverter.
 * Converts contacts from GData format to vCards.
 */
class GDataContactsConverter
{
  public:
    /*!
     *  @brief Constructor.
     */
    GDataContactsConverter();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~GDataContactsConverter();

    /*!
     * @brief Converts GDataContactsContact to vCard
     * @param [in] contact contact to be converted
     * @param [out] vCard converted vCard.
     */
    static bool convert(GDataContactsContact* contact, std::string& vCard);

    /*!
     * @brief Converts GDataContactsContact to vCard with photo data.
     * @param [in] contact contact to be converted.
     * @param [in] photoData photo data.
     * @param [in] photoDataLen size of photoData.
     * @param [in] photoType format of photo.
     * @param [out] vCard converted vCard.
     */
    static bool convert(GDataContactsContact* contact, guint8* photoData, gsize photoDataLen, gchar* photoType, std::string& vCard);

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    GDataContactsConverter(GDataContactsConverter const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    GDataContactsConverter& operator=(GDataContactsConverter const &other);

    /*!
     * @brief Generic function for generating one vCard field.
     * @param [in] contact to be converted.
     * @param [in, out] stream, stream to which converted data should be added.
     */

    typedef bool (*FieldParseFunction)(GDataContactsContact* contac, std::ostringstream& stream);

    static std::vector<FieldParseFunction> fieldParsers;

    static bool convertCommon(GDataContactsContact* contact, std::ostringstream& oss);
    static void populateParsers();
};

#endif // GDATACONTACTSCONVERTER_HPP_
