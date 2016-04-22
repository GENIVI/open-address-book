/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PIMContactItem.hpp
 */

#ifndef PIMCONTACTITEM_HPP_
#define PIMCONTACTITEM_HPP_

#include <string>
#include <map>
#include <vector>
#include <set>
#include <PIMItem/PIMItem.hpp>
#include <PIMItem/PIMItemIndex.hpp>

/*!
 * @brief namespace Ias
 */
namespace OpenAB {

class VCardField;

/**
 * @brief Class representing PIM contact item.
 */
class PIMContactItem : public PIMItem
{
  public:
    /*!
     *  @brief Constructor.
     */
    PIMContactItem();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~PIMContactItem();

    /**
     * @brief Parses PIMContacItem from vCard string.
     * @note during parsing new non-standard vCard fields can be
     * added to support matching and comparison of items
     * (currently fields 'n_family', 'n_given', 'n_middle', 'n_prefix', 'n_suffix' are generated from N vCard field).
     * @param [in] vCard vCard string to be parsed.
     * @return true if item was parsed successfully, false otherwise.
     */
    bool parse(const std::string& vCard);

    SmartPtr<PIMItemIndex> getIndex();

    /**
     * @brief Returns vCard string of item.
     * @return vCard of item
     */
    std::string getRawData() const;

    /**
     * @brief Sets id of item
     * @param [in] id to be assigned
     * @param [in] replace should UID in vCard be replaced with given id
     */
    void setId(const PIMItem::ID& id,
               bool replace = false);

#ifdef TESTING
    std::map<std::string, std::vector<VCardField> > getFields()
    {
      return fields;
    }
#endif

  private:
    void substituteVCardUID(const std::string newUID);

    /**
     * @brief Map of all parsed vCard fields.
     * VCard can have more then one occurrence of the same field,
     * each of it is stored as VCardField object
     */
    std::map<std::string, std::vector<VCardField> > fields;

    std::string vCard;
};

/**
 * @brief Represents single parsed vCard field.
 * Filed has its value and additionally it can have set of
 * parameters (each of parameter can have multiple values).
 * @note example of vCard field with multiple parameters:
 * TEL;TYPE=WORK;TYPE=FAX:0035387123100
 * the same field can be written as
 * TEL;TYPE=WORK,FAX:0035387123100
 */
class VCardField {
  public:
    /**
     * @brief Default constructor
     */
    VCardField();

    /**
     * @brief Constructor
     * @param [in] v value of field.
     */
    VCardField(const std::string& v);

    /**
     *  @brief Destructor, virtual by default.
     */
    ~VCardField();

    /**
     * @brief Parses field from string.
     * @param [in] vCardFieldString field string from vCard to be parsed.
     * @return true if field was parsed successfully, false otherwise.
     */
    bool parse(std::string vCardFieldString);

    /**
     * @brief Sets given value
     * @param [in] value value to be set
     */
    void setValue (const std::string& value);

    /**
     * @brief Comparison operator.
     * Compares two VCardField objects in alphabetical order
     * (using toString to create string representation of VCardField).
     * @param [in] other VCardField instance to compare with.
     * @return true if field should be sorted before other field (according to their string representations).
     */
    bool operator<(const VCardField& other) const;

    /**
     * @brief Converts content of VCardField into string format.
     * Used for comparing two VCardField instances and for debugging purposes.
     * @return string representation of VCardField.
     */
    std::string toString() const;

    /**
     * @brief Returns value of VCardField.
     * VCardField value is the string that occurred after last ":" character in field string from vCard.
     * @return value of VCardField.
     */
    std::string getValue() const;

    /**
     * @brief Return map of all parameters assigned to given field.
     * @return map of all parameters assigned to field.
     */
    std::map<std::string, std::set<std::string> > getParams() const;

  private:
    void processParam(std::string paramLine);

    void addParam(std::string paramName, std::set<std::string> values);

    std::string value;

    std::map<std::string, std::set<std::string> > params;
};

/**
 * @brief Helper class for handling vCard fields of PHOTO or LOGO type
 */
class VCardPhoto
{
  public:
    /**
     * @brief calculates checksum of photo field.
     * Field can have photo data embedded as value or contain URI pointing to photo.
     * @param [in] photoField field
     * @return checksum of photo, or 0 in case where photo data cannot be decoded or URI is invalid.
     */
    static unsigned long GetCheckSum(const VCardField& photoField);
};

} // namespace OpenAB

#endif // PIMCONTACTITEM_HPP_
