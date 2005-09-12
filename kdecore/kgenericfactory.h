/* This file is part of the KDE project
 * Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef kgenericfactory_h
#define kgenericfactory_h

#include <klibloader.h>
#include <ktypelist.h>
#include <kinstance.h>
#include <kgenericfactory.tcc>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

/* @internal */
template <class T>
class KGenericFactoryBase
{
public:
    KGenericFactoryBase( const char *instanceName )
        : m_instanceName( instanceName )
    {
        m_aboutData=0L;
        s_self = this;
        m_catalogInitialized = false;
    }
    KGenericFactoryBase( const KAboutData *data )
        : m_aboutData(data)
    {
        s_self = this;
        m_catalogInitialized = false;
    }

    virtual ~KGenericFactoryBase()
    {
        if ( s_instance )
            KGlobal::locale()->removeCatalog( QString::fromAscii( s_instance->instanceName() ) );
        delete s_instance;
        s_instance = 0;
        s_self = 0;
    }

    static KInstance *instance();

protected:
    virtual KInstance *createInstance()
    {
        if ( m_aboutData )
            return new KInstance( m_aboutData );
        if ( m_instanceName.isNull() ) {
            kdWarning() << "KGenericFactory: instance requested but no instance name or about data passed to the constructor!" << endl;
            return 0;
        }
        return new KInstance( m_instanceName );
    }

    virtual void setupTranslations( void )
    {
        if ( instance() )
            KGlobal::locale()->insertCatalog( QString::fromAscii( instance()->instanceName() ) );
    }

    void initializeMessageCatalog()
    {
        if ( !m_catalogInitialized )
        {
            m_catalogInitialized = true;
            setupTranslations();
        }
    }

private:
    QByteArray m_instanceName;
    const KAboutData *m_aboutData;
    bool m_catalogInitialized;

    static KInstance *s_instance;
    static KGenericFactoryBase<T> *s_self;
};

/* @internal */
template <class T>
KInstance *KGenericFactoryBase<T>::s_instance = 0;

/* @internal */
template <class T>
KGenericFactoryBase<T> *KGenericFactoryBase<T>::s_self = 0;

/* @internal */
template <class T>
KInstance *KGenericFactoryBase<T>::instance()
{
    if ( !s_instance && s_self )
        s_instance = s_self->createInstance();
    return s_instance;
}

/**
 * This template provides a generic implementation of a KLibFactory ,
 * for use with shared library components. It implements the pure virtual
 * createObject method of KLibFactory and instantiates objects of the
 * specified class (template argument) when the class name argument of
 * createObject matches a class name in the given hierarchy.
 *
 * In case you are developing a KParts component, skip this file and
 * go directly to KParts::GenericFactory .
 *
 * Note that the class specified as template argument needs to provide
 * a certain constructor:
 * <ul>
 *     <li>If the class is derived from QObject then it needs to have
 *         a constructor like:
 *         <code>MyClass( QObject *parent, const char *name,
 *                        const QStringList &args );</code>
 *     <li>If the class is derived from QWidget then it needs to have
 *         a constructor like:
 *         <code>MyWidget( QWidget *parent, const char *name,
 *                         const QStringList &args);</code>
 *     <li>If the class is derived from KParts::Part then it needs to have
 *         a constructor like:
 *         <code>MyPart( QWidget *parentWidget, const char *widgetName,
 *                       QObject *parent, const char *name,
 *                       const QStringList &args );</code>
 * </ul>
 * The args QStringList passed to the constructor is the args string list
 * that the caller passed to KLibFactory's create method.
 *
 * In addition upon instantiation this template provides a central
 * KInstance object for your component, accessible through the
 * static instance() method. The instanceName argument of the
 * KGenericFactory constructor is passed to the KInstance object.
 *
 * The creation of the KInstance object can be customized by inheriting
 * from this template class and re-implementing the virtual createInstance
 * method. For example it could look like this:
 * \code
 *     KInstance *MyFactory::createInstance()
 *     {
 *         return new KInstance( myAboutData );
 *     }
 * \endcode
 *
 * Example of usage of the whole template:
 * \code
 *     class MyPlugin : public KParts::Plugin
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyPlugin( QObject *parent, const char *name,
 *                   const QStringList &args );
 *         ...
 *     };
 *
 *     K_EXPORT_COMPONENT_FACTORY( libmyplugin, KGenericFactory&lt;MyPlugin&gt; )
 * \endcode
 */
