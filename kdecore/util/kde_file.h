/*
   This file is part of the KDE libraries
   Copyright (C) 2001 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>

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

#ifndef _KDE_FILE_H_
#define _KDE_FILE_H_

/**
 * \file kde_file.h
 * \brief This file provides portable defines for file support.
 *
 * Use the KDE_xxx defines instead of the normal C
 * functions and structures.
 * \since 3.3
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#if defined _WIN32 || defined _WIN64
#include <kde_file_win.h>
#endif
#include <kdecore_export.h>

/* added not for Solaris and OpenSolaris platforms */

#if (defined _LFS64_LARGEFILE) && (defined _LARGEFILE64_SOURCE) && (!defined _GNU_SOURCE) && (!defined __sun)
/*
 * This section provides portable defines for large file support.
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
#define KDE_stat		::stat64
#define KDE_lstat		::lstat64
#define KDE_fstat		::fstat64
#define KDE_open		::open64
#define KDE_lseek		::lseek64
#define KDE_fseek		::fseek64
#define KDE_ftell		::ftell64
#define KDE_fgetpos		::fgetpos64
#define KDE_fsetpos		::fsetpos64
#define KDE_readdir		::readdir64
#define KDE_sendfile		::sendfile64
#define KDE_struct_stat 	struct stat64
#define KDE_struct_dirent	struct dirent64
#define KDE_rename		::rename
#define KDE_mkdir		::mkdir
/* TODO: define for win32 */

#else /* !_LFS64_LARGEFILE */

/*
 * This section defines portable defines for standard file support.
 */

/*
     Platform specific definitions for Solaris and OpenSolaris tested with gcc 4.3.2
*/
#if defined __sun__ 
#define KDE_stat		::stat
#define KDE_lstat		::lstat
#define KDE_fstat		::fstat
#define KDE_open		::open
#define KDE_lseek		::lseek
#define KDE_fseek		::fseek
#define KDE_ftell		::ftell
#define KDE_fgetpos		::fgetpos
#define KDE_fsetpos		::fsetpos
#define KDE_readdir		::readdir
#define KDE_sendfile		::sendfile
#define KDE_struct_stat 	struct stat
#define KDE_struct_dirent	struct dirent
#define KDE_rename		::rename
#define KDE_mkdir		::mkdir

#else

#if defined _WIN32 || defined _WIN64
#define KDE_stat		kdewin32_stat
#define KDE_lstat		kdewin32_lstat
#define KDE_open		kdewin32_open
#define KDE_rename		kdewin32_rename
#define KDE_mkdir		kdewin32_mkdir
#else /* unix */
#define KDE_stat		::stat
#define KDE_lstat		::lstat
#define KDE_open		::open
#define KDE_rename		::rename
#define KDE_mkdir		::mkdir
#endif

#define KDE_fstat		::fstat
#define KDE_lseek		::lseek
#define KDE_fseek		::fseek
#define KDE_ftell		::ftell
#define KDE_fgetpos		::fgetpos
#define KDE_fsetpos		::fsetpos
#define KDE_readdir		::readdir
#define KDE_sendfile		::sendfile
#define KDE_struct_stat 	struct stat
#define KDE_struct_dirent	struct dirent
#endif

#ifdef _LFS64_STDIO
#define KDE_fopen		::fopen64
#define KDE_freopen		::freopen64
/* TODO: define for win32 */
#else
#if defined _WIN32 || defined _WIN64
#define KDE_fopen		kdewin32_fopen
#define KDE_freopen		kdewin32_freopen
#else /* unix */
#define KDE_fopen		::fopen
#endif
#endif
#endif

/* functions without 64-bit version but wrapped for compatibility reasons */
#if defined _WIN32 || defined _WIN64
#define KDE_fdopen	kdewin32_fdopen
#define KDE_signal	kdewin32_signal
#else /* unix */
#define KDE_fdopen	::fdopen
#define KDE_signal	::signal
#endif

#include <QtCore/QFile>
class QString;
namespace KDE
{
  /** replacement for ::access() to handle filenames in a platform independent way */
  KDECORE_EXPORT int access(const QString &path, int mode);
  /** replacement for ::chmod() to handle filenames in a platform independent way */
  KDECORE_EXPORT int chmod(const QString &path, mode_t mode);
  /** replacement for ::lstat()/::lstat64() to handle filenames in a platform independent way */
  KDECORE_EXPORT int lstat(const QString &path, KDE_struct_stat *buf);
  /** replacement for ::mkdir() to handle pathnames in a platform independent way */
  KDECORE_EXPORT int mkdir(const QString &pathname, mode_t mode);
  /** replacement for ::open()/::open64() to handle filenames in a platform independent way */
  KDECORE_EXPORT int open(const QString &pathname, int flags, mode_t mode = 0);
  /** replacement for ::rename() to handle pathnames in a platform independent way */
  KDECORE_EXPORT int rename(const QString &in, const QString &out);
  /** replacement for ::stat()/::stat64() to handle filenames in a platform independent way */
  KDECORE_EXPORT int stat(const QString &path, KDE_struct_stat *buf);
  /** replacement for ::utime() to handle filenames in a platform independent way */
  KDECORE_EXPORT int utime(const QString &filename, struct utimbuf *buf);
#ifndef Q_WS_WIN
  inline int access(const QString &path, int mode)
  {
    return ::access( QFile::encodeName(path).constData(), mode );
  }
  inline int chmod(const QString &path, mode_t mode)
  {
    return ::chmod( QFile::encodeName(path).constData(), mode );
  }
  inline int lstat(const QString &path, KDE_struct_stat *buf)
  {
    return KDE_lstat( QFile::encodeName(path).constData(), buf );
  }
  inline int mkdir(const QString &pathname, mode_t mode)
  {
    return KDE_mkdir( QFile::encodeName(pathname).constData(), mode );
  }
  inline int open(const QString &pathname, int flags, mode_t mode)
  {
    return KDE_open( QFile::encodeName(pathname).constData(), flags, mode );
  }
  inline int rename(const QString &in, const QString &out)
  {
    return KDE_rename( QFile::encodeName(in).constData(), QFile::encodeName(out).constData() );
  }
  inline int stat(const QString &path, KDE_struct_stat *buf)
  {
    return KDE_stat( QFile::encodeName(path).constData(), buf );
  }
  inline int utime(const QString &filename, struct utimbuf *buf)
  {
    return ::utime( QFile::encodeName(filename).constData(), buf );
  }
#endif
}

#if defined _WIN32 || defined _WIN64
#define KPATH_SEPARATOR ';'
#define KDIR_SEPARATOR '\\' /* faster than QDir::separator() */
#else
#ifndef O_BINARY
#define O_BINARY 0 /* for open() */
#endif
#define KPATH_SEPARATOR ':'
#define KDIR_SEPARATOR '/' /* faster than QDir::separator() */
#endif

#endif /* _KDE_FILE_H_ */
