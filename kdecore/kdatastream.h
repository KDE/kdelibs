#ifndef __kdatastream__h
#define __kdatastream__h

#include <qdatastream.h>

inline QDataStream & operator << (QDataStream & str, bool b)
{
  str << Q_INT8(b);
  return str;
}

inline QDataStream & operator >> (QDataStream & str, bool & b)
{
  Q_INT8 l;
  str >> l;
  b = bool(l);
  return str;
}

inline QDataStream & operator << (QDataStream & str, long long int ll)
{
  Q_INT32 l1,l2;
  l1 = ll & 0xffffffffLL;
  l2 = ll >> 32;
  str << l1 << l2;
  return str;
}

inline QDataStream & operator >> (QDataStream & str, long long int&ll)
{
  Q_INT32 l1,l2;
  str >> l1 >> l2;
  ll = ((long long int)(l2) << 32) + (long long int) l1;
  return str;
}

inline QDataStream & operator << (QDataStream & str, unsigned long long int ll)
{
  Q_UINT32 l1,l2;
  l1 = ll & 0xffffffffLL;
  l2 = ll >> 32;
  str << l1 << l2;
  return str;
}

inline QDataStream & operator >> (QDataStream & str, unsigned long long int &ll)
{
  Q_UINT32 l1,l2;
  str >> l1 >> l2;
  ll = ((unsigned long long int)(l2) << 32) + (unsigned long long int) l1;
  return str;
}

#endif
