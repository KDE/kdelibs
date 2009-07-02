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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _KSOCKS_H_
#define _KSOCKS_H_

#if defined(I_KNOW_KSOCKS_ISNT_PUBLIC)
/* "ksocks.h is not public API" */

#include <kdecore_export.h>

#include <QtCore/QStringList>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef Q_OS_UNIX

class KConfigGroup;
struct sockaddr;


/**
 * This class provides you with an interface to a 
 * <a href="http://www.socks.nec.com/">SOCKS</a> Proxy server. A SOCKS server
 * is able to provide full internet access behind a firewall.
 * KSocks is a singleton; there can only be one instance at any
 * given time. To obtain a reference to that instance, use
 * self().
 *
 * @short Access to a SOCKS Proxy.
 * @deprecated Use KSocketFactory or KLocalSocket instead
 */
class KDECORE_EXPORT KSocks {
public:

   /**
    * Return an instance of class KSocks *.
    * You cannot delete this object.  It is a singleton class.
    * @return the KSock instance
    */
   static KSocks *self();

   /**
    * Checks whether KSocks has been started (ie someone called self())
    * @return true if activated
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
    * Checks whether SOCKS is currently being used.
    * @return true if SOCKS is currently being used.
    */
   bool usingSocks();

   /**
    * Checks whether SOCKS is available for use.
    * @return true if SOCKS is available for use.
    */
   bool hasSocks();

   /**
    * Returns whether asynchronous connects work with the
    * selected SOCKS impementation
    */
   bool hasWorkingAsyncConnect();

   /*
    **     REIMPLEMENTATIONS OF LIBC SOCKET FUNCTIONS
    **/
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     connect     (int sockfd, const sockaddr *serv_addr, 
                                kde_socklen_t addrlen);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    signed long int read  (int fd, void *buf, unsigned long int count);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    signed long int write (int fd, const void *buf, unsigned long int count);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     recvfrom    (int s, void *buf, unsigned long int len, int flags, 
                                sockaddr *from, kde_socklen_t *fromlen);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     sendto      (int s, const void *msg, unsigned long int len, int flags,
                             const sockaddr *to, kde_socklen_t tolen);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     recv        (int s, void *buf, unsigned long int len, int flags);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     send        (int s, const void *msg, unsigned long int len, int flags);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     getsockname (int s, sockaddr *name, kde_socklen_t *namelen);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     getpeername (int s, sockaddr *name, kde_socklen_t *namelen);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     accept      (int s, sockaddr *addr, kde_socklen_t *addrlen);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     select      (int n, fd_set *readfds, fd_set *writefds, 
                                fd_set *exceptfds, struct timeval *timeout);
   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     listen      (int s, int backlog);

   /**
    * This is the re-implementation of libc's function of the same
    * name. Read the appropriate man page.
    */
    int     bind        (int sockfd, sockaddr *my_addr, 
                                     kde_socklen_t addrlen);
    int     bind        (int sockfd, const sockaddr *my_addr, 
                                     kde_socklen_t addrlen);

    /**
    *  If you're using this, you're probably doing something wrong.
    *  Please don't use it.
    * @internal
    */
   void die();

   /**
    *  Set this before the first call to KSocks::self() and it will fail
    *  to initialize SOCKS.
    */
   static void disable();

   /**
    *  Set this before the first call to KSocks::self() and it will use
    *  @p config to read its configuration from.
    */
   static void setConfig(const KConfigGroup *config);

private:
   explicit KSocks(const KConfigGroup *config);
   ~KSocks();

   void stopSocks();

   static KSocks *_me;
   static bool _disabled;


   class KSocksPrivate;
   KSocksPrivate * const d;
};

#endif //Q_OS_UNIX

#endif
#endif //_KSOCKS_H_
