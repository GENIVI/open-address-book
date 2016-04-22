/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file test_plugin.cpp
 */

#include <plugin/Plugin.hpp>
#include <plugin/source/Source.hpp>
#include <string>

namespace OpenAB_TESTS {

class TestSourcePlugin : public OpenAB_Source::Source
{
  public:

    TestSourcePlugin() : OpenAB_Source::Source(OpenAB::eContact){}
    virtual ~TestSourcePlugin(){}
    OpenAB_Source::Source::eInit init(){return OpenAB_Source::Source::eInitOk;}
    OpenAB_Source::Source::eSuspendRet suspend() {return OpenAB_Source::Source::eSuspendRetNotSupported;}
    OpenAB_Source::Source::eResumeRet resume() {return OpenAB_Source::Source::eResumeRetNotSupported;}
    OpenAB_Source::Source::eCancelRet cancel() {return OpenAB_Source::Source::eCancelRetNotSupported;}
    OpenAB_Source::Source::eGetItemRet getItem(OpenAB::SmartPtr<OpenAB::PIMItem> &item) {
      return OpenAB_Source::Source::eGetItemRetError;
    }
    int getTotalCount() const {
      return 0;
    }
};


class TestSourcePluginFactory : OpenAB_Source::Factory
{
  public:
    TestSourcePluginFactory() : OpenAB_Source::Factory("TestSourcePlugin"){};

    virtual ~TestSourcePluginFactory(){}

    OpenAB_Source::Source * newIstance(const OpenAB_Source::Parameters & params)
    {
      return new TestSourcePlugin();
    };
} factory1;

} // namespace OpenAB
