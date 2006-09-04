#ifndef __kparts_componentfactory_h__
#define __kparts_componentfactory_h__

#include <kparts/factory.h>
#include <kparts/part.h>
#include <kservicetypetrader.h>
#include <klibloader.h>
#include <kmimetypetrader.h>

namespace KParts
{
    class ComponentFactory
    {
    public:
        /**
         * This template function allows to ask the given kparts factory to create an
         * instance of the given template type.
         *
         * Example of usage:
         * \code
         *     KViewPart *doc = KParts::ComponentFactory::createPartInstanceFromFactory&lt;KViewPart&gt;( factory, parent );
         * \endcode
         *
         * @param factory The factory to ask for the creation of the component
         * @param parentWidget the parent widget for the part
         * @param parent The parent object (see QObject constructor)
         * @param args A list of string arguments, passed to the factory and possibly
         *             to the component (see KLibFactory)
         * @return A pointer to the newly created object or a null pointer if the
         *         factory was unable to create an object of the given type.
         */
        template <class T>
        static T *createPartInstanceFromFactory( KParts::Factory *factory,
                                                 QWidget *parentWidget = 0,
                                                 QObject *parent = 0,
                                                 const QStringList &args = QStringList() )
        {
            KParts::Part *object = factory->createPart( parentWidget,
                                                        parent,
                                                        T::staticMetaObject.className(),
                                                        args );

            T *result = dynamic_cast<T *>( object );
            if ( !result )
                delete object;
            return result;
        }

        template <class T>
        static T *createPartInstanceFromLibrary( const char *libraryName,
                                                 QWidget *parentWidget = 0,
                                                 QObject *parent = 0,
                                                 const QStringList &args = QStringList(),
                                                 int *error = 0 )
        {
            KLibrary *library = KLibLoader::self()->library( libraryName );
            if ( !library )
            {
                if ( error )
                    *error = KLibLoader::ErrNoLibrary;
                return 0;
            }
            KLibFactory *factory = library->factory();
            if ( !factory )
            {
                library->unload();
                if ( error )
                    *error = KLibLoader::ErrNoFactory;
                return 0;
            }
            KParts::Factory *partFactory = dynamic_cast<KParts::Factory *>( factory );
            if ( !partFactory )
            {
                library->unload();
                if ( error )
                    *error = KLibLoader::ErrNoFactory;
                return 0;
            }
            T *res = createPartInstanceFromFactory<T>( partFactory, parentWidget,
                                                       parent, args );
            if ( !res )
            {
                library->unload();
                if ( error )
                    *error = KLibLoader::ErrNoComponent;
            }
            return res;
        }

        template <class T>
        static T *createPartInstanceFromService( const KService::Ptr &service,
                                                 QWidget *parentWidget = 0,
                                                 QObject *parent = 0,
                                                 const QStringList &args = QStringList(),
                                                 int *error = 0 )
        {
            QString library = service->library();
            if ( library.isEmpty() )
            {
                if ( error )
                    *error = KLibLoader::ErrServiceProvidesNoLibrary;
                return 0;
            }

            return createPartInstanceFromLibrary<T>( library.toLocal8Bit().data(), parentWidget,
                                                     parent, args, error );
        }

        template <class T, class ServiceIterator>
        static T *createPartInstanceFromServices( ServiceIterator begin,
                                                  ServiceIterator end,
                                                  QWidget *parentWidget = 0,
                                                  QObject *parent = 0,
                                                  const QStringList &args = QStringList(),
                                                  int *error = 0 )
         {
            for (; begin != end; ++begin )
            {
                KService::Ptr service = *begin;

                if ( error )
                    *error = 0;

                T *component = createPartInstanceFromService<T>( service, parentWidget,
                                                                 parent, args, error );
                if ( component )
                    return component;
            }

            if ( error )
                *error = KLibLoader::ErrNoServiceFound;

            return 0;

        }

        /**
         * This method creates and returns a KParts part from a serviceType (e.g. a mimetype).
         *
         * You can use this method to create a generic viewer - that can display any
         * kind of file, provided that there is a ReadOnlyPart installed for it - in 5 lines:
         * \code
         * // Given the following: KUrl url, QWidget* parentWidget and QObject* parentObject.
         * QString mimetype = KMimeType::findByURL( url )->name();
         * KParts::ReadOnlyPart* part = KParts::ComponentFactory::createPartInstanceFromQuery<KParts::ReadOnlyPart>( mimetype, QString(), parentWidget, parentObject );
         * if ( part ) {
         *     part->openUrl( url );
         *     part->widget()->show();  // also insert the widget into a layout, or simply use a KVBox as parentWidget
         * }
         * \endcode
         *
         * @param mimeType the mimetype which this part is associated with
         * @param constraint an optional constraint to pass to the trader (see KTrader)
         * @param parentWidget the parent widget, will be set as the parent of the part's widget
         * @param parent the parent object for the part itself
         * @param args A list of string arguments, passed to the factory and possibly
         *             to the component (see KLibFactory)
         * @param error The int passed here will receive an error code in case of errors.
         *              (See enum KLibLoader::ComponentLoadingError)
         * @return A pointer to the newly created object or a null pointer if the
         *         factory was unable to create an object of the given type.
         */
        template <class T>
        static T *createPartInstanceFromQuery( const QString &mimeType,
                                               const QString &constraint,
                                               QWidget *parentWidget = 0,
                                               QObject *parent = 0,
                                               const QStringList &args = QStringList(),
                                               int *error = 0 )
        {
            const KService::List offers = KMimeTypeTrader::self()->query( mimeType, QLatin1String("KParts/ReadOnlyPart"), constraint );
            if ( offers.isEmpty() )
            {
                if ( error )
                    *error = KLibLoader::ErrNoServiceFound;
                return 0;
            }

            return createPartInstanceFromServices<T>( offers.begin(), offers.end(),
                                                      parentWidget,
                                                      parent, args, error );
        }

    };
}

/*
 * vim: et sw=4
 */

#endif

