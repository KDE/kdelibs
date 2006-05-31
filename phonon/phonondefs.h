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

#ifndef PHONONDEFS_H
#define PHONONDEFS_H

#define K_DECLARE_PRIVATE( classname ) \
	inline classname ## Private* k_func() { return reinterpret_cast<classname ## Private*>( k_ptr ); } \
	inline const classname ## Private* k_func() const { return reinterpret_cast<classname ## Private*>( k_ptr ); } \
	friend class classname ## Private;
#define K_DECLARE_PUBLIC( classname ) \
	inline classname* q_func() { return static_cast<classname*>( q_ptr ); } \
	inline const classname* q_func() const { return static_cast<classname*>( q_ptr ); } \
	friend class classname;
#define K_D( classname ) classname ## Private* const d = k_func()
#define K_Q( classname ) classname* const q = q_func()

#define PHONON_PRIVATECLASS( classname, baseclass ) \
protected: \
	virtual bool aboutToDeleteIface(); \
	virtual void createIface(); \

#define PHONON_PRIVATEABSTRACTCLASS( classname, baseclass ) \
protected: \
	virtual bool aboutToDeleteIface(); \

/**
 * \internal
 * Used in class declarations to provide the needed functions. This is used for
 * abstract base classes.
 *
 * \param classname The Name of the class this macro is used for.
 *
 * Example:
 * \code
 * class AbstractEffect : public QObject
 * {
 *   Q _OBJECT
 *   Q_PROPERTY( int propertyA READ propertyA WRITE setPropertyA )
 *   PHONON_ABSTRACTBASE( AbstractEffect )
 *   public:
 *     int propertyA() const;
 *     void setPropertyA( int );
 * };
 * \endcode
 *
 * \see PHONON_OBJECT
 * \see PHONON_HEIR
 */
