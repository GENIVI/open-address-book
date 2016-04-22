/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CardDAVStorage.cpp
 */

#include "CardDAVStorage.hpp"
#include <PIMItem/Contact/PIMContactItem.hpp>
#include <algorithm>
#include <math.h>
#include <locale>
#include <unistd.h>
#include "helpers/OAuth2HttpAuthorizer.hpp"
#include "helpers/BasicHttpAuthorizer.hpp"
#include <OpenAB.hpp>


#define QUERY_SIZE 1000

#define EXP_BACKOFF(numRetries) \
  for (unsigned int _i = 0; _i < (numRetries); ++_i, usleep(pow(2.0, _i) * 10))

CardDAVStorage::CardDAVStorage(const std::string& url,
                               const std::string& login,
                               const OpenAB::SecureString& password)
    : OpenAB_Storage::ContactsStorage(),
      serverUrl (url),
      userLogin(login),
      userPassword(password),
      clientId(),
      clientSecret(),
      refreshToken(),
      syncToken(),
      authorizer(NULL),
      cardDAVHelper(NULL),
      sourceIterator(NULL)
{
  LOG_FUNC();
  construct();
}

CardDAVStorage::CardDAVStorage(const std::string& url,
                               const std::string& clientId,
                               const OpenAB::SecureString& clientSecret,
                               const OpenAB::SecureString& refreshToken)
    : OpenAB_Storage::ContactsStorage(),
      serverUrl (url),
      userLogin(),
      userPassword(),
      clientId(clientId),
      clientSecret(clientSecret),
      refreshToken(refreshToken),
      syncToken(),
      authorizer(NULL),
      cardDAVHelper(NULL),
      sourceIterator(NULL)
{
  LOG_FUNC();
  construct();
}

void CardDAVStorage::construct()
{
  std::cout << "****in construct***" << std::endl;
  curlSession.init();
}

CardDAVStorage::~CardDAVStorage()
{
  cleanup();

  clientSecret.clear();
  refreshToken.clear();
  userPassword.clear();

  std::cout << "***** destructor *****" << std::endl;
}

void CardDAVStorage::cleanup()
{
  std::cout << "**** in cleanup ****" << std::endl;
  if (cardDAVHelper)
  {
    delete cardDAVHelper;
    cardDAVHelper = NULL;
  }

  if (authorizer)
  {
    delete authorizer;
    authorizer = NULL;
  }
  LOG_FUNC();
}

enum OpenAB_Storage::Storage::eInit CardDAVStorage::init()
{
  LOG_DEBUG()<<"Initializing CardDAV"<<std::endl;
  cleanup();

  if (!userLogin.empty())
  {
    OpenAB::BasicHttpAuthorizer* basicAuthorizer = new OpenAB::BasicHttpAuthorizer();
    authorizer = basicAuthorizer;
    basicAuthorizer->setCredentials(userLogin, userPassword);
  }
  else
  {
    OpenAB::OAuth2HttpAuthorizer* oauth2Authorizer = new OpenAB::OAuth2HttpAuthorizer();
    authorizer = oauth2Authorizer;

    bool authorizationSuccesful = false;

    authorizationSuccesful = oauth2Authorizer->authorize(clientId, clientSecret, refreshToken);
    if (!authorizationSuccesful)
    {
      LOG_ERROR() << "Cannot authenticate user"<<std::endl;
      return eInitFail;
    }
  }

  cardDAVHelper = new CardDAVHelper(serverUrl, &curlSession, authorizer);

  if (!cardDAVHelper->findPrincipalUrl())
  {
    LOG_ERROR() << "Cannot connect to CardDAV server"<<std::endl;
    return eInitFail;
  }

  if (!cardDAVHelper->findAddressbookSet())
  {
    LOG_ERROR() << "Cannot connect to CardDAV server"<<std::endl;
    return eInitFail;
  }

  if (!cardDAVHelper->findAddressbooks())
  {
    LOG_ERROR() << "Cannot connect to CardDAV server"<<std::endl;
    return eInitFail;
  }

  return eInitOk;
}

enum OpenAB_Storage::Storage::eAddItem CardDAVStorage::addContact(const std::string& vCard,
                                                               OpenAB::PIMItem::ID& newId,
                                                               OpenAB::PIMItem::Revision& revision)
{
  if (!cardDAVHelper->addContact(vCard, newId, revision))
  {
    return eAddItemFail;
  }
  return eAddItemOk;
}

