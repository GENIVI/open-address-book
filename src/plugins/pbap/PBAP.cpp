/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PBAP.cpp
 */

#include "PBAP.hpp"
#include <PIMItem/Contact/PIMContactItem.hpp>
#include <OpenAB.hpp>
#include <algorithm>
#include <locale>
#include <fcntl.h>

PBAPSource::PBAPSource(const std::string& mac,
                       const std::string& pb,
                       const std::string& ignoreFields,
                       unsigned int batchDownloadTime)
    : OpenAB_Source::Source(OpenAB::eContact),
      macAddress(mac),
      pbLocation(pb),
      batchDownloadDesiredTime(batchDownloadTime),
      tempFIFO("NA"),
      fifoBuffer(),
      threadCreated(false),
      connection(NULL),
      proxyClient1(NULL),
      proxyPhonebookAccess1(NULL),
      proxyTransfer1(NULL),
      transferStatus(eGetItemRetOk),
      sessionPath(),
      phonebookSize(0)
{
  LOG_FUNC();
  if (!ignoreFields.empty())
  {
    std::string field;
    size_t pos = 0;
    size_t nextPos = std::string::npos;
    while(std::string::npos != (nextPos = ignoreFields.find_first_of(',', pos)))
    {
      field = ignoreFields.substr(pos, nextPos - pos);
      LOG_DEBUG()<<"Ignore field " << field<<std::endl;
      pos = nextPos+1;
      //remove spaces
      field.erase(std::remove_if(field.begin(), field.end(), ::isspace), field.end());
      ignoredFields.push_back(field);
    }
    field = ignoreFields.substr(pos);
    field.erase(std::remove_if(field.begin(), field.end(), ::isspace), field.end());
    ignoredFields.push_back(field);
    LOG_DEBUG()<<"Ignore field " << field<<std::endl;
  }
}

PBAPSource::~PBAPSource()
{
  cleanup();
}

void PBAPSource::cleanup()
{
  LOG_FUNC();

  if(threadCreated)
  {
    pthread_join(threadFIFO, NULL);
  }

  if(transfer.isInitialized())
  {
    transfer.cancel();
  }

  if (NULL != proxyTransfer1)
  {
    g_object_unref(proxyTransfer1);
  }

  if (NULL != proxyPhonebookAccess1)
  {
    g_object_unref(proxyPhonebookAccess1);
  }

  if (NULL != proxyClient1)
  {
    g_object_unref(proxyClient1);
  }

  if (NULL != connection)
  {
    g_object_unref(connection);
  }
}

enum OpenAB_Source::Source::eInit PBAPSource::init()
{
  LOG_FUNC() << "Open PBAP: " << macAddress<<std::endl;
  cleanup();

  GError * gerror = NULL;

#if GLIB_VERSION_MIN_REQUIRED < G_ENCODE_VERSION(2,36)
  /* g_type_init has been deprecated since 2.36 */
  g_type_init();
#endif

  connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &gerror);

  if (NULL == connection){
    LOG_ERROR() << "Cannot open connection to bus: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return eInitFail;
  }

  if (!createSession())
  {
    return eInitFail;
  }

  if (!selectPhonebook())
  {
    return eInitFail;
  }

  if (!getPhonebookSize())
  {
    return eInitFail;
  }

  if (!getSupportedFilters())
  {
    return eInitFail;
  }

  if(!ignoredFields.empty())
  {
    std::vector<std::string>::iterator it;
    std::vector<std::string>::iterator it2;
    for(it = supportedFilters.begin(); it != supportedFilters.end(); ++it)
    {
      bool ignoreField = false;
      for(it2 = ignoredFields.begin(); it2 != ignoredFields.end(); ++it2)
      {
        std::string field1 ((*it));
        std::string field2 ((*it2));
        std::transform( field1.begin(), field1.end(), field1.begin(), ::tolower );
        std::transform( field2.begin(), field2.end(), field2.begin(), ::tolower );

        if(field1 == field2)
        {
          ignoreField = true;
          break;
        }
      }
      if(!ignoreField)
        filters.push_back((*it));
    }
  }
  else
  {
    filters = supportedFilters;
  }
  std::vector<std::string>::iterator it;
  LOG_DEBUG()<<"FILTERS:"<<std::endl;
  for(it = filters.begin(); it != filters.end(); ++it)
  {
    LOG_DEBUG()<<(*it)<<std::endl;
  }

  threadCreated = true;
  pthread_create(&threadFIFO,NULL,threadFuncFIFOWrapper,this);

  return eInitOk;
}

