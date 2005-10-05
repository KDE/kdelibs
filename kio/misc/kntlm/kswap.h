/*
   This file is part of the KDE libraries.
   Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSWAP_H
#define KSWAP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qglobal.h>

/** 
 * \defgroup KSWAP Byte-swapping functions
 * kswap.h contains functions that will help converting
 * 16, 32 and 64 bit length data between little-endian and
 * big-endian representations.
 *
 * The KSWAP_16, KSWAP_32 and KSWAP_64 functions are always
 * swaps the byte order of the supplied argument (which should be
 * 16, 32 or 64 bit wide). These functions are inline, and tries to
 * use the most optimized function of the underlying system
 * (bswap_xx functions from byteswap.h in GLIBC, or ntohs and ntohl
 * on little-endian machines, and if neither are applicable, some fast
 * custom code).
 *
 * The KFromTo{Little|Big}Endian functions are for converting big-endian and 
 * little-endian data to and from the machine endianness.
 */

#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>

  inline Q_UINT16 KSWAP_16( Q_UINT16 b ) { return bswap_16( b ); }
  inline Q_INT16 KSWAP_16( Q_INT16 b ) { return bswap_16( (Q_UINT16)b ); }
  inline Q_UINT32 KSWAP_32( Q_UINT32 b ) { return bswap_32( b ); }
  inline Q_INT32 KSWAP_32( Q_INT32 b ) { return bswap_32( (Q_UINT32)b ); }
  inline Q_UINT64 KSWAP_64( Q_UINT64 b ) { return bswap_64( b ); }
  inline Q_INT64 KSWAP_64( Q_INT64 b ) { return bswap_64( (Q_UINT64)b ); }

#else /* HAVE_BYTESWAP_H */
#ifdef WORDS_BIGENDIAN
  inline Q_UINT16 KSWAP_16( Q_UINT16 b ) 
  { 
    return (((b) & 0x00ff) << 8 | ((b) & 0xff00) >> 8); 
  }

  inline Q_INT16 KSWAP_16( Q_INT16 b ) 
  { 
    return ((((Q_UINT16)b) & 0x00ff) << 8 | (((Q_UINT16)b) & 0xff00) >> 8); 
  }

  inline Q_UINT32 KSWAP_32( Q_UINT32 b ) 
  {
    return
      ((((b) & 0xff000000) >> 24) | (((b) & 0x00ff0000) >>  8) | \
       (((b) & 0x0000ff00) <<  8) | (((b) & 0x000000ff) << 24)); 
  }

  inline Q_INT32 KSWAP_32( Q_INT32 b ) 
  {
    return 
      (((((Q_UINT32)b) & 0xff000000) >> 24) | ((((Q_UINT32)b) & 0x00ff0000) >>  8) | \
       ((((Q_UINT32)b) & 0x0000ff00) <<  8) | ((((Q_UINT32)b) & 0x000000ff) << 24)); 
  }
#else /* WORDS_BIGENDIAN */
#include <sys/types.h>
#include <netinet/in.h>

  inline Q_UINT16 KSWAP_16( Q_UINT16 b ) { return htons(b); }
  inline Q_INT16 KSWAP_16( Q_INT16 b ) { return htons((Q_UINT16)b); }
  inline Q_UINT32 KSWAP_32( Q_UINT32 b ) { return htonl(b); }
  inline Q_INT32 KSWAP_32( Q_INT32 b ) { return htonl((Q_UINT32)b); }
#endif
  inline Q_UINT64 KSWAP_64( Q_UINT64 b ) 
  {
    union { 
        Q_UINT64 ll;
        Q_UINT32 l[2]; 
    } w, r;
    w.ll = b;
    r.l[0] = KSWAP_32( w.l[1] );
    r.l[1] = KSWAP_32( w.l[0] );
    return r.ll;
  }

  inline Q_INT64 KSWAP_64( Q_INT64 b ) 
  {
    union { 
        Q_UINT64 ll;
        Q_UINT32 l[2]; 
    } w, r;
    w.ll = (Q_UINT64) b;
    r.l[0] = KSWAP_32( w.l[1] );
    r.l[1] = KSWAP_32( w.l[0] );
    return r.ll;
  }
#endif	/* !HAVE_BYTESWAP_H */

