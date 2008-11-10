/*
 * The Type2Type template and the Inheritance Detector are from
 * <http://www.cuj.com/experts/1810/alexandr.htm>
 * (c) Andrei Alexandrescu <andrei@metalanguage.com> and
 * free for any use.
 *
 * The rest is:
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
// -*- mode: c++ -*-
//
//  W A R N I N G
//  -------------
//
// This file is not part of the KDE API.  It exists for the convenience
// of KGenericFactory. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef KGENERICFACTORY_TCC
#define KGENERICFACTORY_TCC

#include <QtCore/QMetaObject>
#include <ktypelist.h>
#include "kdebug.h"

namespace KParts
{
    class Part;
}

namespace KDEPrivate
{
    template <class Base>
    struct InheritanceDetector
    {
        typedef char ConversionExists;
        struct ConversionDoesNotExist { char bleh[ 2 ]; };
        static ConversionExists test( Base * );
        static ConversionDoesNotExist test( ... );
    };

    /* Simon: KCC doesn't eat the generic InheritanceDetector<Base>.
       Instead we have to use concrete specializations :-(

    template <class Base, class Derived>
    struct InheritanceTest
    {
        typedef Derived * DerivedPtr;
        enum { Result = sizeof( InheritanceDetector<Base>::test( DerivedPtr() ) ) ==
                        sizeof( InheritanceDetector<Base>::ConversionExists ) };
    };
    */

    template <class Derived>
    struct QWidgetInheritanceTest
    {
        typedef Derived * DerivedPtr;
        enum { Result = sizeof( InheritanceDetector<QWidget>::test( DerivedPtr() ) ) ==
                        sizeof( InheritanceDetector<QWidget>::ConversionExists ) };
    };

    template <class Derived>
    struct PartInheritanceTest
    {
        typedef Derived * DerivedPtr;
        enum { Result = sizeof( InheritanceDetector<KParts::Part>::test( DerivedPtr() ) ) ==
                        sizeof( InheritanceDetector<KParts::Part>::ConversionExists ) };
    };


    template <bool condition, typename Then, typename Else>
    struct If
    {
        typedef Else Result;
    };

    template <typename Then, typename Else>
    struct If<true, Then, Else>
    {
        typedef Then Result;
    };

    // a small helper template, to ease the overloading done in ConcreteFactory
    // to choose the right constructor for the given class.
    template <class T>
    struct Type2Type
    {
        typedef T OriginalType;
    };


    // this template is called from the MultiFactory one. It instantiates
    // the given class if the className matches. Instantiating is done by
    // calling the right constructor (a parentwidget/parent
    // one for Parts, a parentwidget one for widgets and last
    // but not least the standard default constructor with a parent .
    // the choice of the right constructor is done using an ordered inheritance
    // test.
    template <class Product, class ParentType = QObject>
    class ConcreteFactory
    {
    public:
        typedef typename If< PartInheritanceTest< Product >::Result,
                             KParts::Part,
                             typename If< QWidgetInheritanceTest< Product >::Result,
                                          QWidget, QObject >::Result >::Result BaseType;

        static inline Product *create( QWidget *parentWidget,
                                       QObject *parent,
                                       const char *className, const QStringList &args )
        {
            const QMetaObject* metaObject = &Product::staticMetaObject;
            while ( metaObject )
            {
                //kDebug(150) << "className=" << className << " metaObject->className()=" << metaObject->className() << endl;
                if ( !qstrcmp( className, metaObject->className() ) )
                    return create( parentWidget,
                                   parent, args, Type2Type<BaseType>() );
                metaObject = metaObject->superClass();
            }
            //kDebug(150) << "error, returning 0" << endl;
            return 0;
        }
    private:
        typedef typename If< QWidgetInheritanceTest<ParentType>::Result,
                             ParentType, QWidget >::Result WidgetParentType;

        static inline Product *create( QWidget *parentWidget,
                                       QObject *parent,
                                       const QStringList &args, Type2Type<KParts::Part> )
        {
            //kDebug(150) << "create - 1" << endl;
            return new Product( parentWidget, parent, args );
        }

        static inline Product *create( QWidget* /*parentWidget*/,
                                       QObject *parent,
                                       const QStringList &args, Type2Type<QWidget> )
        {
            //kDebug(150) << "create - 2" << endl;
            WidgetParentType *p = dynamic_cast<WidgetParentType *>( parent );
            if ( parent && !p )
                return 0;
            return new Product( p, args );
        }

        static inline Product *create( QWidget* /*parentWidget*/,
                                       QObject *parent,
                                       const QStringList &args, Type2Type<QObject> )
        {
            //kDebug(150) << "create - 3" << endl;
            ParentType *p = dynamic_cast<ParentType *>( parent );
            if ( parent && !p )
                return 0;
            return new Product( p, args );
        }
    };

    // this template is used to iterate through the typelist and call the
    // concrete factory for each type. the specializations of this template
    // are the ones actually being responsible for iterating, in fact.
    template <class Product, class ParentType = QObject>
    class MultiFactory
    {
    public:
        inline static QObject *create( QWidget *parentWidget,
                                       QObject *parent,
                                       const char *className,
                                       const QStringList &args )
        {
            return ConcreteFactory<Product, ParentType>::create( parentWidget,
                                                                 parent, className,
                                                                 args );
        }

    };

    // this specialized template we 'reach' at the end of a typelist
    // (the last item in a typelist is the NullType)
    template <>
    class MultiFactory<KDE::NullType>
    {
    public:
        inline static QObject *create( QWidget *, QObject *,
                                       const char *,
                                       const QStringList & )
        { return 0; }
    };

     // this specialized template we 'reach' at the end of a typelist
    // (the last item in a typelist is the NullType)
    template <>
    class MultiFactory<KDE::NullType, KDE::NullType>
    {
    public:
        inline static QObject *create( QWidget *, QObject *,
                                       const char *,
                                       const QStringList & )
        { return 0; }
    };

    template <class Product, class ProductListTail>
    class MultiFactory< KTypeList<Product, ProductListTail>, QObject >
    {
    public:
        inline static QObject *create( QWidget *parentWidget,
                                       QObject *parent,
                                       const char *className,
                                       const QStringList &args )
        {
            // try with the head of the typelist first. the head is always
            // a concrete type.
            QObject *object = MultiFactory<Product>::create( parentWidget,
                                                             parent, className,
                                                             args );

            if ( !object )
                object = MultiFactory<ProductListTail>::create( parentWidget,
                                                                parent, className,
                                                                args );

            return object;
        }
    };

    template <class Product, class ProductListTail,
              class ParentType, class ParentTypeListTail>
    class MultiFactory< KTypeList<Product, ProductListTail>,
                        KTypeList<ParentType, ParentTypeListTail> >
    {
    public:
        inline static QObject *create( QWidget *parentWidget,
                                       QObject *parent,
                                       const char *className,
                                       const QStringList &args )
        {
            // try with the head of the typelist first. the head is always
            // a concrete type.
            QObject *object = MultiFactory<Product, ParentType>
                                  ::create( parentWidget,
                                            parent, className, args );

            // if that failed continue by advancing the typelist, calling this
            // template specialization recursively (with T2 being a typelist) .
            // at the end we reach the nulltype specialization.
            if ( !object )
                object = MultiFactory<ProductListTail, ParentTypeListTail>
                             ::create( parentWidget,
                                       parent, className, args );

            return object;
        }
    };
}

#endif

/*
 * vim: et sw=4
 */