bool PBAPSource::createSession()
{
  GError* gerror = NULL;
  GVariantBuilder *builder;
  GVariant* response = NULL;
  GVariant* sessionPathVariant = NULL;
  bool res = true;

  proxyClient1 = g_dbus_proxy_new_sync(connection,
                                       G_DBUS_PROXY_FLAGS_NONE,
                                       NULL,
                                       "org.bluez.obex",
                                       "/org/bluez/obex",
                                       "org.bluez.obex.Client1",
                                       NULL, &gerror);

  if (NULL != gerror)
  {
    LOG_ERROR() << "Cannot create org.bluez.obex.Client1 proxy : " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return false;
  }

  builder = g_variant_builder_new(G_VARIANT_TYPE_ARRAY);
  g_variant_builder_add(builder, "{sv}", "Target", g_variant_new_string("PBAP"));

  response = g_dbus_proxy_call_sync(proxyClient1,
                                    "CreateSession",
                                    g_variant_new("(sa{sv})",
                                                  macAddress.c_str(),
                                                  builder),
                                    G_DBUS_CALL_FLAGS_NONE, -1,
                                    NULL, &gerror);
  g_variant_builder_unref (builder);
  if (NULL == response)
  {
    LOG_ERROR() << "Cannot call CreateSession : " << GERROR_MESSAGE(gerror)<<std::endl;
    if (NULL != gerror && gerror->domain == G_DBUS_ERROR)
    {
      char* err = g_dbus_error_get_remote_error(gerror);
      LOG_ERROR() << "Exception: " << err<<std::endl;
      g_free(err);
      GERROR_FREE(gerror);
    }
    res = false;
  }

  if (res)
  {
    sessionPathVariant = g_variant_get_child_value(response, 0);
    sessionPath = g_variant_get_string(sessionPathVariant, NULL);
    g_variant_unref(sessionPathVariant);

    proxyPhonebookAccess1 = g_dbus_proxy_new_sync(connection,
                                                  G_DBUS_PROXY_FLAGS_NONE,
                                                  NULL,
                                                  "org.bluez.obex",
                                                  sessionPath.c_str(),
                                                  "org.bluez.obex.PhonebookAccess1",
                                                  NULL, &gerror);

    if (NULL != gerror)
    {
      LOG_ERROR() << "Cannot create org.bluez.obex.PhonebookAccess1 proxy : " << GERROR_MESSAGE(gerror)<<std::endl;
      GERROR_FREE(gerror);
      res = false;
    }
  }

  if (NULL != response)
  {
    g_variant_unref(response);
  }
  return res;
}

bool PBAPSource::disconnectSession()
{
  if(sessionPath.empty())
  {
    return false;
  }

  GError* gerror = NULL;
  GVariant* response = NULL;
  bool res = true;

  LOG_DEBUG()<<"Disconnectin session "<<sessionPath<<std::endl;
  response = g_dbus_proxy_call_sync(proxyClient1,
                                    "RemoveSession",
                                    g_variant_new("(o)", sessionPath.c_str()),
                                    G_DBUS_CALL_FLAGS_NONE, -1,
                                    NULL, &gerror);
  if (NULL == response)
  {
    LOG_ERROR() << "Cannot call RemoveSession : " << GERROR_MESSAGE(gerror)<<std::endl;
    if (NULL != gerror && gerror->domain == G_DBUS_ERROR)
    {
      char* err = g_dbus_error_get_remote_error(gerror);
      LOG_ERROR() << "Exception: " << err<<std::endl;
      g_free(err);
      GERROR_FREE(gerror);
    }
    res = false;
  }

  if (res)
  {
    sessionPath.clear();
    transfer.clean();
    g_variant_unref(response);
  }

  return res;
}