enum OpenAB_Storage::Storage::eAddItem CardDAVStorage::addContacts(const std::vector<std::string> &vCards,
                                                                OpenAB::PIMItem::IDs& newIds,
                                                                OpenAB::PIMItem::Revisions& revisions)
{
  newIds.clear();
  for (unsigned int i = 0; i < vCards.size(); ++i)
  {
    std::string newId;
    std::string etag;
    if (eAddItemFail == addContact(vCards[i], newId, etag))
    {
      newIds.clear();
      revisions.clear();
      return eAddItemFail;
    }
    newIds.push_back(newId);
    revisions.push_back(etag);
  }
  return eAddItemOk;
}

enum OpenAB_Storage::Storage::eModifyItem CardDAVStorage::modifyContact( const std::string& vCard,
                                                                      const OpenAB::PIMItem::ID& id,
                                                                      OpenAB::PIMItem::Revision& revision)
{
  if (!cardDAVHelper->modifyContact(id, vCard, revision))
  {
    return eModifyItemFail;
  }
  return eModifyItemOk;
}

enum OpenAB_Storage::Storage::eModifyItem CardDAVStorage::modifyContacts( const std::vector<std::string> &vCard,
                                                                       const OpenAB::PIMItem::IDs& ids,
                                                                       OpenAB::PIMItem::Revisions& revisions)
{
  for (unsigned int i = 0; i < vCard.size(); ++i)
  {
    std::string newId;
    std::string etag;
    if (eModifyItemFail == modifyContact(vCard[i], ids[i], etag))
    {
      revisions.clear();
      return eModifyItemFail;
    }
    revisions.push_back(etag);
  }
  return eModifyItemOk;
}

enum OpenAB_Storage::Storage::eRemoveItem CardDAVStorage::removeContact( const OpenAB::PIMItem::ID& id)
{
  if (!cardDAVHelper->removeContact(id))
  {
    return eRemoveItemFail;
  }
  return eRemoveItemOk;
}

enum OpenAB_Storage::Storage::eRemoveItem CardDAVStorage::removeContacts( const OpenAB::PIMItem::IDs& ids)
{
  for (unsigned int i = 0; i < ids.size(); ++i)
  {
    std::string newId;
    if (eRemoveItemFail == removeContact(ids[i]))
    {
      return eRemoveItemFail;
    }
  }
  return eRemoveItemOk;
}

