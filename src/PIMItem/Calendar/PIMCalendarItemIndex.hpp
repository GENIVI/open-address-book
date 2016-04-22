/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PIMCalendarItemIndex.hpp
 */

#ifndef PIMCALENDARITEMINDEX_HPP_
#define PIMCALENDARITEMINDEX_HPP_

#include "PIMItem/PIMItemIndex.hpp"
/*!
 * @brief namespace OpenAB
 */
namespace OpenAB {

/*!
 * @brief Documentation for class PIMCalendarItemIndex.
 * It implements PIMItemIndex for OpenAB::eEvent or OpenAB::eTask type, additionally
 * allows to define which PIMItemCheck should be used to
 *  construct PIMCalendarItemIndex objects.
 */
class PIMCalendarItemIndex : public PIMItemIndex
{
  public:
    /*!
     *  @brief Constructor.
     */
    PIMCalendarItemIndex(PIMItemType t);

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~PIMCalendarItemIndex();

    std::string toString() const;
    std::string toStringFull() const;

    /**
     * @brief Defines new PIMItemCheck for PIMCalendarItemIndex objects.
     * @param [in] fieldName name of iCalendar field to be checked
     * @param [in] role role of field check
     * @return true if new check was added successfully,
     * false if check for the same field already exists.
     */
    static bool addCheck(const std::string& fieldName,
                         PIMItemCheck::eFieldRole role);

    /**
     * @brief Removes check for PIMCalendarItemIndex objects.
     * @param [in] fieldName name of iCalendar field
     * @return true if check was removed, false if it does not exist.
     */
    static bool removeCheck(const std::string& fieldName);

    /**
     * @brief Removes all defined PIMItemCheck for PIMCalendarItemIndex.
     */
    static void clearAllChecks();

    /**
     * @brief Returns all defined PIMItemCheck for PIMCalendarItemIndex objects.
     * @return all defined PIMItemCheck for PIMCalendarItemIndex objects.
     */
    static std::vector<PIMItemCheck> getAllChecks();


    /**
     * @brief Compare operator.
     * Checks if two PIMItemIndex matches (may not be exactly the same)
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if other PIMItemIndex is of OpenAB::eEvent or OpenAB::eTask type and
     * all PIMItemCheck::eKey fields are the same in both PIMItemIndex
     * objects, false otherwise.
     */
    bool operator==(const PIMItemIndex& other) const;

    /**
     * @brief Compare operator.
     * Checks if two PIMItemIndex not matches (are totally different)
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if other PIMItemIndex is not of OpenAB::eEvent or OpenAB::eTask or
     * any PIMItemCheck::eKey fields is different in both PIMItemIndex
     *  objects, false otherwise.
     */
    bool operator!=(const PIMItemIndex& other) const;

    /**
     * @brief Compare operator.
     * @param [in] other instance of PIMItemIndex to be compared with.
     * @return true if other PIMItemIndex is of OpenAB::eEvent or OpenAB::eTask type and
     * if index should be sorted before other index, according to
     * theirs PIMItemCheck::eKey fields alphabetic order.
     * @note Required to be able to store PIMItemIndex objects in std::map containers.
     */
    bool operator<(const PIMItemIndex& other) const;


    bool compare(const PIMItemIndex& other) const;
  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    PIMCalendarItemIndex(PIMCalendarItemIndex const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    PIMCalendarItemIndex& operator=(PIMCalendarItemIndex const &other);

  protected:
    static std::vector<PIMItemCheck> fields_desc;

    virtual bool equal(const PIMItemIndex& other) const;
    virtual bool notEqual(const PIMItemIndex& other) const;
    virtual bool lessThan(const PIMItemIndex& other) const;
    virtual bool compareWith(const PIMItemIndex& other) const;
};


class PIMCalendarEventItemIndex : public PIMCalendarItemIndex
{
  public:
    /*!
     *  @brief Constructor.
     */
    PIMCalendarEventItemIndex();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~PIMCalendarEventItemIndex();
};

class PIMCalendarTaskItemIndex : public PIMCalendarItemIndex
{
  public:
    /*!
     *  @brief Constructor.
     */
    PIMCalendarTaskItemIndex();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~PIMCalendarTaskItemIndex();
};

} // namespace OpenAB

#endif // PIMCALENDARITEMINDEX_HPP_
