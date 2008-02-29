/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef PHONONDEFS_P_H
#define PHONONDEFS_P_H

#include <QtCore/QMetaType>
#include "medianode_p.h"

#define K_D(Class) Class##Private *const d = k_func()

#define PHONON_CONCAT_HELPER_INTERNAL(x, y) x ## y
#define PHONON_CONCAT_HELPER(x, y) PHONON_CONCAT_HELPER_INTERNAL(x, y)

#define PHONON_PRIVATECLASS \
protected: \
    virtual bool aboutToDeleteBackendObject(); \
    virtual void createBackendObject(); \
    /**
     * \internal
     * After construction of the Iface object this method is called
     * throughout the complete class hierarchy in order to set up the
     * properties that were already set on the public interface.
     *
     * An example implementation could look like this:
     * \code
     * ParentClassPrivate::setupBackendObject();
     * m_iface->setPropertyA(d->propertyA);
     * m_iface->setPropertyB(d->propertyB);
     * \endcode
     */ \
    void setupBackendObject();

#define PHONON_PRIVATEABSTRACTCLASS \
protected: \
    virtual bool aboutToDeleteBackendObject(); \
    /**
     * \internal
     * After construction of the Iface object this method is called
     * throughout the complete class hierarchy in order to set up the
     * properties that were already set on the public interface.
     *
     * An example implementation could look like this:
     * \code
     * ParentClassPrivate::setupBackendObject();
     * m_iface->setPropertyA(d->propertyA);
     * m_iface->setPropertyB(d->propertyB);
     * \endcode
     */ \
    void setupBackendObject();

#define PHONON_ABSTRACTBASE_IMPL \
PHONON_CLASSNAME::PHONON_CLASSNAME(PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) &dd, QObject *parent) \
    : QObject(parent), \
    MediaNode(dd) \
{ \
}

#define PHONON_OBJECT_IMPL \
PHONON_CLASSNAME::PHONON_CLASSNAME(QObject *parent) \
    : QObject(parent), \
    MediaNode(*new PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private)()) \
{ \
} \
void PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private)::createBackendObject() \
{ \
    if (m_backendObject) \
        return; \
    Q_Q(PHONON_CLASSNAME); \
    m_backendObject = Factory::PHONON_CONCAT_HELPER(create, PHONON_CLASSNAME)(q); \
    if (m_backendObject) { \
        setupBackendObject(); \
    } \
}

#define PHONON_HEIR_IMPL(parentclass) \
PHONON_CLASSNAME::PHONON_CLASSNAME(QObject *parent) \
    : parentclass(*new PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private), parent) \
{ \
} \
void PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private)::createBackendObject() \
{ \
    if (m_backendObject) \
        return; \
    Q_Q(PHONON_CLASSNAME); \
    m_backendObject = Factory::PHONON_CONCAT_HELPER(create, PHONON_CLASSNAME)(q); \
    if (m_backendObject) { \
        setupBackendObject(); \
    } \
}

#define BACKEND_GET(returnType, returnVar, methodName) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar))
#define BACKEND_GET1(returnType, returnVar, methodName, varType1, var1) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar), Q_ARG(varType1, var1))
#define BACKEND_GET2(returnType, returnVar, methodName, varType1, var1, varType2, var2) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar), Q_ARG(varType1, var1), Q_ARG(varType2, var2))
#define BACKEND_CALL(methodName) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection)
#define BACKEND_CALL1(methodName, varType1, var1) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection, Q_ARG(varType1, var1))
#define BACKEND_CALL2(methodName, varType1, var1, varType2, var2) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection, Q_ARG(varType1, var1), Q_ARG(varType2, var2))

