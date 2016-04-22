/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file OAuthHttpAuthorizer.hpp
 */

#ifndef OAUTH2HTTPAUTHORIZER_HPP_
#define OAUTH2HTTPAUTHORIZER_HPP_

#include "SecureString.hpp"
#include "Http.hpp"

namespace OpenAB
{
/*!
 * @brief Class OAuth2HttpAuthorizer
 * Implements OAuth2 authentication method.
 * @todo Current implementation is Google specific.
 */
class OAuth2HttpAuthorizer : public HttpAuthorizer
{
  public:
    /*!
     *  @brief Constructor.
     */
    OAuth2HttpAuthorizer();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~OAuth2HttpAuthorizer();

    /*!
     * @brief Obtains new access token, based on provided data.
     * @param [in] clientId id of client application
     * @param [in] clientSecret secret of client application
     * @param [in] refreshToken user refresh token
     * @return true if new access token was obtained successfully, false otherwise.
     */
    bool authorize (const std::string& clientId,
                    const OpenAB::SecureString& clientSecret,
                    const OpenAB::SecureString& refreshToken);

    bool authorizeMessage (HttpMessage* curl);

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    OAuth2HttpAuthorizer(OAuth2HttpAuthorizer const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    OAuth2HttpAuthorizer& operator=(OAuth2HttpAuthorizer const &other);

    OpenAB::SecureString token;
};

} //namespace OpenAB
#endif // OAUTH2AUTHORIZER_HPP_
