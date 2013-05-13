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

#include "kopenssl.h"

#include <ksslconfig.h>

#if KSSL_HAVE_SSL
#include <openssl/opensslv.h>
#endif

#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <klibrary.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QRegExp>


#include <stdio.h>
#include <unistd.h>

extern "C" {
#if KSSL_HAVE_SSL
static int (*K_SSL_connect)     (SSL *) = 0L;
static int (*K_SSL_accept)      (SSL *) = 0L;
static int (*K_SSL_read)        (SSL *, void *, int) = 0L;
static int (*K_SSL_write)       (SSL *, const void *, int) = 0L;
static SSL *(*K_SSL_new)        (SSL_CTX *) = 0L;
static void (*K_SSL_free)       (SSL *) = 0L;
static int (*K_SSL_shutdown)    (SSL *) = 0L;
static SSL_CTX *(*K_SSL_CTX_new)(SSL_METHOD *) = 0L;
static void (*K_SSL_CTX_free)   (SSL_CTX *) = 0L;
static int (*K_SSL_set_fd)      (SSL *, int) = 0L;
static int (*K_SSL_pending)     (SSL *) = 0L;
static int (*K_SSL_peek)        (SSL *, void *, int) = 0L;
static int (*K_SSL_CTX_set_cipher_list)(SSL_CTX *, const char *) = 0L;
static void (*K_SSL_CTX_set_verify)(SSL_CTX *, int,
                         int (*)(int, X509_STORE_CTX *)) = 0L;
static int (*K_SSL_use_certificate)(SSL *, X509 *) = 0L;
static SSL_CIPHER *(*K_SSL_get_current_cipher)(SSL *) = 0L;
static long (*K_SSL_ctrl)      (SSL *,int, long, char *) = 0L;
static int (*K_RAND_egd)        (const char *) = 0L;
static const char* (*K_RAND_file_name) (char *, size_t) = 0L;
static int (*K_RAND_load_file)  (const char *, long) = 0L;
static int (*K_RAND_write_file) (const char *) = 0L;
static SSL_METHOD * (*K_TLSv1_client_method) () = 0L;
static SSL_METHOD * (*K_SSLv23_client_method) () = 0L;
static X509 * (*K_SSL_get_peer_certificate) (SSL *) = 0L;
static int (*K_SSL_CIPHER_get_bits) (SSL_CIPHER *,int *) = 0L;
static char * (*K_SSL_CIPHER_get_version) (SSL_CIPHER *) = 0L;
static const char * (*K_SSL_CIPHER_get_name) (SSL_CIPHER *) = 0L;
static char * (*K_SSL_CIPHER_description) (SSL_CIPHER *, char *, int) = 0L;
static X509 * (*K_d2i_X509) (X509 **,unsigned char **,long) = 0L;
static int (*K_i2d_X509) (X509 *,unsigned char **) = 0L;
static int (*K_X509_cmp) (X509 *, X509 *) = 0L;
static void (*K_X509_STORE_CTX_free) (X509_STORE_CTX *) = 0L;
static int (*K_X509_verify_cert) (X509_STORE_CTX *) = 0L;
static X509_STORE_CTX *(*K_X509_STORE_CTX_new) (void) = 0L;
static void (*K_X509_STORE_free) (X509_STORE *) = 0L;
static X509_STORE *(*K_X509_STORE_new) (void) = 0L;
static void (*K_X509_free) (X509 *) = 0L;
static char *(*K_X509_NAME_oneline) (X509_NAME *,char *,int) = 0L;
static X509_NAME *(*K_X509_get_subject_name) (X509 *) = 0L;
static X509_NAME *(*K_X509_get_issuer_name) (X509 *) = 0L;
static X509_LOOKUP *(*K_X509_STORE_add_lookup) (X509_STORE *, X509_LOOKUP_METHOD *) = 0L;
static X509_LOOKUP_METHOD *(*K_X509_LOOKUP_file)(void) = 0L;
static void (*K_X509_LOOKUP_free)(X509_LOOKUP *) = 0L;
static int (*K_X509_LOOKUP_ctrl)(X509_LOOKUP *, int, const char *, long, char **) = 0L;
static void (*K_X509_STORE_CTX_init)(X509_STORE_CTX *, X509_STORE *, X509 *, STACK_OF(X509) *) = 0L;
static void (*K_CRYPTO_free)       (void *) = 0L;
static X509* (*K_X509_dup)         (X509 *) = 0L;
static BIO_METHOD *(*K_BIO_s_mem) (void) = 0L;
static BIO* (*K_BIO_new) (BIO_METHOD *) = 0L;
static BIO* (*K_BIO_new_fp)   (FILE *, int) = 0L;
static BIO* (*K_BIO_new_mem_buf) (void *, int) = 0L;
static int  (*K_BIO_free)           (BIO *) = 0L;
static long (*K_BIO_ctrl) (BIO *,int,long,void *) = 0L;
static int  (*K_BIO_write) (BIO *b, const void *data, int len) = 0L;
static int (*K_PEM_ASN1_write_bio) (int (*)(),const char *,BIO *,char *,
                                   const EVP_CIPHER *,unsigned char *,int ,
                                            pem_password_cb *, void *) = 0L;
static int (*K_ASN1_item_i2d_fp)(ASN1_ITEM *,FILE *,unsigned char *) = 0L;
static ASN1_ITEM *K_NETSCAPE_X509_it = 0L;
static int (*K_X509_print_fp)  (FILE *, X509*) = 0L;
static int (*K_i2d_PKCS12)  (PKCS12*, unsigned char**) = 0L;
static int (*K_i2d_PKCS12_fp)  (FILE *, PKCS12*) = 0L;
static int (*K_PKCS12_newpass) (PKCS12*, char*, char*) = 0L;
static PKCS12* (*K_d2i_PKCS12_fp) (FILE*, PKCS12**) = 0L;
static PKCS12* (*K_PKCS12_new) (void) = 0L;
static void (*K_PKCS12_free) (PKCS12 *) = 0L;
static int (*K_PKCS12_parse) (PKCS12*, const char *, EVP_PKEY**,
                                             X509**, STACK_OF(X509)**) = 0L;
static void (*K_EVP_PKEY_free) (EVP_PKEY *) = 0L;
static EVP_PKEY* (*K_EVP_PKEY_new) () = 0L;
static void (*K_X509_REQ_free) (X509_REQ *) = 0L;
static X509_REQ* (*K_X509_REQ_new) () = 0L;
static int (*K_SSL_CTX_use_PrivateKey) (SSL_CTX*, EVP_PKEY*) = 0L;
static int (*K_SSL_CTX_use_certificate) (SSL_CTX*, X509*) = 0L;
static int (*K_SSL_get_error) (SSL*, int) = 0L;
static STACK_OF(X509)* (*K_SSL_get_peer_cert_chain) (SSL*) = 0L;
static void (*K_X509_STORE_CTX_set_chain) (X509_STORE_CTX *, STACK_OF(X509)*) = 0L;
static void (*K_X509_STORE_CTX_set_purpose) (X509_STORE_CTX *, int) = 0L;
static void (*K_sk_free) (STACK*) = 0L;
static int (*K_sk_num) (STACK*) = 0L;
static char* (*K_sk_pop) (STACK*) = 0L;
static char* (*K_sk_value) (STACK*, int) = 0L;
static STACK* (*K_sk_new) (int (*)()) = 0L;
static int (*K_sk_push) (STACK*, char*) = 0L;
static STACK* (*K_sk_dup) (STACK *) = 0L;
static char * (*K_i2s_ASN1_INTEGER) (X509V3_EXT_METHOD *, ASN1_INTEGER *) =0L;
static ASN1_INTEGER * (*K_X509_get_serialNumber) (X509 *) = 0L;
static EVP_PKEY *(*K_X509_get_pubkey)(X509 *) = 0L;
static int (*K_i2d_PublicKey)(EVP_PKEY *, unsigned char **) = 0L;
static int (*K_X509_check_private_key)(X509 *, EVP_PKEY *) = 0L;
static char * (*K_BN_bn2hex)(const BIGNUM *) = 0L;
static int (*K_X509_digest)(const X509 *,const EVP_MD *, unsigned char *, unsigned int *) = 0L;
static EVP_MD* (*K_EVP_md5)() = 0L;
static void (*K_ASN1_INTEGER_free)(ASN1_INTEGER *) = 0L;
static int (*K_OBJ_obj2nid)(ASN1_OBJECT *) = 0L;
static const char * (*K_OBJ_nid2ln)(int) = 0L;
static int (*K_X509_get_ext_count)(X509*) = 0L;
static int (*K_X509_get_ext_by_NID)(X509*, int, int) = 0L;
static int (*K_X509_get_ext_by_OBJ)(X509*,ASN1_OBJECT*,int) = 0L;
static X509_EXTENSION *(*K_X509_get_ext)(X509*, int loc) = 0L;
static X509_EXTENSION *(*K_X509_delete_ext)(X509*, int) = 0L;
static int (*K_X509_add_ext)(X509*, X509_EXTENSION*, int) = 0L;
static void *(*K_X509_get_ext_d2i)(X509*, int, int*, int*) = 0L;
static char *(*K_i2s_ASN1_OCTET_STRING)(X509V3_EXT_METHOD*, ASN1_OCTET_STRING*) = 0L;
static int (*K_ASN1_BIT_STRING_get_bit)(ASN1_BIT_STRING*, int) = 0L;
static PKCS7 *(*K_PKCS7_new)() = 0L;
static void (*K_PKCS7_free)(PKCS7*) = 0L;
static void (*K_PKCS7_content_free)(PKCS7*) = 0L;
static int (*K_i2d_PKCS7)(PKCS7*, unsigned char**) = 0L;
static PKCS7 *(*K_d2i_PKCS7)(PKCS7**, unsigned char**,long) = 0L;
static int (*K_i2d_PKCS7_fp)(FILE*,PKCS7*) = 0L;
static PKCS7* (*K_d2i_PKCS7_fp)(FILE*,PKCS7**) = 0L;
static int (*K_i2d_PKCS7_bio)(BIO *bp,PKCS7 *p7) = 0L;
static PKCS7 *(*K_d2i_PKCS7_bio)(BIO *bp,PKCS7 **p7) = 0L;
static PKCS7* (*K_PKCS7_dup)(PKCS7*) = 0L;
static STACK_OF(X509_NAME) *(*K_SSL_load_client_CA_file)(const char*) = 0L;
static STACK_OF(X509_INFO) *(*K_PEM_X509_INFO_read)(FILE*, STACK_OF(X509_INFO)*, pem_password_cb*, void*) = 0L;
static char *(*K_ASN1_d2i_fp)(char *(*)(),char *(*)(),FILE*,unsigned char**) = 0L;
static X509 *(*K_X509_new)() = 0L;
static int (*K_X509_PURPOSE_get_count)() = 0L;
static int (*K_X509_PURPOSE_get_id)(X509_PURPOSE *) = 0L;
static int (*K_X509_check_purpose)(X509*,int,int) = 0L;
static X509_PURPOSE* (*K_X509_PURPOSE_get0)(int) = 0L;
static int (*K_EVP_PKEY_assign)(EVP_PKEY*, int, char*) = 0L;
static int (*K_X509_REQ_set_pubkey)(X509_REQ*, EVP_PKEY*) = 0L;
static RSA *(*K_RSA_generate_key)(int, unsigned long, void (*)(int,int,void *), void *) = 0L;
static int (*K_i2d_X509_REQ_fp)(FILE*, X509_REQ*) = 0L;
static void (*K_ERR_clear_error)() = 0L;
static unsigned long (*K_ERR_get_error)() = 0L;
static void (*K_ERR_print_errors_fp)(FILE*) = 0L;
static PKCS7 *(*K_PKCS7_sign)(X509*, EVP_PKEY*, STACK_OF(X509)*, BIO*, int) = 0L;
static int (*K_PKCS7_verify)(PKCS7*,STACK_OF(X509)*,X509_STORE*,BIO*,BIO*,int) = 0L;
static STACK_OF(X509) *(*K_PKCS7_get0_signers)(PKCS7 *, STACK_OF(X509) *, int) = 0L;
static PKCS7 *(*K_PKCS7_encrypt)(STACK_OF(X509) *, BIO *, EVP_CIPHER *, int) = 0L;
static int (*K_PKCS7_decrypt)(PKCS7 *, EVP_PKEY *, X509 *, BIO *, int) = 0L;
static SSL_SESSION* (*K_SSL_get1_session)(SSL*) = 0L;
static void (*K_SSL_SESSION_free)(SSL_SESSION*) = 0L;
static int (*K_SSL_set_session)(SSL*,SSL_SESSION*) = 0L;
static SSL_SESSION* (*K_d2i_SSL_SESSION)(SSL_SESSION**,unsigned char**, long) = 0L;
static int (*K_i2d_SSL_SESSION)(SSL_SESSION*,unsigned char**) = 0L;
static STACK *(*K_X509_get1_email)(X509 *x) = 0L;
static void (*K_X509_email_free)(STACK *sk) = 0L;
static EVP_CIPHER *(*K_EVP_des_ede3_cbc)() = 0L;
static EVP_CIPHER *(*K_EVP_des_cbc)() = 0L;
static EVP_CIPHER *(*K_EVP_rc2_cbc)() = 0L;
static EVP_CIPHER *(*K_EVP_rc2_64_cbc)() = 0L;
static EVP_CIPHER *(*K_EVP_rc2_40_cbc)() = 0L;
static int (*K_i2d_PrivateKey_fp)(FILE*,EVP_PKEY*) = 0L;
static int (*K_i2d_PKCS8PrivateKey_fp)(FILE*, EVP_PKEY*, const EVP_CIPHER*, char*, int, pem_password_cb*, void*) = 0L;
static void (*K_RSA_free)(RSA*) = 0L;
static EVP_CIPHER *(*K_EVP_bf_cbc)() = 0L;
static int (*K_X509_REQ_sign)(X509_REQ*, EVP_PKEY*, const EVP_MD*) = 0L;
static int (*K_X509_NAME_add_entry_by_txt)(X509_NAME*, char*, int, unsigned char*, int, int, int) = 0L;
static X509_NAME *(*K_X509_NAME_new)() = 0L;
static int (*K_X509_REQ_set_subject_name)(X509_REQ*,X509_NAME*) = 0L;
static unsigned char *(*K_ASN1_STRING_data)(ASN1_STRING*) = 0L;
static int (*K_ASN1_STRING_length)(ASN1_STRING*) = 0L;
static STACK_OF(SSL_CIPHER) *(*K_SSL_get_ciphers)(const SSL *ssl) = 0L;

#endif
}


