/* 
   This file is part of the KDE libraries
   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
   
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

#ifndef _KDE_LARGEFILE_H_
#define _KDE_LARGEFILE_H_

/**
 * This file defines portable defines for large file support.
 * To use this you must compile your code with _LARGEFILE64_SOURCE 
 * defined and use the KDE_xxx defines instead of the normal
 * C functions and structures.
 *
 * Please note that not every platform supports 64 bit file structures,
 * in that case the normal 32 bit functions will be used.
 *
 * @see http://www.suse.de/~aj/linux_lfs.html 
 * @see http://ftp.sas.com/standards/large.file/xopen/x_open.05Mar96.html
 *
 * KDE makes use of the "Transitional Extensions" since we can not ensure
 * that all modules and libraries used by KDE will be compiled with
 * 64-bit support. 
 * (A.3.2.3 Mixed API and Compile Environments within a Single Process)
 */

#include <unistd.h>

#ifdef _LFS64_LARGEFILE
#define KDE_stat		::stat64
#define KDE_lstat		::lstat64
#define KDE_fstat		::fstat64
#define KDE_open		::open64
#define KDE_lseek		::lseek64
#define KDE_readdir		::readdir64
#define KDE_struct_stat 	struct stat64
#define KDE_struct_dirent	struct dirent64
#else
#define KDE_stat		::stat
#define KDE_lstat		::lstat
#define KDE_fstat		::fstat
#define KDE_open		::open
#define KDE_lseek		::lseek
#define KDE_readdir		::readdir
#define KDE_struct_stat 	struct stat
#define KDE_struct_dirent	struct dirent
#endif

#ifdef _LFS64_STDIO
#define KDE_fopen		fopen64
#else
#define KDE_fopen		fopen
#endif

#endif
