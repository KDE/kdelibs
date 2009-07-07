/* This file is part of the KDE libraries
   Copyright (C) 2001-2003 George Staikos <staikos@kde.org>

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

#include <config.h>
#define I_KNOW_KSOCKS_ISNT_PUBLIC
#include "k3socks.h" //krazy:exclude=includes (not public: you should know you need kde_socklen_t if you use this)
#undef I_KNOW_KSOCKS_ISNT_PUBLIC

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <QtCore/QFile>
#include <QtCore/QCharRef>
#include <QtCore/QMap>

#include <klocale.h>
#include <kdebug.h>
#include "klibloader.h"
#include <kconfig.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>

#include <kconfiggroup.h>

// DO NOT RE-ORDER THESE.
enum SymbolKeys {
      S_SOCKSinit    =  0,
      S_connect      =  1,
      S_read         =  2,
      S_write        =  3,
      S_recvfrom     =  4,
      S_sendto       =  5,
      S_recv         =  6,
      S_send         =  7,
      S_getsockname  =  8,
      S_getpeername  =  9,
      S_accept       = 10,
      S_select       = 11,
      S_listen       = 12,
      S_bind         = 13
     };


extern "C" {
// Function pointer table
static int     (*F_SOCKSinit)   (char *) = 0L;
static int     (*F_connect)     (int, const struct sockaddr *, kde_socklen_t) = 0L;
static signed long int (*F_read)        (int, void *, unsigned long int) = 0L;
static signed long int (*F_write)       (int, const void *, unsigned long int) = 0L;
static int     (*F_recvfrom)    (int, void *, unsigned long int, int, struct sockaddr *,
                                 kde_socklen_t *) = 0L;
static int     (*F_sendto)      (int, const void *, unsigned long int, int,
                                 const struct sockaddr *, kde_socklen_t) = 0L;
static int     (*F_recv)        (int, void *, unsigned long int, int) = 0L;
static int     (*F_send)        (int, const void *, unsigned long int, int) = 0L;
static int     (*F_getsockname) (int, struct sockaddr *, kde_socklen_t *) = 0L;
static int     (*F_getpeername) (int, struct sockaddr *, kde_socklen_t *) = 0L;
static int     (*F_accept)      (int, struct sockaddr *, kde_socklen_t *) = 0L;
static int     (*F_select)      (int, fd_set *, fd_set *, fd_set *,
                                                     struct timeval *) = 0L;
static int     (*F_listen)      (int, int) = 0L;
static int     (*F_bind)        (int, const struct sockaddr *, kde_socklen_t) = 0L;
}


class KSocksTable {
 public:
   KSocksTable();
  virtual ~KSocksTable();

   // The name of each symbol and it's SOCKS replacement
   QMap<SymbolKeys,QString>  symbols;
   // The name of this library
   QString                   myname;
   bool                      hasWorkingAsyncConnect;
};


KSocksTable::KSocksTable() : myname("Unknown"), hasWorkingAsyncConnect(true) {
}

KSocksTable::~KSocksTable() {
}


/*
 *   How to add support for a new SOCKS package.
 *
 *   1) Subclass KSocksTable as is done below and write out all the symbols
 *   1.b) Give the class a "myname"
 *   2) Make sure that all possible library names are written into the
 *      _libNames string list.  Don't forget that different OSes name shared
 *      libraries differently.  Expect .so, .sl, .a (!) (AIX does this).
 *   3) Find a unique symbol in the library that we can use to identify that
 *      library and write out the test case in the constructor
 *   4) Make necessary changes to the KControl module in kdebase/kcontrol/....
 *   5) TEST!
 *
 */

//////////////////////////////////////////////////////////////////
///////        Define each library symbol table here       ///////
//////////////////////////////////////////////////////////////////


//
//    Support for NEC SOCKS client
//

class KNECSocksTable : public KSocksTable {
  public:
    KNECSocksTable();
    virtual ~KNECSocksTable();
};


KNECSocksTable::KNECSocksTable() : KSocksTable() {
  myname = i18n("NEC SOCKS client");
  symbols.insert(S_SOCKSinit,   "SOCKSinit");
  symbols.insert(S_connect,     "connect");
  symbols.insert(S_read,        "read");
  symbols.insert(S_write,       "write");
  symbols.insert(S_recvfrom,    "recvfrom");
  symbols.insert(S_sendto,      "sendto");
  symbols.insert(S_recv,        "recv");
  symbols.insert(S_send,        "send");
  symbols.insert(S_getsockname, "getsockname");
  symbols.insert(S_getpeername, "getpeername");
  symbols.insert(S_accept,      "accept");
  symbols.insert(S_select,      "select");
  symbols.insert(S_listen,      "listen");
  symbols.insert(S_bind,        "bind");
}

