/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PBAP.hpp
 */

#ifndef PBAP_H_
#define PBAP_H_

#include <plugin/source/Source.hpp>

#include <glib2/OpenAB_glib2_global.h>
#include <gio/gio.h>

#include <iostream>
#include <fstream>

#include <queue>

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include "BluezOBEXTransfer.hpp"

class FIFOBuffer
{
  public:
    FIFOBuffer();
    ~FIFOBuffer();

    void tryPush(const std::string& s);
    bool tryPop(std::string& s);
    void close();

  private:
    static const unsigned int fifoBufferSize = 5000000;
    unsigned int size;
    bool active;
    std::queue<std::string> fifoBuffer;
    pthread_mutex_t mutex;
};

/**
 * @defgroup PBAPSource PBAP Source Plugin
 * @ingroup SourcePlugin
 *
 * @brief Provides OpenAB::PIMContactItem items using PBAP protocol.
 *
 * Plugin Name: "PBAP"
 *
 * Parameters:
 * | Type       | Description                                              | Mandatory |
 * |:-----------|:---------------------------------------------------------| :         |
 * | "MAC"      | Bluetooth MAC Address of the device                      | Yes       |
 * | "loc"      | Location of the addressbook (default = "int")            | No        |
 * | "ignore_fields" | Comma separated list of vCard fields to be not downloaded    | No |
 * | "batch_download_time" | Desired time of single PBAP batch download (when set to 0 - default - all contacts are downloaded in single PBAP transfer)| No |
 *
 * @todo expose some more parameters for batch download like default size of batches with/without photos
 *
 * **Loc** possible values:
 * | Type     | Description                       |
 * |:---------|:----------------------------------|
 * | "int"    | internal (default if unspecified) |
 * | "sim"    | (sim1)                            |
 * | "sim2"   | (sim2)                            |
 * |  ...     |                                   |
 */
class PBAPSource : public OpenAB_Source::Source
{
  public:
    /*!
     *  @brief Constructor.
     */
    PBAPSource(const std::string& mac, const std::string& pb,
               const std::string& filter, unsigned int batchDownloadTime);

    virtual ~PBAPSource();

    enum OpenAB_Source::Source::eInit init();

    enum OpenAB_Source::Source::eGetItemRet getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item);

    enum OpenAB_Source::Source::eSuspendRet suspend();

    enum OpenAB_Source::Source::eResumeRet resume();

    enum OpenAB_Source::Source::eCancelRet cancel();

    int getTotalCount() const;

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    PBAPSource(PBAPSource const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    PBAPSource& operator=(PBAPSource const &other);

    void cleanup();

    static void* threadFuncFIFOWrapper(void* ptr);
    void threadFuncFIFO();
    static void transferStatusChanged(BluezOBEXTransfer::Status status, void*userData);

    bool createSession();
    bool selectPhonebook();
    bool getPhonebookSize();
    bool getSupportedFilters();
    bool pullPhonebook(unsigned int offset, unsigned int count);
    bool disconnectSession();


    std::string macAddress;
    std::string pbLocation;
    unsigned int batchDownloadDesiredTime;


    std::vector<std::string> ignoredFields;
    std::vector<std::string> filters;
    std::vector<std::string> supportedFilters;

    std::string tempFIFO;
    FIFOBuffer  fifoBuffer;
    pthread_t   threadFIFO;
    bool        threadCreated;

    GDBusConnection*    connection;
    GDBusProxy*         proxyClient1;
    GDBusProxy*         proxyPhonebookAccess1;
    GDBusProxy*         proxyTransfer1;
    BluezOBEXTransfer   transfer;
    OpenAB_Source::Source::eGetItemRet transferStatus;

    std::string         sessionPath;
    unsigned int        phonebookSize;
};

#endif // PBAP_H_
