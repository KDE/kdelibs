/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (c) 2006 Matthias Kretz <kretz@kde.org>
  
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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef KCMODULELOADER_H
#define KCMODULELOADER_H

#include <kcmodule.h>
#include <kcmoduleinfo.h>

class QWidget;

/**
 * @short Loads a KControl Module.
 *
 * KCModuleLoader tries in several ways 
 * to locate and load a KCModule. If loading fails a 
 * zero pointer is returned. \n
 * It is very unlikely KCModuleLoader is what you want 
 * and @ref KCModuleProxy suits your needs.
 * 
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
 * @author Frans Englich <frans.englich@telia.com>
 * @internal
**/
namespace KCModuleLoader
{
    /**
     * Determines the way errors are reported
     */
    enum ErrorReporting {
      /** 
       * no error reporting is done 
       * */
      None = 0,
      /**
       * the error report is shown instead of the 
       * KCModule that should have * been loaded 
       */
      Inline = 1,
      /** 
       * shows a dialog with the error report 
       */
      Dialog = 2,
      /** 
       * does both Inline and Dialog 
       */
      Both = 3
    };

    /**
     * Loads a @ref KCModule. If loading fails a zero pointer is returned.
     * @param module what module to load
     * @param report see ErrorReporting
     *
     * @return a pointer to the loaded @ref KCModule
     */
    KUTILS_EXPORT KCModule *loadModule(const KCModuleInfo &module, ErrorReporting
        report, QWidget * parent = 0, const QStringList& args = QStringList() );

    /**
     * Loads a @ref KCModule. If loading fails a zero pointer is returned.
     *
     * @param module what module to load
     * @param report see ErrorReporting
     *
     * @return a pointer to the loaded @ref KCModule
     */
    KUTILS_EXPORT KCModule *loadModule( const QString &module, ErrorReporting report,
        QWidget *parent = 0, const QStringList& args = QStringList() );

    /**
     * Unloads the module's library
     * @param mod What module to unload for
     */
    KUTILS_EXPORT void unloadModule(const KCModuleInfo &mod);

    /**
     * Display a message box explaining an error occurred and possible
     * reasons to why.
     *
     * @deprecated Use a constructor with ErrorReporting set to Dialog to show a
     * message box like this function did.
     */
    KUTILS_EXPORT KDE_DEPRECATED void showLastLoaderError(QWidget *parent);

    /**
     * Returns a KCModule containing the messages @p report and @p text.
     *
     * @param report the type of error reporting, see ErrorReporting
     * @param text the main message
     * @param details any additional details
     *
     * @internal
     */
    KUTILS_EXPORT KCModule* reportError( ErrorReporting report, const QString & text,
        const QString &details, QWidget * parent );

}

// vim: ts=2 sw=2 et
#endif // KCMODULELOADER_H