enum OpenAB_Storage::Storage::eGetItem CardDAVStorage::getContact (const OpenAB::PIMItem::ID & id,
                                                                OpenAB::SmartPtr<OpenAB::PIMContactItem> & item)
{
  std::vector<std::string> uris;
  uris.push_back(id);
  std::vector<std::string> vcards;

  if (!cardDAVHelper->downloadVCards(uris, vcards))
  {
    return eGetItemFail;
  }
  if (vcards.size() != 1)
  {
    return eGetItemFail;
  }
  OpenAB::PIMContactItem* newContact = new OpenAB::PIMContactItem();
  if (!newContact->parse(vcards.at(0)))
  {
    delete newContact;
    return eGetItemFail;
  }
  newContact->setId(id);
  //update contacts with revisions information
  CardDAVHelper::ContactsMetadata md = cardDAVHelper->getContactsMetadata();
  CardDAVHelper::ContactsMetadata::iterator it;
  for (it = md.begin(); it != md.end(); ++it)
  {
    if ((*it).uri == id)
    {
      newContact->setRevision((*it).etag);
      break;
    }
  }
  item = newContact;
  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetItem CardDAVStorage::getContacts (const OpenAB::PIMItem::IDs & ids,
                                                                std::vector<OpenAB::SmartPtr<OpenAB::PIMContactItem> > & items)
{
  std::vector<std::string> uris;
  for (unsigned int i = 0; i < ids.size(); ++i)
  {
    uris.push_back(ids[i]);
  }
  std::vector<std::string> vcards;

  if (!cardDAVHelper->downloadVCards(uris, vcards))
  {
    LOG_ERROR()<<"Download vcards failed"<<std::endl;
    return eGetItemFail;
  }

  for (unsigned int i = 0; i < vcards.size(); ++i)
  {
    OpenAB::PIMContactItem* newContact = new OpenAB::PIMContactItem();
    if (!newContact->parse(vcards.at(i)))
    {
      delete newContact;
      LOG_ERROR()<<"Cannot parse vcards"<<std::endl;
      return eGetItemFail;
    }
    newContact->setId(ids[i]);
    CardDAVHelper::ContactsMetadata md = cardDAVHelper->getContactsMetadata();
    CardDAVHelper::ContactsMetadata::iterator it;
    for (it = md.begin(); it != md.end(); ++it)
    {
      if ((*it).uri == ids[i])
      {
        newContact->setRevision((*it).etag);
        break;
      }
    }
    items.push_back(newContact);
  }
  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetRevisions CardDAVStorage::getRevisions(std::map<std::string, std::string>& revisions)
{
  if (!cardDAVHelper->queryContactsMetadata())
  {
    LOG_FUNC()<<" Cannot query metadata"<<std::endl;
    return eGetRevisionsFail;
  }
  CardDAVHelper::ContactsMetadata contactsMetadata = cardDAVHelper->getContactsMetadata();
  CardDAVHelper::ContactsMetadata::iterator it;
  for (it = contactsMetadata.begin(); it != contactsMetadata.end(); ++it)
  {
    revisions[(*it).uri] = (*it).etag;
  }

  return eGetRevisionsOk;
}

enum OpenAB_Storage::Storage::eGetRevisions CardDAVStorage::getChangedRevisions(const std::string& token,
                                                                        std::map<std::string, std::string>& revisions,
                                                                        std::vector<OpenAB::PIMItem::ID>& removed)
{
  if (token.empty())
  {
    return eGetRevisionsFail;
  }

  if (!cardDAVHelper->queryChangedContactsMetadata(token, removed))
  {
    LOG_FUNC()<<" Cannot query metadata"<<std::endl;
    return eGetRevisionsFail;
  }
  syncToken = token;
  CardDAVHelper::ContactsMetadata contactsMetadata = cardDAVHelper->getContactsMetadata();
  CardDAVHelper::ContactsMetadata::iterator it;
  for (it = contactsMetadata.begin(); it != contactsMetadata.end(); ++it)
  {
    revisions[(*it).uri] = (*it).etag;
  }

  return eGetRevisionsOk;
}

enum OpenAB_Storage::Storage::eGetSyncToken CardDAVStorage::getLatestSyncToken(std::string& token)
{
  if (!cardDAVHelper->queryAddressbookMetadata())
  {
    return eGetSyncTokenFail;
  }

  token = cardDAVHelper->getSyncToken();

  return eGetSyncTokenOk;
}

OpenAB_Storage::StorageItemIterator* CardDAVStorage::newStorageItemIterator()
{
  CardDAVStorageItemIterator * ie = new CardDAVStorageItemIterator();
  if (NULL == ie)
  {
    LOG_ERROR() << "Error Cannot create the CardDAV IndexElemIterator"<<std::endl;
    return NULL;
  }
  if (ie->eCursorInitOK != ie->cursorInit(cardDAVHelper))
  {
    LOG_ERROR() << "Error Cannot Init the CardDAV IndexElemIterator"<<std::endl;
    delete ie;
    return NULL;
  }
  return ie;
}

enum OpenAB_Source::Source::eGetItemRet CardDAVStorage::getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  if (NULL == sourceIterator)
  {
    sourceIterator = static_cast<CardDAVStorageItemIterator*>(newStorageItemIterator());
  }

  OpenAB_Storage::StorageItem* stItem = sourceIterator->next();

  if (stItem == NULL)
  {
    delete sourceIterator;
    sourceIterator = NULL;
    return OpenAB_Source::Source::eGetItemRetEnd;
  }

  item = stItem->item;

  return OpenAB_Source::Source::eGetItemRetOk;
}

enum OpenAB_Source::Source::eSuspendRet CardDAVStorage::suspend()
{
  /*if (sourceIterator)
  {
    return sourceIterator->suspend();
  }*/
  return OpenAB_Source::Source::eSuspendRetFail;
}

enum OpenAB_Source::Source::eResumeRet CardDAVStorage::resume()
{
  /*if (sourceIterator)
  {
    return sourceIterator->resume();
  }*/
  return OpenAB_Source::Source::eResumeRetFail;
}

enum OpenAB_Source::Source::eCancelRet CardDAVStorage::cancel()
{
  /*if (sourceIterator)
  {
    return sourceIterator->cancel();
  }*/
  return OpenAB_Source::Source::eCancelRetFail;
}

int CardDAVStorage::getTotalCount() const
{
  if (sourceIterator)
  {
    return sourceIterator->getSize();
  }
  return 0;
}

CardDAVStorageItemIterator::CardDAVStorageItemIterator() :
    cardDavHelper(NULL),
    total(0),
    offset(0),
    offsetOfCachedVCards(0),
    paused(false),
    cancelled(false),
    threadCreated(false),
    transferStatus(OpenAB_Source::Source::eGetItemRetOk)
{
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&bufferReadyCond, NULL);
}

