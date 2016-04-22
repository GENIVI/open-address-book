/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Plugin.hpp
 */

#ifndef PLUGIN_TEMPLATE_HPP_
#define PLUGIN_TEMPLATE_HPP_

#include <map>
#include <typeinfo>
#include <helpers/Log.hpp>

namespace OpenAB_Plugin{

template<typename __C>
class Node
{
  public:
    Node(__C*p,std::string& name):p(p),next(NULL),name(name){};
    Node(__C*p,std::string& name,Node*n):p(p),next(n),name(name){};
    __C* getElement(){return p;};
    Node*getNext(){return next;};
    void setNext(Node*n){next=n;};

  private:
    __C*        p;
    Node*       next;
    std::string name;
};

template<typename __C>
class NodeIterator
{
  public:
    NodeIterator(Node<__C>*c):current(c){};

    NodeIterator& operator++()
    {
      if (NULL != current){
        current = current->getNext();
      }
      return *this;
    };
    bool operator!=(NodeIterator<__C> it){ return current != it.current; };
    __C* operator->(){return current->getElement();};

  private:
    Node<__C>*current;
};

template<typename __C>
class LinkedList
{
  public:
    typedef Node<__C> typeNode;
    typedef NodeIterator<__C> iterator;

    LinkedList()
    {};
    ~LinkedList()
    {};
    __C* operator[](std::string name)
    {
      typeNode * node = findNode(name);
      if (NULL == node)
      return NULL;
      else
      return node->getElement();
    };

    iterator begin()
    {
      return iterator(first);
    };
    iterator end()
    {
      return iterator(NULL);
    };

    void push_front(__C*elem,std::string& name){
      typeNode * node = new typeNode(elem,name,first);
      first = node;
    };

    typeNode * findNode(std::string name)
    {
      typeNode * l = first;
      while(NULL != l){
        if (0 == l->getElement()->getName().compare(name))
        {
          return l;
        }
        l = l->getNext();
      }
      return NULL;
    };

    void eraseNode(std::string name)
    {
      LOG_FUNC() << " Erase: " << " \"" << name << "\""<<std::endl;
      typeNode * l = first;
      typeNode * prev = first;
      while(NULL != l){
        if (0 == l->getElement()->getName().compare(name))
        {
          if (l==prev){
            first = l->getNext();
          }else{
            prev->setNext(l->getNext());
          }
          delete l;
          return;
        }
        if (l!=prev)
          prev=l;
        l = l->getNext();
      }
      return;
    };
  private:
    typeNode* first;
};

/**
 * @defgroup  pluginGroup  Plugin Interface
 */

template<typename __C,typename __P>
class Factory
{
  public:

    /*!
     *  @brief List of the specialized Factories
     */
    typedef LinkedList<Factory< __C , __P > > typeList;

    /*!
     *  @brief Constructor.
     */
    Factory(std::string name):name(name){
      LOG_FUNC() << " Adding new " << typeid(__C).name() << " \"" << name << "\""<<std::endl;
      if (NULL == factories[name])
      {
        factories.push_front(this,name);
        if (NULL != loadedCb)
        {
          loadedCb(name);
        }
      }
    };
    /*!
     *  @brief Destructor, virtual by default.
     */
    ~Factory(){factories.eraseNode(name);};

    virtual __C * newIstance(const __P &) = 0;

    std::string getName(){return name;};

    typedef void (*PluginLoadedCallback) (const std::string& pluginName);
  private:

    std::string name;
    static PluginLoadedCallback loadedCb;

  public:
    static typeList factories;
    static void setPluginLoadedCallback(PluginLoadedCallback callback) {loadedCb = callback;}
};

class Parameters
{
  public:
    Parameters(){};
    virtual ~Parameters(){};

    virtual bool fromJSON(const std::string& json) = 0;
    virtual std::string toJSON() const = 0;
};

/**
 * @brief Exports plugin interface. Made new plugin interface available to be used in OpenAB.
 * This should be called at the end of *.cpp file of new plugin interface.
 * @param [in] PlugyoinNamespace namespace in which plugin interface is defined.
 * @param [in] PluginInterface class name defining interface of new plugin.
 * @param [in] PluginParameters class name defining parameters for new plugin itnerface.
 */
#define EXPORT_PLUGIN_INTERFACE(PluginNamespace, PluginInterface, PluginParameters) \
  namespace PluginNamespace {                                       \
  Factory::typeList factories;                                      \
  template<>                                                        \
  Factory::PluginLoadedCallback Factory::loadedCb = NULL;           \
                                                                    \
  template<>                                                        \
  Factory::typeList Factory::factories = PluginNamespace::factories;\
  }

/**
 * @brief Declares new plugin interface. Should be called at the end of *.hpp file with
 *  new plugin interface.
 * @param [in] PluginNamespace namespace in which plugin interface is defined.
 * @param [in] PluginInterface class name defining interface of new plugin.
 * @param [in] PluginParameters class name defining parameters for new plugin itnerface.
 */
#define DECLARE_PLUGIN_INTERFACE(PluginNamespace, PluginInterface, PluginParameters) \
  namespace PluginNamespace {                                                 \
  typedef OpenAB_Plugin::Factory< PluginInterface , PluginParameters > Factory;  \
  }

/**
 * @brief Registers new plugin factory. New plugin interface implementations can register its
 * factories so they can be used in OpenAB. This should be called at the end of *.cpp file of plugin.
 * @note Factory for new plugin implementation should be defined in unnamed namespace.
 * @param [in] PluginFactory factory class of new plugin implementation.
 */
#define REGISTER_PLUGIN_FACTORY(PluginFactory) \
  namespace {                                  \
    PluginFactory factory;                     \
  }

} // namespace OpenAB_Plugin

#endif // PLUGIN_TEMPLATE_HPP_
