/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PIMContactItemIndex.hpp
 */

#ifndef PIMCONTACTITEMINDEX_HPP_
#define PIMCONTACTITEMINDEX_HPP_

#include "PIMItem/PIMItemIndex.hpp"
/*!
 * @brief namespace OpenAB
 */
namespace OpenAB {

/*!
 * @brief Documentation for class PIMContactItemIndex.
 * It implements PIMItemIndex for OpenAB::eContact type, additionally
 * allows to define which PIMItemCheck should be used to
 *  construct PIMContactItemIndex objects.
 */
class PIMContactItemIndex : public PIMItemIndex
{
  public:
    /*!
     *  @brief Constructor.
     */
    PIMContactItemIndex();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~PIMContactItemIndex();

    /**
     * @brief Compare operator.
     * Checks if two PIMItemIndex matches (may not be exactly the same)
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if other PIMItemIndex is of OpenAB::eContact type and
     * all PIMItemCheck::eKey fields are the same in both PIMItemIndex
     * objects, false otherwise.
     */
    bool operator==(const PIMItemIndex& other) const;

    /**
     * @brief Compare operator.
     * Checks if two PIMItemIndex not matches (are totally different)
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if other PIMItemIndex is not of OpenAB::eContact or
     * any PIMItemCheck::eKey fields is different in both PIMItemIndex
     *  objects, false otherwise.
     */
    bool operator!=(const PIMItemIndex& other) const;

    /**
     * @brief Compare operator.
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if other PIMItemIndex is of OpenAB::eContact type and
     * if index should be sorted before other index, according to
     * theirs PIMItemCheck::eKey fields alphabetic order.
     * @note Required to be able to store PIMItemIndex objects in std::map containers.
     */
    bool operator<(const PIMItemIndex& other) const;

    /**
     * @brief Compare operator.
     * Checks if two PIMItemIndex matches (may not be exactly the same)
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if all PIMItemCheck::eKey fields are the same
     * in both PIMItemIndex objects, false otherwise.
     */
    bool operator==(const PIMContactItemIndex& other) const;

    /**
     * @brief Compare operator.
     * Checks if two PIMItemIndex not matches (are totally different)
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if any PIMItemCheck::eKey fields is different in
     * both PIMItemIndex objects, false otherwise.
     */
    bool operator!=(const PIMContactItemIndex& other) const;

    /**
     * @brief Compare operator.
     * @param [in] other instance of PIMContactItemIndex to be compared with.
     * @return true if index should be sorted before other index,
     * according to theirs PIMItemCheck::eKey fields alphabetic order.
     * @note Required to be able store PIMContactItemIndex objects in std::map containers.
     */
    bool operator<(const PIMContactItemIndex& other) const;

    bool compare(const PIMItemIndex& other) const;

    /**
     * @brief Compares two PIMContactItemIndex object, in opposite to compare
     * operator it checks also PIMItemCheck::eConflict fields.
     * Checks if two PIMContactItemIndex are exactly the same.
     * @param [in] other instance of PIMContactItemIndex to be compared with.
     * @return true if all PIMItemCheck::eKey and PIMItemCheck::eConflict fields
     * are the same in both PIMContactItemIndex objects, false otherwise.
     */
    bool compare(const PIMContactItemIndex& other) const;

    std::string toString() const;
    std::string toStringFull() const;

    /**
     * @brief Defines new PIMItemCheck for PIMContactItem objects.
     * @param [in] fieldName name of vCard field to be checked
     * @param [in] role role of field check
     * @return true if new check was added successfully,
     * false if check for the same field already exists.
     */
    static bool addCheck(const std::string& fieldName,
                         PIMItemCheck::eFieldRole role);

    /**
     * @brief Removes check for PIMContactItem objects.
     * @param [in] fieldName name of vCard field
     * @return true if check was removed, false if it does not exist.
     */
    static bool removeCheck(const std::string& fieldName);

    /**
     * @brief Disables PIMItemCheck for PIMContactItem objects.
     * @note Used for synchronization phases of OpenAB_Sync::Sync - when synchronization
     * phase is ignoring some vCard fields from OpenAB_Source::Source, this allows to temporary
     * disable checks for fields that are ignored, preventing from unwanted detection
     * of not matching contacts from OpenAB_Storage::Storage.
     * @param [in] fieldName name of vCard field for which check should be disabled
     * @return true if check was disabled, false if check for given field does not exits.
     */
    static bool disableCheck(const std::string& fieldName);

    /**
     * @brief Enables PIMItemCheck for PIMContactItem objects.
     * @note for purpose enabling/disabling PIMItemCheck see @ref disableCheck()
     * @param [in] fieldName name of vCard field for which check should be enabled
     * @return true if check was enabled, false if check for given field does not exits.
     */
    static bool enableCheck(const std::string& fieldName);

    /**
     * @brief Enables all PIMItemCheck for PIMContactItem objects.
     * @note for purpose enabling/disabling PIMItemCheck see @ref disableCheck()
     */
    static void enableAllChecks();

    /**
     * @brief Removes all defined PIMItemCheck for PIMContactItem.
     */
    static void clearAllChecks();

    /**
     * @brief Returns all defined PIMItemCheck for PIMContactItem objects.
     * @return all defined PIMItemCheck for PIMContactItem objects.
     */
    static std::vector<PIMItemCheck> getAllChecks();

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    PIMContactItemIndex(PIMContactItemIndex const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    PIMContactItemIndex& operator=(PIMContactItemIndex const &other);

    static std::vector<PIMItemCheck> fields_desc;
    static bool anyCheckDisabled;
};

} // namespace OpenAB

#endif // PIMCONTACTITEMINDEX_HPP_
