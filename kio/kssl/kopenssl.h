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


// IF YOU ARE USING THIS CLASS, YOU ARE MAKING A MISTAKE.

#ifndef __KOPENSSLPROXY_H 
#define __KOPENSSLPROXY_H

#define KOSSL KOpenSSLProxy
class KOpenSSLProxyPrivate;

#include <config.h>
#include <klibloader.h>

#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/asn1.h>
#include <openssl/pkcs7.h>
#include <openssl/pkcs12.h>
#include <openssl/evp.h>
#include <openssl/stack.h>
#include <openssl/bn.h>
#undef crypt
#endif

#include <kstaticdeleter.h>

class KOpenSSLProxy {
friend class KStaticDeleter<KOpenSSLProxy>;
public:

   /**
    * Return an instance of class KOpenSSLProxy *
    * You cannot delete this object.  It is a singleton class.
    */
   static KOpenSSLProxy *self();

   /**
    *   Return true of libcrypto was found and loaded
    */
   bool hasLibCrypto() const;

   /**
    *   Return true of libssl was found and loaded
    */
   bool hasLibSSL() const;

   /**
    *   Destroy the class and start over - don't use this unless you know
    *   what you are doing.
    */
   void destroy();

   // Here are the symbols that we need.
#ifdef HAVE_SSL

   /*
    *   SSL_connect - initiate the TLS/SSL handshake with an TLS/SSL server
    */
   int SSL_connect(SSL *ssl);

   /*
    *   SSL_accept - initiate the TLS/SSL handshake with an TLS/SSL server
    */
   int SSL_accept(SSL *ssl);

   /*
    *   SSL_get_error - get the error code
    */
   int SSL_get_error(SSL *ssl, int rc);

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
    *   SSL_shutdown - shutdown an allocated SSL connection
    */
   int SSL_shutdown(SSL *ssl);

   /*
    *   SSL_CTX_new - create a new SSL_CTX object as framework for TLS/SSL enabled functions
    */
   SSL_CTX *SSL_CTX_new(SSL_METHOD *method);

   /*
    *   SSL_CTX_free - free an allocated SSL_CTX object
    */
   void SSL_CTX_free(SSL_CTX *ctx);

   /*
    *   SSL_set_fd - connect the SSL object with a file descriptor
    */
   int SSL_set_fd(SSL *ssl, int fd);

   /*
    *   SSL_pending - obtain number of readable bytes buffered in an SSL object
    */
   int SSL_pending(SSL *ssl);

   /*
    *   SSL_peek - obtain bytes buffered in an SSL object
    */
   int SSL_peek(SSL *ssl, void *buf, int num);

   /*
    *   SSL_CTX_set_cipher_list - choose list of available SSL_CIPHERs
    */
   int SSL_CTX_set_cipher_list(SSL_CTX *ctx, const char *str);

   /*
    *   SSL_CTX_set_verify - set peer certificate verification parameters
    */
   void SSL_CTX_set_verify(SSL_CTX *ctx, int mode,
                         int (*verify_callback)(int, X509_STORE_CTX *));

   /*
    *   SSL_use_certificate - load certificate
    */
   int SSL_use_certificate(SSL *ssl, X509 *x);

   /*
    *   SSL_get_current_cipher - get SSL_CIPHER of a connection
    */
   SSL_CIPHER *SSL_get_current_cipher(SSL *ssl);

   /*
    *   SSL_set_options - manipulate SSL engine options
    *   Note: These are all mapped to SSL_ctrl so call them as the comment
    *         specifies but know that they use SSL_ctrl.  They are #define
    *         so they will map to the one in this class if called as a
    *         member function of this class.
    */
   /* long SSL_set_options(SSL *ssl, long options); */
   long    SSL_ctrl(SSL *ssl,int cmd, long larg, char *parg);

   /*
    *   RAND_egd - set the path to the EGD
    */
   int RAND_egd(const char *path);


   /*
    *   RAND_file_name 
    */
   const char *RAND_file_name(char *buf, size_t num);


   /*
    *   RAND_load_file 
    */
   int RAND_load_file(const char *filename, long max_bytes);


   /*
    *   RAND_write_file 
    */
   int RAND_write_file(const char *filename);


   /*
    *   TLSv1_client_method - return a TLSv1 client method object
    */
   SSL_METHOD *TLSv1_client_method();


