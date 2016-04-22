/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file OneWaySync.cpp
 */

#include "Google.hpp"
#include <PIMItem/Contact/PIMContactItem.hpp>
#include <helpers/StringHelper.hpp>
#include <algorithm>
#include <math.h>
#include <locale>
#include <sys/time.h>
#include <libsoup/soup.h>
#include "GDataContactsConverter.hpp"
#include "GDataOAuth2Authorizer.h"

#define QUERY_SIZE 500

#define EXP_BACKOFF(numRetries) \
  for (unsigned int _i = 0; _i < (numRetries); ++_i, usleep(pow(2.0, _i) * 10))

GoogleSource::GoogleSource(const std::string& login,
                           const OpenAB::SecureString& password,
                           const std::string& ignoreFields)
    : OpenAB_Source::Source(OpenAB::eContact),
      userLogin(login),
      userPassword(password),
      clientId(),
      clientSecret(),
      refreshToken(),
      transferStatus(eGetItemRetOk),
      authorizer(NULL),
      service(NULL),
      cancellable(NULL),
      totalNumber(0),
      threadCreated(false),
      paused(false),
      cancelled(false)
{
  LOG_FUNC();
  construct(ignoreFields);
}

GoogleSource::GoogleSource(const std::string& clientId,
                           const OpenAB::SecureString& clientSecret,
                           const OpenAB::SecureString& refreshToken,
                           const std::string& ignoreFields)
    : OpenAB_Source::Source(OpenAB::eContact),
      userLogin(),
      userPassword(),
      clientId(clientId),
      clientSecret(clientSecret),
      refreshToken(refreshToken),
      transferStatus(eGetItemRetOk),
      authorizer(NULL),
      service(NULL),
      cancellable(NULL),
      totalNumber(0),
      threadCreated(false),
      paused(false),
      cancelled(false)
{
  LOG_FUNC();
  construct(ignoreFields);
}

void GoogleSource::construct(const std::string& ignoreFields)
{
  if (!ignoreFields.empty())
  {
    std::string field;
    size_t pos = 0;
    size_t nextPos = std::string::npos;
    while (std::string::npos != (nextPos = ignoreFields.find_first_of(',', pos)))
    {
      field = ignoreFields.substr(pos, nextPos - pos);
      LOG_DEBUG() << "Ignore field " << field << std::endl;
      pos = nextPos + 1;
      //remove spaces
      field.erase(std::remove_if(field.begin(), field.end(), ::isspace), field.end());
      ignoredFields.push_back(field);
    }
    field = ignoreFields.substr(pos);
    field.erase(std::remove_if(field.begin(), field.end(), ::isspace), field.end());
    ignoredFields.push_back(field);
    LOG_DEBUG() << "Ignore field " << field << std::endl;
  }

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&bufferReadyCond, NULL);
}

GoogleSource::~GoogleSource()
{
  cancelled = true;
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&bufferReadyCond);
  if (threadCreated)
  {
    pthread_join(downloadThread, NULL);
  }
  cleanup();

  clientSecret.clear();
  refreshToken.clear();
  userPassword.clear();
}

void GoogleSource::cleanup()
{
  LOG_FUNC();

  if (service)
  {
    g_object_unref(service);
    service = NULL;
  }

  if (authorizer)
  {
    g_object_unref(authorizer);
    authorizer = NULL;
  }

  if (cancellable)
  {
    g_object_unref(cancellable);
    cancellable = NULL;
  }
}

enum OpenAB_Source::Source::eInit GoogleSource::init()
{
  cleanup();

