/*
 * This file is part of the KDE libraries
 * Copyright (c) 1999-2000 Waldo Bastian <bastian@kde.org>
 *           (c) 1999 Mario Weilguni <mweilguni@sime.com>
 *           (c) 2001 Lubos Lunak <l.lunak@kde.org>
 *
 * $Id$
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		// Needed on some systems.
#endif

#include <errno.h>
#include <fcntl.h>
#include <setproctitle.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <qstring.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klibloader.h>
#include <kapplication.h>
#include <klocale.h>
#include <kstartupinfo.h>

#include "ltdl.h"
#include "klauncher_cmds.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#ifdef RTLD_GLOBAL
# define LTDL_GLOBAL	RTLD_GLOBAL
#else
# ifdef DL_GLOBAL
#  define LTDL_GLOBAL	DL_GLOBAL
# else
#  define LTDL_GLOBAL	0
# endif
#endif

extern char **environ;

extern int lt_dlopen_flag;
#ifdef Q_WS_X11
static int X11fd = -1;
static Display *X11display = 0;
static int X11_startup_notify_fd = -1;
static Display *X11_startup_notify_display = 0;
#endif
static const KInstance *s_instance = 0;
#define MAX_SOCK_FILE 255
static char sock_file[MAX_SOCK_FILE];
static Atom net_current_desktop;

#ifdef Q_WS_X11
#define DISPLAY "DISPLAY"
#elif defined(Q_WS_QWS)
#define DISPLAY "QWS_DISPLAY"
#else
#error Use QT/X11 or QT/Embedded
#endif

/* Group data */
struct {
  int maxname;
  int fd[2];
  int launcher[2]; /* socket pair for launcher communication */
  int deadpipe[2]; /* pipe used to detect dead children */
  int initpipe[2];
  int wrapper; /* socket for wrapper communication */
  char result;
  int exit_status;
  pid_t fork;
  pid_t launcher_pid;
  pid_t my_pid;
  int n;
  lt_dlhandle handle;
  lt_ptr sym;
  char **argv;
  int (*func)(int, char *[]);
  int (*launcher_func)(int);
  bool debug_wait;
  int lt_dlopen_flag;
  QCString errorMsg;
} d;

extern "C" {
int kdeinit_xio_errhandler( Display * );
}

/*
 * Close fd's which are only useful for the parent process.
 * Restore default signal handlers.
 */
static void close_fds()
{
   if (d.deadpipe[0] != -1)
   {
      close(d.deadpipe[0]);
      d.deadpipe[0] = -1;
   }

   if (d.deadpipe[1] != -1)
   {
      close(d.deadpipe[1]);
      d.deadpipe[1] = -1;
   }

   if (d.initpipe[0] != -1)
   {
      close(d.initpipe[0]);
      d.initpipe[0] = -1;
   }

   if (d.initpipe[1] != -1)
   {
      close(d.initpipe[1]);
      d.initpipe[1] = -1;
   }

   if (d.launcher_pid)
   {
      close(d.launcher[0]);
      d.launcher_pid = 0;
   }
   if (d.wrapper)
   {
      close(d.wrapper);
      d.wrapper = 0;
   }
#ifdef Q_WS_X11
   if (X11fd >= 0)
   {
      close(X11fd);
      X11fd = -1;
   }
   if (X11_startup_notify_fd >= 0 && X11_startup_notify_fd != X11fd )
   {
      close(X11_startup_notify_fd);
      X11_startup_notify_fd = -1;
   }
#endif

   signal(SIGCHLD, SIG_DFL);
   signal(SIGPIPE, SIG_DFL);
}

static void exitWithErrorMsg(const QString &errorMsg)
{
   QCString utf8ErrorMsg = errorMsg.utf8();
   d.result = 3; // Error with msg
   write(d.fd[1], &d.result, 1);
   int l = utf8ErrorMsg.length();
   write(d.fd[1], &l, sizeof(int));
   write(d.fd[1], utf8ErrorMsg.data(), l);
   close(d.fd[1]);
   exit(255);
}

static void setup_tty( const char* tty )
{
    if( tty == NULL || *tty == '\0' )
        return;
    int fd = open( tty, O_WRONLY );
    if( fd < 0 )
    {
        perror( "kdeinit: couldn't open() tty" );
        return;
    }
    if( dup2( fd, STDOUT_FILENO ) < 0 )
    {
        perror( "kdeinit: couldn't dup2() tty" );
        close( fd );
        return;
    }
    if( dup2( fd, STDERR_FILENO ) < 0 )
    {
        perror( "kdeinit: couldn't dup2() tty" );
        close( fd );
        return;
    }
    close( fd );
}

// from kdecore/netwm.cpp
static int get_current_desktop( Display* disp )
{
#ifdef Q_WS_X11 // Only X11 supports multiple desktops
    Atom type_ret;
    int format_ret;
    unsigned char *data_ret;
    unsigned long nitems_ret, unused;
    if( XGetWindowProperty( disp, DefaultRootWindow( disp ), net_current_desktop,
        0l, 1l, False, XA_CARDINAL, &type_ret, &format_ret, &nitems_ret, &unused, &data_ret )
	    == Success)
    {
	if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 1)
	    return *((long *) data_ret) + 1;
    }
#endif
    return 0;
}

