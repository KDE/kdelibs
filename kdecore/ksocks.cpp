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

#include "ksocks.h"
#include <qstring.h>
#include <qmap.h>
#include <klocale.h>
#include <kdebug.h>
#include "klibloader.h"


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
static int     (*F_SOCKSinit)   (char *);
static int     (*F_connect)     (int, const struct sockaddr *, socklen_t);
static ssize_t (*F_read)        (int, void *, ssize_t);
static ssize_t (*F_write)       (int, const void *, size_t);
static int     (*F_recvfrom)    (int, void *, size_t, int, struct sockaddr *, 
                                 socklen_t *);
static int     (*F_sendto)      (int, const void *, size_t, int,
                                 const struct sockaddr *, socklen_t);
static int     (*F_recv)        (int, void *, size_t, int);
static int     (*F_send)        (int, const void *, size_t, int);
static int     (*F_getsockname) (int, struct sockaddr *, socklen_t *);
static int     (*F_getpeername) (int, struct sockaddr *, socklen_t *);
static int     (*F_accept)      (int, struct sockaddr *, socklen_t *);
static int     (*F_select)      (int, fd_set *, fd_set *, fd_set *, 
                                 struct timeval *);
static int     (*F_listen)      (int, int);
static int     (*F_bind)        (int, struct sockaddr *, socklen_t);
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


KSocks *KSocks::_me = NULL;

KSocks *KSocks::self() {
  if (!_me) {
    _me = new KSocks;
  }
  return _me;
}


KSocks::KSocks() : _socksLib(NULL), _st(NULL) {
   _libNames << "libsocks.so"                  // Dante
             << "libsocks5.so"                 // ?
             << "libsocks5_sh.so";             // NEC

   // FIXME: read in kcmsocks config file and append those libs to _libNames.


   // Load the proper libsocks and KSocksTable
   KLibLoader *ll = KLibLoader::self();
   
   _hasSocks = false;
   _useSocks = false;

   for (QStringList::Iterator it  = _libNames.begin();
                              it != _libNames.end();
                              ++it) {
      _socksLib = ll->library((*it).latin1());
      if (_socksLib) {
         if (_socksLib->symbol("S5LogShowThreadIDS") != NULL) {     // NEC SOCKS
            kdDebug() << "Found NEC SOCKS" << endl;
            _st = new KNECSocksTable;
            _useSocks = true;
            _hasSocks = true;
            break;
         } else if (_socksLib->symbol("sockaddr2ruleaddress") != NULL) { //Dante
            kdDebug() << "Found Dante SOCKS" << endl;
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
           (void *)F_SOCKSinit = _socksLib->symbol(it.data().latin1());
          break;
         case S_connect:
           (void *)F_connect = _socksLib->symbol(it.data().latin1());
          break;
         case S_read:
           (void *)F_read = _socksLib->symbol(it.data().latin1());
          break;
         case S_write:
           (void *)F_write = _socksLib->symbol(it.data().latin1());
          break;
         case S_recvfrom:
           (void *)F_recvfrom = _socksLib->symbol(it.data().latin1());
          break;
         case S_sendto:
           (void *)F_sendto = _socksLib->symbol(it.data().latin1());
          break;
         case S_recv:
           (void *)F_recv = _socksLib->symbol(it.data().latin1());
          break;
         case S_send:
           (void *)F_send = _socksLib->symbol(it.data().latin1());
          break;
         case S_getsockname:
           (void *)F_getsockname = _socksLib->symbol(it.data().latin1());
          break;
         case S_getpeername:
           (void *)F_getpeername = _socksLib->symbol(it.data().latin1());
          break;
         case S_accept:
           (void *)F_accept = _socksLib->symbol(it.data().latin1());
          break;
         case S_select:
           (void *)F_select = _socksLib->symbol(it.data().latin1());
          break;
         case S_listen:
           (void *)F_listen = _socksLib->symbol(it.data().latin1());
          break;
         case S_bind:
           (void *)F_bind = _socksLib->symbol(it.data().latin1());
          break;
         default:
          kdDebug() << "KSocks got a symbol it doesn't know about!" << endl;
          break;
         }
      }
 
      // Now we check for the critical stuff.
      if (F_SOCKSinit) {
        int rc = (*F_SOCKSinit)((char *)"KDE");
        if (rc != 0) stopSocks();
        else  kdDebug() << "SOCKS has been activated!" << endl;
      } else {
        stopSocks();
      }
   }
}


KSocks::~KSocks() {
  stopSocks();
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

int KSocks::connect (int sockfd, const struct sockaddr *serv_addr,
                                                   socklen_t addrlen) {
  if (_useSocks)
    return (*F_connect)(sockfd, serv_addr, addrlen);
  else return ::connect(sockfd, serv_addr, addrlen);
}


ssize_t KSocks::read (int fd, void *buf, ssize_t count) {
  if (_useSocks)
    return (*F_read)(fd, buf, count);
  else return ::read(fd, buf, count);
}


ssize_t KSocks::write (int fd, const void *buf, size_t count) {
  if (_useSocks)
    return (*F_write)(fd, buf, count);
  else return ::write(fd, buf, count);
}


int KSocks::recvfrom (int s, void *buf, size_t len, int flags,
                                struct sockaddr *from, socklen_t *fromlen) {
  if (_useSocks)
    return (*F_recvfrom)(s, buf, len, flags, from, fromlen);
  else return ::recvfrom(s, buf, len, flags, from, fromlen);
}


int KSocks::sendto (int s, const void *msg, size_t len, int flags,
                             const struct sockaddr *to, socklen_t tolen) {
  if (_useSocks)
    return (*F_sendto)(s, msg, len, flags, to, tolen);
  else return ::sendto(s, msg, len, flags, to, tolen);
}


int KSocks::recv (int s, void *buf, size_t len, int flags) {
  if (_useSocks)
    return (*F_recv)(s, buf, len, flags);
  else return ::recv(s, buf, len, flags);
}


int KSocks::send (int s, const void *msg, size_t len, int flags) {
  if (_useSocks)
    return (*F_send)(s, msg, len, flags);
  else return ::send(s, msg, len, flags);
}


int KSocks::getsockname (int s, struct sockaddr *name, socklen_t *namelen) {
  if (_useSocks)
    return (*F_getsockname)(s, name, namelen);
  else return ::getsockname(s, name, namelen);
}


int KSocks::getpeername (int s, struct sockaddr *name, socklen_t *namelen) {
  if (_useSocks)
    return (*F_getpeername)(s, name, namelen);
  else return ::getpeername(s, name, namelen);
}


int KSocks::accept (int s, struct sockaddr *addr, socklen_t *addrlen) {
  if (_useSocks)
    return (*F_accept)(s, addr, addrlen);
  else return ::accept(s, addr, addrlen);
}


int KSocks::select (int n, fd_set *readfds, fd_set *writefds,
                                fd_set *exceptfds, struct timeval *timeout) {
  if (_useSocks)
    return (*F_select)(n, readfds, writefds, exceptfds, timeout);
  else return ::select(n, readfds, writefds, exceptfds, timeout);
}


int KSocks::listen (int s, int backlog) {
  if (_useSocks)
    return (*F_listen)(s, backlog);
  else return ::listen(s, backlog);
}


int KSocks::bind (int sockfd, struct sockaddr *my_addr, socklen_t addrlen) {
  if (_useSocks)
    return (*F_bind)(sockfd, my_addr, addrlen);
  else return ::bind(sockfd, my_addr, addrlen);
}



