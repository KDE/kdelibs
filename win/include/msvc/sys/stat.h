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

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <kdelibs_export.h>

/* regular header from msvc includes */
#include <../include/sys/stat.h>

typedef short nlink_t;

#include <time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined _STAT_H_ && !defined _INC_DIRECT

#define	_IFMT		0170000	// type of file 
#define		_IFDIR	0040000	// directory 
#define		_IFCHR	0020000	// character special 
#define		_IFBLK	0060000	// block special 
#define		_IFREG	0100000	// regular 
#define		_IFLNK	0120000	// symbolic link 
#define		_IFSOCK	0140000	// socket 
#define		_IFIFO	0010000	// fifo 

#define 	S_BLKSIZE  1024 // size of a block 

#define	S_ISUID		0004000	// set user id on execution 
#define	S_ISGID		0002000	// set group id on execution 
#define	S_ISVTX		0001000	// save swapped text even after use 

#define	S_IFBLK		_IFBLK
#define	S_IFLNK		_IFLNK
#define	S_IFSOCK	_IFSOCK
#define	S_IFIFO		_IFIFO

#define	S_IRWXU 	(S_IRUSR | S_IWUSR | S_IXUSR)
#define		S_IRUSR	0000400	/* read permission, owner */
#define		S_IWUSR	0000200	/* write permission, owner */
#define		S_IXUSR 0000100/* execute/search permission, owner */
#define	S_IRWXG		(S_IRGRP | S_IWGRP | S_IXGRP)
#define		S_IRGRP	0000040	/* read permission, group */
#define		S_IWGRP	0000020	/* write permission, grougroup */
#define		S_IXGRP 0000010/* execute/search permission, group */
#define	S_IRWXO		(S_IROTH | S_IWOTH | S_IXOTH)
#define		S_IROTH	0000004	/* read permission, other */
#define		S_IWOTH	0000002	/* write permission, other */
#define		S_IXOTH 0000001/* execute/search permission, other */

#define	S_ISBLK(m)	(((m)&_IFMT) == _IFBLK)
#define	S_ISCHR(m)	(((m)&_IFMT) == _IFCHR)
#define	S_ISDIR(m)	(((m)&_IFMT) == _IFDIR)
#define	S_ISFIFO(m)	(((m)&_IFMT) == _IFIFO)
#define	S_ISREG(m)	(((m)&_IFMT) == _IFREG)
#define	S_ISLNK(m)	(((m)&_IFMT) == _IFLNK)
#define	S_ISSOCK(m)	(((m)&_IFMT) == _IFSOCK)

KDEWIN32_EXPORT int lstat( const char *__path, struct stat *__buf);

// KDEWIN32_EXPORT int mkdir( const char *_path, mode_t __mode );
 extern int mkdir( const char *_path );
 extern int chdir( const char *dirname );
 extern char *getcwd( char *buffer, int maxlen );
 extern int rmdir( const char *dirname );

#endif

#ifdef __cplusplus
}
#endif

/*
#ifdef __cplusplus
//this is a C++ function!!!
//TODO: make available C function too...
KDEWIN32_EXPORT int	mkdir( const char *_path, mode_t __mode );
#endif
*/
#endif /* _SYS_STAT_H */
