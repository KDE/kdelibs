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
     * @param p the pointer
     */
    KSharedPtr( T* p ) // TODO: Make explicit
        : ptr(p) { if(ptr) ptr->ref.ref(); }

    /**
     * Copies a pointer.
     * @param p the pointer to copy
     */
    KSharedPtr( const KSharedPtr& p )
        : ptr(p.ptr) { if(ptr) ptr->ref.ref(); }

    /**
     * Unreferences the object that this pointer points to. If it was
     * the last reference, the object will be deleted.
     */
    ~KSharedPtr() { if (ptr && !ptr->ref.deref()) delete ptr; }

    inline KSharedPtr<T>& operator= ( const KSharedPtr& p ) { attach(p); return *this; }
    inline KSharedPtr<T>& operator= ( T* p ) { attach(p); return *this; }
    inline bool operator== ( const KSharedPtr& p ) const { return ( ptr == p.ptr ); }
    inline bool operator!= ( const KSharedPtr& p ) const { return ( ptr != p.ptr ); }
    inline bool operator== ( const T* p ) const { return ( ptr == p ); }
    inline bool operator!= ( const T* p ) const { return ( ptr != p ); }
    inline operator bool() const { return ( ptr != 0 ); }

    /**
     * @return the pointer
     */
    inline T* data() { return ptr; }

    /**
     * @return the pointer
     */
    inline const T* data() const { return ptr; }

    /**
     * @return a const pointer to the shared object.
     */
    inline const T* constData() const { return ptr; }

    inline const T& operator*() const { Q_ASSERT(ptr); return *ptr; }
    inline T& operator*() { Q_ASSERT(ptr); return *ptr; }
    inline const T* operator->() const { Q_ASSERT(ptr); return ptr; }
    inline T* operator->() { Q_ASSERT(ptr); return ptr; }

    /**
     * Attach the given pointer to the KSharedPtr.
     * If the previous shared pointer is not owned by any KSharedPtr,
     * it is deleted.
     */
    inline void attach(T *p)
    {
        if (ptr != p) {
            T *x = p;
            if (x) x->ref.ref();
            x = qAtomicSetPtr(&ptr, x);
            if (x && !x->ref.deref())
                delete x;
        }
    }

    /**
     * Attach the given pointer to the KSharedPtr.
     * @see KSharedPtr<T>::attach(T *p)
     */
    inline void attach(const KSharedPtr& p) { attach(p.ptr); }

    /**
     * Returns the number of references.
     * @return the number of references
     */
    inline int count() const { return ptr ? (int)ptr->ref : 0; } // for debugging purposes

    /**
     * Detach the pointer by attaching a new copy of the pointer.
     * The new copy is created only if the pointer is shared by other pointers
     * and is not null.
     */
    inline void detach() { if (ptr && ptr->ref>1) attach(new T(*ptr)); }

    /**
     * @return Whether this is the only shared pointer pointing to
     * to the pointee, or whether it's shared among multiple
     * shared pointers.
     */
    inline bool isUnique() const { return count() == 1; }

    template <class U> friend class KSharedPtr;

    /**
     * Convert KSharedPtr<U> to KSharedPtr<T>, using a static_cast.
     * This will compile whenever T* and U* are compatible, i.e.
     * T is a subclass of U or vice-versa.
     * Example syntax:
     * <code>
     *   KSharedPtr<T> tPtr;
     *   KSharedPtr<U> uPtr = KSharedPtr<U>::staticCast( tPtr );
     * </code>
     */
    template <class U>
    static KSharedPtr<T> staticCast( const KSharedPtr<U>& other ) {
        return KSharedPtr<T>( static_cast<T *>( other.ptr ) );
    }
    /**
     * Convert KSharedPtr<U> to KSharedPtr<T>, using a dynamic_cast.
     * This will compile whenever T* and U* are compatible, i.e.
     * T is a subclass of U or vice-versa.
     * Example syntax:
     * <code>
     *   KSharedPtr<T> tPtr;
     *   KSharedPtr<U> uPtr = KSharedPtr<U>::dynamicCast( tPtr );
     * </code>
     * Since a dynamic_cast is used, if U derives from T, and tPtr isn't an instance of U, uPtr will be 0.
     */
    template <class U>
    static KSharedPtr<T> dynamicCast( const KSharedPtr<U>& other ) {
        return KSharedPtr<T>( dynamic_cast<T *>( other.ptr ) );
    }

private:
    T* ptr;
};

#endif // KSHAREDPTR_H
