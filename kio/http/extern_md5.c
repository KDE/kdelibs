/*
 * $Id$
 */

#include <config.h>

#include <sys/types.h>

#ifdef HAVE_SSL
#define DO_MD5
#endif

#ifdef DO_MD5
#include <openssl/md5.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "extern_md5.h"
#include "base64.h"

/* From the HTTP draft */
#ifdef DO_MD5

void CvtHex(
    IN HASH Bin,
    OUT HASHHEX Hex
    )
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++) {
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
            Hex[i*2] = (j + '0');
         else
            Hex[i*2] = (j + 'a' - 10);
        j = Bin[i] & 0xf;
        if (j <= 9)
            Hex[i*2+1] = (j + '0');
         else
            Hex[i*2+1] = (j + 'a' - 10);
    };
    Hex[HASHHEXLEN] = '\0';
};

/* calculate H(A1) as per spec */
void DigestCalcHA1(
    IN const char * pszAlg,
    IN const char * pszUserName,
    IN const char * pszRealm,
    IN const char * pszPassword,
    IN const char * pszNonce,
    IN const char * pszCNonce,
    OUT HASHHEX SessionKey
    )
{
      MD5_CTX Md5Ctx;
      HASH HA1;

      MD5_Init(&Md5Ctx);
      MD5_Update(&Md5Ctx, pszUserName, strlen(pszUserName));
      MD5_Update(&Md5Ctx, ":", 1);
      MD5_Update(&Md5Ctx, pszRealm, strlen(pszRealm));
      MD5_Update(&Md5Ctx, ":", 1);
      MD5_Update(&Md5Ctx, pszPassword, strlen(pszPassword));
      MD5_Final(HA1, &Md5Ctx);
      if (strcmp(pszAlg, "md5-sess") == 0) {
            MD5_Init(&Md5Ctx);
            MD5_Update(&Md5Ctx, HA1, HASHLEN);
            MD5_Update(&Md5Ctx, ":", 1);
            MD5_Update(&Md5Ctx, pszNonce, strlen(pszNonce));
            MD5_Update(&Md5Ctx, ":", 1);
            MD5_Update(&Md5Ctx, pszCNonce, strlen(pszCNonce));
            MD5_Final(HA1, &Md5Ctx);
      };
      CvtHex(HA1, SessionKey);
};

/* calculate request-digest/response-digest as per HTTP Digest spec */
void DigestCalcResponse(
    IN HASHHEX HA1,           /* H(A1) */
    IN const char * pszNonce,       /* nonce from server */
    IN const char * pszNonceCount,  /* 8 hex digits */
    IN const char * pszCNonce,      /* client nonce */
    IN const char * pszQop,         /* qop-value: "", "auth", "auth-int" */
    IN const char * pszMethod,      /* method from the request */
    IN const char * pszDigestUri,   /* requested URL */
    IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
    OUT HASHHEX Response      /* request-digest or response-digest */
    )
{
      MD5_CTX Md5Ctx;
      HASH HA2;
      HASH RespHash;
       HASHHEX HA2Hex;

       /* calculate H(A2) */
      MD5_Init(&Md5Ctx);
      MD5_Update(&Md5Ctx, pszMethod, strlen(pszMethod));
      MD5_Update(&Md5Ctx, ":", 1);
      MD5_Update(&Md5Ctx, pszDigestUri, strlen(pszDigestUri));
      if (strcmp(pszQop, "auth-int") == 0) {
            MD5_Update(&Md5Ctx, ":", 1);
            MD5_Update(&Md5Ctx, HEntity, HASHHEXLEN);
      };
      MD5_Final(HA2, &Md5Ctx);
       CvtHex(HA2, HA2Hex);

       /* calculate response */
      MD5_Init(&Md5Ctx);
      MD5_Update(&Md5Ctx, HA1, HASHHEXLEN);
      MD5_Update(&Md5Ctx, ":", 1);
      MD5_Update(&Md5Ctx, pszNonce, strlen(pszNonce));
      MD5_Update(&Md5Ctx, ":", 1);
      if (*pszQop) {
          MD5_Update(&Md5Ctx, pszNonceCount, strlen(pszNonceCount));
          MD5_Update(&Md5Ctx, ":", 1);
          MD5_Update(&Md5Ctx, pszCNonce, strlen(pszCNonce));
          MD5_Update(&Md5Ctx, ":", 1);
          MD5_Update(&Md5Ctx, pszQop, strlen(pszQop));
          MD5_Update(&Md5Ctx, ":", 1);
      };
      MD5_Update(&Md5Ctx, HA2Hex, HASHHEXLEN);
      MD5_Final(RespHash, &Md5Ctx);
      CvtHex(RespHash, Response);
};

#endif
