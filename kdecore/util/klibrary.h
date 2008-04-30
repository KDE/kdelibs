/* This file is part of the KDE libraries
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de.org>

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
#ifndef KLIBRARY_H
#define KLIBRARY_H

#include <kdecore_export.h>

#include <kglobal.h>

#include <QtCore/QLibrary>

class KLibraryPrivate;

class KPluginFactory;

/**
 * \class KLibrary klibrary.h <KLibrary>
 * 
 * Thin wrapper around QLibrary; you should rarely use this directly, see KPluginLoader for higher-level loading of plugins.
 * KLibrary adds kde3-factory and kde4-factory support to QLibrary (for the deprecated KLibLoader)
 * KLibrary also searches libs in the kde search paths.
 */
class KDECORE_EXPORT KLibrary : public QLibrary
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
public:
    typedef void (*void_function_ptr) ();

    explicit KLibrary(QObject *parent = 0);
    explicit KLibrary(const QString &name, const KComponentData &cData = KGlobal::mainComponent(), QObject *parent = 0);
    KLibrary(const QString &name, int verNum, const KComponentData &cData = KGlobal::mainComponent(), QObject *parent = 0);

    virtual ~KLibrary();

    /**
     * Returns the factory of the library.
     * @param factoryname The postfix to the init_ symbol used to create the
     * factory object. It corresponds to the first parameter to
     * K_EXPORT_COMPONENT_FACTORY.
     * @return The factory of the library if there is any, otherwise 0
     * @deprecated use KPluginLoader::factory
     */
    KDE_DEPRECATED KPluginFactory* factory( const char* factoryname = 0 );

    /**
     * Looks up a symbol from the library. This is a very low level
     * function that you usually don't want to use.
     * @param name the name of the symbol to look up
     * @return the address of the symbol, or 0 if it does not exist
     */
    void *resolveSymbol(const char *name);

    /**
     * Looks up a symbol from the library. This is a very low level
     * function that you usually don't want to use.
     * @param name the name of the symbol to look up
     * @return the address of the symbol, or 0 if it does not exist
     */
    void_function_ptr resolveFunction(const char *name);

    void setFileName(const QString &name, const KComponentData &data = KGlobal::mainComponent());

    bool unload() { return false; } //this is only temporary. i will remove it as soon as I have removed all dangerous users of it
private:
    KLibraryPrivate *d_ptr;
};

#endif
