#ifndef EXTERN_MD5_H
#define EXTERN_MD5_H

#ifdef __cplusplus
extern "C" {
#endif
  char *base64_encode_line(const char *s);
  char *base64_encode_string(const char *s, unsigned int len);
#ifdef __cplusplus
}
#endif

#ifdef DO_MD5

#define HASHLEN 16
typedef char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];
#define IN
#define OUT


#ifdef __cplusplus
extern "C" {

/* calculate H(A1) as per HTTP Digest spec */
extern void DigestCalcHA1(
    IN const char * pszAlg,
    IN const char * pszUserName,
    IN const char * pszRealm,
    IN const char * pszPassword,
    IN const char * pszNonce,
    IN const char * pszCNonce,
    OUT HASHHEX SessionKey
    );

/* calculate request-digest/response-digest as per HTTP Digest spec */
extern void DigestCalcResponse(
    IN HASHHEX HA1,           /* H(A1) */
    IN const char * pszNonce,       /* nonce from server */
    IN const char * pszNonceCount,  /* 8 hex digits */
    IN const char * pszCNonce,      /* client nonce */
    IN const char * pszQop,         /* qop-value: "", "auth", "auth-int" */
    IN const char * pszMethod,      /* method from the request */
    IN const char * pszDigestUri,   /* requested URL */
    IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
    OUT HASHHEX Response      /* request-digest or response-digest */
    );
}
#endif
#endif
#endif