// var has to be e.g. "DISPLAY=", i.e. with =
const char* get_env_var( const char* var, int envc, const char* envs )
{
    if( envc > 0 )
    { // get the var from envs
        const char* env_l = envs;
        int ln = strlen( var );
        for (int i = 0;  i < envc; i++)
        {
            if( strncmp( env_l, var, ln ) == 0 )
                return env_l + ln;
            while(*env_l != 0) env_l++;
                env_l++;
        }
    }
    return NULL;
}

#ifdef Q_WS_X11 // FIXME(E): Implement for Qt/Embedded
static void init_startup_info( KStartupInfoId& id, const char* bin,
    int envc, const char* envs )
{
    const char* dpy = get_env_var( DISPLAY"=", envc, envs );
    // this may be called in a child, so it can't use display open using X11display
    // also needed for multihead
    X11_startup_notify_display = XOpenDisplay( dpy );
    if( X11_startup_notify_display == NULL )
        return;
    X11_startup_notify_fd = XConnectionNumber( X11_startup_notify_display );
    KStartupInfoData data;
    int desktop = get_current_desktop( X11_startup_notify_display );
    data.setDesktop( desktop );
    data.setBin( bin );
    KStartupInfo::sendStartupX( X11_startup_notify_display, id, data );
    XFlush( X11_startup_notify_display );
}

static void complete_startup_info( KStartupInfoId& id, pid_t pid )
{
    if( X11_startup_notify_display == NULL )
        return;
    KStartupInfoData data;
    data.addPid( pid );
    data.setHostname();
    KStartupInfo::sendChangeX( X11_startup_notify_display, id, data );
    XCloseDisplay( X11_startup_notify_display );
    X11_startup_notify_display = NULL;
    X11_startup_notify_fd = -1;
}
#endif

QCString execpath_avoid_loops( const QCString& exec, int envc, const char* envs, bool avoid_loops )
{
     QStringList paths;
     if( envc > 0 ) /* use the passed environment */
     {
         const char* path = get_env_var( "PATH=", envc, envs );
         if( path != NULL )
             paths = QStringList::split( QRegExp( "[:\b]" ), path, true );
     }
     else
         paths = QStringList::split( QRegExp( "[:\b]" ), getenv( "PATH" ), true );
     QCString execpath = QFile::encodeName(
         s_instance->dirs()->findExe( exec, paths.join( QString( ":" ))));
     if( avoid_loops && !execpath.isEmpty())
     {
         int pos = execpath.findRev( '/' );
         QString bin_path = execpath.left( pos );
         for( QStringList::Iterator it = paths.begin();
              it != paths.end();
              ++it )
             if( ( *it ) == bin_path || ( *it ) == bin_path + '/' )
             {
                 paths.remove( it );
                 break; // -->
             }
         execpath = QFile::encodeName(
             s_instance->dirs()->findExe( exec, paths.join( QString( ":" ))));
     }
     return execpath;
}

