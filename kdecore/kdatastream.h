#ifndef __kdatastream__h
#define __kdatastream__h

#include <qdatastream.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if SIZEOF_LONG == 8
inline QDataStream & operator << (QDataStream & str, long l)
{
  str << Q_INT32((l & 0xffffffff00000000) >> 32);
  str << Q_INT32(l & 0x00000000ffffffff);
  return str;
}

inline QDataStream & operator >> (QDataStream & str, long & l)
{
  Q_INT32 hi, lo;
  str >> hi;
  l = hi << 32;
  str >> lo;
  l |= lo;
  return str;
}
#else
#if SIZEOF_LONG == 4

inline QDataStream & operator << (QDataStream & str, long l)
{
  str << Q_INT32(l);
  return str;
}

inline QDataStream & operator >> (QDataStream & str, long & l)
{
  Q_INT32 i;
  str >> i;
  l = long(i);
  return str;
}

#else
#warning SIZEOF_LONG is not 4 nor 8 ?
#endif
#endif

inline QDataStream & operator << (QDataStream & str, unsigned long l)
{
  str << (signed long)l;
  return str;
}

inline QDataStream & operator >> (QDataStream & str, unsigned long & l)
{
  long sl;
  str >> sl;
  l = (unsigned long)sl;
  return str;
}

inline QDataStream & operator << (QDataStream & str, bool b)
{
  str << Q_INT32(b);
  return str;
}

inline QDataStream & operator >> (QDataStream & str, bool & b)
{
  Q_INT32 l;
  str >> l;
  b = bool(l);
  return str;
}

#endif
