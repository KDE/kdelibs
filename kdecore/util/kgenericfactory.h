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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef kgenericfactory_h
#define kgenericfactory_h

#include <klibloader.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ktypelist.h>
#include <kcomponentdata.h>
#include <kgenericfactory.tcc>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

/* @internal */
template <class T>
class KGenericFactoryBase : public KPluginFactory
{
public:
    explicit KGenericFactoryBase(const char *componentName, const char *catalogName)
        : KPluginFactory(componentName, catalogName)
    {
        s_self = this;
        s_createComponentDataCalled = false;
    }

    explicit KGenericFactoryBase( const KAboutData *data )
        : KPluginFactory(data)
    {
        s_self = this;
        s_createComponentDataCalled = false;
    }

    virtual ~KGenericFactoryBase()
    {
        s_self = 0;
    }

    static KComponentData componentData()
    {
        Q_ASSERT(s_self);
        if (!s_createComponentDataCalled) {
            s_createComponentDataCalled = true;

            KComponentData *kcd = s_self->createComponentData();
            Q_ASSERT(kcd);
            s_self->setComponentData(*kcd);
            delete kcd;
        }
        return static_cast<KPluginFactory *>(s_self)->componentData();
    }

protected:
    virtual KComponentData *createComponentData()
    {
        return new KComponentData(componentData());
    }

private:
    static bool s_createComponentDataCalled;
    static KGenericFactoryBase<T> *s_self;
};

/* @internal */
template <class T>
KGenericFactoryBase<T> *KGenericFactoryBase<T>::s_self = 0;

/* @internal */
template <class T>
bool KGenericFactoryBase<T>::s_createComponentDataCalled = false;

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
 *         <code>MyClass( QObject *parent,
 *                        const QStringList &args );</code>
 *     <li>If the class is derived from QWidget then it needs to have
 *         a constructor like:
 *         <code>MyWidget( QWidget *parent,
 *                         const QStringList &args);</code>
 *     <li>If the class is derived from KParts::Part then it needs to have
 *         a constructor like:
 *         <code>MyPart( QWidget *parentWidget,
 *                       QObject *parent,
 *                       const QStringList &args );</code>
 * </ul>
 * The args QStringList passed to the constructor is the args string list
 * that the caller passed to KLibFactory's create method.
 *
 * In addition upon instantiation this template provides a central
 * KComponentData object for your component, accessible through the
 * static componentData() method. The componentName and catalogName arguments
 * of the KGenericFactory constructor are passed to the KComponentData object.
 *
 * The creation of the KComponentData object can be customized by inheriting
 * from this template class and re-implementing the virtual createComponentData
 * method. For example it could look like this:
 * \code
 *     KComponentData *MyFactory::createComponentData()
 *     {
 *         return new KComponentData( myAboutData );
 *     }
 * \endcode
 *
 * Example of usage of the whole template:
 * \code
 *     class MyPlugin : public KParts::Plugin
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyPlugin( QObject *parent, const QStringList &args );
 *         ...
 *     };
 *
 *     K_EXPORT_COMPONENT_FACTORY( libmyplugin, KGenericFactory<MyPlugin> )
 * \endcode
 */
template <class Product, class ParentType = QObject>
class KDE_DEPRECATED KGenericFactory : public KGenericFactoryBase<Product>
{
public:
    explicit KGenericFactory( const char *componentName = 0, const char *catalogName = 0 )
        : KGenericFactoryBase<Product>(componentName, catalogName)
    {}

    explicit KGenericFactory( const KAboutData *data )
        : KGenericFactoryBase<Product>(data)
    {}

protected:
    virtual QObject *createObject( QObject *parent,
                                   const char *className, const QStringList &args )
    {
        return KDEPrivate::ConcreteFactory<Product, ParentType>
            ::create( 0, parent, className, args );
    }
};

