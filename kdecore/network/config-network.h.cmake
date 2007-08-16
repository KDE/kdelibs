#ifndef CONFIG_NETWORK_H
#define CONFIG_NETWORK_H


/********** structs used by kdecore/network (and netsupp.cpp) *********/

/* TODO: write those two tests */

#define HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID 1 /* Define to 1 if `sin6_scope_id' is member of `struct sockaddr_in6'. */

/* functions */
#cmakedefine   HAVE_FREEADDRINFO 1
#cmakedefine   HAVE_GETADDRINFO 1
#cmakedefine   HAVE_GETHOSTBYNAME 1
#cmakedefine   HAVE_GETHOSTBYNAME2 1
#cmakedefine   HAVE_GETHOSTBYNAME2_R 1
#cmakedefine   HAVE_GETHOSTBYNAME_R 1
#cmakedefine   HAVE_GETNAMEINFO 1
#cmakedefine   HAVE_GETPROTOBYNAME_R 1
#cmakedefine   HAVE_GETSERVBYNAME_R 1
#cmakedefine   HAVE_GETSERVBYPORT_R 1
#cmakedefine   HAVE_IF_NAMETOINDEX 1
#cmakedefine   HAVE_INET_NTOP 1
#cmakedefine   HAVE_INET_PTON 1
#cmakedefine   HAVE_POLL 1
#cmakedefine   HAVE_RES_INIT 1
#cmakedefine   HAVE_GAI_STRERROR_PROTO 1

/* header files */
#cmakedefine HAVE_NET_IF_H 1
#cmakedefine HAVE_IDNA_H 1
#cmakedefine HAVE_SYS_FILIO_H 1

/* structs */
#cmakedefine HAVE_STRUCT_ADDRINFO 1
#cmakedefine HAVE_STRUCT_SOCKADDR_IN6 1

#cmakedefine HAVE_GETSERVBYNAME_R_PROTO 1
#cmakedefine HAVE_RES_INIT_PROTO 1

#cmakedefine  HAVE_STRUCT_SOCKADDR_SA_LEN 1

#if !defined(HAVE_RES_INIT_PROTO)
#ifdef __cplusplus
extern "C" {
#endif
int res_init(void);
#ifdef __cplusplus
}
#endif
#endif

#endif /* CONFIG_NETWORK_H */
