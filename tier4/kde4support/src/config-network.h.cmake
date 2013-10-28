#ifndef CONFIG_NETWORK_H
#define CONFIG_NETWORK_H


/********** structs used by kdecore/network (and netsupp.cpp) *********/

/* TODO: write those two tests */

#define HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID 1 /* Define to 1 if `sin6_scope_id' is member of `struct sockaddr_in6'. */

/* functions */
#cmakedefine01 HAVE_GETADDRINFO
#cmakedefine01 HAVE_GETHOSTBYNAME2
#cmakedefine01 HAVE_GETHOSTBYNAME2_R
#cmakedefine01 HAVE_GETHOSTBYNAME_R
#cmakedefine01 HAVE_GETNAMEINFO
#cmakedefine01 HAVE_GETPROTOBYNAME_R
#cmakedefine01 HAVE_GETSERVBYNAME_R
#cmakedefine01 HAVE_GETSERVBYPORT_R
#cmakedefine01 HAVE_IF_NAMETOINDEX
#cmakedefine01 HAVE_INET_NTOP
#cmakedefine01 HAVE_INET_PTON
#cmakedefine01 HAVE_POLL
#cmakedefine01 HAVE_RES_INIT
#cmakedefine01 HAVE_GAI_STRERROR_PROTO
#cmakedefine01 HAVE_STRUCT_SOCKADDR_SA_LEN

/* header files */
#cmakedefine01 HAVE_NET_IF_H
#cmakedefine01 HAVE_SYS_FILIO_H

/* structs */
#cmakedefine01 HAVE_STRUCT_ADDRINFO
#cmakedefine01 HAVE_BROKEN_GETADDRINFO
#cmakedefine01 HAVE_STRUCT_SOCKADDR_IN6

#cmakedefine01 HAVE_GETSERVBYNAME_R_PROTO
#cmakedefine01 HAVE_RES_INIT_PROTO

#if !HAVE_RES_INIT_PROTO
#ifdef __cplusplus
extern "C" {
#endif
int res_init(void);
#ifdef __cplusplus
}
#endif
#endif

#endif /* CONFIG_NETWORK_H */
