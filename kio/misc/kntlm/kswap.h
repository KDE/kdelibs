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

  inline quint16 KSWAP_16( quint16 b ) { return bswap_16( b ); }
  inline qint16 KSWAP_16( qint16 b ) { return bswap_16( (quint16)b ); }
  inline quint32 KSWAP_32( quint32 b ) { return bswap_32( b ); }
  inline qint32 KSWAP_32( qint32 b ) { return bswap_32( (quint32)b ); }
  inline quint64 KSWAP_64( quint64 b ) { return bswap_64( b ); }
  inline qint64 KSWAP_64( qint64 b ) { return bswap_64( (quint64)b ); }

#else /* HAVE_BYTESWAP_H */
#ifdef WORDS_BIGENDIAN
  inline quint16 KSWAP_16( quint16 b ) 
  { 
    return (((b) & 0x00ff) << 8 | ((b) & 0xff00) >> 8); 
  }

  inline qint16 KSWAP_16( qint16 b ) 
  { 
    return ((((quint16)b) & 0x00ff) << 8 | (((quint16)b) & 0xff00) >> 8); 
  }

  inline quint32 KSWAP_32( quint32 b ) 
  {
    return
      ((((b) & 0xff000000) >> 24) | (((b) & 0x00ff0000) >>  8) | \
       (((b) & 0x0000ff00) <<  8) | (((b) & 0x000000ff) << 24)); 
  }

  inline qint32 KSWAP_32( qint32 b ) 
  {
    return 
      (((((quint32)b) & 0xff000000) >> 24) | ((((quint32)b) & 0x00ff0000) >>  8) | \
       ((((quint32)b) & 0x0000ff00) <<  8) | ((((quint32)b) & 0x000000ff) << 24)); 
  }
#else /* WORDS_BIGENDIAN */
#include <sys/types.h>
#include <netinet/in.h>

  inline quint16 KSWAP_16( quint16 b ) { return htons(b); }
  inline qint16 KSWAP_16( qint16 b ) { return htons((quint16)b); }
  inline quint32 KSWAP_32( quint32 b ) { return htonl(b); }
  inline qint32 KSWAP_32( qint32 b ) { return htonl((quint32)b); }
#endif
  inline quint64 KSWAP_64( quint64 b ) 
  {
    union { 
        quint64 ll;
        quint32 l[2]; 
    } w, r;
    w.ll = b;
    r.l[0] = KSWAP_32( w.l[1] );
    r.l[1] = KSWAP_32( w.l[0] );
    return r.ll;
  }

  inline qint64 KSWAP_64( qint64 b ) 
  {
    union { 
        quint64 ll;
        quint32 l[2]; 
    } w, r;
    w.ll = (quint64) b;
    r.l[0] = KSWAP_32( w.l[1] );
    r.l[1] = KSWAP_32( w.l[0] );
    return r.ll;
  }
#endif	/* !HAVE_BYTESWAP_H */

/**
 * \ingroup KSWAP
 * Converts a 16 bit unsigned value from/to big-endian byte order to/from the machine order.
 */
inline quint16 KFromToBigEndian( quint16 b )
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
inline void KFromToBigEndian( quint16 *out, quint16 *in, uint len )
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
inline quint32 KFromToBigEndian( quint32 b )
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
inline void KFromToBigEndian( quint32 *out, quint32 *in, uint len )
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
inline quint64 KFromToBigEndian( quint64 b )
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
inline void KFromToBigEndian( quint64 *out, quint64 *in, uint len )
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
inline qint16 KFromToBigEndian( qint16 b )
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
inline void KFromToBigEndian( qint16 *out, qint16 *in, uint len )
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
inline qint32 KFromToBigEndian( qint32 b )
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
inline void KFromToBigEndian( qint32 *out, qint32 *in, uint len )
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
inline qint64 KFromToBigEndian( qint64 b )
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
inline void KFromToBigEndian( qint64 *out, qint64 *in, uint len )
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
inline quint16 KFromToLittleEndian( quint16 b )
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
inline void KFromToLittleEndian( quint16 *out, quint16 *in, uint len )
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
inline quint32 KFromToLittleEndian( quint32 b )
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
inline void KFromToLittleEndian( quint32 *out, quint32 *in, uint len )
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
inline quint64 KFromToLittleEndian( quint64 b )
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
inline void KFromToLittleEndian( quint64 *out, quint64 *in, uint len )
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
inline qint16 KFromToLittleEndian( qint16 b )
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
inline void KFromToLittleEndian( qint16 *out, qint16 *in, uint len )
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
inline qint32 KFromToLittleEndian( qint32 b )
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
inline void KFromToLittleEndian( qint32 *out, qint32 *in, uint len )
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
inline qint64 KFromToLittleEndian( qint64 b )
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
inline void KFromToLittleEndian( qint64 *out, qint64 *in, uint len )
{
#ifndef WORDS_BIGENDIAN
  if ( out != in ) memcpy( out, in, len<<3 ) ;
#else
  while ( len>0 ) { *out = KSWAP_64( *in ); out++; in++; len--; }
#endif
}

#endif /* KSWAP_H */
