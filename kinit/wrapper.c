/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
            (c) 1999 Mario Weilguni <mweilguni@sime.com>
            (c) 2001 Lubos Lunak <l.lunak@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "klauncher_cmds.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <limits.h>

#if  defined(HAVE_NSGETENVIRON) && defined(HAVE_CRT_EXTERNS_H)
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#endif

extern char **environ;

static char *getDisplay()
{
   const char *display;
   char *result;
   char *screen;
   char *colon;
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
   result = malloc(strlen(display)+1);
   if (result == NULL)
      return NULL;

   strcpy(result, display);
   screen = strrchr(result, '.');
   colon = strrchr(result, ':');
   if (screen && (screen > colon))
      *screen = '\0';
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
  socklen_t socklen;
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

  /* append $DISPLAY */
  display = getDisplay();
  if (display == NULL)
  {
     fprintf(stderr, "Error: Could not determine display.\n");
     return -1;
  }

  if (strlen(sock_file)+strlen(display)+strlen("/kdeinit-")+2 > MAX_SOCK_FILE)
  {
     fprintf(stderr, "Warning: Socket name will be too long.\n");
     return -1;
  }
  strcat(sock_file, "/kdeinit-");
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

static pid_t kwrapper_pid;
static volatile pid_t got_sig; /* = 0 */
static pid_t last_sig; /* = 0 */

static void sig_pass_handler( int signo );
static void setup_signals( void );

static void setup_signal_handler( int signo, int clean )
{    
    struct sigaction sa;
    if( clean )
        sa.sa_handler = SIG_DFL;
    else
        sa.sa_handler = sig_pass_handler;
    sigemptyset( &sa.sa_mask );
    sigaddset( &sa.sa_mask, signo );
    sa.sa_flags = 0; /* don't use SA_RESTART */
    sigaction( signo, &sa, 0 );
}

static void sig_pass_handler( int signo )
{
    int save_errno = errno;
    if( signo == SIGTSTP )
        kill( kwrapper_pid, SIGSTOP ); /* pass the signal to the real process */
    else                               /* SIGTSTP wouldn't work ... I don't think is much */
        kill( kwrapper_pid, signo );   /* of a problem */
    got_sig = signo;
    if( got_sig == SIGCONT )
        setup_signals(); /* restore signals */
    errno = save_errno;
}

static void setup_signals()
{
    setup_signal_handler( SIGHUP, 0 );
    setup_signal_handler( SIGINT, 0 );
    setup_signal_handler( SIGQUIT, 0 );
    setup_signal_handler( SIGILL, 0 );  /* e.g. this one is probably doesn't make sense to pass */
    setup_signal_handler( SIGABRT, 0 ); /* but anyway ... */
    setup_signal_handler( SIGFPE, 0 );
    /*   SIGKILL   can't be handled :( */
    setup_signal_handler( SIGSEGV, 0 );
    setup_signal_handler( SIGPIPE, 0 );
    setup_signal_handler( SIGALRM, 0 );
    setup_signal_handler( SIGTERM, 0 );
    setup_signal_handler( SIGUSR1, 0 );
    setup_signal_handler( SIGUSR2, 0 );
    setup_signal_handler( SIGCHLD, 0 ); /* is this a good idea ??? */
    setup_signal_handler( SIGCONT, 0 ); /* SIGSTOP can't be handled, but SIGTSTP and SIGCONT can */
    /* SIGSTOP */                       /* which should be enough */
    setup_signal_handler( SIGTSTP, 0 );
    setup_signal_handler( SIGTTIN, 0 ); /* is this a good idea ??? */
    setup_signal_handler( SIGTTOU, 0 ); /* is this a good idea ??? */
    /* some more ? */
}

static void kwrapper_run( pid_t pid )
{
    kwrapper_pid = pid;
    setup_signals();
    for(;;)
    {
        sleep( 1 ); /* poll and see if the real process is still alive */
        if( got_sig != last_sig ) /* did we get a signal ? */
            {
            last_sig = got_sig;
            if( got_sig == SIGCHLD )
                ; /* nothing, ignore */
            else if( got_sig == SIGCONT )
                ; /* done in signal handler */
            else /* do the default action ( most of them quit the app ) */
                {
                setup_signal_handler( got_sig, 1 );
                raise( got_sig ); /* handle the signal again */
                }
            }
        if( kill( pid, 0 ) < 0 )
            break;
    }
 /* I'm afraid it won't be that easy to get the return value ... */
}

int main(int argc, char **argv)
{
   int i;
   int wrapper = 0;
   int ext_wrapper = 0;
   int kwrapper = 0;
   long arg_count;
   long env_count;
   klauncher_header header;
   char *start, *p, *buffer;
   char cwd[8192];
   const char *tty = NULL;
   long avoid_loops = 0;
   const char* startup_id = NULL;

   long size = 0;
   int sock = openSocket();

   start = argv[0];
   p = start + strlen(argv[0]);
   while (--p > start)
   {
      if (*p == '/') break;
   }
   if ( p > start)
      p++;
   start = p;

   if (strcmp(start, "kdeinit_wrapper") == 0)
      wrapper = 1;
   else if (strcmp(start, "kshell") == 0)
      ext_wrapper = 1;
   else if (strcmp(start, "kwrapper") == 0)
      kwrapper = 1;
   else if (strcmp(start, "kdeinit_shutdown") == 0)
   {
      if( sock < 0 )
      {
          fprintf( stderr, "Error: Can't contact kdeinit!\n" );
          exit( 255 );
      }
      header.cmd = LAUNCHER_TERMINATE_KDE;
      header.arg_length = 0;
      write_socket(sock, (char *) &header, sizeof(header));
      return 0;
   }

   if (wrapper || ext_wrapper || kwrapper)
   {
      argv++;
      argc--;
      if (argc < 1)
      {
         fprintf(stderr, "Usage: %s <application> [<args>]\n", start);
         exit(255); /* usage should be documented somewhere ... */
      }
      start = argv[0];
   }

   if( sock < 0 ) /* couldn't contact kdeinit, start argv[ 0 ] directly */
   {
      execvp( argv[ 0 ], argv );
      fprintf( stderr, "Error: Can't run %s !\n", argv[ 0 ] );
      exit( 255 );
   }
   
   if( !wrapper && !ext_wrapper && !kwrapper )
       { /* was called as a symlink */
       avoid_loops = 1;
#if defined(WE_ARE_KWRAPPER)
       kwrapper = 1;
#elif defined(WE_ARE_KSHELL)
       ext_wrapper = 1;
#else
       wrapper = 1;
#endif
       }

   arg_count = argc;

   size += sizeof(long); /* Number of arguments*/

   size += strlen(start)+1; /* Size of first argument. */

   for(i = 1; i < argc; i++)
   {
      size += strlen(argv[i])+1;
   }
   if( wrapper )
   {
      size += sizeof(long); /* empty envs */
   }
   if (ext_wrapper || kwrapper)
   {
      if (!getcwd(cwd, 8192))
         cwd[0] = '\0';
      size += strlen(cwd)+1;

      env_count = 0;
      size += sizeof(long); /* Number of env.vars. */

      for(; environ[env_count] ; env_count++)
      {
         int l = strlen(environ[env_count])+1;
         size += l;
      }

      if( kwrapper )
      {
          tty = ttyname(1);
          if (!tty || !isatty(2))
             tty = "";
          size += strlen(tty)+1;
      }
   }
   
   size += sizeof( avoid_loops );
   
   if( !wrapper )
   {
       startup_id = getenv( "KDE_STARTUP_ENV" );
       if( startup_id == NULL )
           startup_id = "";
       size += strlen( startup_id ) + 1;
   }

   if (wrapper)
      header.cmd = LAUNCHER_EXEC_NEW;
   else if (kwrapper)
      header.cmd = LAUNCHER_KWRAPPER;
   else
      header.cmd = LAUNCHER_SHELL;
   header.arg_length = size;
   write_socket(sock, (char *) &header, sizeof(header));

   buffer = (char *) malloc(size);
   if (buffer == NULL)
   {
        fprintf(stderr, "Error: malloc() failed.");
        exit(255);
   }
   p = buffer;
      
   memcpy(p, &arg_count, sizeof(arg_count));
   p += sizeof(arg_count);

   memcpy(p, start, strlen(start)+1);
   p += strlen(start)+1;

   for(i = 1; i < argc; i++)
   {
      memcpy(p, argv[i], strlen(argv[i])+1);
      p += strlen(argv[i])+1;
   }

   if( wrapper )
   {
      long dummy = 0;
      memcpy(p, &dummy, sizeof(dummy)); /* empty envc */
      p+= sizeof( dummy );
   }
   if (ext_wrapper || kwrapper)
   {
      memcpy(p, cwd, strlen(cwd)+1);
      p+= strlen(cwd)+1;

      memcpy(p, &env_count, sizeof(env_count));
      p+= sizeof(env_count);

      for(i = 0; i < env_count; i++)
      {
         int l = strlen(environ[i])+1;
         memcpy(p, environ[i], l);
         p += l;
      }

      if( kwrapper )
      {
          memcpy(p, tty, strlen(tty)+1);
          p+=strlen(tty)+1;
      }
   }
   
   memcpy( p, &avoid_loops, sizeof( avoid_loops ));
   p += sizeof( avoid_loops );

   if( !wrapper )
   {
       memcpy(p, startup_id, strlen(startup_id)+1);
       p+= strlen(startup_id)+1;
   }
   
   if( p - buffer != size ) /* should fail only if you change this source and do */
                                 /* a stupid mistake, it should be assert() actually */
   {
      fprintf(stderr, "Oops. Invalid format.\n");
      exit(255);
   }

   write_socket(sock, buffer, size);
   free( buffer );

   if (read_socket(sock, (char *) &header, sizeof(header))==-1)
   {
      fprintf(stderr, "Communication error with KInit.\n");
      exit(255);
   }

   if (header.cmd == LAUNCHER_OK)
   {
      long pid;
      buffer = (char *) malloc(header.arg_length);
      if (buffer == NULL)
      {
          fprintf(stderr, "Error: malloc() failed\n");
          exit(255);
      }
      read_socket(sock, buffer, header.arg_length);
      pid = *((long *) buffer);
      if( !kwrapper ) /* kwrapper shouldn't print any output */
          printf("Launched ok, pid = %ld\n", pid);
      else
          kwrapper_run( pid );
   }
   else if (header.cmd == LAUNCHER_ERROR)
   {
      fprintf(stderr, "KInit could not launch '%s'.\n", start);
      exit(255);
   }
   else 
   {
      fprintf(stderr, "Unexected response from KInit (response = %ld).\n", header.cmd);
      exit(255);
   }
   exit(0);
}