   /*
    *   SSLv2_client_method - return a SSLv2 client method object
    */
   SSL_METHOD *SSLv2_client_method();


   /*
    *   SSLv3_client_method - return a SSLv3 client method object
    */
   SSL_METHOD *SSLv3_client_method();


   /*
    *   SSLv23_client_method - return a SSLv23 client method object
    */
   SSL_METHOD *SSLv23_client_method();


   /*
    *   SSL_get_peer_certificate - return the peer's certificate
    */
   X509 *SSL_get_peer_certificate(SSL *s);


   /*
    *   SSL_get_peer_cert_chain - get the peer's certificate chain
    */
   STACK_OF(X509) *SSL_get_peer_cert_chain(SSL *s);

   /*
    *   SSL_CIPHER_get_bits - get the number of bits in this cipher
    */
   int SSL_CIPHER_get_bits(SSL_CIPHER *c,int *alg_bits);


   /*
    *   SSL_CIPHER_get_version - get the version of this cipher
    */
   char *SSL_CIPHER_get_version(SSL_CIPHER *c);


   /*
    *   SSL_CIPHER_get_name - get the name of this cipher
    */
   const char *SSL_CIPHER_get_name(SSL_CIPHER *c);


   /*
    *   SSL_CIPHER_description - get the description of this cipher
    */
   char *SSL_CIPHER_description(SSL_CIPHER *,char *buf,int size);


   /*
    *   SSL_CTX_use_PrivateKey - set the private key for the session.
    *                          - for use with client certificates
    */
   int SSL_CTX_use_PrivateKey(SSL_CTX *ctx, EVP_PKEY *pkey);


   /*
    *   SSL_CTX_use_certificate - set the client certificate for the session.
    */
   int SSL_CTX_use_certificate(SSL_CTX *ctx, X509 *x);


   /*
    *   d2i_X509 - Covert a text representation of X509 to an X509 object
    */
   X509 * d2i_X509(X509 **a,unsigned char **pp,long length);


   /*
    *   i2d_X509 - Covert an X509 object into a text representation
    */
   int i2d_X509(X509 *a,unsigned char **pp);


   /*
    *   X509_cmp - compare two X509 objects
    */
   int X509_cmp(X509 *a, X509 *b);


   /*
    *   X509_dup - duplicate an X509 object
    */
   X509 *X509_dup(X509 *x509);


   /*
    *   X509_STORE_CTX_new - create an X509 store context
    */
   X509_STORE_CTX *X509_STORE_CTX_new(void);


   /*
    *   X509_STORE_CTX_free - free up an X509 store context
    */
   void X509_STORE_CTX_free(X509_STORE_CTX *v);


   /*
    *   X509_STORE_CTX_set_chain - set the certificate chain
    */
   void X509_STORE_CTX_set_chain(X509_STORE_CTX *v, STACK_OF(X509)* x);


   /*
    *   X509_verify_cert - verify the certificate
    */
   int X509_verify_cert(X509_STORE_CTX *v);


   /*
    *   X509_STORE_new - create an X509 store
    */
   X509_STORE *X509_STORE_new(void);


   /*
    *   X509_STORE_free - free up an X509 store
    */
   void X509_STORE_free(X509_STORE *v);


   /*
    *   X509_free - free up an X509
    */
   void X509_free(X509 *v);


   /*
    *   X509_NAME_oneline - return the X509 data in a string
    */
   char *X509_NAME_oneline(X509_NAME *a, char *buf, int size);


   /*
    *   X509_get_subject_name - return the X509_NAME for the subject field
    */
   X509_NAME *X509_get_subject_name(X509 *a);


   /*
    *   X509_get_issuer_name - return the X509_NAME for the issuer field
    */
   X509_NAME *X509_get_issuer_name(X509 *a);


   /*
    *   X509_STORE_add_lookup - add a lookup file/method to an X509 store
    */
   X509_LOOKUP *X509_STORE_add_lookup(X509_STORE *v, X509_LOOKUP_METHOD *m);


   /*
    *   X509_LOOKUP_file - Definition of the LOOKUP_file method
    */
   X509_LOOKUP_METHOD *X509_LOOKUP_file(void);


   /*
    *   X509_LOOKUP_free - Free an X509_LOOKUP
    */
   void X509_LOOKUP_free(X509_LOOKUP *x);


