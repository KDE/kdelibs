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
#include <kstaticdeleter.h>
#include <qregexp.h>


#include <stdio.h>
#include <unistd.h>
#include "kopenssl.h"

extern "C" {
#ifdef HAVE_SSL
static int (*K_SSL_connect)     (SSL *) = NULL;
static int (*K_SSL_accept)      (SSL *) = NULL;
static int (*K_SSL_read)        (SSL *, void *, int) = NULL;
static int (*K_SSL_write)       (SSL *, const void *, int) = NULL;
static SSL *(*K_SSL_new)        (SSL_CTX *) = NULL;
static void (*K_SSL_free)       (SSL *) = NULL;
static int (*K_SSL_shutdown)    (SSL *) = NULL;
static SSL_CTX *(*K_SSL_CTX_new)(SSL_METHOD *) = NULL;
static void (*K_SSL_CTX_free)   (SSL_CTX *) = NULL;
static int (*K_SSL_set_fd)      (SSL *, int) = NULL;
static int (*K_SSL_pending)     (SSL *) = NULL;
static int (*K_SSL_peek)        (SSL *, void *, int) = NULL;
static int (*K_SSL_CTX_set_cipher_list)(SSL_CTX *, const char *) = NULL;
static void (*K_SSL_CTX_set_verify)(SSL_CTX *, int,
                         int (*)(int, X509_STORE_CTX *)) = NULL;
static int (*K_SSL_use_certificate)(SSL *, X509 *) = NULL;
static SSL_CIPHER *(*K_SSL_get_current_cipher)(SSL *) = NULL;
static long (*K_SSL_ctrl)      (SSL *,int, long, char *) = NULL;
static int (*K_RAND_egd)        (const char *) = NULL;
static const char* (*K_RAND_file_name) (char *, size_t) = NULL;
static int (*K_RAND_load_file)  (const char *, long) = NULL;
static int (*K_RAND_write_file) (const char *) = NULL;
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
static void (*K_X509_LOOKUP_free)(X509_LOOKUP *) = NULL;
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
static int (*K_X509_print_fp)  (FILE *, X509*) = NULL;
static int (*K_i2d_PKCS12)  (PKCS12*, unsigned char**) = NULL;
static int (*K_i2d_PKCS12_fp)  (FILE *, PKCS12*) = NULL;
static int (*K_PKCS12_newpass) (PKCS12*, char*, char*) = NULL;
static PKCS12* (*K_d2i_PKCS12_fp) (FILE*, PKCS12**) = NULL;
static PKCS12* (*K_PKCS12_new) (void) = NULL;
static void (*K_PKCS12_free) (PKCS12 *) = NULL;
static int (*K_PKCS12_parse) (PKCS12*, const char *, EVP_PKEY**, 
                                             X509**, STACK_OF(X509)**) = NULL;
static void (*K_EVP_PKEY_free) (EVP_PKEY *) = NULL;
static EVP_PKEY* (*K_EVP_PKEY_new) () = NULL;
static void (*K_X509_REQ_free) (X509_REQ *) = NULL;
static X509_REQ* (*K_X509_REQ_new) () = NULL;
static int (*K_SSL_CTX_use_PrivateKey) (SSL_CTX*, EVP_PKEY*) = NULL;
static int (*K_SSL_CTX_use_certificate) (SSL_CTX*, X509*) = NULL;
static int (*K_SSL_get_error) (SSL*, int) = NULL;
static STACK_OF(X509)* (*K_SSL_get_peer_cert_chain) (SSL*) = NULL;
static void (*K_X509_STORE_CTX_set_chain) (X509_STORE_CTX *, STACK_OF(X509)*) = NULL;
static void (*K_X509_STORE_CTX_set_purpose) (X509_STORE_CTX *, int) = NULL;
static void (*K_sk_free) (STACK*) = NULL;
static int (*K_sk_num) (STACK*) = NULL;
static char* (*K_sk_pop) (STACK*) = NULL;
static char* (*K_sk_value) (STACK*, int) = NULL;
static STACK* (*K_sk_new) (int (*)()) = NULL;
static int (*K_sk_push) (STACK*, char*) = NULL;
static STACK* (*K_sk_dup) (STACK *) = NULL;
static char * (*K_i2s_ASN1_INTEGER) (X509V3_EXT_METHOD *, ASN1_INTEGER *) =NULL;
static ASN1_INTEGER * (*K_X509_get_serialNumber) (X509 *) = NULL;
static EVP_PKEY *(*K_X509_get_pubkey)(X509 *) = NULL;
static int (*K_i2d_PublicKey)(EVP_PKEY *, unsigned char **) = NULL;
static int (*K_X509_check_private_key)(X509 *, EVP_PKEY *) = NULL;
static char * (*K_BN_bn2hex)(const BIGNUM *) = NULL;
static int (*K_X509_digest)(const X509 *,const EVP_MD *, unsigned char *, unsigned int *) = NULL;
static EVP_MD* (*K_EVP_md5)() = NULL;
static void (*K_ASN1_INTEGER_free)(ASN1_INTEGER *) = NULL;
static int (*K_OBJ_obj2nid)(ASN1_OBJECT *) = NULL;
static const char * (*K_OBJ_nid2ln)(int) = NULL;
static int (*K_X509_get_ext_count)(X509*) = NULL;
static int (*K_X509_get_ext_by_NID)(X509*, int, int) = NULL;
static int (*K_X509_get_ext_by_OBJ)(X509*,ASN1_OBJECT*,int) = NULL;
static X509_EXTENSION *(*K_X509_get_ext)(X509*, int loc) = NULL;
static X509_EXTENSION *(*K_X509_delete_ext)(X509*, int) = NULL;
static int (*K_X509_add_ext)(X509*, X509_EXTENSION*, int) = NULL;
static void *(*K_X509_get_ext_d2i)(X509*, int, int*, int*) = NULL;
static char *(*K_i2s_ASN1_OCTET_STRING)(X509V3_EXT_METHOD*, ASN1_OCTET_STRING*) = NULL;
static int (*K_ASN1_BIT_STRING_get_bit)(ASN1_BIT_STRING*, int) = NULL;
static PKCS7 *(*K_PKCS7_new)() = NULL;
static void (*K_PKCS7_free)(PKCS7*) = NULL;
static void (*K_PKCS7_content_free)(PKCS7*) = NULL;
static int (*K_i2d_PKCS7)(PKCS7*, unsigned char**) = NULL;
static PKCS7 *(*K_d2i_PKCS7)(PKCS7**, unsigned char**,long) = NULL;
static int (*K_i2d_PKCS7_fp)(FILE*,PKCS7*) = NULL;
static PKCS7* (*K_d2i_PKCS7_fp)(FILE*,PKCS7**) = NULL;
static PKCS7* (*K_PKCS7_dup)(PKCS7*) = NULL;
static STACK_OF(X509_NAME) *(*K_SSL_load_client_CA_file)(const char*) = NULL;
static STACK_OF(X509_INFO) *(*K_PEM_X509_INFO_read)(FILE*, STACK_OF(X509_INFO)*, pem_password_cb*, void*) = NULL;
static char *(*K_ASN1_d2i_fp)(char *(*)(),char *(*)(),FILE*,unsigned char**) = NULL;
static X509 *(*K_X509_new)() = NULL;
static int (*K_X509_PURPOSE_get_count)() = NULL;
static int (*K_X509_PURPOSE_get_id)(X509_PURPOSE *) = NULL;
static int (*K_X509_check_purpose)(X509*,int,int) = NULL;
static X509_PURPOSE* (*K_X509_PURPOSE_get0)(int) = NULL;
static int (*K_EVP_PKEY_assign)(EVP_PKEY*, int, char*) = NULL;
static int (*K_X509_REQ_set_pubkey)(X509_REQ*, EVP_PKEY*) = NULL;
static RSA *(*K_RSA_generate_key)(int, unsigned long, void (*)(int,int,void *), void *) = NULL;
static int (*K_i2d_X509_REQ_fp)(FILE*, X509_REQ*) = NULL;
static void (*K_ERR_clear_error)() = NULL;
static void (*K_ERR_print_errors_fp)(FILE*) = NULL;
#endif
};


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

