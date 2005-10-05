/*
   This file is part of the KDE libraries
   Copyright (C) 2003-2004 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KDEWIN_STRING_H
#define KDEWIN_STRING_H

#include <kdecore/kdelibs_export.h>

#define strncasecmp strnicmp
#define strcasecmp stricmp

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* implemented in fakes.c */
KDECORE_EXPORT unsigned long strlcpy(char *dst, const char *src, unsigned long siz);

/* implemented in fakes.c */
KDECORE_EXPORT unsigned long strlcat(char *dst, const char *src, unsigned long siz);

KDEWIN32_EXPORT char* strndup(const char *src, size_t n);

#ifdef __cplusplus
}
#endif

/* regular string.h */
#ifdef _MSC_VER
# include <msvc/string.h>
#elif defined(__BORLANDC__)
# include <bcc/string.h>
#endif

#endif /* KDEWIN_STRING_H */
