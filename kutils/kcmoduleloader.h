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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
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
 * KCModuleLoader tries in several ways 
 * to locate and load a KCModule. If loading fails a 
 * zero pointer is returned. \n
 * It is very unlikely KCModuleLoader is what you want 
 * and @ref KCModuleProxy suits your needs.
 * 
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
 * @author Frans Englich <frans.englich@telia.com>
 * @since 3.2
 * @internal
**/
class KUTILS_EXPORT KCModuleLoader
{
  public:

    /**
     * Loads a @ref KCModule. If loading fails a zero pointer is returned.
     * @param module what module to load
     * @param withFallback if true and loading failed a separate window
     * with the module may appear and a zero pointer is a returned
     *
     * @return a pointer to the loaded @ref KCModule
     *
     * @deprecated use the function which explicitly states the error reporting
     * method
     */
    static KCModule *loadModule(const KCModuleInfo &module, bool withFallback=true, 
        QWidget * parent = 0, const char * name = 0,
        const QStringList & args = QStringList() ) KDE_DEPRECATED;

    /**
     * Loads a @ref KCModule. If loading fails a zero pointer is returned.
     * @param module what module to load
     * with the module may appear and a zero pointer is a returned
     *
     * @deprecated use the function which explicitly states the error reporting
     * method
     */
    static KCModule *loadModule(const QString &module, QWidget *parent = 0,
        const char *name = 0, const QStringList & args = QStringList()) KDE_DEPRECATED;

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
     * @param withFallback if true and loading failed a separate window
     * with the module may appear and a zero pointer is a returned
     *
     * @return a pointer to the loaded @ref KCModule
     * @since 3.4
     */
    static KCModule *loadModule(const KCModuleInfo &module, ErrorReporting
        report, bool withFallback=true, QWidget * parent = 0,
        const char * name = 0, const QStringList & args = QStringList() );

    /**
     * Loads a @ref KCModule. If loading fails a zero pointer is returned.
     * @param module what module to load
     * @param report see ErrorReporting
     * with the module may appear and a zero pointer is a returned
     *
     * @return a pointer to the loaded @ref KCModule
     * @since 3.4
     */
    static KCModule *loadModule(const QString &module, ErrorReporting
        report, QWidget *parent = 0, const char *name = 0,
        const QStringList & args = QStringList());

    /**
     * Unloads the module's library
     * @param mod What module to unload for
     */
    static void unloadModule(const KCModuleInfo &mod);

    /**
     * Display a message box explaining an error occured and possible
     * reasons to why.
     *
     * @deprecated Use a constructor with ErrorReporting set to Dialog to show a
     * message box like this function did.
     */
    static void showLastLoaderError(QWidget *parent) KDE_DEPRECATED;


    /**
     * Checks whether an KCModule should be shown by running its 
     * test function. If it is unsure whether a module should be shown, it should 
     * be made available, leaving the decision to the user.
     * If false is returned, the module should not be loaded in any interface.
     * 
     * A module declares it needs to be tested by having "X-KDE-Test-Module=true" in 
     * its desktop file. When that line exists, the following code must be available 
     * in the module's library:
     *
     * \code
     * extern "C"
     * {
     *    bool test_moduleName()
     *    {
     *      // Code testing for hardware/software presence.
     *      return true; // and the modue will be loaded.
     *    }
     *
     * }
     * \endcode
     *
     * where moduleName is the library name for the module.
     *
     * @param module the module to check
     * @returns true if the module should be loaded
     * @since 3.4
     */
    static bool testModule( const QString& module );

    /**
     * Convenience function, essentially the same as above.
     *
     * @param module the module to check
     * @returns true if the module should be loaded
     * @since 3.4
     */
    static bool testModule( const KCModuleInfo& module );

    /**
     * Returns a KCModule containing the messages @p report and @p text.
     *
     * @param report the type of error reporting, see ErrorReporting
     * @param text the main message
     * @param details any additional details
     *
     * @since 3.4
     * @internal
     */
    static KCModule* reportError( ErrorReporting report, const QString & text,
        QString details, QWidget * parent );

  private:

    /**
     * Internal loader called by the public loaders.
     * @internal
     */
    static KCModule* load(const KCModuleInfo &mod, const QString &libname, 
        KLibLoader *loader, ErrorReporting report, QWidget * parent = 0,
        const char * name = 0, const QStringList & args = QStringList() );

};

// vim: ts=2 sw=2 et
#endif // MODLOADER_H

