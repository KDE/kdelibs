/*
 * stat.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Symbolic constants for opening and creating files, also stat, fstat and
 * chmod functions.
 *
 */
#ifndef KDE_STAT_H_
#define KDE_STAT_H_

#include_next <sys/stat.h>

#define _S_IFLNK	0xF000  /* Pretend */
#define S_ISLNK(m)	(((m) & _S_IFMT) == _S_IFLNK) /* Should always be zero.*/

#endif
