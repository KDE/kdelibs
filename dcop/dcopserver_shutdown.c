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

#define BUFFER_SIZE 4096

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
      display = "NODISPLAY";
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

static void getDCOPFile(char *dcop_file, char *dcop_file_old, int max_length)
{
  const char *home_dir;
  char *display;
  char *i;
  int n;

  n = max_length;
  home_dir = getenv("HOME");
  strncpy(dcop_file, home_dir, n);
  dcop_file[ n - 1 ] = '\0';
  n -= strlen(home_dir);
  
  strncat(dcop_file, "/.DCOPserver_", n);
  n -= strlen("/.DCOPserver_");

  if (gethostname(dcop_file+strlen(dcop_file), n) != 0)
  {
     perror("Error. Could not determine hostname: ");
     dcop_file[0] = '\0';
     return;
  }
  n = max_length - strlen(dcop_file);

  strncat(dcop_file, "_", n);
  n -= strlen("_");

  display = getDisplay();
  if (display == NULL)
  {
     dcop_file[0] = '\0';
     return; /* barf */
  }

  strcpy(dcop_file_old, dcop_file);
  strncat(dcop_file_old,display, n);
  while((i = strchr(display, ':')))
     *i = '_';
  strncat(dcop_file, display, n);
  free(display);

  return;
}

static void cleanupDCOPsocket(char *buffer)
{
   char cmd[BUFFER_SIZE];
   const char *socket_file;
   int l; 

   l = strlen(buffer);
   if (!l)
      return;
   buffer[l-1] = '\0'; /* strip LF */

   socket_file = strchr(buffer, ':');
   if (socket_file)
     socket_file++;

   if (socket_file)
      unlink(socket_file);

   snprintf(cmd, BUFFER_SIZE, "iceauth remove netid='%s'", buffer);
   system(cmd);
}

static void cleanupDCOP(int dont_kill_dcop)
{
   FILE *f;
   char dcop_file[2048+1];
   char dcop_file_old[2048+1];
   char buffer[2048+1];
   pid_t pid = 0;

   getDCOPFile(dcop_file, dcop_file_old, 2048);
   if (strlen(dcop_file) == 0)
      return;

   f = fopen(dcop_file, "r");
   unlink(dcop_file); /* Clean up .DCOPserver file */
   unlink(dcop_file_old);
   if (!f)
      return;

   while (!feof(f))
   {
      fgets(buffer, 2048, f);
      pid = strtol(buffer, NULL, 10);
      if (pid)
         break;
      cleanupDCOPsocket(buffer);
   }
   if (!dont_kill_dcop && pid)
      kill(pid, SIGTERM);
   fclose(f);
}

int main(int argc, char **argv)
{
   int dont_kill_dcop = (argc == 2) && (strcmp(argv[1], "--nokill") == 0);

   cleanupDCOP(dont_kill_dcop);
   return 0;
}