static pid_t launch(int argc, const char *_name, const char *args,
                    const char *cwd=0, int envc=0, const char *envs=0,
                    bool reset_env = false,
                    const char *tty=0, bool avoid_loops = false,
                    const char* startup_id_str = "0" )
{
  int launcher = 0;
  QCString lib;
  QCString name;
  QCString exec;

  if (strcmp(_name, "klauncher") == 0) {
     /* klauncher is launched in a special way:
      * instead of calling 'main(argc, argv)',
      * we call 'start_launcher(comm_socket)'.
      * The other end of the socket is d.launcher[0].
      */
     if (0 > socketpair(AF_UNIX, SOCK_STREAM, 0, d.launcher))
     {
        perror("kdeinit: socketpair() failed!\n");
        exit(255);
     }
     launcher = 1;
  }

  QCString libpath;
  QCString execpath;
  if (_name[0] != '/')
  {
     /* Relative name without '.la' */
     name = _name;
     lib = name + ".la";
     exec = name;
     libpath = QFile::encodeName(KLibLoader::findLibrary( lib, s_instance ));
     execpath = execpath_avoid_loops( exec, envc, envs, avoid_loops );
  }
  else
  {
     lib = _name;
     name = _name;
     name = name.mid( name.findRev('/') + 1);
     exec = _name;
     if (lib.right(3) == ".la")
        libpath = lib;
     else
        execpath = exec;
  }
  if (!args)
  {
    argc = 1;
  }

#ifdef Q_WS_X11
  KStartupInfoId startup_id;
  startup_id.initId( startup_id_str );
  if( !startup_id.none())
      init_startup_info( startup_id, name, envc, envs );
#endif

  if (0 > pipe(d.fd))
  {
     perror("kdeinit: pipe() failed!\n");
     exit(255);
  }

  d.errorMsg = 0;
  d.fork = fork();
  switch(d.fork) {
  case -1:
     perror("kdeinit: fork() failed!\n");
     exit(255);
     break;
  case 0:
     /** Child **/
     close(d.fd[0]);
     close_fds();

     if (cwd && *cwd)
        chdir(cwd);

     if( reset_env ) // KWRAPPER/SHELL
     {

         QStrList unset_envs;
         for( int tmp_env_count = 0;
              environ[tmp_env_count];
              tmp_env_count++)
             unset_envs.append( environ[ tmp_env_count ] );
         for( QStrListIterator it( unset_envs );
              it.current() != NULL ;
              ++it )
         {
             QCString tmp( it.current());
             int pos = tmp.find( '=' );
             if( pos >= 0 )
                 unsetenv( tmp.left( pos ));
         }
     }

     for (int i = 0;  i < envc; i++)
     {
        putenv((char *)envs);
        while(*envs != 0) envs++;
        envs++;
     }

#ifdef Q_WS_X11
      if( startup_id.none())
          KStartupInfo::resetStartupEnv();
      else
          startup_id.setupStartupEnv();
#endif
     {
       QCString procTitle( name );
       d.argv = (char **) malloc(sizeof(char *) * (argc+1));
       d.argv[0] = (char *) _name;
       for (int i = 1;  i < argc; i++)
       {
          d.argv[i] = (char *) args;
          procTitle += " ";
          procTitle += (char *) args;
          while(*args != 0) args++;
          args++;
       }
       d.argv[argc] = 0;

       /** Give the process a new name **/
       kdeinit_setproctitle( "%s", procTitle.data() );
     }

     d.handle = 0;
     if (libpath.isEmpty() && execpath.isEmpty())
     {
        QString errorMsg = i18n("Could not find '%1' executable.").arg(QFile::decodeName(_name));
        exitWithErrorMsg(errorMsg);
     }

     if ( !libpath.isEmpty())
     {
       d.handle = lt_dlopen( QFile::encodeName(libpath) );
       if (!d.handle )
       {
          const char * ltdlError = lt_dlerror();
          if (execpath.isEmpty())
          {
             // Error
             QString errorMsg = i18n("Could not dlopen library '%1'.\n%2").arg(QFile::decodeName(libpath))
		.arg(ltdlError ? QFile::decodeName(ltdlError) : i18n("Unknown error"));
             exitWithErrorMsg(errorMsg);
          }
          else
          {
             // Print warning
             fprintf(stderr, "Could not dlopen library %s: %s\n", lib.data(), ltdlError != 0 ? ltdlError : "(null)" );
          }
       }
     }
     lt_dlopen_flag = d.lt_dlopen_flag;
     if (!d.handle )
     {
        d.result = 2; // Try execing
        write(d.fd[1], &d.result, 1);

        // We set the close on exec flag.
        // Closing of d.fd[1] indicates that the execvp succeeded!
        fcntl(d.fd[1], F_SETFD, FD_CLOEXEC);

        setup_tty( tty );

        execvp(execpath.data(), d.argv);
        d.result = 1; // Error
        write(d.fd[1], &d.result, 1);
        close(d.fd[1]);
        exit(255);
     }

     if (!launcher)
     {
        d.sym = lt_dlsym( d.handle, "kdemain");
        if (!d.sym )
        {
           d.sym = lt_dlsym( d.handle, "main");
           if (!d.sym )
           {
              const char * ltdlError = lt_dlerror();
              fprintf(stderr, "Could not find main: %s\n", ltdlError != 0 ? ltdlError : "(null)" );
              QString errorMsg = i18n("Could not find 'main' in '%1'.\n%2").arg(libpath)
		.arg(ltdlError ? QFile::decodeName(ltdlError) : i18n("Unknown error"));
              exitWithErrorMsg(errorMsg);
           }
        }

        d.result = 0; // Success
        write(d.fd[1], &d.result, 1);
        close(d.fd[1]);

        d.func = (int (*)(int, char *[])) d.sym;
        if (d.debug_wait)
        {
           fprintf(stderr, "kdeinit: Suspending process\n"
                           "kdeinit: 'gdb kdeinit %d' to debug\n"
                           "kdeinit: 'kill -SIGCONT %d' to continue\n",
                           getpid(), getpid());
           kill(getpid(), SIGSTOP);
        }
        else
            setup_tty( tty );

	exit( d.func(argc, d.argv)); /* Launch! */
     }
     else
     {
        d.sym = lt_dlsym( d.handle, "start_launcher");
        if (!d.sym )
        {
           const char * ltdlError = lt_dlerror();
           fprintf(stderr, "Could not find start_launcher: %s\n", ltdlError != 0 ? ltdlError : "(null)" );
           d.result = 1; // Error
           write(d.fd[1], &d.result, 1);
           close(d.fd[1]);
           exit(255);
        }

        d.result = 0; // Success
        write(d.fd[1], &d.result, 1);
        close(d.fd[1]);

        d.launcher_func = (int (*)(int)) d.sym;
        close(d.launcher[0]); // Close non-used socket.

        exit( d.launcher_func( d.launcher[1] )); /* Launch! */
     }

     break;
  default:
     /** Parent **/
     close(d.fd[1]);
     if (launcher)
     {
        close(d.launcher[1]);
        d.launcher_pid = d.fork;
     }
     bool exec = false;
     for(;;)
     {
       d.n = read(d.fd[0], &d.result, 1);
       if (d.n == 1)
       {
          if (d.result == 2)
          {
#ifndef NDEBUG
             fprintf(stderr, "Could not load library! Trying exec....\n");
#endif
             exec = true;
             continue;
          }
          if (d.result == 3)
          {
             int l = 0;
             d.n = read(d.fd[0], &l, sizeof(int));
             if (d.n == sizeof(int))
             {
                QCString tmp;
                tmp.resize(l+1);
                d.n = read(d.fd[0], tmp.data(), l);
                tmp[l] = 0;
                if (d.n == l)
                   d.errorMsg = tmp;
             }
          }
          // Finished
          break;
       }
       if (d.n == -1)
       {
          if (errno == ECHILD) {  // a child died.
             continue;
          }
          if (errno == EINTR || errno == EAGAIN) { // interrupted or more to read
             continue;
          }
       }
       if (exec)
       {
          d.result = 0;
          break;
       }
       if (d.n == 0)
       {
          perror("kdeinit: Pipe closed unexpectedly");
          d.result = 1; // Error
          break;
       }
       perror("kdeinit: Error reading from pipe");
       d.result = 1; // Error
       break;
     }
     close(d.fd[0]);
     if (launcher && (d.result == 0))
     {
        // Trader launched successfull
        d.launcher_pid = d.fork;
     }
  }
#ifdef Q_WS_X11
  if( !startup_id.none())
      complete_startup_info( startup_id, d.fork );
#endif
  return d.fork;
}

