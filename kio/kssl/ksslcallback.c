/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

// DON'T INCLUDE ANYTHING IN HERE.  THIS FILE IS NOT COMPILED AS IT IS.

#ifdef KSSL_HAVE_SSL
#ifndef _kde_ksslcallback_c
#define _kde_ksslcallback_c

extern "C" {
static int X509Callback(int ok, X509_STORE_CTX *ctx) {
 
  kdDebug(7029) << "X509Callback: ok = " << ok << " error = " << ctx->error << endl;
  // Here is how this works.  We put "ok = 1;" in any case that we
  // don't consider to be an error.  In that case, it will return OK
  // for the certificate check as long as there are no other critical
  // errors.  Don't forget that there can be multiple errors.
  // 
  // Of course we can also put other code in here but any data returned
  // back will not be threadsafe ofcourse.
 
  if (!ok) {
    switch (ctx->error) {
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
      case X509_V_ERR_UNABLE_TO_GET_CRL:
      case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
      case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
      case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
      case X509_V_ERR_CERT_SIGNATURE_FAILURE:
      case X509_V_ERR_CRL_SIGNATURE_FAILURE:
      case X509_V_ERR_CERT_NOT_YET_VALID:
      case X509_V_ERR_CERT_HAS_EXPIRED:
      case X509_V_ERR_CRL_NOT_YET_VALID:
      case X509_V_ERR_CRL_HAS_EXPIRED:
      case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
      case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
      case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
      case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
      case X509_V_ERR_OUT_OF_MEM:
      case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
      case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
      case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
      case X509_V_ERR_CERT_CHAIN_TOO_LONG:
      case X509_V_ERR_CERT_REVOKED:
      case X509_V_ERR_INVALID_CA:
      case X509_V_ERR_PATH_LENGTH_EXCEEDED:
      case X509_V_ERR_INVALID_PURPOSE:
      case X509_V_ERR_CERT_UNTRUSTED:
      case X509_V_ERR_CERT_REJECTED:
      case X509_V_ERR_APPLICATION_VERIFICATION:
    default:
     break;
    }
  }
 
  return(ok);
}
};

#endif
#endif