bool PBAPSource::selectPhonebook()
{
  GError* gerror = NULL;
  GVariant* response;
  bool res = true;

  response = g_dbus_proxy_call_sync(proxyPhonebookAccess1,
                                    "Select",
                                    g_variant_new("(ss)", pbLocation.c_str(), "pb"),
                                    G_DBUS_CALL_FLAGS_NONE, -1,
                                    NULL, &gerror);
  if (!response)
  {
    LOG_ERROR() << "Cannot call Select : " << GERROR_MESSAGE(gerror)<<std::endl;
    if (NULL != gerror && gerror->domain == G_DBUS_ERROR)
    {
      char* err = g_dbus_error_get_remote_error(gerror);
      LOG_ERROR() << "Exception: " << err<<std::endl;
      g_free(err);
      GERROR_FREE(gerror);
    }
    res = false;
  }

  if (res)
  {
    g_variant_unref(response);
  }
  return res;
}

bool PBAPSource::getPhonebookSize()
{
  GError* gerror = NULL;
  GVariant* response;
  bool res = true;

  response = g_dbus_proxy_call_sync(proxyPhonebookAccess1,
                                    "GetSize",
                                    NULL,
                                    G_DBUS_CALL_FLAGS_NONE, -1,
                                    NULL, &gerror);
  if(!response)
  {
    LOG_ERROR() << "Cannot call GetSize : " << GERROR_MESSAGE(gerror)<<std::endl;
    if (NULL != gerror && gerror->domain == G_DBUS_ERROR)
    {
      char* err = g_dbus_error_get_remote_error(gerror);
      LOG_ERROR() << "Exception: " << err<<std::endl;
      g_free(err);
      GERROR_FREE(gerror);
    }
    res = false;
  }

  if (res)
  {
    GVariant* pb_size = g_variant_get_child_value(response, 0);
    phonebookSize = g_variant_get_uint16(pb_size);
    g_variant_unref(pb_size);
    g_variant_unref(response);
  }

  return res;
}

bool PBAPSource::getSupportedFilters()
{
  GError* gerror = NULL;
  GVariant* response;
  GVariant* filters;
  const char ** filterList;
  bool res = true;

  response = g_dbus_proxy_call_sync(proxyPhonebookAccess1,
                                    "ListFilterFields",
                                    NULL,
                                    G_DBUS_CALL_FLAGS_NONE, -1,
                                    NULL, &gerror);
  if(!response)
  {
    LOG_ERROR() << "Cannot call ListFilterFields : " << GERROR_MESSAGE(gerror)<<std::endl;
    if (NULL != gerror && gerror->domain == G_DBUS_ERROR)
    {
      char* err = g_dbus_error_get_remote_error(gerror);
      LOG_ERROR() << "Exception: " << err<<std::endl;
      g_free(err);
      GERROR_FREE(gerror);
    }
    res = false;
  }

  if (res)
  {
    filters = g_variant_get_child_value(response, 0);
    filterList = g_variant_get_strv(filters, NULL);
    LOG_DEBUG() << "Supported filters :"<<std::endl;
    for (const char ** filter = filterList;NULL != *filter;++filter){
      LOG_DEBUG() << *filter<<std::endl;
      supportedFilters.push_back(*filter);
    }
    g_free(filterList);
    g_variant_unref(filters);
    g_variant_unref(response);
  }

  return res;
}

