/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file GenericParameters.hpp
 */

#ifndef GENERICPARAMETERS_H_
#define GENERICPARAMETERS_H_
#include <plugin/Plugin.hpp>
#include <helpers/Variant.hpp>

/*!
 * @brief namespace OpenAB_Plugin
 */
namespace OpenAB_Plugin {

/**
 * @brief Generic storage for plugin parameters.
 * Allows to store map of parameters with different types (@ref OpenAB::Variant).
 * Additionally allow for serialization and deserialization to/from JSON format.
 */
class GenericParameters : public Parameters
{
  public:
    /**
     * @brief Default constructor.
     */
    GenericParameters();

    /**
     * @brief Deserializes parameters from JSON sting.
     * @param [in] json JSON string to deserialize from.
     */
    GenericParameters(const std::string& json);

    /**
     *  @brief Destructor, virtual by default.
     */
    virtual ~GenericParameters();

    /**
     * @brief Serializes parameters to JSON string.
     * @return JSON string if serialization was successful or empty string otherwise.
     */
    std::string toJSON() const;

    /**
     * @brief Deserializes parameters from JSON string.
     * @note all previous contents of parameters will be removed.
     * @param [in] json JSON string to deserialize from.
     * @return true if deserialization was successful, false otherwise.
     */
    bool fromJSON(const std::string& json);

    /**
     * @brief Returns value assigned to given key
     * @param [in] key key to lookup
     * @return value assigned to key, or invalid OpenAB::Variant
     * (@ref OpenAB::Variant::invalid) if provided key does not exist.
     */
    OpenAB::Variant getValue(const std::string& key) const;

    /**
     * @brief Assigns or updates value to/of given key
     * @param [in] key key to create/modify
     * @param [in] value new value
     */
    void setValue(const std::string& key, const OpenAB::Variant& value);

    /**
     * @brief Removes key and its assigned value.
     * @param [in] key key to remove
     */
    void removeKey(const std::string& key);

    /**
     * @brief Returns list of all keys.
     * @retur list of all keys.
     */
    std::vector<std::string> getAllKeys() const;

  private:

    std::map<std::string, OpenAB::Variant> config;
};

} // namespace OpenAB_Plugin

#endif // GENERICPARAMETERS_H_
