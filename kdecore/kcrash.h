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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef __KCRASH_H
#define __KCRASH_H

#include <qstring.h>

/**
 * This class handles segmentation-faults.
 * By default it displays a  message-box saying the application crashed.
 * This default can be overridden by setting a custom crash handler with
 * @ref setCrashHandler().
 * If a function is specified with @ref setEmergencySaveFunction() it will
 * be called by the default crash handler, giving the application a chance
 * to save its data.
 */
class KCrash
{
 private: // ;o)
  static const char *appName;
  static const char *appPath;

 public:
  static void defaultCrashHandler (int signal);
  typedef void (*HandlerType)(int);

  /**
   * Install a function to be called in case a SIGSEGV is caught.
   * @param HandlerType handler can be one of
   * @li null in which case signal-catching is disabled
   *  (by calling signal(SIGSEGV, SIG_DFL))
   * @li if handler is omitted the default crash handler is installed.
   * @li an user defined function in the form:
   * static (if in a class) void myCrashHandler(int);
   */

  static void setCrashHandler (HandlerType handler = defaultCrashHandler);

  /**
   * Returns the installed crash handler
   */
  static HandlerType crashHandler() { return _crashHandler; }

  /**
   * Installs a function which should try to save the applications data.
   * It is the crash handler´s responsibility to call this function.
   * Therefore, if no crash handler is set, the default crash handler
   * is installed to ensure the save function is called.
   */
  static void setEmergencySaveFunction (HandlerType saveFunction = (HandlerType)0);
  /**
   * Return the currently set emergency save function.
   */
  static HandlerType emergencySaveFunction() { return _emergencySaveFunction; }

  /**
   * Sets the application path @p path which should be passed to
   * Dr. Konqi, our nice crash display application.
   */
  static void setApplicationPath (QString path) { appPath = qstrdup(path.local8Bit().data()); }

  /**
   * Sets the application name @p name which should be passed to
   * Dr. Konqi, our nice crash display application.
   */
  static void setApplicationName (QString name) { appName = qstrdup(name.local8Bit().data()); }

 protected:
  static HandlerType _crashHandler;
  static HandlerType _emergencySaveFunction;
};

#endif