#ifdef __OpenBSD__
#include <qdir.h>
#include <qstring.h>
#include <qstringlist.h>

static QString findMostRecentLib(QString dir, QString name)
{
       // Grab all shared libraries in the directory
       QString filter = "lib"+name+".so.*";
       QDir d(dir, filter);
       if (!d.exists())
               return NULL;
       QStringList l = d.entryList();

       // Find the best one
       int bestmaj = -1;
       int bestmin = -1;
       QString best = NULL;
       // where do we start
       uint s = filter.length()-1;
       for (QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
               QString numberpart = (*it).mid(s);
               uint endmaj = numberpart.find('.');
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
               return NULL;
       else
               return dir+"/"+best;
}
#endif


KOpenSSLProxy::KOpenSSLProxy() {
KLibLoader *ll = KLibLoader::self();
_ok = false;
QStringList libpaths, libnamesc, libnamess;
KConfig *cfg;

   _cryptoLib = NULL;
   _sslLib = NULL;

   cfg = new KConfig("cryptodefaults", false, false);
   cfg->setGroup("OpenSSL");
   QString upath = cfg->readEntry("Path", "");
   if (!upath.isEmpty())
      libpaths << upath;

   delete cfg;

#ifdef __OpenBSD__
   {
   QString libname = findMostRecentLib("/usr/lib", "crypto");
   if (!libname.isNull())
         _cryptoLib = ll->globalLibrary(libname.latin1());
   }
#else
   libpaths 
            #ifdef _AIX
            << "/opt/freeware/lib/"
	    #endif
	    << "/usr/lib/"
	    << "/usr/ssl/lib/"
            << "/usr/local/lib/"
            << "/usr/local/openssl/lib/"
            << "/usr/local/ssl/lib/"
            << "/opt/openssl/lib/"
            << "/lib/"
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
             << "libssl.so"
	     << "libssl.so.0"
	     << "libssl.so.0.9.6"
	     << "libssl.so.0.9.6b"
	     << "libssl.so.0.9.6c"
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
             << "libcrypto.so"
	     << "libcrypto.so.0"
	     << "libcrypto.so.0.9.6c"
	     << "libcrypto.so.0.9.6b"
	     << "libcrypto.so.0.9.6"
             #endif
	     ;

   for (QStringList::Iterator it = libpaths.begin();
                              it != libpaths.end();
                              ++it) {
      for (QStringList::Iterator shit = libnamesc.begin();
                                 shit != libnamesc.end();
                                 ++shit) {
         QString alib = *it;
         if (!alib.isEmpty() && !alib.endsWith("/"))
            alib += "/";
         alib += *shit;
	 QString tmpStr(alib.latin1());
	 tmpStr.replace(QRegExp("\\(.*\\)"), "");
	 if (!access(tmpStr.latin1(), R_OK))
            _cryptoLib = ll->globalLibrary(alib.latin1());
         if (_cryptoLib) break;
      }
      if (_cryptoLib) break;
   }
#endif

   if (_cryptoLib) {
#ifdef HAVE_SSL 
      K_X509_free = (void (*) (X509 *)) _cryptoLib->symbol("X509_free");
      K_RAND_egd = (int (*)(const char *)) _cryptoLib->symbol("RAND_egd");
      K_RAND_load_file = (int (*)(const char *, long)) _cryptoLib->symbol("RAND_load_file");
      K_RAND_file_name = (const char* (*)(char *, size_t)) _cryptoLib->symbol("RAND_file_name");
      K_RAND_write_file = (int (*)(const char *)) _cryptoLib->symbol("RAND_write_file");
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
      K_X509_LOOKUP_free = (void (*)(X509_LOOKUP *)) _cryptoLib->symbol("X509_LOOKUP_free");
      K_X509_LOOKUP_ctrl = (int (*)(X509_LOOKUP *, int, const char *, long, char **)) _cryptoLib->symbol("X509_LOOKUP_ctrl");
      K_X509_STORE_CTX_init = (void (*)(X509_STORE_CTX *, X509_STORE *, X509 *, STACK_OF(X509) *)) _cryptoLib->symbol("X509_STORE_CTX_init");
      K_X509_dup = (X509* (*)(X509*)) _cryptoLib->symbol("X509_dup");
      K_BIO_new_fp = (BIO* (*)(FILE*, int)) _cryptoLib->symbol("BIO_new_fp");
      K_BIO_free = (int (*)(BIO*)) _cryptoLib->symbol("BIO_free");
      K_PEM_ASN1_write_bio = (int (*)(int (*)(), const char *,BIO*, char*, const EVP_CIPHER *, unsigned char *, int, pem_password_cb *, void *)) _cryptoLib->symbol("PEM_ASN1_write_bio");
      K_X509_asn1_meth = (ASN1_METHOD* (*)(void)) _cryptoLib->symbol("X509_asn1_meth");
      K_ASN1_i2d_fp = (int (*)(int (*)(), FILE*, unsigned char *)) _cryptoLib->symbol("ASN1_i2d_fp");
      K_i2d_ASN1_HEADER = (int (*)(ASN1_HEADER *, unsigned char **)) _cryptoLib->symbol("i2d_ASN1_HEADER");
      K_X509_print_fp = (int (*)(FILE*, X509*)) _cryptoLib->symbol("X509_print_fp");
      K_i2d_PKCS12 = (int (*)(PKCS12*, unsigned char**)) _cryptoLib->symbol("i2d_PKCS12");
      K_i2d_PKCS12_fp = (int (*)(FILE *, PKCS12*)) _cryptoLib->symbol("i2d_PKCS12_fp");
      K_PKCS12_newpass = (int (*)(PKCS12*, char*, char*)) _cryptoLib->symbol("PKCS12_newpass");
      K_d2i_PKCS12_fp = (PKCS12* (*)(FILE*, PKCS12**)) _cryptoLib->symbol("d2i_PKCS12_fp");
      K_PKCS12_new = (PKCS12* (*)()) _cryptoLib->symbol("PKCS12_new");
      K_PKCS12_free = (void (*)(PKCS12 *)) _cryptoLib->symbol("PKCS12_free");
      K_PKCS12_parse = (int (*)(PKCS12*, const char *, EVP_PKEY**,
                X509**, STACK_OF(X509)**)) _cryptoLib->symbol("PKCS12_parse");
      K_EVP_PKEY_free = (void (*) (EVP_PKEY *)) _cryptoLib->symbol("EVP_PKEY_free");
      K_EVP_PKEY_new = (EVP_PKEY* (*)()) _cryptoLib->symbol("EVP_PKEY_new");
      K_X509_REQ_free = (void (*)(X509_REQ*)) _cryptoLib->symbol("X509_REQ_free");
      K_X509_REQ_new = (X509_REQ* (*)()) _cryptoLib->symbol("X509_REQ_new");
      K_X509_STORE_CTX_set_chain = (void (*)(X509_STORE_CTX *, STACK_OF(X509)*)) _cryptoLib->symbol("X509_STORE_CTX_set_chain");
      K_X509_STORE_CTX_set_purpose = (void (*)(X509_STORE_CTX *, int)) _cryptoLib->symbol("X509_STORE_CTX_set_purpose");
      K_sk_free = (void (*) (STACK *)) _cryptoLib->symbol("sk_free");
      K_sk_num = (int (*) (STACK *)) _cryptoLib->symbol("sk_num");
      K_sk_pop = (char* (*) (STACK *)) _cryptoLib->symbol("sk_pop");
      K_sk_value = (char* (*) (STACK *, int)) _cryptoLib->symbol("sk_value");
      K_sk_new = (STACK* (*) (int (*)())) _cryptoLib->symbol("sk_new");
      K_sk_push = (int (*) (STACK*, char*)) _cryptoLib->symbol("sk_push");
      K_sk_dup = (STACK* (*) (STACK *)) _cryptoLib->symbol("sk_dup");
      K_i2s_ASN1_INTEGER = (char *(*) (X509V3_EXT_METHOD *, ASN1_INTEGER *)) _cryptoLib->symbol("i2s_ASN1_INTEGER");
      K_X509_get_serialNumber = (ASN1_INTEGER * (*) (X509 *)) _cryptoLib->symbol("X509_get_serialNumber");
      K_X509_get_pubkey = (EVP_PKEY *(*)(X509 *)) _cryptoLib->symbol("X509_get_pubkey");
      K_i2d_PublicKey = (int (*)(EVP_PKEY *, unsigned char **)) _cryptoLib->symbol("i2d_PublicKey");
      K_X509_check_private_key = (int (*)(X509 *, EVP_PKEY *)) _cryptoLib->symbol("X509_check_private_key");
      K_BN_bn2hex = (char *(*)(const BIGNUM *)) _cryptoLib->symbol("BN_bn2hex");
      K_X509_digest = (int (*)(const X509 *,const EVP_MD *, unsigned char *, unsigned int *)) _cryptoLib->symbol("X509_digest");
      K_EVP_md5 = (EVP_MD *(*)()) _cryptoLib->symbol("EVP_md5");
      K_ASN1_INTEGER_free = (void (*)(ASN1_INTEGER *)) _cryptoLib->symbol("ASN1_INTEGER_free");
      K_OBJ_obj2nid = (int (*)(ASN1_OBJECT *)) _cryptoLib->symbol("OBJ_obj2nid");
      K_OBJ_nid2ln = (const char *(*)(int)) _cryptoLib->symbol("OBJ_nid2ln");
      K_X509_get_ext_count = (int (*)(X509*)) _cryptoLib->symbol("X509_get_ext_count");
      K_X509_get_ext_by_NID = (int (*)(X509*,int,int)) _cryptoLib->symbol("X509_get_ext_by_NID");
      K_X509_get_ext_by_OBJ = (int (*)(X509*,ASN1_OBJECT*,int)) _cryptoLib->symbol("X509_get_ext_by_OBJ");
      K_X509_get_ext = (X509_EXTENSION* (*)(X509*,int)) _cryptoLib->symbol("X509_get_ext");
      K_X509_delete_ext = (X509_EXTENSION* (*)(X509*,int)) _cryptoLib->symbol("X509_delete_ext");
      K_X509_add_ext = (int (*)(X509*,X509_EXTENSION*,int)) _cryptoLib->symbol("X509_add_ext");
      K_X509_get_ext_d2i = (void* (*)(X509*,int,int*,int*)) _cryptoLib->symbol("X509_get_ext_d2i");
      K_i2s_ASN1_OCTET_STRING = (char *(*)(X509V3_EXT_METHOD*,ASN1_OCTET_STRING*)) _cryptoLib->symbol("i2s_ASN1_OCTET_STRING");
      K_ASN1_BIT_STRING_get_bit = (int (*)(ASN1_BIT_STRING*,int)) _cryptoLib->symbol("ASN1_BIT_STRING_get_bit");
      K_PKCS7_new = (PKCS7 *(*)()) _cryptoLib->symbol("PKCS7_new");
      K_PKCS7_free = (void (*)(PKCS7*)) _cryptoLib->symbol("PKCS7_free");
      K_PKCS7_content_free = (void (*)(PKCS7*)) _cryptoLib->symbol("PKCS7_content_free");
      K_i2d_PKCS7 = (int (*)(PKCS7*, unsigned char**)) _cryptoLib->symbol("i2d_PKCS7");
      K_i2d_PKCS7_fp = (int (*)(FILE*,PKCS7*)) _cryptoLib->symbol("i2d_PKCS7_fp");
      K_d2i_PKCS7 = (PKCS7* (*)(PKCS7**,unsigned char**,long)) _cryptoLib->symbol("d2i_PKCS7");
      K_d2i_PKCS7_fp = (PKCS7 *(*)(FILE *,PKCS7**)) _cryptoLib->symbol("d2i_PKCS7_fp");
      K_PKCS7_dup = (PKCS7* (*)(PKCS7*)) _cryptoLib->symbol("PKCS7_dup");
      K_PEM_X509_INFO_read = (STACK_OF(X509_INFO) *(*)(FILE*, STACK_OF(X509_INFO)*, pem_password_cb*, void *)) _cryptoLib->symbol("PEM_X509_INFO_read");
      K_ASN1_d2i_fp = (char *(*)(char *(*)(),char *(*)(),FILE*,unsigned char**)) _cryptoLib->symbol("ASN1_d2i_fp");
      K_X509_new = (X509 *(*)()) _cryptoLib->symbol("X509_new");
      K_X509_PURPOSE_get_count = (int (*)()) _cryptoLib->symbol("X509_PURPOSE_get_count");
      K_X509_PURPOSE_get_id = (int (*)(X509_PURPOSE *)) _cryptoLib->symbol("X509_PURPOSE_get_id");
      K_X509_check_purpose = (int (*)(X509*,int,int)) _cryptoLib->symbol("X509_check_purpose");
      K_X509_PURPOSE_get0 = (X509_PURPOSE *(*)(int)) _cryptoLib->symbol("X509_PURPOSE_get0");
      K_EVP_PKEY_assign = (int (*)(EVP_PKEY*, int, char*)) _cryptoLib->symbol("EVP_PKEY_assign");
      K_X509_REQ_set_pubkey = (int (*)(X509_REQ*, EVP_PKEY*)) _cryptoLib->symbol("X509_REQ_set_pubkey");
      K_RSA_generate_key = (RSA* (*)(int, unsigned long, void (*)(int,int,void *), void *)) _cryptoLib->symbol("RSA_generate_key");
      K_i2d_X509_REQ_fp = (int (*)(FILE *, X509_REQ *)) _cryptoLib->symbol("i2d_X509_REQ_fp");
      K_ERR_clear_error = (void (*)()) _cryptoLib->symbol("ERR_clear_error");
      K_ERR_print_errors_fp = (void (*)(FILE*)) _cryptoLib->symbol("ERR_print_errors_fp");
#endif
   }

#ifdef __OpenBSD__
   {
   QString libname = findMostRecentLib("/usr/lib", "ssl");
   if (!libname.isNull())
         _sslLib = ll->globalLibrary(libname.latin1());
   }
#else
   for (QStringList::Iterator it = libpaths.begin();
                              it != libpaths.end();
                              ++it) {
      for (QStringList::Iterator shit = libnamess.begin();
                                 shit != libnamess.end();
                                 ++shit) {
         QString alib = *it;
         if (!alib.isEmpty() && !alib.endsWith("/"))
            alib += "/";
         alib += *shit;
	 QString tmpStr(alib.latin1());
	 tmpStr.replace(QRegExp("\\(.*\\)"), "");
	 if (!access(tmpStr.latin1(), R_OK))
         	_sslLib = ll->globalLibrary(alib.latin1());
         if (_sslLib) break;
      }
      if (_sslLib) break;
   }
#endif

   if (_sslLib) {
#ifdef HAVE_SSL 
      // stand back from your monitor and look at this.  it's fun! :)
      K_SSL_connect = (int (*)(SSL *)) _sslLib->symbol("SSL_connect");
      K_SSL_accept = (int (*)(SSL *)) _sslLib->symbol("SSL_accept");
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
      K_SSL_use_certificate = (int (*)(SSL*, X509*)) 
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
      K_SSL_CTX_use_PrivateKey = (int (*)(SSL_CTX*, EVP_PKEY*)) _sslLib->symbol("SSL_CTX_use_PrivateKey");
      K_SSL_CTX_use_certificate = (int (*)(SSL_CTX*, X509*)) _sslLib->symbol("SSL_CTX_use_certificate");
      K_SSL_get_error = (int (*)(SSL*, int)) _sslLib->symbol("SSL_get_error");
      K_SSL_get_peer_cert_chain = (STACK_OF(X509)* (*)(SSL*)) _sslLib->symbol("SSL_get_peer_cert_chain");
      K_SSL_load_client_CA_file = (STACK_OF(X509_NAME)* (*)(const char *)) _sslLib->symbol("SSL_load_client_CA_file");
      K_SSL_peek = (int (*)(SSL*,void*,int)) _sslLib->symbol("SSL_peek");
#endif


      // Initialize the library (once only!)
      void *x;
      x = _sslLib->symbol("SSL_library_init");
      if (_cryptoLib) {
         if (x) ((int (*)())x)();
         x = _cryptoLib->symbol("OpenSSL_add_all_algorithms");
         if (x) ((void (*)())x)();
         x = _cryptoLib->symbol("OpenSSL_add_all_ciphers");
         if (x) ((void (*)())x)();
         x = _cryptoLib->symbol("OpenSSL_add_all_digests");
         if (x) ((void (*)())x)();
      }
   }

}


KOpenSSLProxy* KOpenSSLProxy::_me = NULL;
static KStaticDeleter<KOpenSSLProxy> med;



KOpenSSLProxy::~KOpenSSLProxy() {
   if (_sslLib) 
	_sslLib->unload();
   if (_cryptoLib)
	_cryptoLib->unload();
   med.setObject(0);
}


// FIXME: we should check "ok" and allow this to init the lib if !ok.

KOpenSSLProxy *KOpenSSLProxy::self() {
#ifdef HAVE_SSL
   if (!_me) {
      _me = med.setObject(new KOpenSSLProxy);
   }
#endif
   return _me;
}







#ifdef HAVE_SSL



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


int KOpenSSLProxy::SSL_use_certificate(SSL *ssl, X509 *x) {
   if (K_SSL_use_certificate) return (K_SSL_use_certificate)(ssl, x);
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


int KOpenSSLProxy::X509_print(FILE *fp, X509 *x) {
   if (K_X509_print_fp) return (K_X509_print_fp)(fp, x);
   return -1;
}


PKCS12 *KOpenSSLProxy::d2i_PKCS12_fp(FILE *fp, PKCS12 **p12) {
   if (K_d2i_PKCS12_fp) return (K_d2i_PKCS12_fp)(fp, p12);
   else return NULL;
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
   else return NULL;
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
   else return NULL;
}


void KOpenSSLProxy::X509_REQ_free(X509_REQ *x) {
   if (K_X509_REQ_free) (K_X509_REQ_free)(x);
}


X509_REQ* KOpenSSLProxy::X509_REQ_new() {
   if (K_X509_REQ_new) return (K_X509_REQ_new)();
   else return NULL;
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
   else return NULL;
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
   else return NULL;
}


char *KOpenSSLProxy::sk_value(STACK *s, int n) {
   if (K_sk_value) return (K_sk_value)(s, n);
   else return NULL;
}


void KOpenSSLProxy::X509_STORE_CTX_set_chain(X509_STORE_CTX *v, STACK_OF(X509)* x) {
   if (K_X509_STORE_CTX_set_chain) (K_X509_STORE_CTX_set_chain)(v,x);
}

void KOpenSSLProxy::X509_STORE_CTX_set_purpose(X509_STORE_CTX *v, int purpose) {
   if (K_X509_STORE_CTX_set_purpose) (K_X509_STORE_CTX_set_purpose)(v,purpose);
}


STACK* KOpenSSLProxy::sk_dup(STACK *s) {
   if (K_sk_dup) return (K_sk_dup)(s);
   else return NULL;
}


STACK* KOpenSSLProxy::sk_new(int (*cmp)()) {
   if (K_sk_new) return (K_sk_new)(cmp);
   else return NULL;
}


int KOpenSSLProxy::sk_push(STACK* s, char* d) {
   if (K_sk_push) return (K_sk_push)(s,d);
   else return -1;
}


char *KOpenSSLProxy::i2s_ASN1_INTEGER(X509V3_EXT_METHOD *meth, ASN1_INTEGER *aint) {
   if (K_i2s_ASN1_INTEGER) return (K_i2s_ASN1_INTEGER)(meth, aint);
   else return NULL;
}


ASN1_INTEGER *KOpenSSLProxy::X509_get_serialNumber(X509 *x) {
   if (K_X509_get_serialNumber) return (K_X509_get_serialNumber)(x);
   else return NULL;
}


EVP_PKEY *KOpenSSLProxy::X509_get_pubkey(X509 *x) {
   if (K_X509_get_pubkey) return (K_X509_get_pubkey)(x);
   else return NULL;
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
   else return NULL;
}


int KOpenSSLProxy::X509_digest(const X509 *x,const EVP_MD *t, unsigned char *md, unsigned int *len) {
   if (K_X509_digest) return (K_X509_digest)(x, t, md, len);
   else return -1;
}


EVP_MD *KOpenSSLProxy::EVP_md5() {
   if (K_EVP_md5) return (K_EVP_md5)();
   return NULL;
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
   else return NULL;
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
   else return NULL;
}


X509_EXTENSION *KOpenSSLProxy::X509_delete_ext(X509 *x, int loc) {
   if (K_X509_delete_ext) return (K_X509_delete_ext)(x,loc);
   else return NULL;
}


int KOpenSSLProxy::X509_add_ext(X509 *x, X509_EXTENSION *ex, int loc) {
   if (K_X509_add_ext) return (K_X509_add_ext)(x,ex,loc);
   else return -1;
}


void *KOpenSSLProxy::X509_get_ext_d2i(X509 *x, int nid, int *crit, int *idx) {
   if (K_X509_get_ext_d2i) return (K_X509_get_ext_d2i)(x,nid,crit,idx);
   else return NULL;
}


char *KOpenSSLProxy::i2s_ASN1_OCTET_STRING(X509V3_EXT_METHOD *method, ASN1_OCTET_STRING *ia5) {
   if (K_i2s_ASN1_OCTET_STRING) return (K_i2s_ASN1_OCTET_STRING)(method,ia5);
   else return NULL;
}


int KOpenSSLProxy::ASN1_BIT_STRING_get_bit(ASN1_BIT_STRING *a, int n) {
   if (K_ASN1_BIT_STRING_get_bit) return (K_ASN1_BIT_STRING_get_bit)(a,n);
   else return -1;
}


PKCS7 *KOpenSSLProxy::PKCS7_new(void) {
   if (K_PKCS7_new) return (K_PKCS7_new)();
   else return NULL;
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
   else return NULL;
}


int KOpenSSLProxy::i2d_PKCS7_fp(FILE *fp,PKCS7 *p7) {
   if (K_i2d_PKCS7_fp) return (K_i2d_PKCS7_fp)(fp,p7);
   else return -1;
}
 

PKCS7 *KOpenSSLProxy::d2i_PKCS7_fp(FILE *fp,PKCS7 **p7) {
   if (K_d2i_PKCS7_fp) return (K_d2i_PKCS7_fp)(fp,p7);
   else return NULL;
}


PKCS7 *KOpenSSLProxy::PKCS7_dup(PKCS7 *p7) {
   if (K_PKCS7_dup) return (K_PKCS7_dup)(p7);
   else return NULL;
}


STACK_OF(X509_NAME) *KOpenSSLProxy::SSL_load_client_CA_file(const char *file) {
   if (K_SSL_load_client_CA_file) return (K_SSL_load_client_CA_file)(file);
   else return NULL;
}


STACK_OF(X509_INFO) *KOpenSSLProxy::PEM_X509_INFO_read(FILE *fp, STACK_OF(X509_INFO) *sk, pem_password_cb *cb, void *u) {
   if (K_PEM_X509_INFO_read) return (K_PEM_X509_INFO_read)(fp,sk,cb,u);
   else return NULL;
}


X509 *KOpenSSLProxy::X509_d2i_fp(FILE *out, X509** buf) {
   if (K_ASN1_d2i_fp) return reinterpret_cast<X509 *>((K_ASN1_d2i_fp)(reinterpret_cast<char *(*)()>(K_X509_new), reinterpret_cast<char *(*)()>(K_d2i_X509), out, reinterpret_cast<unsigned char **>(buf)));
   else return NULL;
}


int KOpenSSLProxy::SSL_peek(SSL *ssl,void *buf,int num) {
   if (K_SSL_peek) return (K_SSL_peek)(ssl,buf,num);
   else return -1;
}


const char *KOpenSSLProxy::RAND_file_name(char *buf, size_t num) {
   if (K_RAND_file_name) return (K_RAND_file_name)(buf, num);
   else return NULL;
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
   else return NULL;
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
   else return NULL;
}


int KOpenSSLProxy::i2d_X509_REQ_fp(FILE *fp, X509_REQ *x) {
   if (K_i2d_X509_REQ_fp) return (K_i2d_X509_REQ_fp)(fp,x);
   else return -1;
}


void KOpenSSLProxy::ERR_clear_error() {
   if (K_ERR_clear_error) (K_ERR_clear_error)();
}


void KOpenSSLProxy::ERR_print_errors_fp(FILE* fp) {
   if (K_ERR_print_errors_fp) (K_ERR_print_errors_fp)(fp);
}



#endif