#define pBACKEND_GET(returnType, returnVar, methodName) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar))
#define pBACKEND_GET1(returnType, returnVar, methodName, varType1, var1) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar), Q_ARG(varType1, var1))
#define pBACKEND_GET2(returnType, returnVar, methodName, varType1, var1, varType2, var2) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar), Q_ARG(varType1, var1), Q_ARG(varType2, var2))
#define pBACKEND_CALL(methodName) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection)
#define pBACKEND_CALL1(methodName, varType1, var1) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection, Q_ARG(varType1, var1))
#define pBACKEND_CALL2(methodName, varType1, var1, varType2, var2) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection, Q_ARG(varType1, var1), Q_ARG(varType2, var2))

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace
    {
        class NoIface;

        /// undefined result if the condition is neither true nor false
        template<bool condition, typename Then, typename Else> struct If;
        /// If false Else is the Result
        template<typename Then, typename Else> struct If<false, Then, Else> { typedef Else Result; };
        /// If true Then is the Result
        template<typename Then, typename Else> struct If<true, Then, Else> { typedef Then Result; };

        /// All template arguments are valid
        template<typename T> struct IsValid { enum { Result = true }; };
        /// except NoIface
        template<> struct IsValid<NoIface> { enum { Result = false }; };

        template<class R, class T>
        struct iface_cast_helper
        {
            static inline R *help(MediaNodePrivate *d)
            {
                return qobject_cast<T *>(d->m_backendObject);
            }

            static inline const R *help(const MediaNodePrivate *d)
            {
                return qobject_cast<T *>(d->m_backendObject);
            }
        };

        template<class R>
        struct iface_cast_helper<R, NoIface>
        {
            static inline R *help(const MediaNodePrivate *)
            {
                return 0;
            }
        };

    } // anonymous namespace

    /**
     * \internal
     *
     * \brief Helper class to cast the backend object to the correct version of the interface.
     *
     * Additions to the backend interfaces cannot be done by adding virtual methods as that would
     * break the binary interface. So the old class is renamed and a new class with the old name
     * inheriting the old class is added, containing all the new virtual methods.
     * Example:
     * \code
       class FooInterface
       {
       public:
           virtual ~FooInterface() {}
           virtual oldMethod() = 0;
       };
       Q_DECLARE_INTERFACE(FooInterface, "FooInterface0.phonon.kde.org")
     * \endcode
     * becomes
     * \code
       class FooInterface0
       {
       public:
           virtual ~FooInterface0() {}
           virtual oldMethod() = 0;
       };
       class FooInterface : public FooInterface0
       {
       public:
           virtual newMethod() = 0;
       };
       Q_DECLARE_INTERFACE(FooInterface0, "FooInterface0.phonon.kde.org")
       Q_DECLARE_INTERFACE(FooInterface,  "FooInterface1.phonon.kde.org")
     * \endcode
     *
     * With this, backends compiled against the old header can be qobject_casted to FooInterface0,
     * but not to FooInterface. On the other hand backends compiled against the new header (they first
     * need to implement newMethod) can only be qobject_casted to FooInterface but not to
     * FooInterface0. (The qobject_cast relies on the string in Q_DECLARE_INTERFACE and not the
     * class name which is why it behaves that way.)
     *
     * Now, in order to call oldMethod, the code needs to try to cast to both FooInterface and
     * FooInterface0 (new backends will work with the former, old backends with the latter) and then
     * if one of them in non-zero call oldMethod on it.
     *
     * To call newMethod only a cast to FooInterface needs to be done.
     *
     * The Iface class does all this for you for up to three (for now) interface revisions. Just
     * create an object like this:
     * \code
       Iface<FooInterface, FooInterface0> iface0(d);
       if (iface0) {
           iface0->oldMethod();
       }
       Iface<FooInterface> iface(d);
       if (iface) {
           iface->newMethod();
       }
     * \endcode
     *
     * This becomes a bit more convenient if you add macros like this:
     * \code
       #define IFACES1 FooInterface
       #define IFACES0 IFACES1, FooInterface0
     * \endcode
     * which you can use like this:
     * \code
       Iface<IFACES0> iface0(d);
       if (iface0) {
           iface0->oldMethod();
       }
       Iface<IFACES1> iface(d);
       if (iface) {
           iface->newMethod();
       }
     * \endcode
     * With the next revision you can then change the macros to
     * \code
       #define IFACES2 FooInterface
       #define IFACES1 IFACES2, FooInterface1
       #define IFACES0 IFACES1, FooInterface0
     * \endcode
     *
     * \author Matthias Kretz <kretz@kde.org>
     */
    template<class T0, class T1 = NoIface, class T2 = NoIface>
    class Iface
    {
    public:
        typedef typename If<IsValid<T2>::Result, T2, typename If<IsValid<T1>::Result, T1, T0>::Result>::Result Type;
        static inline Type *cast(MediaNodePrivate *const d)
        {
            Type *ret;
            ret = qobject_cast<T0 *>(d->m_backendObject);
            if (ret) return ret;
            ret = iface_cast_helper<Type, T1>::help(d);
            if (ret) return ret;
            ret = iface_cast_helper<Type, T2>::help(d);
            return ret;
        }

        static inline const Type *cast(const MediaNodePrivate *const d)
        {
            const Type *ret;
            ret = qobject_cast<T0 *>(d->m_backendObject);
            if (ret) return ret;
            ret = iface_cast_helper<Type, T1>::help(d);
            if (ret) return ret;
            ret = iface_cast_helper<Type, T2>::help(d);
            return ret;
        }

        inline Iface(MediaNodePrivate *const d) : iface(cast(d)) {}
        inline operator       Type *()       { return iface; }
        inline operator const Type *() const { return iface; }
        inline       Type *operator->()       { return iface; }
        inline const Type *operator->() const { return iface; }
    private:
        Type *const iface;
    };

    template<class T0, class T1 = NoIface, class T2 = NoIface>
    class ConstIface
    {
    public:
        typedef typename Iface<T0, T1, T2>::Type Type;
        inline ConstIface(const MediaNodePrivate *const d) : iface(Iface<T0, T1, T2>::cast(d)) {}
        inline operator const Type *() const { return iface; }
        inline const Type *operator->() const { return iface; }
    private:
        const Type *const iface;
    };
} // namespace Phonon

