#include <qdatastream.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if SIZEOF_LONG == 8
QDataStream & operator << (QDataStream & str, unsigned long l)
{
  return operator << (str, (signed)l);
}

QDataStream & operator >> (QDataStream & str, unsigned long & l)
{
  return operator >> (str, (signed)l);
}

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
#endif

QDataStream & operator << (QDataStream & str, bool b)
{
  Q_INT32 l = Q_INT32(b);
  str << l;
  return str;
}

QDataStream & operator >> (QDataStream & str, bool & b)
{
  Q_INT32 l;
  str >> l;
  b = bool(l);
  return str;
}
