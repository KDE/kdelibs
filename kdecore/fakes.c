/* This file is part of the KDE libraries
   Copyright (c) 2000 Stephan Kulow <coolo@kde.org>
   Copyright (c) 2002 Dirk Mueller <mueller@kde.org>
   Copyright (c) 2002 Oswald Buddenhagen <ossi@kde.org>
   Copyright (c) 2003 Joseph Wenninger <kde@jowenn.at>
   Copyright (c) 2005 Jarosław Staniek <staniek@kde.org>
   Copyright (c) 2007 Rafael Fernández López <ereslibre@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kdecore_export.h>

#include <config.h>



#define KDE_open open
#define KDE_mkdir mkdir


#ifndef HAVE_SETENV

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

KDECORE_EXPORT int setenv(const char *name, const char *value, int overwrite) {
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
#endif /* !HAVE_SETENV */

#ifndef HAVE_UNSETENV

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef environ
extern char ** environ;
#endif

KDECORE_EXPORT int unsetenv (const char *name)
{
  size_t len;
  char **ep;

  if (name == NULL || *name == '\0' || strchr (name, '=') != NULL)
    {
      errno = EINVAL;
      return -1;
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

  return 0;
}

#endif /* !HAVE_UNSETENV */

#ifndef HAVE_USLEEP

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if defined(HAVE_SYS_TIME_H)
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

#endif /* !HAVE_USLEEP */

#ifndef HAVE_RANDOM
long int random()
{
    return lrand48();
}

void srandom(unsigned int seed)
{
    srand48(seed);
}
#endif /* !HAVE_RANDOM */

#ifndef HAVE_SETEUID
int seteuid(uid_t euid)
{
    return setreuid(-1, euid); /* Well, if you have neither you are in trouble :) */
}
#endif /* !HAVE_SETEUID */

#ifndef HAVE_MKSTEMPS
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <fcntl.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* this is based on code taken from the GNU libc, distributed under the LGPL license */

/* Generate a unique temporary file name from TEMPLATE.

   TEMPLATE has the form:

   <path>/ccXXXXXX<suffix>

   SUFFIX_LEN tells us how long <suffix> is (it can be zero length).

   The last six characters of TEMPLATE before <suffix> must be "XXXXXX";
   they are replaced with a string that makes the filename unique.

   Returns a file descriptor open on the file for reading and writing.  */

KDECORE_EXPORT int mkstemps (char* _template, int suffix_len)
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

      fd = KDE_open (_template, O_RDWR|O_CREAT|O_EXCL, 0600);
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

#endif /* !HAVE_MKSTEMPS */

#ifndef HAVE_MKSTEMP
KDECORE_EXPORT int mkstemp (char* _template)
{
  return mkstemps( _template, 0 );
}
#endif /* !HAVE_MKSTEMP */

#ifndef HAVE_MKDTEMP

#ifndef HAVE_MKSTEMPS
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#endif

/* Generate a unique temporary directory name from TEMPLATE.

   TEMPLATE has the form:

   <path>/ccXXXXXX


   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.

   Returns a file descriptor open on the file for reading and writing.  */

KDECORE_EXPORT char* mkdtemp (char* _template)
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

      if (!KDE_mkdir(_template,0700))
	return _template;
    }
    return 0;
}
#endif /* !HAVE_MKDTEMP */

#ifndef HAVE_STRLCPY

#ifdef HAVE_STRING_H
#include <string.h>
#endif

KDECORE_EXPORT unsigned long strlcpy(char* d, const char* s, unsigned long bufsize)
{
    unsigned long len, ret = strlen(s);

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
#endif /* !HAVE_STRLCPY */

#ifndef HAVE_STRLCAT

#ifdef HAVE_STRING_H
#include <string.h>
#endif

KDECORE_EXPORT unsigned long strlcat(char* d, const char* s, unsigned long bufsize)
{
    char *cp;
    unsigned long ret, len1, len2 = strlen(s);

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
#endif /* !HAVE_STRLCAT */

#ifndef HAVE_STRCASESTR
/*
 * My personal strstr() implementation that beats most other algorithms.
 * Until someone tells me otherwise, I assume that this is the
 * fastest implementation of strstr() in C.
 * I deliberately chose not to comment it.  You should have at least
 * as much fun trying to understand it, as I had to write it :-).
 *
 * Stephen R. van den Berg, berg@pool.informatik.rwth-aachen.de */

#include <ctype.h>

KDECORE_EXPORT char *strcasestr (phaystack, pneedle)
     const char *phaystack;
     const char *pneedle;
{
  register const unsigned char *haystack, *needle;
  register unsigned b, c;

  haystack = (const unsigned char *) phaystack;
  needle = (const unsigned char *) pneedle;

  b = tolower (*needle);
  if (b != '\0')
    {
      haystack--;                               /* possible ANSI violation */
      do
        {
          c = *++haystack;
          if (c == '\0')
            goto ret0;
        }
      while (tolower (c) != (int) b);

      c = tolower (*++needle);
      if (c == '\0')
        goto foundneedle;
      ++needle;
      goto jin;

      for (;;)
        {
          register unsigned a;
          register const unsigned char *rhaystack, *rneedle;

          do
            {
              a = *++haystack;
              if (a == '\0')
                goto ret0;
              if (tolower (a) == (int) b)
                break;
              a = *++haystack;
              if (a == '\0')
                goto ret0;
shloop:
              ;
            }
          while (tolower (a) != (int) b);

jin:      a = *++haystack;
          if (a == '\0')
            goto ret0;

          if (tolower (a) != (int) c)
            goto shloop;

          rhaystack = haystack-- + 1;
          rneedle = needle;
          a = tolower (*rneedle);

          if (tolower (*rhaystack) == (int) a)
            do
              {
                if (a == '\0')
                  goto foundneedle;
                ++rhaystack;
                a = tolower (*++needle);
                if (tolower (*rhaystack) != (int) a)
                  break;
                if (a == '\0')
                  goto foundneedle;
                ++rhaystack;
                a = tolower (*++needle);
              }
            while (tolower (*rhaystack) == (int) a);

          needle = rneedle;             /* took the register-poor approach */

          if (a == '\0')
            break;
        }
    }
foundneedle:
  return (char*) haystack;
ret0:
  return 0;
}
#endif /* !HAVE_STRCASESTR */

#ifndef HAVE_TRUNC

#include <math.h> /* floor */

/*
 * Here we simulate the trunc() function behavior. This function is not
 * available for not C99 compatible systems.
 *
 * For example, Solaris 8.
 */

KDECORE_EXPORT double trunc (double x)
{
       return x < 0 ? -floor(-x) : floor(x);
}
#endif /* !HAVE_TRUNC */