class KOpenSSLProxyPrivate
{
public:
   KOpenSSLProxyPrivate()
    : sslLib(0), cryptoLib(0), ok(false)
   {}

   KLibrary *sslLib;
   KLibrary *cryptoLib;
   bool ok;

   static KOpenSSLProxy *sSelf;
   static void cleanupKOpenSSLProxy() {
      delete KOpenSSLProxyPrivate::sSelf;
   }
};
KOpenSSLProxy *KOpenSSLProxyPrivate::sSelf = 0;

bool KOpenSSLProxy::hasLibSSL() const {
   return d->sslLib != 0L;
}


bool KOpenSSLProxy::hasLibCrypto() const {
   return d->cryptoLib != 0L;
}


void KOpenSSLProxy::destroy() {
   KOpenSSLProxy *x = KOpenSSLProxyPrivate::sSelf;
   KOpenSSLProxyPrivate::sSelf = 0;
   delete x;
}

#ifdef __OpenBSD__
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QStringList>

static QString findMostRecentLib(QString dir, QString name)
{
       // Grab all shared libraries in the directory
       QString filter = "lib"+name+".so.*";
       QDir d(dir, filter);
       if (!d.exists())
               return 0L;
       QStringList l = d.entryList();

       // Find the best one
       int bestmaj = -1;
       int bestmin = -1;
       QString best = 0L;
       // where do we start
       uint s = filter.length()-1;
       for (QStringList::const_iterator it = l.begin(); it != l.end(); ++it) {
               QString numberpart = (*it).mid(s);
               uint endmaj = numberpart.indexOf('.');
               if (endmaj == -1)
                       continue;
               bool ok;
               int maj = numberpart.left(endmaj).toInt(&ok);
               if (!ok)
                       continue;
               int min = numberpart.mid(endmaj+1).toInt(&ok);
               if (!ok)
                       continue;
               if (maj > bestmaj || (maj == bestmaj && min > bestmin)) {
                       bestmaj = maj;
                       bestmin = min;
                       best = (*it);
               }
       }
       if (best.isNull())
               return 0L;
       else
               return dir+'/'+best;
}
#endif

