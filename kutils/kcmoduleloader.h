/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
  
  This file is part of the KDE project
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/
#ifndef MODLOADER_H
#define MODLOADER_H

#include <kcmodule.h>
#include <kcmoduleinfo.h>

class QWidget;
class KLibLoader;

/**
 * @short Loads a KControl Module.
 *
 * @description KCModuleLoader tries in several ways 
 * to locate and load a KCModule. If loading fails a 
 * zero pointer is returned. \n
 * It is very unlikely KCModuleLoader is what you want 
 * and @ref KCModuleProxy suits your needs.
 * 
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
 * @since 3.2
 * @internal
**/
class KCModuleLoader
{
  public:

    /**
     * Loads a @ref KCModule. If loading fails a zero pointer is returned.
     * @param mod what module to load
     * @param withFallback if true and loading failed a separate window
     * with the module may appear and a zero pointer is a returned
     *
     * @return a pointer to the loaded @ref KCModule
     *
     */
    static KCModule *loadModule(const KCModuleInfo &mod, bool withfallback=true, 
        QWidget * parent = 0, const char * name = 0, const QStringList & args = QStringList() );

    /**
     * Same as above but takes a @ref QString instead of a @ref KCModuleInfo as 
     * module specifier
     * @param module what module to load
     */
    static KCModule *loadModule(const QString &module, QWidget *parent = 0,
        const char *name = 0, const QStringList & args = QStringList());

    /**
     * Unloads the module's library
     * @param mod What module to unload for
     */
    static void unloadModule(const KCModuleInfo &mod);

    /**
     * Display a message box explaining an error occured and possible
     * reasons to why.
     */
    static void showLastLoaderError(QWidget *parent);

  private:

    /**
     * Internal loader called by the public loaders.
     */
    static KCModule* load(const KCModuleInfo &mod, const QString &libname, 
        KLibLoader *loader, QWidget * parent = 0, const char * name = 0, const QStringList & args = QStringList() );

};

// vim: ts=2 sw=2 et
#endif // MODLOADER_H

