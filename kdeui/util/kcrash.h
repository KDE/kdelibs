/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Timo Hummel <timo.hummel@sap.com>
 *                    Tom Braun <braunt@fh-konstanz.de>
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef KCRASH_H
#define KCRASH_H

#include <kdeui_export.h>

class QString;

/**
 * This class handles segmentation-faults.
 * By default it displays a  message-box saying the application crashed.
 * This default can be overridden by setting a custom crash handler with
 * setCrashHandler().
 * If a function is specified with setEmergencySaveFunction() it will
 * be called by the default crash handler, giving the application a chance
 * to save its data.
 */
namespace KCrash
{
  /**
   * The default crash handler.
   * @param signal the signal number
   */
  KDEUI_EXPORT void defaultCrashHandler (int signal);

  /**
   * This function type is a pointer to a crash handler function.
   * The function's argument is the number of the signal.
   */
  typedef void (*HandlerType)(int);

  /**
   * Install a function to be called in case a SIGSEGV is caught.
   * @param handler HandlerType handler can be one of
   * @li null in which case signal-catching is disabled
   *  (by calling signal(SIGSEGV, SIG_DFL))
   * @li if handler is omitted the default crash handler is installed.
   * @li an user defined function in the form:
   * static (if in a class) void myCrashHandler(int);
   * @param handler the crash handler
   */

  KDEUI_EXPORT void setCrashHandler (HandlerType handler = defaultCrashHandler);

  /**
   * Returns the installed crash handler.
   * @return the crash handler
   */
  KDEUI_EXPORT HandlerType crashHandler();

  /**
   * Installs a function which should try to save the applications data.
   * It is the crash handler's responsibility to call this function.
   * Therefore, if no crash handler is set, the default crash handler
   * is installed to ensure the save function is called.
   * @param saveFunction the handler to install
   */
  KDEUI_EXPORT void setEmergencySaveFunction (HandlerType saveFunction = 0);

  /**
   * Return the currently set emergency save function.
   * @return the emergency save function
   */
  KDEUI_EXPORT HandlerType emergencySaveFunction();

  /**
   * Options to determine how KCrash should behave while firing up DrKonqi.
   */
  enum CrashFlag {
    KeepFDs = 1,          ///< don't close all file descriptors immediately
    SaferDialog = 2,      ///< start DrKonqi without arbitrary disk access
    AlwaysDirectly = 4,   ///< never try to to start DrKonqi via kdeinit
    AutoRestart = 8       ///< autorestart this application. Only sensible for KUniqueApplications. @since 4.1.
  };
  Q_DECLARE_FLAGS(CrashFlags, CrashFlag)

  /**
   * Set DrKonqi fire-up options.
   * @param flags ORed together CrashFlags
   */
  KDEUI_EXPORT void setFlags( CrashFlags flags );

  /**
   * Sets the application @p path which should be passed to
   * Dr. Konqi, our nice crash display application.
   * @param path the application path.
   */
  KDEUI_EXPORT void setApplicationPath (const QString &path);

  /**
   * Sets the application name @p name which should be passed to
   * Dr. Konqi, our nice crash display application.
   * @param name the name of the application, as shown in Dr. Konqi
   */
  KDEUI_EXPORT void setApplicationName (const QString &name);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(KCrash::CrashFlags)

#endif