KNECSocksTable::~KNECSocksTable() {
}




//
//    Support for Dante SOCKS client
//

class KDanteSocksTable : public KSocksTable {
  public:
    KDanteSocksTable();
    virtual ~KDanteSocksTable();
};

KDanteSocksTable::KDanteSocksTable() : KSocksTable() {
  hasWorkingAsyncConnect = false;
  myname = i18n("Dante SOCKS client");
  symbols.insert(S_SOCKSinit,   "SOCKSinit");
  symbols.insert(S_connect,     "Rconnect");
  symbols.insert(S_read,        "Rread");
  symbols.insert(S_write,       "Rwrite");
  symbols.insert(S_recvfrom,    "Rrecvfrom");
  symbols.insert(S_sendto,      "Rsendto");
  symbols.insert(S_recv,        "Rrecv");
  symbols.insert(S_send,        "Rsend");
  symbols.insert(S_getsockname, "Rgetsockname");
  symbols.insert(S_getpeername, "Rgetpeername");
  symbols.insert(S_accept,      "Raccept");
  symbols.insert(S_select,      "Rselect");
  symbols.insert(S_listen,      "Rlisten");
  symbols.insert(S_bind,        "Rbind");
}


KDanteSocksTable::~KDanteSocksTable() {
}



//////////////////////////////////////////////////////////////////
///////        End of all symbol table definitions         ///////
//////////////////////////////////////////////////////////////////

class KSocks::KSocksPrivate
{
public:
    KSocksPrivate() :
        _useSocks(false),
        _hasSocks(false),
        _socksLib(0L),
        _st(0L)
    {}

    static int debugArea() { static int s_area = KDebug::registerArea("kdecore (KSocks)"); return s_area; }

    QStringList _libNames;
    QStringList _libPaths;
    bool _useSocks;
    bool _hasSocks;
    KLibrary* _socksLib;
    KSocksTable *_st;
};

KSocks *KSocks::_me = 0;
#ifdef __CYGWIN__
bool KSocks::_disabled = true;
#else
bool KSocks::_disabled = false;
#endif

void KSocks::disable()
{
   if (!_me)
      _disabled = true;
}

KSocks *KSocks::self() {
  // Note that we don't use a static deleter here. It makes no sense and tends to cause crashes.
  if (!_me) {
     if (KGlobal::hasMainComponent()) {
        KConfigGroup cfg(KGlobal::config(), "Socks");
        _me = new KSocks(&cfg);
     } else {
        _disabled = true;
        _me = new KSocks(0);
     }
  }
  return _me;
}

void KSocks::setConfig(const KConfigGroup *config)
{
  // We can change the config from disabled to enabled
  // but not the other way around.
  if (_me && _disabled) {
     delete _me;
     _me = 0;
     _disabled = false;
  }
  if (!_me)
    _me = new KSocks(config);
}

bool KSocks::activated() { return (_me != 0L); }

// Function for the KControl module to test if the socks support works.
KDECORE_EXPORT bool kdeHasSocks() { return KSocks::self()->hasSocks(); }

