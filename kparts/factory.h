/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __kparts_factory_h__
#define __kparts_factory_h__

#include <klibloader.h>
#include <qmetaobject.h>
#include <kdebug.h>
#include <kparts/part.h>

class QWidget;
class KAboutData;
class KInstance;

namespace KParts
{

class Part;

/**
 * A generic factory object to create a Part.
 *
 * Factory is an abstract class. Reimplement the @ref
 * createPartObject() method to give it functionality.
 *
 * @see KLibFactory.
 */
class Factory : public KLibFactory
{
  Q_OBJECT
public:
  Factory( QObject *parent = 0, const char *name = 0 );
  virtual ~Factory();

    /**
     * Creates a part.
     *
     * The QStringList can be used to pass additional arguments to the part.
     * If the part needs additional arguments, it should take them as
     * name="value" pairs. This is the way additional arguments will get passed
     * to the part from eg. khtml. You can for example emebed the part into HTML
     * by using the following code:
     * <pre>
     *    <object type="my_mimetype" data="url_to_my_data">
     *        <param name="name1" value="value1">
     *        <param name="name2" value="value2">
     *    </object>
     * </pre>
     * This could result in a call to
     * <pre>
     *     createPart( parentWidget, name, parentObject, parentName, "Kparts::Part",
     *                 QStringList("name1="value1"", "name2="value2") );
     * </pre>
     *
     * @returns the newly created part.
     *
     * createPart() automatically emits a signal @ref KLibFactory::objectCreated to tell
     * the library about its newly created object.  This is very
     * important for reference counting, and allows unloading the
     * library automatically once all its objects have been destroyed.
     */
     Part *createPart( QWidget *parentWidget = 0, const char *widgetName = 0, QObject *parent = 0, const char *name = 0, const char *classname = "KParts::Part", const QStringList &args = QStringList() );

     /**
      * This template function allows you to load a library and ask the factory (a KParts::Factory) to
      * create an instance of the given template argument (a part) , in a type-safe manner. 
      *
      * @returns an instance of the class given as template argument or a null pointer if 
      * the factory exported by the library does support creating objects of the given type.
      *
      * Example of usage:
      * <pre>
      *     KoDocument *document = KParts::Factory::create&lt;KoDocument&gt;("libkword" );
      * </pre>
      */
     template <class T>
     static T *createPart( const char *libraryName, QWidget *parentWidget = 0, const char *widgetName = 0,
                           QObject *parent = 0, const char *name = 0, 
                           const QStringList &args = QStringList() )
     {
	 KLibFactory *factory = KLibLoader::self()->factory( libraryName );
	 if ( !factory )
	     return 0;

         KParts::Factory *partFactory = dynamic_cast<KParts::Factory *>( factory );
         if ( !partFactory )
             return 0;

         KParts::Part *part = partFactory->createPart( parentWidget, widgetName,
                                                       parent, name, T::staticMetaObject()->className(),
		                                       args );
	 T *res = dynamic_cast<T *>( part );
	 if ( !res )
	     delete part;
	 return res;
     }


protected:

    /**
     * Reimplement this method in your implementation to create the Part.
     *
     * The QStringList can be used to pass additional arguments to the part.
     * If the part needs additional arguments, it should take them as
     * name="value" pairs. This is the way additional arguments will get passed
     * to the part from eg. khtml. You can for example emebed the part into HTML
     * by using the following code:
     * <pre>
     *    <object type="my_mimetype" data="url_to_my_data">
     *        <param name="name1" value="value1">
     *        <param name="name2" value="value2">
     *    </object>
     * </pre>
     * This could result in a call to
     * <pre>
     *     createPart( parentWidget, name, parentObject, parentName, "Kparts::Part",
     *                 QStringList("name1="value1"", "name2="value2") );
     * </pre>
     *
     * @returns the newly created part.
     */
    virtual Part *createPartObject( QWidget *parentWidget = 0, const char *widgetName = 0, QObject *parent = 0, const char *name = 0, const char *classname = "KParts::Part", const QStringList &args = QStringList() ) = 0;
    
    /**
     * Reimplemented from KLibFactory. Calls @ref createPart()
     */
    virtual QObject *createObject( QObject *parent = 0, const char *name = 0, const char *classname = "QObject", const QStringList &args = QStringList() );
};

/**
 * A template for a KParts::Factory implementation. It implements the pure virtual
 * createPartObject method by instantiating the template argument when requested
 * through the className field. In addition it is a container for a parts @ref KInstance
 * object, by providing a static KInstance *instance() method.
 *
 * The template argument has to inherit from KParts::Part and has to implement two methods:
 *  1) There needs to be a public constructor with the following signature:
 *         MyPart( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name )
 *
 *  2) It needs to provide one static method to create an @ref KAboutData object per
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
 */
template <class T>
class GenericFactory : public Factory
{
public:
    GenericFactory() 
    {
        if ( s_self )
            kdWarning() << "KParts::GenericFactory instantiated more than once!" << endl;
        s_self = this;
    }
    virtual ~GenericFactory()
    {
        delete s_aboutData;
        delete s_instance;
        s_aboutData = 0;
        s_instance = 0;
        s_self = 0;
    }

    virtual KParts::Part *createPartObject( QWidget *parentWidget, const char *widgetName,
	                                    QObject *parent, const char *name,
					    const char *className,
					    const QStringList & )
    {
        QMetaObject *metaObject = T::staticMetaObject();
        while ( metaObject )
        {
            if ( !qstrcmp( className, metaObject->className() ) )
            {
                T *part = new T( parentWidget, widgetName, parent, name );
                if ( !qstrcmp( className, "KParts::ReadOnlyPart" ) )
                {
                    KParts::ReadWritePart *rwp = dynamic_cast<KParts::ReadWritePart *>( part );
                    if ( rwp )
                        rwp->setReadWrite( false );
                }
                return part;    
            }
            metaObject = metaObject->superClass();
        }
        return 0;
    }

    static KInstance *instance();
    static KAboutData *aboutData();

protected:
    virtual KInstance *createInstance()
    {
        return new KInstance( aboutData() );
    }

private:
    static GenericFactory<T> *s_self;
    static KInstance *s_instance;
    static KAboutData *s_aboutData;
};

template <class T>
GenericFactory<T> *GenericFactory<T>::s_self = 0;

template <class T>
KInstance *GenericFactory<T>::s_instance = 0;

template <class T>
KAboutData *GenericFactory<T>::s_aboutData = 0;

template <class T>
KInstance *GenericFactory<T>::instance()
{
    if ( !s_instance && s_self )
        s_instance = s_self->createInstance();
    return s_instance;
}

template <class T>
KAboutData *GenericFactory<T>::aboutData()
{
    if ( !s_aboutData )
        s_aboutData = T::createAboutData();
    return s_aboutData;
}

};

/*
 * vim: et sw=4
 */

#endif
