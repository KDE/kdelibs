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

#ifndef __KOPENSSLPROXY_H 
#define __KOPENSSLPROXY_H

#define KOSSL KOpenSSLProxy;
class KOpenSSLProxyPrivate;

#include <klibloader.h>

class KOpenSSLProxy {
public:

   /**
    * Return an instance of class KOpenSSLProxy *
    * You cannot delete this object.  It is a singleton class.
    */
   static KOpenSSLProxy *self();

   // Here are the symbols that we need.
#ifdef HAVE_SSL

   /*
    *   SSL_connect - initiate the TLS/SSL handshake with an TLS/SSL server
    */
   int SSL_connect(SSL *ssl);

   /*
    *   SSL_read - read bytes from a TLS/SSL connection.
    */
   int SSL_read(SSL *ssl, void *buf, int num);

   /*
    *   SSL_write - write bytes to a TLS/SSL connection.
    */
   int SSL_write(SSL *ssl, const void *buf, int num);

   /*
    *   SSL_new - create a new SSL structure for a connection
    */
   SSL *SSL_new(SSL_CTX *ctx);

   /*
    *   SSL_free - free an allocated SSL structure
    */
   void SSL_free(SSL *ssl);

   /*
    *   SSL_CTX_new - create a new SSL_CTX object as framework for TLS/SSL enabled functions
    */
   SSL_CTX *SSL_CTX_new(SSL_METHOD *method);

   /*
    *   SSL_CTX_free - free an allocated SSL_CTX object
    */
   void SSL_CTX_free(SSL_CTX *ctx);

   /*
    *   
    */


#endif

private:
   KOpenSSLProxy();
   ~KOpenSSLProxy();
   KOpenSSLProxyPrivate *d;

   KLibrary *_sslLib;
   static KOpenSSLProxy *_me;

};

#endif

