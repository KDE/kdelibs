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

#include <kdebug.h>
#include <kconfig.h>

#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/asn1.h>
#undef crypt
#endif

#include <stdio.h>
#include "kopenssl.h"

#ifdef HAVE_SSL
extern "C" {
static int (*K_SSL_connect)     (SSL *ssl) = NULL;
static int (*K_SSL_read)        (SSL *ssl, void *buf, int num) = NULL;
static int (*K_SSL_write)       (SSL *ssl, const void *buf, int num) = NULL;
static SSL *(*K_SSL_new)        (SSL_CTX *ctx) = NULL;
static void (*K_SSL_free)       (SSL *ssl) = NULL;
static int (*K_SSL_shutdown)    (SSL *ssl) = NULL;
static SSL_CTX *(*K_SSL_CTX_new)(SSL_METHOD *method) = NULL;
static void (*K_SSL_CTX_free)   (SSL_CTX *ctx) = NULL;
static int (*K_SSL_set_fd)      (SSL *ssl, int fd) = NULL;
static int (*K_SSL_pending)     (SSL *ssl) = NULL;
static int (*K_SSL_CTX_set_cipher_list)(SSL_CTX *ctx, const char *str) = NULL;
static void (*K_SSL_CTX_set_verify)(SSL_CTX *ctx, int mode,
                         int (*verify_callback)(int, X509_STORE_CTX *)) = NULL;
static int (*K_SSL_CTX_use_certificate)(SSL_CTX *ctx, X509 *x) = NULL;
static SSL_CIPHER *(*K_SSL_get_current_cipher)(SSL *ssl) = NULL;
static long (*K_SSL_ctrl)      (SSL *ssl,int cmd, long larg, char *parg) = NULL;
static int (*K_RAND_egd)        (const char *path) = NULL;
static SSL_METHOD * (*K_TLSv1_client_method) () = NULL;
static SSL_METHOD * (*K_SSLv2_client_method) () = NULL;
static SSL_METHOD * (*K_SSLv3_client_method) () = NULL;
static SSL_METHOD * (*K_SSLv23_client_method) () = NULL;
static X509 * (*K_SSL_get_peer_certificate) (SSL *) = NULL;
static int (*K_SSL_CIPHER_get_bits) (SSL_CIPHER *,int *) = NULL;
static char * (*K_SSL_CIPHER_get_version) (SSL_CIPHER *) = NULL;
static const char * (*K_SSL_CIPHER_get_name) (SSL_CIPHER *) = NULL;
static char * (*K_SSL_CIPHER_description) (SSL_CIPHER *, char *, int) = NULL;
static X509 * (*K_d2i_X509) (X509 **,unsigned char **,long) = NULL;
static int (*K_i2d_X509) (X509 *,unsigned char **) = NULL;
static int (*K_X509_cmp) (X509 *, X509 *) = NULL;
static void (*K_X509_STORE_CTX_free) (X509_STORE_CTX *) = NULL;
static int (*K_X509_verify_cert) (X509_STORE_CTX *) = NULL;
static X509_STORE_CTX *(*K_X509_STORE_CTX_new) (void) = NULL;
static void (*K_X509_STORE_free) (X509_STORE *) = NULL;
static X509_STORE *(*K_X509_STORE_new) (void) = NULL;
static void (*K_X509_free) (X509 *) = NULL;
static char *(*K_X509_NAME_oneline) (X509_NAME *,char *,int) = NULL;
static X509_NAME *(*K_X509_get_subject_name) (X509 *) = NULL;
static X509_NAME *(*K_X509_get_issuer_name) (X509 *) = NULL;
static X509_LOOKUP *(*K_X509_STORE_add_lookup) (X509_STORE *, X509_LOOKUP_METHOD *) = NULL;
static X509_LOOKUP_METHOD *(*K_X509_LOOKUP_file)(void) = NULL;
static int (*K_X509_LOOKUP_ctrl)(X509_LOOKUP *, int, const char *, long, char **) = NULL;
static void (*K_X509_STORE_CTX_init)(X509_STORE_CTX *, X509_STORE *, X509 *, STACK_OF(X509) *) = NULL;
static void (*K_CRYPTO_free)       (void *) = NULL;
static X509* (*K_X509_dup)         (X509 *) = NULL;
static BIO* (*K_BIO_new_fp)   (FILE *, int) = NULL;
static int  (*K_BIO_free)           (BIO *) = NULL;
static int (*K_PEM_ASN1_write_bio) (int (*)(),const char *,BIO *,char *,
                                   const EVP_CIPHER *,unsigned char *,int ,
                                            pem_password_cb *, void *) = NULL;
static ASN1_METHOD* (*K_X509_asn1_meth) (void) = NULL;
static int (*K_ASN1_i2d_fp)(int (*)(),FILE *,unsigned char *) = NULL;
static int (*K_i2d_ASN1_HEADER)(ASN1_HEADER *, unsigned char **) = NULL;
};
#endif