#define PHONON_ABSTRACTBASE( classname ) \
protected: \
	/**
	 * \internal
	 * Constructor that is called from derived classes.
	 *
	 * \param parent Standard QObject parent.
	 */ \
	classname( classname ## Private& dd, QObject* parent ); \
	/**
	 * \internal
	 * After construction of the Iface object this method is called
	 * throughout the complete class hierarchy in order to set up the
	 * properties that were already set on the public interface.
	 *
	 * An example implementation could look like this:
	 * \code
	 * ParentClass::setupIface();
	 * m_iface->setPropertyA( d->propertyA );
	 * m_iface->setPropertyB( d->propertyB );
	 * \endcode
	 */ \
	void setupIface();

/**
 * \internal
 * Used in class declarations to provide the needed functions. This is used for
 * classes that inherit QObject directly.
 *
 * \param classname The Name of the class this macro is used for.
 *
 * Example:
 * \code
 * class EffectSettings : public QObject
 * {
 *   Q _OBJECT
 *   Q_PROPERTY( int propertyA READ propertyA WRITE setPropertyA )
 *   PHONON_OBJECT( EffectSettings )
 *   public:
 *     int propertyA() const;
 *     void setPropertyA( int );
 * };
 * \endcode
 *
 * \see PHONON_ABSTRACTBASE
 * \see PHONON_HEIR
 */
#define PHONON_OBJECT( classname ) \
public: \
	/**
	 * Standard QObject constructor.
	 *
	 * \param parent QObject parent
	 */ \
	classname( QObject* parent = 0 ); \
protected: \
	/**
	 * \internal
	 *
	 * Constructs new instance of this class with private data \p dd and a
	 * \p parent.
	 */ \
	classname( classname ## Private& dd, QObject* parent ); \
	/**
	 * \internal
	 * After construction of the Iface object this method is called
	 * throughout the complete class hierarchy in order to set up the
	 * properties that were already set on the public interface.
	 *
	 * An example implementation could look like this:
	 * \code
	 * ParentClass::setupIface();
	 * m_iface->setPropertyA( d->propertyA );
	 * m_iface->setPropertyB( d->propertyB );
	 * \endcode
	 */ \
	void setupIface();

/**
 * \internal
 * Used in class declarations to provide the needed functions. This is used for
 * classes that inherit another Phonon object.
 *
 * \param classname The Name of the class this macro is used for.
 *
 * Example:
 * \code
 * class ConcreteEffect : public AbstractEffect
 * {
 *   Q _OBJECT
 *   Q_PROPERTY( int propertyB READ propertyB WRITE setPropertyB )
 *   PHONON_HEIR( ConcreteEffect )
 *   public:
 *     int propertyB() const;
 *     void setPropertyB( int );
 * };
 * \endcode
 *
 * \see PHONON_ABSTRACTBASE
 * \see PHONON_OBJECT
 */
#define PHONON_HEIR( classname ) \
public: \
	/**
	 * Standard QObject constructor.
	 *
	 * \param parent QObject parent
	 */ \
	classname( QObject* parent = 0 ); \
protected: \
	classname( classname ## Private& dd, QObject* parent ); \
	/**
	 * \internal
	 * After construction of the Iface object this method is called
	 * throughout the complete class hierarchy in order to set up the
	 * properties that were already set on the public interface.
	 *
	 * An example implementation could look like this:
	 * \code
	 * ParentClass::setupIface();
	 * m_iface->setPropertyA( d->propertyA );
	 * m_iface->setPropertyB( d->propertyB );
	 * \endcode
	 */ \
	void setupIface();

#define PHONON_ABSTRACTBASE_IMPL( classname ) \
classname::classname( classname ## Private& dd, QObject* parent ) \
	: QObject( parent ) \
	, Base( dd ) \
{ \
}

#define PHONON_OBJECT_IMPL( classname ) \
classname::classname( QObject* parent ) \
	: QObject( parent ) \
	, Base( *new classname ## Private() ) \
{ \
	K_D( classname ); \
	d->createIface(); \
} \
classname::classname( classname ## Private& dd, QObject* parent ) \
	: QObject( parent ) \
	, Base( dd ) \
{ \
} \
void classname ## Private::createIface() \
{ \
	if( backendObject ) \
		return; \
	K_Q( classname ); \
	backendObject = Factory::self()->create ## classname( q ); \
	if( backendObject ) \
		q->setupIface(); \
}

#define PHONON_HEIR_IMPL( classname, parentclass ) \
classname::classname( QObject* parent ) \
	: parentclass( *new classname ## Private, parent ) \
{ \
	K_D( classname ); \
	d->createIface(); \
} \
classname::classname( classname ## Private& dd, QObject* parent ) \
	: parentclass( dd, parent ) \
{ \
} \
void classname ## Private::createIface() \
{ \
	if( backendObject ) \
		return; \
	K_Q( classname ); \
	backendObject = Factory::self()->create ## classname( q ); \
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

#define PHONON_GETTER( classname, rettype, name, retdefault ) \
rettype classname::name() const \
{ \
	K_D( const classname ); \
	if( !d->backendObject ) \
		return retdefault; \
	rettype ret; \
	BACKEND_GET( rettype, ret, #name ); \
	return ret; \
}

#define PHONON_GETTER1( classname, rettype, name, retdefault, argtype1, argvar1 ) \
rettype classname::name( argtype1 argvar1 ) const \
{ \
	K_D( const classname ); \
	if( !d->backendObject ) \
		return retdefault; \
	rettype ret; \
	BACKEND_GET1( rettype, ret, #name, const QObject*, argvar1->iface() ); \
	return ret; \
}

#define PHONON_SETTER( classname, functionname, privatevar, argtype1 ) \
void classname::functionname( argtype1 x ) \
{ \
	K_D( classname ); \
	if( iface() ) \
		BACKEND_CALL1( #functionname, argtype1, x ); \
	else \
		d->privatevar = x; \
}

#define NAMEDESCRIPTIONFROMINDEX( classname, backendPrefix ) \
classname classname::fromIndex( int index ) \
{ \
	QObject* b = Factory::self()->backend(); \
	QSet<int> indexes; \
	QMetaObject::invokeMethod( b, #backendPrefix"Indexes", Qt::DirectConnection, Q_RETURN_ARG( QSet<int>, indexes ) ); \
	if( !indexes.contains( index ) ) \
		return classname(); \
	QString name, description; \
	QMetaObject::invokeMethod( b, #backendPrefix"Name", Qt::DirectConnection, Q_RETURN_ARG( QString, name ), Q_ARG( int, index ) ); \
	QMetaObject::invokeMethod( b, #backendPrefix"Description", Qt::DirectConnection, Q_RETURN_ARG( QString, description ), Q_ARG( int, index ) ); \
	return classname( index, name, description ); \
}

#endif // PHONONDEFS_H
