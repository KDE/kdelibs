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

#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#undef crypt
#endif

#include "kopenssl.h"

#ifdef HAVE_SSL
extern "C" {
static int (*K_SSL_connect)     (SSL *ssl) = NULL;
static int (*K_SSL_read)        (SSL *ssl, void *buf, int num) = NULL;
static int (*K_SSL_write)       (SSL *ssl, const void *buf, int num) = NULL;
static SSL *(*K_SSL_new)        (SSL_CTX *ctx) = NULL;
static void (*K_SSL_free)       (SSL *ssl) = NULL;
static SSL_CTX *(*K_SSL_CTX_new)(SSL_METHOD *method) = NULL;
static void (*K_SSL_CTX_free)   (SSL_CTX *ctx) = NULL;
static int (*K_SSL_set_fd)      (SSL *ssl, int fd) = NULL;
static int (*K_SSL_pending)     (SSL *ssl) = NULL;
static int (*K_SSL_CTX_set_cipher_list)(SSL_CTX *ctx, const char *str) = NULL;
static void (*K_SSL_CTX_set_verify)(SSL_CTX *ctx, int mode,
                         int (*verify_callback)(int, X509_STORE_CTX *)) = NULL;
static int (*K_SSL_CTX_use_certificate)(SSL_CTX *ctx, X509 *x) = NULL;
static SSL_CIPHER *(*K_SSL_get_current_cipher)(SSL *ssl) = NULL;
};
#endif


KOpenSSLProxy::KOpenSSLProxy() {
KLibLoader *ll = KLibLoader::self();
_ok = false;

KLibrary *sslLib;

   sslLib = ll->library("/usr/lib/libssl.so");

   if (sslLib) {
      // stand back from your monitor and look at this.  it's fun! :)
      K_SSL_connect = (int (*)(SSL *)) sslLib->symbol("SSL_connect");
      K_SSL_read = (int (*)(SSL *, void *, int)) sslLib->symbol("SSL_read");
      K_SSL_write = (int (*)(SSL *, const void *, int)) 
                            sslLib->symbol("SSL_write");
      K_SSL_new = (SSL* (*)(SSL_CTX *)) sslLib->symbol("SSL_new");
      K_SSL_free = (void (*)(SSL *)) sslLib->symbol("SSL_free");
      K_SSL_CTX_new = (SSL_CTX* (*)(SSL_METHOD*)) sslLib->symbol("SSL_CTX_new");
      K_SSL_CTX_free = (void (*)(SSL_CTX*)) sslLib->symbol("SSL_CTX_free");
      K_SSL_set_fd = (int (*)(SSL *, int)) sslLib->symbol("SSL_set_fd");
      K_SSL_pending = (int (*)(SSL *)) sslLib->symbol("SSL_pending");
      K_SSL_CTX_set_cipher_list = (int (*)(SSL_CTX *, const char *))
                                  sslLib->symbol("SSL_CTX_set_cipher_list");
      K_SSL_CTX_set_verify = (void (*)(SSL_CTX*, int, int (*)(int, X509_STORE_CTX*))) sslLib->symbol("SSL_CTX_set_verify");
      K_SSL_CTX_use_certificate = (int (*)(SSL_CTX*, X509*)) 
                                  sslLib->symbol("SSL_CTX_use_certificate");
      K_SSL_get_current_cipher = (SSL_CIPHER *(*)(SSL *)) 
                                  sslLib->symbol("SSL_get_current_cipher");
   }
}


KOpenSSLProxy::~KOpenSSLProxy() {

}


KOpenSSLProxy* KOpenSSLProxy::_me = NULL;


KOpenSSLProxy *KOpenSSLProxy::self() {
#ifdef HAVE_SSL
   if (!_me) {
      _me = new KOpenSSLProxy;
   }
#endif
   return _me;
}







#ifdef HAVE_SSL



int KOpenSSLProxy::SSL_connect(SSL *ssl) {
   if (K_SSL_connect) return (K_SSL_connect)(ssl);
   return -1;
}


int KOpenSSLProxy::SSL_read(SSL *ssl, void *buf, int num) {
   if (K_SSL_read) return (K_SSL_read)(ssl, buf, num);
   return -1;
}


int KOpenSSLProxy::SSL_write(SSL *ssl, const void *buf, int num) {
   if (K_SSL_write) return (K_SSL_write)(ssl, buf, num);
   return -1;
}


SSL *KOpenSSLProxy::SSL_new(SSL_CTX *ctx) {
   if (K_SSL_new) return (K_SSL_new)(ctx);
   return NULL;
}


void KOpenSSLProxy::SSL_free(SSL *ssl) {
   if (K_SSL_free) (K_SSL_free)(ssl);
}


SSL_CTX *KOpenSSLProxy::SSL_CTX_new(SSL_METHOD *method) {
   if (K_SSL_CTX_new) return (K_SSL_CTX_new)(method);
   return NULL;
}


void KOpenSSLProxy::SSL_CTX_free(SSL_CTX *ctx) {
   if (K_SSL_CTX_free) (K_SSL_CTX_free)(ctx);
}


int KOpenSSLProxy::SSL_set_fd(SSL *ssl, int fd) {
   if (K_SSL_set_fd) return (K_SSL_set_fd)(ssl, fd);
   return -1;
}


int KOpenSSLProxy::SSL_pending(SSL *ssl) {
   if (K_SSL_pending) return (K_SSL_pending)(ssl);
   return -1;
}


int KOpenSSLProxy::SSL_CTX_set_cipher_list(SSL_CTX *ctx, const char *str) {
   if (K_SSL_CTX_set_cipher_list) return (K_SSL_CTX_set_cipher_list)(ctx, str);
   return -1;
}


void KOpenSSLProxy::SSL_CTX_set_verify(SSL_CTX *ctx, int mode,
                              int (*verify_callback)(int, X509_STORE_CTX *)) {
   if (K_SSL_CTX_set_verify) (K_SSL_CTX_set_verify)(ctx, mode, verify_callback);
}


int KOpenSSLProxy::SSL_CTX_use_certificate(SSL_CTX *ctx, X509 *x) {
   if (K_SSL_CTX_use_certificate) return (K_SSL_CTX_use_certificate)(ctx, x);
   return -1;
}


SSL_CIPHER *KOpenSSLProxy::SSL_get_current_cipher(SSL *ssl) {
   if (K_SSL_get_current_cipher) return (K_SSL_get_current_cipher)(ssl);
   return NULL;
}







#endif
