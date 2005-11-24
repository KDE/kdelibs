/* This file is part of the KDE project
   Copyright (C) 2003-2005 Jaroslaw Staniek <js@iidea.pl>
   Copyright (C) 2005 Christian Ehrlicher <Ch.Ehrlicher@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <sys/time.h>
#include <windows.h>

#define KDE_SECONDS_SINCE_1601	11644473600LL
#define KDE_USEC_IN_SEC			1000000LL

KDEWIN32_EXPORT int gettimeofday(struct timeval *__p, struct timezone *__t)
{
	union {
		unsigned long long ns100; /*time since 1 Jan 1601 in 100ns units */
		FILETIME ft;
	} now;
	
	GetSystemTimeAsFileTime (&now.ft);
	__p->tv_usec = (long) ((now.ns100 / 10LL) % (KDE_USEC_IN_SEC * 10));
	__p->tv_sec  = (long)(((now.ns100 / 10LL ) / KDE_USEC_IN_SEC) - KDE_SECONDS_SINCE_1601);
	
	return (0); 
}

KDEWIN32_EXPORT int settimeofday(const struct timeval *__p, const struct timezone *__t)
{
	union {
		unsigned long long ns100; /*time since 1 Jan 1601 in 100ns units */
		FILETIME ft;
	} now;
	SYSTEMTIME st;
	
	now.ns100 = ((((__p->tv_sec + KDE_SECONDS_SINCE_1601) * KDE_USEC_IN_SEC) + __p->tv_usec) * 10LL);
	
	FileTimeToSystemTime( &now.ft, &st );
	SetSystemTime( &st );
	return 1;
}

KDEWIN32_EXPORT struct tm* localtime_r(const time_t *t,struct tm *p)
{
	*p = *localtime(t);
	return p; 
}