bool KOpenSSLProxy::hasLibSSL() const {
   return _sslLib != NULL;
}


bool KOpenSSLProxy::hasLibCrypto() const {
   return _cryptoLib != NULL;
}


void KOpenSSLProxy::destroy() {
  delete this;
  _me = NULL;
}


KOpenSSLProxy::KOpenSSLProxy() {
KLibLoader *ll = KLibLoader::self();
_ok = false;
QStringList libpaths, libnamesc, libnamess;
KConfig *cfg;

   cfg = new KConfig("cryptodefaults", false, false);
   cfg->setGroup("OpenSSL");
   QString upath = cfg->readEntry("Path", "");
   if (upath.length() > 0)
      libpaths << upath;

   delete cfg;

   libpaths << "/usr/lib/"
            << "/usr/local/lib/"
            << "/usr/local/openssl/lib/"
            << "/usr/local/ssl/lib/"
            << "/opt/openssl/lib/"
            << "";

// FIXME: #define here for the various OS types to optimize
   libnamess << "libssl.so"
             << "libssl.sl";

   libnamesc << "libcrypto.so"
             << "libcrypto.sl";

   for (QStringList::Iterator it = libpaths.begin();
                              it != libpaths.end();
                              ++it) {
      for (QStringList::Iterator shit = libnamesc.begin();
                                 shit != libnamesc.end();
                                 ++shit) {
         QString alib = *it+*shit;
         _cryptoLib = ll->globalLibrary(alib.latin1());
         if (_cryptoLib) break;
      }
      if (_cryptoLib) break;
   }

   if (_cryptoLib) {
#ifdef HAVE_SSL 
      K_X509_free = (void (*) (X509 *)) _cryptoLib->symbol("X509_free");
      K_RAND_egd = (int (*)(const char *)) _cryptoLib->symbol("RAND_egd");
      K_CRYPTO_free = (void (*) (void *)) _cryptoLib->symbol("CRYPTO_free");
      K_d2i_X509 = (X509 * (*)(X509 **,unsigned char **,long)) _cryptoLib->symbol("d2i_X509");
      K_i2d_X509 = (int (*)(X509 *,unsigned char **)) _cryptoLib->symbol("i2d_X509");
      K_X509_cmp = (int (*)(X509 *, X509 *)) _cryptoLib->symbol("X509_cmp");
      K_X509_STORE_CTX_new = (X509_STORE_CTX * (*) (void)) _cryptoLib->symbol("X509_STORE_CTX_new");
      K_X509_STORE_CTX_free = (void (*) (X509_STORE_CTX *)) _cryptoLib->symbol("X509_STORE_CTX_free");
      K_X509_verify_cert = (int (*) (X509_STORE_CTX *)) _cryptoLib->symbol("X509_verify_cert");
      K_X509_STORE_new = (X509_STORE * (*) (void)) _cryptoLib->symbol("X509_STORE_new");
      K_X509_STORE_free = (void (*) (X509_STORE *)) _cryptoLib->symbol("X509_STORE_free");
      K_X509_NAME_oneline = (char * (*) (X509_NAME *,char *,int)) _cryptoLib->symbol("X509_NAME_oneline");
      K_X509_get_subject_name = (X509_NAME * (*) (X509 *)) _cryptoLib->symbol("X509_get_subject_name");
      K_X509_get_issuer_name = (X509_NAME * (*) (X509 *)) _cryptoLib->symbol("X509_get_issuer_name");
      K_X509_STORE_add_lookup = (X509_LOOKUP *(*) (X509_STORE *, X509_LOOKUP_METHOD *)) _cryptoLib->symbol("X509_STORE_add_lookup");
      K_X509_LOOKUP_file = (X509_LOOKUP_METHOD *(*)(void)) _cryptoLib->symbol("X509_LOOKUP_file");
      K_X509_LOOKUP_ctrl = (int (*)(X509_LOOKUP *, int, const char *, long, char **)) _cryptoLib->symbol("X509_LOOKUP_ctrl");
      K_X509_STORE_CTX_init = (void (*)(X509_STORE_CTX *, X509_STORE *, X509 *, STACK_OF(X509) *)) _cryptoLib->symbol("X509_STORE_CTX_init");
      K_X509_dup = (X509* (*)(X509*)) _cryptoLib->symbol("X509_dup");
      K_BIO_new_fp = (BIO* (*)(FILE*, int)) _cryptoLib->symbol("BIO_new_fp");
      K_BIO_free = (int (*)(BIO*)) _cryptoLib->symbol("BIO_free");
      K_PEM_ASN1_write_bio = (int (*)(int (*)(), const char *,BIO*, char*, const EVP_CIPHER *, unsigned char *, int, pem_password_cb *, void *)) _cryptoLib->symbol("PEM_ASN1_write_bio");
      K_X509_asn1_meth = (ASN1_METHOD* (*)(void)) _cryptoLib->symbol("X509_asn1_meth");
      K_ASN1_i2d_fp = (int (*)(int (*)(), FILE*, unsigned char *)) _cryptoLib->symbol("ASN1_i2d_fp");
      K_i2d_ASN1_HEADER = (int (*)(ASN1_HEADER *, unsigned char **)) _cryptoLib->symbol("i2d_ASN1_HEADER");

#endif
   }

   for (QStringList::Iterator it = libpaths.begin();
                              it != libpaths.end();
                              ++it) {
      for (QStringList::Iterator shit = libnamess.begin();
                                 shit != libnamess.end();
                                 ++shit) {
         QString alib = *it+*shit;
         _sslLib = ll->globalLibrary(alib.latin1());
         if (_sslLib) break;
      }
      if (_sslLib) break;
   }

   if (_sslLib) {
#ifdef HAVE_SSL 
      // stand back from your monitor and look at this.  it's fun! :)
      K_SSL_connect = (int (*)(SSL *)) _sslLib->symbol("SSL_connect");
      K_SSL_read = (int (*)(SSL *, void *, int)) _sslLib->symbol("SSL_read");
      K_SSL_write = (int (*)(SSL *, const void *, int)) 
                            _sslLib->symbol("SSL_write");
      K_SSL_new = (SSL* (*)(SSL_CTX *)) _sslLib->symbol("SSL_new");
      K_SSL_free = (void (*)(SSL *)) _sslLib->symbol("SSL_free");
      K_SSL_shutdown = (int (*)(SSL *)) _sslLib->symbol("SSL_shutdown");
      K_SSL_CTX_new = (SSL_CTX* (*)(SSL_METHOD*)) _sslLib->symbol("SSL_CTX_new");
      K_SSL_CTX_free = (void (*)(SSL_CTX*)) _sslLib->symbol("SSL_CTX_free");
      K_SSL_set_fd = (int (*)(SSL *, int)) _sslLib->symbol("SSL_set_fd");
      K_SSL_pending = (int (*)(SSL *)) _sslLib->symbol("SSL_pending");
      K_SSL_CTX_set_cipher_list = (int (*)(SSL_CTX *, const char *))
                                  _sslLib->symbol("SSL_CTX_set_cipher_list");
      K_SSL_CTX_set_verify = (void (*)(SSL_CTX*, int, int (*)(int, X509_STORE_CTX*))) _sslLib->symbol("SSL_CTX_set_verify");
      K_SSL_CTX_use_certificate = (int (*)(SSL_CTX*, X509*)) 
                                  _sslLib->symbol("SSL_CTX_use_certificate");
      K_SSL_get_current_cipher = (SSL_CIPHER *(*)(SSL *)) 
                                  _sslLib->symbol("SSL_get_current_cipher");
      K_SSL_ctrl = (long (*)(SSL * ,int, long, char *))
                                  _sslLib->symbol("SSL_ctrl");
      K_TLSv1_client_method = (SSL_METHOD *(*)()) _sslLib->symbol("TLSv1_client_method");
      K_SSLv2_client_method = (SSL_METHOD *(*)()) _sslLib->symbol("SSLv2_client_method");
      K_SSLv3_client_method = (SSL_METHOD *(*)()) _sslLib->symbol("SSLv3_client_method");
      K_SSLv23_client_method = (SSL_METHOD *(*)()) _sslLib->symbol("SSLv23_client_method");
      K_SSL_get_peer_certificate = (X509 *(*)(SSL *)) _sslLib->symbol("SSL_get_peer_certificate");
      K_SSL_CIPHER_get_bits = (int (*)(SSL_CIPHER *,int *)) _sslLib->symbol("SSL_CIPHER_get_bits");
      K_SSL_CIPHER_get_version = (char * (*)(SSL_CIPHER *)) _sslLib->symbol("SSL_CIPHER_get_version");
      K_SSL_CIPHER_get_name = (const char * (*)(SSL_CIPHER *)) _sslLib->symbol("SSL_CIPHER_get_name");
      K_SSL_CIPHER_description = (char * (*)(SSL_CIPHER *, char *, int)) _sslLib->symbol("SSL_CIPHER_description");
#endif


      // Initialize the library (once only!)
      void *x;
      x = _sslLib->symbol("SSL_library_init");
      if (x) ((int (*)())x)();
      x = _cryptoLib->symbol("OpenSSL_add_all_algorithms");
      if (x) ((void (*)())x)();
      x = _cryptoLib->symbol("OpenSSL_add_all_ciphers");
      if (x) ((void (*)())x)();
      x = _cryptoLib->symbol("OpenSSL_add_all_digests");
      if (x) ((void (*)())x)();
   }

}