/**
 * \class KGenericFactory kgenericfactory.h <KGenericFactory>
 * 
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
 *         <code>MyClass( QObject *parent,
 *                        const QStringList &args );</code>
 *     <li>If the class is derived from QWidget then it needs to have
 *         a constructor like:
 *         <code>MyWidget( QWidget *parent,
 *                         const QStringList &args);</code>
 *     <li>If the class is derived from KParts::Part then it needs to have
 *         a constructor like:
 *         <code>MyPart( QWidget *parentWidget,
 *                       QObject *parent,
 *                       const QStringList &args );</code>
 * </ul>
 * The args QStringList passed to the constructor is the args string list
 * that the caller passed to KLibFactory's create method.
 *
 * In addition upon instantiation this template provides a central
 * KComponentData object for your component, accessible through the
 * static componentData() method. The componentName and catalogName arguments
 * of the KGenericFactory constructor are passed to the KComponentData object.
 *
 * The creation of the KComponentData object can be customized by inheriting
 * from this template class and re-implementing the virtual createComponentData
 * method. For example it could look like this:
 * \code
 *     KComponentData *MyFactory::createComponentData()
 *     {
 *         return new KComponentData( myAboutData );
 *     }
 * \endcode
 *
 * Example of usage of the whole template:
 * \code
 *     class MyPlugin : public KParts::Plugin
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyPlugin( QObject *parent,
 *                   const QStringList &args );
 *         ...
 *     };
 *
 *     class MyDialogComponent : public KDialog
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyDialogComponent( QWidget *parentWidget,
 *                            const QStringList &args );
 *         ...
 *     };
 *
 *     typedef K_TYPELIST_2( MyPlugin, MyDialogComponent ) Products;
 *     K_EXPORT_COMPONENT_FACTORY( libmyplugin, KGenericFactory<Products> )
 * \endcode
 */
template <class Product, class ProductListTail>
class KGenericFactory< KTypeList<Product, ProductListTail>, QObject >
    : public KGenericFactoryBase<KTypeList<Product, ProductListTail> >
{
public:
    explicit KGenericFactory( const char *componentName  = 0, const char *catalogName  = 0 )
        : KGenericFactoryBase<KTypeList<Product, ProductListTail> >(componentName, catalogName)
    {}

    explicit KGenericFactory( const KAboutData *data )
        : KGenericFactoryBase<KTypeList<Product, ProductListTail> >(data)
    {}


protected:
    virtual QObject *createObject( QObject *parent,
                                   const char *className, const QStringList &args )
    {
        return KDEPrivate::MultiFactory< KTypeList< Product, ProductListTail > >
            ::create( 0, parent, className, args );
    }
};

/**
 * \class KGenericFactory kgenericfactory.h <KGenericFactory>
 * 
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
 *         <code>MyClass( QObject *parent,
 *                        const QStringList &args );</code>
 *     <li>If the class is derived from QWidget then it needs to have
 *         a constructor like:
 *         <code>MyWidget( QWidget *parent,
 *                         const QStringList &args);</code>
 *     <li>If the class is derived from KParts::Part then it needs to have
 *         a constructor like:
 *         <code>MyPart( QWidget *parentWidget,
 *                       QObject *parent,
 *                       const QStringList &args );</code>
 * </ul>
 * The args QStringList passed to the constructor is the args string list
 * that the caller passed to KLibFactory's create method.
 *
 * In addition upon instantiation this template provides a central
 * KComponentData object for your component, accessible through the
 * static componentData() method. The componentName and catalogNames arguments
 * of the KGenericFactory constructor are passed to the KComponentData object.
 *
 * The creation of the KComponentData object can be customized by inheriting
 * from this template class and re-implementing the virtual createComponentData
 * method. For example it could look like this:
 * \code
 *     KComponentData *MyFactory::createComponentData()
 *     {
 *         return new KComponentData( myAboutData );
 *     }
 * \endcode
 *
 * Example of usage of the whole template:
 * \code
 *     class MyPlugin : public KParts::Plugin
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyPlugin( QObject *parent,
 *                   const QStringList &args );
 *         ...
 *     };
 *
 *     class MyDialogComponent : public KDialog
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyDialogComponent( QWidget *parentWidget,
 *                            const QStringList &args );
 *         ...
 *     };
 *
 *     typedef K_TYPELIST_2( MyPlugin, MyDialogComponent ) Products;
 *     K_EXPORT_COMPONENT_FACTORY( libmyplugin, KGenericFactory<Products> )
 * \endcode
 */
template <class Product, class ProductListTail,
          class ParentType, class ParentTypeListTail>
class KGenericFactory< KTypeList<Product, ProductListTail>,
                       KTypeList<ParentType, ParentTypeListTail> >
    : public KGenericFactoryBase<KTypeList<Product, ProductListTail> >
{
public:
    explicit KGenericFactory( const char *componentName  = 0, const char *catalogName  = 0 )
        : KGenericFactoryBase<KTypeList<Product, ProductListTail> >(componentName, catalogName)
    {}
    explicit KGenericFactory( const KAboutData *data )
        : KGenericFactoryBase<KTypeList<Product, ProductListTail> >(data)
    {}


protected:
    virtual QObject *createObject( QObject *parent,
                                   const char *className, const QStringList &args )
    {
        return KDEPrivate::MultiFactory< KTypeList< Product, ProductListTail >,
                                         KTypeList< ParentType, ParentTypeListTail > >
                                       ::create( 0, 0, parent,
                                                 className, args );
    }
};

/*
 * vim: et sw=4
 */

#endif