/**
 * \ingroup KSWAP
 * Converts a 16 bit unsigned value from/to big-endian byte order to/from the machine order.
 */
inline Q_UINT16 KFromToBigEndian( Q_UINT16 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_16(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit unsigned array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( Q_UINT16 *out, Q_UINT16 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<1 ) ;
#else
  while ( len>0 ) { *out = KSWAP_16( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit unsigned value from/to big-endian byte order to/from the machine order.
 */
inline Q_UINT32 KFromToBigEndian( Q_UINT32 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_32(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit unsigned array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( Q_UINT32 *out, Q_UINT32 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<2 ) ;
#else
  while ( len>0 ) { *out = KSWAP_32( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit unsigned value from/to big-endian byte order to/from the machine order.
 */
inline Q_UINT64 KFromToBigEndian( Q_UINT64 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_64(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit unsigned array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( Q_UINT64 *out, Q_UINT64 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<3 ) ;
#else
  while ( len>0 ) { *out = KSWAP_64( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit signed value from/to big-endian byte order to/from the machine order.
 */
inline Q_INT16 KFromToBigEndian( Q_INT16 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_16(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit signed array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( Q_INT16 *out, Q_INT16 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<1 ) ;
#else
  while ( len>0 ) { *out = KSWAP_16( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit signed value from/to big-endian byte order to/from the machine order.
 */
inline Q_INT32 KFromToBigEndian( Q_INT32 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_32(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit signed array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( Q_INT32 *out, Q_INT32 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<2 ) ;
#else
  while ( len>0 ) { *out = KSWAP_32( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit signed value from/to big-endian byte order to/from the machine order.
 */
inline Q_INT64 KFromToBigEndian( Q_INT64 b )
{
#ifdef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_64(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit signed array from/to big-endian byte order to/from the machine order.
 */
inline void KFromToBigEndian( Q_INT64 *out, Q_INT64 *in, uint len )
{
#ifdef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<3 ) ;
#else
  while ( len>0 ) { *out = KSWAP_64( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit unsigned value from/to little-endian byte order to/from the machine order.
 */
inline Q_UINT16 KFromToLittleEndian( Q_UINT16 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_16(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit unsigned array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( Q_UINT16 *out, Q_UINT16 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<1 ) ;
#else
  while ( len>0 ) { *out = KSWAP_16( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit unsigned value from/to little-endian byte order to/from the machine order.
 */
inline Q_UINT32 KFromToLittleEndian( Q_UINT32 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_32(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit unsigned array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( Q_UINT32 *out, Q_UINT32 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<2 ) ;
#else
  while ( len>0 ) { *out = KSWAP_32( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit unsigned value from/to little-endian byte order to/from the machine order.
 */
inline Q_UINT64 KFromToLittleEndian( Q_UINT64 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_64(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit unsigned array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( Q_UINT64 *out, Q_UINT64 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<3 ) ;
#else
  while ( len>0 ) { *out = KSWAP_64( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit signed value from/to little-endian byte order to/from the machine order.
 */
inline Q_INT16 KFromToLittleEndian( Q_INT16 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_16(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 16 bit signed array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( Q_INT16 *out, Q_INT16 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<1 ) ;
#else
  while ( len>0 ) { *out = KSWAP_16( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit signed value from/to little-endian byte order to/from the machine order.
 */
inline Q_INT32 KFromToLittleEndian( Q_INT32 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_32(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 32 bit signed array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( Q_INT32 *out, Q_INT32 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<2 ) ;
#else
  while ( len>0 ) { *out = KSWAP_32( *in ); out++; in++; len--; }
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit signed value from/to little-endian byte order to/from the machine order.
 */
inline Q_INT64 KFromToLittleEndian( Q_INT64 b )
{
#ifndef WORDS_BIGENDIAN
  return b;
#else
  return KSWAP_64(b);
#endif
}

/**
 * \ingroup KSWAP
 * Converts a 64 bit signed array from/to little-endian byte order to/from the machine order.
 */
inline void KFromToLittleEndian( Q_INT64 *out, Q_INT64 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<3 ) ;
#else
  while ( len>0 ) { *out = KSWAP_64( *in ); out++; in++; len--; }
#endif
}

#endif /* KSWAP_H */
