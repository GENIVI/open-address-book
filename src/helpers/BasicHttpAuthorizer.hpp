/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file BasicHttpAuthorizer.hpp
 */

#ifndef BASICHTTPAUTHORIZER_HPP_
#define BASICHTTPAUTHORIZER_HPP_

#include <helpers/SecureString.hpp>
#include <helpers/Http.hpp>

namespace OpenAB
{
/*!
 * @brief Class BasicHttpAuthorizer.
 * Implements HTTP Basic authentication method.
 */
class BasicHttpAuthorizer : public HttpAuthorizer
{
  public:
    /*!
     *  @brief Constructor.
     */
    BasicHttpAuthorizer();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~BasicHttpAuthorizer();

    /*!
     * @brief Sets user credentials.
     * @param [in] login user login.
     * @param [in] password user password.
     */
    void setCredentials (const std::string& login,
                         const OpenAB::SecureString& password);

    bool authorizeMessage (HttpMessage* curl);

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    BasicHttpAuthorizer(BasicHttpAuthorizer const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    BasicHttpAuthorizer& operator=(BasicHttpAuthorizer const &other);

    std::string login;
    OpenAB::SecureString pass;
};

} //namespace OpenAB
#endif // BASICHTTPAUTHORIZER_HPP_
