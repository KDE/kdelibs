/*
 * This file is part of the KDE libraries
 * Copyright (c) 1999-2000 Waldo Bastian <bastian@kde.org>
 *           (c) 1999 Mario Weilguni <mweilguni@sime.com>
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
#include <kinstance.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <kapp.h>

#include "ltdl.h"
#include "klauncher_cmds.h"

#include <X11/Xlib.h>

int waitForPid;
int X11fd;
static Display *X11display = 0;
#define MAX_SOCK_FILE 255
static char sock_file[MAX_SOCK_FILE];

/* Group data */
struct {
  int maxname;
  int fd[2];
  int launcher[2]; /* socket pair for launcher communication */
  int deadpipe[2]; /* pipe used to detect dead children */
  int wrapper; /* socket for wrapper communication */
  char result;
  int exit_status;
  pid_t fork;
  pid_t launcher_pid;
  int n;
  lt_dlhandle handle;
  lt_ptr_t sym;
  char **argv;
  int (*func)(int, char *[]);
  int (*launcher_func)(int);
} d;

/*
 * Close fd's which are only usefull for the parent process.
 * Restore default signal handlers.
 */
static void close_fds()
{
   if (d.deadpipe[0] != -1)
      close(d.deadpipe[0]);
   d.deadpipe[0] = -1;

   if (d.deadpipe[1] != -1)
      close(d.deadpipe[1]);
   d.deadpipe[1] = -1;

   if (d.launcher_pid)
   {
      close(d.launcher[0]);
      close(d.launcher[1]);
      d.launcher_pid = 0;
   }
   if (d.wrapper)
   {
      close(d.wrapper);
   }
   if (X11fd >= 0)
   {
      close(X11fd);
   }

   signal(SIGCHLD, SIG_DFL);
   signal(SIGPIPE, SIG_DFL);
}

static pid_t launch(int argc, const char *_name, const char *args)
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

  if (_name[0] != '/')
  {
     /* Relative name without '.la' */
     name = _name;
     lib = name + ".la";
     exec = name;
  }
  else
  {
     lib = _name;
     name = _name;
     name = name.mid( name.findRev('/') + 1);
     exec = _name;
  }
  if (!args)
  {
    argc = 1;
  }

  if (0 > pipe(d.fd))
  {
     perror("kdeinit: pipe() failed!\n");
     exit(255);
  }

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

     /** Give the process a new name **/
     kdeinit_setproctitle( "%s", name.data() );

     d.argv = (char **) malloc(sizeof(char *) * (argc+1));
     d.argv[0] = name.data();
     for (int i = 1;  i < argc; i++)
     {
        d.argv[i] = (char *) args;
        while(*args != 0) args++;
        args++;
     }
     d.argv[argc] = 0;

     d.handle = 0;
     if (lib.right(3) == ".la")
        d.handle = lt_dlopen( lib.data() );
     if (!d.handle )
     {
        const char * ltdlError = lt_dlerror();
        fprintf(stderr, "Could not dlopen library %s: %s\n", lib.data(), ltdlError != 0 ? ltdlError : "(null)" );
        d.result = 2; // Try execing
        write(d.fd[1], &d.result, 1);

        // We set the close on exec flag.
        // Closing of d.fd[1] indicates that the execvp succeeded!
        fcntl(d.fd[1], F_SETFD, FD_CLOEXEC);
        execvp(exec.data(), d.argv);
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
              d.result = 1; // Error
              write(d.fd[1], &d.result, 1);
              close(d.fd[1]);
              exit(255);
           }
        }

        d.result = 0; // Success
        write(d.fd[1], &d.result, 1);
        close(d.fd[1]);

        d.func = (int (*)(int, char *[])) d.sym;

        exit( d.func( argc, d.argv)); /* Launch! */
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

        exit( d.launcher_func( d.launcher[1] )); /* Launch! */
     }

     break;
  default:
     /** Parent **/
     close(d.fd[1]);
     if (launcher)
     {
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
             fprintf(stderr, "Could not load library! Trying exec....\n");
             exec = true;
             continue;
          }
          // Finished
          break;
       }
       if (d.n == -1)
       {
          if (errno == ECHILD)
          {
             fprintf(stderr, "kdeinit: a child died...\n");
             continue;
          }
          if (errno == EINTR)
          {
             fprintf(stderr, "kdeinit: interrupted.\n");
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
          perror("kdeinit: Pipe closed unexpected.\n");
          d.result = 1; // Error
          break;
       }
       perror("kdeinit: Error reading from pipe.\n");
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
     perror("Aborting. Can't create pipe: ");
     exit(255);
  }

  options = fcntl(d.deadpipe[0], F_GETFL);
  if (options == -1)
  {
     perror("Aborting. Can't make pipe non-blocking: ");
     exit(255);
  }

  if (fcntl(d.deadpipe[0], F_SETFL, options | O_NONBLOCK) == -1)
  {
     perror("Aborting. Can't make pipe non-blocking: ");
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
  act.sa_flags = 0;
  sigaction( SIGPIPE, &act, 0L);
}

