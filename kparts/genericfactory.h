#ifndef __kparts__genericfactory_h__
#define __kparts__genericfactory_h__

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
                kdWarning() << "KParts::GenericFactory instantiated more than once!" << endl;
            s_self = this;
        }
        virtual ~GenericFactoryBase()
        {
            delete s_aboutData;
            delete s_instance;
            s_aboutData = 0;
            s_instance = 0;
            s_self = 0;
        }

        static KInstance *instance();
        static KAboutData *aboutData();

    protected:
        virtual KInstance *createInstance()
        {
            return new KInstance( aboutData() );
        }

    private:
        static GenericFactoryBase<T> *s_self;
        static KInstance *s_instance;
        static KAboutData *s_aboutData;
    };

    /**
     * A template for a KParts::Factory implementation. It implements the pure virtual
     * createPartObject method by instantiating the template argument when requested
     * through the className field. In addition it is a container for a part's @ref KInstance
     * object, by providing a static KInstance *instance() method.
     *
     * The template argument has to inherit from KParts::Part and has to implement two methods:
     *  1) There needs to be a public constructor with the following signature:
     *         MyPart( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const QStringList& args )
     *
     *  2) It needs to provide one static method to create a @ref KAboutData object per
     *     request, holding information about the component's name, its authors, license, etc.
     *     The signature of that static method has to be
     *         KAboutData *createAboutData()
     *
     * The template will take care of memory management of the KInstance and the KAboutData object.
     *
     * For advanced use you can also inherit from the template and re-implement additionally the
     * virtual KInstance *createInstance() method, for example in case you want to extend the
     * paths of your instance's KStandardDirs object.
     *
     * If a KParts::ReadOnlyPart is requested through this factory and the template argument
     * implements a KParts::ReadWritePart then setReadWrite( false ) will automatically be
     * called in createPartObject.
     *
     * Use the factory through the K_EXPORT_COMPONENT_FACTORY macro, like that:
     * <pre>
     * typedef KParts::GenericFactory&lt;YourKPart&gt; YourKPartFactory;
     * K_EXPORT_COMPONENT_FACTORY( yourlibrary, YourKPartFactory );
     * </pre>
     * yourlibrary is the library name that you compiled your KPart into.
     */
    template <class T>
    class GenericFactory : public GenericFactoryBase<T>
    {
    public:
        GenericFactory() { }

        virtual KParts::Part *createPartObject( QWidget *parentWidget, const char *widgetName,
                                                QObject *parent, const char *name,
                                                const char *className,
                                                const QStringList &args )
        {
            T *part = KDEPrivate::ConcreteFactory<T>::create( parentWidget,
                                                              widgetName,
                                                              parent,
                                                              name,
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

        virtual KParts::Part *createPartObject( QWidget *parentWidget, const char *widgetName,
                                                QObject *parent, const char *name,
                                                const char *className,
                                                const QStringList &args )
        {
            QObject *object = KDEPrivate::MultiFactory< KTypeList<T1, T2> >::create( parentWidget,
                                                                                     widgetName,
                                                                                     parent, name,
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
    KInstance *GenericFactoryBase<T>::s_instance = 0;

    /**
     * @internal
     */
    template <class T>
    KAboutData *GenericFactoryBase<T>::s_aboutData = 0;

    /**
     * @internal
     */
    template <class T>
    KInstance *GenericFactoryBase<T>::instance()
    {
        if ( !s_instance )
        {
            if ( s_self )
                s_instance = s_self->createInstance();
            else
                s_instance = new KInstance( aboutData() );
        }
        return s_instance;
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

};

#endif

/**
 * vim: et sw=4
 */