KOpenSSLProxy::KOpenSSLProxy()
    : d(new KOpenSSLProxyPrivate())
{
    QStringList libpaths, libnamesc, libnamess;

   d->cryptoLib = 0L;
   d->sslLib = 0L;

   KConfig cfg("cryptodefaults", KConfig::NoGlobals );
   KConfigGroup cg(&cfg, "OpenSSL");
   QString upath = cg.readPathEntry("Path", QString());
   if (!upath.isEmpty())
      libpaths << upath;

#ifdef Q_OS_WIN
    d->cryptoLib = new KLibrary("libeay32.dll");
    if (!d->cryptoLib->load()) {
       delete d->cryptoLib;
       d->cryptoLib = 0;
    }
#elif defined(__OpenBSD__)
   {
   QString libname = findMostRecentLib("/usr/lib" KDELIBSUFF, "crypto");
   if (!libname.isNull()) {
         d->cryptoLib = new KLibrary(libname);
         d->cryptoLib->setLoadHints(QLibrary::ExportExternalSymbolsHint);
         if (!d->cryptoLib->load()) {
             delete d->cryptoLib;
             d->cryptoLib = 0;
         }
   }
   }
#elif defined(__CYGWIN__)
   libpaths << "/usr/bin/"
   		<< "";

   libnamess << "cygssl-0.9.8.dll"
		 << "cygssl-0.9.7.dll"
		 << "";

   libnamesc << "cygcrypto-0.9.8.dll"
		 << "cygcrypto-0.9.7.dll"
		 << "";
#else
   libpaths
            #ifdef _AIX
            << "/opt/freeware/lib/"
	    #endif
	    << "/usr/lib" KDELIBSUFF "/"
	    << "/usr/ssl/lib" KDELIBSUFF "/"
	    << "/usr/local/lib" KDELIBSUFF "/"
            << "/usr/local/openssl/lib" KDELIBSUFF "/"
            << "/usr/local/ssl/lib" KDELIBSUFF "/"
	    << "/opt/openssl/lib" KDELIBSUFF "/"
	    << "/lib" KDELIBSUFF "/"
            << "";

// FIXME: #define here for the various OS types to optimize
   libnamess
	     #ifdef hpux
             << "libssl.sl"
             #elif defined(_AIX)
             << "libssl.a(libssl.so.0)"
	     #elif defined(__APPLE__)
	     << "libssl.dylib"
	     << "libssl.0.9.dylib"
             #else
             #ifdef SHLIB_VERSION_NUMBER
             << "libssl.so." SHLIB_VERSION_NUMBER
             #endif
             << "libssl.so"
	     << "libssl.so.0"
             #endif
	     ;

   libnamesc
             #ifdef hpux
             << "libcrypto.sl"
             #elif defined(_AIX)
             << "libcrypto.a(libcrypto.so.0)"
	     #elif defined(__APPLE__)
	     << "libcrypto.dylib"
	     << "libcrypto.0.9.dylib"
	     #else
             #ifdef SHLIB_VERSION_NUMBER
             << "libcrypto.so." SHLIB_VERSION_NUMBER
             #endif
             << "libcrypto.so"
	     << "libcrypto.so.0"
             #endif
	     ;
#endif

   for (QStringList::const_iterator it = libpaths.constBegin();
                              it != libpaths.constEnd();
                              ++it) {
      for (QStringList::const_iterator shit = libnamesc.constBegin();
                                 shit != libnamesc.constEnd();
                                 ++shit) {
         QString alib = *it;
         if (!alib.isEmpty() && !alib.endsWith('/'))
            alib += '/';
         alib += *shit;
	     // someone knows why this is needed?
	     QString tmpStr(alib.toLatin1().constData());
	     tmpStr.remove(QRegExp("\\(.*\\)"));
         if (!access(tmpStr.toLatin1(), R_OK)) {
            d->cryptoLib = new KLibrary(alib);
            d->cryptoLib->setLoadHints(QLibrary::ExportExternalSymbolsHint);
         } 
         if (d->cryptoLib && d->cryptoLib->load()) {
             break;
         }
         else {
             delete d->cryptoLib;
             d->cryptoLib = 0;
         }
      }
      if (d->cryptoLib) break;
   }

   if (d->cryptoLib) {
#if KSSL_HAVE_SSL
      K_X509_free = (void (*) (X509 *)) d->cryptoLib->resolveFunction("X509_free");
      K_RAND_egd = (int (*)(const char *)) d->cryptoLib->resolveFunction("RAND_egd");
      K_RAND_load_file = (int (*)(const char *, long)) d->cryptoLib->resolveFunction("RAND_load_file");
      K_RAND_file_name = (const char* (*)(char *, size_t)) d->cryptoLib->resolveFunction("RAND_file_name");
      K_RAND_write_file = (int (*)(const char *)) d->cryptoLib->resolveFunction("RAND_write_file");
      K_CRYPTO_free = (void (*) (void *)) d->cryptoLib->resolveFunction("CRYPTO_free");
      K_d2i_X509 = (X509 * (*)(X509 **,unsigned char **,long)) d->cryptoLib->resolveFunction("d2i_X509");
      K_i2d_X509 = (int (*)(X509 *,unsigned char **)) d->cryptoLib->resolveFunction("i2d_X509");
      K_X509_cmp = (int (*)(X509 *, X509 *)) d->cryptoLib->resolveFunction("X509_cmp");
      K_X509_STORE_CTX_new = (X509_STORE_CTX * (*) (void)) d->cryptoLib->resolveFunction("X509_STORE_CTX_new");
      K_X509_STORE_CTX_free = (void (*) (X509_STORE_CTX *)) d->cryptoLib->resolveFunction("X509_STORE_CTX_free");
      K_X509_verify_cert = (int (*) (X509_STORE_CTX *)) d->cryptoLib->resolveFunction("X509_verify_cert");
      K_X509_STORE_new = (X509_STORE * (*) (void)) d->cryptoLib->resolveFunction("X509_STORE_new");
      K_X509_STORE_free = (void (*) (X509_STORE *)) d->cryptoLib->resolveFunction("X509_STORE_free");
      K_X509_NAME_oneline = (char * (*) (X509_NAME *,char *,int)) d->cryptoLib->resolveFunction("X509_NAME_oneline");
      K_X509_get_subject_name = (X509_NAME * (*) (X509 *)) d->cryptoLib->resolveFunction("X509_get_subject_name");
      K_X509_get_issuer_name = (X509_NAME * (*) (X509 *)) d->cryptoLib->resolveFunction("X509_get_issuer_name");
      K_X509_STORE_add_lookup = (X509_LOOKUP *(*) (X509_STORE *, X509_LOOKUP_METHOD *)) d->cryptoLib->resolveFunction("X509_STORE_add_lookup");
      K_X509_LOOKUP_file = (X509_LOOKUP_METHOD *(*)(void)) d->cryptoLib->resolveFunction("X509_LOOKUP_file");
      K_X509_LOOKUP_free = (void (*)(X509_LOOKUP *)) d->cryptoLib->resolveFunction("X509_LOOKUP_free");
      K_X509_LOOKUP_ctrl = (int (*)(X509_LOOKUP *, int, const char *, long, char **)) d->cryptoLib->resolveFunction("X509_LOOKUP_ctrl");
      K_X509_STORE_CTX_init = (void (*)(X509_STORE_CTX *, X509_STORE *, X509 *, STACK_OF(X509) *)) d->cryptoLib->resolveFunction("X509_STORE_CTX_init");
      K_X509_dup = (X509* (*)(X509*)) d->cryptoLib->resolveFunction("X509_dup");
      K_BIO_s_mem = (BIO_METHOD *(*) (void)) d->cryptoLib->resolveFunction("BIO_s_mem");
      K_BIO_new = (BIO* (*)(BIO_METHOD *)) d->cryptoLib->resolveFunction("BIO_new");
      K_BIO_new_fp = (BIO* (*)(FILE*, int)) d->cryptoLib->resolveFunction("BIO_new_fp");
      K_BIO_new_mem_buf = (BIO* (*)(void *, int)) d->cryptoLib->resolveFunction("BIO_new_mem_buf");
      K_BIO_free = (int (*)(BIO*)) d->cryptoLib->resolveFunction("BIO_free");
      K_BIO_ctrl = (long (*) (BIO *,int,long,void *)) d->cryptoLib->resolveFunction("BIO_ctrl");
      K_BIO_write = (int (*) (BIO *b, const void *data, int len)) d->cryptoLib->resolveFunction("BIO_write");
      K_PEM_ASN1_write_bio = (int (*)(int (*)(), const char *,BIO*, char*, const EVP_CIPHER *, unsigned char *, int, pem_password_cb *, void *)) d->cryptoLib->resolveFunction("PEM_ASN1_write_bio");
      K_ASN1_item_i2d_fp = (int (*)(ASN1_ITEM *, FILE*, unsigned char *))
          d->cryptoLib->resolveFunction("ASN1_item_i2d_fp");
      K_NETSCAPE_X509_it = (ASN1_ITEM *) d->cryptoLib->resolveFunction("NETSCAPE_X509_it");
      K_X509_print_fp = (int (*)(FILE*, X509*)) d->cryptoLib->resolveFunction("X509_print_fp");
      K_i2d_PKCS12 = (int (*)(PKCS12*, unsigned char**)) d->cryptoLib->resolveFunction("i2d_PKCS12");
      K_i2d_PKCS12_fp = (int (*)(FILE *, PKCS12*)) d->cryptoLib->resolveFunction("i2d_PKCS12_fp");
      K_PKCS12_newpass = (int (*)(PKCS12*, char*, char*)) d->cryptoLib->resolveFunction("PKCS12_newpass");
      K_d2i_PKCS12_fp = (PKCS12* (*)(FILE*, PKCS12**)) d->cryptoLib->resolveFunction("d2i_PKCS12_fp");
      K_PKCS12_new = (PKCS12* (*)()) d->cryptoLib->resolveFunction("PKCS12_new");
      K_PKCS12_free = (void (*)(PKCS12 *)) d->cryptoLib->resolveFunction("PKCS12_free");
      K_PKCS12_parse = (int (*)(PKCS12*, const char *, EVP_PKEY**,
                X509**, STACK_OF(X509)**)) d->cryptoLib->resolveFunction("PKCS12_parse");
      K_EVP_PKEY_free = (void (*) (EVP_PKEY *)) d->cryptoLib->resolveFunction("EVP_PKEY_free");
      K_EVP_PKEY_new = (EVP_PKEY* (*)()) d->cryptoLib->resolveFunction("EVP_PKEY_new");
      K_X509_REQ_free = (void (*)(X509_REQ*)) d->cryptoLib->resolveFunction("X509_REQ_free");
      K_X509_REQ_new = (X509_REQ* (*)()) d->cryptoLib->resolveFunction("X509_REQ_new");
      K_X509_STORE_CTX_set_chain = (void (*)(X509_STORE_CTX *, STACK_OF(X509)*)) d->cryptoLib->resolveFunction("X509_STORE_CTX_set_chain");
      K_X509_STORE_CTX_set_purpose = (void (*)(X509_STORE_CTX *, int)) d->cryptoLib->resolveFunction("X509_STORE_CTX_set_purpose");
      K_sk_free = (void (*) (STACK *)) d->cryptoLib->resolveFunction("sk_free");
      K_sk_num = (int (*) (STACK *)) d->cryptoLib->resolveFunction("sk_num");
      K_sk_pop = (char* (*) (STACK *)) d->cryptoLib->resolveFunction("sk_pop");
      K_sk_value = (char* (*) (STACK *, int)) d->cryptoLib->resolveFunction("sk_value");
      K_sk_new = (STACK* (*) (int (*)())) d->cryptoLib->resolveFunction("sk_new");
      K_sk_push = (int (*) (STACK*, char*)) d->cryptoLib->resolveFunction("sk_push");
      K_sk_dup = (STACK* (*) (STACK *)) d->cryptoLib->resolveFunction("sk_dup");
      K_i2s_ASN1_INTEGER = (char *(*) (X509V3_EXT_METHOD *, ASN1_INTEGER *)) d->cryptoLib->resolveFunction("i2s_ASN1_INTEGER");
      K_X509_get_serialNumber = (ASN1_INTEGER * (*) (X509 *)) d->cryptoLib->resolveFunction("X509_get_serialNumber");
      K_X509_get_pubkey = (EVP_PKEY *(*)(X509 *)) d->cryptoLib->resolveFunction("X509_get_pubkey");
      K_i2d_PublicKey = (int (*)(EVP_PKEY *, unsigned char **)) d->cryptoLib->resolveFunction("i2d_PublicKey");
      K_X509_check_private_key = (int (*)(X509 *, EVP_PKEY *)) d->cryptoLib->resolveFunction("X509_check_private_key");
      K_BN_bn2hex = (char *(*)(const BIGNUM *)) d->cryptoLib->resolveFunction("BN_bn2hex");
      K_X509_digest = (int (*)(const X509 *,const EVP_MD *, unsigned char *, unsigned int *)) d->cryptoLib->resolveFunction("X509_digest");
      K_EVP_md5 = (EVP_MD *(*)()) d->cryptoLib->resolveFunction("EVP_md5");
      K_ASN1_INTEGER_free = (void (*)(ASN1_INTEGER *)) d->cryptoLib->resolveFunction("ASN1_INTEGER_free");
      K_OBJ_obj2nid = (int (*)(ASN1_OBJECT *)) d->cryptoLib->resolveFunction("OBJ_obj2nid");
      K_OBJ_nid2ln = (const char *(*)(int)) d->cryptoLib->resolveFunction("OBJ_nid2ln");
      K_X509_get_ext_count = (int (*)(X509*)) d->cryptoLib->resolveFunction("X509_get_ext_count");
      K_X509_get_ext_by_NID = (int (*)(X509*,int,int)) d->cryptoLib->resolveFunction("X509_get_ext_by_NID");
      K_X509_get_ext_by_OBJ = (int (*)(X509*,ASN1_OBJECT*,int)) d->cryptoLib->resolveFunction("X509_get_ext_by_OBJ");
      K_X509_get_ext = (X509_EXTENSION* (*)(X509*,int)) d->cryptoLib->resolveFunction("X509_get_ext");
      K_X509_delete_ext = (X509_EXTENSION* (*)(X509*,int)) d->cryptoLib->resolveFunction("X509_delete_ext");
      K_X509_add_ext = (int (*)(X509*,X509_EXTENSION*,int)) d->cryptoLib->resolveFunction("X509_add_ext");
      K_X509_get_ext_d2i = (void* (*)(X509*,int,int*,int*)) d->cryptoLib->resolveFunction("X509_get_ext_d2i");
      K_i2s_ASN1_OCTET_STRING = (char *(*)(X509V3_EXT_METHOD*,ASN1_OCTET_STRING*)) d->cryptoLib->resolveFunction("i2s_ASN1_OCTET_STRING");
      K_ASN1_BIT_STRING_get_bit = (int (*)(ASN1_BIT_STRING*,int)) d->cryptoLib->resolveFunction("ASN1_BIT_STRING_get_bit");
      K_PKCS7_new = (PKCS7 *(*)()) d->cryptoLib->resolveFunction("PKCS7_new");
      K_PKCS7_free = (void (*)(PKCS7*)) d->cryptoLib->resolveFunction("PKCS7_free");
      K_PKCS7_content_free = (void (*)(PKCS7*)) d->cryptoLib->resolveFunction("PKCS7_content_free");
      K_i2d_PKCS7 = (int (*)(PKCS7*, unsigned char**)) d->cryptoLib->resolveFunction("i2d_PKCS7");
      K_i2d_PKCS7_fp = (int (*)(FILE*,PKCS7*)) d->cryptoLib->resolveFunction("i2d_PKCS7_fp");
      K_i2d_PKCS7_bio = (int (*)(BIO *bp,PKCS7 *p7)) d->cryptoLib->resolveFunction("i2d_PKCS7_bio");
      K_d2i_PKCS7 = (PKCS7* (*)(PKCS7**,unsigned char**,long)) d->cryptoLib->resolveFunction("d2i_PKCS7");
      K_d2i_PKCS7_fp = (PKCS7 *(*)(FILE *,PKCS7**)) d->cryptoLib->resolveFunction("d2i_PKCS7_fp");
      K_d2i_PKCS7_bio = (PKCS7 *(*)(BIO *bp,PKCS7 **p7)) d->cryptoLib->resolveFunction("d2i_PKCS7_bio");
      K_PKCS7_dup = (PKCS7* (*)(PKCS7*)) d->cryptoLib->resolveFunction("PKCS7_dup");
      K_PKCS7_sign = (PKCS7 *(*)(X509*, EVP_PKEY*, STACK_OF(X509)*, BIO*, int)) d->cryptoLib->resolveFunction("PKCS7_sign");
      K_PKCS7_verify = (int (*)(PKCS7*,STACK_OF(X509)*,X509_STORE*,BIO*,BIO*,int)) d->cryptoLib->resolveFunction("PKCS7_verify");
      K_PKCS7_get0_signers = (STACK_OF(X509) *(*)(PKCS7 *, STACK_OF(X509) *, int)) d->cryptoLib->resolveFunction("PKCS7_get0_signers");
      K_PKCS7_encrypt = (PKCS7* (*)(STACK_OF(X509) *, BIO *, EVP_CIPHER *, int)) d->cryptoLib->resolveFunction("PKCS7_encrypt");
      K_PKCS7_decrypt = (int (*)(PKCS7 *, EVP_PKEY *, X509 *, BIO *, int)) d->cryptoLib->resolveFunction("PKCS7_decrypt");
      K_PEM_X509_INFO_read = (STACK_OF(X509_INFO) *(*)(FILE*, STACK_OF(X509_INFO)*, pem_password_cb*, void *)) d->cryptoLib->resolveFunction("PEM_X509_INFO_read");
      K_ASN1_d2i_fp = (char *(*)(char *(*)(),char *(*)(),FILE*,unsigned char**)) d->cryptoLib->resolveFunction("ASN1_d2i_fp");
      K_X509_new = (X509 *(*)()) d->cryptoLib->resolveFunction("X509_new");
      K_X509_PURPOSE_get_count = (int (*)()) d->cryptoLib->resolveFunction("X509_PURPOSE_get_count");
      K_X509_PURPOSE_get_id = (int (*)(X509_PURPOSE *)) d->cryptoLib->resolveFunction("X509_PURPOSE_get_id");
      K_X509_check_purpose = (int (*)(X509*,int,int)) d->cryptoLib->resolveFunction("X509_check_purpose");
      K_X509_PURPOSE_get0 = (X509_PURPOSE *(*)(int)) d->cryptoLib->resolveFunction("X509_PURPOSE_get0");
      K_EVP_PKEY_assign = (int (*)(EVP_PKEY*, int, char*)) d->cryptoLib->resolveFunction("EVP_PKEY_assign");
      K_X509_REQ_set_pubkey = (int (*)(X509_REQ*, EVP_PKEY*)) d->cryptoLib->resolveFunction("X509_REQ_set_pubkey");
      K_RSA_generate_key = (RSA* (*)(int, unsigned long, void (*)(int,int,void *), void *)) d->cryptoLib->resolveFunction("RSA_generate_key");
      K_i2d_X509_REQ_fp = (int (*)(FILE *, X509_REQ *)) d->cryptoLib->resolveFunction("i2d_X509_REQ_fp");
      K_ERR_clear_error = (void (*)()) d->cryptoLib->resolveFunction("ERR_clear_error");
      K_ERR_get_error = (unsigned long (*)()) d->cryptoLib->resolveFunction("ERR_get_error");
      K_ERR_print_errors_fp = (void (*)(FILE*)) d->cryptoLib->resolveFunction("ERR_print_errors_fp");
      K_X509_get1_email = (STACK *(*)(X509 *x)) d->cryptoLib->resolveFunction("X509_get1_email");
      K_X509_email_free = (void (*)(STACK *sk)) d->cryptoLib->resolveFunction("X509_email_free");
      K_EVP_des_ede3_cbc = (EVP_CIPHER *(*)()) d->cryptoLib->resolveFunction("EVP_des_ede3_cbc");
      K_EVP_des_cbc = (EVP_CIPHER *(*)()) d->cryptoLib->resolveFunction("EVP_des_cbc");
      K_EVP_rc2_cbc = (EVP_CIPHER *(*)()) d->cryptoLib->resolveFunction("EVP_rc2_cbc");
      K_EVP_rc2_64_cbc = (EVP_CIPHER *(*)()) d->cryptoLib->resolveFunction("EVP_rc2_64_cbc");
      K_EVP_rc2_40_cbc = (EVP_CIPHER *(*)()) d->cryptoLib->resolveFunction("EVP_rc2_40_cbc");
      K_i2d_PrivateKey_fp = (int (*)(FILE*,EVP_PKEY*)) d->cryptoLib->resolveFunction("i2d_PrivateKey_fp");
      K_i2d_PKCS8PrivateKey_fp = (int (*)(FILE*, EVP_PKEY*, const EVP_CIPHER*, char*, int, pem_password_cb*, void*)) d->cryptoLib->resolveFunction("i2d_PKCS8PrivateKey_fp");
      K_RSA_free = (void (*)(RSA*)) d->cryptoLib->resolveFunction("RSA_free");
      K_EVP_bf_cbc = (EVP_CIPHER *(*)()) d->cryptoLib->resolveFunction("EVP_bf_cbc");
      K_X509_REQ_sign = (int (*)(X509_REQ*, EVP_PKEY*, const EVP_MD*)) d->cryptoLib->resolveFunction("X509_REQ_sign");
      K_X509_NAME_add_entry_by_txt = (int (*)(X509_NAME*, char*, int, unsigned char*, int, int, int)) d->cryptoLib->resolveFunction("X509_NAME_add_entry_by_txt");
      K_X509_NAME_new = (X509_NAME *(*)()) d->cryptoLib->resolveFunction("X509_NAME_new");
      K_X509_REQ_set_subject_name = (int (*)(X509_REQ*,X509_NAME*)) d->cryptoLib->resolveFunction("X509_REQ_set_subject_name");
      K_ASN1_STRING_data = (unsigned char *(*)(ASN1_STRING*)) d->cryptoLib->resolveFunction("ASN1_STRING_data");
      K_ASN1_STRING_length = (int (*)(ASN1_STRING*)) d->cryptoLib->resolveFunction("ASN1_STRING_length");
#endif
   }

#ifdef Q_OS_WIN
    d->sslLib = new KLibrary("ssleay32.dll");
    if (!d->sslLib->load()) {
       delete d->sslLib;
       d->sslLib = 0;
    }
#elif defined(__OpenBSD__)
   {
   QString libname = findMostRecentLib("/usr/lib", "ssl");
   if (!libname.isNull()) {
         d->sslLib = new KLibrary(libname);
         d->sslLib->setLoadHints(QLibrary::ExportExternalSymbolsHint);
         if (!d->sslLib->load()) {
             delete d->sslLib;
             d->sslLib = 0;
         }
   }
   }
#else
   for (QStringList::const_iterator it = libpaths.constBegin();
                              it != libpaths.constEnd();
                              ++it) {
      for (QStringList::const_iterator shit = libnamess.constBegin();
                                 shit != libnamess.constEnd();
                                 ++shit) {
         QString alib = *it;
         if (!alib.isEmpty() && !alib.endsWith('/'))
            alib += '/';
         alib += *shit;
	     QString tmpStr(alib.toLatin1());
	     tmpStr.remove(QRegExp("\\(.*\\)"));
         if (!access(tmpStr.toLatin1(), R_OK)) {
         	d->sslLib = new KLibrary(alib);
            d->sslLib->setLoadHints(QLibrary::ExportExternalSymbolsHint);
         }
         if (d->sslLib && d->sslLib->load()) {
             break;
         }
         else {
             delete d->sslLib;
             d->sslLib = 0;
         }
      }
      if (d->sslLib) break;
   }
#endif

   if (d->sslLib) {
#if KSSL_HAVE_SSL
      // stand back from your monitor and look at this.  it's fun! :)
      K_SSL_connect = (int (*)(SSL *)) d->sslLib->resolveFunction("SSL_connect");
      K_SSL_accept = (int (*)(SSL *)) d->sslLib->resolveFunction("SSL_accept");
      K_SSL_read = (int (*)(SSL *, void *, int)) d->sslLib->resolveFunction("SSL_read");
      K_SSL_write = (int (*)(SSL *, const void *, int))
                            d->sslLib->resolveFunction("SSL_write");
      K_SSL_new = (SSL* (*)(SSL_CTX *)) d->sslLib->resolveFunction("SSL_new");
      K_SSL_free = (void (*)(SSL *)) d->sslLib->resolveFunction("SSL_free");
      K_SSL_shutdown = (int (*)(SSL *)) d->sslLib->resolveFunction("SSL_shutdown");
      K_SSL_CTX_new = (SSL_CTX* (*)(SSL_METHOD*)) d->sslLib->resolveFunction("SSL_CTX_new");
      K_SSL_CTX_free = (void (*)(SSL_CTX*)) d->sslLib->resolveFunction("SSL_CTX_free");
      K_SSL_set_fd = (int (*)(SSL *, int)) d->sslLib->resolveFunction("SSL_set_fd");
      K_SSL_pending = (int (*)(SSL *)) d->sslLib->resolveFunction("SSL_pending");
      K_SSL_CTX_set_cipher_list = (int (*)(SSL_CTX *, const char *))
                                  d->sslLib->resolveFunction("SSL_CTX_set_cipher_list");
      K_SSL_CTX_set_verify = (void (*)(SSL_CTX*, int, int (*)(int, X509_STORE_CTX*))) d->sslLib->resolveFunction("SSL_CTX_set_verify");
      K_SSL_use_certificate = (int (*)(SSL*, X509*))
                                  d->sslLib->resolveFunction("SSL_CTX_use_certificate");
      K_SSL_get_current_cipher = (SSL_CIPHER *(*)(SSL *))
                                  d->sslLib->resolveFunction("SSL_get_current_cipher");
      K_SSL_ctrl = (long (*)(SSL * ,int, long, char *))
                                  d->sslLib->resolveFunction("SSL_ctrl");
      K_TLSv1_client_method = (SSL_METHOD *(*)()) d->sslLib->resolveFunction("TLSv1_client_method");
      K_SSLv23_client_method = (SSL_METHOD *(*)()) d->sslLib->resolveFunction("SSLv23_client_method");
      K_SSL_get_peer_certificate = (X509 *(*)(SSL *)) d->sslLib->resolveFunction("SSL_get_peer_certificate");
      K_SSL_CIPHER_get_bits = (int (*)(SSL_CIPHER *,int *)) d->sslLib->resolveFunction("SSL_CIPHER_get_bits");
      K_SSL_CIPHER_get_version = (char * (*)(SSL_CIPHER *)) d->sslLib->resolveFunction("SSL_CIPHER_get_version");
      K_SSL_CIPHER_get_name = (const char * (*)(SSL_CIPHER *)) d->sslLib->resolveFunction("SSL_CIPHER_get_name");
      K_SSL_CIPHER_description = (char * (*)(SSL_CIPHER *, char *, int)) d->sslLib->resolveFunction("SSL_CIPHER_description");
      K_SSL_CTX_use_PrivateKey = (int (*)(SSL_CTX*, EVP_PKEY*)) d->sslLib->resolveFunction("SSL_CTX_use_PrivateKey");
      K_SSL_CTX_use_certificate = (int (*)(SSL_CTX*, X509*)) d->sslLib->resolveFunction("SSL_CTX_use_certificate");
      K_SSL_get_error = (int (*)(SSL*, int)) d->sslLib->resolveFunction("SSL_get_error");
      K_SSL_get_peer_cert_chain = (STACK_OF(X509)* (*)(SSL*)) d->sslLib->resolveFunction("SSL_get_peer_cert_chain");
      K_SSL_load_client_CA_file = (STACK_OF(X509_NAME)* (*)(const char *)) d->sslLib->resolveFunction("SSL_load_client_CA_file");
      K_SSL_peek = (int (*)(SSL*,void*,int)) d->sslLib->resolveFunction("SSL_peek");
      K_SSL_get1_session = (SSL_SESSION* (*)(SSL*)) d->sslLib->resolveFunction("SSL_get1_session");
      K_SSL_SESSION_free = (void (*)(SSL_SESSION*)) d->sslLib->resolveFunction("SSL_SESSION_free");
      K_SSL_set_session = (int (*)(SSL*,SSL_SESSION*)) d->sslLib->resolveFunction("SSL_set_session");
      K_d2i_SSL_SESSION = (SSL_SESSION* (*)(SSL_SESSION**,unsigned char**, long)) d->sslLib->resolveFunction("d2i_SSL_SESSION");
      K_i2d_SSL_SESSION = (int (*)(SSL_SESSION*,unsigned char**)) d->sslLib->resolveFunction("i2d_SSL_SESSION");
      K_SSL_get_ciphers = (STACK_OF(SSL_CIPHER) *(*)(const SSL*)) d->sslLib->resolveFunction("SSL_get_ciphers");
#endif


      // Initialize the library (once only!)
      KLibrary::void_function_ptr x;
      x = d->sslLib->resolveFunction("SSL_library_init");
      if (d->cryptoLib) {
         if (x) ((int (*)())x)();
         x = d->cryptoLib->resolveFunction("OpenSSL_add_all_algorithms");
         if (!x)
            x = d->cryptoLib->resolveFunction("OPENSSL_add_all_algorithms");
         if (x) {
           ((void (*)())x)();
         } else {
           x = d->cryptoLib->resolveFunction("OpenSSL_add_all_algorithms_conf");
           if (!x)
              x = d->cryptoLib->resolveFunction("OPENSSL_add_all_algorithms_conf");
           if (x) {
             ((void (*)())x)();
           } else {
             x = d->cryptoLib->resolveFunction("OpenSSL_add_all_algorithms_noconf");
             if (!x)
                x = d->cryptoLib->resolveFunction("OPENSSL_add_all_algorithms_noconf");
             if (x)
               ((void (*)())x)();
           }
         }
         x = d->cryptoLib->resolveFunction("OpenSSL_add_all_ciphers");
         if (!x)
            x = d->cryptoLib->resolveFunction("OPENSSL_add_all_ciphers");
         if (x) ((void (*)())x)();
         x = d->cryptoLib->resolveFunction("OpenSSL_add_all_digests");
         if (!x)
            x = d->cryptoLib->resolveFunction("OPENSSL_add_all_digests");
         if (x) ((void (*)())x)();
      }
   }

}