template <class Product, class ParentType = QObject>
class KGenericFactory : public KLibFactory, public KGenericFactoryBase<Product>
{
public:
    KGenericFactory( const char *instanceName = 0 )
        : KGenericFactoryBase<Product>( instanceName )
    {}

    /**
     * @since 3.3
	*/
    KGenericFactory( const KAboutData *data )
        : KGenericFactoryBase<Product>( data )
    {}


protected:
    virtual QObject *createObject( QObject *parent, const char *name,
                                  const char *className, const QStringList &args )
    {
        KGenericFactoryBase<Product>::initializeMessageCatalog();
        return KDEPrivate::ConcreteFactory<Product, ParentType>
            ::create( 0, 0, parent, name, className, args );
    }
};

/**
 * This template provides a generic implementation of a KLibFactory ,
 * for use with shared library components. It implements the pure virtual
 * createObject method of KLibFactory and instantiates objects of the
 * specified classes in the given typelist template argument when the class
 * name argument of createObject matches a class names in the given hierarchy
 * of classes.
 *
 * Note that each class in the specified in the typelist template argument
 * needs to provide a certain constructor:
 * <ul>
 *     <li>If the class is derived from QObject then it needs to have
 *         a constructor like:
 *         <code>MyClass( QObject *parent, const char *name,
 *                        const QStringList &args );</code>
 *     <li>If the class is derived from QWidget then it needs to have
 *         a constructor like:
 *         <code>MyWidget( QWidget *parent, const char *name,
 *                         const QStringList &args);</code>
 *     <li>If the class is derived from KParts::Part then it needs to have
 *         a constructor like:
 *         <code>MyPart( QWidget *parentWidget, const char *widgetName,
 *                       QObject *parent, const char *name,
 *                       const QStringList &args );</code>
 * </ul>
 * The args QStringList passed to the constructor is the args string list
 * that the caller passed to KLibFactory's create method.
 *
 * In addition upon instantiation this template provides a central
 * KInstance object for your component, accessible through the
 * static instance() method. The instanceName argument of the
 * KGenericFactory constructor is passed to the KInstance object.
 *
 * The creation of the KInstance object can be customized by inheriting
 * from this template class and re-implementing the virtual createInstance
 * method. For example it could look like this:
 * \code
 *     KInstance *MyFactory::createInstance()
 *     {
 *         return new KInstance( myAboutData );
 *     }
 * \endcode
 *
 * Example of usage of the whole template:
 * \code
 *     class MyPlugin : public KParts::Plugin
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyPlugin( QObject *parent, const char *name,
 *                   const QStringList &args );
 *         ...
 *     };
 *
 *     class MyDialogComponent : public KDialogBase
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyDialogComponent( QWidget *parentWidget, const char *name,
 *                            const QStringList &args );
 *         ...
 *     };
 *
 *     typedef K_TYPELIST_2( MyPlugin, MyDialogComponent ) Products;
 *     K_EXPORT_COMPONENT_FACTORY( libmyplugin, KGenericFactory&lt;Products&gt; )
 * \endcode
 */
