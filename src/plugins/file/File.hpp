/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file File.hpp
 */

#ifndef FILE_HPP_
#define FILE_HPP_

#include <plugin/source/Source.hpp>
#include <fstream>
#include <iostream>

/**
 * @defgroup FileSource File Source Plugin
 * @ingroup SourcePlugin
 *
 * @brief Provides OpenAB::PIMContactItem items from vCard files.
 * Supports both folders of vCards and files containing multiple vCards.
 *
 * Plugin Name: "File"
 *
 * Parameters:
 * | Type          | Name |  Description                 | Mandatory |
 * |:--------------|:     |: ----------------------------|:           |
 * | String | "filename"     | Path to the file or directory containing vCards | Yes |
 * | Bool   | "count_vcards" | Any value, if provided number of vCards will be counted, so OpenAB_Sync::Sync progress will be available, but it make OpenAB_Source::Source initialization process slower| No |
 *
 * @todo implement support for count_vcards parameter
 */

/*!
 * @brief Documentation for class File
 */
class FileSource : public OpenAB_Source::Source
{
  public:
    /*!
     *  @brief Constructor.
     */
    FileSource(const std::string& f);

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~FileSource();

    enum OpenAB_Source::Source::eInit init();

    enum OpenAB_Source::Source::eGetItemRet getItem(OpenAB::SmartPtr<OpenAB::PIMItem> &item);

    enum OpenAB_Source::Source::eSuspendRet suspend();

    enum OpenAB_Source::Source::eResumeRet resume();

    enum OpenAB_Source::Source::eCancelRet cancel();

    int getTotalCount() const;

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    FileSource(FileSource const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    FileSource& operator=(FileSource const &other);

    std::string path;
    std::ifstream infile;
    std::vector<std::string> filenames;
    int totalNumberOfVCards;
    std::vector<std::string>::iterator currentFile;
};

#endif // FILE_HPP_
