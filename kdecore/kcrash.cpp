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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
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
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>

#include <qwindowdefs.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kapplication.h>
#include <dcopclient.h>

#include <../kinit/klauncher_cmds.h>

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

  // Close all remaining file descriptors except for stdin/stdout/stderr
  struct rlimit rlp;
  getrlimit(RLIMIT_NOFILE, &rlp);
  for (int i = 3; i < (int)rlp.rlim_cur; i++)
    close(i);


  // this code is leaking, but this should not hurt cause we will do a
  // exec() afterwards. exec() is supposed to clean up.
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

          const char * argv[24]; // don't forget to update this
          int i = 0;

          // argument 0 has to be drkonqi
          argv[i++] = "drkonqi";

#if defined Q_WS_X11
          // start up on the correct display
          argv[i++] = "-display";
          if ( QX11Info::display() )
            argv[i++] = XDisplayString(QX11Info::display());
          else
            argv[i++] = getenv("DISPLAY");
#elif defined(Q_WS_QWS)
          // start up on the correct display
          argv[i++] = "-display";
          argv[i++] = getenv("QWS_DISPLAY");
#endif

          // we have already tested this
          argv[i++] = "--appname";
          argv[i++] = s_appName;
          if (KApplication::loadedByKdeinit)
            argv[i++] = "--kdeinit";

          // only add apppath if it's not NULL
          if (s_appPath && *s_appPath) {
            argv[i++] = "--apppath";
            argv[i++] = s_appPath;
          }

          // signal number -- will never be NULL
          char sigtxt[ 10 ];
          sprintf( sigtxt, "%d", sig );
          argv[i++] = "--signal";
          argv[i++] = sigtxt;

          char pidtxt[ 10 ];
          sprintf( pidtxt, "%d", getpid());
          argv[i++] = "--pid";
          argv[i++] = pidtxt;

          const KInstance *instance = KGlobal::instance();
          const KAboutData *about = instance ? instance->aboutData() : 0;
          if (about) {
            if (about->internalVersion()) {
              argv[i++] = "--appversion";
              argv[i++] = about->internalVersion();
            }

            if (about->internalProgramName()) {
              argv[i++] = "--programname";
              argv[i++] = about->internalProgramName();
            }

            if (about->internalBugAddress()) {
              argv[i++] = "--bugaddress";
              argv[i++] = about->internalBugAddress();
            }
          }

          if ( kapp && !kapp->startupId().isNull()) {
            argv[i++] = "--startupid";
            argv[i++] = kapp->startupId().data();
          }

          if ( s_safer )
            argv[i++] = "--safer";

          // NULL terminated list
          argv[i] = NULL;

          startDrKonqi( argv, i );

          _exit(253);
      }
      else {
        fprintf(stderr, "Unknown appname\n");
      }
    }

    if (crashRecursionCounter < 4)
    {
      fprintf(stderr, "Unable to start Dr. Konqi\n");
    }
#endif //Q_OS_UNIX

  _exit(255);
}

#ifdef Q_OS_UNIX

// Since we can't fork() in the crashhandler, we cannot execute any external code
// (there can be functions registered to be performed before fork(), for example
// handling of malloc locking, which doesn't work when malloc crashes because of heap corruption).

static int write_socket(int sock, char *buffer, int len);
static int read_socket(int sock, char *buffer, int len);
static int openSocket();

void KCrash::startDrKonqi( const char* argv[], int argc )
{
  int socket = openSocket();
  if( socket < -1 )
  {
    startDirectly( argv, argc );
    return;
  }
  klauncher_header header;
  header.cmd = LAUNCHER_EXEC_NEW;
  const int BUFSIZE = 8192; // make sure this is big enough
  char buffer[ BUFSIZE + 10 ];
  int pos = 0;
  long argcl = argc;
  memcpy( buffer + pos, &argcl, sizeof( argcl ));
  pos += sizeof( argcl );
  for( int i = 0;
       i < argc;
       ++i )
  {
    int len = strlen( argv[ i ] ) + 1; // include terminating \0
    if( pos + len > BUFSIZE )
    {
      fprintf( stderr, "BUFSIZE in KCrash not big enough!\n" );
      startDirectly( argv, argc );
      return;
    }
    memcpy( buffer + pos, argv[ i ], len );
    pos += len;
  }
  long env = 0;
  memcpy( buffer + pos, &env, sizeof( env ));
  pos += sizeof( env );
  long avoid_loops = 0;
  memcpy( buffer + pos, &avoid_loops, sizeof( avoid_loops ));
  pos += sizeof( avoid_loops );
  header.arg_length = pos;
  write_socket(socket, (char *) &header, sizeof(header));
  write_socket(socket, buffer, pos);
  if( read_socket( socket, (char *) &header, sizeof(header)) < 0
      || header.cmd != LAUNCHER_OK )
  {
    startDirectly( argv, argc );
    return;
  }
  long pid;
  read_socket(socket, buffer, header.arg_length);
  pid = *((long *) buffer);

  alarm(0); // Seems we made it....

  for(;;)
  {
    if( kill( pid, 0 ) < 0 )
      _exit(253);
    sleep( 1 ); // the debugger should stop this process anyway
  }
}