KOpenSSLProxy::~KOpenSSLProxy() {
   if (d->sslLib) {
	d->sslLib->unload();
   }
   if (d->cryptoLib) {
	d->cryptoLib->unload();
   }

   KOpenSSLProxyPrivate::sSelf = 0;
   delete d;
}


// FIXME: we should check "ok" and allow this to init the lib if !ok.

KOpenSSLProxy *KOpenSSLProxy::self() {
#if KSSL_HAVE_SSL
   if(!KOpenSSLProxyPrivate::sSelf) {
      KOpenSSLProxyPrivate::sSelf = new KOpenSSLProxy();
      qAddPostRoutine(KOpenSSLProxyPrivate::cleanupKOpenSSLProxy);
   }
#endif
   return KOpenSSLProxyPrivate::sSelf;
}







#if KSSL_HAVE_SSL



int KOpenSSLProxy::SSL_connect(SSL *ssl) {
   if (K_SSL_connect) return (K_SSL_connect)(ssl);
   return -1;
}


int KOpenSSLProxy::SSL_accept(SSL *ssl) {
   if (K_SSL_accept) return (K_SSL_accept)(ssl);
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
   return 0L;
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
   return 0L;
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


int KOpenSSLProxy::SSL_use_certificate(SSL *ssl, X509 *x) {
   if (K_SSL_use_certificate) return (K_SSL_use_certificate)(ssl, x);
   return -1;
}


SSL_CIPHER *KOpenSSLProxy::SSL_get_current_cipher(SSL *ssl) {
   if (K_SSL_get_current_cipher) return (K_SSL_get_current_cipher)(ssl);
   return 0L;
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
   return 0L;
}


SSL_METHOD *KOpenSSLProxy::SSLv23_client_method() {
   if (K_SSLv23_client_method) return (K_SSLv23_client_method)();
   return 0L;
}


X509 *KOpenSSLProxy::SSL_get_peer_certificate(SSL *s) {
   if (K_SSL_get_peer_certificate) return (K_SSL_get_peer_certificate)(s);
   return 0L;
}


int KOpenSSLProxy::SSL_CIPHER_get_bits(SSL_CIPHER *c,int *alg_bits) {
   if (K_SSL_CIPHER_get_bits) return (K_SSL_CIPHER_get_bits)(c, alg_bits);
   return -1;
}


char * KOpenSSLProxy::SSL_CIPHER_get_version(SSL_CIPHER *c) {
   if (K_SSL_CIPHER_get_version) return (K_SSL_CIPHER_get_version)(c);
   return 0L;
}


const char * KOpenSSLProxy::SSL_CIPHER_get_name(SSL_CIPHER *c) {
   if (K_SSL_CIPHER_get_name) return (K_SSL_CIPHER_get_name)(c);
   return 0L;
}


char * KOpenSSLProxy::SSL_CIPHER_description(SSL_CIPHER *c,char *buf,int size) {
   if (K_SSL_CIPHER_description) return (K_SSL_CIPHER_description)(c,buf,size);
   return 0L;
}


X509 * KOpenSSLProxy::d2i_X509(X509 **a,unsigned char **pp,long length) {
   if (K_d2i_X509) return (K_d2i_X509)(a,pp,length);
   return 0L;
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
   return 0L;
}


void KOpenSSLProxy::X509_STORE_free(X509_STORE *v) {
   if (K_X509_STORE_free) (K_X509_STORE_free)(v);
}


X509_STORE_CTX *KOpenSSLProxy::X509_STORE_CTX_new(void) {
   if (K_X509_STORE_CTX_new) return (K_X509_STORE_CTX_new)();
   return 0L;
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
   return 0L;
}


X509_NAME *KOpenSSLProxy::X509_get_subject_name(X509 *a) {
   if (K_X509_get_subject_name) return (K_X509_get_subject_name)(a);
   return 0L;
}


X509_NAME *KOpenSSLProxy::X509_get_issuer_name(X509 *a) {
   if (K_X509_get_issuer_name) return (K_X509_get_issuer_name)(a);
   return 0L;
}


X509_LOOKUP *KOpenSSLProxy::X509_STORE_add_lookup(X509_STORE *v, X509_LOOKUP_METHOD *m) {
   if (K_X509_STORE_add_lookup) return (K_X509_STORE_add_lookup)(v,m);
   return 0L;
}


X509_LOOKUP_METHOD *KOpenSSLProxy::X509_LOOKUP_file(void) {
   if (K_X509_LOOKUP_file) return (K_X509_LOOKUP_file)();
   return 0L;
}


void KOpenSSLProxy::X509_LOOKUP_free(X509_LOOKUP *x) {
   if (K_X509_LOOKUP_free) (K_X509_LOOKUP_free)(x);
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
   return 0L;
}


BIO *KOpenSSLProxy::BIO_new(BIO_METHOD *type) {
    if (K_BIO_new) return (K_BIO_new)(type);
    else return 0L;
}


BIO_METHOD *KOpenSSLProxy::BIO_s_mem(void) {
    if (K_BIO_s_mem) return (K_BIO_s_mem)();
    else return 0L;
}


BIO *KOpenSSLProxy::BIO_new_fp(FILE *stream, int close_flag) {
   if (K_BIO_new_fp) return (K_BIO_new_fp)(stream, close_flag);
   return 0L;
}


BIO *KOpenSSLProxy::BIO_new_mem_buf(void *buf, int len) {
    if (K_BIO_new_mem_buf) return (K_BIO_new_mem_buf)(buf,len);
    else return 0L;
}


int KOpenSSLProxy::BIO_free(BIO *a) {
   if (K_BIO_free) return (K_BIO_free)(a);
   return -1;
}


long KOpenSSLProxy::BIO_ctrl(BIO *bp,int cmd,long larg,void *parg) {
    if (K_BIO_ctrl) return (K_BIO_ctrl)(bp,cmd,larg,parg);
    else return 0; // failure return for BIO_ctrl is quite individual, maybe we should abort() instead
}


int KOpenSSLProxy::BIO_write(BIO *b, const void *data, int len) {
    if (K_BIO_write) return (K_BIO_write)(b, data, len);
    else return -1;
}


int KOpenSSLProxy::PEM_write_bio_X509(BIO *bp, X509 *x) {
   if (K_PEM_ASN1_write_bio) return (K_PEM_ASN1_write_bio) ((int (*)())K_i2d_X509, PEM_STRING_X509, bp, (char *)x, 0L, 0L, 0, 0L, 0L);
   else return -1;
}

int KOpenSSLProxy::ASN1_item_i2d_fp(FILE *out,unsigned char *x) {
   if (K_ASN1_item_i2d_fp && K_NETSCAPE_X509_it)
        return (K_ASN1_item_i2d_fp)(K_NETSCAPE_X509_it, out, x);
   else return -1;
}


int KOpenSSLProxy::X509_print(FILE *fp, X509 *x) {
   if (K_X509_print_fp) return (K_X509_print_fp)(fp, x);
   return -1;
}


PKCS12 *KOpenSSLProxy::d2i_PKCS12_fp(FILE *fp, PKCS12 **p12) {
   if (K_d2i_PKCS12_fp) return (K_d2i_PKCS12_fp)(fp, p12);
   else return 0L;
}


int KOpenSSLProxy::PKCS12_newpass(PKCS12 *p12, char *oldpass, char *newpass) {
   if (K_PKCS12_newpass) return (K_PKCS12_newpass)(p12, oldpass, newpass);
   else return -1;
}


int KOpenSSLProxy::i2d_PKCS12(PKCS12 *p12, unsigned char **p) {
   if (K_i2d_PKCS12) return (K_i2d_PKCS12)(p12, p);
   else return -1;
}


int KOpenSSLProxy::i2d_PKCS12_fp(FILE *fp, PKCS12 *p12) {
   if (K_i2d_PKCS12_fp) return (K_i2d_PKCS12_fp)(fp, p12);
   else return -1;
}


PKCS12 *KOpenSSLProxy::PKCS12_new(void) {
   if (K_PKCS12_new) return (K_PKCS12_new)();
   else return 0L;
}


void KOpenSSLProxy::PKCS12_free(PKCS12 *a) {
   if (K_PKCS12_free) (K_PKCS12_free)(a);
}


int KOpenSSLProxy::PKCS12_parse(PKCS12 *p12, const char *pass, EVP_PKEY **pkey,
                    X509 **cert, STACK_OF(X509) **ca) {
   if (K_PKCS12_parse) return (K_PKCS12_parse) (p12, pass, pkey, cert, ca);
   else return -1;
}


void KOpenSSLProxy::EVP_PKEY_free(EVP_PKEY *x) {
   if (K_EVP_PKEY_free) (K_EVP_PKEY_free)(x);
}


EVP_PKEY* KOpenSSLProxy::EVP_PKEY_new() {
   if (K_EVP_PKEY_new) return (K_EVP_PKEY_new)();
   else return 0L;
}


void KOpenSSLProxy::X509_REQ_free(X509_REQ *x) {
   if (K_X509_REQ_free) (K_X509_REQ_free)(x);
}


X509_REQ* KOpenSSLProxy::X509_REQ_new() {
   if (K_X509_REQ_new) return (K_X509_REQ_new)();
   else return 0L;
}


int KOpenSSLProxy::SSL_CTX_use_PrivateKey(SSL_CTX *ctx, EVP_PKEY *pkey) {
   if (K_SSL_CTX_use_PrivateKey) return (K_SSL_CTX_use_PrivateKey)(ctx,pkey);
   else return -1;
}


int KOpenSSLProxy::SSL_CTX_use_certificate(SSL_CTX *ctx, X509 *x) {
   if (K_SSL_CTX_use_certificate) return (K_SSL_CTX_use_certificate)(ctx,x);
   else return -1;
}


int KOpenSSLProxy::SSL_get_error(SSL *ssl, int rc) {
   if (K_SSL_get_error) return (K_SSL_get_error)(ssl,rc);
   else return -1;
}


STACK_OF(X509) *KOpenSSLProxy::SSL_get_peer_cert_chain(SSL *s) {
   if (K_SSL_get_peer_cert_chain) return (K_SSL_get_peer_cert_chain)(s);
   else return 0L;
}


void KOpenSSLProxy::sk_free(STACK *s) {
   if (K_sk_free) (K_sk_free)(s);
}


int KOpenSSLProxy::sk_num(STACK *s) {
   if (K_sk_num) return (K_sk_num)(s);
   else return -1;
}


char *KOpenSSLProxy::sk_pop(STACK *s) {
   if (K_sk_pop) return (K_sk_pop)(s);
   else return 0L;
}


char *KOpenSSLProxy::sk_value(STACK *s, int n) {
   if (K_sk_value) return (K_sk_value)(s, n);
   else return 0L;
}


void KOpenSSLProxy::X509_STORE_CTX_set_chain(X509_STORE_CTX *v, STACK_OF(X509)* x) {
   if (K_X509_STORE_CTX_set_chain) (K_X509_STORE_CTX_set_chain)(v,x);
}

void KOpenSSLProxy::X509_STORE_CTX_set_purpose(X509_STORE_CTX *v, int purpose) {
   if (K_X509_STORE_CTX_set_purpose) (K_X509_STORE_CTX_set_purpose)(v,purpose);
}


STACK* KOpenSSLProxy::sk_dup(STACK *s) {
   if (K_sk_dup) return (K_sk_dup)(s);
   else return 0L;
}


STACK* KOpenSSLProxy::sk_new(int (*cmp)()) {
   if (K_sk_new) return (K_sk_new)(cmp);
   else return 0L;
}


int KOpenSSLProxy::sk_push(STACK* s, char* d) {
   if (K_sk_push) return (K_sk_push)(s,d);
   else return -1;
}


char *KOpenSSLProxy::i2s_ASN1_INTEGER(X509V3_EXT_METHOD *meth, ASN1_INTEGER *aint) {
   if (K_i2s_ASN1_INTEGER) return (K_i2s_ASN1_INTEGER)(meth, aint);
   else return 0L;
}


ASN1_INTEGER *KOpenSSLProxy::X509_get_serialNumber(X509 *x) {
   if (K_X509_get_serialNumber) return (K_X509_get_serialNumber)(x);
   else return 0L;
}


EVP_PKEY *KOpenSSLProxy::X509_get_pubkey(X509 *x) {
   if (K_X509_get_pubkey) return (K_X509_get_pubkey)(x);
   else return 0L;
}


int KOpenSSLProxy::i2d_PublicKey(EVP_PKEY *a, unsigned char **pp) {
   if (K_i2d_PublicKey) return (K_i2d_PublicKey)(a,pp);
   else return 0;
}


int KOpenSSLProxy::X509_check_private_key(X509 *x, EVP_PKEY *p) {
   if (K_X509_check_private_key) return (K_X509_check_private_key)(x,p);
   return -1;
}


char *KOpenSSLProxy::BN_bn2hex(const BIGNUM *a) {
   if (K_BN_bn2hex) return (K_BN_bn2hex)(a);
   else return 0L;
}


int KOpenSSLProxy::X509_digest(const X509 *x,const EVP_MD *t, unsigned char *md, unsigned int *len) {
   if (K_X509_digest) return (K_X509_digest)(x, t, md, len);
   else return -1;
}


EVP_MD *KOpenSSLProxy::EVP_md5() {
   if (K_EVP_md5) return (K_EVP_md5)();
   return 0L;
}


void KOpenSSLProxy::ASN1_INTEGER_free(ASN1_INTEGER *a) {
   if (K_ASN1_INTEGER_free) (K_ASN1_INTEGER_free)(a);
}


int KOpenSSLProxy::OBJ_obj2nid(ASN1_OBJECT *o) {
   if (K_OBJ_obj2nid) return (K_OBJ_obj2nid)(o);
   else return -1;
}


const char * KOpenSSLProxy::OBJ_nid2ln(int n) {
   if (K_OBJ_nid2ln) return (K_OBJ_nid2ln)(n);
   else return 0L;
}


int KOpenSSLProxy::X509_get_ext_count(X509 *x) {
   if (K_X509_get_ext_count) return (K_X509_get_ext_count)(x);
   else return -1;
}


int KOpenSSLProxy::X509_get_ext_by_NID(X509 *x, int nid, int lastpos) {
   if (K_X509_get_ext_by_NID) return (K_X509_get_ext_by_NID)(x,nid,lastpos);
   else return -1;
}


int KOpenSSLProxy::X509_get_ext_by_OBJ(X509 *x,ASN1_OBJECT *obj,int lastpos) {
   if (K_X509_get_ext_by_OBJ) return (K_X509_get_ext_by_OBJ)(x,obj,lastpos);
   else return -1;
}


X509_EXTENSION *KOpenSSLProxy::X509_get_ext(X509 *x, int loc) {
   if (K_X509_get_ext) return (K_X509_get_ext)(x,loc);
   else return 0L;
}


X509_EXTENSION *KOpenSSLProxy::X509_delete_ext(X509 *x, int loc) {
   if (K_X509_delete_ext) return (K_X509_delete_ext)(x,loc);
   else return 0L;
}


int KOpenSSLProxy::X509_add_ext(X509 *x, X509_EXTENSION *ex, int loc) {
   if (K_X509_add_ext) return (K_X509_add_ext)(x,ex,loc);
   else return -1;
}


void *KOpenSSLProxy::X509_get_ext_d2i(X509 *x, int nid, int *crit, int *idx) {
   if (K_X509_get_ext_d2i) return (K_X509_get_ext_d2i)(x,nid,crit,idx);
   else return 0L;
}


char *KOpenSSLProxy::i2s_ASN1_OCTET_STRING(X509V3_EXT_METHOD *method, ASN1_OCTET_STRING *ia5) {
   if (K_i2s_ASN1_OCTET_STRING) return (K_i2s_ASN1_OCTET_STRING)(method,ia5);
   else return 0L;
}


int KOpenSSLProxy::ASN1_BIT_STRING_get_bit(ASN1_BIT_STRING *a, int n) {
   if (K_ASN1_BIT_STRING_get_bit) return (K_ASN1_BIT_STRING_get_bit)(a,n);
   else return -1;
}


PKCS7 *KOpenSSLProxy::PKCS7_new(void) {
   if (K_PKCS7_new) return (K_PKCS7_new)();
   else return 0L;
}


void KOpenSSLProxy::PKCS7_free(PKCS7 *a) {
   if (K_PKCS7_free) (K_PKCS7_free)(a);
}


void KOpenSSLProxy::PKCS7_content_free(PKCS7 *a) {
   if (K_PKCS7_content_free) (K_PKCS7_content_free)(a);
}


int KOpenSSLProxy::i2d_PKCS7(PKCS7 *a, unsigned char **pp) {
   if (K_i2d_PKCS7) return (K_i2d_PKCS7)(a,pp);
   else return -1;
}


PKCS7 *KOpenSSLProxy::d2i_PKCS7(PKCS7 **a, unsigned char **pp,long length) {
   if (K_d2i_PKCS7) return (K_d2i_PKCS7)(a,pp,length);
   else return 0L;
}


int KOpenSSLProxy::i2d_PKCS7_fp(FILE *fp,PKCS7 *p7) {
   if (K_i2d_PKCS7_fp) return (K_i2d_PKCS7_fp)(fp,p7);
   else return -1;
}


PKCS7 *KOpenSSLProxy::d2i_PKCS7_fp(FILE *fp,PKCS7 **p7) {
   if (K_d2i_PKCS7_fp) return (K_d2i_PKCS7_fp)(fp,p7);
   else return 0L;
}


int KOpenSSLProxy::i2d_PKCS7_bio(BIO *bp,PKCS7 *p7) {
    if (K_i2d_PKCS7_bio) return (K_i2d_PKCS7_bio)(bp, p7);
    else return -1;
}


PKCS7 *KOpenSSLProxy::d2i_PKCS7_bio(BIO *bp,PKCS7 **p7) {
    if (K_d2i_PKCS7_bio) return (K_d2i_PKCS7_bio)(bp, p7);
    else return 0L;
}


PKCS7 *KOpenSSLProxy::PKCS7_dup(PKCS7 *p7) {
   if (K_PKCS7_dup) return (K_PKCS7_dup)(p7);
   else return 0L;
}


PKCS7 *KOpenSSLProxy::PKCS7_sign(X509 *signcert, EVP_PKEY *pkey, STACK_OF(X509) *certs,
				 BIO *data, int flags) {
    if (K_PKCS7_sign) return (K_PKCS7_sign)(signcert,pkey,certs,data,flags);
    else return 0L;
}


int KOpenSSLProxy::PKCS7_verify(PKCS7* p, STACK_OF(X509)* st, X509_STORE* s, BIO* in, BIO *out, int flags) {
   if (K_PKCS7_verify) return (K_PKCS7_verify)(p,st,s,in,out,flags);
   else return 0;
}


STACK_OF(X509) *KOpenSSLProxy::PKCS7_get0_signers(PKCS7 *p7, STACK_OF(X509) *certs, int flags) {
    if (K_PKCS7_get0_signers) return (K_PKCS7_get0_signers)(p7,certs,flags);
    else return 0L;
}


PKCS7 *KOpenSSLProxy::PKCS7_encrypt(STACK_OF(X509) *certs, BIO *in, EVP_CIPHER *cipher,
				    int flags) {
    if (K_PKCS7_encrypt) return (K_PKCS7_encrypt)(certs,in,cipher,flags);
    else return 0L;
}


int KOpenSSLProxy::PKCS7_decrypt(PKCS7 *p7, EVP_PKEY *pkey, X509 *cert, BIO *data, int flags) {
    if (K_PKCS7_decrypt) return (K_PKCS7_decrypt)(p7,pkey,cert,data,flags);
    else return 0;
}


STACK_OF(X509_NAME) *KOpenSSLProxy::SSL_load_client_CA_file(const char *file) {
   if (K_SSL_load_client_CA_file) return (K_SSL_load_client_CA_file)(file);
   else return 0L;
}


STACK_OF(X509_INFO) *KOpenSSLProxy::PEM_X509_INFO_read(FILE *fp, STACK_OF(X509_INFO) *sk, pem_password_cb *cb, void *u) {
   if (K_PEM_X509_INFO_read) return (K_PEM_X509_INFO_read)(fp,sk,cb,u);
   else return 0L;
}


X509 *KOpenSSLProxy::X509_d2i_fp(FILE *out, X509** buf) {
   if (K_ASN1_d2i_fp) return reinterpret_cast<X509 *>((K_ASN1_d2i_fp)(reinterpret_cast<char *(*)()>(K_X509_new), reinterpret_cast<char *(*)()>(K_d2i_X509), out, reinterpret_cast<unsigned char **>(buf)));
   else return 0L;
}


int KOpenSSLProxy::SSL_peek(SSL *ssl,void *buf,int num) {
   if (K_SSL_peek) return (K_SSL_peek)(ssl,buf,num);
   else return -1;
}


const char *KOpenSSLProxy::RAND_file_name(char *buf, size_t num) {
   if (K_RAND_file_name) return (K_RAND_file_name)(buf, num);
   else return 0L;
}


int KOpenSSLProxy::RAND_load_file(const char *filename, long max_bytes) {
   if (K_RAND_load_file) return (K_RAND_load_file)(filename, max_bytes);
   else return -1;
}


int KOpenSSLProxy::RAND_write_file(const char *filename) {
   if (K_RAND_write_file) return (K_RAND_write_file)(filename);
   else return -1;
}


int KOpenSSLProxy::X509_PURPOSE_get_count() {
   if (K_X509_PURPOSE_get_count) return (K_X509_PURPOSE_get_count)();
   else return -1;
}


int KOpenSSLProxy::X509_PURPOSE_get_id(X509_PURPOSE *p) {
   if (K_X509_PURPOSE_get_id) return (K_X509_PURPOSE_get_id)(p);
   else return -1;
}


int KOpenSSLProxy::X509_check_purpose(X509 *x, int id, int ca) {
   if (K_X509_check_purpose) return (K_X509_check_purpose)(x, id, ca);
   else return -1;
}


X509_PURPOSE *KOpenSSLProxy::X509_PURPOSE_get0(int idx) {
   if (K_X509_PURPOSE_get0) return (K_X509_PURPOSE_get0)(idx);
   else return 0L;
}


int KOpenSSLProxy::EVP_PKEY_assign(EVP_PKEY *pkey, int type, char *key) {
   if (K_EVP_PKEY_assign) return (K_EVP_PKEY_assign)(pkey, type, key);
   else return -1;
}


int KOpenSSLProxy::X509_REQ_set_pubkey(X509_REQ *x, EVP_PKEY *pkey) {
   if (K_X509_REQ_set_pubkey) return (K_X509_REQ_set_pubkey)(x, pkey);
   else return -1;
}


RSA* KOpenSSLProxy::RSA_generate_key(int bits, unsigned long e, void
                        (*callback)(int,int,void *), void *cb_arg) {
   if (K_RSA_generate_key) return (K_RSA_generate_key)(bits, e, callback, cb_arg);
   else return 0L;
}

STACK *KOpenSSLProxy::X509_get1_email(X509 *x) {
   if (K_X509_get1_email) return (K_X509_get1_email)(x);
   else return 0L;
}

void KOpenSSLProxy::X509_email_free(STACK *sk) {
   if (K_X509_email_free) (K_X509_email_free)(sk);
}

EVP_CIPHER *KOpenSSLProxy::EVP_des_ede3_cbc() {
    if (K_EVP_des_ede3_cbc) return (K_EVP_des_ede3_cbc)();
    else return 0L;
}

EVP_CIPHER *KOpenSSLProxy::EVP_des_cbc() {
    if (K_EVP_des_cbc) return (K_EVP_des_cbc)();
    else return 0L;
}

EVP_CIPHER *KOpenSSLProxy::EVP_rc2_cbc() {
    if (K_EVP_rc2_cbc) return (K_EVP_rc2_cbc)();
    else return 0L;
}

EVP_CIPHER *KOpenSSLProxy::EVP_rc2_64_cbc() {
    if (K_EVP_rc2_64_cbc) return (K_EVP_rc2_64_cbc)();
    else return 0L;
}

EVP_CIPHER *KOpenSSLProxy::EVP_rc2_40_cbc() {
    if (K_EVP_rc2_40_cbc) return (K_EVP_rc2_40_cbc)();
    else return 0L;
}

int KOpenSSLProxy::i2d_X509_REQ_fp(FILE *fp, X509_REQ *x) {
   if (K_i2d_X509_REQ_fp) return (K_i2d_X509_REQ_fp)(fp,x);
   else return -1;
}


void KOpenSSLProxy::ERR_clear_error() {
   if (K_ERR_clear_error) (K_ERR_clear_error)();
}


unsigned long KOpenSSLProxy::ERR_get_error() {
    if (K_ERR_get_error) return (K_ERR_get_error)();
    else return 0xffffffff;
}


void KOpenSSLProxy::ERR_print_errors_fp(FILE* fp) {
   if (K_ERR_print_errors_fp) (K_ERR_print_errors_fp)(fp);
}


SSL_SESSION *KOpenSSLProxy::SSL_get1_session(SSL *ssl) {
   if (K_SSL_get1_session) return (K_SSL_get1_session)(ssl);
   else return 0L;
}


void KOpenSSLProxy::SSL_SESSION_free(SSL_SESSION *session) {
   if (K_SSL_SESSION_free) (K_SSL_SESSION_free)(session);
}


int KOpenSSLProxy::SSL_set_session(SSL *ssl, SSL_SESSION *session) {
   if (K_SSL_set_session) return (K_SSL_set_session)(ssl, session);
   else return -1;
}


SSL_SESSION *KOpenSSLProxy::d2i_SSL_SESSION(SSL_SESSION **a, unsigned char **pp, long length) {
   if (K_d2i_SSL_SESSION) return (K_d2i_SSL_SESSION)(a, pp, length);
   else return 0L;
}


int KOpenSSLProxy::i2d_SSL_SESSION(SSL_SESSION *in, unsigned char **pp) {
   if (K_i2d_SSL_SESSION) return (K_i2d_SSL_SESSION)(in, pp);
   else return -1;
}


int KOpenSSLProxy::i2d_PrivateKey_fp(FILE *fp, EVP_PKEY *p) {
   if (K_i2d_PrivateKey_fp) return (K_i2d_PrivateKey_fp)(fp, p);
   else return -1;
}


int KOpenSSLProxy::i2d_PKCS8PrivateKey_fp(FILE *fp, EVP_PKEY *p, const EVP_CIPHER *c, char *k, int klen, pem_password_cb *cb, void *u) {
   if (K_i2d_PKCS8PrivateKey_fp) return (K_i2d_PKCS8PrivateKey_fp)(fp, p, c, k, klen, cb, u);
   else return -1;
}


void KOpenSSLProxy::RSA_free(RSA *rsa) {
   if (K_RSA_free) (K_RSA_free)(rsa);
}


EVP_CIPHER *KOpenSSLProxy::EVP_bf_cbc() {
   if (K_EVP_bf_cbc) return (K_EVP_bf_cbc)();
   return 0L;
}


int KOpenSSLProxy::X509_REQ_sign(X509_REQ *x, EVP_PKEY *pkey, const EVP_MD *md) {
   if (K_X509_REQ_sign) return (K_X509_REQ_sign)(x, pkey, md);
   return -1;
}


int KOpenSSLProxy::X509_NAME_add_entry_by_txt(X509_NAME *name, char *field,
                   int type, unsigned char *bytes, int len, int loc, int set) {
   if (K_X509_NAME_add_entry_by_txt) return (K_X509_NAME_add_entry_by_txt)(name, field, type, bytes, len, loc, set);
   return -1;
}


X509_NAME *KOpenSSLProxy::X509_NAME_new() {
   if (K_X509_NAME_new) return (K_X509_NAME_new)();
   return 0L;
}


int KOpenSSLProxy::X509_REQ_set_subject_name(X509_REQ *req,X509_NAME *name) {
   if (K_X509_REQ_set_subject_name) return (K_X509_REQ_set_subject_name)(req, name);
   return -1;
}


unsigned char *KOpenSSLProxy::ASN1_STRING_data(ASN1_STRING *x) {
   if (K_ASN1_STRING_data) return (K_ASN1_STRING_data)(x);
   return 0L;
}


int KOpenSSLProxy::ASN1_STRING_length(ASN1_STRING *x) {
   if (K_ASN1_STRING_length) return (K_ASN1_STRING_length)(x);
   return 0L;
}


STACK_OF(SSL_CIPHER) *KOpenSSLProxy::SSL_get_ciphers(const SSL* ssl) {
  if (K_SSL_get_ciphers) return (K_SSL_get_ciphers)(ssl);
  return 0L;
}

#endif

