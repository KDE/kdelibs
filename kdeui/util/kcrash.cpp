/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Timo Hummel <timo.hummel@sap.com>
 *                    Tom Braun <braunt@fh-konstanz.de>
 * Copyright 2009 KDE e.V.
 *   By Adriaan de Groot <groot@kde.org>
 * Copyright (C) 2010 George Kiagiadakis <kiagiadakis.george@gmail.com>
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

#include "kcrash.h"
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <config-kstandarddirs.h>

#include <config.h>

#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>

#include <qwindowdefs.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kapplication.h>

#include <../kinit/klauncher_cmds.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#if defined Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#endif

#ifdef Q_OS_SOLARIS
// Solaris has built-in, thread-safe, async-signal-safe, mechanisms
// to walk the stack in the case of a crash, as well as (optionally)
// to demangle C++ symbol names. In the case of a crash, dump a stack
// trace to stderr before starting drKonqui (because what drKonqui is
// going to do is -- through a complicated process -- print the
// exact same information, but less reliably).
#include <ucontext.h>
#endif

static KCrash::HandlerType s_emergencySaveFunction = 0;
static KCrash::HandlerType s_crashHandler = 0;
static char *s_appName = 0;
static char *s_autoRestartCommand = 0;
static char *s_appPath = 0;
static char *s_drkonqiPath = 0;
static KCrash::CrashFlags s_flags = 0;
static bool s_launchDrKonqi = false;

namespace KCrash
{
    void startProcess(int argc, const char *argv[], bool waitAndExit);
}

void
KCrash::setEmergencySaveFunction (HandlerType saveFunction)
{
  s_emergencySaveFunction = saveFunction;

  /*
   * We need at least the default crash handler for
   * emergencySaveFunction to be called
   */
  if (s_emergencySaveFunction && !s_crashHandler) {
      setCrashHandler(defaultCrashHandler);
  }
}

KCrash::HandlerType
KCrash::emergencySaveFunction()
{
    return s_emergencySaveFunction;
}

// Set the default crash handler in 10 seconds
// This is used after an autorestart, the second instance of the application
// is started with --nocrashhandler (no drkonqi, more precisely), and we
// set the defaultCrashHandler (to handle autorestart) after 10s.
// The delay is to see if we stay up for more than 10s time, to avoid infinite
// respawning if the app crashes on startup.
class KCrashDelaySetHandler : public QObject
{
public:
    KCrashDelaySetHandler() {
        startTimer(10000); // 10 s
    }
protected:
    void timerEvent(QTimerEvent *event) {
        if (!s_crashHandler) // not set meanwhile
            KCrash::setCrashHandler(KCrash::defaultCrashHandler);
        killTimer(event->timerId());
        this->deleteLater();
    }
};



void
KCrash::setFlags(KCrash::CrashFlags flags)
{
    s_flags = flags;
    if (s_flags & AutoRestart) {
        // We need at least the default crash handler for autorestart to work.
        if (!s_crashHandler) {
            KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
            if (!args->isSet("crashhandler")) // --nocrashhandler was passed, probably due to a crash, delay restart handler
                new KCrashDelaySetHandler;
            else // probably because KDE_DEBUG=1. set restart handler immediately.
                setCrashHandler(defaultCrashHandler);
        }
    }
}

//### KDE5:Consider merging the setApplicationPath and setApplicationName methods into one.
void
KCrash::setApplicationPath(const QString& path)
{
    s_appPath = qstrdup(QFile::encodeName(path).constData());

    //if the appName has also been specified, update s_autoRestartCommand to be in the form "absolutePath/appName"
    if (s_appName) {
        delete[] s_autoRestartCommand;
        QFileInfo appExecutable(QDir(path), QFile::decodeName(s_appName));
        QByteArray cmd = QFile::encodeName(appExecutable.absoluteFilePath());
        s_autoRestartCommand = qstrdup(cmd.constData());
    }
}