  GError * gerror = NULL;

#if GLIB_VERSION_MIN_REQUIRED < G_ENCODE_VERSION(2,36)
  /* g_type_init has been deprecated since 2.36 */
  g_type_init();
#endif
  //initialize GDataAuthorizer
  if (!userLogin.empty())
  {
    bool authorizationSuccesful = false;
    authorizer = GDATA_AUTHORIZER(
        gdata_client_login_authorizer_new("678546420751-fvs60cfnfi0p8114a6i4t7qeqck4cp3n.apps.googleusercontent.com",
                                          GDATA_TYPE_CONTACTS_SERVICE));
    if (NULL == authorizer)
    {
      LOG_ERROR() << "Cannot allocate authorizer" << std::endl;
      return eInitFail;
    }

    authorizationSuccesful = gdata_client_login_authorizer_authenticate(GDATA_CLIENT_LOGIN_AUTHORIZER(authorizer),
                                                                        userLogin.c_str(),
                                                                        userPassword.str(),
                                                                        NULL,
                                                                        &gerror);
    userPassword.clearStr();
    if (!authorizationSuccesful)
    {
      LOG_ERROR() << "Cannot authenticate user: " << userLogin << " - " << GERROR_MESSAGE(gerror) << std::endl;
      GERROR_FREE(gerror);
      return eInitFail;
    }
  }
  else
  {
    bool authorizationSuccesful = false;

    authorizer = GDATA_AUTHORIZER(gdata_oauth2_authorizer_new(GDATA_TYPE_CONTACTS_SERVICE));
    if (NULL == authorizer)
    {
      LOG_ERROR() << "Cannot allocate authorizer" << std::endl;
      return eInitFail;
    }

    authorizationSuccesful = gdata_oauth2_authorizer_authenticate(GDATA_OAUTH2_AUTHORIZER(authorizer),
                                                                  clientId.c_str(),
                                                                  clientSecret.str(),
                                                                  refreshToken.str(), NULL,
                                                                  &gerror);

    clientSecret.clearStr();
    refreshToken.clearStr();
    if (!authorizationSuccesful)
    {
      LOG_ERROR() << "Cannot authenticate user: " << userLogin << " - " << GERROR_MESSAGE(gerror) << std::endl;
      GERROR_FREE(gerror);
      return eInitFail;
    }
  }

  service = gdata_contacts_service_new(authorizer);
  if (NULL == service)
  {
    LOG_ERROR() << "Cannot allocate contacts service" << std::endl;
    return eInitFail;
  }

  cancellable = g_cancellable_new();

  bufferedVCards.clear();
  totalNumber = 0;
  paused = false;
  cancelled = false;

  //start worker thread
  pthread_create(&downloadThread, NULL, downloadThreadFunc, this);
  threadCreated = true;

  return eInitOk;
}

std::string GoogleSource::convertContact(GDataContactsContact* contact,
                                         guint8* photoData,
                                         gsize photoDataLen,
                                         gchar* photoContentType)
{
  std::string vcard;
  if (NULL != photoData)
  {
    GDataContactsConverter::convert(contact, photoData, photoDataLen, photoContentType, vcard);
    g_free(photoData);
    g_free(photoContentType);
  }
  else
  {
    GDataContactsConverter::convert(contact, vcard);
  }

  return vcard;
}

enum OpenAB_Source::Source::eGetItemRet GoogleSource::getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  std::string vCard;

  pthread_mutex_lock(&mutex);

  if (bufferedVCards.empty())
  {
    if (eGetItemRetEnd == transferStatus)
    {
      //if buffer is empty and transfer status is end, process finished
      pthread_mutex_unlock(&mutex);
      return eGetItemRetEnd;
    }
    //otherwise wait for new vCards
    pthread_cond_wait(&bufferReadyCond, &mutex);
  }
  //check after being wake up, if there was some new contacts added
  if (eGetItemRetEnd == transferStatus && bufferedVCards.empty())
  {
    LOG_DEBUG() << "Download end" << std::endl;
    pthread_mutex_unlock(&mutex);
    return eGetItemRetEnd;
  }

  if (eGetItemRetError == transferStatus)
  {
    LOG_DEBUG() << "Download error" << std::endl;
    pthread_mutex_unlock(&mutex);
    return eGetItemRetError;
  }

  vCard = bufferedVCards.front();
  bufferedVCards.pop_front();
  pthread_mutex_unlock(&mutex);

  OpenAB::PIMContactItem *newContactItem = new OpenAB::PIMContactItem();
  if (newContactItem->parse(vCard))
  {
    item = newContactItem;
    return eGetItemRetOk;
  }
  else
  {
    LOG_DEBUG() << "Parsing error" << std::endl;
    delete newContactItem;
    return eGetItemRetError;
  }

  return transferStatus;
}

enum OpenAB_Source::Source::eSuspendRet GoogleSource::suspend()
{
  pthread_mutex_lock(&mutex);
  paused = true;
  pthread_mutex_unlock(&mutex);
  return eSuspendRetOk;
}

