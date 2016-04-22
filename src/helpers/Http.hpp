/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Http.hpp
 */

#ifndef HTTP_HPP_
#define HTTP_HPP_

#include <curl/curl.h>
#include <string>
#include <vector>

namespace OpenAB
{

class HttpMessage;

/**
 * @brief HttpSession class.
 * Allows to send Http requests.
 */
class HttpSession
{
  public:
    /**
     *  @brief Constructor.
     */
    HttpSession();

    /**
     *  @brief Destructor, virtual by default.
     */
    virtual ~HttpSession();

    /**
     * @brief Initializes http session.
     */
    bool init();

    /**
     * @brief Cleans up http session.
     */
    void cleanup();

    /**
     * @brief Executes given http message.
     * After executing message object will be updated with response, response code etc.
     * @param [in] msg message to be executed.
     * @return true if request was executed successfully, false otherwise.
     */
    bool execute (HttpMessage* msg);

    void enableTrace(bool enabled);

  private:
    /**
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    HttpSession(HttpSession const &other);

    /**
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    HttpSession& operator=(HttpSession const &other);

    void reset();

    CURL* curl;
    curl_slist * internalHeader;

    std::string lastErrorStr;
    std::string responseBuffer;
    std::string responseHeaders;
    const char* writeBuffer;
    unsigned int currentWriteOffset;
    bool traceEnabled;

    static size_t readResponse (void* ptr, size_t size, size_t nmemb, HttpSession* curl);
    static size_t readResponseHeaders (void* ptr, size_t size, size_t nmemb, HttpSession* curl);
    static size_t writeData (void* ptr, size_t size, size_t nmemb, HttpSession* curl);
    static int writeSeek(HttpSession *curl, curl_off_t offset, int origin);

    static int printTrace (CURL* curl, curl_infotype type, char* data, size_t size, void* userp);
};


/**
 * @brief HttpMessage class.
 * Represents Http request that can be executed using HttpSession object.
 */
class HttpMessage
{
  public:
    /**
     *  @brief Constructor.
     */
    HttpMessage();

    /**
     *  @brief Destructor, virtual by default.
     */
    ~HttpMessage();

    /**
     * @enum Http request type
     */
    enum RequestType
    {
      POST = 0, //< POST request
      GET,      //< GET request
      PUT,      //< PUT request
      CUSTOM    //< Custom request
    };

    enum ResponseCode
    {
      OK = 200,
      CREATED = 201,
      ACCEPTED = 202,
      NO_CONTENT = 204,
      MULTISTATUS = 207,
      MOVED_PERMAMENTLY = 301,
      BAD_REQUEST = 400,
      UNAUTHORIZED = 401,
      FORBIDDEN = 403,
      NOT_FOUND = 404,
      PRECONDITION_FAILED = 412
    };

    static std::string responseCodeDescription(long code);


    /**
     * @brief Sets message request type.
     * @param [in] type request type to be set.
     * @note This version of setRequestType does not accept RequestType::CUSTOM.
     */
    void setRequestType(RequestType type);

    /**
     * @brief Sets custom message request type.
     * Automatically sets request type to RequestType::CUSTOM.
     * @param [in] custom custom request type.
     */
    void setRequestType(const std::string& custom);

    /**
     * @brief Returns currently set request type.
     * @return currently set request type.
     */
    RequestType getRequestType() const;

    /**
     * @brief Returns custom request type, if request type is set to RequestType::CUSTOM.
     * @return custom request type or empty string.
     */
    std::string getCustomRequestType() const;

    typedef std::vector<std::pair<std::string, std::string> > Headers;

    /**
     * @brief Appends given header to message headers.
     * @param [in] key of header.
     * @param [in] value of header.
     */
    void appendHeader(const std::string& key, const std::string& value);

    /**
     * @brief Returns all headers set for message.
     */
    const Headers& getHeaders() const;

    /**
     * @brief Sets body of message.
     * @param [in] data body of message.
     * @todo add support for non text data
     */
    void setData (const std::string& data);

