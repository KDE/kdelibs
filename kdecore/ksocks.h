/* This file is part of the KDE libraries
   Copyright (C) 2001 George Staikos <staikos@kde.org>
 
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
#ifndef _KSOCKS_H_
#define _KSOCKS_H_

#include <qstringlist.h>
#include <sys/types.h>
#include <sys/socket.h>

class KSocksTable;
class KSocksPrivate;
class KLibrary;




class KSocks {
public:

   /**
    * Return an instance of class KSocks *
    * You cannot delete this object.  It is a singleton class.
    */
   static KSocks *self();

   /**
    * Disable the use of SOCKS immediately
    */
   void disableSocks();

   /**
    * Enable the use of SOCKS immediately if hasSocks() is true.
    */
   void enableSocks();

   /**
    * True if SOCKS is currently being used.
    */
   bool usingSocks();

   /**
    * True if SOCKS is available for use.
    */
   bool hasSocks();

   /**
    **     REIMPLEMENTATIONS OF LIBC SOCKET FUNCTIONS
    **/
    int     connect     (int sockfd, const struct sockaddr *serv_addr, 
                                socklen_t addrlen);
    ssize_t read        (int fd, void *buf, ssize_t count);
    ssize_t write       (int fd, const void *buf, size_t count);
    int     recvfrom    (int s, void *buf, size_t len, int flags, 
                                struct sockaddr *from, socklen_t *fromlen);
    int     sendto      (int s, const void *msg, size_t len, int flags,
                             const struct sockaddr *to, socklen_t tolen);
    int     recv        (int s, void *buf, size_t len, int flags);
    int     send        (int s, const void *msg, size_t len, int flags);
    int     getsockname (int s, struct sockaddr *name, socklen_t *namelen);
    int     getpeername (int s, struct sockaddr *name, socklen_t *namelen);
    int     accept      (int s, struct sockaddr *addr, socklen_t *addrlen);
    int     select      (int n, fd_set *readfds, fd_set *writefds, 
                                fd_set *exceptfds, struct timeval *timeout);
    int     listen      (int s, int backlog);
    int     bind        (int sockfd, struct sockaddr *my_addr, 
                                     socklen_t addrlen);



private:
   KSocks();
   ~KSocks();

   void stopSocks();

   static KSocks *_me;
   QStringList _libNames;
   bool _useSocks, _hasSocks;
   KLibrary* _socksLib;


   KSocksTable *_st;
   KSocksPrivate *d;
};




#endif