QT_END_NAMESPACE

#define INTERFACE_CALL(function) \
Iface<PHONON_INTERFACENAME >::cast(d)->function

#define pINTERFACE_CALL(function) \
Iface<PHONON_INTERFACENAME >::cast(this)->function

#define PHONON_GETTER(rettype, name, retdefault) \
rettype PHONON_CLASSNAME::name() const \
{ \
    const PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    if (!d->m_backendObject) \
        return retdefault; \
    rettype ret; \
    BACKEND_GET(rettype, ret, #name); \
    return ret; \
}

#define PHONON_INTERFACE_GETTER(rettype, name, retdefault) \
rettype PHONON_CLASSNAME::name() const \
{ \
    const PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    if (!d->m_backendObject) \
        return retdefault; \
    return Iface<PHONON_INTERFACENAME >::cast(d)->name(); \
}

#define PHONON_GETTER1(rettype, name, retdefault, argtype1, argvar1) \
rettype PHONON_CLASSNAME::name(argtype1 argvar1) const \
{ \
    const PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    if (!d->m_backendObject) \
        return retdefault; \
    rettype ret; \
    BACKEND_GET1(rettype, ret, #name, const QObject *, argvar1->k_ptr->backendObject()); \
    return ret; \
}

#define PHONON_INTERFACE_GETTER1(rettype, name, retdefault, argtype1, argvar1) \
rettype PHONON_CLASSNAME::name(argtype1 argvar1) const \
{ \
    const PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    if (!d->m_backendObject) \
        return retdefault; \
    return Iface<PHONON_INTERFACENAME >::cast(d)->name(argvar1->k_ptr->backendObject()); \
}

#define PHONON_SETTER(functionname, privatevar, argtype1) \
void PHONON_CLASSNAME::functionname(argtype1 x) \
{ \
    PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    d->privatevar = x; \
    if (k_ptr->backendObject()) { \
        BACKEND_CALL1(#functionname, argtype1, x); \
    } \
}

#define PHONON_INTERFACE_SETTER(functionname, privatevar, argtype1) \
void PHONON_CLASSNAME::functionname(argtype1 x) \
{ \
    PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    d->privatevar = x; \
    if (k_ptr->backendObject()) { \
        Iface<PHONON_INTERFACENAME >::cast(d)->functionname(x); \
    } \
}

#ifndef METATYPE_QLIST_INT_DEFINED
#define METATYPE_QLIST_INT_DEFINED
// Want this exactly once, see phonondefs_p.h kcm/outputdevicechoice.cpp
Q_DECLARE_METATYPE(QList<int>)
#endif

#endif // PHONONDEFS_P_H