bool PBAPSource::pullPhonebook(unsigned int offset, unsigned int count)
{
  GError* gerror = NULL;
  GVariantBuilder *builder;
  GVariant* response;
  GVariant* transferPathVariant;
  bool res = true;
  const gchar** fields = new const gchar*[filters.size()];
  for(unsigned int i = 0; i < filters.size(); ++i)
  {
    fields[i] = filters[i].c_str();
  }

  builder = g_variant_builder_new(G_VARIANT_TYPE_ARRAY);
  g_variant_builder_add(builder, "{sv}", "Format", g_variant_new_string("vcard30"));
  g_variant_builder_add(builder, "{sv}", "Fields", g_variant_new_strv(fields, filters.size()));
  g_variant_builder_add(builder, "{sv}", "Offset", g_variant_new_uint16(offset));
  g_variant_builder_add(builder, "{sv}", "MaxCount", g_variant_new_uint16(count));


  response = g_dbus_proxy_call_sync(proxyPhonebookAccess1,
                                    "PullAll",
                                    g_variant_new("(sa{sv})",
                                                  tempFIFO.c_str(),
                                                  builder),
                                    G_DBUS_CALL_FLAGS_NONE, -1,
                                    NULL, &gerror);
  g_variant_builder_unref (builder);
  if(!response)
  {
    LOG_ERROR() << "Cannot call PullAll : " << GERROR_MESSAGE(gerror)<<std::endl;
    if (NULL != gerror && gerror->domain == G_DBUS_ERROR)
    {
      char* err = g_dbus_error_get_remote_error(gerror);
      LOG_ERROR() << "Exception: " << err<<std::endl;
      g_free(err);
      GERROR_FREE(gerror);
    }

    res = false;
  }
  if (res)
  {
    transferPathVariant = g_variant_get_child_value(response, 0);
    const gchar *transferPath = g_variant_get_string(transferPathVariant, NULL);
    transfer.init(connection, transferPath);
    transferStatus = eGetItemRetOk;
    LOG_FUNC();
    g_variant_unref(transferPathVariant);
    g_variant_unref(response);
  }

  delete[] fields;
  return res;
}

enum OpenAB_Source::Source::eGetItemRet PBAPSource::getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  static int counter = 0;
  std::string line;
  std::string vCard;
  while (fifoBuffer.tryPop(line))
  {
    //LOG_DEBUG() << line;
    vCard += line + "\n";
    if (0 == line.compare(0, 11, "BEGIN:VCARD"))
    {
      vCard.clear();
      vCard += line + "\n";
      counter++;
    }
    else if (0 == line.compare(0, 9, "END:VCARD"))
    {
      OpenAB::PIMContactItem *newContactItem = new OpenAB::PIMContactItem();
      if (newContactItem->parse(vCard))
      {
        item = newContactItem;
        return eGetItemRetOk;
      }
      else
      {
        delete newContactItem;
        return eGetItemRetError;
      }
    }
  }
  vCard.clear();
  return transferStatus;
}

enum OpenAB_Source::Source::eSuspendRet PBAPSource::suspend()
{
  unsigned int numRetries = 5;
  while(!transfer.isInitialized() && numRetries--)
  {
    LOG_DEBUG()<<"Waiting for transfer to suspend"<<std::endl;
    usleep(1000);
  }

  if(transfer.isInitialized())
  {
    if (transfer.suspend())
      return eSuspendRetOk;
  }
  LOG_DEBUG()<<"Suspend failed"<<std::endl;
  return eSuspendRetFail;
}

enum OpenAB_Source::Source::eResumeRet PBAPSource::resume()
{
  if(transfer.isInitialized())
  {
    if (transfer.resume())
      return eResumeRetOk;
  }

  return eResumeRetFail;
}

enum OpenAB_Source::Source::eCancelRet PBAPSource::cancel()
{
  unsigned int numRetries = 5;
  while(!transfer.isInitialized() && numRetries--)
  {
    usleep(1000);
  }

  if(transfer.isInitialized())
  {
    if (transfer.cancel())
      return eCancelRetOk;
  }

  return eCancelRetFail;
}

int PBAPSource::getTotalCount() const
{
  return phonebookSize;
}