   /*
    *   X509_LOOKUP_ctrl - This is not normally called directly (use macros)
    */
   int X509_LOOKUP_ctrl(X509_LOOKUP *ctx, int cmd, const char *argc, long argl, char **ret);


   /*
    *   X509_STORE_CTX_init - initialize an X509 STORE context
    */
   void X509_STORE_CTX_init(X509_STORE_CTX *ctx, X509_STORE *store, X509 *x509, STACK_OF(X509) *chain);


   /*
    *   CRYPTO_free - free up an internally allocated object
    */
   void CRYPTO_free(void *x);


   /*
    *   BIO_new_fp - nastiness called BIO - used to create BIO* from FILE*
    */
   BIO *BIO_new_fp(FILE *stream, int close_flag);


   /*
    *   BIO_free - nastiness called BIO - used to destroy BIO*
    */
   int BIO_free(BIO *a);


   /*
    *   PEM_write_bio_X509 - write a PEM encoded cert to a BIO*
    */
   int PEM_write_bio_X509(BIO *bp, X509 *x);


   /*
    *   X509_asn1_meth - used for netscape output
    */
   ASN1_METHOD *X509_asn1_meth();


   /*
    *   ASN1_i2d_fp - used for netscape output
    */
   int ASN1_i2d_fp(FILE *out, unsigned char *x);


   /*
    *   ASN1_d2i_fp - read an X509 from a DER encoded file (buf can be NULL)
    */
   X509 *X509_d2i_fp(FILE *out, X509** buf);


   /*
    *   X509_print - print the text form of an X509
    */
   int X509_print(FILE *fp, X509 *x);


   /*
    *   Read a PKCS#12 cert from fp
    */
   PKCS12 *d2i_PKCS12_fp(FILE *fp, PKCS12 **p12);


   /*
    *   Change the password on a PKCS#12 cert
    */
   int PKCS12_newpass(PKCS12 *p12, char *oldpass, char *newpass);


   /*
    *   Write a PKCS#12 to mem
    */
   int i2d_PKCS12(PKCS12 *p12, unsigned char **p);


   /*
    *   Write a PKCS#12 to FILE*
    */
   int i2d_PKCS12_fp(FILE *fp, PKCS12 *p12);


   /*
    *   Create a new PKCS#12 object
    */
   PKCS12 *PKCS12_new(void);


   /*
    *   Destroy that PKCS#12 that you created!
    */
   void PKCS12_free(PKCS12 *a);


   /* 
    *   Parse the PKCS#12 
    */
   int PKCS12_parse(PKCS12 *p12, const char *pass, EVP_PKEY **pkey,
                    X509 **cert, STACK_OF(X509) **ca);


   /* 
    *   Free the Private Key
    */
   void EVP_PKEY_free(EVP_PKEY *x);


   /* 
    *   Pop off the stack
    */
   char *sk_pop(STACK *s);


   /* 
    *   Free the stack
    */
   void sk_free(STACK *s);


   /* 
    *  Number of elements in the stack
    */
   int sk_num(STACK *s);


   /* 
    *  Value of element n in the stack
    */
   char *sk_value(STACK *s, int n);


   /* 
    *  Create a new stack
    */
   STACK *sk_new(int (*cmp)());


   /* 
    *  Add an element to the stack
    */
   int sk_push(STACK *s, char *d);


   /* 
    *  Duplicate the stack
    */
   STACK *sk_dup(STACK *s);


   /*
    *  Convert an ASN1_INTEGER to it's text form
    */
   char *i2s_ASN1_INTEGER(X509V3_EXT_METHOD *meth, ASN1_INTEGER *aint);


   /*
    *  Get the certificate's serial number
    */
   ASN1_INTEGER *X509_get_serialNumber(X509 *x);


   /*
    *  Get the certificate's public key
    */
   EVP_PKEY *X509_get_pubkey(X509 *x);


   /*
    *  Convert the public key to a decimal form
    */
   int i2d_PublicKey(EVP_PKEY *a, unsigned char **pp); 


   /*
    *  Check the private key of a PKCS bundle against the X509
    */
   int X509_check_private_key(X509 *x, EVP_PKEY *p);


   /*
    *  Convert a BIGNUM to a hex string
    */
   char *BN_bn2hex(const BIGNUM *a);


   /*
    *  Compute the digest of an X.509
    */
   int X509_digest(const X509 *x,const EVP_MD *t, unsigned char *md, unsigned int *len);


