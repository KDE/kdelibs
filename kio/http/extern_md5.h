#ifndef EXTERN_MD5_H
#define EXTERN_MD5_H "$Id$"

typedef unsigned INT32_BASETYPE UINT32_T;

#ifdef DO_MD5

#define HASHLEN 16
typedef char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];

/* MD5 context. */
typedef struct Local_MD5Context {
  UINT32_T state[4];   /* state (ABCD) */
  UINT32_T count[2];   /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];     /* input buffer */
} Local_MD5_CTX;

#ifdef __cplusplus
extern "C" {
#endif
	/* calculate H(A1) as per HTTP Digest spec */
	extern void DigestCalcHA1 (const char *pszAlg, const char *pszUserName, const char *pszRealm, const char *pszPassword, const char *pszNonce, const char *pszCNonce, HASHHEX SessionKey);

	/* calculate request-digest/response-digest as per HTTP Digest spec */
	extern void DigestCalcResponse (HASHHEX HA1, const char *pszNonce, const char *pszNonceCount, const char *pszCNonce, const char *pszQop, const char *pszMethod, const char *pszDigestUri,  HASHHEX HEntity, HASHHEX Response);

	void MD5Init (Local_MD5_CTX *);
	void MD5Update (Local_MD5_CTX *, const unsigned char *, unsigned int);
	void MD5Pad (Local_MD5_CTX *);
	void MD5Final (unsigned char [16], Local_MD5_CTX *);
	char *MD5End(Local_MD5_CTX *, char *);
	char *MD5File(const char *, char *);
	char *MD5Data(const unsigned char *, unsigned int, char *);
	static void MD5Transform (UINT32_T state[4], const unsigned char block[64]);
#ifdef __cplusplus
}
#endif

#endif
#endif
