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

#include <kdelibs_export.h>

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
class KDECORE_EXPORT KCrash
{
 public:
  /**
   * The default crash handler.
   * @param signal the signal number
   */
  static void defaultCrashHandler (int signal);

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

  static void setCrashHandler (HandlerType handler = defaultCrashHandler);

  /**
   * Returns the installed crash handler.
   * @return the crash handler
   */
  static HandlerType crashHandler();

  /**
   * Installs a function which should try to save the applications data.
   * It is the crash handler's responsibility to call this function.
   * Therefore, if no crash handler is set, the default crash handler
   * is installed to ensure the save function is called.
   * @param saveFunction the handler to install
   */
  static void setEmergencySaveFunction (HandlerType saveFunction = 0);

  /**
   * Return the currently set emergency save function.
   * @return the emergency save function
   */
  static HandlerType emergencySaveFunction();

  /**
   * Set whether to start drkonqi without arbitrary disk access
   */
  static void setSafer( bool on );

  /**
   * Sets the application @p path which should be passed to
   * Dr. Konqi, our nice crash display application.
   * @param path the application path.
   */
  static void setApplicationPath (const QString &path);

  /**
   * Sets the application name @p name which should be passed to
   * Dr. Konqi, our nice crash display application.
   * @param name the name of the application, as shown in Dr. Konqi
   */
  static void setApplicationName (const QString &name);


 private:
  static void startDrKonqi( const char* argv[], int argc );
  static void startDirectly( const char* argv[], int argc );
};

#endif

