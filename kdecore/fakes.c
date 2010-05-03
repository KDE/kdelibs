/* This file is part of the KDE libraries
   Copyright (c) 2000 Stephan Kulow <coolo@kde.org>
   Copyright (c) 2002 Dirk Mueller <mueller@kde.org>
   Copyright (c) 2002 Oswald Buddenhagen <ossi@kde.org>
   Copyright (c) 2003 Joseph Wenninger <kde@jowenn.at>
   Copyright (c) 2005 Jarosław Staniek <staniek@kde.org>
   Copyright (c) 2007 Rafael Fernández López <ereslibre@kde.org>

   unsetenv() taken from the GNU C Library.
   Copyright (C) 1992,1995-1999,2000-2002 Free Software Foundation, Inc.

   getgrouplist() taken from the FreeBSD libc. The copyright notice
   in the file /usr/src/lib/libc/gen/getgrouplist.c on FreeBSD is out
   of date, as it has incorrect copyright years and still names the
   4-clause BSD license -- however, the Regents of the University of
   California at Berkeley have declared that clause 3 is void; only
   clauses 1,2 and 4 apply. See the full license text below.
   Copyright (c) 1991, 1993
        The Regents of the University of California.  All rights reserved.

 

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

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $OpenBSD: strlcat.c,v 1.2 1999/06/17 16:28:58 millert Exp $
 * $FreeBSD: src/lib/libc/string/strlcat.c,v 1.2.4.2 2001/07/09 23:30:06 obrien Exp $
 * $DragonFly: src/sys/libkern/strlcat.c,v 1.5 2007/06/07 23:45:02 dillon Exp $
 */
 
/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(initial dst) + strlen(src); if retval >= siz,
 * truncation occurred.
 */

#ifndef HAVE_STRLCAT

#ifdef HAVE_STRING_H
#include <string.h>
#endif

KDECORE_EXPORT unsigned long strlcat(char *dst, const char *src, unsigned long siz)
{
    char *d = dst;
    const char *s = src;
    unsigned long n = siz;
    unsigned long dlen;
    
    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
        d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0)
        return(dlen + strlen(s));
    while (*s != '\0') {
        if (n != 1) {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return(dlen + (s - src));       /* count does not include NUL */
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


#ifndef HAVE_GETGROUPLIST
/* Although this full license text is 4-clause BSD, it is taken directly
   from the FreeBSD source tree; the Regents of the University of
   California have deleted clause 3. See
      http://www.freebsd.org/copyright/license.html
   In addition, FreeBSD itself is now under a 2-clause BSD license,
   which strikes clause 4 itself. That makes the *effective* license
   on this file a 2-clause BSD license. I am including the complete
   file, unmodified except for the #if 0 which is needed to avoid
   the FreeBSD SCCS IDs.
*/
/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getgrouplist.c	8.2 (Berkeley) 12/8/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/getgrouplist.c,v 1.14 2005/05/03 16:20:03 delphij Exp $");
#endif

/*
 * get credential
 */
#include <sys/types.h>

#include <grp.h>
#include <string.h>
#include <unistd.h>

int
getgrouplist(const char *uname, gid_t agroup, gid_t *groups, int *grpcnt)
{
	const struct group *grp;
	int i, maxgroups, ngroups, ret;

	ret = 0;
	ngroups = 0;
	maxgroups = *grpcnt;
	/*
	 * When installing primary group, duplicate it;
	 * the first element of groups is the effective gid
	 * and will be overwritten when a setgid file is executed.
	 */
	groups[ngroups++] = agroup;
	if (maxgroups > 1)
		groups[ngroups++] = agroup;
	/*
	 * Scan the group file to find additional groups.
	 */
	setgrent();
	while ((grp = getgrent()) != NULL) {
		for (i = 0; i < ngroups; i++) {
			if (grp->gr_gid == groups[i])
				goto skip;
		}
		for (i = 0; grp->gr_mem[i]; i++) {
			if (!strcmp(grp->gr_mem[i], uname)) {
				if (ngroups >= maxgroups) {
					ret = -1;
					break;
				}
				groups[ngroups++] = grp->gr_gid;
				break;
			}
		}
skip:
		;
	}
	endgrent();
	*grpcnt = ngroups;
	return (ret);
}
#endif

