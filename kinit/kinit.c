/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

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

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "ltdl.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <setproctitle.h>

#define MAX_ARGLENGTH 128*1024

int waitForPid;

void sigChildHandler(int signum)
{
  int status;
  pid_t this_pid;
  int saved_errno;

  saved_errno = errno;
  // since waitpid and write change errno, we have to save it and restore it
  // (Richard Stevens, Advanced programming in the Unix Environment)

  // Waba: Check for multiple childs exiting at the same time
  do
  {
    this_pid = waitpid(-1, &status, WNOHANG);
  }
  while (this_pid > 0); 

  errno = saved_errno;

}

/* Group data */
struct {
  int maxname;
  int fd[2];
  char result;
  pid_t fork;
  int n;
  lt_dlhandle handle;
  lt_ptr_t sym;
  char **argv;
  int (*func)(int, char *[]);
} d;

char cmdLine[MAX_ARGLENGTH];

pid_t launch(int argc, char *name, char *args)
{
  int l;
  char *cmd;

  strncpy(cmdLine, name, d.maxname);
  cmdLine[d.maxname] = 0;
  name = cmdLine;
  cmd = strcpy(name + strlen(name) + 1, "lib");
  strncat(cmd, name, MAX_ARGLENGTH - 10);
  strcat(cmd, ".la");
  l = strlen(cmd);
  if (args)
  {
    strncpy( cmd + l + 1, args, MAX_ARGLENGTH - l - l);
    args = cmd + l + 1;
  }
  else
  {
    argc = 1;
  }

  if (0 > pipe(d.fd))
  {
     perror("kinit: pipe() failed!\n");
     exit(255);
  }
  
  d.fork = fork();
  switch(d.fork) {
  case -1:
     perror("kinit: fork() failed!\n");
     exit(255);
     break;
  case 0:
     /** Child **/
     close(d.fd[0]);
     
     setsid();

fprintf(stderr, "arg[0] = %s\n", name);

     /** Give the process a new name **/
     kinit_setproctitle( "%s", name );
         
     d.argv = (char **) malloc(sizeof(char *) * argc);
     d.argv[0] = name;
     for ( l = 1;  l < argc; l++)
     {
        d.argv[l] = args;
fprintf(stderr, "arg[%d] = %s\n", l, args);
        while(*args != 0) args++;
        args++; 
     }

     printf("Opening \"%s\"\n", cmd);
     d.handle = lt_dlopen( cmd );
     if (!d.handle )
     {
        fprintf(stderr, "Could not dlopen library: %s\n", lt_dlerror());        
        d.result = 1; // Error
        write(d.fd[1], &d.result, 1);
        close(d.fd[1]);
        exit(255);
     }
     
     d.sym = lt_dlsym( d.handle, "main");
     if (!d.sym )
     {
        fprintf(stderr, "Could not find kde_main: %s\n", lt_dlerror());        
        d.result = 0; // Error
        write(d.fd[1], &d.result, 1);
        close(d.fd[1]);
        exit(255);
     }
     
     d.result = 0; // Success
     write(d.fd[1], &d.result, 1);
     close(d.fd[1]);
 
     d.func = (int (*)(int, char *[])) d.sym;

     exit( d.func( argc, d.argv)); /* Launch! */
     break;
  default:
     /** Parent **/
     close(d.fd[1]);
     for(;;)
     {
       d.n = read(d.fd[0], &d.result, 1);
       if (d.n == 1) break;
       if (d.n == 0)
       {
          perror("kinit: Pipe closed unexpected.\n");
          exit(255);
       }
       if (errno != EINTR)
       {
          perror("kinit: Error reading from pipe.\n");
          exit(255);
       }
     }
     close(d.fd[0]);
  }
  return d.fork;
}

void init()
{
  struct sigaction act;

  act.sa_handler=sigChildHandler;
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

void WaitPid( pid_t waitForPid)
{
  int result;
  while(1)
  {
    result = waitpid(waitForPid, 0, 0);
    if ((result == -1) && (errno == ECHILD))
       return;
  }
}

int main(int argc, char **argv, char **envp)
{
   int i;
#if 0
   char *myArgv1[] = { "libdcopserver.la", 0 };
   char *myArgv2[] = { "libkded.la", 0 };
   char *myArgv3[] = { "libkcminit.la", 0 };
   char *myArgv4[] = { "libkicker.la", 0 };
   char *myArgv5[] = { "libkdesktop.la", 0 };
   char *myArgv6[] = { "libkaudioserver.la", 0 };
#endif
   pid_t pid;
   int launch_dcop = 1;
   int keep_running = 1;

   /** Save arguments first... **/
   d.argv = (char **) malloc( sizeof(char *) * argc);
   for(i = 0; i < argc; i++) 
   {
      d.argv[i] = strcpy(malloc(strlen(argv[i])+1), argv[i]);
      if (strcmp(d.argv[i], "--no-dcop") == 0)
         launch_dcop = 0;
      if (strcmp(d.argv[i], "--exit") == 0)
         keep_running = 0;
   }
   
   /** Prepare to change process name **/
   kinit_initsetproctitle(argc, argv, envp);  
   kinit_setproctitle("Starting up...");

   unsetenv("LD_BIND_NOW");

   d.maxname = strlen(argv[0]);
   init();

   printf("Pre Launcher, pid = %d\n", getpid());

   if (launch_dcop)
   {
      pid = launch( 1, "dcopserver", 0 );
      printf("DCOPServer: pid = %d result = %d\n", pid, d.result);
      WaitPid(pid); /* Wait for dcopserver to fork() */
      sleep(1); /* Wait for dcopserver to settle */
   }

   for(i = 1; i < argc; i++)
   {
      if (d.argv[i][0] == '+')
      {
         pid = launch( 1, d.argv[i]+1, 0);
         printf("Launched: %s, pid = %d result = %d\n", d.argv[i]+1, pid, d.result);
         WaitPid(pid);
      }
      else if (d.argv[i][0] == '-')
      {
         // Ignore
      }
      else
      {
         pid = launch( 1, d.argv[i], 0 );
         printf("Launched: %s, pid = %d result = %d\n", d.argv[i], pid, d.result);
      }
   }

   /** Free arguments **/
   d.argv = (char **) malloc( sizeof(char *) * argc);
   for(i = 0; i < argc; i++) 
   {
      free(d.argv[i]);
   }
   free (d.argv);

   kinit_setproctitle("Running...");

   if (keep_running)
   {
     while (1)
       sleep(20000);
   }
   return 0;
}