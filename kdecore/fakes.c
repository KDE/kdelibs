/* This file is part of the KDE libraries
   Copyright (c) 2000 The KDE Project

   unsetenv() taken from the GNU C Library.
   Copyright (C) 1992,1995-1999,2000-2002 Free Software Foundation, Inc.

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

#include <config.h>

#ifndef HAVE_SETENV

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int setenv(const char *name, const char *value, int overwrite) {
    int i;
    char * a;

    if (!overwrite && getenv(name)) return 0;

    i = strlen(name) + strlen(value) + 2;
    a = (char*)malloc(i);
    if (!a) return 1;

    strcpy(a, name);
    strcat(a, "=");
    strcat(a, value);

    return putenv(a);
}
#endif

#ifndef HAVE_UNSETENV

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

extern char ** environ;

void unsetenv (name)
     const char *name;
{
  size_t len;
  char **ep;

  if (name == NULL || *name == '\0' || strchr (name, '=') != NULL)
    {
      errno = EINVAL;
      return;
    }

  len = strlen (name);

  ep = environ;
  while (*ep != NULL)
    if (!strncmp (*ep, name, len) && (*ep)[len] == '=')
      {
	/* Found it.  Remove this pointer by moving later ones back.  */
	char **dp = ep;

	do
	  dp[0] = dp[1];
	while (*dp++);
	/* Continue the loop in case NAME appears again.  */
      }
    else
      ++ep;

}

#endif

#ifndef HAVE_USLEEP

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef __cplusplus  /* this is supposed to be a C source file but still.. */
extern "C" {
#endif

void usleep(unsigned int usec) {
        struct timeval _usleep_tv;
        _usleep_tv.tv_sec = usec/1000000;
        _usleep_tv.tv_usec = usec%1000000;
        select(0,0,0,0,&_usleep_tv);
}

#ifdef __cplusplus
}
#endif

#endif

#if !defined(HAVE_GETDOMAINNAME)

#include <sys/utsname.h>
#include <netdb.h>
#include <strings.h>
#include <errno.h>
#include <stdio.h>

int getdomainname(char *name, size_t len)
{
        struct utsname uts;
        struct hostent *hent;
        int rv = -1;

        if (name == 0L)
          errno = EINVAL;
        else
        {
                name[0] = '\0';
                if (uname(&uts) >= 0)
                {
                        if ((hent = gethostbyname(uts.nodename)) != 0L)
                        {
                                char *p = strchr(hent->h_name, '.');
                                if (p != 0L)
                                {
                                        ++p;
                                        if (strlen(p) > len-1)
                                          errno = EINVAL;
                                        else
                                        {
                                                strcpy(name, p);
                                                rv = 0;
                                        }
                                }
                        }
                }
        }
        return rv;
}


#endif

#ifndef HAVE_RANDOM
long int random()
{
    return lrand48();
}

void srandom(unsigned int seed)
{
    srand48(seed);
}
#endif

#ifndef HAVE_SETEUID
int seteuid(uid_t euid)
{
    setreuid(-1, euid); /* Well, if you have neither you are in trouble :) */
}
#endif

#ifndef HAVE_MKSTEMPS
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

/* this is based on code taken from the GNU libc, distributed under the LGPL license */

/* Generate a unique temporary file name from TEMPLATE.

   TEMPLATE has the form:

   <path>/ccXXXXXX<suffix>

   SUFFIX_LEN tells us how long <suffix> is (it can be zero length).

   The last six characters of TEMPLATE before <suffix> must be "XXXXXX";
   they are replaced with a string that makes the filename unique.

   Returns a file descriptor open on the file for reading and writing.  */

int mkstemps (char* _template, int suffix_len)
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

#endif

#ifndef HAVE_MKSTEMP
int mkstemp (char* _template)
{
  return mkstemps( _template, 0 );
}
#endif


#ifndef HAVE_REVOKE
#include <errno.h>
int revoke(const char *tty)
{
        errno = ENOTSUP;
        return -1;
}
#endif

#ifndef HAVE_STRLCPY
unsigned int strlcpy(char* d, const char* s, unsigned int bufsize)
{
    size_t len, ret = strlen(s);

    if (ret >= bufsize) {
        if (bufsize) {
            len = bufsize - 1;
            memcpy(d, s, len);
            d[len] = '\0';
        }
    } else
	memcpy(d, s, ret + 1);
	
    return ret;
}
#endif

#ifndef HAVE_STRLCAT
unsigned int strlcat(char* d, const char* s, unsigned int bufsize)
{
    char *cp;
    unsigned int len1;
    unsigned int len2 = strlen(s);
    unsigned int ret;

    cp = memchr (d, '\0', bufsize);
    if (!cp)
	return bufsize + len2;
    len1 = cp - d;
    ret = len1 + len2;
    if (ret >= bufsize) {
        len2 = bufsize - len1 - 1;
        memcpy(cp, s, len2);
        cp[len2] = '\0';
    } else
        memcpy(cp, s, len2 + 1);

    return ret;
}
#endif
