/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#define PHONON_CONCAT_HELPER_INTERNAL( x, y ) x ## y
#define PHONON_CONCAT_HELPER( x, y ) PHONON_CONCAT_HELPER_INTERNAL( x, y )

#define K_DECLARE_PUBLIC( classname ) \
	inline classname* q_func() { return static_cast<classname*>( q_ptr ); } \
	inline const classname* q_func() const { return static_cast<classname*>( q_ptr ); } \
	friend class classname;
#define K_D( classname ) PHONON_CONCAT_HELPER( classname, Private )* const d = k_func()
#define K_Q( classname ) classname* const q = q_func()

#define PHONON_PRIVATECLASS( baseclass ) \
protected: \
	virtual bool aboutToDeleteIface(); \
	virtual void createIface(); \

#define PHONON_PRIVATEABSTRACTCLASS( baseclass ) \
protected: \
	virtual bool aboutToDeleteIface(); \

#define PHONON_ABSTRACTBASE_IMPL \
PHONON_CLASSNAME::PHONON_CLASSNAME( PHONON_CONCAT_HELPER( PHONON_CLASSNAME, Private )& dd, QObject* parent ) \
	: QObject( parent ) \
	, Base( dd ) \
{ \
}

#define PHONON_OBJECT_IMPL \
PHONON_CLASSNAME::PHONON_CLASSNAME( QObject* parent ) \
	: QObject( parent ) \
	, Base( *new PHONON_CONCAT_HELPER( PHONON_CLASSNAME, Private )() ) \
{ \
	K_D( PHONON_CLASSNAME ); \
	d->createIface(); \
} \
void PHONON_CONCAT_HELPER( PHONON_CLASSNAME, Private )::createIface() \
{ \
	if( backendObject ) \
		return; \
	K_Q( PHONON_CLASSNAME ); \
	backendObject = Factory::self()->PHONON_CONCAT_HELPER( create, PHONON_CLASSNAME )( q ); \
	if( backendObject ) \
		q->setupIface(); \
}

#define PHONON_HEIR_IMPL( parentclass ) \
PHONON_CLASSNAME::PHONON_CLASSNAME( QObject* parent ) \
	: parentclass( *new PHONON_CONCAT_HELPER( PHONON_CLASSNAME, Private ), parent ) \
{ \
	K_D( PHONON_CLASSNAME ); \
	d->createIface(); \
} \
void PHONON_CONCAT_HELPER( PHONON_CLASSNAME, Private )::createIface() \
{ \
	if( backendObject ) \
		return; \
	K_Q( PHONON_CLASSNAME ); \
	backendObject = Factory::self()->PHONON_CONCAT_HELPER( create, PHONON_CLASSNAME )( q ); \
	if( backendObject ) \
		q->setupIface(); \
}

#define BACKEND_GET( returnType, returnVar, methodName ) \
QMetaObject::invokeMethod( d->backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG( returnType, returnVar ) )
#define BACKEND_GET1( returnType, returnVar, methodName, varType1, var1 ) \
QMetaObject::invokeMethod( d->backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG( returnType, returnVar ), Q_ARG( varType1, var1 ) )
#define BACKEND_GET2( returnType, returnVar, methodName, varType1, var1, varType2, var2 ) \
QMetaObject::invokeMethod( d->backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG( returnType, returnVar ), Q_ARG( varType1, var1 ), Q_ARG( varType2, var2 ) )
#define BACKEND_CALL( methodName ) \
QMetaObject::invokeMethod( d->backendObject, methodName, Qt::DirectConnection )
#define BACKEND_CALL1( methodName, varType1, var1 ) \
QMetaObject::invokeMethod( d->backendObject, methodName, Qt::DirectConnection, Q_ARG( varType1, var1 ) )
#define BACKEND_CALL2( methodName, varType1, var1, varType2, var2 ) \
QMetaObject::invokeMethod( d->backendObject, methodName, Qt::DirectConnection, Q_ARG( varType1, var1 ), Q_ARG( varType2, var2 ) )

#define pBACKEND_GET( returnType, returnVar, methodName ) \
QMetaObject::invokeMethod( backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG( returnType, returnVar ) )
#define pBACKEND_GET1( returnType, returnVar, methodName, varType1, var1 ) \
QMetaObject::invokeMethod( backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG( returnType, returnVar ), Q_ARG( varType1, var1 ) )
#define pBACKEND_GET2( returnType, returnVar, methodName, varType1, var1, varType2, var2 ) \
QMetaObject::invokeMethod( backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG( returnType, returnVar ), Q_ARG( varType1, var1 ), Q_ARG( varType2, var2 ) )
#define pBACKEND_CALL( methodName ) \
QMetaObject::invokeMethod( backendObject, methodName, Qt::DirectConnection )
#define pBACKEND_CALL1( methodName, varType1, var1 ) \
QMetaObject::invokeMethod( backendObject, methodName, Qt::DirectConnection, Q_ARG( varType1, var1 ) )
#define pBACKEND_CALL2( methodName, varType1, var1, varType2, var2 ) \
QMetaObject::invokeMethod( backendObject, methodName, Qt::DirectConnection, Q_ARG( varType1, var1 ), Q_ARG( varType2, var2 ) )

