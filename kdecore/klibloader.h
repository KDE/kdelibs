/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KLIBLOADER_H
#define KLIBLOADER_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qasciidict.h>
#include <qlist.h>

#include "ltdl.h"

class KInstance;
class QTimer;
class KLibFactoryPrivate;
class KLibLoaderPrivate;
class KLibraryPrivate;

/**
 * If you develop a library that is to be loaded dynamically at runtime, then
 * you should provide a function that returns a pointer to your factory like this:
 * <pre>
 * extern "C"
 * {
 *   void* init_libkspread()
 *   {
 *     return new KSpreadFactory;
 *   }
 * };
 * </pre>
 * You should especially see that the function must follow the naming pattern
 * "init_libname".
 *
 * In the constructor of your factory you should create an instance of @ref KInstance
 * like this:
 * <pre>
 *     s_global = new KInstance( "kspread" );
 * </pre>
 * This @ref KInstance is compareable to @ref KGlobal used by normal applications.
 * It allows you to find ressource files (images, XML, sound etc.) belonging
 * to the library.
 * Furthermore if you provide any QObject derived classes, you must call
 * @ref setMocClasses() with an array of names of these classes. To generate
 * that list automatically use am_edit.
 *
 * If you want to load a library, use @ref KLibLoader. You can query @ref KLibLoader
 * directly for a pointer to the libraries factory by using the @ref KLibLoader::factory()
 * function.
 *
 * The KLibFactory is used to create the components, the library has to offer.
 * The factory of KSpread for example will create instances of KSpreadDoc,
 * while the Konqueror factory will create KonqView widgets.
 * All objects created by the factory must be derived from @ref QObject, since @ref QObject
 * offers type safe casting.
 *
 * KLibFactory is an abstract class. You have to overload the @ref create() method.
 *
 * @author Torben Weis <weis@kde.org>
 */
class KLibFactory : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a new factory.
     */
    KLibFactory( QObject* parent = 0, const char* name = 0 );
    virtual ~KLibFactory();

    /**
     * Create a new object. The returned object has to be derived from
     * the requested classname.
     *
     * It is valid behavior to create different kinds of objects
     * depending on the requested @p classname. For example a koffice
     * library may usually return a pointer to @ref KoDocument.  But
     * if asked for a "QWidget", it could create a wrapper widget,
     * that encapsulates the Koffice specific features.
     *
     * When derived classes implement this method, they should emit the signal
     * @ref objectCreated to tell the factory about their newly created object.
     * This is very important for reference counting, and allows unloading
     * the library automatically once all its objects have been destroyed.
     */

    virtual QObject* create( QObject* parent = 0, const char* name = 0, const char* classname = "QObject", const QStringList &args = QStringList() ) = 0;

    /**
     * Tells the libloading system, which QObject derived classes are
     * provided by the library.  This is necessary for unloading, to remove
     * any references to the meta objects created by Qt's object system.
     *
     * @p names an array of class names, for which this library provides
     * meta classes; the end of array is indicated by a 0 element
     */
    void setMocClasses(const char **names);
    
    void destroyMocClasses() const;

signals:
    /**
     * Emit this in @ref create
     */
    void objectCreated( QObject *obj );
public:
    KLibFactoryPrivate *d;
};

/**
 * @short Represents a dynamically loaded library.
 *
 * KLibrary allows you to look up symbols of the shared library.
 *
 * @author Torben Weis <weis@kde.org>
 */
class KLibrary : public QObject
{
    Q_OBJECT
public:
    KLibrary( const QString& libname, const QString& filename, lt_dlhandle handel );
    ~KLibrary();

    /**
     * @return The name of the library like "libkspread".
     */
    QString name() const;
    /**
     * @return The filename of the library, for example "/opt/kde2&/lib/libkspread.la"
     */
    QString fileName() const;

    /**
     * @return The factory of the library if there is any.
     */
    KLibFactory* factory();
    /**
     * Looks up a symbol from the library. This is a very low level
     * function that you usually dont want to use.
     */
    void* symbol( const char* name );

private slots:
    void slotObjectCreated( QObject *obj );
    void slotObjectDestroyed();
    void slotTimeout();

private:
    QString m_libname;
    QString m_filename;
    KLibFactory* m_factory;
    lt_dlhandle m_handle;
    QList<QObject> m_objs;
    QTimer *m_timer;
    KLibraryPrivate *d;
};

/**
 * The KLibLoader allows you to load libraries dynamically at runtime.
 * Dependend libraries are loaded automatically.
 *
 * KLibLoader follows the singleton pattern. You can not create multiple
 * instances. Use @ref #self() to get a pointer to the loader.
 *
 * @author Torben Weis <weis@kde.org>
 */
class KLibLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * You should NEVER destruct an instance of KLibLoader
     * until you know what you are doing. This will release
     * the loaded library.
     */
    ~KLibLoader();

    /**
     * Loads and initializes a library. Loading a library multiple times is
     * handled gracefully.
     *
     * This is a convenience function that returns the factory immediately
     *
     * @see #library
     */
    KLibFactory* factory( const char* libname );

    /**
     * Loads and initializes a library. Loading a library multiple times is
     * handled gracefully.
     *
     * @param libname  This is the library name without extension. Usually that is something like
     *                 "libkspread". The function will then search for a file named
     *                 "libkspread.la" in the KDE library paths.
     *                 The *.la files are created by libtool and contain
     *                 important information especially about the libraries dependencies
     *                 on other shared libs. Loading a "libfoo.so" could not solve the
     *                 dependencies problem.
     *
     *                 You can, however, give a library name ending in ".so"
     *                 (or whatever is used on your platform), and the library
     *                 will be loaded without resolving dependencies. USE WITH CARE :)
     *
     * @see #factory
     */
    virtual KLibrary* library( const char* libname );

    virtual void unloadLibrary( const char *libname );

    /**
     * @return a pointer to the loader. If no loader exists until now
     *         then one is created.
     */
    static KLibLoader* self();

    /**
     * @internal method, called by the KApplication destructor
     */
    static void cleanUp();

protected:
    KLibLoader( QObject* parent = 0, const char* name = 0 );

private slots:
    void slotLibraryDestroyed();
private:
    QAsciiDict<KLibrary> m_libs;

    static KLibLoader* s_self;

    KLibLoaderPrivate *d;
};

#endif
