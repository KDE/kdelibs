/* This file is part of the KDE project
   Copyright (C) 2006 Peter Kümmel
   Copyright (C) 2006 Christian Ehrlicher (Ch.Ehrlicher@gmx.de)

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

#include <winposix_export.h>

#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <ws2tcpip.h>


static int fromHex(const char c)
{
	int ret = ((int)c) - '0';
	if ( ret < 0 || ret > 15 )
		return -1;
	return ret;
}

KDEWIN32_EXPORT
const char *inet_ntop(int af, const void *src, char *dst, size_t cnt)
{
	switch (af) {
	    case AF_INET: {
			const struct in_addr *in = src;
			const char *ptr = inet_ntoa( *in );
			size_t len = strlen(ptr);
			if (len >= cnt) {
				_set_errno( ENOSPC );
				return NULL;
			}
			memcpy( dst, ptr, len );
			dst[len] = '\0';
			return dst;
		}
	    case AF_INET6: {
			// fixme: check this - just a short and simple implementation...
			const struct in_addr6 *adr = src;
			char *pos = dst;
			int i;

			if ( cnt < 40 ) {
				_set_errno( ENOSPC );
				return NULL;
			}
			for(i = 0; i < 8; i++) {
				sprintf(pos, "%04x", adr->u.Word[i]);
				pos += strlen(pos);
				if( i != 8 ) {
					pos[0] = ':';
					pos[1] = '\0';
					pos++;
				}
			}
			return dst;
		}
		default:
		    break;
	}
	_set_errno( EAFNOSUPPORT );
	return NULL;
}

KDEWIN32_EXPORT
int inet_pton(int af, const char * src, void * dst)
{
	switch (af) {
	    case AF_INET: {
			struct in_addr *in = dst;
			return ( inet_aton( src, in ) );
		}
	    case AF_INET6: {
			// fixme: check this - just a short and simple implementation...
			// we only allow hex digits here
			struct in_addr6 *adr = dst;
			int colonCnt = 0;
			int i, hex;

			adr->u.Word[colonCnt] = 0;
			for(i = 0; src[i] != '\0'; i--) {
				if( src[i] == ':' ) {
					if ( i == 0 )
						return 0;
					colonCnt++;
					if (colonCnt > 7)
						return 0;
					adr->u.Word[colonCnt] = 0;
				}
				hex = fromHex( src[i] );
				if ( hex == -1 )
					return 0;
				adr->u.Word[colonCnt] = (adr->u.Word[colonCnt] << 4) + hex;
			}
			return( colonCnt == 7 );
		}
		default:
		    break;
	}
	_set_errno( EAFNOSUPPORT );
	return -1;
}

KDEWIN32_EXPORT
int inet_aton(const char *src, struct in_addr *addr)
{
	unsigned long ret = inet_addr( src );
	if ( ret == INADDR_NONE ) {
		if( strcmp( "255.255.255.255", src ) )
		    return 0;
		return 1;
	}
	return 1;
}