enum OpenAB_Source::Source::eResumeRet GoogleSource::resume()
{
  pthread_mutex_lock(&mutex);
  paused = false;
  pthread_mutex_unlock(&mutex);
  return eResumeRetOk;
}

enum OpenAB_Source::Source::eCancelRet GoogleSource::cancel()
{
  pthread_mutex_lock(&mutex);
  cancelled = true;
  pthread_mutex_unlock(&mutex);

  if (NULL != cancellable)
  {
    g_cancellable_cancel(cancellable);
  }
  return eCancelRetOk;
}

int GoogleSource::getTotalCount() const
{
  //We can provide total count after download of contact finished,
  //before that we cannot say how many results will be
  if (eGetItemRetEnd == transferStatus)
    return totalNumber;
  return 0;
}

bool GoogleSource::downloadPhoto()
{
  if (OpenAB::contains(ignoredFields, "photo"))
  {
    return false;
  }
  return true;
}
void* GoogleSource::downloadThreadFunc(void* ptr)
{
  GoogleSource& source = *static_cast<GoogleSource*> (ptr);
  GDataContactsQuery* query = NULL;
  GDataFeed *feed = NULL;
  GError * gerror = NULL;
  GList * contactsList = NULL;
  int numDownloaded = 100;
  unsigned int offset = 0;

  //while last query returned some results
  while (numDownloaded > 0)
  {
    while (source.paused && !source.cancelled)
    {
      usleep(1000);
    }

    if (source.cancelled)
    {
      //if we are cancelled, wake up thread waiting for new vCards
      pthread_mutex_lock(&source.mutex);
      pthread_cond_signal(&source.bufferReadyCond);
      pthread_mutex_unlock(&source.mutex);
      return NULL;
    }

    //prepare new query
    numDownloaded = 0;
    query = gdata_contacts_query_new_with_limits(NULL, offset, QUERY_SIZE);
    if (NULL == query)
    {
      source.transferStatus = eGetItemRetError;
      LOG_ERROR() << "Cannot create query" << std::endl;
      pthread_mutex_lock(&source.mutex);
      pthread_cond_signal(&source.bufferReadyCond);
      pthread_mutex_unlock(&source.mutex);
      return NULL;
    }

    //try 5 times using exponential backoff to query contacts
    EXP_BACKOFF(5)
    {
      feed = gdata_contacts_service_query_contacts(source.service,
                                                   GDATA_QUERY(query),
                                                   source.cancellable, NULL, NULL,
                                                   &gerror);
      if (NULL != feed)
      {
        break;
      }
      GERROR_FREE(gerror);
    }
    g_object_unref(query);

    if (NULL == feed)
    {
      LOG_ERROR() << "Cannot query contacts " << GERROR_MESSAGE(gerror) << std::endl;
      source.transferStatus = eGetItemRetError;
      GERROR_FREE(gerror);
      pthread_mutex_lock(&source.mutex);
      pthread_cond_signal(&source.bufferReadyCond);
      pthread_mutex_unlock(&source.mutex);
      return NULL;
    }

    //convert contacts returned by query
    for (contactsList = gdata_feed_get_entries(feed); contactsList != NULL;
        contactsList = contactsList->next, numDownloaded++)
    {
      GDataContactsContact *contact = GDATA_CONTACTS_CONTACT(contactsList->data);
      gsize photoDataLen = 0;
      gchar *photoContentType = NULL;
      guint8* photoData = NULL;

      //check if operation was cancelled
      if (source.cancelled)
      {
        g_object_unref(feed);
        pthread_mutex_lock(&source.mutex);
        pthread_cond_signal(&source.bufferReadyCond);
        pthread_mutex_unlock(&source.mutex);
        return NULL;
      }

      if (source.downloadPhoto())
      {
        const gchar* photoETag = gdata_contacts_contact_get_photo_etag(contact);
        if (photoETag)
        {
          EXP_BACKOFF(5)
          {
            photoData = gdata_contacts_contact_get_photo(contact,
                                                         source.service,
                                                         &photoDataLen,
                                                         &photoContentType,
                                                         NULL,
                                                         &gerror);
            if (NULL == gerror)
            {
              break;
            }
            GERROR_FREE(gerror);
          }
          if (gerror)
          {
            LOG_ERROR() << "Cannot download photo " << GERROR_MESSAGE(gerror) << std::endl;
            source.transferStatus = eGetItemRetError;
            GERROR_FREE(gerror);
            g_object_unref(feed);
            pthread_mutex_lock(&source.mutex);
            pthread_cond_signal(&source.bufferReadyCond);
            pthread_mutex_unlock(&source.mutex);
            return NULL;
          }
        }

      }

      //convert contact is freeing photoData and unrefing contact
      std::string vCard = source.convertContact(contact, photoData, photoDataLen, photoContentType);

      pthread_mutex_lock(&source.mutex);
      source.bufferedVCards.push_back(vCard);
      pthread_cond_signal(&source.bufferReadyCond);
      pthread_mutex_unlock(&source.mutex);
    }
    g_object_unref(feed);

    offset += QUERY_SIZE;
    pthread_mutex_lock(&source.mutex);
    source.totalNumber += numDownloaded;
    LOG_ERROR() << "TOTAL DOWNLOADED " << source.totalNumber << std::endl;
    if (numDownloaded < QUERY_SIZE)
    {
      source.transferStatus = eGetItemRetEnd;
      pthread_cond_signal(&source.bufferReadyCond);
      numDownloaded = 0;
      pthread_mutex_unlock(&source.mutex);
      return NULL;
    }
    pthread_mutex_unlock(&source.mutex);
  }

  pthread_mutex_lock(&source.mutex);
  pthread_cond_signal(&source.bufferReadyCond);
  pthread_mutex_unlock(&source.mutex);
  return NULL;
}

