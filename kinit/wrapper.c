/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
            (c) 1999 Mario Weilguni <mweilguni@sime.com>

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
#include "config.h"
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
     else if ((result == -1) && (errno != EINTR))
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
     else if ((result == -1) && (errno != EINTR))
        return -1;
  }
  return 0;
}

static int openSocket()
{
  ksize_t socklen;
  int s;
  struct sockaddr_un server;
#define MAX_SOCK_FILE 255
  char sock_file[MAX_SOCK_FILE];
  char *home_dir = getenv("HOME");
  char *kde_home = getenv("KDEHOME");
  char *display;

  sock_file[0] = 0;

  if (!kde_home || !kde_home[0])
  {
     kde_home = "~/.kde/";
  }

  if (kde_home[0] == '~')
  {
     if (!home_dir || !home_dir[0])
     {
        fprintf(stderr, "Aborting. $HOME not set!");
        exit(255);
     }
     if (strlen(home_dir) > (MAX_SOCK_FILE-100))
     {
        fprintf(stderr, "Aborting. Home directory path too long!");
        exit(255);
     }
     kde_home++;
     strcat(sock_file, home_dir);
  }
  strcat(sock_file, kde_home);

  /** Strip trailing '/' **/
  if ( sock_file[strlen(sock_file)-1] == '/')
     sock_file[strlen(sock_file)-1] = 0;
  
  strcat(sock_file, "/socket-");
  if (gethostname(sock_file+strlen(sock_file), MAX_SOCK_FILE - strlen(sock_file) - 1) != 0)
  {
     perror("Aborting. Could not determine hostname: ");
     exit(255);
  }

  /* append $DISPLAY */
  display = getenv("DISPLAY");
  if (!display) 
  {
     fprintf(stderr, "Aborting. $DISPLAY is not set.\n");
     exit(255);
  }
  if (strlen(sock_file)+strlen(display)+2 > MAX_SOCK_FILE)
  {
     fprintf(stderr, "Aborting. Socket name will be too long.\n");
     exit(255);
  }
  strcat(sock_file, "/kdeinit-");
  strcat(sock_file, display);

  if (strlen(sock_file) >= sizeof(server.sun_path))
  {
     fprintf(stderr, "Aborting. Path of socketfile exceeds UNIX_PATH_MAX.\n");
     exit(255);
  }

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
  if(connect(s, (struct sockaddr *)&server, socklen) == -1) 
  {
     perror("connect() failed: ");
     close(s);
     exit(255);
  }
  return s;
}

int main(int argc, char **argv)
{
   int i;
   long arg_count;
   klauncher_header header;
   char *start, *p, *buffer;
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
   {
      argv++;
      argc--;
      start = argv[0];
   }
   arg_count = argc;

   size += sizeof(long); /* Number of arguments*/

   size += strlen(start)+1; /* Size of first argument. */

   for(i = 1; i < argc; i++)
   {
      size += strlen(argv[i])+1;
   }

   header.cmd = LAUNCHER_EXEC;
   header.arg_length = size;
   write_socket(sock, (char *) &header, sizeof(header));

   buffer = (char *) malloc(size);
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
      read_socket(sock, buffer, header.arg_length);
      pid = *((long *) buffer);
      printf("Launched ok, pid = %ld\n", pid);
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
   exit(2);
}