static void sig_child_handler(int)
{
   /*
    * Write into the pipe of death.
    * This way we are sure that we return from the select()
    *
    * A signal itself causes select to return as well, but
    * this creates a race-condition in case the signal arrives
    * just before we enter the select.
    */
   char c = 0;
   write(d.deadpipe[1], &c, 1);
}

static void init_signals()
{
  struct sigaction act;
  long options;

  if (pipe(d.deadpipe) != 0)
  {
     perror("kdeinit: Aborting. Can't create pipe: ");
     exit(255);
  }

  options = fcntl(d.deadpipe[0], F_GETFL);
  if (options == -1)
  {
     perror("kdeinit: Aborting. Can't make pipe non-blocking: ");
     exit(255);
  }

  if (fcntl(d.deadpipe[0], F_SETFL, options | O_NONBLOCK) == -1)
  {
     perror("kdeinit: Aborting. Can't make pipe non-blocking: ");
     exit(255);
  }

  /*
   * A SIGCHLD handler is installed which sends a byte into the
   * pipe of death. This is to ensure that a dying child causes
   * an exit from select().
   */
  act.sa_handler=sig_child_handler;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGCHLD);
  sigprocmask(SIG_UNBLOCK, &(act.sa_mask), 0L);
  act.sa_flags = SA_NOCLDSTOP;

  // CC: take care of SunOS which automatically restarts interrupted system
  // calls (and thus does not have SA_RESTART)

#ifdef SA_RESTART
  act.sa_flags |= SA_RESTART;
#endif
  sigaction( SIGCHLD, &act, 0L);

  act.sa_handler=SIG_IGN;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGPIPE);
  sigprocmask(SIG_UNBLOCK, &(act.sa_mask), 0L);
  act.sa_flags = 0;
  sigaction( SIGPIPE, &act, 0L);
}

static void init_kdeinit_socket()
{
  struct sockaddr_un sa;
  socklen_t socklen;
  long options;
  const char *home_dir = getenv("HOME");
  int max_tries = 10;
  if (!home_dir || !home_dir[0])
  {
     fprintf(stderr, "kdeinit: Aborting. $HOME not set!");
     exit(255);
  }
  chdir(home_dir);

  {
     QCString path = home_dir;
     if (access(path.data(), R_OK|W_OK))
     {
       if (errno == ENOENT)
          fprintf(stderr, "kdeinit: Aborting. $HOME directory (%s) does not exist.\n", path.data());
       else
          fprintf(stderr, "kdeinit: Aborting. No write access to $HOME directory (%s).\n", path.data());
       exit(255);
     }
     path += "/.ICEauthority";
     if (access(path.data(), R_OK|W_OK) && (errno != ENOENT))
     {
       fprintf(stderr, "kdeinit: Aborting. No write access to '%s'.\n", path.data());
       exit(255);
     }
  }

  /** Test if socket file is already present
   *  note that access() resolves symlinks, and so we check the actual
   *  socket file if it exists
   */
  if (access(sock_file, W_OK) == 0)
  {
     int s;
     struct sockaddr_un server;

//     fprintf(stderr, "kdeinit: Warning, socket_file already exists!\n");
     /*
      * create the socket stream
      */
     s = socket(PF_UNIX, SOCK_STREAM, 0);
     if (s < 0)
     {
        perror("socket() failed: ");
        exit(255);
     }
     server.sun_family = AF_UNIX;
     strcpy(server.sun_path, sock_file);
     socklen = sizeof(server);

     if(connect(s, (struct sockaddr *)&server, socklen) == 0)
     {
        fprintf(stderr, "kdeinit: Shutting down running client.\n");
        klauncher_header request_header;
        request_header.cmd = LAUNCHER_TERMINATE_KDEINIT;
        request_header.arg_length = 0;
        write(s, &request_header, sizeof(request_header));
        sleep(1); // Give it some time
     }
     close(s);
  }

  /** Delete any stale socket file (and symlink) **/
  unlink(sock_file);

  /** create socket **/
  d.wrapper = socket(PF_UNIX, SOCK_STREAM, 0);
  if (d.wrapper < 0)
  {
     perror("kdeinit: Aborting. socket() failed: ");
     exit(255);
  }

  options = fcntl(d.wrapper, F_GETFL);
  if (options == -1)
  {
     perror("kdeinit: Aborting. Can't make socket non-blocking: ");
     close(d.wrapper);
     exit(255);
  }

  if (fcntl(d.wrapper, F_SETFL, options | O_NONBLOCK) == -1)
  {
     perror("kdeinit: Aborting. Can't make socket non-blocking: ");
     close(d.wrapper);
     exit(255);
  }

  while (1) {
      /** bind it **/
      sa.sun_family = AF_UNIX;
      strcpy(sa.sun_path, sock_file);
      socklen = sizeof(sa);
      if(bind(d.wrapper, (struct sockaddr *)&sa, socklen) != 0)
      {
          if (max_tries == 0) {
	      perror("kdeinit: Aborting. bind() failed: ");
	      close(d.wrapper);
	      exit(255);
	  }
	  max_tries--;
      } else
          break;
  }

  /** set permissions **/
  if (chmod(sock_file, 0600) != 0)
  {
     perror("kdeinit: Aborting. Can't set permissions on socket: ");
     unlink(sock_file);
     close(d.wrapper);
     exit(255);
  }

  if(listen(d.wrapper, SOMAXCONN) < 0)
  {
     perror("kdeinit: Aborting. listen() failed: ");
     unlink(sock_file);
     close(d.wrapper);
     exit(255);
  }
}

