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
#include <ksockaddr.h>
#include <sys/types.h>
#include <kstaticdeleter.h>

class KSocksTable;
class KSocksPrivate;
class KLibrary;
class KConfigBase;
struct sockaddr;


class KSocks {
   friend class KStaticDeleter<KSocks>;

public:

   /**
    * Return an instance of class KSocks *
    * You cannot delete this object.  It is a singleton class.
    */
   static KSocks *self();

   /**
    * Return true if KSocks has been started (ie someone called self())
    */
   static bool activated();

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
    int     connect     (int sockfd, const sockaddr *serv_addr, 
                                ksocklen_t addrlen);
    signed long int read  (int fd, void *buf, unsigned long int count);
    signed long int write (int fd, const void *buf, unsigned long int count);
    int     recvfrom    (int s, void *buf, unsigned long int len, int flags, 
                                sockaddr *from, ksocklen_t *fromlen);
    int     sendto      (int s, const void *msg, unsigned long int len, int flags,
                             const sockaddr *to, ksocklen_t tolen);
    int     recv        (int s, void *buf, unsigned long int len, int flags);
    int     send        (int s, const void *msg, unsigned long int len, int flags);
    int     getsockname (int s, sockaddr *name, ksocklen_t *namelen);
    int     getpeername (int s, sockaddr *name, ksocklen_t *namelen);
    int     accept      (int s, sockaddr *addr, ksocklen_t *addrlen);
    int     select      (int n, fd_set *readfds, fd_set *writefds, 
                                fd_set *exceptfds, struct timeval *timeout);
    int     listen      (int s, int backlog);
    int     bind        (int sockfd, sockaddr *my_addr, 
                                     ksocklen_t addrlen);

   /*
    *  If you're using this, you're probably doing something wrong.
    *  Please don't use it.
    */
   void die();

   /*
    *  Set this before the first call to KSocks::self() and it will fail
    *  to initialize SOCKS.
    */
   static void disable();

   /*
    *  Set this before the first call to KSocks::self() and it will use
    *  @p config to read its configuration from.
    */
   static void setConfig(KConfigBase *config);

private:
   KSocks(KConfigBase *config);
   ~KSocks();

   void stopSocks();

   static KSocks *_me;
   static bool _disabled;
   QStringList _libNames;
   QStringList _libPaths;
   bool _useSocks, _hasSocks;
   KLibrary* _socksLib;


   KSocksTable *_st;
   KSocksPrivate *d;
};




#endif
