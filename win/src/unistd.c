/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <windows.h>
#include <winposix_export.h>

#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

#include "win32_utils.h"

KDEWIN32_EXPORT int getgroups(int size, gid_t list[])
{
	/* TODO */
	return 0;
}

KDEWIN32_EXPORT int readlink(const char *__path, char *__buf, int __buflen)
{
    if (!__path) {
      errno = EINVAL;
      return -1;
    }
    if ( (__buflen < 0) || ((int)strlen(__path)>(__buflen-1)) )
    {
      errno = ENAMETOOLONG;
      return -1;
    }
    if (access(__path, R_OK) == 0) {
      /* ok, copy to buf */
      strncpy(__buf,__path,__buflen);
      errno = 0;
      return 0;
    }
    errno = ENOENT;
    return -1;
}

KDEWIN32_EXPORT int symlink(const char *__name1, const char *__name2)
{
	return fcopy(__name1, __name2);
}

KDEWIN32_EXPORT int link(const char *__name1, const char *__name2)
{
	return fcopy(__name1, __name2);
}

KDEWIN32_EXPORT int chown(const char *__path, uid_t __owner, gid_t __group)
{ 
  return 0; 
}

KDEWIN32_EXPORT int fchown(int __fd, uid_t __owner, gid_t __group )
{
  return 0; 
}

KDEWIN32_EXPORT int lstat(const char *path, struct stat *sb)
{
  return _stat(path,(struct _stat*)sb);
}

KDEWIN32_EXPORT int fchmod(int __fd, mode_t __mode)
{
  return 0;
}


/* Get the real user ID of the calling process.  */
KDEWIN32_EXPORT uid_t getuid()
{
  return 1; /* NOT A ROOT! */
}

/* Get the effective user ID of the calling process.  */
KDEWIN32_EXPORT uid_t geteuid (void)
{
  return 1; /* NOT A ROOT! */
}

/* Get the real group ID of the calling process.  */
KDEWIN32_EXPORT gid_t getgid (void)
{
  return 1; /* NOT A ROOT GR! */
}

/* Get the effective group ID of the calling process.  */
KDEWIN32_EXPORT gid_t getegid (void)
{
  return 1; /* NOT A ROOT GR! */
}

KDEWIN32_EXPORT int pipe(int *fd)
{
  /** @todo */
  return _pipe( fd, 256, O_BINARY ); /* OK? */
}

KDEWIN32_EXPORT pid_t fork(void)
{
  /** @todo */
  return -1;
}

KDEWIN32_EXPORT pid_t setsid(void)
{
  /** @todo */
  return -1;
}

KDEWIN32_EXPORT unsigned alarm(unsigned __secs )
{
  /** @todo */
  return 1;
}

typedef unsigned int size_t;

/*#define INCL_WINSOCK_API_PROTOTYPES 0
#include <winsock2.h>*/

// using winsock gethostname(), which would be taken normally requires WSAStartup called before 
// which will not be done in every case
//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/winsock/gethostname_2.asp

KDEWIN32_EXPORT int kde_gethostname(char *__name, size_t __len)
{
  size_t len = __len;
  if (0==GetComputerNameA(__name, &len))
    return -1;
  return 0;
}

#define getlogin_buf_size 255
char getlogin_buf[getlogin_buf_size+1];

KDEWIN32_EXPORT char* getlogin()
{
/*! @todo make this reentrant!*/
	size_t size = sizeof(getlogin_buf);
	*getlogin_buf = 0;
	if (!GetUserNameA(getlogin_buf, (LPDWORD)&size))
		return 0;
	return getlogin_buf;
}

KDEWIN32_EXPORT void usleep(unsigned int usec)
{
	Sleep(usec/1000);
}

KDEWIN32_EXPORT void sleep(unsigned int sec)
{
	Sleep(sec*1000);
}

KDEWIN32_EXPORT long int random()
{
	return rand();
}

KDEWIN32_EXPORT void srandom(unsigned int seed)
{
	srand(seed);
}

