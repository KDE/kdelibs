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

#include <config.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "ksocks.h"
#include <qstring.h>
#include <qmap.h>
#include <klocale.h>
#include <kdebug.h>
#include "klibloader.h"
#include "kstaticdeleter.h"
#include <kconfig.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>


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
static int     (*F_SOCKSinit)   (char *) = NULL;
static int     (*F_connect)     (int, const struct sockaddr *, ksocklen_t) = NULL;
static signed long int (*F_read)        (int, void *, unsigned long int) = NULL;
static signed long int (*F_write)       (int, const void *, unsigned long int) = NULL;
static int     (*F_recvfrom)    (int, void *, unsigned long int, int, struct sockaddr *, 
                                 ksocklen_t *) = NULL;
static int     (*F_sendto)      (int, const void *, unsigned long int, int,
                                 const struct sockaddr *, ksocklen_t) = NULL;
static int     (*F_recv)        (int, void *, unsigned long int, int) = NULL;
static int     (*F_send)        (int, const void *, unsigned long int, int) = NULL;
static int     (*F_getsockname) (int, struct sockaddr *, ksocklen_t *) = NULL;
static int     (*F_getpeername) (int, struct sockaddr *, ksocklen_t *) = NULL;
static int     (*F_accept)      (int, struct sockaddr *, ksocklen_t *) = NULL;
static int     (*F_select)      (int, fd_set *, fd_set *, fd_set *, 
                                 struct timeval *) = NULL;
static int     (*F_listen)      (int, int) = NULL;
static int     (*F_bind)        (int, struct sockaddr *, ksocklen_t) = NULL;
};


class KSocksTable {
 public:
   KSocksTable();
 
   // The name of each symbol and it's SOCKS replacement
   QMap<SymbolKeys,QString>  symbols;
   // The name of this library
   QString                   myname;
};


