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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KLIBLOADER_H
#define KLIBLOADER_H

#include <kglobal.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QLibrary>
#include <QtCore/QtPlugin>

#include "kpluginfactory.h"
#include "kpluginloader.h"
#include "klibrary.h"

# define K_EXPORT_COMPONENT_FACTORY( libname, factory ) \
    extern "C" { KDE_EXPORT KPluginFactory *init_##libname() { return new factory; } }

/**
 * \class KLibLoader klibloader.h <KLibLoader>
 * 
 * The KLibLoader allows you to load libraries dynamically at runtime.
 * Dependent libraries are loaded automatically.
 *
 * KLibLoader follows the singleton pattern. You can not create multiple
 * instances. Use self() to get a pointer to the loader.
 *
 * @deprecated You have two other possibilites:
 *       KPluginLoader or KService::createInstance for plugins
 *       KLibrary for other libraries
 *
 * @see KLibrary
 * @see KPluginLoader
 * @author Torben Weis <weis@kde.org>
 */
class KDECORE_EXPORT KLibLoader : public QObject //krazy:exclude=dpointer (private class is kept as a global static)
{
    friend class KLibrary;
    friend class KLibraryPrivate;
    friend class KLibLoaderPrivate;

    Q_OBJECT
public:
    /**
     * Loads and initializes a library. Loading a library multiple times is
     * handled gracefully.
     *
     * This is a convenience function that returns the factory immediately
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
     *                 will be loaded without resolving dependencies. Use with caution.
     * @return the KPluginFactory, or 0 if the library does not exist or it does
     *         not have a factory
     * @see library
     */
    KPluginFactory* factory( const QString &libname, QLibrary::LoadHints loadHint = 0);

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
     *                 will be loaded without resolving dependencies. Use with caution.
     * @return KLibrary is invalid (0) when the library couldn't be dlopened. in such
     * a case you can retrieve the error message by calling KLibLoader::lastErrorMessage()
     *
     * @see factory
     */
    KLibrary* library( const QString &libname, QLibrary::LoadHints loadHint = 0 );

    /**
     * Returns an error message that can be useful to debug the problem.
     * Returns QString() if the last call to library() was successful.
     * You can call this function more than once. The error message is only
     * reset by a new call to library().
     * @return the last error message, or QString() if there was no error
     */
    QString lastErrorMessage() const;

    /**
     * Unloads the library with the given name.
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
     *                 will be loaded without resolving dependencies. Use with caution.
     */
    void unloadLibrary( const QString &libname );

    /**
     * Returns a pointer to the factory. Use this function to get an instance
     * of KLibLoader.
     * @return a pointer to the loader. If no loader exists until now
     *         then one is created.
     *
     * @deprecated use KPluginLoader
     */
    static KDE_DEPRECATED KLibLoader* self();

    /**
     * Helper method which looks for a library in the standard paths
     * ("module" and "lib" resources).
     * Made public for code that doesn't use KLibLoader itself, but still
     * wants to open modules.
     * @param libname of the library. If it is not a path, the function searches in
     *                the "module" and "lib" resources. If there is no extension,
     *                ".la" will be appended.
     * @param cData a KComponentData used to get the standard paths
     */
    static QString findLibrary(const QString &libname, const KComponentData &cData = KGlobal::mainComponent());

    /**
     * This enum type defines the possible error cases that can happen
     * when loading a component.
     *
     * <ul>
     *  <li><code>ErrNoLibrary</code> - the specified library could not be
     *      loaded. Use KLibLoader::lastErrorMessage for details.</li>
     *  <li><code>ErrNoFactory</code> - the library does not export a factory
     *      for creating components</li>
     *  <li><code>ErrNoComponent</code> - the factory does not support creating
     *      components of the specified type</li>
     *  <li><code>ErrServiceProvidesNoLibrary</code> - the specified service
     *      provides no shared library (when using KService)</li>
     *  <li><code>ErrNoServiceFound</code> - no service implementing the
     *      given servicetype and fullfilling the given constraint expression
     *      can be found (when using KServiceTypeTrader).</li>
     * </ul>
     */
    enum ComponentLoadingError { ErrNoLibrary = 1,
                                 ErrNoFactory,
                                 ErrNoComponent,
                                 ErrServiceProvidesNoLibrary,
                                 ErrNoServiceFound };

    /**
     * Returns a translated error message for @p componentLoadingError
     * @param componentLoadingError the error code, as returned in the "error"
     * output parameter of createInstance.
     */
    static QString errorString( int componentLoadingError );


    /**
     * This template allows to load the specified library and ask the
     * factory to create an instance of the given template type.
     *
     * @param libraryName The library to open
     * @param parent The parent object (see QObject constructor)
     * @param args A list of string arguments, passed to the factory and possibly
     *             to the component (see KPluginFactory)
     * @param error
     * @return A pointer to the newly created object or a null pointer if the
     *         factory was unable to create an object of the given type.
     */
    template <typename T>
    static KDE_DEPRECATED T *createInstance(const QString &keyword, const QString &libname, QObject *parent = 0,
                              const QVariantList &args = QVariantList(),
                              int *error = 0 )
    {
        KLibrary *library = KLibLoader::self()->library( libname );
        if ( !library )
        {
            if ( error )
                *error = ErrNoLibrary;
            return 0;
        }
        KPluginFactory *factory = library->factory();
        if ( !factory )
        {
            library->unload();
            if ( error )
                *error = ErrNoFactory;
            return 0;
        }
        QObject *object = factory->template create<T>(keyword, parent, args);
        T *res = qobject_cast<T *>( object );
        if ( !res )
        {
            delete object;
            library->unload();
            if ( error )
                *error = ErrNoComponent;
        }
        return res;
    }

    template <typename T>
    static KDE_DEPRECATED T *createInstance( const QString &libname, QObject *parent = 0,
                              const QVariantList &args = QVariantList(),
                              int *error = 0 )
    {
        return createInstance<T>(QString(), libname, parent, args, error);
    }

    /**
     * @deprecated Use one of the other createInstance methods or
     *             KPluginLoader or KService::createInstance instead
     */
    template <typename T>
    static KDE_DEPRECATED T *createInstance( const QString &libname, QObject *parent,
                              const QStringList &args,
                              int *error = 0 )
    {
        KLibrary *library = KLibLoader::self()->library( libname );
        if ( !library )
        {
            if ( error )
                *error = ErrNoLibrary;
            return 0;
        }
        KPluginFactory *factory = library->factory();
        if ( !factory )
        {
            library->unload();
            if ( error )
                *error = ErrNoFactory;
            return 0;
        }
        QObject *object = factory->template create<T>(parent, args);
        T *res = qobject_cast<T *>( object );
        if ( !res )
        {
            delete object;
            library->unload();
            if ( error )
                *error = ErrNoComponent;
        }
        return res;
    }

private:
    ~KLibLoader();

    KLibLoader();
};

#endif