/*
 * Read 'len' bytes from 'sock' into buffer.
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

static void WaitPid( pid_t waitForPid)
{
  int result;
  while(1)
  {
    result = waitpid(waitForPid, &d.exit_status, 0);
    if ((result == -1) && (errno == ECHILD))
       return;
  }
}

static void launcher_died()
{
   /* This is bad. */
   fprintf(stderr, "kdeinit: Communication error with launcher. Exiting!\n");
   ::exit(255);
   return;
}

static void handle_launcher_request(int sock = -1)
{
   bool launcher = false;
   if (sock < 0)
   {
       sock = d.launcher[0];
       launcher = true;
   }

   klauncher_header request_header;
   char *request_data = 0L;
   int result = read_socket(sock, (char *) &request_header, sizeof(request_header));
   if (result != 0)
   {
      if (launcher)
         launcher_died();
      return;
   }

   if ( request_header.arg_length != 0 )
   {
       request_data = (char *) malloc(request_header.arg_length);

       result = read_socket(sock, request_data, request_header.arg_length);
       if (result != 0)
       {
           if (launcher)
               launcher_died();
           free(request_data);
           return;
       }
   }

   if ((request_header.cmd == LAUNCHER_EXEC) ||
       (request_header.cmd == LAUNCHER_EXT_EXEC) ||
       (request_header.cmd == LAUNCHER_SHELL ) ||
       (request_header.cmd == LAUNCHER_KWRAPPER) ||
       (request_header.cmd == LAUNCHER_EXEC_NEW))
   {
      pid_t pid;
      klauncher_header response_header;
      long response_data;
      long l;
      memcpy( &l, request_data, sizeof( long ));
      int argc = l;
      const char *name = request_data + sizeof(long);
      const char *args = name + strlen(name) + 1;
      const char *cwd = 0;
      int envc = 0;
      const char *envs = 0;
      const char *tty = 0;
      int avoid_loops = 0;
      const char *startup_id = "0";

#ifndef NDEBUG
     fprintf(stderr, "kdeinit: Got %s '%s' from %s.\n",
        (request_header.cmd == LAUNCHER_EXEC ? "EXEC" :
        (request_header.cmd == LAUNCHER_EXT_EXEC ? "EXT_EXEC" :
        (request_header.cmd == LAUNCHER_EXEC_NEW ? "EXEC_NEW" :
        (request_header.cmd == LAUNCHER_SHELL ? "SHELL" : "KWRAPPER" )))),
         name, launcher ? "launcher" : "socket" );
#endif

      const char *arg_n = args;
      for(int i = 1; i < argc; i++)
      {
        arg_n = arg_n + strlen(arg_n) + 1;
      }

      if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER )
      {
         // Shell or kwrapper
         cwd = arg_n; arg_n += strlen(cwd) + 1;
      }
      if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER
          || request_header.cmd == LAUNCHER_EXT_EXEC || request_header.cmd == LAUNCHER_EXEC_NEW )
      {
         memcpy( &l, arg_n, sizeof( long ));
         envc = l;
         arg_n += sizeof(long);
         envs = arg_n;
         for(int i = 0; i < envc; i++)
         {
           arg_n = arg_n + strlen(arg_n) + 1;
         }
         if( request_header.cmd == LAUNCHER_KWRAPPER )
         {
             tty = arg_n;
             arg_n += strlen( tty ) + 1;
         }
      }

     if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER
         || request_header.cmd == LAUNCHER_EXT_EXEC || request_header.cmd == LAUNCHER_EXEC_NEW )
     {
         memcpy( &l, arg_n, sizeof( long ));
         avoid_loops = l;
         arg_n += sizeof( long );
     }

     if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER
         || request_header.cmd == LAUNCHER_EXT_EXEC )
     {
         startup_id = arg_n;
         arg_n += strlen( startup_id ) + 1;
     }

     if ((arg_n - request_data) != request_header.arg_length)
     {
#ifndef NDEBUG
       fprintf(stderr, "kdeinit: EXEC request has invalid format.\n");
#endif
       free(request_data);
       d.debug_wait = false;
       return;
     }

      // support for the old a bit broken way of setting DISPLAY for multihead
      QCString olddisplay = getenv(DISPLAY);
      QCString kdedisplay = getenv("KDE_DISPLAY");
      bool reset_display = (! olddisplay.isEmpty() &&
                            ! kdedisplay.isEmpty() &&
                            olddisplay != kdedisplay);

      if (reset_display)
          setenv(DISPLAY, kdedisplay, true);

      pid = launch( argc, name, args, cwd, envc, envs,
          request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER,
          tty, avoid_loops, startup_id );

      if (reset_display) {
          unsetenv("KDE_DISPLAY");
          setenv(DISPLAY, olddisplay, true);
      }

      if (pid && (d.result == 0))
      {
         response_header.cmd = LAUNCHER_OK;
         response_header.arg_length = sizeof(response_data);
         response_data = pid;
         write(sock, &response_header, sizeof(response_header));
         write(sock, &response_data, response_header.arg_length);
      }
      else
      {
         int l = d.errorMsg.length();
         if (l) l++; // Include trailing null.
         response_header.cmd = LAUNCHER_ERROR;
         response_header.arg_length = l;
         write(sock, &response_header, sizeof(response_header));
         if (l)
            write(sock, d.errorMsg.data(), l);
      }
      d.debug_wait = false;
   }
   else if (request_header.cmd == LAUNCHER_SETENV)
   {
      const char *env_name;
      const char *env_value;
      env_name = request_data;
      env_value = env_name + strlen(env_name) + 1;

#ifndef NDEBUG
      if (launcher)
         fprintf(stderr, "kdeinit: Got SETENV '%s=%s' from klauncher.\n", env_name, env_value);
      else
         fprintf(stderr, "kdeinit: Got SETENV '%s=%s' from socket.\n", env_name, env_value);
#endif

      if ( request_header.arg_length !=
          (int) (strlen(env_name) + strlen(env_value) + 2))
      {
#ifndef NDEBUG
         fprintf(stderr, "kdeinit: SETENV request has invalid format.\n");
#endif
         free(request_data);
         return;
      }
      setenv( env_name, env_value, 1);
   }
   else if (request_header.cmd == LAUNCHER_TERMINATE_KDE)
   {
#ifndef NDEBUG
       fprintf(stderr,"kdeinit: terminate KDE.\n");
#endif
       kdeinit_xio_errhandler( 0L );
   }
   else if (request_header.cmd == LAUNCHER_TERMINATE_KDEINIT)
   {
#ifndef NDEBUG
       fprintf(stderr,"kdeinit: Killing kdeinit/klauncher.\n");
#endif
       if (d.launcher_pid)
          kill(d.launcher_pid, SIGTERM);
       if (d.my_pid)
          kill(d.my_pid, SIGTERM);
   }
   else if (request_header.cmd == LAUNCHER_DEBUG_WAIT)
   {
#ifndef NDEBUG
       fprintf(stderr,"kdeinit: Debug wait activated.\n");
#endif
       d.debug_wait = true;
   }
   if (request_data)
       free(request_data);
}

