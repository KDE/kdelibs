/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KDEWIN_MACHTYPES_H
#define KDEWIN_MACHTYPES_H

// include everywhere
#include <sys/types.h>

/*
 *  The following section is RTEMS specific and is needed to more
 *  closely match the types defined in the BSD machine/types.h.
 *  This is needed to let the RTEMS/BSD TCP/IP stack compile.
 */

#if defined(__rtems__)
typedef signed long long   int64_t;
#if defined( __h8300__)
typedef signed long        int32_t;
#else
typedef signed int         int32_t;
#endif
typedef signed short       int16_t;
typedef signed char        int8_t;

typedef unsigned long long u_int64_t;
#if defined( __h8300__)
typedef unsigned long      u_int32_t;
#else
typedef unsigned int       u_int32_t;
#endif
typedef unsigned short     u_int16_t;
typedef unsigned char      u_int8_t;
#endif

#define	_CLOCK_T_	unsigned long		/* clock() */
#define	_TIME_T_	long			/* time() */
#define _CLOCKID_T_ 	unsigned long
#define _TIMER_T_   	unsigned long

#ifndef _HAVE_SYSTYPES
typedef long int __off_t;
typedef int __pid_t;
#ifdef __GNUC__
__extension__ typedef long long int __loff_t;
#else
typedef long int __loff_t;
#endif
#endif

#endif	// KDEWIN_MACHTYPES_H
