/*
 * This file is part of the KDE libraries
 * Copyright (c) 2006 Lubos Lunak <l.lunak@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <config-prefix.h>
#include <config-kdeinit.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define EXECUTE BIN_INSTALL_DIR "/kdeinit4"

#ifdef KDEINIT_OOM_PROTECT

/*
 Prevent getting killed by bad heuristic in Linux OOM-killer.
 This wrapper decreases the chance OOM killer kills it (or its children,
 namely kdeinit), opens a pipe and forks. Child drops privileges
 and launches kdeinit. Since processes started by kdeinit should
 not have this protection, kdeinit will after forking send the new
 PID using the pipe and wait for a signal. This parent will reset the protection
 and SIGUSR1 the process to continue.
 returns 1 if pid is valid
*/

static int set_protection( pid_t pid, int enable )
{
   char buf[ 1024 ];
   int procfile;
   sprintf( buf, "/proc/%d/stat", pid );
   if( !enable ) {
       /* Be paranoid and check that the pid we got from the pipe
          belongs to this user. */
       struct stat st;
       if( lstat( buf, &st ) < 0 || st.st_uid != getuid())
           return 0;
   }
   sprintf( buf, "/proc/%d/oom_adj", pid );
   procfile = open( buf, O_WRONLY );
   if( procfile >= 0 ) {
      if( enable )
         write( procfile, "-5", sizeof( "-5" ));
      else
         write( procfile, "0", sizeof( "0" ));
      close( procfile );
   }
   return 1;
}

int main(int argc, char **argv)
{
   int pipes[ 2 ];
   int new_argc;
   const char** new_argv;
   char helper_num[ 1024 ];
   unsigned i;
   char** orig_environ = NULL;
   char header[ 7 ];
   if( pipe( pipes ) < 0 ) {
      perror( "pipe()" );
      return 1;
   }
   if( argc < 0 || argc > 1000 )
       abort(); /* paranoid */
   set_protection( getpid(), 1 );
   switch( fork()) {
      case -1:
         perror( "fork()" );
         return 1;
      default: /* parent, drop privileges and exec */
         if (setgid(getgid())) {
             perror("setgid()");
             return 1;
         }
         if (setuid(getuid()) || geteuid() != getuid()) {
            perror("setuid()");
            return 1;
         }
         close( pipes[ 0 ] );
         /* read original environment passed by start_kdeinit_wrapper */
         if( read( 0, header, 7 ) == 7 && strncmp( header, "environ", 7 ) == 0 ) {
             unsigned count;
             if( read( 0, &count, sizeof( unsigned )) == sizeof( unsigned )
                 && count && count < (1<<16)) {
                 char** env = malloc(( count + 1 ) * sizeof( char* ));
                 int ok = 1;
                 for( i = 0;
                      i < count && ok;
                      ++i ) {
                     unsigned len;
                     if( read( 0, &len, sizeof( unsigned )) == sizeof( unsigned )
                         && len && len < (1<<12)) {
                         env[ i ] = malloc( len + 1 );
                         if( (unsigned) read( 0, env[ i ], len ) == len ) {
                             env[ i ][ len ] = '\0';
                         } else {
                             ok = 0;
                         }
                     }
                 }
                 if( ok ) {
                   env[ i ] = NULL;
                   orig_environ = env;
                 }
             }
         }
         if(argc == 0)
            return 1;
         new_argc = argc + 2;
         new_argv = malloc( sizeof( char* ) * ( new_argc + 1 ));
         if( new_argv == NULL )
            return 1;
         new_argv[ 0 ] = EXECUTE;
         new_argv[ 1 ] = "--oom-pipe";
         sprintf( helper_num, "%d", pipes[ 1 ] );
         new_argv[ 2 ] = helper_num;
         for( i = 1;
              i <= (unsigned) argc;
              ++i )
             new_argv[ i + 2 ] = argv[ i ];
         if( orig_environ )
             execve(EXECUTE, (char**)new_argv, orig_environ);
         else
             execv(EXECUTE, (char**)new_argv);
         perror(EXECUTE);
         return 1;
      case 0: /* child, keep privileges and do the privileged work */
         close( pipes[ 1 ] );
         for(;;) {
            pid_t pid = 0;
            int ret = read( pipes[ 0 ], &pid, sizeof( pid_t ));
            if( ret < 0 && errno == EINTR )
               continue;
            if( ret <= 0 ) /* pipe closed or error, exit */
               _exit(0);
            if( pid != 0 ) {
                if (set_protection( pid, 0 ))
                    kill( pid, SIGUSR1 );
            }
         }
   }
}

#else /* not Linux, the simple non-setuid case */

int main(int argc, char **argv)
{
   if(argc == 0)
      return 1;
   argv[0] = (char*)EXECUTE;
   execv(EXECUTE,argv);
   perror(EXECUTE);
   return 1;
}
#endif