CardDAVStorageItemIterator::~CardDAVStorageItemIterator()
{
  cancelled = true;
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&bufferReadyCond);
  if (threadCreated)
  {
    pthread_join(downloadThread, NULL);
  }
}

enum CardDAVStorageItemIterator::eCursorInit CardDAVStorageItemIterator::cursorInit(CardDAVHelper* helper)
{
  cardDavHelper = helper;

  if (!cardDavHelper->queryContactsMetadata())
  {
    return eCursorInitFail;
  }
  contactsMetadata = cardDavHelper->getContactsMetadata();
  total = contactsMetadata.size();


  offsetOfCachedVCards = 0;
  offset = 0;
  cachedContacts.clear();

  cancelled = false;
  transferStatus = OpenAB_Source::Source::eGetItemRetOk;
  pthread_create(&downloadThread, NULL, downloadThreadFunc, this);
  threadCreated = true;

  return eCursorInitOK;
}

void* CardDAVStorageItemIterator::downloadThreadFunc(void* ptr)
{
  CardDAVStorageItemIterator& iterator = *static_cast<CardDAVStorageItemIterator*>(ptr);
  unsigned int offset = 0;

  while (offset < iterator.total)
  {
    while (iterator.paused && !iterator.cancelled)
    {
      usleep(1000);
    }

    if (iterator.cancelled)
    {
      pthread_mutex_lock(&iterator.mutex);
      pthread_cond_signal(&iterator.bufferReadyCond);
      pthread_mutex_unlock(&iterator.mutex);
      return NULL;
    }

    if (!iterator.downloadVCards(offset, QUERY_SIZE))
    {
      LOG_DEBUG()<<"DownloadThread download error"<<std::endl;
      iterator.transferStatus = OpenAB_Source::Source::eGetItemRetError;
      pthread_mutex_lock(&iterator.mutex);
      pthread_cond_signal(&iterator.bufferReadyCond);
      pthread_mutex_unlock(&iterator.mutex);
      return NULL;
    }
    offset += QUERY_SIZE;
  }

  pthread_mutex_lock(&iterator.mutex);
  iterator.transferStatus = OpenAB_Source::Source::eGetItemRetEnd;
  pthread_cond_signal(&iterator.bufferReadyCond);
  pthread_mutex_unlock(&iterator.mutex);
  return NULL;
}

bool CardDAVStorageItemIterator::downloadVCards(unsigned int offset, unsigned int size)
{
  LOG_FUNC();
  std::vector<std::string> ids;
  std::vector<std::string> vcards;

  unsigned int len = offset + size;
  if (len > contactsMetadata.size())
  {
    len = contactsMetadata.size();
  }

  for (unsigned int i = offset; i < len; ++i)
  {
    ids.push_back(contactsMetadata[i].uri);
  }
  if (!cardDavHelper->downloadVCards(ids, vcards))
  {
    return false;
  }

  pthread_mutex_lock(&mutex);
  for (unsigned int i = 0; i < vcards.size(); ++i)
  {
    OpenAB::PIMContactItem* newItem = new OpenAB::PIMContactItem();
    newItem->parse(vcards.at(i));
    newItem->setId(contactsMetadata[offset + i].uri);
    newItem->setRevision(contactsMetadata[offset + i].etag);
    cachedContacts.push_back(newItem);
  }
  pthread_cond_signal(&bufferReadyCond);
  pthread_mutex_unlock(&mutex);

  return true;
}

