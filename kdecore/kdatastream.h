#ifndef __kdatastream__h
#define __kdatastream__h

#include <qdatastream.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

QDataStream & operator << (QDataStream & str, long l);

QDataStream & operator >> (QDataStream & str, long & l);

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