KSocksTable::KSocksTable() : myname("Unknown") {
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


KSocks *KSocks::_me = 0;
bool KSocks::_disabled = false;
static KStaticDeleter<KSocks> med;

void KSocks::disable() { _disabled = true; }

KSocks *KSocks::self() {
  if (!_me) {
    _me = med.setObject(new KSocks);
  }
  return _me;
}


bool KSocks::activated() { return (_me != NULL); }


KSocks::KSocks() : _socksLib(NULL), _st(NULL), cfg(0) {

   if (_disabled) {
      _hasSocks = _useSocks = false;
      return;
   }

   _libPaths << ""
             << "/usr/lib/"
             << "/usr/local/lib/"
             << "/usr/local/socks5/lib/"
             << "/opt/socks5/lib/";
   _libNames << "libsocks.so"                  // Dante
             << "libsocks5.so"                 // ?
             << "libsocks5_sh.so";             // NEC

   cfg = new KConfig("ksocksrc", true, false);

   // Add the custom library paths here
   QStringList newlibs = cfg->readListEntry("Lib Path");

   for (QStringList::Iterator it = newlibs.begin();
                              it != newlibs.end();
                              ++it) {
      QString thisone = *it;
      if (thisone[thisone.length()-1] != '/') thisone += "/";
      _libPaths << thisone;
      kdDebug(171) << "KSocks added a new library path: " << thisone << endl;
   }

   // Load the proper libsocks and KSocksTable
   KLibLoader *ll = KLibLoader::self();
   
   _hasSocks = false;
   _useSocks = false;

   if (!(cfg->readBoolEntry("Enable SOCKS", false))) return;

   int _meth = cfg->readNumEntry("SOCKS Method", 1);
         /****       Current methods
          *   1) Autodetect (read: any)     2) NEC
          *   3) Dante                      4) Custom
          */

   if (_meth == 4) {         // try to load^H^H^H^Hguess at a custom library
      _socksLib = ll->library(cfg->readEntry("Custom Lib", "").latin1());
      if (_socksLib && _socksLib->symbol("Rconnect")) {  // Dante compatible?
         _st = new KDanteSocksTable;       
         _useSocks = true;
         _hasSocks = true;
      } else if (_socksLib && _socksLib->symbol("connect")) { // NEC compatible?
         _st = new KNECSocksTable;       
         _useSocks = true;
         _hasSocks = true;
      } else if (_socksLib) {
         ll->unloadLibrary(_socksLib->name().latin1());
         _socksLib = NULL;
      }
   } else              // leave this here   "else for {}"
   for (QStringList::Iterator pit  = _libPaths.begin();
                              !_hasSocks && pit != _libPaths.end();
                              ++pit)
   for (QStringList::Iterator it  = _libNames.begin();
                              it != _libNames.end();
                              ++it) {
      _socksLib = ll->library((*pit + *it).latin1());
      if (_socksLib) {
         if ((_meth == 1 || _meth == 2) &&
             _socksLib->symbol("S5LogShowThreadIDS") != NULL) {  // NEC SOCKS
            kdDebug(171) << "Found NEC SOCKS" << endl;
            _st = new KNECSocksTable;
            _useSocks = true;
            _hasSocks = true;
            break;
         } else if ((_meth == 1 || _meth == 3) && 
                    _socksLib->symbol("sockaddr2ruleaddress") != NULL) { //Dante
            kdDebug(171) << "Found Dante SOCKS" << endl;
            _st = new KDanteSocksTable;
            _useSocks = true;
            _hasSocks = true;
            break;
         } else {
           ll->unloadLibrary(_socksLib->name().latin1());
           _socksLib = NULL;
         }
      }
   }

   // Load in all the symbols
   if (_st) {
      for (QMap<SymbolKeys,QString>::Iterator it  = _st->symbols.begin();
                                              it != _st->symbols.end();
                                              ++it) {
         switch(it.key()) {
         case S_SOCKSinit:
           F_SOCKSinit = (int (*)(char *))
                         _socksLib->symbol(it.data().latin1());
          break;
         case S_connect:
           F_connect = (int (*)(int, const struct sockaddr *, ksocklen_t))
                       _socksLib->symbol(it.data().latin1());
          break;
         case S_read:
           F_read = (signed long int (*)(int, void *, unsigned long int))
                    _socksLib->symbol(it.data().latin1());
          break;
         case S_write:
           F_write = (signed long int (*)(int, const void *, unsigned long int))
                     _socksLib->symbol(it.data().latin1());
          break;
         case S_recvfrom:
           F_recvfrom = (int (*)(int, void *, unsigned long int, int, 
                                 struct sockaddr *, ksocklen_t *))
                        _socksLib->symbol(it.data().latin1());
          break;
         case S_sendto:
           F_sendto = (int (*)(int, const void *, unsigned long int, int,
                               const struct sockaddr *, ksocklen_t))
                      _socksLib->symbol(it.data().latin1());
          break;
         case S_recv:
           F_recv = (int (*)(int, void *, unsigned long int, int))
                    _socksLib->symbol(it.data().latin1());
          break;
         case S_send:
           F_send = (int (*)(int, const void *, unsigned long int, int))
                    _socksLib->symbol(it.data().latin1());
          break;
         case S_getsockname:
           F_getsockname = (int (*)(int, struct sockaddr *, ksocklen_t *))
                           _socksLib->symbol(it.data().latin1());
          break;
         case S_getpeername:
           F_getpeername = (int (*)(int, struct sockaddr *, ksocklen_t *))
                           _socksLib->symbol(it.data().latin1());
          break;
         case S_accept:
           F_accept = (int (*)(int, struct sockaddr *, ksocklen_t *))
                      _socksLib->symbol(it.data().latin1());
          break;
         case S_select:
           F_select = (int (*)(int, fd_set *, fd_set *, fd_set *, struct timeval *))
                      _socksLib->symbol(it.data().latin1());
          break;
         case S_listen:
           F_listen = (int (*)(int, int))
                      _socksLib->symbol(it.data().latin1());
          break;
         case S_bind:
           F_bind = (int (*)(int, struct sockaddr *, ksocklen_t))
                    _socksLib->symbol(it.data().latin1());
          break;
         default:
          kdDebug(171) << "KSocks got a symbol it doesn't know about!" << endl;
          break;
         }
      }
 
      // Now we check for the critical stuff.
      if (F_SOCKSinit) {
        int rc = (*F_SOCKSinit)((char *)"KDE");
        if (rc != 0) stopSocks();
        else  kdDebug(171) << "SOCKS has been activated!" << endl;
      } else {
        stopSocks();
      }
   }
}


KSocks::~KSocks() {
  stopSocks();
  delete cfg;
  _me = med.setObject(0);
}


void KSocks::die() {
  if (_me == this) 
    delete this;
}

void KSocks::stopSocks() {
   if (_hasSocks) {
        // This library doesn't even provide the basics.
        // it's probably broken.  Lets abort.
          _useSocks = false;
          _hasSocks = false;
          if (_socksLib)
            KLibLoader::self()->unloadLibrary(_socksLib->name().latin1());
          _socksLib = NULL;
          delete _st;
          _st = NULL;
   }
}

 
bool KSocks::usingSocks() {
   return _useSocks;
}


bool KSocks::hasSocks() {
return _hasSocks;
}


void KSocks::disableSocks() {
  _useSocks = false;
}


void KSocks::enableSocks() {
  if (_hasSocks)
     _useSocks = true;
}



/*
 *   REIMPLEMENTED FUNCTIONS FROM LIBC
 *
 */

int KSocks::connect (int sockfd, const sockaddr *serv_addr,
                                                   ksocklen_t addrlen) {
  if (_useSocks && F_connect)
    return (*F_connect)(sockfd, serv_addr, addrlen);
  else return ::connect(sockfd, (sockaddr*) serv_addr, (socklen_t)addrlen);
}


signed long int KSocks::read (int fd, void *buf, unsigned long int count) {
  if (_useSocks && F_read)
    return (*F_read)(fd, buf, count);
  else return ::read(fd, buf, count);
}


signed long int KSocks::write (int fd, const void *buf, unsigned long int count) {
  if (_useSocks && F_write)
    return (*F_write)(fd, buf, count);
  else return ::write(fd, buf, count);
}


int KSocks::recvfrom (int s, void *buf, unsigned long int len, int flags,
                                sockaddr *from, ksocklen_t *fromlen) {
  if (_useSocks && F_recvfrom)
    return (*F_recvfrom)(s, buf, len, flags, from, fromlen);
  else {
    socklen_t casted_len = (socklen_t) *fromlen;
    int rc = ::recvfrom(s, (char*) buf, len, flags, from, &casted_len);
    *fromlen = casted_len;
    return rc;
  }
}


int KSocks::sendto (int s, const void *msg, unsigned long int len, int flags,
                             const sockaddr *to, ksocklen_t tolen) {
  if (_useSocks && F_sendto)
    return (*F_sendto)(s, msg, len, flags, to, tolen);
  else return ::sendto(s, (char*) msg, len, flags, to, (socklen_t)tolen);
}


int KSocks::recv (int s, void *buf, unsigned long int len, int flags) {
  if (_useSocks && F_recv)
    return (*F_recv)(s, buf, len, flags);
  else return ::recv(s, (char*) buf, len, flags);
}


int KSocks::send (int s, const void *msg, unsigned long int len, int flags) {
  if (_useSocks && F_send)
    return (*F_send)(s, msg, len, flags);
  else return ::send(s, (char*) msg, len, flags);
}


int KSocks::getsockname (int s, sockaddr *name, ksocklen_t *namelen) {
  if (_useSocks && F_getsockname)
    return (*F_getsockname)(s, name, namelen);
  else {
    socklen_t casted_len = *namelen;
    int rc = ::getsockname(s, name, &casted_len);
    *namelen = casted_len;
    return rc;
  }
}


int KSocks::getpeername (int s, sockaddr *name, ksocklen_t *namelen) {
  if (_useSocks && F_getpeername)
    return (*F_getpeername)(s, name, namelen);
  else {
    socklen_t casted_len = *namelen;
    int rc = ::getpeername(s, name, &casted_len);
    *namelen = casted_len;
    return rc;
  }
}


int KSocks::accept (int s, sockaddr *addr, ksocklen_t *addrlen) {
  if (_useSocks && F_accept)
    return (*F_accept)(s, addr, addrlen);
  else {
    socklen_t casted_len = *addrlen;
    int rc = ::accept(s, addr, &casted_len);
    *addrlen = casted_len;
    return rc;
  }
}


int KSocks::select (int n, fd_set *readfds, fd_set *writefds,
                                fd_set *exceptfds, struct timeval *timeout) {
  if (_useSocks && F_select)
    return (*F_select)(n, readfds, writefds, exceptfds, timeout);
  else return ::select(n, readfds, writefds, exceptfds, timeout);
}


int KSocks::listen (int s, int backlog) {
  if (_useSocks && F_listen)
    return (*F_listen)(s, backlog);
  else return ::listen(s, backlog);
}


int KSocks::bind (int sockfd, sockaddr *my_addr, ksocklen_t addrlen) {
  if (_useSocks && F_bind)
    return (*F_bind)(sockfd, my_addr, addrlen);
  else return ::bind(sockfd, my_addr, (socklen_t)addrlen);
}