static void handle_requests(pid_t waitForPid)
{
   int max_sock = d.wrapper;
   if (d.launcher_pid && (d.launcher[0] > max_sock))
      max_sock = d.launcher[0];
#ifdef _WS_X11
   if (X11fd > max_sock)
      max_sock = X11fd;
#endif
   max_sock++;

   while(1)
   {
      fd_set rd_set;
      fd_set wr_set;
      fd_set e_set;
      int result;
      pid_t exit_pid;
      char c;

      /* Flush the pipe of death */
      while( read(d.deadpipe[0], &c, 1) == 1);

      /* Handle dying children */
      do {
        exit_pid = waitpid(-1, 0, WNOHANG);
        if (exit_pid > 0)
        {
#ifndef NDEBUG
           fprintf(stderr, "kdeinit: PID %ld terminated.\n", (long) exit_pid);
#endif
           if (waitForPid && (exit_pid == waitForPid))
              return;
           if (d.launcher_pid)
           {
           // TODO send process died message
              klauncher_header request_header;
              long request_data[2];
              request_header.cmd = LAUNCHER_DIED;
              request_header.arg_length = sizeof(long) * 2;
              request_data[0] = exit_pid;
              request_data[1] = 0; /* not implemented yet */
              write(d.launcher[0], &request_header, sizeof(request_header));
              write(d.launcher[0], request_data, request_header.arg_length);
           }
        }
      }
      while( exit_pid > 0);

      FD_ZERO(&rd_set);
      FD_ZERO(&wr_set);
      FD_ZERO(&e_set);

      if (d.launcher_pid)
      {
         FD_SET(d.launcher[0], &rd_set);
      }
      FD_SET(d.wrapper, &rd_set);
      FD_SET(d.deadpipe[0], &rd_set);
#ifdef Q_WS_X11
      if(X11fd >= 0) FD_SET(X11fd, &rd_set);
#endif

      result = select(max_sock, &rd_set, &wr_set, &e_set, 0);

      /* Handle wrapper request */
      if ((result > 0) && (FD_ISSET(d.wrapper, &rd_set)))
      {
         struct sockaddr_un client;
         socklen_t sClient = sizeof(client);
         int sock = accept(d.wrapper, (struct sockaddr *)&client, &sClient);
         if (sock >= 0)
         {
            if (fork() == 0)
            {
                close_fds();
                handle_launcher_request(sock);
                exit(255); /* Terminate process. */
            }
            close(sock);
         }
      }

      /* Handle launcher request */
      if ((result > 0) && (d.launcher_pid) && (FD_ISSET(d.launcher[0], &rd_set)))
      {
         handle_launcher_request();
      }

#ifdef Q_WS_X11
      /* Look for incoming X11 events */
      if((result > 0) && (X11fd >= 0))
      {
        if(FD_ISSET(X11fd,&rd_set))
        {
          if (X11display != 0) {
	    XEvent event_return;
	    while (XPending(X11display))
	      XNextEvent(X11display, &event_return);
	  }
        }
      }
#endif
   }
}

