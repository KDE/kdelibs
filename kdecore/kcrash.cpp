/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Timo Hummel <timo.hummel@sap.com>
 *                    Tom Braun <braunt@fh-konstanz.de>
 *
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
 */

/*
 * This file is used to catch signals which would normally
 * crash the application (like segmentation fault, floating
 * point exception and such).
 */

#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <kdebug.h>

#include "kcrash.h"


KCrash::HandlerType KCrash::_emergencySaveFunction = 0;
KCrash::HandlerType KCrash::_crashHandler = 0;
QString KCrash::appName = QString::null;
QString KCrash::appPath = QString::null;

// This function sets the function which should be called when the 
// application crashes and the
// application is asked to try to save its data.
void
KCrash::setEmergencySaveFunction (HandlerType saveFunction)
{
  _emergencySaveFunction = saveFunction;
  
  /* 
   * We need at least the default crash handler for 
   * emergencySaveFunction to be called
   */
  if (_emergencySaveFunction && !_crashHandler)
    _crashHandler = defaultCrashHandler;
}


// This function sets the function which should be responsible for 
// the application crash handling.

void
KCrash::setCrashHandler (HandlerType handler)
{
  if (!handler)
    signal (SIGSEGV, SIG_DFL);

  signal (SIGSEGV, handler);

  _crashHandler = handler;
}

void
KCrash::defaultCrashHandler (int signal)
{
  // Handle possible recursions
  static int crashRecursionCounter;
#warning PLEASE: Someone should check if this static work
#warning If this procedure crashes, what happens? Does it exit?
#warning If not, please re-implement resetCrashRecursion,
#warning add a new member variable to this class and 
#warning remove the comment in kapp.cpp (resetCrashRecursion)

  crashRecursionCounter++;

  if (crashRecursionCounter < 2) {
    if (_emergencySaveFunction) {
      _emergencySaveFunction (signal);
    }
  }
  
  if (crashRecursionCounter < 3) {
      execlp ("drkonqi", "drkonqi", 
              "--appname", appName.latin1(),
              "--apppath", appPath.latin1(),
              "--signal", QCString().sprintf("%d", signal).data(),
	      NULL);
  }

  kdDebug(0) << "Unable to start dr. konqi\n";
  exit (1);
}