void * PBAPSource::threadFuncFIFOWrapper(void* ptr)
{
  LOG_FUNC();
  PBAPSource * input = static_cast<PBAPSource*>(ptr);
  if (NULL == input)
  {
    return NULL;
  }
  input->threadFuncFIFO();
  return NULL;
}

void PBAPSource::threadFuncFIFO()
{
  unsigned int startOffset = 0;
  unsigned int chunkStart = 0;
  unsigned int chunkSize = phonebookSize;
  unsigned int chunksEnd = phonebookSize;
  unsigned int toDownload = phonebookSize;
  OpenAB::TimeStamp lastChunkDownloadStartTime;
  OpenAB::TimeStamp lastChunkDownloadStopTime;
  unsigned int lastChunkDownloadTime;

  if (batchDownloadDesiredTime > 0)
  {
    startOffset = rand()%phonebookSize;
    chunkStart = startOffset;
  }

  while (toDownload && transferStatus != eGetItemRetError)
  {
    if (batchDownloadDesiredTime > 0)
    {
      lastChunkDownloadTime = (lastChunkDownloadStopTime - lastChunkDownloadStartTime).toMs();
      if (lastChunkDownloadTime != 0)
      {
        LOG_DEBUG()<<"Chunk download time "<<lastChunkDownloadTime<<" ms"<<std::endl;
        chunkSize = (batchDownloadDesiredTime * chunkSize) / lastChunkDownloadTime;
        LOG_DEBUG()<<"Updating chunk size to "<<chunkSize<<std::endl;
      }
      else
      {
        chunkSize = 10;
      }

      if (chunkStart + chunkSize > chunksEnd)
      {
        chunkSize = chunksEnd - chunkStart;
      }
    }

    /**
     * create a FIFO (tempFIFO)
     */
    tempFIFO = tmpnam(NULL);
    LOG_DEBUG() << "Temporary FIFO: " << tempFIFO<<std::endl;

    if (0 != mkfifo( tempFIFO.c_str() , 0600 )){
      LOG_ERROR() << "Cannot create the FIFO: " << tempFIFO << " err:" << strerror(errno)<<std::endl;
      transferStatus = eGetItemRetError;
      fifoBuffer.close();
      disconnectSession();
      return;
    }

    //open FIFO temprary in non-blocking mode so that BlueZ can start writing to FIFO
    int fd = open(tempFIFO.c_str(), O_RDONLY | O_NONBLOCK);

    if (!pullPhonebook(chunkStart, chunkSize))
    {
      LOG_ERROR()<<"Cannot pull phonebook"<<std::endl;
      transferStatus = eGetItemRetError;
      fifoBuffer.close();
      disconnectSession();
      return;
    }
    lastChunkDownloadStartTime.setNow();

    //open FIFO using ifstream (ifstream is not supporting non-blocking mode)
    std::ifstream fifoStream(tempFIFO.c_str());
    close(fd);


    transfer.setCallback(transferStatusChanged, this);
    if (transfer.getStatus() == BluezOBEXTransfer::eStatusComplete)
    {
     transferStatus = eGetItemRetEnd;
     transfer.clean();
    }
    else if(transfer.getStatus() == BluezOBEXTransfer::eStatusError)
    {
      transferStatus = eGetItemRetError;
      transfer.clean();
    }

    std::string line;
    while(transferStatus != eGetItemRetEnd &&
          transferStatus != eGetItemRetError)
    {
      while(std::getline(fifoStream,line)){
        fifoBuffer.tryPush(line);
      }
    }

    while(std::getline(fifoStream,line)){
      fifoBuffer.tryPush(line);
    }
    toDownload -= chunkSize;
    chunkStart += chunkSize;
    if (chunkStart >= chunksEnd)
    {
      chunkStart = 0;
      chunksEnd = startOffset;
    }

    fifoStream.close();
    if (-1 != access(tempFIFO.c_str(), F_OK))
    {
      if (0 != unlink(tempFIFO.c_str()))
      {
        LOG_ERROR() << "Cannot remove the FIFO: " << tempFIFO << "err: " << strerror(errno)<<std::endl;
      }
    }
    lastChunkDownloadStopTime.setNow();
  }
  fifoBuffer.close();
  disconnectSession();
}

