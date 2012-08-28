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
     strlcat, strlcpy, 
     strcasestr,
     setenv, unsetenv, 
     usleep, initgroups, 
     random, srandom  (this is for KRandom itself, prefer using KRandom in any other code)
     mkdtemp (this is for KTempDir itself, prefer using KTempDir everywhere else)
     mkstemp, mkstemps (prefer to use QTemporaryfile instead)
     trunc
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

#cmakedefine01 HAVE_STRLCPY_PROTO
#if ! HAVE_STRLCPY_PROTO
#ifdef __cplusplus
extern "C" {
#endif
unsigned long strlcpy(char*, const char*, unsigned long);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_STRCASESTR_PROTO
#if ! HAVE_STRCASESTR_PROTO
#ifdef __cplusplus
extern "C" {
#endif
char *strcasestr(const char *str1, const char *str2);
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

#cmakedefine01 HAVE_SETENV_PROTO
#if ! HAVE_SETENV_PROTO
#ifdef __cplusplus
extern "C" {
#endif
int setenv (const char *, const char *, int);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_UNSETENV_PROTO
#if ! HAVE_UNSETENV_PROTO
#ifdef __cplusplus
extern "C" {
#endif
int unsetenv (const char *);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_USLEEP_PROTO
#if ! HAVE_USLEEP_PROTO
#ifdef __cplusplus
extern "C" {
#endif
int usleep (unsigned int);
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

#cmakedefine01 HAVE_MKDTEMP_PROTO
#if ! HAVE_MKDTEMP_PROTO
#ifdef __cplusplus
extern "C" {
#endif
char *mkdtemp(char *);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_MKSTEMPS_PROTO
#if ! HAVE_MKSTEMPS_PROTO
#ifdef __cplusplus
extern "C" {
#endif
int mkstemps(char *, int);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_MKSTEMP_PROTO
#if ! HAVE_MKSTEMP_PROTO
#ifdef __cplusplus
extern "C" {
#endif
int mkstemp(char *);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_TRUNC
#if ! HAVE_TRUNC
#ifdef __cplusplus
extern "C" {
#endif
double trunc(double);
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

#cmakedefine01 HAVE_SYS_TIME_H
#cmakedefine01 HAVE_TIME_H
#cmakedefine01 HAVE_ALLOCA_H 
#cmakedefine01 HAVE_STRING_H
#cmakedefine01 HAVE_STRINGS_H
#cmakedefine01 HAVE_STDLIB_H
#cmakedefine01 HAVE_UNISTD_H
#cmakedefine01 HAVE_SYS_STAT_H
#cmakedefine01 HAVE_SYS_TYPES_H
#cmakedefine01 HAVE_MKSTEMPS
#cmakedefine01 HAVE_MKSTEMP
#cmakedefine01 HAVE_MKSTEMPS 
#cmakedefine01 HAVE_SETENV
#cmakedefine01 HAVE_SYS_SELECT_H
#cmakedefine01 HAVE_RANDOM
#cmakedefine01 HAVE_SETEUID
#cmakedefine01 HAVE_ERRNO_H
#cmakedefine01 HAVE_USLEEP
#cmakedefine01 HAVE_MKDTEMP

/* FIXME: Move in a config-kdecore.h file ?*/
#cmakedefine01 HAVE_BACKTRACE
#cmakedefine01 HAVE_VOLMGT
#cmakedefine01 HAVE_SYS_MNTTAB_H
#cmakedefine01 HAVE_MNTENT_H
#cmakedefine01 HAVE_SYS_MNTENT_H
#cmakedefine01 HAVE_SYS_MOUNT_H
#cmakedefine01 HAVE_SYS_PARAM_H
#cmakedefine01 HAVE_FSTAB_H
#cmakedefine01 HAVE_GETMNTINFO
#cmakedefine01 HAVE_SETMNTENT
#cmakedefine01 HAVE_LIMITS_H
#cmakedefine01 HAVE_X11
#cmakedefine01 ENABLE_NLS
#cmakedefine01 HAVE_SYS_TIME_H
#cmakedefine01 TIME_WITH_SYS_TIME
#cmakedefine01 HAVE_DIRENT_D_TYPE
#cmakedefine01 GETMNTINFO_USES_STATVFS
#endif /* KDEFAKES_H */
