/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file unimplemented_plugin.cpp
 */

#include <plugin/Plugin.hpp>
#include <plugin/source/Source.hpp>
#include <string>

namespace OpenAB_TESTS {

class UnimplementedPlugin : public OpenAB_Source::Source
{
  public:

    UnimplementedPlugin() : OpenAB_Source::Source(OpenAB::eContact){}
    virtual ~UnimplementedPlugin(){}
    OpenAB_Source::Source::eInit init();
    OpenAB_Source::Source::eSuspendRet suspend();
    OpenAB_Source::Source::eResumeRet resume();
    OpenAB_Source::Source::eCancelRet cancel();
    OpenAB_Source::Source::eGetItemRet getItem(OpenAB::SmartPtr<OpenAB::PIMItem> &item);
    int getTotalCount() const;
};


class TestSourcePluginFactory : OpenAB_Source::Factory
{
  public:
    TestSourcePluginFactory() : OpenAB_Source::Factory("UnimplementedPlugin"){};

    virtual ~TestSourcePluginFactory(){}

    OpenAB_Source::Source * newIstance(const OpenAB_Source::Parameters & params)
    {
      return new UnimplementedPlugin();
    };
} factory1;

} // namespace OpenAB
