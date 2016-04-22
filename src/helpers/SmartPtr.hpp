/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file SmartPtr.hpp
 */

#ifndef SMARTPTR_HPP_
#define SMARTPTR_HPP_

#include <helpers/Log.hpp>

/*!
 * @brief namespace OpenAB
 */
namespace OpenAB {
/**
 * @defgroup  libOpenAB  OpenAB Library
 *
 */

/**
 * @brief Smart pointer implementation for safely passing around dynamically created data.
 * Smart pointers are reference counted, data that they are managing is automatically
 * freed when no other references to it exist.
 * @note it is not advised to operate directly on pointers managed by SmartPtr.
 * @todo check if other comparison operators also need to be overloaded
 */
template<typename __C>
class SmartPtr
{
  public:
    /**
     * @brief Default constructor
     */
    SmartPtr()
        : ptr(0),
          refCount(0)
    {
      refCount = new unsigned int;
      *refCount = 1;
    }

    /**
     * @brief Constructor.
     * Creates new SmartPtr from given pointer
     * @param [in] p pointer to be managed by new SmartPtr
     */
    SmartPtr(__C* p) :
    ptr(p),
    refCount(0)
    {
      refCount = new unsigned int;
      *refCount = 1;
    }

    /**
     * @brief Copy constructor.
     * Creates new SmartPtr from other instance of SmartPtr increasing its reference count.
     * @param [in] other instance of SmartPtr to be copied.
     */
    SmartPtr(const SmartPtr& other) :
    ptr(other.ptr),
    refCount(other.refCount)
    {
      (*refCount)++;
    }

    /**
     * @brief Destructor.
     * Decrements reference count, if it drops to 0 frees managed data.
     */
    ~SmartPtr()
    {
      (*refCount)--;
      if(*refCount == 0)
      {
        delete refCount;
        if (ptr)
          delete ptr;
      }
    }

    /**
     * @brief Assignment operator
     * Creates new SmartPtr from other instance of SmartPtr increasing its reference count.
     * Decreases reference count of previous data, and it is needed frees it.
     */
    SmartPtr& operator=(const SmartPtr& other)
    {
      if (this != &other)
      {
        (*refCount)--;
        if(*refCount == 0)
        {
          delete refCount;
          if (ptr)
            delete ptr;
        }

        ptr = other.ptr;
        refCount = other.refCount;
        (*refCount)++;
      }
      return *this;
    }

    bool operator==(const SmartPtr& other) const
    {
      if (NULL == ptr || NULL == other.ptr)
      {
        if (NULL == ptr && NULL == other.ptr)
        {
          return true;
        }
        return false;
      }
      return ((*ptr) == (*other.ptr));
    }

    bool operator!=(const SmartPtr& other) const
    {
      if (NULL == ptr || NULL == other.ptr)
      {
        if (NULL == ptr && NULL == other.ptr)
        {
          return false;
        }
        return true;
      }
      return ((*ptr) != (*other.ptr));
    }

    bool operator<(const SmartPtr& other) const
    {
      if (NULL == ptr || NULL == other.ptr)
      {
        if (NULL == ptr && NULL == other.ptr)
        {
          return false;
        }
        if(!ptr)
        {
          return true;
        }
        else
        {
          return false;
        }
      }
      return ((*ptr) < (*other.ptr));
    }

    __C* operator->() const {return ptr;}
    __C& operator*() const {return *ptr;}
    __C* getPointer() const {return ptr;}

    operator __C*() const {return ptr;}
    operator const __C*() const {return (const __C*) ptr;}
   // operator __C() {return *ptr;}

  private:
    __C* ptr;
    unsigned int* refCount;
};

} // namespace OpenAB

#endif // SMARTPTR_HPP_