void
KCrash::setApplicationName(const QString& name)
{
    s_appName = qstrdup(QFile::encodeName(name).constData());

    //update the autoRestartCommand
    delete[] s_autoRestartCommand;
    if (s_appPath) {
        //if we have appPath, make autoRestartCommand be in the form "absolutePath/appName"...
        QFileInfo appExecutable(QDir(QFile::decodeName(s_appPath)), name);
        QByteArray cmd = QFile::encodeName(appExecutable.absoluteFilePath());
        s_autoRestartCommand = qstrdup(cmd.constData());
    } else {
        //...else just use the appName for the autoRestartCommand
        s_autoRestartCommand = qstrdup(s_appName);
    }
}

void KCrash::setDrKonqiEnabled(bool enabled)
{
    s_launchDrKonqi = enabled;
    if (s_launchDrKonqi && !s_drkonqiPath) {
        s_drkonqiPath = qstrdup(QFile::encodeName(KStandardDirs::findExe("drkonqi")).constData());
        if (!s_drkonqiPath) {
            kError() << "Could not find drkonqi";
            s_launchDrKonqi = false;
        }
    }

    //we need at least the default crash handler to launch drkonqi
    if (s_launchDrKonqi && !s_crashHandler) {
        setCrashHandler(defaultCrashHandler);
    }
}

bool KCrash::isDrKonqiEnabled()
{
    return s_launchDrKonqi;
}

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
#ifdef SIGBUS
  signal (SIGBUS, handler);
  sigaddset(&mask, SIGBUS);
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

static void
closeAllFDs()
{
  // Close all remaining file descriptors except for stdin/stdout/stderr
  struct rlimit rlp;
  getrlimit(RLIMIT_NOFILE, &rlp);
  for (int i = 3; i < (int)rlp.rlim_cur; i++)
    close(i);
}

void
KCrash::defaultCrashHandler (int sig)
{
#ifdef Q_OS_UNIX
    // WABA: Do NOT use kDebug() in this function because it is much too risky!
    // Handle possible recursions
    static int crashRecursionCounter = 0;
    crashRecursionCounter++; // Nothing before this, please !

    signal(SIGALRM, SIG_DFL);
    alarm(3); // Kill me... (in case we deadlock in malloc)

#ifdef Q_OS_SOLARIS
    (void) printstack(2 /* stderr, assuming it's still open. */);
#endif

    if (crashRecursionCounter < 2) {
        if (s_emergencySaveFunction) {
            s_emergencySaveFunction (sig);
        }
        if ((s_flags & AutoRestart) && s_autoRestartCommand) {
            sleep(1);
            const char *restartArgv[3] = { s_autoRestartCommand, "--nocrashhandler", NULL };
            startProcess(2, restartArgv, false);
        }
        crashRecursionCounter++;
    }

    if (!(s_flags & KeepFDs))
        closeAllFDs();
#if defined(Q_WS_X11)
    else if (QX11Info::display())
        close(ConnectionNumber(QX11Info::display()));
#endif

    if (crashRecursionCounter < 3)
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

        if (!s_launchDrKonqi) {
            setCrashHandler(0);
            raise(sig); // dump core, or whatever is the default action for this signal.
            return;
        }

        const char * argv[25]; // don't forget to update this
        int i = 0;

        // argument 0 has to be drkonqi
        argv[i++] = s_drkonqiPath;

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

        argv[i++] = "--appname";
        argv[i++] = s_appName ? s_appName : "<unknown>";

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

        const KComponentData componentData = KGlobal::mainComponent();
        const KAboutData *about = componentData.isValid() ? componentData.aboutData() : 0;
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

        char sidtxt[256];
        if ( kapp && !kapp->startupId().isNull()) {
            argv[i++] = "--startupid";
            strlcpy(sidtxt, kapp->startupId().constData(), sizeof(sidtxt));
            argv[i++] = sidtxt;
        }

        if ( s_flags & SaferDialog )
            argv[i++] = "--safer";

        if ((s_flags & AutoRestart) && s_autoRestartCommand)
            argv[i++] = "--restarted"; //tell drkonqi if the app has been restarted

        // NULL terminated list
        argv[i] = NULL;

        startProcess(i, argv, true);
    }

    if (crashRecursionCounter < 4)
    {
      fprintf(stderr, "Unable to start Dr. Konqi\n");
    }
#endif //Q_OS_UNIX

    _exit(255);
}

#ifdef Q_OS_UNIX