static void kdeinit_library_path()
{
   QStringList ltdl_library_path =
     QStringList::split(':', QFile::decodeName(getenv("LTDL_LIBRARY_PATH")));
   QStringList ld_library_path =
     QStringList::split(':', QFile::decodeName(getenv("LD_LIBRARY_PATH")));

   QCString extra_path;
   QStringList candidates = s_instance->dirs()->resourceDirs("lib");
   for (QStringList::ConstIterator it = candidates.begin();
        it != candidates.end();
        it++)
   {
      QString d = *it;
      if (ltdl_library_path.contains(d))
          continue;
      if (ld_library_path.contains(d))
          continue;
      if (d[d.length()-1] == '/')
      {
         d.truncate(d.length()-1);
         if (ltdl_library_path.contains(d))
            continue;
         if (ld_library_path.contains(d))
            continue;
      }
      if ((d == "/lib") || (d == "/usr/lib"))
         continue;

      QCString dir = QFile::encodeName(d);

      if (access(dir, R_OK))
          continue;

      if ( !extra_path.isEmpty())
         extra_path += ":";
      extra_path += dir;
   }

   if (lt_dlinit())
   {
      const char * ltdlError = lt_dlerror();
      fprintf(stderr, "can't initialize dynamic loading: %s\n", ltdlError != 0 ? ltdlError : "(null)" );
   }
   if (!extra_path.isEmpty())
      lt_dlsetsearchpath(extra_path.data());

   QCString display = getenv(DISPLAY);
   if (display.isEmpty())
   {
     fprintf(stderr, "kdeinit: Aborting. $"DISPLAY" is not set.\n");
     exit(255);
   }
   int i;
   if((i = display.findRev('.')) > display.findRev(':') && i >= 0)
     display.truncate(i);

   QCString socketName = QFile::encodeName(locateLocal("socket", QString("kdeinit-%1").arg(display), s_instance));
   if (socketName.length() >= MAX_SOCK_FILE)
   {
     fprintf(stderr, "kdeinit: Aborting. Socket name will be too long:\n");
     fprintf(stderr, "         '%s'\n", socketName.data());
     exit(255);
   }
   strcpy(sock_file, socketName.data());
}

int kdeinit_xio_errhandler( Display * )
{
    qWarning( "kdeinit: Fatal IO error: client killed" );

    if (sock_file[0])
    {
      /** Delete any stale socket file **/
      unlink(sock_file);
    }

    qWarning( "kdeinit: sending SIGHUP to children." );

    /* this should remove all children we started */
    signal(SIGHUP, SIG_IGN);
    kill(0, SIGHUP);

    sleep(2);

    qWarning( "kdeinit: sending SIGTERM to children." );

    /* and if they don't listen to us, this should work */
    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);

    qWarning( "kdeinit: Exit." );

    exit( 1 );
    return 0;
}

#ifdef Q_WS_X11
// Borrowed from kdebase/kaudio/kaudioserver.cpp
static int initXconnection()
{
  X11display = XOpenDisplay(NULL);
  if ( X11display != 0 ) {
    XSetIOErrorHandler(kdeinit_xio_errhandler);
    XCreateSimpleWindow(X11display, DefaultRootWindow(X11display), 0,0,1,1, \
        0,
        BlackPixelOfScreen(DefaultScreenOfDisplay(X11display)),
        BlackPixelOfScreen(DefaultScreenOfDisplay(X11display)) );
#ifndef NDEBUG
    fprintf(stderr, "kdeinit: opened connection to %s\n", DisplayString(X11display));
#endif
    net_current_desktop = XInternAtom( X11display, "_NET_CURRENT_DESKTOP", False );
    return XConnectionNumber( X11display );
  } else
    fprintf(stderr, "kdeinit: Can't connect to the X Server.\n" \
     "kdeinit: Might not terminate at end of session.\n");

  return -1;
}
#endif

#ifdef __KCC
/* One of my horrible hacks.  KCC includes in each "main" function a call
   to _main(), which is provided by the C++ runtime system.  It is
   responsible for calling constructors for some static objects.  That must
   be done only once, so _main() is guarded against multiple calls.
   For unknown reasons the designers of KAI's libKCC decided it would be
   a good idea to actually abort() when it's called multiple times, instead
   of ignoring further calls.  This breaks our mechanism of KLM's, because
   most KLM's have a main() function which is called from us.
   The "solution" is to simply define our own _main(), which ignores multiple
   calls, which is easy, and which does the same work as KAI'c _main(),
   which is difficult.  Currently (KAI 4.0f) it only calls __call_ctors(void)
   (a C++ function), but if that changes we need to change our's too.
   (matz) */
extern "C" void _main(void);
extern "C" void __call_ctors__Fv(void);
static int main_called = 0;
void _main(void)
{
  if (main_called)
    return;
  main_called = 1;
  __call_ctors__Fv ();
}
#endif

static void secondary_child_handler(int)
{
   waitpid(-1, 0, WNOHANG);
}

