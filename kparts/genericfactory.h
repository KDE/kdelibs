/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KPARTS_GENERICFACTORY_H
#define KPARTS_GENERICFACTORY_H

#include <kparts/factory.h>
#include <kparts/part.h>
#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kdebug.h>

namespace KParts
{

    /**
     * @internal
     */
    template <class T>
    class GenericFactoryBase : public KParts::Factory
    {
    public:
        GenericFactoryBase()
        {
            if ( s_self )
            {
                kWarning() << "KParts::GenericFactory instantiated more than once!";
            }
            s_self = this;
        }
        virtual ~GenericFactoryBase()
        {
            delete s_aboutData;
            delete s_componentData;
            s_aboutData = 0;
            s_componentData = 0;
            s_self = 0;
        }

        static const KComponentData &componentData();
        static KAboutData *aboutData();
        virtual KComponentData partComponentData()
        {
            return componentData();
        }


    protected:
        virtual KComponentData *createComponentData()
        {
            return new KComponentData(aboutData());
        }


    private:
        static GenericFactoryBase<T> *s_self;
        static KComponentData *s_componentData;
        static KAboutData *s_aboutData;
    };

    /**
     * A template for a KParts::Factory implementation. It implements the pure virtual
     * createPartObject method by instantiating the template argument when requested
     * through the className field. In addition it is a container for a part's KComponentData
     * object, by providing a static KComponentData componentData() method.
     *
     * The template argument has to inherit from KParts::Part and has to implement two methods:
     *  1) There needs to be a public constructor with the following signature:
     *         MyPart( QWidget *parentWidget, QObject *parent, const QStringList& args )
     *
     *  2) It needs to provide one static method to create a KAboutData object per
     *     request, holding information about the component's name, its authors, license, etc.
     *     The signature of that static method has to be
     *         KAboutData *createAboutData()
     *
     * The template will take care of memory management of the KComponentData and the KAboutData object,
     * meaning ownership of what createAboutData returns is passed to the caller (this template) .
     *
     * For advanced use you can also inherit from the template and re-implement additionally the
     * virtual KComponentData createComponentData() method, for example in case you want to extend the
     * paths of your instance's KStandardDirs object.
     *
     * If a KParts::ReadOnlyPart is requested through this factory and the template argument
     * implements a KParts::ReadWritePart then setReadWrite( false ) will automatically be
     * called in createPartObject.
     *
     * Use the factory through the K_EXPORT_COMPONENT_FACTORY macro, like that:
     * \code
     * typedef KParts::GenericFactory&lt;YourKPart&gt; YourKPartFactory;
     * K_EXPORT_COMPONENT_FACTORY( yourlibrary, YourKPartFactory )
     * \endcode
     * yourlibrary is the library name that you compiled your KPart into.
     */
    template <class T>
    class GenericFactory : public GenericFactoryBase<T>
    {
    public:
        GenericFactory() { }

        virtual KParts::Part *createPartObject( QWidget *parentWidget,
                                                QObject *parent,
                                                const char *className,
                                                const QStringList &args )
        {
            T *part = KDEPrivate::ConcreteFactory<T>::create( parentWidget,
                                                              parent,
                                                              className,
                                                              args );

            if ( part && !qstrcmp( className, "KParts::ReadOnlyPart" ) )
            {
                KParts::ReadWritePart *rwp = dynamic_cast<KParts::ReadWritePart *>( part );
                if ( rwp )
                    rwp->setReadWrite( false );
            }
            return part;
        }
    };

    template <class T1, class T2>
    class GenericFactory< KTypeList<T1, T2> > : public GenericFactoryBase<T1>
    {
    public:
        GenericFactory() { }

        virtual KParts::Part *createPartObject( QWidget *parentWidget,
                                                QObject *parent,
                                                const char *className,
                                                const QStringList &args )
        {
            QObject *object = KDEPrivate::MultiFactory< KTypeList<T1, T2> >::create( parentWidget,
                                                                                     parent,
                                                                                     className,
                                                                                     args );

            // (this cast is guaranteed to work...)
            KParts::Part *part = dynamic_cast<KParts::Part *>( object );

            if ( part && !qstrcmp( className, "KParts::ReadOnlyPart" ) )
            {
                KParts::ReadWritePart *rwp = dynamic_cast<KParts::ReadWritePart *>( part );
                if ( rwp )
                    rwp->setReadWrite( false );
            }
            return part;
        }
    };

    /**
     * @internal
     */
    template <class T>
    GenericFactoryBase<T> *GenericFactoryBase<T>::s_self = 0;

    /**
     * @internal
     */
    template <class T>
    KComponentData *GenericFactoryBase<T>::s_componentData = 0;

    /**
     * @internal
     */
    template <class T>
    KAboutData *GenericFactoryBase<T>::s_aboutData = 0;

    /**
     * @internal
     */
    template <class T>
    const KComponentData &GenericFactoryBase<T>::componentData()
    {
        if ( !s_componentData )
        {
            if ( s_self )
                s_componentData = s_self->createComponentData();
            else
                s_componentData = new KComponentData(aboutData());
        }
        return *s_componentData;
    }

    /**
     * @internal
     */
    template <class T>
    KAboutData *GenericFactoryBase<T>::aboutData()
    {
        if ( !s_aboutData )
            s_aboutData = T::createAboutData();
        return s_aboutData;
    }

}

#endif

/**
 * vim: et sw=4
 */