KDEWIN32_EXPORT int setreuid(uid_t ruid, uid_t euid)
{
	/*! @todo */
	return 0;
}


/* this is based on code taken from the GNU libc, distributed under the LGPL license */

/* Generate a unique temporary file name from TEMPLATE.

   TEMPLATE has the form:

   <path>/ccXXXXXX<suffix>

   SUFFIX_LEN tells us how long <suffix> is (it can be zero length).

   The last six characters of TEMPLATE before <suffix> must be "XXXXXX";
   they are replaced with a string that makes the filename unique.

   Returns a file descriptor open on the file for reading and writing.  */

KDEWIN32_EXPORT int mkstemps (char* _template, int suffix_len)
{
  static const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  char *XXXXXX;
  int len;
  int count;
  int value;

  len = strlen (_template);

  if ((int) len < 6 + suffix_len || strncmp (&_template[len - 6 - suffix_len], "XXXXXX", 6))
      return -1;

  XXXXXX = &_template[len - 6 - suffix_len];

  value = rand();
  for (count = 0; count < 256; ++count)
  {
      int v = value;
      int fd;

      /* Fill in the random bits.  */
      XXXXXX[0] = letters[v % 62];
      v /= 62;
      XXXXXX[1] = letters[v % 62];
      v /= 62;
      XXXXXX[2] = letters[v % 62];
      v /= 62;
      XXXXXX[3] = letters[v % 62];
      v /= 62;
      XXXXXX[4] = letters[v % 62];
      v /= 62;
      XXXXXX[5] = letters[v % 62];

      fd = open (_template, O_RDWR|O_CREAT|O_EXCL, 0600);
      if (fd >= 0)
		/* The file does not exist.  */
		return fd;

      /* This is a random value.  It is only necessary that the next
	 TMP_MAX values generated by adding 7777 to VALUE are different
	 with (module 2^32).  */
      value += 7777;
    }
  /* We return the null string if we can't find a unique file name.  */
  _template[0] = '\0';
  return -1;
}

// from kdecore/fakes.c
KDEWIN32_EXPORT int mkstemp (char* _template)
{
  return mkstemps( _template, 0 );
}

// from kdecore/fakes.c
int seteuid(uid_t euid)
{
    return setreuid(-1, euid); /* Well, if you have neither you are in trouble :) */
}

// from kdecore/fakes.c
/* Generate a unique temporary directory name from TEMPLATE.

   TEMPLATE has the form:

   <path>/ccXXXXXX


   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.

   Returns a file descriptor open on the file for reading and writing.  */

KDEWIN32_EXPORT char* mkdtemp (char* _template)
{
  static const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  char *XXXXXX;
  int len;
  int count;
  int value;

  len = strlen (_template);

  if ((int) len < 6 || strncmp (&_template[len - 6], "XXXXXX", 6))
      return 0;

  XXXXXX = &_template[len - 6];

  value = rand();
  for (count = 0; count < 256; ++count)
  {
      int v = value;

      /* Fill in the random bits.  */
      XXXXXX[0] = letters[v % 62];
      v /= 62;
      XXXXXX[1] = letters[v % 62];
      v /= 62;
      XXXXXX[2] = letters[v % 62];
      v /= 62;
      XXXXXX[3] = letters[v % 62];
      v /= 62;
      XXXXXX[4] = letters[v % 62];
      v /= 62;
      XXXXXX[5] = letters[v % 62];

      /* This is a random value.  It is only necessary that the next
	 TMP_MAX values generated by adding 7777 to VALUE are different
	 with (module 2^32).  */
      value += 7777;

      if (!mkdir(_template))
		return _template;	
    }
    return 0;
}


// from kdecore/fakes.c
#ifndef ENOTSUP
#define ENOTSUP 134 /* Not supported */
#endif
KDEWIN32_EXPORT int revoke(const char *tty)
{
        errno = ENOTSUP;
        return -1;
}

