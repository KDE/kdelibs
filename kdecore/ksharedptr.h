/*
 * This file is part of the KDE libraries.
 *
 * Copyright 2005 Frerich Raabe <raabe@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef KSHAREDPTR_H
#define KSHAREDPTR_H

#include <qshareddata.h>
#include <kdemacros.h>

typedef QSharedData KShared;

/**
 * Can be used to control the lifetime of an object that has derived
 * KShared. As long a someone holds a KSharedPtr on some QSharedData
 * object it won't become deleted but is deleted once its reference
 * count is 0.  This struct emulates C++ pointers virtually perfectly.
 * So just use it like a simple C++ pointer.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
template< class T >
class KSharedPtr
{
public:
    /**
     * Creates a null pointer.
     */
    KSharedPtr()
        : ptr(0) { }
    /**
     * Creates a new pointer.
     * @param t the pointer
     */
    KSharedPtr( T* t ) // TODO explicit
        : ptr(t) { if ( ptr ) ptr->ref.ref(); }

    /**
     * Copies a pointer.
     * @param p the pointer to copy
     */
    KSharedPtr( const KSharedPtr& p )
        : ptr(p.ptr) { if ( ptr ) ptr->ref.ref(); }

    /**
     * Unreferences the object that this pointer points to. If it was
     * the last reference, the object will be deleted.
     */
    ~KSharedPtr() { if ( ptr ) deref(); }

    KSharedPtr<T>& operator= ( const KSharedPtr<T>& p ) {
        if ( ptr == p.ptr ) return *this;
        if ( ptr ) deref();
        ptr = p.ptr;
        if ( ptr ) ptr->ref.ref();
        return *this;
    }
    KSharedPtr<T>& operator= ( T* p ) {
        if ( ptr == p ) return *this;
        if ( ptr ) deref();
        ptr = p;
        if ( ptr ) ptr->ref.ref();
        return *this;
    }
    bool operator== ( const KSharedPtr<T>& p ) const { return ( ptr == p.ptr ); }
    bool operator!= ( const KSharedPtr<T>& p ) const { return ( ptr != p.ptr ); }
    bool operator== ( const T* p ) const { return ( ptr == p ); }
    bool operator!= ( const T* p ) const { return ( ptr != p ); }
    operator bool() const { return ( ptr != 0 ); }

    /**
     * @deprecated use data()
     */
    KDE_DEPRECATED T *get() const { return ptr; }

    /**
     * @return the pointer
     */
    T* data() { return ptr; }

    /**
     * @return the pointer
     */
    const T* data() const { return ptr; }
    /**
     * @return a const pointer to the shared object.
     */
    const T* constData() const { return ptr; }

    const T& operator*() const { return *ptr; }
    T& operator*() { return *ptr; }
    const T* operator->() const { return ptr; }
    T* operator->() { return ptr; }

    /**
     * Returns the number of references.
     * @return the number of references
     */
    int count() const { return ptr->ref; } // for debugging purposes

    /**
     * @return Whether this is the only shared pointer pointing to
     * to the pointee, or whether it's shared among multiple
     * shared pointers.
     */
    bool isUnique() const { return count() == 1; }

    template <class U> friend class KSharedPtr;

    /**
     *
     */
    template <class U>
    static KSharedPtr<T> staticCast( const KSharedPtr<U>& other ) {
        return KSharedPtr<T>( static_cast<T *>( other.ptr ) );
    }
    template <class U>
    static KSharedPtr<T> dynamicCast( const KSharedPtr<U>& other ) {
        return KSharedPtr<T>( dynamic_cast<T *>( other.ptr ) );
    }

private:
    void deref() {
        if ( !ptr->ref.deref() )
            delete ptr;
    }

private:
    T* ptr;
};

#endif // KSHAREDPTR_H
