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

#include <kdelibs_export.h>

#include <stdlib.h>
#include <string.h>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/*
// from kdecore/fakes.c
KDEWIN32_EXPORT unsigned long strlcpy(char *d, const char *s, unsigned long bufsize)
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
*/
/*
// from kdecore/fakes.c
KDEWIN32_EXPORT unsigned long strlcat(char *d, const char *s, unsigned long bufsize)
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
*/
KDEWIN32_EXPORT char* strndup(const char *src, size_t n)
{
	const size_t len = MIN(strlen(src), n);
	char *copy = malloc(len + 1);
	if (copy) {
		memcpy (copy, src, len);
		copy[len] = '\0';
	}
	return copy;
}