static void init_kdeinit_socket()
{
  struct sockaddr_un sa;
  ksize_t socklen;
  long options;
  char *home_dir = getenv("HOME");
  int max_tries = 10;
  if (!home_dir || !home_dir[0])
  {
     fprintf(stderr, "Aborting. $HOME not set!");
     exit(255);
  }
  chdir(home_dir);

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
        fprintf(stderr, "kdeinit: Already running.\n");
        close(s);
        exit(255);
     }
//     perror("connect() failed: ");
     close(s);
  }

  /** Delete any stale socket file (and symlink) **/
  unlink(sock_file);

  /** create socket **/
  d.wrapper = socket(PF_UNIX, SOCK_STREAM, 0);
  if (d.wrapper < 0)
  {
     perror("Aborting. socket() failed: ");
     exit(255);
  }

  options = fcntl(d.wrapper, F_GETFL);
  if (options == -1)
  {
     perror("Aborting. Can't make socket non-blocking: ");
     close(d.wrapper);
     exit(255);
  }

  if (fcntl(d.wrapper, F_SETFL, options | O_NONBLOCK) == -1)
  {
     perror("Aborting. Can't make socket non-blocking: ");
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
	      perror("Aborting. bind() failed: ");
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
     perror("Aborting. Can't set permissions on socket: ");
     unlink(sock_file);
     close(d.wrapper);
     exit(255);
  }

  if(listen(d.wrapper, SOMAXCONN) < 0)
  {
     perror("Aborting. listen() failed: ");
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
     else if ((result == -1) && (errno != EINTR))
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

static void kill_launcher()
{
/*   pid_t pid; */
   /* This is bad. Best thing we can do is to kill the launcher. */
   fprintf(stderr, "kdeinit: Communication error with launcher. Killing launcher!\n");
   if (d.launcher_pid)
   {
     close(d.launcher[0]);
     close(d.launcher[1]);

     kill(d.launcher_pid, SIGTERM);
   }
   d.launcher_pid = 0;
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
   char *request_data;
   int result = read_socket(sock, (char *) &request_header, sizeof(request_header));
   if (result != 0)
   {
      if (launcher)
         kill_launcher();
      return;
   }

   request_data = (char *) malloc(request_header.arg_length);

   result = read_socket(sock, request_data, request_header.arg_length);
   if (result != 0)
   {
      if (launcher)
         kill_launcher();
      free(request_data);
      return;
   }

   if (request_header.cmd == LAUNCHER_EXEC)
   {
      char *name;
      char *args;
      pid_t pid;
      klauncher_header response_header;
      long response_data;
      int argc;
      argc = *((long *) request_data);
      name = request_data + sizeof(long);
      args = name + strlen(name) + 1;

      if (launcher)
         fprintf(stderr, "KInit: Got EXEC '%s' from klauncher\n", name);
      else
         fprintf(stderr, "KInit: Got EXEC '%s' from socket\n", name);

      {
         int i = 1;
         char *arg_n;
         arg_n = args;
         while (i < argc)
         {
           arg_n = arg_n + strlen(arg_n) + 1;
           i++;
         }
         if ((arg_n - request_data) != request_header.arg_length)
         {
           fprintf(stderr, "kdeinit: EXEC request has invalid format.\n");
           free(request_data);
           return;
         }
      }
      pid = launch(argc, name, args);

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
         response_header.cmd = LAUNCHER_ERROR;
         response_header.arg_length = 0;
         write(sock, &response_header, sizeof(response_header));
      }
   }
   else if (request_header.cmd == LAUNCHER_SETENV)
   {
      char *env_name;
      char *env_value;
      env_name = request_data;
      env_value = env_name + strlen(env_name) + 1;

      if (launcher)
         fprintf(stderr, "Got SETENV '%s=%s' from klauncher\n", env_name, env_value);
      else
         fprintf(stderr, "Got SETENV '%s=%s' from socket\n", env_name, env_value);

      if ( request_header.arg_length !=
          (int) (strlen(env_name) + strlen(env_value) + 2))
      {
         fprintf(stderr, "kdeinit: SETENV request has invalid format.\n");
         free(request_data);
         return;
      }
      setenv( env_name, env_value, 1);
   }
   free(request_data);
}

