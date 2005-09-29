/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2003 Thiago Macieira <thiago.macieira@kdemail.net>>
 *
 *  win32 version of netsupp.cpp
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


#include "netsupp.h"

KDECORE_EXPORT void kde_freeaddrinfo(struct kde_addrinfo *ai)
{
	//TODO
}

KDECORE_EXPORT int kde_getaddrinfo(const char *name, const char *service,
		    const struct addrinfo* hint,
		    struct kde_addrinfo** result)
{
	//TODO
	int err = EAI_SERVICE;
	return err;
}

KDECORE_EXPORT char *gai_strerror(int errorcode)
{
  return 0;
}
int getnameinfo(const struct sockaddr *sa,
			 unsigned int salen,
			 char *host, size_t hostlen,
			 char *serv, size_t servlen,
			 int flags)
{
	//TODO
	return -1;
}

KDECORE_EXPORT const char* inet_ntop(int af, const void *cp, char *buf, size_t len)
{
	//TODO
	return 0;
}

KDECORE_EXPORT int inet_pton(int af, const char *cp, void *buf)
{
	//TODO
	return -1;
}

