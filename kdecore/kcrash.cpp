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
#include "kcrash.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <qwindowdefs.h> 
#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kapplication.h>
#include <dcopclient.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

KCrash::HandlerType KCrash::_emergencySaveFunction = 0;
KCrash::HandlerType KCrash::_crashHandler = 0;
const char *KCrash::appName = 0;
const char *KCrash::appPath = 0;

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
    handler = SIG_DFL;

  sigset_t mask;
  sigemptyset(&mask);

#ifdef SIGSEGV
  signal (SIGSEGV, handler);
  sigaddset(&mask, SIGSEGV);
#endif
#ifdef SIGFPE
  signal (SIGFPE, handler);
  sigaddset(&mask, SIGFPE);
#endif
#ifdef SIGILL
  signal (SIGILL, handler);
  sigaddset(&mask, SIGILL);
#endif
#ifdef SIGABRT
  signal (SIGABRT, handler);
  sigaddset(&mask, SIGABRT);
#endif

  sigprocmask(SIG_UNBLOCK, &mask, 0);

  _crashHandler = handler;
}

void
KCrash::defaultCrashHandler (int sig)
{
  // WABA: Do NOT use kdDebug() in this function because it is much too risky!
  // Handle possible recursions
  static int crashRecursionCounter = 0;
  crashRecursionCounter++; // Nothing before this, please !

  signal(SIGALRM, SIG_DFL);
  alarm(3); // Kill me... (in case we deadlock in malloc)

  if (crashRecursionCounter < 2) {
    if (_emergencySaveFunction) {
      _emergencySaveFunction (sig);
    }
    crashRecursionCounter++; // 
  }
  
  if (crashRecursionCounter < 3)
  {
    if (appName) 
    {
      fprintf(stderr, "KCrash: crashing.... crashRecursionCounter = %d\n", crashRecursionCounter);
      fprintf(stderr, "KCrash: Application Name = %s path = %s pid = %d\n", appName ? appName : "<unknown>" , appPath ? appPath : "<unknown>", getpid());
      pid_t pid = fork();

      if (pid <= 0) {
        // this code is leaking, but this should not hurt cause we will do a
        // exec() afterwards. exec() is supposed to clean up.
        char * argv[18]; // don't forget to update this
        int i = 0;

        // argument 0 has to be drkonqi
        argv[i++] = qstrdup("drkonqi");

        // start up on the correct display
        argv[i++] = qstrdup("-display");
#ifdef Q_WS_X11
        if ( qt_xdisplay() )
          argv[i++] = XDisplayString(qt_xdisplay());
        else
          argv[i++] = getenv("DISPLAY");
#elif defined(Q_WS_QWS)
	argv[i++] = getenv("QWS_DISPLAY");
#endif

        // we have already tested this
        argv[i++] = qstrdup("--appname");
        argv[i++] = qstrdup(appName);
        if (KApplication::loadedByKdeinit)
          argv[i++] = qstrdup("--kdeinit");

        // only add apppath if it's not NULL
        if (appPath) {
          argv[i++] = qstrdup("--apppath");
          argv[i++] = qstrdup(appPath);
        }

        // signal number -- will never be NULL
        QCString tmp;
        tmp.setNum(sig);
        argv[i++] = qstrdup("--signal");
        argv[i++] = qstrdup(tmp.data());

        // pid number -- only include if this is the child
        // the debug stuff will be disabled if we was not able to fork
        if (pid == 0) {
	  tmp.setNum(getppid());
          argv[i++] = qstrdup("--pid");
	  argv[i++] = qstrdup(tmp.data());
        }

        const KInstance *instance = KGlobal::_instance;
        const KAboutData *about = instance ? instance->aboutData() : 0;
        if (about) {
	  if (!about->version().isNull()) {
	    argv[i++] = qstrdup("--appversion");
	    argv[i++] = qstrdup(about->version().utf8());
	  }

	  if (!about->programName().isNull()) {
	    argv[i++] = qstrdup("--programname");
	    argv[i++] = qstrdup(about->programName().utf8());
	  }

	  if (!about->bugAddress().isNull()) {
	    argv[i++] = qstrdup("--bugaddress");
	    argv[i++] = qstrdup(about->bugAddress().utf8());
	  }
        }

	if ( kapp && !kapp->startupId().isNull()) {
	    argv[i++] = qstrdup("--startupid");
	    argv[i++] = qstrdup(kapp->startupId());
	}

        // NULL terminated list
        argv[i++] = NULL;

        setgid(getgid());
        setuid(getuid());

        execvp("drkonqi", argv);

        // we could clean up here
        // i = 0;
        // while (argv[i])
        //   free(argv[i++]);
      }
      else
      {
        // Close dcop connections
        DCOPClient::emergencyClose();
        // Close all remaining file descriptors
        struct rlimit rlp;
        getrlimit(RLIMIT_NOFILE, &rlp);
        for (int i = 0; i < (int)rlp.rlim_cur; i++)
           close(i);
           
        alarm(0); // Seems we made it....

        // wait for child to exit
        waitpid(pid, NULL, 0);
        _exit(253);
      }
    }
    else {
      fprintf(stderr, "Unknown appname\n");
    }
  }
   
  if (crashRecursionCounter < 4)
  {
     fprintf(stderr, "Unable to start dr. konqi\n");
  }
  _exit(255);
}
