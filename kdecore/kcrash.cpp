/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Timo Hummel (timo.hummel@sap.com)
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

#include "kcrash.h"

int CrashRecursionCounter;	// If a crash occurs in our crash handler procedure, we can handle it :)
void (*emergencySaveFunction) (int);

// This function sets the function which should be called when the application crashes and the
// application is asked to try to save its data.
void
setEmergencySaveFunction (void (*saveFunction) (int))
{
  if ((kdesignal_t)saveFunction == KDE_SAVE_NONE)
    {
      emergencySaveFunction = KDE_SAVE_NONE;
    }

  emergencySaveFunction = saveFunction;
}


// This function sets the function which should be responsible for the application crash handling.
// Usually, this should be KDE_CRASH_INTERNAL.

void
setCrashHandler (void (*crashHandler) (int))
{
  printf ("Trying to install kcrash handler...\n");

  if (crashHandler == KDE_CRASH_DEFAULT)
    {
      printf ("Installing the system's default bug handler...");
      signal (SIGSEGV, SIG_DFL);
      printf ("done.\n");
      return;
    }

  if (crashHandler == KDE_CRASH_INTERNAL)
    {
      printf ("Installing the KDE internal crash handler...");
      signal (SIGSEGV, defaultCrashHandler);
      printf ("done\n");
      return;
    }

  printf ("Installing the application's crash handler...");
  signal (SIGSEGV, crashHandler);
  printf ("done.\n");
}

void
resetCrashRecursion (void)
{
  printf ("Crash recursion set to zero.\n");
  CrashRecursionCounter = 0;
}

void
defaultCrashHandler (int signal)
{
  // Handle possible recursions

  CrashRecursionCounter++;

  if (CrashRecursionCounter < 2)
    {
     
      // Check if 
      if (emergencySaveFunction != KDE_SAVE_NONE)
	{
	  emergencySaveFunction (signal);
	}
    }

  if (CrashRecursionCounter < 3)
    {
      printf ("Would start dr. konqi here\n");

      execlp ("xmessage", "xmessage", "An application crashed!",
	      NULL);
    }

  printf ("Unable to start dr. konqi\n");
  exit (1);
}
