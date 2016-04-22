/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PIMItemIndex.hpp
 */

#ifndef PIMITEMINDEX_HPP_
#define PIMITEMINDEX_HPP_

#include <string>
#include <vector>
#include <helpers/SmartPtr.hpp>

/*!
 * @brief namespace OpenAB
 */
namespace OpenAB {
/** @enum
 * Type of PIMItem
 */
enum PIMItemType
{
  eContact,   /**< Contact*/
  eEvent,     /**< Calendar event*/
  eTask       /**< Task */
};

/**
 * @brief Documentation for class PIMItemIndex.
 * PIMItemIndex is a PIMItem representation used to match and compare PIMItems.
 * Set of PIMItem fields is stored along with comparison rules allowing matching and comparing of PIMItems.
 * PIMItemIndex is intended to be used by OpenAB_Sync::Sync plugins.
 */
class PIMItemIndex
{
  public:
    /*!
     *  @brief Constructor.
     *  @param [in] t type of item that index is representing.
     */
    PIMItemIndex(OpenAB::PIMItemType t);

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~PIMItemIndex();

    /**
     * @brief Definition of check that will be made on PIMItemIndex instances when comparing them.
     * Each check checks single field of PIMItem, and based on its role, detects if items are totally different, or items are modified version of the same item.
     */
    struct PIMItemCheck
    {
        /** @enum
         * Field role
         */
        enum eFieldRole
        {
          eKey,      /**< Field is key, if it differs between PIMItemIndex they are considered as not matching*/
          eConflict  /**< Field is conflict key, if it differs between PIMItemIndex it means that items are not the same*/
        };

        /**
         * @brief Constructor
         * @param [in] name name of field to be checked
         * @param [in] role role of field
         */
        PIMItemCheck(const std::string& name, eFieldRole role) :
          fieldName(name),
          fieldRole(role),
          enabled(true)
        {}

        ~PIMItemCheck() {}

        std::string fieldName;
        eFieldRole fieldRole;
        bool enabled;
    };

    /**
     * @brief Compare operator.
     * Checks if two PIMItemIndex matches (may not be exactly the same)
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if all PIMItemCheck::eKey fields are the same in both PIMItemIndex objects, false otherwise.
     */
    virtual bool operator==(const PIMItemIndex& other) const = 0;

    /**
     * @brief Compare operator.
     * Checks if two PIMItemIndex not matches (are totally different)
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if any PIMItemCheck::eKey fields is different in both PIMItemIndex objects, false otherwise.
     */
    virtual bool operator!=(const PIMItemIndex& other) const = 0;

    /**
     * @brief Compare operator.
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if index should be sorted before other index, according to theirs PIMItemCheck::eKey fields alphabetic order.
     * @note Required to be able to store PIMItemIndex objects in std::map containers.
     */
    virtual bool operator<(const PIMItemIndex& other) const = 0;

    /**
     * @brief Adds new PIMItemCheck::eKey field.
     * @param [in] name name of field
     * @param [in] value of field
     */
    virtual void addKeyField(const std::string& name, const std::string& value);

    /**
     * @brief Adds new PIMItemCheck::eConflict field.
     * @param [in] name name of field
     * @param [in] value of field
     */
    virtual void addConflictField(const std::string& name, const std::string& value);

    /**
     * @brief Compares two PIMItemIndex object, checks if two PIMItemIndex are exactly the same.
     * In opposite to compare operator it checks PIMItemCheck::eConflict fields.
     * @note PIMItemCheck::eKey fields are not checked by this function, assumption is made that items were compared
     * using operator ==() before, and only matching items are tested using this function.
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if all PIMItemCheck::eConflict fields are the same in both PIMItemIndex objects, false otherwise.
     */
    virtual bool compare(const PIMItemIndex& other) const = 0;

    /**
     * @brief Returns string version of PIMItemIndex.
     * It contains only PIMItemCheck::eKey fields
     * @return string version of PIMItemIndex
     */
    virtual std::string toString() const = 0;

    /**
     * @brief Returns string version of PIMItemIndex.
     * It contains both PIMItemCheck::eKey and PIMItemCheck::eConflict fields
     * @return string version of PIMItemIndex
     */
    virtual std::string toStringFull() const = 0;

    /**
     * @brief Returns type of item that index is representing
     * @return type of item that index is representing
     */
    OpenAB::PIMItemType getType() const;

  protected:
    bool compareVectors(const std::vector<std::string>& v1,
                        const std::vector<std::string>& v2) const;

    std::vector<std::string> key_fields;
    std::vector<std::string> conflict_fields;
    std::vector<std::string> key_fields_names;
    std::vector<std::string> conflict_fields_names;

    mutable std::string cached_to_string;

  private:
    OpenAB::PIMItemType type;
};

} // namespace OpenAB

#endif // PIMITEMINDEX_HPP_
