/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PIMCalendarItem.hpp
 */

#ifndef PIMCALENDARITEM_HPP_
#define PIMCALENDARITEM_HPP_

#include <string>
#include <map>
#include <vector>
#include <set>
#include <PIMItem/PIMItem.hpp>
#include <PIMItem/PIMItemIndex.hpp>
/*!
 * @brief namespace OpenAB
 */
namespace OpenAB {

/*!
 * @brief Class representing single field of iCalendar object.
 * Field can have assigned value and list of parameters.
 * Fields itself does not know its name.
 */
class ItemField
{
  public:
    /*!
     * @brief default constructor
     */
    ItemField();

    /*!
     * @brief create field with given value
     */
    ItemField(const std::string& value);
    ~ItemField();

    /*!
     * @brief Parses given field from raw string
     * @param [in] rawFieldString string from which field should be parsed
     * @param [in] forceParams set to true if field does not have any value, but can have parameters list (eg. RRULE of VTIMEZONE)
     */
    bool parse(std::string rawFieldString, bool forceParams = false);

    /*!
     * @brief Sets value of field
     * @param [in] value value to be set
     */
    void setValue(const std::string& value);

    /*!
     * @brief comparison operator, required so fields can be stored in std::map
     */
    bool operator<(const ItemField& other) const;

    /*!
     * @brief Returns string representation of field
     * @return string representation of field
     */
    std::string toString() const;

    /*!
     * @brief Returns value of field
     * @return value of field
     */
    std::string getValue() const;

    /*!
     * @brief Returns all parameters of field
     * @return all parameters of field, keys in returned map are params names
     */
    std::map<std::string, std::set<std::string> > getParams() const;

  private:
    /*!
     * @biref Helper function that processes parameters
     * @param [in] paramLine raw string with parameters to be processed
     */
    void processParam (std::string paramLine);

    /*!
     * @brief Adds new parameter to field
     * @param [in] paramName name of parameter to be added
     * @param [in] values set of values assigned to given parameter
     */
    void addParam(std::string paramName, std::set<std::string> values);

    std::string value;
    std::map<std::string, std::set<std::string> > params;
};

/*!
 * @brief Class representing item containing fields in form of key value pairs,
 *  that can contain additional parameters for each field.
 *  Each item can also contain nested items.
 */
class KeyValueItem
{
  public:
    /*!
     * @brief default constructor
     */
    KeyValueItem();
    virtual ~KeyValueItem();

    /*!
     * @brief parses item from raw data string
     * @param [in] rawData string to be parsed
     * @return true if string was parsed successfully, false otherwise
     */
    bool parse(const std::string& rawData);

    /*!
     * @brief Returns all fields of item.
     * @return all fields of item.
     */
    std::map<std::string, std::vector<ItemField> >& getFields() {return fields;}

  protected:
    /*!
     * @brief Handler that processed contents of given fields.
     * Can be override in child classes.
     */
    virtual bool processField();

    std::string currentLine;
    std::string currentFieldName;
    std::string currentFieldValue;
    std::string currentFieldValueOriginal;
    std::map<std::string, std::vector<ItemField> > fields;
    std::map<std::string, std::vector< SmartPtr<KeyValueItem> > > subcomponents;

  private:
    bool processSubcomponent();

    bool inSubcomponent;
    std::string currentSubcomponentName;
    std::string currentSubcomponentData;
};

/*!
 * @brief Special implementation for VDAYLIGHT item of VTIMEZONE that ignores item
 */
class VDayLightTimeZoneKeyValueItem : public KeyValueItem
{
  public:
    VDayLightTimeZoneKeyValueItem(){};
    ~VDayLightTimeZoneKeyValueItem(){};

    bool processField() {return true;}
};

/*!
 * @brief Special implementation for VSTANDARD item of VTIMEZONE that ignores item
 */
class VStandardTimeZoneKeyValueItem : public KeyValueItem
{
  public:
    VStandardTimeZoneKeyValueItem(){};
    ~VStandardTimeZoneKeyValueItem(){};

    bool processField() {return true;}
};

/*!
 * @brief Factory for creating KeyValueItem object
 */
class KeyValueItemFactory
{
  public:
    KeyValueItemFactory() {}
    virtual ~KeyValueItemFactory() {}

    /*!
     * @brief creates KeyValueItem that can be used for parsing given component type
     * @param [in] componentType name of component
     * @return new KeyValueItem for given componentType, if there is no special class registered for handling given component type default one (KeyValueItem) will be returned.
     */
    static KeyValueItem* createItem(const std::string& componentType);

    /*!
     * @brief Registers factory for given component type
     * @param [in] factory to be registered
     * @param [in] component type for which factory should be registered
     */
    static void registerFactory(KeyValueItemFactory* factory, const std::string& componentType);

  protected:
    virtual KeyValueItem* createItem() = 0;

  private:
    static std::map<std::string, KeyValueItemFactory*> factories;
};

/*!
 * @brief Helper macro that defines new KeyValueItemFactory for given component type
 * and registers it
 */
#define CREATE_KEY_VALUE_ITEM_FACTORY(KeyValueItemClass, ComponentType) \
 class KeyValueItemClass##Factory : public KeyValueItemFactory          \
 {                                                                      \
   public:                                                              \
    KeyValueItemClass##Factory() : KeyValueItemFactory()                \
    {                                                                   \
      KeyValueItemFactory::registerFactory(this, ComponentType);        \
    }                                                                   \
    ~KeyValueItemClass##Factory(){}                                     \
   protected:                                                           \
    KeyValueItem* createItem()                                          \
    {                                                                   \
      return new KeyValueItemClass();                                   \
    }                                                                   \
 } KeyValueItemClass##FactoryInstance;

/*!
 * @brief Documentation for class PIMCalendarItem
 */
class PIMCalendarItem : public PIMItem, public KeyValueItem
{
  public:
    /*!
     *  @brief Constructor.
     */
    PIMCalendarItem(PIMItemType t);

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~PIMCalendarItem()=0;

    bool parse(const std::string& iCalendar);

    SmartPtr<PIMItemIndex> getIndex();

    std::string getRawData() const;

  private:
    std::string iCalendar;
};

class PIMCalendarEventItem : public PIMCalendarItem
{
  public:
     /*!
      *  @brief Constructor.
      */
    PIMCalendarEventItem();

     /*!
      *  @brief Destructor, virtual by default.
      */
     virtual ~PIMCalendarEventItem();
};

class PIMCalendarTaskItem : public PIMCalendarItem
{
  public:
    /*!
     *  @brief Constructor.
     */
    PIMCalendarTaskItem();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~PIMCalendarTaskItem();
};

} // namespace OpenAB

#endif // PIMCALENDARITEM_HPP_
