/*
  This file is part of the KDE libraries
  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>

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

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
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
#include <grp.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

/* Misc helper functions: */
static int my_seteuid( uid_t euid)
{
#ifdef HAVE_SETEUID
     return seteuid(euid);
#else
     return setreuid(-1, euid);
#endif
}

/* Misc. functions */
static int my_setegid( gid_t egid)
{
#ifdef HAVE_SETEUID
     return setegid(egid);
#else
     return setregid(-1, egid);
#endif
}

/*
 * @return 0 everything ok.
 *         1 directory needs to be made
 *         2 directory/file needs to be removed, then remade
 */
static int check_tmp_dir(const char *tmp_dir)
{
  int result;
  struct stat stat_buf;
 
  printf("Checking \"%s\".\n", tmp_dir);

  result = lstat(tmp_dir, &stat_buf);
  if ((result == -1) && (errno == ENOENT))
  {
    return 1;
  }
  if ((result == -1) || (!S_ISDIR(stat_buf.st_mode)))
  {
     fprintf(stderr, "Warning: \"%s\" is not a directory.\n", tmp_dir);
     return 2;
  }

  if (stat_buf.st_uid != getuid())
  {
     fprintf(stderr, "Warning: \"%s\" is owned by uid %d instead of uid %d.\n", tmp_dir, stat_buf.st_uid, getuid());
     return 2;
  }
  return 0;
}

int main(/*int argc, char **argv*/)
{
  char user_tmp_dir[PATH_MAX+1];
  struct passwd *pw_ent;
  int result;
  int gidset_size;
  gid_t *gidset;

  pw_ent = getpwuid(getuid());
  if (!pw_ent)
  {
     fprintf(stderr, "Error: Can not find password entry for uid %d.\n", getuid());
     return 1;
  }

  strcpy(user_tmp_dir, "/tmp/");
  strcat(user_tmp_dir, pw_ent->pw_name);

  result = check_tmp_dir(user_tmp_dir);
  
  if (result == 0)
      return 0; /* Ok, we are done. */

  if (result == 2)
  {
     char *buf;
     if(setuid(0) != 0) 
     {
        fprintf(stderr, "Error: Can not set root privileges.\n");
        return 1;
     }
     buf = (char *) malloc(strlen(user_tmp_dir)+100);
     if (!buf)
     {
        fprintf(stderr, "Error: Can not allocate memory.\n");
        return 1;
     }
     sprintf(buf, "/bin/rm -rf \'%s\'", user_tmp_dir);
     printf("Removing \"%s\".\n", user_tmp_dir);
     system(buf);
  }

  gidset_size = getgroups(0,0);
  gidset = (gid_t *) malloc(sizeof(gid_t) * gidset_size);

  if( getgroups( gidset_size, gidset) == -1 ||
      initgroups(pw_ent->pw_name, pw_ent->pw_gid) != 0 ||
      my_setegid(pw_ent->pw_gid) != 0 ||
      my_seteuid(pw_ent->pw_uid) != 0) 
  {
      /* Error */
      fprintf(stderr, "Error: Unable to set proper credentials.\n");
      return 1;
  }
  printf("Creating \"%s\".\n", user_tmp_dir);
  if (mkdir(user_tmp_dir, 0755) != 0)
  {
      fprintf(stderr, "Error: Could not create directory \"%s\".\n", user_tmp_dir);
      return 1;
  }
  return 0;
}
