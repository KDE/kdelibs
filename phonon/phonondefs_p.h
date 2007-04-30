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
    : QObject(parent) \
    , k_ptr(&dd) \
{ \
    k_ptr->q_ptr = this; \
} \
bool PHONON_CLASSNAME::isValid() const \
{ \
    return (k_func()->m_backendObject != 0); \
}

#define PHONON_OBJECT_IMPL \
PHONON_CLASSNAME::PHONON_CLASSNAME(QObject *parent) \
    : QObject(parent) \
    , k_ptr(new PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private)()) \
{ \
    PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    d->q_ptr = this; \
    d->createBackendObject(); \
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
} \
bool PHONON_CLASSNAME::isValid() const \
{ \
    return (k_func()->m_backendObject != 0); \
}

#define PHONON_HEIR_IMPL(parentclass) \
PHONON_CLASSNAME::PHONON_CLASSNAME(QObject *parent) \
    : parentclass(*new PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private), parent) \
{ \
    PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    d->createBackendObject(); \
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

#define INTERFACE_CALL(function) \
qobject_cast<PHONON_INTERFACENAME *>(d->m_backendObject)->function

#define pINTERFACE_CALL(function) \
qobject_cast<PHONON_INTERFACENAME *>(m_backendObject)->function

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
    return qobject_cast<PHONON_INTERFACENAME *>(d->m_backendObject)->name(); \
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
    return qobject_cast<PHONON_INTERFACENAME *>(d->m_backendObject)->name(argvar1->k_ptr->backendObject()); \
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
        qobject_cast<PHONON_INTERFACENAME *>(d->m_backendObject)->functionname(x); \
    } \
}

#endif // PHONONDEFS_P_H