template <class Product, class ProductListTail>
class KGenericFactory< KTypeList<Product, ProductListTail>, QObject >
    : public KLibFactory,
      public KGenericFactoryBase< KTypeList<Product, ProductListTail> >
{
public:
    KGenericFactory( const char *instanceName  = 0 )
        : KGenericFactoryBase< KTypeList<Product, ProductListTail> >( instanceName )
    {}

    /**
     * @since 3.3
	*/
    KGenericFactory( const KAboutData *data )
        : KGenericFactoryBase< KTypeList<Product, ProductListTail> >( data )
    {}


protected:
    virtual QObject *createObject( QObject *parent, const char *name,
                                   const char *className, const QStringList &args )
    {
        this->initializeMessageCatalog();
        return KDEPrivate::MultiFactory< KTypeList< Product, ProductListTail > >
            ::create( 0, 0, parent, name, className, args );
    }
};

/**
 * This template provides a generic implementation of a KLibFactory ,
 * for use with shared library components. It implements the pure virtual
 * createObject method of KLibFactory and instantiates objects of the
 * specified classes in the given typelist template argument when the class
 * name argument of createObject matches a class names in the given hierarchy
 * of classes.
 *
 * Note that each class in the specified in the typelist template argument
 * needs to provide a certain constructor:
 * <ul>
 *     <li>If the class is derived from QObject then it needs to have
 *         a constructor like:
 *         <code>MyClass( QObject *parent, const char *name,
 *                        const QStringList &args );</code>
 *     <li>If the class is derived from QWidget then it needs to have
 *         a constructor like:
 *         <code>MyWidget( QWidget *parent, const char *name,
 *                         const QStringList &args);</code>
 *     <li>If the class is derived from KParts::Part then it needs to have
 *         a constructor like:
 *         <code>MyPart( QWidget *parentWidget, const char *widgetName,
 *                       QObject *parent, const char *name,
 *                       const QStringList &args );</code>
 * </ul>
 * The args QStringList passed to the constructor is the args string list
 * that the caller passed to KLibFactory's create method.
 *
 * In addition upon instantiation this template provides a central
 * KInstance object for your component, accessible through the
 * static instance() method. The instanceName argument of the
 * KGenericFactory constructor is passed to the KInstance object.
 *
 * The creation of the KInstance object can be customized by inheriting
 * from this template class and re-implementing the virtual createInstance
 * method. For example it could look like this:
 * \code
 *     KInstance *MyFactory::createInstance()
 *     {
 *         return new KInstance( myAboutData );
 *     }
 * \endcode
 *
 * Example of usage of the whole template:
 * \code
 *     class MyPlugin : public KParts::Plugin
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyPlugin( QObject *parent, const char *name,
 *                   const QStringList &args );
 *         ...
 *     };
 *
 *     class MyDialogComponent : public KDialogBase
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyDialogComponent( QWidget *parentWidget, const char *name,
 *                            const QStringList &args );
 *         ...
 *     };
 *
 *     typedef K_TYPELIST_2( MyPlugin, MyDialogComponent ) Products;
 *     K_EXPORT_COMPONENT_FACTORY( libmyplugin, KGenericFactory&lt;Products&gt; )
 * \endcode
 */
template <class Product, class ProductListTail,
          class ParentType, class ParentTypeListTail>
class KGenericFactory< KTypeList<Product, ProductListTail>,
                       KTypeList<ParentType, ParentTypeListTail> >
    : public KLibFactory,
      public KGenericFactoryBase< KTypeList<Product, ProductListTail> >
{
public:
    KGenericFactory( const char *instanceName  = 0 )
        : KGenericFactoryBase< KTypeList<Product, ProductListTail> >( instanceName )
    {}
	/**
	* @since 3.3
	*/
    KGenericFactory( const KAboutData *data )
        : KGenericFactoryBase< KTypeList<Product, ProductListTail> >( data )
    {}


protected:
    virtual QObject *createObject( QObject *parent, const char *name,
                                   const char *className, const QStringList &args )
    {
        this->initializeMessageCatalogs();
        return KDEPrivate::MultiFactory< KTypeList< Product, ProductListTail >,
                                         KTypeList< ParentType, ParentTypeListTail > >
                                       ::create( 0, 0, parent, name,
                                                 className, args );
    }
};


/*
 * vim: et sw=4
 */

#endif

