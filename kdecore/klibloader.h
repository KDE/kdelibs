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
#include <kglobal.h>

#include <stdlib.h> // For backwards compatibility

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
 * KLibFactory is an abstract class. Reimplement the @ref
 * createObject() method to give it functionality.
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
     * Creates a new object. The returned object has to be derived from
     * the requested classname.
     *
     * It is valid behavior to create different kinds of objects
     * depending on the requested @p classname. For example a koffice
     * library may usually return a pointer to @ref KoDocument.  But
     * if asked for a "QWidget", it could create a wrapper widget,
     * that encapsulates the Koffice specific features.
     *
     * Never reimplement this function. Instead, reimplement @ref
     * createObject().
     *
     * create() automatically emits a signal @ref objectCreated to tell
     * the library about its newly created object.  This is very
     * important for reference counting, and allows unloading the
     * library automatically once all its objects have been destroyed.
     *
     * This function is virtual for compatibility reasons only.
     */

    virtual QObject* create( QObject* parent = 0, const char* name = 0, const char* classname = "QObject", const QStringList &args = QStringList() );

signals:
    /**
     * Emitted in @ref create
     */
    void objectCreated( QObject *obj );


protected:

    /**
     * Creates a new object. The returned object has to be derived from
     * the requested classname.
     *
     * It is valid behavior to create different kinds of objects
     * depending on the requested @p classname. For example a koffice
     * library may usually return a pointer to @ref KoDocument.  But
     * if asked for a "QWidget", it could create a wrapper widget,
     * that encapsulates the Koffice specific features.
     *
     * This function is called by @ref create()
     */
    virtual QObject* createObject( QObject* parent = 0, const char* name = 0, const char* classname = "QObject", const QStringList &args = QStringList() );


private:
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
    /**
     * @internal
     * Don't create KLibrary objects on your own. Instead use @ref KLibLoader.
     */
    KLibrary( const QString& libname, const QString& filename, void * handle );
    /**
     * @internal
     * Don't destruct KLibrary objects yourself. Instead use @ref KLibLoader::unloadLibrary.
     */
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
    void* symbol( const char* name ) const;

private slots:
    void slotObjectCreated( QObject *obj );
    void slotObjectDestroyed();
    void slotTimeout();

private:
    QString m_libname;
    QString m_filename;
    KLibFactory* m_factory;
    void * m_handle;
    QList<QObject> m_objs;
    QTimer *m_timer;
    KLibraryPrivate *d;
};

class KLibWrapPrivate;

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
     * the loaded libraries.
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
     * @return KLibrariy is invalid (0) when the library couldn't be dlopened. in such
     * a case you can retrieve the error message by calling KLibLoader::lastErrorMessage()
     *
     * @see #factory
     */
    virtual KLibrary* library( const char* libname );

    /**
     * Loads and initializes a library. Loading a library multiple times is
     * handled gracefully.   The library is loaded such that the symbols are
     * globally accessible so libraries with dependencies can be loaded
     * sequentially.
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
     * @return KLibrariy is invalid (0) when the library couldn't be dlopened. in such
     * a case you can retrieve the error message by calling KLibLoader::lastErrorMessage()
     *
     * @see #factory
     */
    KLibrary* globalLibrary( const char *name );

    /*
     * returns an error message that can be useful to debug the problem
     * returns QString::null if the last call to ::library(const char*) was successful
     * you can call this function more than once. The error message is only
     * reset by a new call to library(). 
     */
    QString lastErrorMessage() const;

    virtual void unloadLibrary( const char *libname );

    /**
     * @return a pointer to the loader. If no loader exists until now
     *         then one is created.
     */
    static KLibLoader* self();

    /**
     * @internal method, called by the KApplication destructor
     * This is what makes it possible to rely on ~KLibFactory
     * being called in all cases, whether the library is unloaded
     * while the application is running or when exiting.
     */
    static void cleanUp();

    /**
     * Helper method which looks for a library in the standard paths
     * ("module" and "lib" resources)
     * Made public for code that doesn't use KLibLoader itself, but still
     * wants to open modules.
     */
    static QString findLibrary( const char * name, const KInstance * instance = KGlobal::instance() );

protected:
    KLibLoader( QObject* parent = 0, const char* name = 0 );

private slots:
    void slotLibraryDestroyed();
private:
    void close_pending( KLibWrapPrivate * );
    QAsciiDict<KLibWrapPrivate> m_libs;

    static KLibLoader* s_self;

    KLibLoaderPrivate *d;
};

#endif
