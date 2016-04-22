/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file BluezOBEXTransfer.cpp
 */

#include "BluezOBEXTransfer.hpp"
#include <helpers/Log.hpp>
#include <glib2/OpenAB_glib2_global.h>
#include <cstdio>
#include <cstring>

BluezOBEXTransfer::BluezOBEXTransfer() :
  transfer1Proxy(NULL),
  connection(NULL),
  mainLoop(NULL),
  threadCreated(false),
  path(""),
  status(eStatusQueued),
  initResult(eInitResultNotInitialized),
  callback(NULL),
  callbackUserData(NULL)
{

}

void * BluezOBEXTransfer::threadFuncTransfer(void* ptr){
  LOG_FUNC();
  GError* gerror = NULL;
  BluezOBEXTransfer * transfer = static_cast<BluezOBEXTransfer*>(ptr);

  transfer->transfer1Proxy = g_dbus_proxy_new_sync(transfer->connection,
                                                    G_DBUS_PROXY_FLAGS_NONE,
                                                    NULL,
                                                    "org.bluez.obex",
                                                    transfer->path.c_str(),
                                                    "org.bluez.obex.Transfer1",
                                                    NULL, &gerror);

  if (NULL != gerror)
  {
    LOG_ERROR() << "Cannot create  org.bluez.obex.Transfer1 proxy : " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    transfer->initResult = eInitResultFail;
    return NULL;
  }

  transfer->mainLoop = g_main_loop_new (NULL, FALSE);
  g_signal_connect(transfer->transfer1Proxy,
                   "g-properties-changed",
                   G_CALLBACK(propertiesChangedHandler),
                   transfer);

  transfer->initResult = eInitResultOK;
  g_main_loop_run (transfer->mainLoop);

  g_object_unref (transfer->transfer1Proxy);

  g_main_loop_unref (transfer->mainLoop);
  transfer->mainLoop = NULL;

  return NULL;
}

bool BluezOBEXTransfer::init(GDBusConnection* conn, const std::string& objectPath)
{
  clean();
  path = objectPath;
  connection = conn;

  initResult = eInitResultNotInitialized;
  pthread_create(&eventsThread,NULL,threadFuncTransfer,this);
  threadCreated = true;

  while (initResult == eInitResultNotInitialized)
  {
    usleep(100000);
  }
  if(initResult == eInitResultFail)
  {
    return false;
  }

  return true;
}

void BluezOBEXTransfer::clean()
{
  if (mainLoop)
  {
    g_main_loop_quit(mainLoop);
  }

  if (threadCreated)
  {
   pthread_join(eventsThread, NULL);
  }

  path = "";
  connection = NULL;
  initResult = eInitResultNotInitialized;
  callback = NULL;
  callbackUserData = NULL;
  status = eStatusQueued;
}

bool BluezOBEXTransfer::isInitialized()
{
  return (initResult == eInitResultOK);
}

BluezOBEXTransfer::~BluezOBEXTransfer()
{
  clean();
}

bool BluezOBEXTransfer::cancel()
{
  LOG_FUNC() << "Canceling transfer " << path<<std::endl;

  GError *gerror = NULL;

  if(!g_dbus_proxy_call_sync(transfer1Proxy,
                             "Cancel",
                             NULL,
                             G_DBUS_CALL_FLAGS_NONE, -1,
                             NULL, &gerror))
  {
    LOG_ERROR() << "Cannot cancel transfer " << path <<": " << GERROR_MESSAGE(gerror)<<std::endl;
    if(NULL != gerror &&
       gerror->domain == G_DBUS_ERROR)
    {
      char* err = g_dbus_error_get_remote_error (gerror);
      LOG_ERROR() << "Exception: " << err<<std::endl;
      g_free(err);
    }
    GERROR_FREE(gerror);
    return false;
  }
  status = eStatusError;

  if (callback)
  {
    callback(status, callbackUserData);
  }

  return true;
}

bool BluezOBEXTransfer::suspend()
{
  LOG_FUNC() << "Suspending transfer " << path<<std::endl;

  GError *gerror = NULL;

  if(!g_dbus_proxy_call_sync(transfer1Proxy,
                             "Suspend",
                             NULL,
                             G_DBUS_CALL_FLAGS_NONE, -1,
                             NULL, &gerror))
  {
    LOG_ERROR() << "Cannot suspend transfer " << path <<": " << GERROR_MESSAGE(gerror)<<std::endl;
    if(NULL != gerror &&
       gerror->domain == G_DBUS_ERROR)
    {
      char* err = g_dbus_error_get_remote_error(gerror);
      LOG_ERROR() << "Exception: " << err<<std::endl;
      g_free(err);
    }
    GERROR_FREE(gerror);
    return false;
  }

  return true;
}

bool BluezOBEXTransfer::resume()
{
  LOG_FUNC() << "Resuming transfer " << path<<std::endl;

  GError *gerror = NULL;

  if(!g_dbus_proxy_call_sync(transfer1Proxy,
                             "Resume",
                             NULL,
                             G_DBUS_CALL_FLAGS_NONE, -1,
                             NULL, &gerror))
  {
    LOG_ERROR() << "Cannot resume transfer " << path <<": " << GERROR_MESSAGE(gerror)<<std::endl;
    if(NULL != gerror &&
       gerror->domain == G_DBUS_ERROR)
    {
      char* err = g_dbus_error_get_remote_error(gerror);
      LOG_ERROR() << "Exception: " << err<<std::endl;
      g_free(err);
    }
    GERROR_FREE(gerror);
    return false;
  }

  return true;
}

BluezOBEXTransfer::Status BluezOBEXTransfer::getStatus()
{
  return status;
}

void BluezOBEXTransfer::setCallback(StatusChangeCallback cb, void* userData)
{
  callback = cb;
  callbackUserData = userData;
}

void BluezOBEXTransfer::propertiesChangedHandler(GDBusProxy *proxy,
                                                 GVariant *changed_properties,
                                                 GStrv invalidated_properties,
                                                 gpointer user_data)
{
  static_cast<void>(proxy);
  static_cast<void>(invalidated_properties);

  LOG_FUNC();
  BluezOBEXTransfer * transfer = static_cast<BluezOBEXTransfer*>(user_data);

  if (transfer->status == eStatusComplete ||
      transfer->status == eStatusError)
  {
    return;
  }

  GVariant* status = NULL;
  LOG_DEBUG()<<"Checking status"<<std::endl;
  status = g_variant_lookup_value(changed_properties, "Status", G_VARIANT_TYPE_STRING);
  if(NULL != status)
  {
    const char* st = g_variant_get_string(status, NULL);
    LOG_DEBUG()<<"Transfer status " << st<<std::endl;

    if(0 == strcmp(st, "queued"))
    {
      transfer->status = eStatusQueued;
    }
    else if(0 == strcmp(st, "active"))
    {
      transfer->status = eStatusActive;
    }
    else if(0 == strcmp(st, "suspended"))
    {
      transfer->status = eStatusSuspended;
    }
    else if(0 == strcmp(st, "complete"))
    {
      transfer->status = eStatusComplete;
    }
    else if(0 == strcmp(st, "error"))
    {
      transfer->status = eStatusError;
    }

    if (transfer->callback)
    {
      transfer->callback(transfer->status, transfer->callbackUserData);
    }

    g_variant_unref (status);
  }
}
