/* This class is property of Troll Tech. It's here temporarily
   until it appears in Qt's CVS 
*/

#ifndef KSharedPTR_H
#define KSharedPTR_H

#include <qshared.h>

/**
 * Can be used to control the lifetime of an object
 * that has derived @ref QShared. As long a someone holds
 * a KSharedPtr on some QShared object it wont become
 * deleted but is deleted once its refrence count is 0.
 * This struct emulates C++ pointers perfectly. So just use
 * it like a simple C++ pointer.
 */
template< class T >
struct KSharedPtr
{
public:
  KSharedPtr() { ptr = 0; }
  KSharedPtr( T* t ) { ptr = t; ptr->ref(); }
  KSharedPtr( const KSharedPtr& p ) { ptr = p.ptr; ptr->ref(); }
  ~KSharedPtr() { if ( ptr->deref() ) delete ptr; }

  KSharedPtr<T>& operator= ( const KSharedPtr<T>& p ) {
    if ( ptr && ptr->deref() ) delete ptr;
    ptr = p.ptr; ptr->ref();
    return *this;
  }
  KSharedPtr<T>& operator= ( T* p ) { 
    if ( ptr && ptr->deref() ) delete ptr;
    ptr = p; ptr->ref();
    return *this;
  }
  bool operator== ( const KSharedPtr<T>& p ) const { return ( ptr == p.ptr ); }
  bool operator!= ( const KSharedPtr<T>& p ) const { return ( ptr != p.ptr ); }
  bool operator== ( const T* p ) const { return ( ptr == p ); }
  bool operator!= ( const T* p ) const { return ( ptr != p ); }
  bool operator!() const { return ( ptr == 0 ); }
  operator bool() const { return ( ptr != 0 ); }
  operator T*() { return ptr; }
  operator const T*() const { return ptr; }

  const T& operator*() const { return *ptr; }
  T& operator*() { return *ptr; }
  const T* operator->() const { return ptr; }
  T* operator->() { return ptr; }

private:
  T* ptr;
};

#endif
