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

#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include <sys/time.h>

//#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>

#if 0
#ifdef __cplusplus
/*extern "C" {*/
#endif

/* SUS symbolic values for the second parm to shutdown(2) */
#define SHUT_RD   0		/* == Win32 SD_RECEIVE */
#define SHUT_WR   1		/* == Win32 SD_SEND    */
#define SHUT_RDWR 2		/* == Win32 SD_BOTH    */

  int accept (int, struct sockaddr *__peer, int *);
  int bind (int, const struct sockaddr *__my_addr, int __addrlen);
  int connect (int, const struct sockaddr *, int);
  int getpeername (int, struct sockaddr *__peer, int *);
  int getsockname (int, struct sockaddr *__addr, int *);
  int listen (int, int __n);
  int recv (int, void *__buff, int __len, unsigned int __flags);
  int recvfrom (int, char *__buff, int __len, int __flags,
                struct sockaddr *__from, int *__fromlen);
  int recvmsg(int s, struct msghdr *msg, int flags);
  int send (int, const void *__buff, int __len, unsigned int __flags);
  int sendmsg(int s, const struct msghdr *msg, int flags);
  int sendto (int, const void *, int, unsigned int, const struct sockaddr *, int);
  int setsockopt (int __s, int __level, int __optname, const void *optval, int __optlen);
  int getsockopt (int __s, int __level, int __optname, void *__optval, int *__optlen);
  int shutdown (int, int);
/* defined in winsock:  int socket (int __family, int __type, int __protocol);*/
  int socketpair (int __domain, int __type, int __protocol, int *__socket_vec);

/*defined in winsock:  struct servent *getservbyname (const char *__name, const char *__proto); */

//#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>

#if 0
#ifdef __cplusplus
/*};*/
#endif

#endif

#endif

#endif /* _SYS_SOCKET_H */