   /*
    *  EVP_md5
    */
   EVP_MD *EVP_md5();


   /*
    *  ASN1_INTEGER free
    */
   void ASN1_INTEGER_free(ASN1_INTEGER *x);


   /*
    *  
    */
   int OBJ_obj2nid(ASN1_OBJECT *o);

   /*
    *  
    */
   const char * OBJ_nid2ln(int n);

   /*
    * get the number of extensions
    */
   int X509_get_ext_count(X509 *x);

   /*
    * 
    */
   int X509_get_ext_by_NID(X509 *x, int nid, int lastpos);

   /*
    *
    */
   int X509_get_ext_by_OBJ(X509 *x,ASN1_OBJECT *obj,int lastpos);

   /*
    *
    */
   X509_EXTENSION *X509_get_ext(X509 *x, int loc);

   /*
    *
    */
   X509_EXTENSION *X509_delete_ext(X509 *x, int loc);

   /*
    *
    */
   int X509_add_ext(X509 *x, X509_EXTENSION *ex, int loc);

   /*
    *
    */
   void *X509_get_ext_d2i(X509 *x, int nid, int *crit, int *idx);

   /*
    *
    */
   char *i2s_ASN1_OCTET_STRING(X509V3_EXT_METHOD *method, ASN1_OCTET_STRING *ia5);

   /*
    *
    */
   int ASN1_BIT_STRING_get_bit(ASN1_BIT_STRING *a, int n);

   /*
    *
    */
   PKCS7 *PKCS7_new(void);

   /*
    *
    */
   void PKCS7_free(PKCS7 *a);

   /*
    *
    */
   void PKCS7_content_free(PKCS7 *a);

   /*
    *
    */
   int i2d_PKCS7(PKCS7 *a, unsigned char **pp);

   /*
    *
    */
   PKCS7 *d2i_PKCS7(PKCS7 **a, unsigned char **pp,long length);

   /*
    *
    */
   int i2d_PKCS7_fp(FILE *fp,PKCS7 *p7);

   /*
    * 
    */
   PKCS7 *d2i_PKCS7_fp(FILE *fp,PKCS7 **p7);

   /*
    *
    */
   PKCS7 *PKCS7_dup(PKCS7 *p7);

   /*
    * Load a CA list file.
    */
   STACK_OF(X509_NAME) *SSL_load_client_CA_file(const char *file);

   /*
    * Load a file of PEM encoded objects.
    */
   STACK_OF(X509_INFO) *PEM_X509_INFO_read(FILE *fp, STACK_OF(X509_INFO) *sk,
		           pem_password_cb *cb, void *u);

   /*
    * Get the number of purposes available
    */
   int X509_PURPOSE_get_count();


   /*
    * Get the ID of a purpose
    */
   int X509_PURPOSE_get_id(X509_PURPOSE *);


   /*
    * Check the existence of purpose id "id" in x.  for CA, set ca = 1, else 0
    */
   int X509_check_purpose(X509 *x, int id, int ca);


   /*
    * Get the purpose with index #idx
    */
   X509_PURPOSE * X509_PURPOSE_get0(int idx);


   /*
    * Create a new Private KEY
    */
   EVP_PKEY* EVP_PKEY_new();


   /*
    * Assign a private key
    */
   int EVP_PKEY_assign(EVP_PKEY *pkey, int type, char *key);


   /*
    * Generate a RSA key
    */
   RSA *RSA_generate_key(int bits, unsigned long e, void
                        (*callback)(int,int,void *), void *cb_arg);


   /*
    * Create/destroy a certificate request
    */
   X509_REQ *X509_REQ_new();
   void X509_REQ_free(X509_REQ *a);


   /*
    * Set the public key in the REQ object
    */
   int X509_REQ_set_pubkey(X509_REQ *x, EVP_PKEY *pkey);

   /* for testing */
   int i2d_X509_REQ_fp(FILE *fp, X509_REQ *x);


   /* clear the current error  - use this often*/
   void ERR_clear_error();

   /* Print the errors to this stream */
   void ERR_print_errors_fp(FILE *fp);


#endif

private:
   KOpenSSLProxy();
   ~KOpenSSLProxy();
   KOpenSSLProxyPrivate *d;

   KLibrary *_sslLib;
   KLibrary *_cryptoLib;
   static KOpenSSLProxy *_me;

   bool _ok;
};

#endif