static void handle_requests(pid_t waitForPid)
{
   int max_sock = d.wrapper;
   if (d.launcher_pid && (d.launcher[0] > max_sock))
      max_sock = d.launcher[0];
   if (X11fd > max_sock)
      max_sock = X11fd;
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
           fprintf(stderr, "kdeinit: PID %d terminated.\n", exit_pid);
           if (waitForPid && (exit_pid == waitForPid))
              return;
           if (d.launcher_pid)
           {
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
      if(X11fd >= 0) FD_SET(X11fd, &rd_set);

      result = select(max_sock, &rd_set, &wr_set, &e_set, 0);

      /* Handle wrapper request */
      if ((result > 0) && (FD_ISSET(d.wrapper, &rd_set)))
      {
         struct sockaddr_un client;
         ksize_t sClient = sizeof(client);
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

      /* Look for incoming X11 events */
      if((result > 0) && (X11fd >= 0))
      {
        if(FD_ISSET(X11fd,&rd_set))
        {
          XEvent event_return;
          if (X11display != 0) XNextEvent(X11display, &event_return);
        }
      }
   }
}

static void kdeinit_library_path()
{
   QCString ltdl_library_path = getenv("LTDL_LIBRARY_PATH");
   QCString ld_library_path = getenv("LD_LIBRARY_PATH");
   KInstance instance( "kdeinit" );
   QStringList candidates = instance.dirs()->resourceDirs("lib");
   for (QStringList::ConstIterator it = candidates.begin();
        it != candidates.end();
        it++)
   {
      QCString dir = QFile::encodeName(*it);
      if (dir[dir.length()-1] == '/') dir.truncate(dir.length()-1);
      if (ltdl_library_path.find(dir) == -1)
      {
         if ( !ltdl_library_path.isEmpty())
             ltdl_library_path += ":";
         ltdl_library_path += dir;
      }
      if (ld_library_path.find(dir) == -1)
      {
         if ( !ld_library_path.isEmpty())
             ld_library_path += ":";
         ld_library_path += dir;
      }
   }
   setenv("LTDL_LIBRARY_PATH", ltdl_library_path.data(), 1);
   setenv("LD_LIBRARY_PATH", ld_library_path.data(), 1);
   if (lt_dlinit())
   {
      const char * ltdlError = lt_dlerror();
      fprintf(stderr, "can't initialize dynamic loading: %s\n", ltdlError != 0 ? ltdlError : "(null)" );
   }

   char *display = getenv("DISPLAY");
   if (!display)
   {
     fprintf(stderr, "Aborting. $DISPLAY is not set.\n");
     exit(255);
   }

   QCString socketName = QFile::encodeName(locateLocal("socket", QString("kdeinit-%1").arg(display)));
   if (socketName.length() >= MAX_SOCK_FILE)
   {
     fprintf(stderr, "Aborting. Socket name will be too long.\n");
     exit(255);
   }
   strcpy(sock_file, socketName.data());
}

static void output_kmapnotify_path()
{
   KInstance instance( "kdeinit" );
   QStringList candidates = instance.dirs()->resourceDirs("lib");

   QCString output;

   for (QStringList::ConstIterator it = candidates.begin();
        it != candidates.end();
        it++)
   {
      QString path(*it + "libkmapnotify.so");
      if (QFile::exists(path) && !output)
         output = QFile::encodeName(path);
   }

   printf("%s\n", (const char *)output);
}

extern "C" {
int kdeinit_xio_errhandler( Display * );
}

int kdeinit_xio_errhandler( Display * )
{
    qWarning( "kdeinit: Fatal IO error: client killed" );

    if (sock_file[0])
    {
      /** Delete any stale socket file **/
      unlink(sock_file);
    }

    /* this should remove all children we started */
    signal(SIGHUP, SIG_IGN);
    kill(0, SIGHUP);

    sleep(2);

    /* and if they don't listen to us, this should work */
    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);

    exit( 1 );
    return 0;
}

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
    kdDebug() << "kdeinit: opened connection to " << DisplayString(X11display) << endl;
    return XConnectionNumber( X11display );
  } else
    fprintf(stderr, "kdeinit: Can't connect to the X Server.\n" \
     "kdeinit: Might not terminate at end of session.\n");

  return -1;
}

