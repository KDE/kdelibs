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

#ifndef _KDEWIN_UNISTD_H
#define _KDEWIN_UNISTD_H

#include <winposix_export.h>

#include <io.h> /* access(), etc.*/
#include <process.h> /* getpid(), etc.*/

/* include most headers here to avoid redefining gethostname() */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#define NOGDI
#include <winsock2.h>
#include "fixwinh.h"

#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	F_OK	0
#define	R_OK	4
#define	W_OK	2
#define	X_OK	1 

/* + from <sys/stat.h>: */
#define	_IFMT		0170000	/* type of file */
#define		_IFDIR	0040000	/* directory */
#define		_IFCHR	0020000	/* character special */
#define		_IFBLK	0060000	/* block special */
#define		_IFREG	0100000	/* regular */
#define		_IFLNK	0120000	/* symbolic link */
#define		_IFSOCK	0140000	/* socket */
#define		_IFIFO	0010000	/* fifo */

#define	S_ISBLK(m)	(((m)&_IFMT) == _IFBLK)
#define	S_ISCHR(m)	(((m)&_IFMT) == _IFCHR)
#define	S_ISDIR(m)	(((m)&_IFMT) == _IFDIR)
#define	S_ISFIFO(m)	(((m)&_IFMT) == _IFIFO)
#define	S_ISREG(m)	(((m)&_IFMT) == _IFREG)
#define	S_ISLNK(m)	(((m)&_IFMT) == _IFLNK)
#define	S_ISSOCK(m)	(((m)&_IFMT) == _IFSOCK)


KDEWIN32_EXPORT int chown(const char *__path, uid_t __owner, gid_t __group);

KDEWIN32_EXPORT int fchmod(int __fd, mode_t __mode);

KDEWIN32_EXPORT int fchown(int __fd, uid_t __owner, gid_t __group );

/* Get the real user ID of the calling process.  */
KDEWIN32_EXPORT uid_t getuid (void);

/* Get the effective user ID of the calling process.  */
KDEWIN32_EXPORT uid_t geteuid (void);

/* Get the real group ID of the calling process.  */
KDEWIN32_EXPORT gid_t getgid (void);

/* Get the effective group ID of the calling process.  */
KDEWIN32_EXPORT gid_t getegid (void);

KDEWIN32_EXPORT int getgroups(int size, gid_t list[]);

/* On win32 we do not have fs-links, so simply 0 (success) is returned
   when __path is accessible. It is then just copied to __buf.
*/
KDEWIN32_EXPORT int readlink(const char *__path, char *__buf, int __buflen);

/* just copies __name1 to __name2 */
KDEWIN32_EXPORT int symlink(const char *__name1, const char *__name2);
/* just copies __name1 to __name2 */
KDEWIN32_EXPORT int link(const char *__name1, const char *__name2);

KDEWIN32_EXPORT char* realpath(const char *path,char *resolved_path);

KDEWIN32_EXPORT int pipe(int *fd);

KDEWIN32_EXPORT pid_t fork(void);

KDEWIN32_EXPORT pid_t setsid(void);

#undef gethostname
#define gethostname kde_gethostname

KDEWIN32_EXPORT int kde_gethostname(char *__name, size_t __len);

KDEWIN32_EXPORT unsigned alarm(unsigned __secs ); 

KDEWIN32_EXPORT char* getlogin();

KDEWIN32_EXPORT int fsync (int fd);

KDEWIN32_EXPORT void usleep(useconds_t useconds);

KDEWIN32_EXPORT void sleep(unsigned int sec);
		
KDEWIN32_EXPORT long int random();

KDEWIN32_EXPORT int setreuid(uid_t ruid, uid_t euid);

KDEWIN32_EXPORT int mkstemps(char* _template, int suffix_len);

// from kdecore/fakes.c

KDEWIN32_EXPORT int seteuid(uid_t euid);

KDEWIN32_EXPORT int mkstemp (char* _template);

KDEWIN32_EXPORT char* mkdtemp (char* _template);

#ifdef __cplusplus
}
#endif

#endif /* _KDEWIN_UNISTD_H */
