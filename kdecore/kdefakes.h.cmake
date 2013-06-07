/* This file is part of the KDE libraries
   Copyright (c) 2006 The KDE Project

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

#ifndef KDEFAKES_H
#define KDEFAKES_H

/* This file defines the prototypes for a few (C library) functions for 
   platforms which either
   1) have those functions, but lack the prototypes in their header files.
   2) don't have those functions, in which case kdecore provides them

   You should include this file in any .cpp file that uses any one of these 
   functions:
     strlcat, 
     initgroups, 
     random, srandom  (this is for KRandom itself, prefer using KRandom in any other code)
     getgrouplist
*/

#cmakedefine01 HAVE_STRLCAT_PROTO
#if ! HAVE_STRLCAT_PROTO
#ifdef __cplusplus
extern "C" {
#endif
unsigned long strlcat(char*, const char*, unsigned long);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_RANDOM_PROTO 
#if ! HAVE_RANDOM_PROTO
#ifdef __cplusplus
extern "C" {
#endif
long int random(void);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_SRANDOM_PROTO 
#if ! HAVE_SRANDOM_PROTO
#ifdef __cplusplus
extern "C" {
#endif
void srandom(unsigned int);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_INITGROUPS_PROTO
#if ! HAVE_INITGROUPS_PROTO
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif
int initgroups(const char *, gid_t);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_GETGROUPLIST
#if ! HAVE_GETGROUPLIST
#include <sys/types.h> /* for gid_t */
#ifdef __cplusplus
extern "C" {
#endif
int getgrouplist(const char *, gid_t , gid_t *, int *);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_ALLOCA_H
#cmakedefine01 HAVE_STRING_H
#cmakedefine01 HAVE_STDLIB_H
#cmakedefine01 HAVE_ERRNO_H
#cmakedefine01 HAVE_UNISTD_H
#cmakedefine01 TIME_WITH_SYS_TIME
#cmakedefine01 HAVE_SYS_SELECT_H
#cmakedefine01 HAVE_RANDOM
#cmakedefine01 HAVE_SETEUID
#cmakedefine01 HAVE_SYS_TYPES_H
#cmakedefine01 HAVE_SYS_STAT_H
#cmakedefine01 HAVE_STRINGS_H

#endif /* KDEFAKES_H */
