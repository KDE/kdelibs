/* The QSharedPtr class is property of Troll Tech. It's here temporarily
   until it appears in Qt's CVS 
*/
/*
   The KShared / KSharedPtr class is Copyright 1999 W. Bastian
*/

#ifndef KSharedPTR_H
#define KSharedPTR_H

#include <qshared.h>

/**
 * Reference counting for shared objects.
 * @author Waldo Bastian <bastian@kde.org>
 * @version $Id$
 */
class KShared {
public:      
   KShared() : count(0) { }      
   KShared( const KShared & ) : count(0) { }      
   KShared &operator=(const KShared & ) { return *this; }      
   void _KShared_ref() { count++; }      
   void _KShared_unref() { if (!--count) delete this; }      
   int _KShared_count() { return count; }
protected:            
   virtual ~KShared() { }      
   int count;
}; 

/**
 * Can be used to control the lifetime of an object
 * that has derived @ref KShared. As long a someone holds
 * a KSharedPtr on some KShared object it won't become
 * deleted but is deleted once its reference count is 0.
 * This struct emulates C++ pointers perfectly. So just use
 * it like a simple C++ pointer.
 *
 * KShared and KSharedPtr are preferred over QShared / 
 * QSharedPtr since they are more safe.
 * @author Waldo Bastian <bastian@kde.org>
 * @version $Id$
 */


template< class T >
struct KSharedPtr
{
public:
  KSharedPtr() 
    : ptr(0) { }
  KSharedPtr( T* t ) 
    : ptr(t) { if ( ptr ) ptr->_KShared_ref(); }
  KSharedPtr( const KSharedPtr& p ) 
    : ptr(p.ptr) { if ( ptr ) ptr->_KShared_ref(); }
    
  ~KSharedPtr() { if ( ptr ) ptr->_KShared_unref(); }

  KSharedPtr<T>& operator= ( const KSharedPtr<T>& p ) {
    if ( ptr == p.ptr ) return *this;
    if ( ptr ) ptr->_KShared_unref();
    ptr = p.ptr; 
    if ( ptr ) ptr->_KShared_ref();
    return *this;
  }
  KSharedPtr<T>& operator= ( T* p ) { 
    if ( ptr == p ) return *this;
    if ( ptr ) ptr->_KShared_unref();
    ptr = p; 
    if ( ptr ) ptr->_KShared_ref();
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

  uint count() const { return ptr->_KShared_count(); } // for debugging purposes
private:
  T* ptr;
};


/**
 * Can be used to control the lifetime of an object
 * that has derived @ref QShared. As long a someone holds
 * a QSharedPtr on some QShared object it won't become
 * deleted but is deleted once its reference count is 0.
 * This struct emulates C++ pointers perfectly. So just use
 * it like a simple C++ pointer.
 */
template< class T >
struct QSharedPtr
{
public:
  QSharedPtr() { ptr = 0; }
  QSharedPtr( T* t ) { ptr = t; }
  QSharedPtr( const QSharedPtr& p ) { ptr = p.ptr; if ( ptr ) ptr->ref(); }
  ~QSharedPtr() { if ( ptr && ptr->deref() ) delete ptr; }

  QSharedPtr<T>& operator= ( const QSharedPtr<T>& p ) {
    if ( ptr && ptr->deref() ) delete ptr;
    ptr = p.ptr; if ( ptr ) ptr->ref();
    return *this;
  }
  QSharedPtr<T>& operator= ( T* p ) { 
    if ( ptr && ptr->deref() ) delete ptr;
    ptr = p;
    return *this;
  }
  bool operator== ( const QSharedPtr<T>& p ) const { return ( ptr == p.ptr ); }
  bool operator!= ( const QSharedPtr<T>& p ) const { return ( ptr != p.ptr ); }
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

  uint count() const { return ptr->count; } // for debugging purposes
private:
  T* ptr;
};

#endif
