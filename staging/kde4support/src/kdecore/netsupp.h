/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2003 Thiago Macieira <thiago.macieira@kdemail.net>>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef _NETSUPP_H_
#define _NETSUPP_H_

#include <kde4support_export.h>

#include <config.h>
#include <config-network.h>

#include <sys/socket.h>
#include <netdb.h>
//#include "ksockaddr.h"


/*
 * Seems some systems don't know about AF_LOCAL
 */
#ifndef AF_LOCAL
#define AF_LOCAL	AF_UNIX
#define PF_LOCAL	PF_UNIX
#endif

#ifdef CLOBBER_IN6
#define kde_in6_addr		in6_addr
#define kde_sockaddr_in6	sockaddr_in6
#endif

/*** IPv6 structures that might be missing from some implementations ***/

/** @internal
 * An IPv6 address.
 * This is taken from RFC 2553
 */
struct kde_in6_addr
{
  unsigned char __u6_addr[16];
};

/** @internal
 * An IPv6 socket address
 * This is taken from RFC 2553.
 */
struct kde_sockaddr_in6
{
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
  quint8		sin6_len;
  quint8		sin6_family;
#else  //HAVE_STRUCT_SOCKADDR_SA_LEN
  quint16		sin6_family;
#endif
  unsigned short       	sin6_port;	/* RFC says in_port_t */
  quint32		sin6_flowinfo;
  struct kde_in6_addr	sin6_addr;
  quint32		sin6_scope_id;
};

/* IPv6 test macros that could be missing from some implementations */

#define KDE_IN6_IS_ADDR_UNSPECIFIED(a) \
	(((quint32 *) (a))[0] == 0 && ((quint32 *) (a))[1] == 0 && \
	 ((quint32 *) (a))[2] == 0 && ((quint32 *) (a))[3] == 0)

#define KDE_IN6_IS_ADDR_LOOPBACK(a) \
	(((quint32 *) (a))[0] == 0 && ((quint32 *) (a))[1] == 0 && \
	 ((quint32 *) (a))[2] == 0 && ((quint32 *) (a))[3] == htonl (1))

#define KDE_IN6_IS_ADDR_MULTICAST(a) (((u_int8_t *) (a))[0] == 0xff)

#define KDE_IN6_IS_ADDR_LINKLOCAL(a) \
	((((quint32 *) (a))[0] & htonl (0xffc00000)) == htonl (0xfe800000))

#define KDE_IN6_IS_ADDR_SITELOCAL(a) \
	((((quint32 *) (a))[0] & htonl (0xffc00000)) == htonl (0xfec00000))

#define KDE_IN6_IS_ADDR_V4MAPPED(a) \
	((((quint32 *) (a))[0] == 0) && (((quint32 *) (a))[1] == 0) && \
	 (((quint32 *) (a))[2] == htonl (0xffff)))

#define KDE_IN6_IS_ADDR_V4COMPAT(a) \
	((((quint32 *) (a))[0] == 0) && (((quint32 *) (a))[1] == 0) && \
	 (((quint32 *) (a))[2] == 0) && (ntohl (((quint32 *) (a))[3]) > 1))

#define KDE_IN6_ARE_ADDR_EQUAL(a,b) \
	((((quint32 *) (a))[0] == ((quint32 *) (b))[0]) && \
	 (((quint32 *) (a))[1] == ((quint32 *) (b))[1]) && \
	 (((quint32 *) (a))[2] == ((quint32 *) (b))[2]) && \
	 (((quint32 *) (a))[3] == ((quint32 *) (b))[3]))

#define KDE_IN6_IS_ADDR_MC_NODELOCAL(a) \
	(KDE_IN6_IS_ADDR_MULTICAST(a) && ((((quint8 *) (a))[1] & 0xf) == 0x1))

#define KDE_IN6_IS_ADDR_MC_LINKLOCAL(a) \
	(KDE_IN6_IS_ADDR_MULTICAST(a) && ((((quint8 *) (a))[1] & 0xf) == 0x2))

#define KDE_IN6_IS_ADDR_MC_SITELOCAL(a) \
	(KDE_IN6_IS_ADDR_MULTICAST(a) && ((((quint8 *) (a))[1] & 0xf) == 0x5))

