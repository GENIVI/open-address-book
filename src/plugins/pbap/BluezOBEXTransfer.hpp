/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file BluezOBEXTransfer.hpp
 */

#ifndef BLUEZOBEXTRANSFER_H_
#define BLUEZOBEXTRANSFER_H_
#include <gio/gio.h>
#include <string>

/*!
 * @brief Documentation for class BluezOBEXTransfer
 */
class BluezOBEXTransfer
{
  public:
    enum Status {
      eStatusQueued = 0,
      eStatusActive,
      eStatusSuspended,
      eStatusComplete,
      eStatusError
    };

    /*!
     *  @brief Constructor.
     */
    BluezOBEXTransfer();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~BluezOBEXTransfer();

    bool init(GDBusConnection* connection, const std::string& objectPath);

    bool isInitialized();

    void clean();

    bool cancel();

    bool suspend();

    bool resume();

    Status getStatus();

    typedef void (*StatusChangeCallback) (Status status, void* userData);

    void setCallback(StatusChangeCallback cb, void* userData);

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    BluezOBEXTransfer(BluezOBEXTransfer const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    BluezOBEXTransfer& operator=(BluezOBEXTransfer const &other);

    enum InitResult
    {
      eInitResultNotInitialized = 0,
      eInitResultOK,
      eInitResultFail
    };
    static void propertiesChangedHandler(GDBusProxy *proxy,
                                         GVariant *changed_properties,
                                         GStrv invalidated_properties,
                                         gpointer user_data);

    static void* threadFuncTransfer(void* ptr);

    GDBusProxy*   transfer1Proxy;
    GDBusConnection* connection;
    GMainLoop*    mainLoop;
    pthread_t     eventsThread;
    bool          threadCreated;
    std::string   path;
    Status        status;
    InitResult    initResult;
    StatusChangeCallback callback;
    void*         callbackUserData;
};
#endif // BLUEZOBEXTRANSFER_H_