static bool startProcessInternal(int argc, const char *argv[], bool waitAndExit, bool directly);
static pid_t startFromKdeinit(int argc, const char *argv[]);
static pid_t startDirectly(const char *argv[]);
static int write_socket(int sock, char *buffer, int len);
static int read_socket(int sock, char *buffer, int len);
static int openSocket();

void KCrash::startProcess(int argc, const char *argv[], bool waitAndExit)
{
    bool startDirectly = true;

    // First try to start the app via kdeinit, if the AlwaysDirectly flag hasn't been specified.
    // This is done because it is dangerous to use fork() in the crash handler
    // (there can be functions registered to be performed before fork(), for example handling
    // of malloc locking, which doesn't work when malloc crashes because of heap corruption).
    if (!(s_flags & AlwaysDirectly)) {
        startDirectly = !startProcessInternal(argc, argv, waitAndExit, false);
    }

    // If we can't reach kdeinit, we can still at least try to fork()
    if (startDirectly) {
        startProcessInternal(argc, argv, waitAndExit, true);
    }
}

static bool startProcessInternal(int argc, const char *argv[], bool waitAndExit, bool directly)
{
    fprintf(stderr, "KCrash: Attempting to start %s %s\n", argv[0], directly ? "directly" : "from kdeinit");

    pid_t pid = directly ? startDirectly(argv) : startFromKdeinit(argc, argv);

    if (pid > 0 && waitAndExit) {
        // Seems we made it....
        alarm(0); //stop the pending alarm that was set at the top of the defaultCrashHandler

        // Wait forever until the started process exits. This code path is executed
        // when launching drkonqi. Note that drkonqi will stop this process in the meantime.
        if (directly) {
            //if the process was started directly, use waitpid(), as it's a child...
            while(waitpid(-1, NULL, 0) != pid) {}
        } else {
            //...else poll its status using kill()
            while(kill(pid, 0) >= 0) {
                sleep(1);
            }
        }
        _exit(253);
    }

    return (pid > 0); //return true on success
}

static pid_t startFromKdeinit(int argc, const char *argv[])
{
  int socket = openSocket();
  if( socket < -1 )
    return 0;
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
    if( pos + len >= BUFSIZE )
    {
      fprintf( stderr, "BUFSIZE in KCrash not big enough!\n" );
      return 0;
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
    return 0;
  }
  long pid;
  read_socket(socket, buffer, header.arg_length);
  pid = *((long *) buffer);
  return static_cast<pid_t>(pid);
}

static pid_t startDirectly(const char *argv[])
{
  pid_t pid = fork();
  switch (pid)
  {
  case -1:
    fprintf( stderr, "KCrash failed to fork(), errno = %d\n", errno );
    return 0;
  case 0:
    if (setgid(getgid()) < 0 || setuid(getuid()) < 0)
      _exit(253); // This cannot happen. Theoretically.
    closeAllFDs(); // We are in the child now. Close FDs unconditionally.
    execvp(argv[0], const_cast< char** >(argv));
    fprintf( stderr, "KCrash failed to exec(), errno = %d\n", errno );
    _exit(253);
  default:
    return pid;
  }
}

// From now on this code is copy&pasted from kinit/wrapper.c :

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
#ifdef NO_DISPLAY
   display = "NODISPLAY";
#elif !defined(QWS)
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
#ifdef __APPLE__
   while((i = strchr(result, '/')))
     *i = '_';
#endif
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
     kde_home = "~/" KDE_DEFAULT_HOME "/";
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
     strlcpy(sock_file, home_dir, MAX_SOCK_FILE);
  }
  strlcat(sock_file, kde_home, MAX_SOCK_FILE);

  /** Strip trailing '/' **/
  if ( sock_file[strlen(sock_file)-1] == '/')
     sock_file[strlen(sock_file)-1] = 0;

  strlcat(sock_file, "/socket-", MAX_SOCK_FILE);
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

  if (strlen(sock_file)+strlen(display)+strlen("/kdeinit4_")+2 > MAX_SOCK_FILE)
  {
     fprintf(stderr, "Warning: Socket name will be too long.\n");
     free(display);
     return -1;
  }
  strcat(sock_file, "/kdeinit4_");
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
  printf("sock_file=%s\n", sock_file);
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