KSocks::KSocks(const KConfigGroup *config)
  : d(new KSocksPrivate())
{
   if (!config)
      return;

   if (!config->readEntry("SOCKS_enable", false)) {
      _disabled = true;
   }

   if (_disabled)
      return;

   d->_libPaths << ""
                << "/usr/lib" KDELIBSUFF "/"
                << "/usr/lib/"
                << "/usr/local/lib" KDELIBSUFF "/"
                << "/usr/local/lib/"
                << "/usr/local/socks5/lib" KDELIBSUFF "/"
                << "/usr/local/socks5/lib/"
                << "/opt/socks5/lib" KDELIBSUFF "/"
                << "/opt/socks5/lib/";
   d->_libNames << "libsocks.so"                  // Dante
                << "libdsocksd.so.0"              // Dante 1.1.14-2 on
                                                  // Debian unstable 17-12-2003
                << "libsocks5.so"                 // ?
                << "libsocks5_sh.so";             // NEC

   // Add the custom library paths here
   QStringList newlibs = config->readEntry("SOCKS_lib_path", QStringList());

   for (QStringList::Iterator it = newlibs.begin();
                              it != newlibs.end();
                              ++it) {
      QString thisone = *it;
      if (thisone[thisone.length()-1] != '/') thisone += '/';
      d->_libPaths << thisone;
      kDebug(d->debugArea()) << "KSocks added a new library path: " << thisone;
   }

   // Load the proper libsocks and KSocksTable
   KLibLoader *ll = KLibLoader::self();


   int _meth = config->readEntry("SOCKS_method", 1);
         /****       Current methods
          *   1) Autodetect (read: any)     2) NEC
          *   3) Dante                      4) Custom
          */

   if (_meth == 4) {         // try to load^H^H^H^Hguess at a custom library
      d->_socksLib = ll->library(config->readPathEntry("SOCKS_lib", QString()));
      if (d->_socksLib && d->_socksLib->resolveFunction("Rconnect")) {  // Dante compatible?
         d->_st = new KDanteSocksTable;
         d->_useSocks = true;
         d->_hasSocks = true;
      } else if (d->_socksLib && d->_socksLib->resolveFunction("connect")) { // NEC compatible?
         d->_st = new KNECSocksTable;
         d->_useSocks = true;
         d->_hasSocks = true;
      } else if (d->_socksLib) {
         d->_socksLib->unload();
         d->_socksLib = 0L;
      }
   } else              // leave this here   "else for {}"
   for (QStringList::Iterator pit  = d->_libPaths.begin();
                              !d->_hasSocks && pit != d->_libPaths.end();
                              ++pit)
   for (QStringList::Iterator it  = d->_libNames.begin();
                              it != d->_libNames.end();
                              ++it) {
      d->_socksLib = ll->library(*pit + *it);
      if (d->_socksLib) {
         if ((_meth == 1 || _meth == 2) &&
            d-> _socksLib->resolveFunction("S5LogShowThreadIDS") != 0L) {  // NEC SOCKS
            kDebug(d->debugArea()) << "Found NEC SOCKS";
            d->_st = new KNECSocksTable;
            d->_useSocks = true;
            d->_hasSocks = true;
            break;
         } else if ((_meth == 1 || _meth == 3) &&
                    d->_socksLib->resolveFunction("sockaddr2ruleaddress") != 0L) { //Dante
            kDebug(d->debugArea()) << "Found Dante SOCKS";
            d->_st = new KDanteSocksTable;
            d->_useSocks = true;
            d->_hasSocks = true;
            break;
         } else {
           d->_socksLib->unload();
           d->_socksLib = 0L;
         }
      }
   }

   // Load in all the symbols
   if (d->_st) {
      for (QMap<SymbolKeys,QString>::Iterator it  = d->_st->symbols.begin();
                                              it != d->_st->symbols.end();
                                              ++it) {
         switch(it.key()) {
         case S_SOCKSinit:
           F_SOCKSinit = (int (*)(char *))
                         d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_connect:
           F_connect = (int (*)(int, const struct sockaddr *, kde_socklen_t))
                       d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_read:
           F_read = (signed long int (*)(int, void *, unsigned long int))
                    d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_write:
           F_write = (signed long int (*)(int, const void *, unsigned long int))
                     d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_recvfrom:
           F_recvfrom = (int (*)(int, void *, unsigned long int, int,
                                 struct sockaddr *, kde_socklen_t *))
                        d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_sendto:
           F_sendto = (int (*)(int, const void *, unsigned long int, int,
                               const struct sockaddr *, kde_socklen_t))
                      d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_recv:
           F_recv = (int (*)(int, void *, unsigned long int, int))
                    d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_send:
           F_send = (int (*)(int, const void *, unsigned long int, int))
                    d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_getsockname:
           F_getsockname = (int (*)(int, struct sockaddr *, kde_socklen_t *))
                           d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_getpeername:
           F_getpeername = (int (*)(int, struct sockaddr *, kde_socklen_t *))
                           d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_accept:
           F_accept = (int (*)(int, struct sockaddr *, kde_socklen_t *))
                      d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_select:
           F_select = (int (*)(int, fd_set *, fd_set *, fd_set *, struct timeval *))
                      d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_listen:
           F_listen = (int (*)(int, int))
                      d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         case S_bind:
           F_bind = (int (*)(int, const struct sockaddr *, kde_socklen_t))
                    d->_socksLib->resolveFunction(it.value().toLatin1());
          break;
         default:
          kDebug(d->debugArea()) << "KSocks got a symbol it doesn't know about!";
          break;
         }
      }

      // Now we check for the critical stuff.
      if (F_SOCKSinit) {
         int rc = (*F_SOCKSinit)((char *)"KDE");
         if (rc != 0)
            stopSocks();
         else kDebug(d->debugArea()) << "SOCKS has been activated!";
      } else {
         stopSocks();
      }
   }
}


KSocks::~KSocks() {
   stopSocks();
   _me = 0;
   delete d;
}

void KSocks::die() {
   if (_me == this) {
      _me = 0;
      delete this;
   }
}

void KSocks::stopSocks() {
   if (d->_hasSocks) {
      // This library doesn't even provide the basics.
      // It's probably broken.  Let's abort.
      d->_useSocks = false;
      d->_hasSocks = false;
      if (d->_socksLib) {
         d->_socksLib->unload();
         d->_socksLib = 0L;
      }
      delete d->_st;
      d->_st = 0L;
   }
}


bool KSocks::usingSocks() {
   return d->_useSocks;
}