int main(int argc, char **argv, char **envp)
{
   int i;
   pid_t pid;
   int launch_dcop = 1;
   int launch_klauncher = 1;
   int launch_kded = 1;
   int keep_running = 1;
   int suicide = 0;

   /** Save arguments first... **/
   char **safe_argv = (char **) malloc( sizeof(char *) * argc);
   for(i = 0; i < argc; i++)
   {
      safe_argv[i] = strcpy((char*)malloc(strlen(argv[i])+1), argv[i]);
      if (strcmp(safe_argv[i], "--no-dcop") == 0)
         launch_dcop = 0;
      if (strcmp(safe_argv[i], "--no-klauncher") == 0)
         launch_klauncher = 0;
      if (strcmp(safe_argv[i], "--no-kded") == 0)
         launch_kded = 0;
      if (strcmp(safe_argv[i], "--suicide") == 0)
         suicide = 1;
      if (strcmp(safe_argv[i], "--exit") == 0)
         keep_running = 0;
   }

   pipe(d.initpipe);

   // Fork here and let parent process exit.
   // Parent process may only exit after all required services have been
   // launched. (dcopserver/klauncher and services which start with '+')
   signal( SIGCHLD, secondary_child_handler);
   if (fork() > 0) // Go into background
   {
      close(d.initpipe[1]);
      d.initpipe[1] = -1;
      // wait till init is complete
      char c;
      while( read(d.initpipe[0], &c, 1) < 0);
      // then exit;
      close(d.initpipe[0]);
      d.initpipe[0] = -1;
      return 0;
   }
   close(d.initpipe[0]);
   d.initpipe[0] = -1;
   d.my_pid = getpid();

   /** Make process group leader (for shutting down children later) **/
   if(keep_running)
      setsid();

   /** Create our instance **/
   s_instance = new KInstance("kdeinit");
   // Don't make it the global instance
   KGlobal::_instance = 0L;

   /** Prepare to change process name **/
   kdeinit_initsetproctitle(argc, argv, envp);
   kdeinit_setproctitle("Starting up...");
   kdeinit_library_path();
   unsetenv("LD_BIND_NOW");
   KApplication::loadedByKdeinit = true;

   d.maxname = strlen(argv[0]);
   d.launcher_pid = 0;
   d.wrapper = 0;
   d.debug_wait = false;
   d.lt_dlopen_flag = lt_dlopen_flag;
   lt_dlopen_flag |= LTDL_GLOBAL;
   init_signals();

   if (keep_running)
   {
      /*
       * Create ~/.kde/tmp-<hostname>/kdeinit-<display> socket for incoming wrapper
       * requests.
       */
      init_kdeinit_socket();
   }

   if (launch_dcop)
   {
      if (suicide)
         pid = launch( 3, "dcopserver", "--nosid\0--suicide" );
      else
         pid = launch( 2, "dcopserver", "--nosid" );
#ifndef NDEBUG
      fprintf(stderr, "kdeinit: Launched DCOPServer, pid = %ld result = %d\n", (long) pid, d.result);
#endif
      WaitPid(pid);
      if (!WIFEXITED(d.exit_status) || (WEXITSTATUS(d.exit_status) != 0))
      {
         fprintf(stderr, "kdeinit: DCOPServer could not be started, aborting.\n");
         exit(1);
      }
   }

   if (!suicide)
   {
      QString konq = locate("lib", "libkonq.la", s_instance);
      if (!konq.isEmpty())
      {
	  lt_dlhandle result = lt_dlopen(QFile::encodeName(konq).data());
      }
   }

   if (launch_klauncher)
   {
      pid = launch( 1, "klauncher", 0 );
#ifndef NDEBUG
      fprintf(stderr, "kdeinit: Launched KLauncher, pid = %ld result = %d\n", (long) pid, d.result);
#endif
      WaitPid(pid);
   }

   if (launch_kded)
   {
      pid = launch( 1, "kded", 0 );
#ifndef NDEBUG
      fprintf(stderr, "kdeinit: Launched KDED, pid = %ld result = %d\n", (long) pid, d.result);
#endif
      handle_requests(pid);
   }

#ifdef Q_WS_X11
   X11fd = initXconnection();
#endif

   for(i = 1; i < argc; i++)
   {
      if (safe_argv[i][0] == '+')
      {
         pid = launch( 1, safe_argv[i]+1, 0);
#ifndef NDEBUG
      fprintf(stderr, "kdeinit: Launched '%s', pid = %ld result = %d\n", safe_argv[i]+1, (long) pid, d.result);
#endif
         handle_requests(pid);
      }
      else if (safe_argv[i][0] == '-')
      {
         // Ignore
      }
      else
      {
         pid = launch( 1, safe_argv[i], 0 );
#ifndef NDEBUG
      fprintf(stderr, "kdeinit: Launched '%s', pid = %ld result = %d\n", safe_argv[i], (long) pid, d.result);
#endif
      }
   }

   /** Free arguments **/
   for(i = 0; i < argc; i++)
   {
      free(safe_argv[i]);
   }
   free (safe_argv);

   kdeinit_setproctitle("Running...");

   if (!keep_running)
      return 0;

   char c = 0;
   write(d.initpipe[1], &c, 1); // Kdeinit is started.
   close(d.initpipe[1]);
   d.initpipe[1] = -1;

   handle_requests(0);

   return 0;
}

