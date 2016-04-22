/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file File.cpp
 */

#include "plugins/file/File.hpp"
#include <PIMItem/Contact/PIMContactItem.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

FileSource::FileSource(const std::string& f)
    : OpenAB_Source::Source(OpenAB::eContact),
      path(f),
      totalNumberOfVCards(0)
{
  LOG_FUNC();
}

FileSource::~FileSource()
{
  LOG_FUNC();
  if (infile.is_open())
  {
    infile.close();
  }
}

enum OpenAB_Source::Source::eInit FileSource::init()
{
  LOG_FUNC() << "Checking whether "<< path <<" is file or directory"<<std::endl;
  struct stat fileStat;
  if (0 != stat(path.c_str(), &fileStat))
  {
    LOG_ERROR()<<"Cannot open "<<path<<": "<<strerror(errno)<<std::endl;
    return eInitFail;
  }

  if (S_ISDIR(fileStat.st_mode))
  {
    LOG_DEBUG() << path <<" is a directory"<<std::endl;
    DIR* dir;
    struct dirent* dirEntry;
    dir = opendir(path.c_str());
    if (dir == NULL)
    {
       LOG_ERROR()<<"Cannot open directory "<< path << std::endl;
       return eInitFail;

    }
    while ((dirEntry = readdir(dir)) != NULL)
    {
      std::string entryFullPath = path + "/" + dirEntry->d_name;
      lstat(entryFullPath.c_str(), &fileStat);

      if (S_ISDIR(fileStat.st_mode))
        continue;

      LOG_DEBUG() << "directory entry: " <<entryFullPath<<std::endl;

      filenames.push_back(entryFullPath);
    }
    closedir(dir);
  }
  else
  {
    LOG_DEBUG() << path <<" is a file"<<std::endl;
    filenames.push_back(path);
  }

  LOG_DEBUG() << "In total "<< (int)filenames.size() <<" files will be processed"<<std::endl;
  LOG_DEBUG() << "Checking number of vCards in files"<<std::endl;

  std::vector<std::string>::iterator it;
  for(it = filenames.begin(); it != filenames.end();)
  {
    LOG_DEBUG() << "Checking "<< (*it) <<std::endl;
    std::string line;
    bool anyVCard = false;

    infile.open((*it).c_str(), std::ifstream::in);
    if(!infile.is_open())
    {
      LOG_ERROR() << "Cannot open file "<< (*it) <<std::endl;
      it = filenames.erase(it);
      continue;
    }
    //go through all files and count number of lines containing END:VCARD
    //it may be not 100% correct but will be quicker
    while (std::getline(infile, line))
    {
      if (0 == line.compare(0, 9, "END:VCARD"))
      {
        totalNumberOfVCards++;
        anyVCard = true;
      }
    }

    infile.close();

    //if file does not contain any vCard exclude it from list
    if(!anyVCard)
    {
      it = filenames.erase(it);
    }
    else
    {
      ++it;
    }
  }
  if (filenames.empty())
  {
    LOG_ERROR()<<"No vcards were found"<<std::endl;
    currentFile = filenames.end();
    return eInitFail;
  }
  else
  {
    currentFile = filenames.begin();
    infile.open((*currentFile).c_str(), std::ifstream::in);
  }

  return eInitOk;
}

enum OpenAB_Source::Source::eGetItemRet FileSource::getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  LOG_FUNC();
  std::string line;
  std::string vCard;
  std::vector<std::string>::iterator it;

  if (currentFile == filenames.end())
      return eGetItemRetEnd;

  while (std::getline(infile, line))
  {
    // LOG_DEBUG() << line;
    vCard+=line + "\n";
    if (0 == line.compare(0, 11, "BEGIN:VCARD"))
    {
      vCard.clear();
      vCard+=line + "\n";
    }
    else if (0 == line.compare(0, 9, "END:VCARD")){
      OpenAB::PIMContactItem * newContactItem = new OpenAB::PIMContactItem();
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
  infile.close();
  vCard.clear();
  ++currentFile;

  if (currentFile == filenames.end())
  {
    return eGetItemRetEnd;
  }

  infile.open((*currentFile).c_str(), std::ifstream::in);
  if (!infile.is_open())
  {
    LOG_DEBUG()<<"Cannot open file "<<(*currentFile).c_str()<<std::endl;
    ++currentFile;
    return eGetItemRetError;
  }
  return getItem(item);
}

enum OpenAB_Source::Source::eSuspendRet FileSource::suspend()
{
  return eSuspendRetNotSupported;
}

enum OpenAB_Source::Source::eResumeRet FileSource::resume()
{
  return eResumeRetNotSupported;
}

enum OpenAB_Source::Source::eCancelRet FileSource::cancel()
{
  return eCancelRetNotSupported;
}

int FileSource::getTotalCount() const
{
  return totalNumberOfVCards;
}

namespace {

class FileFactory : OpenAB_Source::Factory
{
  public:
    FileFactory()
        : OpenAB_Source::Factory::Factory("File")
    {
      LOG_FUNC();
    }
    ;
    virtual ~FileFactory()
    {
      LOG_FUNC();
    }
    ;

    OpenAB_Source::Source * newIstance(const OpenAB_Source::Parameters & params)
    {
      LOG_FUNC();
      OpenAB::Variant param = params.getValue("filename");
      if (param.invalid()){
        LOG_ERROR() << "Parameter 'filename' not found";
        return NULL;
      }

      FileSource * fi =new FileSource(param.getString());
      if (NULL == fi)
      {
        LOG_ERROR() << "Cannot Initialize FileInput";
        return NULL;
      }

      return fi;
    }
};
}

REGISTER_PLUGIN_FACTORY(FileFactory);
