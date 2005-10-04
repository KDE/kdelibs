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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * This file is used to catch signals which would normally
 * crash the application (like segmentation fault, floating
 * point exception and such).
 */

#include "config.h"

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

#if defined Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#endif

static KCrash::HandlerType s_emergencySaveFunction = 0;
static KCrash::HandlerType s_crashHandler = 0;
static char *s_appName = 0;
static char *s_appPath = 0;
static bool s_safer = false;

// This function sets the function which should be called when the
// application crashes and the
// application is asked to try to save its data.
void
KCrash::setEmergencySaveFunction (HandlerType saveFunction)
{
  s_emergencySaveFunction = saveFunction;

  /*
   * We need at least the default crash handler for
   * emergencySaveFunction to be called
   */
  if (s_emergencySaveFunction && !s_crashHandler)
    s_crashHandler = defaultCrashHandler;
}

KCrash::HandlerType
KCrash::emergencySaveFunction()
{
	return s_emergencySaveFunction;
}

void
KCrash::setSafer( bool on )
{
	s_safer = on;
}

void
KCrash::setApplicationPath(const QString& path)
{
	s_appPath = qstrdup(path.toLatin1().data());
}

void
KCrash::setApplicationName(const QString& name)
{
	s_appName = qstrdup(name.toLatin1().data());
}

// This function sets the function which should be responsible for
// the application crash handling.
void
KCrash::setCrashHandler (HandlerType handler)
{
#ifdef Q_OS_UNIX
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
#endif //Q_OS_UNIX

  s_crashHandler = handler;
}

KCrash::HandlerType
KCrash::crashHandler()
{
	return s_crashHandler;
}

void
KCrash::defaultCrashHandler (int sig)
{
#ifdef Q_OS_UNIX
  // WABA: Do NOT use kdDebug() in this function because it is much too risky!
  // Handle possible recursions
  static int crashRecursionCounter = 0;
  crashRecursionCounter++; // Nothing before this, please !

  signal(SIGALRM, SIG_DFL);
  alarm(3); // Kill me... (in case we deadlock in malloc)

  if (crashRecursionCounter < 2) {
    if (s_emergencySaveFunction) {
      s_emergencySaveFunction (sig);
    }
    crashRecursionCounter++; //
  }

        // Close dcop connections
  DCOPClient::emergencyClose();
  // Close all remaining file descriptors except for stdin/stdout/stderr
  struct rlimit rlp;
  getrlimit(RLIMIT_NOFILE, &rlp);
  for (int i = 3; i < (int)rlp.rlim_cur; i++)
    close(i);

  bool shuttingDown = false;

  // don't load drkonqi during shutdown
  if ( !shuttingDown )
  {
    if (crashRecursionCounter < 3)
    {
      if (s_appName)
      {
#ifndef NDEBUG
        fprintf(stderr, "KCrash: crashing... crashRecursionCounter = %d\n",
		crashRecursionCounter);
        fprintf(stderr, "KCrash: Application Name = %s path = %s pid = %d\n",
		s_appName ? s_appName : "<unknown>" ,
		s_appPath ? s_appPath : "<unknown>", getpid());
#else
        fprintf(stderr, "KCrash: Application '%s' crashing...\n",
		s_appName ? s_appName : "<unknown>");
#endif

        pid_t pid = fork();

        if (pid <= 0) {
          // this code is leaking, but this should not hurt cause we will do a
          // exec() afterwards. exec() is supposed to clean up.
          char * argv[24]; // don't forget to update this
          int i = 0;

          // argument 0 has to be drkonqi
          argv[i++] = qstrdup("drkonqi");

#if defined Q_WS_X11
          // start up on the correct display
          argv[i++] = qstrdup("-display");
          if ( QX11Info::display() )
            argv[i++] = XDisplayString(QX11Info::display());
          else
            argv[i++] = getenv("DISPLAY");
#elif defined(Q_WS_QWS)
          // start up on the correct display
          argv[i++] = qstrdup("-display");
          argv[i++] = getenv("QWS_DISPLAY");
#endif

          // we have already tested this
          argv[i++] = qstrdup("--appname");
          argv[i++] = qstrdup(s_appName);
          if (KApplication::loadedByKdeinit)
            argv[i++] = qstrdup("--kdeinit");

          // only add apppath if it's not NULL
          if (s_appPath && *s_appPath) {
            argv[i++] = qstrdup("--apppath");
            argv[i++] = qstrdup(s_appPath);
          }

          // signal number -- will never be NULL
          argv[i++] = qstrdup("--signal");
          argv[i++] = qstrdup(QByteArray::number(sig).data());

          // pid number -- only include if this is the child
          // the debug stuff will be disabled if we were not able to fork
          if (pid == 0) {
            argv[i++] = qstrdup("--pid");
            argv[i++] = qstrdup(QByteArray::number(getppid()).data());
          }

          const KInstance *instance = KGlobal::instance();
          const KAboutData *about = instance ? instance->aboutData() : 0;
          if (about) {
            if (!about->version().isNull()) {
              argv[i++] = qstrdup("--appversion");
              argv[i++] = qstrdup(about->version().toLatin1());
            }

            if (!about->programName().isNull()) {
              argv[i++] = qstrdup("--programname");
              argv[i++] = qstrdup(about->programName().toLatin1());
            }

            if (!about->bugAddress().isNull()) {
              argv[i++] = qstrdup("--bugaddress");
              argv[i++] = qstrdup(about->bugAddress().toLatin1());
            }
          }

          if ( kapp && !kapp->startupId().isNull()) {
            argv[i++] = qstrdup("--startupid");
            argv[i++] = qstrdup(kapp->startupId());
          }

          if ( s_safer )
            argv[i++] = qstrdup("--safer");

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
      fprintf(stderr, "Unable to start Dr. Konqi\n");
    }
  }
#endif //Q_OS_UNIX

  _exit(255);
}