#define KDE_IN6_IS_ADDR_MC_ORGLOCAL(a) \
	(KDE_IN6_IS_ADDR_MULTICAST(a) && ((((quint8 *) (a))[1] & 0xf) == 0x8))

#define KDE_IN6_IS_ADDR_MC_GLOBAL(a) \
	(KDE_IN6_IS_ADDR_MULTICAST(a) && ((((quint8 *) (a))[1] & 0xf) == 0xe))

#ifdef NEED_IN6_TESTS
# define IN6_IS_ADDR_UNSPECIFIED	KDE_IN6_IS_ADDR_UNSPECIFIED
# define IN6_IS_ADDR_LOOPBACK		KDE_IN6_IS_ADDR_LOOPBACK
# define IN6_IS_ADDR_MULTICAST		KDE_IN6_IS_ADDR_MULTICAST
# define IN6_IS_ADDR_LINKLOCAL		KDE_IN6_IS_ADDR_LINKLOCAL
# define IN6_IS_ADDR_SITELOCAL		KDE_IN6_IS_ADDR_SITELOCAL
# define IN6_IS_ADDR_V4MAPPED		KDE_IN6_IS_ADDR_V4MAPPED
# define IN6_IS_ADDR_V4COMPAT		KDE_IN6_IS_ADDR_V4COMPAT
# define IN6_ARE_ADDR_EQUAL		KDE_IN6_ARE_ADDR_EQUAL
# define IN6_IS_ADDR_MC_NODELOCAL	KDE_IN6_IS_ADDR_MC_NODELOCAL
# define IN6_IS_ADDR_MC_LINKLOCAL	KDE_IN6_IS_ADDR_MC_LINKLOCAL
# define IN6_IS_ADDR_MC_SITELOCAL	KDE_IN6_IS_ADDR_MC_SITELOCAL
# define IN6_IS_ADDR_MC_ORGLOCAL	KDE_IN6_IS_ADDR_MC_ORGLOCAL
# define IN6_IS_ADDR_MC_GLOBAL		KDE_IN6_IS_ADDR_MC_GLOBAL
#endif

/* Special internal structure */

#define KAI_SYSTEM		0	/* data is all-system */
#define KAI_LOCALUNIX		1	/* data contains a Unix addrinfo allocated by us */
#define KAI_QDNS		2	/* data contains data derived from QDns */

struct addrinfo;		/* forward declaration; this could be needed */

/**
 * @internal
 * Special purpose structure, to return data from kde_getaddrinfo to the
 * library functions. This defines an extra field to let us know how to
 * process this better.
 *
 * Currently, we use it to determine how to deallocate this stuff
 */
struct kde_addrinfo
{
  struct addrinfo *data;
  int origin;
};

extern KDECORE_EXPORT int kde_getaddrinfo(const char *name, const char *service,
			   const struct addrinfo* hint,
			   struct kde_addrinfo** result);
extern KDECORE_EXPORT void kde_freeaddrinfo(struct kde_addrinfo *p);

#if !defined(HAVE_GETADDRINFO) || defined(HAVE_BROKEN_GETADDRINFO)

# ifndef HAVE_STRUCT_ADDRINFO
/**
 * @internal
 */
struct addrinfo
{
  int ai_flags;			/* Input flags.  */
  int ai_family;		/* Protocol family for socket.  */
  int ai_socktype;		/* Socket type.  */
  int ai_protocol;		/* Protocol for socket.  */
  int ai_addrlen;		/* Length of socket address.  */
  struct sockaddr *ai_addr;	/* Socket address for socket.  */
  char *ai_canonname;		/* Canonical name for service location.  */
  struct addrinfo *ai_next;	/* Pointer to next in list.  */
};
# endif

# ifdef AI_PASSIVE
#  undef AI_PASSIVE
#  undef AI_CANONNAME
#  undef AI_NUMERICHOST
# endif

/* Possible values for `ai_flags' field in `addrinfo' structure.  */
# define AI_PASSIVE	1	/* Socket address is intended for `bind'.  */
# define AI_CANONNAME	2	/* Request for canonical name.  */
# define AI_NUMERICHOST	4	/* Don't use name resolution.  */