OpenAB_Storage::StorageItem* CardDAVStorageItemIterator::next()
{
  pthread_mutex_lock(&mutex);

  if (cachedContacts.empty())
  {
    if (OpenAB_Source::Source::eGetItemRetEnd == transferStatus)
    {
      //Cache empty, download finished
      pthread_mutex_unlock(&mutex);
      return NULL;
    }
    //Cache empty, waiting for new batch
    pthread_cond_wait(&bufferReadyCond, &mutex);
  }

  if (OpenAB_Source::Source::eGetItemRetEnd == transferStatus && cachedContacts.empty())
  {
    //Download end
    pthread_mutex_unlock(&mutex);
    return NULL;
  }

  if (OpenAB_Source::Source::eGetItemRetError == transferStatus)
  {
    //Download error
    pthread_mutex_unlock(&mutex);
    return NULL;
  }

  OpenAB::SmartPtr<OpenAB::PIMContactItem> nextItem = cachedContacts.front();
  cachedContacts.pop_front();
  pthread_mutex_unlock(&mutex);

  elem.item = new OpenAB::PIMContactItem(*nextItem.getPointer());
  elem.id = nextItem->getId();
  return &elem;
}

OpenAB_Storage::StorageItem CardDAVStorageItemIterator::operator*()
{
  LOG_FUNC();
  return elem;
}

OpenAB_Storage::StorageItem* CardDAVStorageItemIterator::operator->()
{
  LOG_FUNC();
  return &elem;
}

unsigned int CardDAVStorageItemIterator::getSize() const
{
  return total;
}

namespace CardDAVFactory
{
OpenAB_Storage::Storage* createInstance (const OpenAB_Storage::Parameters& params)
{
  LOG_FUNC();
  std::string login;
  OpenAB::SecureString password;
  std::string clientId;
  OpenAB::SecureString clientSecret;
  OpenAB::SecureString refreshToken;
  std::string ignoreFields = "";
  std::string serverUrl = "";
  CardDAVStorage* src = NULL;
  OpenAB::Variant param;

  param = params.getValue("server_url");
  if (param.invalid() || OpenAB::Variant::STRING != param.getType())
  {
    LOG_ERROR() << "Server url not provided" << std::endl;
    return NULL;
  }
  serverUrl = param.getString();

  bool useOAuth2 = true;
  if (params.getValue("refresh_token").invalid() || params.getValue("client_id").invalid()
      || params.getValue("client_secret").invalid())
  {
    useOAuth2 = false;
  }

  if (useOAuth2)
  {
    if (OpenAB::Variant::STRING != params.getValue("client_id").getType()
        || OpenAB::Variant::STRING != params.getValue("client_secret").getType()
        || OpenAB::Variant::STRING != params.getValue("refresh_token").getType())
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
    if (param.invalid() || OpenAB::Variant::STRING != param.getType())
    {
      LOG_ERROR() << "Parameter 'password' not found" << std::endl;
      return NULL;
    }
    password = param.getString();
  }

  param = params.getValue("ignore_fields");
  if (!param.invalid())
  {
    ignoreFields = param.getString();
  }

  if (useOAuth2)
  {
    src = new CardDAVStorage(serverUrl, clientId, clientSecret, refreshToken);
  }
  else
  {
    src = new CardDAVStorage(serverUrl, login, password);
  }
  if (NULL == src)
  {
    LOG_ERROR() << "Cannot Initialize CardDAV" << std::endl;
    return NULL;
  }

  return src;
}

class CardDAVSourceFactory : OpenAB_Source::Factory
{
  public:
    /*!
     *  @brief Constructor.
     */
  CardDAVSourceFactory()
        : Factory::Factory("CardDAV")
    {
      LOG_FUNC();
    }
    ;

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~CardDAVSourceFactory()
    {
      LOG_FUNC();
    }
    ;

    OpenAB_Source::Source * newIstance(const OpenAB_Source::Parameters & params)
    {
      return createInstance(params);
    }
};


class CardDAVStorageFactory : OpenAB_Storage::Factory
{
  public:
    /*!
     *  @brief Constructor.
     */
    CardDAVStorageFactory()
        : Factory::Factory("CardDAV")
    {
      LOG_FUNC();
    }
    ;

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~CardDAVStorageFactory()
    {
     // LOG_FUNC();
    }
    ;

    OpenAB_Storage::Storage * newIstance(const OpenAB_Storage::Parameters & params)
    {
      return createInstance(params);
    }
};
}

namespace CardDAVSourceFactory
{
REGISTER_PLUGIN_FACTORY(CardDAVFactory::CardDAVSourceFactory);
}

namespace CardDAVStorageFactory
{
REGISTER_PLUGIN_FACTORY(CardDAVFactory::CardDAVStorageFactory);
}
