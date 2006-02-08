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

#ifndef _BYTEORDER_H
#define _BYTEORDER_H

#undef INCL_WINSOCK_API_PROTOTYPES
#define INCL_WINSOCK_API_PROTOTYPES 1 //for ntohl(), etc.
#include <winsock2.h>
#include "fixwinh.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#undef ntohl
#undef ntohs
#undef htonl
#undef htons
#endif

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif

#ifndef __LITTLE_ENDIAN_BITFIELD
#define __LITTLE_ENDIAN_BITFIELD
#endif

#if 1
//defined in winsock: extern unsigned long int	ntohl(unsigned long int);
//defined in winsock: extern unsigned short int	ntohs(unsigned short int);
//defined in winsock: extern unsigned long int	htonl(unsigned long int);
//defined in winsock: extern unsigned short int	htons(unsigned short int);

/*
extern __inline__ unsigned long int	__ntohl(unsigned long int);
extern __inline__ unsigned short int	__ntohs(unsigned short int);
extern __inline__ unsigned long int	__constant_ntohl(unsigned long int);
extern __inline__ unsigned short int	__constant_ntohs(unsigned short int);
*/

/*
extern __inline__ unsigned long int
__ntohl(unsigned long int x)
{
	__asm__("xchgb %b0,%h0\n\t"	// swap lower bytes	
		"rorl $16,%0\n\t"	// swap words		
		"xchgb %b0,%h0"		// swap higher bytes	
		:"=q" (x)
		: "0" (x));
	return x;
}
*/

#define __constant_ntohl(x) \
	((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
			     (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
			     (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
			     (((unsigned long int)(x) & 0xff000000U) >> 24)))
/*
extern __inline__ unsigned short int
__ntohs(unsigned short int x)
{
	__asm__("xchgb %b0,%h0"		// swap bytes		
		: "=q" (x)
		:  "0" (x));
	return x;
}
*/
#define __constant_ntohs(x) \
	((unsigned short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
			      (((unsigned short int)(x) & 0xff00) >> 8))) \

#define __htonl(x) __ntohl(x)
#define __htons(x) __ntohs(x)
#define __constant_htonl(x) __constant_ntohl(x)
#define __constant_htons(x) __constant_ntohs(x)

#ifdef  __OPTIMIZE__
#  define ntohl(x) \
(__builtin_constant_p((long)(x)) ? \
 __constant_ntohl((x)) : \
 __ntohl((x)))
#  define ntohs(x) \
(__builtin_constant_p((short)(x)) ? \
 __constant_ntohs((x)) : \
 __ntohs((x)))
#  define htonl(x) \
(__builtin_constant_p((long)(x)) ? \
 __constant_htonl((x)) : \
 __htonl((x)))
#  define htons(x) \
(__builtin_constant_p((short)(x)) ? \
 __constant_htons((x)) : \
 __htons((x)))
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
