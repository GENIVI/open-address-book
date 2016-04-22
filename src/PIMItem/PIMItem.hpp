/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PIMItem.hpp
 */

#ifndef PIMITEM_HPP_
#define PIMITEM_HPP_

#include <PIMItem/PIMItemIndex.hpp>

/*!
 * @brief namespace OpenAB
 */
namespace OpenAB {
/**
 * @brief Class representing PIM item.
 */
class PIMItem
{
  public:

    /**
     *  @brief Constructor.
     *  @param [in] t type of item
     */
    PIMItem(PIMItemType t)
    {
      type = t;
    };

    /**
     *  @brief Destructor, virtual by default.
     */
    virtual ~PIMItem(){};

    /**
     * @brief Parses item from string.
     * Format of string to be parsed from depends on type of item.
     * @note during parsing new fields (not defined by standard format from which item is being parsed)
     * can be added to improve matching and comparison of items. For more details see documentation of classes
     *  implementing PIMItem interface.
     * @param [in] raw raw data to be parsed.
     * @return true if item was parsed successfully, false otherwise.
     */
    virtual bool parse(const std::string& raw) = 0;

    /**
     * @brief Returns index for given item.
     * @return index for given item.
     */
    virtual SmartPtr<PIMItemIndex> getIndex() = 0;

    /**
     * @brief Returns raw data of item.
     * Format of raw data depends on type of item.
     * @return raw data of item
     */
    virtual std::string getRawData() const = 0;

    typedef std::string ID;

    /**
     * @brief Returns Source/Storage specific id of item.
     * Can be empty, as not all Sources/Storages provides id of items.
     * @return id of item
     */
    virtual ID getId() const
    {
      return id;
    }

    /**
     * @brief Sets id of item
     * @param [in] id to be assigned
     * @param [in] replace should id in raw metadata be replaced with provided id.
     * @note default implementation ignores replace parameter
     */
    virtual void setId(const ID& id,
                       bool replace = false)
    {
      (void) replace;
      this->id = id;
    }

    typedef std::string Revision;

    /**
     * @brief Returns Source/Storage specific revision of item.
     * Can be empty, as not all Sources/Storages provides revision of items.
     * @return revision of item
     */
    virtual Revision getRevision() const
    {
      return revision;
    }

    /**
     * @brief Sets revision of item
     * @param [in] revision revision to be set
     */
    virtual void setRevision(const Revision& rev)
    {
      revision = rev;
    }

    typedef std::vector<OpenAB::PIMItem::ID> IDs;
    typedef std::vector<OpenAB::PIMItem::Revision> Revisions;


    /**
     * @brief Returns type of item
     * @return type of item
     */
    PIMItemType getType() const
    {
      return type;
    }

  protected:
    PIMItemType type;
    ID id;
    Revision revision;
};

} // namespace OpenAB

#endif // PIMITEM_HPP_