// If we can't reach kdeinit we can still at least try to fork()
void KCrash::startDirectly( const char* argv[], int )
{
  fprintf( stderr, "KCrash cannot reach kdeinit, launching directly.\n" );
  pid_t pid = fork();
  if (pid <= 0)
  {
    setgid(getgid());
    setuid(getuid());
    execvp("drkonqi", const_cast< char** >( argv ));
  }
  else
  {
    alarm(0); // Seems we made it....
    // wait for child to exit
    waitpid(pid, NULL, 0);
    _exit(253);
  }
}

// From now on this code is copy&pasted from kinit/wrapper.c :

extern char **environ;

static char *getDisplay()
{
   const char *display;
   char *result;
   char *screen;
   char *colon;
   char *i;
/*
 don't test for a value from qglobal.h but instead distinguish
 Qt/X11 from Qt/Embedded by the fact that Qt/E apps have -DQWS
 on the commandline (which in qglobal.h however triggers Q_WS_QWS,
 but we don't want to include that here) (Simon)
#ifdef Q_WS_X11
 */
#if !defined(QWS)
   display = getenv("DISPLAY");
#else
   display = getenv("QWS_DISPLAY");
#endif
   if (!display || !*display)
   {
      display = ":0";
   }
   result = (char*)malloc(strlen(display)+1);
   if (result == NULL)
      return NULL;

   strcpy(result, display);
   screen = strrchr(result, '.');
   colon = strrchr(result, ':');
   if (screen && (screen > colon))
      *screen = '\0';
   while((i = strchr(result, ':')))
     *i = '_';
   return result;
}

/*
 * Write 'len' bytes from 'buffer' into 'sock'.
 * returns 0 on success, -1 on failure.
 */
static int write_socket(int sock, char *buffer, int len)
{
  ssize_t result;
  int bytes_left = len;
  while ( bytes_left > 0)
  {
     result = write(sock, buffer, bytes_left);
     if (result > 0)
     {
        buffer += result;
        bytes_left -= result;
     }
     else if (result == 0)
        return -1;
     else if ((result == -1) && (errno != EINTR) && (errno != EAGAIN))
        return -1;
  }
  return 0;
}

/*
 * Read 'len' bytes from 'sock' into 'buffer'.
 * returns 0 on success, -1 on failure.
 */
static int read_socket(int sock, char *buffer, int len)
{
  ssize_t result;
  int bytes_left = len;
  while ( bytes_left > 0)
  {
     result = read(sock, buffer, bytes_left);
     if (result > 0)
     {
        buffer += result;
        bytes_left -= result;
     }
     else if (result == 0)
        return -1;
     else if ((result == -1) && (errno != EINTR) && (errno != EAGAIN))
        return -1;
  }
  return 0;
}

static int openSocket()
{
  kde_socklen_t socklen;
  int s;
  struct sockaddr_un server;
#define MAX_SOCK_FILE 255
  char sock_file[MAX_SOCK_FILE + 1];
  const char *home_dir = getenv("HOME");
  const char *kde_home = getenv("KDEHOME");
  char *display;

  sock_file[0] = sock_file[MAX_SOCK_FILE] = 0;

  if (!kde_home || !kde_home[0])
  {
     kde_home = "~/.kde/";
  }

  if (kde_home[0] == '~')
  {
     if (!home_dir || !home_dir[0])
     {
        fprintf(stderr, "Warning: $HOME not set!\n");
        return -1;
     }
     if (strlen(home_dir) > (MAX_SOCK_FILE-100))
     {
        fprintf(stderr, "Warning: Home directory path too long!\n");
        return -1;
     }
     kde_home++;
     strncpy(sock_file, home_dir, MAX_SOCK_FILE);
  }
  strncat(sock_file, kde_home, MAX_SOCK_FILE - strlen(sock_file));

  /** Strip trailing '/' **/
  if ( sock_file[strlen(sock_file)-1] == '/')
     sock_file[strlen(sock_file)-1] = 0;
  
  strncat(sock_file, "/socket-", MAX_SOCK_FILE - strlen(sock_file));
  if (gethostname(sock_file+strlen(sock_file), MAX_SOCK_FILE - strlen(sock_file) - 1) != 0)
  {
     perror("Warning: Could not determine hostname: ");
     return -1;
  }
  sock_file[sizeof(sock_file)-1] = '\0';

  /* append $DISPLAY */
  display = getDisplay();
  if (display == NULL)
  {
     fprintf(stderr, "Error: Could not determine display.\n");
     return -1;
  }

  if (strlen(sock_file)+strlen(display)+strlen("/kdeinit_")+2 > MAX_SOCK_FILE)
  {
     fprintf(stderr, "Warning: Socket name will be too long.\n");
     return -1;
  }
  strcat(sock_file, "/kdeinit_");
  strcat(sock_file, display);
  free(display);

  if (strlen(sock_file) >= sizeof(server.sun_path))
  {
     fprintf(stderr, "Warning: Path of socketfile exceeds UNIX_PATH_MAX.\n");
     return -1;
  }

  /*
   * create the socket stream
   */
  s = socket(PF_UNIX, SOCK_STREAM, 0);
  if (s < 0) 
  {
     perror("Warning: socket() failed: ");
     return -1;
  }

  server.sun_family = AF_UNIX;
  strcpy(server.sun_path, sock_file);
  socklen = sizeof(server);
  if(connect(s, (struct sockaddr *)&server, socklen) == -1) 
  {
     perror("Warning: connect() failed: ");
     close(s);
     return -1;
  }
  return s;
}

#endif // Q_OS_UNIX
