/*
 * This file is part of the DOM implementation for KDE.
 * Copyright (C) 2005, 2006 Apple Computer, Inc.
 * Copyright (C) 2002 Lars Knoll
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */


#ifndef SHARED_H
#define SHARED_H

namespace khtml {

template<class type> class Shared
{
public:
    Shared() { _ref=0; /*counter++;*/ }
    ~Shared() { /*counter--;*/ }

    void ref() { _ref++;  }
    void deref() {
	if(_ref) _ref--;
	if(!_ref)
	    delete static_cast<type *>(this);
    }
    bool hasOneRef() { //kdDebug(300) << "ref=" << _ref << endl;
    	return _ref==1; }

    int refCount() const { return _ref; }
//    static int counter;
protected:
    unsigned int _ref;
};

template<class type> class TreeShared
{
public:
    TreeShared() { _ref = 0; m_parent = 0; /*counter++;*/ }
    TreeShared( type *parent ) { _ref=0; m_parent = parent; /*counter++;*/ }
    ~TreeShared() { /*counter--;*/ }
    
    virtual void removedLastRef() { delete static_cast<type*>(this); }

    void ref() { _ref++;  }
    void deref() {
	if(_ref) _ref--;
	if(!_ref && !m_parent) {
	    removedLastRef();
	}
    }
    bool hasOneRef() { //kdDebug(300) << "ref=" << _ref << endl;
    	return _ref==1; }

    int refCount() const { return _ref; }
//    static int counter;

    void setParent(type *parent) { m_parent = parent; }
    type *parent() const { return m_parent; }
private:
    unsigned int _ref;
protected:
    type *m_parent;
};

template <class T> class SharedPtr;

template <class T> bool operator==(const SharedPtr<T> &a, const SharedPtr<T> &b);
template <class T> bool operator==(const SharedPtr<T> &a, const T *b);
template <class T> bool operator==(const T *a, const SharedPtr<T> &b);

template <class T> class SharedPtr
{
public:
    SharedPtr() : m_ptr(0) {}
    explicit SharedPtr(T *ptr) : m_ptr(ptr) { if (m_ptr) m_ptr->ref(); }
    SharedPtr(const SharedPtr &o) : m_ptr(o.m_ptr) { if (m_ptr) m_ptr->ref(); }
    ~SharedPtr() { if (m_ptr) m_ptr->deref(); }

    bool isNull() const { return m_ptr == 0; }
    bool notNull() const { return m_ptr != 0; }

    void reset() { if (m_ptr) m_ptr->deref(); m_ptr = 0; }
    void reset(T *o) { if (o) o->ref(); if (m_ptr) m_ptr->deref(); m_ptr = o; }

    T * get() const { return m_ptr; }
    T &operator*() const { return *m_ptr; }
    T *operator->() const { return m_ptr; }

    bool operator!() const { return m_ptr == 0; }

    template <class C> friend bool operator==(const SharedPtr<C> &a, const SharedPtr<C> &b);
    template <class C> friend bool operator==(const SharedPtr<C> &a, const C *b);
    template <class C> friend bool operator==(const C *a, const SharedPtr<C> &b);

    SharedPtr &operator=(const SharedPtr &);

private:
    T* m_ptr;
};

template <class T> SharedPtr<T> &SharedPtr<T>::operator=(const SharedPtr<T> &o)
{
    if (o.m_ptr)
        o.m_ptr->ref();
    if (m_ptr)
        m_ptr->deref();
    m_ptr = o.m_ptr;
    return *this;
}

template <class T> inline bool operator==(const SharedPtr<T> &a, const SharedPtr<T> &b) { return a.m_ptr == b.m_ptr; }
template <class T> inline bool operator==(const SharedPtr<T> &a, const T *b) { return a.m_ptr == b; }
template <class T> inline bool operator==(const T *a, const SharedPtr<T> &b) { return a == b.m_ptr; }

template <class T> inline bool operator!=(const SharedPtr<T> &a, const SharedPtr<T> &b) { return !(a==b); }
template <class T> inline bool operator!=(const SharedPtr<T> &a, const T *b) { return !(a == b); }
template <class T> inline bool operator!=(const T *a, const SharedPtr<T> &b) { return !(a == b); }

template <class T, class U> inline SharedPtr<T> static_pointer_cast(const SharedPtr<U> &p) { return SharedPtr<T>(static_cast<T *>(p.get())); }
template <class T, class U> inline SharedPtr<T> const_pointer_cast(const SharedPtr<U> &p) { return SharedPtr<T>(const_cast<T *>(p.get())); }

//A special pointer for nodes keeping track of the document,
//which helps distinguish back links from them to it, in order to break 
//cycles
template <class T> class DocPtr {
public:
    DocPtr() : m_ptr(0) {}
    DocPtr(T *ptr) : m_ptr(ptr) { if (ptr) ptr->selfOnlyRef(); }
    DocPtr(const DocPtr &o) : m_ptr(o.m_ptr) { if (T *ptr = m_ptr) ptr->selfOnlyRef(); }
    ~DocPtr() { if (T *ptr = m_ptr) ptr->selfOnlyDeref(); }
    
    template <class U> DocPtr(const DocPtr<U> &o) : m_ptr(o.get()) { if (T *ptr = m_ptr) ptr->selfOnlyRef(); }
    
    void resetSkippingRef(T *o) { m_ptr = o; }
    
    T *get() const { return m_ptr; }
    
    T &operator*() const { return *m_ptr; }
    T *operator->() const { return m_ptr; }
    
    bool operator!() const { return !m_ptr; }

    // this type conversion operator allows implicit conversion to
    // bool but not to other integer types
    
    typedef T * (DocPtr::*UnspecifiedBoolType)() const;
    operator UnspecifiedBoolType() const
    {
        return m_ptr ? &DocPtr::get : 0;
    }
    
    DocPtr &operator=(const DocPtr &);
    DocPtr &operator=(T *);
    
 private:
    T *m_ptr;
};

template <class T> DocPtr<T> &DocPtr<T>::operator=(const DocPtr<T> &o) 
{
    T *optr = o.m_ptr;
    if (optr)
        optr->selfOnlyRef();
    if (T *ptr = m_ptr)
        ptr->selfOnlyDeref();
    m_ptr = optr;
        return *this;
}

template <class T> inline DocPtr<T> &DocPtr<T>::operator=(T *optr)
{
    if (optr)
        optr->selfOnlyRef();
    if (T *ptr = m_ptr)
        ptr->selfOnlyDeref();
    m_ptr = optr;
    return *this;
}

template <class T> inline bool operator==(const DocPtr<T> &a, const DocPtr<T> &b) 
{ 
    return a.get() == b.get(); 
}

template <class T> inline bool operator==(const DocPtr<T> &a, const T *b) 
{ 
    return a.get() == b; 
}

template <class T> inline bool operator==(const T *a, const DocPtr<T> &b) 
{
    return a == b.get(); 
}

template <class T> inline bool operator!=(const DocPtr<T> &a, const DocPtr<T> &b) 
{ 
    return a.get() != b.get(); 
}

template <class T> inline bool operator!=(const DocPtr<T> &a, const T *b)
{
    return a.get() != b; 
}

template <class T> inline bool operator!=(const T *a, const DocPtr<T> &b) 
{ 
    return a != b.get(); 
}


} // namespace

#endif
