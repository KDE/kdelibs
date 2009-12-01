/*
 * This file is part of the KDE libraries
 * Copyright (c) 2007 Lubos Lunak <l.lunak@kde.org>
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define EXECUTE LIBEXEC_INSTALL_DIR "/start_kdeinit"

#ifdef KDEINIT_OOM_PROTECT

/*
 The start_kdeinit wrapper is setuid, which means some shell variables like LD_LIBRARY_PATH
 get unset before it's launched. However kdeinit is used to launch most of KDE, so such variables
 should not be dropped. Therefore this wrapper for the setuid wrapper read the environment
 and writes it to start_kdeinit's stdin, which after dropping privileges reads it and uses it
 for launching the real kdeinit.
*/
int main(int argc, char **argv)
{
   int pipes[ 2 ];
   if(argc == 0)
      return 1;
   if( pipe( pipes ) < 0 ) {
      perror( "pipe()" );
      return 1;
   }
   switch( fork()) {
      case -1:
         perror( "fork()" );
         return 1;
      default: /* parent, exec */ 
         close( pipes[ 1 ] );
         close( 0 ); /* stdin */
         dup2( pipes[ 0 ], 0 );
         close( pipes[ 0 ] );
         argv[ 0 ] = (char*)EXECUTE;
         execvp(EXECUTE, argv);
         perror("start_kdeinit");
         return 1;
      case 0: { /* child, pass env and exit */
         extern char** environ;
         int i;
         close( pipes[ 0 ] );
         write( pipes[ 1 ], "environ", 7 ); /* header, just in case */
         for( i = 0;
              environ[ i ] != NULL;
              ++i )
             {}
         write( pipes[ 1 ], &i, sizeof( int )); /* write count */
         for( i = 0;
              environ[ i ] != NULL;
              ++i )
             {
             int len = strlen( environ[ i ] );
             write( pipes[ 1 ], &len, sizeof( int )); /* write length */
             write( pipes[ 1 ], environ[ i ], strlen( environ[ i ] ));
             }
         close( pipes[ 1 ] );
         }     
   }
   return 0;
}

#else /* not Linux, the simple non-setuid case */

int main(int argc, char **argv)
{
   if(argc == 0)
      return 1;
   argv[0] = (char*)EXECUTE;
   execvp(EXECUTE,argv);
   perror("start_kdeinit");
   return 1;
}
#endif
