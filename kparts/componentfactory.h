#ifndef __kparts_componentfactory_h__
#define __kparts_componentfactory_h__

#include <kparts/factory.h>
#include <ktrader.h>
#include <qmetaobject.h>

namespace KParts
{

    class ComponentFactory
    {
    public:
        enum ComponentLoadingError { ErrNoServiceFound = 1,
                                     ErrServiceProvidesNoLibrary,
                                     ErrNoFactory,
                                     ErrNoComponent };

        /**
         * This template function allows to ask the given factory to create an
         * instance of the given template argument.
         *
         * Example of usage:
         * <pre>
         *     MyPlugin *plugin = KParts::ComponentFactory::createInstance&lt;MyPlugin&gt;( factory, parent );
         * </pre>
         *
         * @param factory The factory to ask for the creation of the component
         */
        template <class T>
        static T *createInstanceFromFactory( KLibFactory *factory, QObject *parent = 0,
                                             const char *name = 0,
                                             const QStringList &args = QStringList() )
        {
            QObject *object = factory->create( parent, name, 
                                               T::staticMetaObject()->className(),
                                               args );

            T *result = dynamic_cast<T *>( object );
            if ( !result )
                    delete object;
            return result;
        }

        /**
         * This template function allows to ask the given kparts factory to create an
         * instance of the given template argument (a class) .
         *
         * Example of usage:
         * <pre>
         *     KViewPart *doc = KParts::ComponentFactory::createInstance&lt;KViewPart&gt;( factory, parent );
         * </pre>
         *
         * @param factory The factory to ask for the creation of the component
         */
        template <class T>
        static T *createPartInstanceFromFactory( KParts::Factory *factory, 
                                                 QWidget *parentWidget = 0,
                                                 const char *widgetName = 0, 
                                                 QObject *parent = 0,
                                                 const char *name = 0,
                                                 const QStringList &args = QStringList() )
        {
            KParts::Part *object = factory->createPart( parentWidget, widgetName,
                                                        parent, name, 
                                                        T::staticMetaObject()->className(),
                                                        args );

            T *result = dynamic_cast<T *>( object );
            if ( !result )
                delete object;
            return result;
        }

        /**
         * This template allows to load the specified library and ask the
         * factory to create an instance of the given template argument.
         *
         * @param libraryName The library do open
         */
        template <class T>
        static T *createInstanceFromLibrary( const char *libraryName, QObject *parent = 0, 
                                             const char *name = 0, 
                                             const QStringList &args = QStringList(),
                                             int *error = 0 )
        {
            KLibFactory *factory = KLibLoader::self()->factory( libraryName );
            if ( !factory )
            {
                if ( error )
                    *error = ErrNoFactory;
                return 0;
            }
            T *res = createInstanceFromFactory<T>( factory, parent, name, args );
            if ( !res && error )
                *error = ErrNoComponent;
            return res;
        }

        template <class T>
        static T *createPartInstanceFromLibrary( const char *libraryName, 
                                                 QWidget *parentWidget = 0, 
                                                 const char *widgetName = 0,
                                                 QObject *parent = 0, 
                                                 const char *name = 0, 
                                                 const QStringList &args = QStringList(),
                                                 int *error = 0 )
        {
            KLibFactory *factory = KLibLoader::self()->factory( libraryName );
            if ( !factory )
            {
                if ( error )
                    *error = ErrNoFactory;
                return 0;
            }
            KParts::Factory *partFactory = dynamic_cast<KParts::Factory *>( factory );
            if ( !partFactory )
            {
                if ( error )
                    *error = ErrNoFactory;
                return 0;
            }
            T *res = createPartInstanceFromFactory<T>( partFactory, parentWidget, 
                                                       widgetName, parent, name, args );
            if ( !res && error )
                *error = ErrNoComponent;
            return res;
        }

        template <class T>
        static T *createInstanceFromService( const KService::Ptr &service,
                                             QObject *parent = 0,
                                             const char *name = 0,
                                             const QStringList &args = QStringList(),
                                             int *error = 0 )
        {
            QString library = service->library();
            if ( library.isEmpty() )
            {
                if ( error )
                    *error = ErrServiceProvidesNoLibrary;
                return 0;
            }

            return createInstanceFromLibrary<T>( library.local8Bit(), parent, name, 
                                                 args, error );
        }

        template <class T>
        static T *createPartInstanceFromService( const KService::Ptr &service,
                                                 QWidget *parentWidget = 0,
                                                 const char *widgetName = 0,
                                                 QObject *parent = 0,
                                                 const char *name = 0,
                                                 const QStringList &args = QStringList(),
                                                 int *error = 0 )
        {
            QString library = service->library();
            if ( library.isEmpty() )
            {
                if ( error )
                    *error = ErrServiceProvidesNoLibrary;
                return 0;
            }

            return createPartInstanceFromLibrary<T>( library.local8Bit(), parent, name, 
                                                     args, error );
        }
 
        template <class T>
        static T *createInstanceFromQuery( const QString &serviceType,
                                           const QString &constraint = QString::null,
                                           QObject *parent = 0,
                                           const char *name = 0,
                                           const QStringList &args = QStringList(),
                                           int *error = 0 )
        {
            KTrader::OfferList offers = KTrader::self()->query( serviceType, constraint );
            if ( offers.isEmpty() )
            {
                if ( error )
                    *error = ErrNoServiceFound;
                return 0;
            }

            KTrader::OfferList::ConstIterator it = offers.begin();
            for (; it != offers.end(); ++it )
            {
                KService::Ptr service = *it;

                if ( error )
                    *error = 0;

                T *component =  createInstanceFromService<T>( service, parent, name,
                                                              args, error );
                if ( component )
                    return component;
            }

            if ( error )
                *error = ErrNoServiceFound;

            return 0;
        }

        template <class T>
        static T *createPartInstanceFromQuery( const QString &serviceType,
                                               const QString &constraint,
                                               QWidget *parentWidget = 0,
                                               const char *widgetName = 0,
                                               QObject *parent = 0,
                                               const char *name = 0,
                                               const QStringList &args = QStringList(),
                                               int *error = 0 )
        {
            KTrader::OfferList offers = KTrader::self()->query( serviceType, constraint );
            if ( offers.isEmpty() )
            {
                if ( error )
                    *error = ErrNoServiceFound;
                return 0;
            }

            KTrader::OfferList::ConstIterator it = offers.begin();
            for (; it != offers.end(); ++it )
            {
                KService::Ptr service = *it;

                if ( error )
                    *error = 0;

                T *component =  createPartInstanceFromService<T>( service, parentWidget, 
                                                                  widgetName, parent, name,
                                                                  args, error );
                if ( component )
                    return component;
            }

            if ( error )
                *error = ErrNoServiceFound;

            return 0;
        }
 
    };

};

/*
 * vim: et sw=4
 */

#endif