#define INTERFACE_CALL( function ) \
qobject_cast<PHONON_INTERFACENAME*>( d->backendObject )->function()
#define INTERFACE_CALL1( function, arg1 ) \
qobject_cast<PHONON_INTERFACENAME*>( d->backendObject )->function( arg1 )
#define INTERFACE_CALL2( function, arg1, arg2 ) \
qobject_cast<PHONON_INTERFACENAME*>( d->backendObject )->function( arg1, arg2 )

#define INTERFACE_GET( function ) \
return qobject_cast<PHONON_INTERFACENAME*>( d->backendObject )->function()
#define INTERFACE_GET1( function, arg1 ) \
return qobject_cast<PHONON_INTERFACENAME*>( d->backendObject )->function( arg1 )
#define INTERFACE_GET2( function, arg1, arg2 ) \
return qobject_cast<PHONON_INTERFACENAME*>( d->backendObject )->function( arg1, arg2 )

#define pINTERFACE_CALL( function ) \
qobject_cast<PHONON_INTERFACENAME*>( backendObject )->function()
#define pINTERFACE_CALL1( function, arg1 ) \
qobject_cast<PHONON_INTERFACENAME*>( backendObject )->function( arg1 )
#define pINTERFACE_CALL2( function, arg1, arg2 ) \
qobject_cast<PHONON_INTERFACENAME*>( backendObject )->function( arg1, arg2 )

#define pINTERFACE_GET( function ) \
return qobject_cast<PHONON_INTERFACENAME*>( backendObject )->function()
#define pINTERFACE_GET1( function, arg1 ) \
return qobject_cast<PHONON_INTERFACENAME*>( backendObject )->function( arg1 )
#define pINTERFACE_GET2( function, arg1, arg2 ) \
return qobject_cast<PHONON_INTERFACENAME*>( backendObject )->function( arg1, arg2 )

#define PHONON_GETTER( rettype, name, retdefault ) \
rettype PHONON_CLASSNAME::name() const \
{ \
	K_D( const PHONON_CLASSNAME ); \
	if( !d->backendObject ) \
		return retdefault; \
	rettype ret; \
	BACKEND_GET( rettype, ret, #name ); \
	return ret; \
}

#define PHONON_INTERFACE_GETTER( rettype, name, retdefault ) \
rettype PHONON_CLASSNAME::name() const \
{ \
	K_D( const PHONON_CLASSNAME ); \
	if( !d->backendObject ) \
		return retdefault; \
	return qobject_cast<PHONON_INTERFACENAME*>( d->backendObject )->name(); \
}

#define PHONON_GETTER1( rettype, name, retdefault, argtype1, argvar1 ) \
rettype PHONON_CLASSNAME::name( argtype1 argvar1 ) const \
{ \
	K_D( const PHONON_CLASSNAME ); \
	if( !d->backendObject ) \
		return retdefault; \
	rettype ret; \
	BACKEND_GET1( rettype, ret, #name, const QObject*, argvar1->iface() ); \
	return ret; \
}

#define PHONON_INTERFACE_GETTER1( rettype, name, retdefault, argtype1, argvar1 ) \
rettype PHONON_CLASSNAME::name( argtype1 argvar1 ) const \
{ \
	K_D( const PHONON_CLASSNAME ); \
	if( !d->backendObject ) \
		return retdefault; \
	return qobject_cast<PHONON_INTERFACENAME*>( d->backendObject )->name( argvar1->iface() ); \
}

#define PHONON_SETTER( functionname, privatevar, argtype1 ) \
void PHONON_CLASSNAME::functionname( argtype1 x ) \
{ \
	K_D( PHONON_CLASSNAME ); \
	if( iface() ) \
		BACKEND_CALL1( #functionname, argtype1, x ); \
	else \
		d->privatevar = x; \
}

#define PHONON_INTERFACE_SETTER( functionname, privatevar, argtype1 ) \
void PHONON_CLASSNAME::functionname( argtype1 x ) \
{ \
	K_D( PHONON_CLASSNAME ); \
	if( iface() ) \
		qobject_cast<PHONON_INTERFACENAME*>( d->backendObject )->functionname( x ); \
	else \
		d->privatevar = x; \
}

#endif // PHONONDEFS_P_H