    /**
     * @biref Returns currently set body of message.
     * @return currently set body of message.
     */
    std::string getData() const;

    /**
     * @brief Should redirections be followed.
     * @param follow true if redirections should be followed.
     */
    void setFollowRedirection (bool follow);

    /**
     * @brief Returns status of redirections.
     * @return true if redirection will be followed.
     */
    bool followRedirection() const;

    /**
     * @brief Sets URL of message
     * @param [in] url URL.
     */
    void setURL (const std::string& url);

    /**
     * @breif Returns currently set URL.
     * @param currently set URL.
     */
    std::string getURL() const;

    /**
     * @brief Sets response data.
     * @param [in] response request response.
     * @todo add support for non textual responses.
     */
    void setResponse(const std::string& response);

    /**
     * @brief Gets request response.
     * @return request response.
     */
    std::string getResponse() const;

    /**
     * @brief Sets response headers data.
     * @param [in] headers response headers.
     */
    void setResponseHeaders(const std::string& headers);

    /**
     * @brief Gets response headers.
     * @return response headers.
     */
    Headers getResponseHeaders() const;

    /**
     * @brief Sets request response code.
     * @param [in] code response code.
     */
    void setResponseCode(long code);

    /**
     * @brief Gets request response code
     * @return request response code
     */
    long getResponseCode() const;

    /**
     * @brief Sets error description
     * @param [in] errorStr error description
     */
    void setErrorString(const std::string& errorStr);

    /**
     * @brief Gets error description.
     * @return Error description or empty string if description was not set.
     */
    std::string getErrorString() const;

    /**
     * @brief Sets credentials for HTTP authentication.
     * @param [in] login login to be used for authentication
     * @param [in] password password to be used for authentication
     */
    void setCredentials(const std::string& login, const std::string& password);

    /**
     * @brief Gets credential for HTTP authentication.
     * @param [out] login login to be used for authentication
     * @param [out] password password to be used for authentication
     */
    void getCredentials(std::string& login, std::string& password);

    /**
     * @brief Enables or disables basic HTTP authentication
     * @note authentication credentials can be set using @ref setCredentials.
     * @param [in] enable enable or disable authentication
     */
    void enableBasicHttpAuthentication(bool enable);

    /**
     * @brief Checks if basic HTTP authentication is enabled.
     * @return true if basic HTTP authentication is enabled, false otherwise.
     */
    bool basicHttpAuthenticationEnabled();

    /**
     * @brief Enables or disables digest HTTP authentication
     * @note authentication credentials can be set using @ref setCredentials.
     * @param [in] enable enable or disable authentication
     */
    void enableDigestHttpAuthentication(bool enable);

    /**
     * @brief Checks if digest HTTP authentication is enabled.
     * @return true if digest HTTP authentication is enabled, false otherwise.
     */
    bool digestHttpAuthenticationEnabled();

  private:
    /**
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    HttpMessage(HttpMessage const &other);

    /**
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    HttpMessage& operator=(HttpMessage const &other);

    RequestType requestType;
    std::string customRequestType;
    Headers headers;
    std::string data;
    std::string url;
    bool redirectionEnabled;
    long responseCode;
    std::string response;
    Headers responseHeaders;
    std::string errorString;
    bool basicAuthenticationEnabled;
    bool digestAuthenticationEnabled;
    std::string login;
    std::string password;
};

/**
 * @brief HttpAuthorizer interface.
 * This should be used by classes implementing different Http authorization methods.
 */
class HttpAuthorizer
{
  public:
    /**
     *  @brief Destructor, virtual by default.
     */
    virtual ~HttpAuthorizer() {};

    /**
     * @brief Authorizes message.
     * @param msg message to be authorized.
     * @return true if message was authorized successfully, false otherwise.
     */
    virtual bool authorizeMessage(HttpMessage* msg) = 0;
};

} //namespace OpenAB
#endif // HTTP_HPP_