namespace {
class GoogleFactory : OpenAB_Source::Factory
{
  public:
    /*!
     *  @brief Constructor.
     */
    GoogleFactory()
        : Factory::Factory("Google")
    {
      LOG_FUNC();
    }
    ;

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~GoogleFactory()
    {
      LOG_FUNC();
    }
    ;

    OpenAB_Source::Source * newIstance(const OpenAB_Source::Parameters & params)
    {
      LOG_FUNC();
      std::string login;
      OpenAB::SecureString password;
      std::string clientId;
      OpenAB::SecureString clientSecret;
      OpenAB::SecureString refreshToken;
      std::string ignoreFields = "";
      GoogleSource* src = NULL;
      OpenAB::Variant param;

      bool useOAuth2 = true;
      if (params.getValue("refresh_token").invalid() || params.getValue("client_id").invalid()
          || params.getValue("client_secret").invalid())
      {
        useOAuth2 = false;
      }

      if (useOAuth2)
      {
        if (OpenAB::Variant::STRING != params.getValue("client_id").getType()
            || OpenAB::Variant::SECURE_STRING != params.getValue("client_secret").getType()
            || OpenAB::Variant::SECURE_STRING != params.getValue("refresh_token").getType())
        {
          LOG_ERROR() << "Wrong type of parameters" << std::endl;
          return NULL;
        }

        clientId = params.getValue("client_id").getString();
        clientSecret = params.getValue("client_secret").getSecureString();
        refreshToken = params.getValue("refresh_token").getSecureString();
      }
      else
      {
        param = params.getValue("login");
        if (param.invalid())
        {
          LOG_ERROR() << "Parameter 'login' not found" << std::endl;
          return NULL;
        }
        login = param.getString();

        param = params.getValue("password");
        if (param.invalid() || OpenAB::Variant::SECURE_STRING != param.getType())
        {
          LOG_ERROR() << "Parameter 'password' not found" << std::endl;
          return NULL;
        }
        password = param.getSecureString();
      }

      param = params.getValue("ignore_fields");
      if (!param.invalid())
      {
        ignoreFields = param.getString();
      }

      if (useOAuth2)
      {
        src = new GoogleSource(clientId, clientSecret, refreshToken, ignoreFields);
      }
      else
      {
        src = new GoogleSource(login, password, ignoreFields);
      }
      if (NULL == src)
      {
        LOG_ERROR() << "Cannot Initialize GoogleSource" << std::endl;
        return NULL;
      }

      return src;
    }
};
}

REGISTER_PLUGIN_FACTORY(GoogleFactory);