bool KSocks::hasSocks() {
   return d->_hasSocks;
}


void KSocks::disableSocks() {
   d->_useSocks = false;
}


void KSocks::enableSocks() {
   if (d->_hasSocks)
      d->_useSocks = true;
}

bool KSocks::hasWorkingAsyncConnect()
{
   return (d->_useSocks && d->_st) ? d->_st->hasWorkingAsyncConnect : true;
}


/*
 *   REIMPLEMENTED FUNCTIONS FROM LIBC
 *
 */

int KSocks::connect (int sockfd, const sockaddr *serv_addr,
                                                   kde_socklen_t addrlen) {
   if (d->_useSocks && F_connect)
      return (*F_connect)(sockfd, serv_addr, addrlen);
   else return ::connect(sockfd, (sockaddr*) serv_addr, (socklen_t)addrlen);
}


signed long int KSocks::read (int fd, void *buf, unsigned long int count) {
   if (d->_useSocks && F_read)
      return (*F_read)(fd, buf, count);
   else return ::read(fd, buf, count);
}


signed long int KSocks::write (int fd, const void *buf, unsigned long int count) {
   if (d->_useSocks && F_write)
      return (*F_write)(fd, buf, count);
   else return ::write(fd, buf, count);
}


int KSocks::recvfrom (int s, void *buf, unsigned long int len, int flags,
                                sockaddr *from, kde_socklen_t *fromlen) {
   if (d->_useSocks && F_recvfrom) {
      return (*F_recvfrom)(s, buf, len, flags, from, fromlen);
   } else {
      socklen_t casted_len = (socklen_t) *fromlen;
      int rc = ::recvfrom(s, (char*) buf, len, flags, from, &casted_len);
      *fromlen = casted_len;
      return rc;
   }
}


int KSocks::sendto (int s, const void *msg, unsigned long int len, int flags,
                             const sockaddr *to, kde_socklen_t tolen) {
   if (d->_useSocks && F_sendto)
      return (*F_sendto)(s, msg, len, flags, to, tolen);
   else return ::sendto(s, (char*) msg, len, flags, to, (socklen_t)tolen);
}


int KSocks::recv (int s, void *buf, unsigned long int len, int flags) {
   if (d->_useSocks && F_recv)
      return (*F_recv)(s, buf, len, flags);
   else return ::recv(s, (char*) buf, len, flags);
}


int KSocks::send (int s, const void *msg, unsigned long int len, int flags) {
   if (d->_useSocks && F_send)
      return (*F_send)(s, msg, len, flags);
   else return ::send(s, (char*) msg, len, flags);
}


int KSocks::getsockname (int s, sockaddr *name, kde_socklen_t *namelen) {
   if (d->_useSocks && F_getsockname) {
      return (*F_getsockname)(s, name, namelen);
   } else {
     socklen_t casted_len = *namelen;
     int rc = ::getsockname(s, name, &casted_len);
     *namelen = casted_len;
     return rc;
   }
}


int KSocks::getpeername (int s, sockaddr *name, kde_socklen_t *namelen) {
   if (d->_useSocks && F_getpeername) {
      return (*F_getpeername)(s, name, namelen);
   } else {
      socklen_t casted_len = *namelen;
      int rc = ::getpeername(s, name, &casted_len);
      *namelen = casted_len;
      return rc;
   }
}


int KSocks::accept (int s, sockaddr *addr, kde_socklen_t *addrlen) {
   if (d->_useSocks && F_accept) {
     return (*F_accept)(s, addr, addrlen);
   } else {
      socklen_t casted_len = *addrlen;
      int rc = ::accept(s, addr, &casted_len);
      *addrlen = casted_len;
      return rc;
   }
}


int KSocks::select (int n, fd_set *readfds, fd_set *writefds,
                                fd_set *exceptfds, struct timeval *timeout) {
   if (d->_useSocks && F_select)
      return (*F_select)(n, readfds, writefds, exceptfds, timeout);
   else return ::select(n, readfds, writefds, exceptfds, timeout);
}


int KSocks::listen (int s, int backlog) {
   if (d->_useSocks && F_listen)
      return (*F_listen)(s, backlog);
   else return ::listen(s, backlog);
}


int KSocks::bind (int sockfd, const sockaddr *my_addr, kde_socklen_t addrlen) {
   if (d->_useSocks && F_bind)
      return (*F_bind)(sockfd, my_addr, addrlen);
   else return ::bind(sockfd, my_addr, (socklen_t)addrlen);
}

int KSocks::bind (int sockfd, sockaddr *my_addr, kde_socklen_t addrlen) {
   if (d->_useSocks && F_bind)
      return (*F_bind)(sockfd, my_addr, addrlen);
   else return ::bind(sockfd, my_addr, (socklen_t)addrlen);
}