int main(int argc, char **argv, char **envp)
{
   int i;
   pid_t pid;
   int launch_dcop = 1;
   int launch_klauncher = 1;
   int launch_kded = 1;
   int keep_running = 1;
   int libkmapnotify = 0;

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
      if (strcmp(safe_argv[i], "--exit") == 0)
         keep_running = 0;
      if (strcmp(safe_argv[i], "--libkmapnotify") == 0)
         libkmapnotify = 1;
   }

   /** Output path to stdout if libkmapnotify was specified **/
   if (libkmapnotify) {
      output_kmapnotify_path();
      return 0;
   }

   /** Make process group leader (for shutting down children later) **/
   if(keep_running)
      setsid();

   /** Prepare to change process name **/
   kdeinit_initsetproctitle(argc, argv, envp);
   kdeinit_setproctitle("Starting up...");
   kdeinit_library_path();
   unsetenv("LD_BIND_NOW");
   KApplication::loadedByKdeinit = true;

   d.maxname = strlen(argv[0]);
   d.launcher_pid = 0;
   d.wrapper = 0;
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
      pid = launch( 2, "dcopserver", "--nosid" );
      fprintf(stderr, "DCOPServer: pid = %d result = %d\n", pid, d.result);
      WaitPid(pid);
      if (!WIFEXITED(d.exit_status) || (WEXITSTATUS(d.exit_status) != 0))
      {
         fprintf(stderr, "kdeinit: DCOPServer could not be started, aborting.\n");
         exit(1);
      }
   }

   if (launch_klauncher)
   {
      pid = launch( 1, "klauncher", 0 );
      fprintf(stderr, "KLauncher: pid = %d result = %d\n", pid, d.result);
      WaitPid(pid);
   }

   // Make sure to launch kdesktop before kded!
   for(i = 1; i < argc; i++)
   {
      if (strcmp(safe_argv[i], "+kdesktop") == 0)
      {
         pid = launch( 2, "kdesktop", "--waitforkded" );
         fprintf(stderr, "KDesktop: pid = %d result = %d\n", pid, d.result);
         WaitPid(pid);
         safe_argv[i][0] = '-'; // Make it an option so that it won't be launched a second time!
         break;
      }
   }

   if (launch_kded)
   {
      pid = launch( 1, "kded", 0 );
      fprintf(stderr, "KDED: pid = %d result = %d\n", pid, d.result);
      WaitPid(pid);
   }

   X11fd = initXconnection();

   for(i = 1; i < argc; i++)
   {
      if (safe_argv[i][0] == '+')
      {
         pid = launch( 1, safe_argv[i]+1, 0);
         fprintf(stderr, "Launched: %s, pid = %d result = %d\n", safe_argv[i]+1, pid, d.result);
//         WaitPid(pid);
         handle_requests(pid);
      }
      else if (safe_argv[i][0] == '-')
      {
         // Ignore
      }
      else
      {
         pid = launch( 1, safe_argv[i], 0 );
         fprintf(stderr, "Launched: %s, pid = %d result = %d\n", safe_argv[i], pid, d.result);
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

   //
   // Fork here and let parent process exit.
   // Parent process may only exit after all required services have been
   // launched. (dcopserver/klauncher and services which start with '+')
   if (fork() > 0) // Go into background
       return 0;


   handle_requests(0);

   return 0;
}