void PBAPSource::transferStatusChanged(BluezOBEXTransfer::Status status, void*userData)
{
  PBAPSource* input = static_cast<PBAPSource*>(userData);
  switch(status)
  {
    case BluezOBEXTransfer::eStatusComplete:
      input->transferStatus = eGetItemRetEnd;
      input->transfer.clean();
      break;
    case BluezOBEXTransfer::eStatusError:
      input->transferStatus = eGetItemRetError;
      input->transfer.clean();
      break;
    case BluezOBEXTransfer::eStatusQueued:
    case BluezOBEXTransfer::eStatusActive:
    case BluezOBEXTransfer::eStatusSuspended:
      break;
  }
}

FIFOBuffer::FIFOBuffer() :
    size(0),
    active(true)
{
  pthread_mutex_init(&mutex, NULL);
}

FIFOBuffer::~FIFOBuffer()
{
  pthread_mutex_destroy(&mutex);
}

void FIFOBuffer::tryPush(const std::string& s)
{
 /* if (!active)
  {
    return;
  }*/

  while (size > fifoBufferSize)
  {
    usleep(100000);
  }
  pthread_mutex_lock(&mutex);
  fifoBuffer.push(s);
  ++size;
  pthread_mutex_unlock(&mutex);
}

bool FIFOBuffer::tryPop(std::string& s)
{
  while (size <= 0)
  {
    if (!active)
    {
      return false;
    }
    usleep(100000);
  }
  pthread_mutex_lock(&mutex);
  --size;
  s = fifoBuffer.front();
  fifoBuffer.pop();
  pthread_mutex_unlock(&mutex);
  return true;
}

void FIFOBuffer::close()
{
  active = false;
};

namespace {
  class PBAPFactory : OpenAB_Source::Factory
  {
    public:
      /*!
       *  @brief Constructor.
       */
       PBAPFactory():Factory::Factory("PBAP"){
         LOG_FUNC();
       };

      /*!
       *  @brief Destructor, virtual by default.
       */
      virtual ~PBAPFactory(){
        LOG_FUNC();
      };

      OpenAB_Source::Source * newIstance(const OpenAB_Source::Parameters & params)
      {
        LOG_FUNC()<<params.toJSON()<<std::endl;
        std::string mac;
        std::string pbLoc = "int";
        std::string ignoreFields = "";
        unsigned int batchDownloadTime = 0;
        PBAPSource * src = NULL;
        OpenAB::Variant param;

        param = params.getValue("MAC");
        if (param.invalid()){
          LOG_ERROR() << "Parameter 'MAC' not found"<<std::endl;
          return NULL;
        }
        mac = param.getString();

        param = params.getValue("loc");
        if (!param.invalid()){
          pbLoc = param.getString();
        }

        param = params.getValue("ignore_fields");
        if (!param.invalid()){
          ignoreFields = param.getString();
        }

        param = params.getValue("batch_download_time");
        if (!param.invalid())
        {
          if (param.getType() == OpenAB::Variant::INTEGER)
          {
            batchDownloadTime = param.getInt();
            LOG_DEBUG()<<"BatchDownloadTime "<<batchDownloadTime;
          }
          else
          {
            LOG_ERROR() << "Parameter 'batch_download_time' expected to be INTEGER type"<<std::endl;
          }
        }
        LOG_DEBUG()<<"BatchDownloadTime "<<batchDownloadTime;

        src = new PBAPSource(mac, pbLoc, ignoreFields, batchDownloadTime);
        if (NULL == src)
        {
          LOG_ERROR() << "Cannot Initialize PBAPInput"<<std::endl;
          return NULL;
        }

        return src;
      }
  };
}

REGISTER_PLUGIN_FACTORY(PBAPFactory);