KOpenSSLProxy::~KOpenSSLProxy() {
   delete _sslLib;
   delete _cryptoLib;
}


KOpenSSLProxy* KOpenSSLProxy::_me = NULL;


// FIXME: we should check "ok" and allow this to init the lib if !ok.

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


int KOpenSSLProxy::SSL_shutdown(SSL *ssl) {
   if (K_SSL_shutdown) return (K_SSL_shutdown)(ssl);
   return -1;
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


long KOpenSSLProxy::SSL_ctrl(SSL *ssl,int cmd, long larg, char *parg) {
   if (K_SSL_ctrl) return (K_SSL_ctrl)(ssl, cmd, larg, parg);
   return -1;
}


int KOpenSSLProxy::RAND_egd(const char *path) {
   if (K_RAND_egd) return (K_RAND_egd)(path);
   return -1;
}


SSL_METHOD *KOpenSSLProxy::TLSv1_client_method() {
   if (K_TLSv1_client_method) return (K_TLSv1_client_method)();
   return NULL;
}


SSL_METHOD *KOpenSSLProxy::SSLv2_client_method() {
   if (K_SSLv2_client_method) return (K_SSLv2_client_method)();
   return NULL;
}


SSL_METHOD *KOpenSSLProxy::SSLv3_client_method() {
   if (K_SSLv3_client_method) return (K_SSLv3_client_method)();
   return NULL;
}


SSL_METHOD *KOpenSSLProxy::SSLv23_client_method() {
   if (K_SSLv23_client_method) return (K_SSLv23_client_method)();
   return NULL;
}


X509 *KOpenSSLProxy::SSL_get_peer_certificate(SSL *s) {
   if (K_SSL_get_peer_certificate) return (K_SSL_get_peer_certificate)(s);
   return NULL;
}


int KOpenSSLProxy::SSL_CIPHER_get_bits(SSL_CIPHER *c,int *alg_bits) {
   if (K_SSL_CIPHER_get_bits) return (K_SSL_CIPHER_get_bits)(c, alg_bits);
   return -1;
}


char * KOpenSSLProxy::SSL_CIPHER_get_version(SSL_CIPHER *c) {
   if (K_SSL_CIPHER_get_version) return (K_SSL_CIPHER_get_version)(c);
   return NULL;
}


const char * KOpenSSLProxy::SSL_CIPHER_get_name(SSL_CIPHER *c) {
   if (K_SSL_CIPHER_get_name) return (K_SSL_CIPHER_get_name)(c);
   return NULL;
}


char * KOpenSSLProxy::SSL_CIPHER_description(SSL_CIPHER *c,char *buf,int size) {
   if (K_SSL_CIPHER_description) return (K_SSL_CIPHER_description)(c,buf,size);
   return NULL;
}


X509 * KOpenSSLProxy::d2i_X509(X509 **a,unsigned char **pp,long length) {
   if (K_d2i_X509) return (K_d2i_X509)(a,pp,length);
   return NULL;
}


int KOpenSSLProxy::i2d_X509(X509 *a,unsigned char **pp) {
   if (K_i2d_X509) return (K_i2d_X509)(a,pp);
   return -1;
}


int KOpenSSLProxy::X509_cmp(X509 *a, X509 *b) {
   if (K_X509_cmp) return (K_X509_cmp)(a,b);
   return 0;
}


X509_STORE *KOpenSSLProxy::X509_STORE_new(void) {
   if (K_X509_STORE_new) return (K_X509_STORE_new)();
   return NULL;
}


void KOpenSSLProxy::X509_STORE_free(X509_STORE *v) {
   if (K_X509_STORE_free) (K_X509_STORE_free)(v);
}


X509_STORE_CTX *KOpenSSLProxy::X509_STORE_CTX_new(void) {
   if (K_X509_STORE_CTX_new) return (K_X509_STORE_CTX_new)();
   return NULL;
}


void KOpenSSLProxy::X509_STORE_CTX_free(X509_STORE_CTX *ctx) {
   if (K_X509_STORE_CTX_free) (K_X509_STORE_CTX_free)(ctx);
}


int KOpenSSLProxy::X509_verify_cert(X509_STORE_CTX *ctx) {
   if (K_X509_verify_cert) return (K_X509_verify_cert)(ctx);
   return -1;
}


void KOpenSSLProxy::X509_free(X509 *a) {
   if (K_X509_free) (K_X509_free)(a);
}


char *KOpenSSLProxy::X509_NAME_oneline(X509_NAME *a,char *buf,int size) {
   if (K_X509_NAME_oneline) return (K_X509_NAME_oneline)(a,buf,size);
   return NULL;
}


X509_NAME *KOpenSSLProxy::X509_get_subject_name(X509 *a) {
   if (K_X509_get_subject_name) return (K_X509_get_subject_name)(a);
   return NULL;
}


X509_NAME *KOpenSSLProxy::X509_get_issuer_name(X509 *a) {
   if (K_X509_get_issuer_name) return (K_X509_get_issuer_name)(a);
   return NULL;
}


X509_LOOKUP *KOpenSSLProxy::X509_STORE_add_lookup(X509_STORE *v, X509_LOOKUP_METHOD *m) {
   if (K_X509_STORE_add_lookup) return (K_X509_STORE_add_lookup)(v,m);
   return NULL;
}


X509_LOOKUP_METHOD *KOpenSSLProxy::X509_LOOKUP_file(void) {
   if (K_X509_LOOKUP_file) return (K_X509_LOOKUP_file)();
   return NULL;
}


int KOpenSSLProxy::X509_LOOKUP_ctrl(X509_LOOKUP *ctx, int cmd, const char *argc, long argl, char **ret) {
   if (K_X509_LOOKUP_ctrl) return (K_X509_LOOKUP_ctrl)(ctx,cmd,argc,argl,ret);
   return -1;
}


void KOpenSSLProxy::X509_STORE_CTX_init(X509_STORE_CTX *ctx, X509_STORE *store, X509 *x509, STACK_OF(X509) *chain) {
   if (K_X509_STORE_CTX_init) (K_X509_STORE_CTX_init)(ctx,store,x509,chain);
}


void KOpenSSLProxy::CRYPTO_free(void *x) {
   if (K_CRYPTO_free) (K_CRYPTO_free)(x);
}


X509 *KOpenSSLProxy::X509_dup(X509 *x509) {
   if (K_X509_dup) return (K_X509_dup)(x509);
   return NULL;
}


BIO *KOpenSSLProxy::BIO_new_fp(FILE *stream, int close_flag) {
   if (K_BIO_new_fp) return (K_BIO_new_fp)(stream, close_flag);
   return NULL;
}


int KOpenSSLProxy::BIO_free(BIO *a) {
   if (K_BIO_free) return (K_BIO_free)(a);
   return -1;
}


int KOpenSSLProxy::PEM_write_bio_X509(BIO *bp, X509 *x) {
   if (K_PEM_ASN1_write_bio) return (K_PEM_ASN1_write_bio) ((int (*)())K_i2d_X509, PEM_STRING_X509, bp, (char *)x, NULL, NULL, 0, NULL, NULL);
   else return -1;
}


ASN1_METHOD *KOpenSSLProxy::X509_asn1_meth(void) {
   if (K_X509_asn1_meth) return (K_X509_asn1_meth)();
   else return NULL;
}


int KOpenSSLProxy::ASN1_i2d_fp(FILE *out,unsigned char *x) {
   if (K_ASN1_i2d_fp && K_i2d_ASN1_HEADER) 
        return (K_ASN1_i2d_fp)((int (*)())K_i2d_ASN1_HEADER, out, x);
   else return -1;
}


#endif

