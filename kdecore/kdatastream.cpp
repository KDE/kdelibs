
#include <kdatastream.h>

#include "config.h"

#warning "Remove operator<<(QDataStream &, long) before Qt 2.2. release"

#if SIZEOF_LONG == 8
QDataStream & operator << (QDataStream & str, long l)
{
  str << Q_INT32((l & 0xffffffff00000000) >> 32);
  str << Q_INT32(l & 0x00000000ffffffff);
  return str;
}

QDataStream & operator >> (QDataStream & str, long & l)
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

QDataStream & operator << (QDataStream & str, long l)
{
  str << Q_INT32(l);
  return str;
}

QDataStream & operator >> (QDataStream & str, long & l)
{
  Q_INT32 i;
  str >> i;
  l = long(i);
  return str;
}

#else
#error SIZEOF_LONG is not 4 nor 8, check config.h
#endif
#endif