# ifdef EAI_ADDRFAMILY
#  undef EAI_ADDRFAMILY
#  undef EAI_AGAIN
#  undef EAI_BADFLAGS
#  undef EAI_FAIL
#  undef EAI_FAMILY
#  undef EAI_MEMORY
#  undef EAI_NODATA
#  undef EAI_NONAME
#  undef EAI_SERVICE
#  undef EAI_SOCKTYPE
#  undef EAI_SYSTEM
# endif

/* Error values for `getaddrinfo' function.  */
# ifndef EAI_ADDRFAMILY
#  define EAI_ADDRFAMILY	1	/* Address family for NAME not supported.  */
# endif
# ifndef EAI_AGAIN
#  define EAI_AGAIN	2	/* Temporary failure in name resolution.  */
# endif
# ifndef EAI_BADFLAGS
#  define EAI_BADFLAGS	3	/* Invalid value for `ai_flags' field.  */
# endif
# ifndef EAI_FAIL
#  define EAI_FAIL	4	/* Non-recoverable failure in name res.  */
# endif
# ifndef EAI_FAMILY
#  define EAI_FAMILY	5	/* `ai_family' not supported.  */
# endif
# ifndef EAI_MEMORY
#  define EAI_MEMORY	6 	/* Memory allocation failure.  */
# endif
# ifndef EAI_NODATA
#  define EAI_NODATA	7	/* No address associated with NAME.  */
# endif
# ifndef EAI_NONAME
#  define EAI_NONAME	8	/* NAME or SERVICE is unknown.  */
# endif
# ifndef EAI_SERVICE
#  define EAI_SERVICE	9	/* SERVICE not supported for `ai_socktype'.  */
# endif
# ifndef EAI_SOCKTYPE
#  define EAI_SOCKTYPE	10	/* `ai_socktype' not supported.  */
# endif
# ifndef EAI_SYSTEM
#  define EAI_SYSTEM	11	/* System error returned in `errno'.  */
# endif

/*
 * These are specified in the RFC
 * We won't undefine them. If someone defined them to a different value
 * the preprocessor will generate an error
 */
# define NI_MAXHOST	1025
# define NI_MAXSERV	32

# ifdef NI_NUMERICHOST
#  undef NI_NUMERICHOST
#  undef NI_NUMERICSERV
#  undef NI_NOFQDN
#  undef NI_NAMEREQD
#  undef NI_DGRAM
# endif

# define NI_NUMERICHOST	1	/* Don't try to look up hostname.  */
# define NI_NUMERICSERV 2	/* Don't convert port number to name.  */
# define NI_NOFQDN	4	/* Only return nodename portion.  */
# define NI_NAMEREQD	8	/* Don't return numeric addresses.  */
# define NI_DGRAM	16	/* Look up UDP service rather than TCP.  */

# ifdef gai_strerror
#  undef gai_strerror
# endif

# ifdef getaddrinfo
#  undef getaddrinfo
# endif

# ifdef freeaddrinfo
#  undef freeaddrinfo
# endif

# ifdef getnameinfo
#  undef getnameinfo
# endif

namespace KDE
{
  /** \internal */
  extern KDECORE_EXPORT int getaddrinfo(const char *name, const char *service,
			 const struct addrinfo* hint,
			 struct addrinfo** result);
  /** \internal */
  extern KDECORE_EXPORT void freeaddrinfo(struct addrinfo* ai);
  /** \internal */
  extern KDECORE_EXPORT int getnameinfo(const struct sockaddr *sa,
			 kde_socklen_t salen,
			 char *host, size_t hostlen,
			 char *serv, size_t servlen,
			 int flags);
}

# ifndef HAVE_GAI_STRERROR_PROTO
/** \internal */
extern KDECORE_EXPORT char *gai_strerror(int errorcode);
# endif
  
# define getaddrinfo	KDE::getaddrinfo
# define freeaddrinfo	KDE::freeaddrinfo
# define getnameinfo	KDE::getnameinfo


#endif

#ifndef HAVE_INET_PTON

namespace KDE
{
  /** \internal */
  extern KDECORE_EXPORT int inet_pton(int af, const char *cp, void* buf);
}

# define inet_pton	KDE::inet_pton
#endif

#ifndef HAVE_INET_NTOP

namespace KDE
{
  /** \internal */
  extern KDECORE_EXPORT const char* inet_ntop(int af, const void *cp, char *buf, size_t len);
}

# define inet_ntop	KDE::inet_ntop
#endif

#endif
