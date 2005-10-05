// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "ustring.h"
#include "operations.h"
#include "identifier.h"
#include <math.h>
#include "dtoa.h"

namespace KJS {
  extern const double NaN;
  extern const double Inf;
}

using namespace KJS;

CString::CString(const char *c)
{
  length = strlen(c);
  data = new char[length+1];
  memcpy(data, c, length + 1);
}

CString::CString(const char *c, int len)
{
  length = len;
  data = new char[len+1];
  memcpy(data, c, len);
  data[len] = 0;
}

CString::CString(const CString &b)
{
  length = b.length;
  data = new char[length+1];
  memcpy(data, b.data, length + 1);
}

CString::~CString()
{
  delete [] data;
}

CString &CString::append(const CString &t)
{
  char *n = new char[length + t.length + 1];
  if (length)
    memcpy(n, data, length);
  if (t.length)
    memcpy(n+length, t.data, t.length);
  length += t.length;
  n[length] = 0;

  delete [] data;
  data = n;

  return *this;
}

CString &CString::operator=(const char *c)
{
  delete [] data;
  length = strlen(c);
  data = new char[length+1];
  memcpy(data, c, length + 1);

  return *this;
}

CString &CString::operator=(const CString &str)
{
  if (this == &str)
    return *this;

  delete [] data;
  length = str.length;
  data = new char[length + 1];
  memcpy(data, str.data, length + 1);

  return *this;
}

bool KJS::operator==(const KJS::CString& c1, const KJS::CString& c2)
{
  int len = c1.size();
  return len == c2.size() && (len == 0 || memcmp(c1.c_str(), c2.c_str(), len) == 0);
}

UChar UChar::null((char)0);
UString::Rep UString::Rep::null = { 0, 0, 0, 1, 1 };
UString::Rep UString::Rep::empty = { 0, 0, 0, 1, 1 };
UString UString::null;
static const int normalStatBufferSize = 4096;
static char *statBuffer = 0;
static int statBufferSize = 0;

UChar UChar::toLower() const
{
  // ### properly support unicode tolower
  if (uc >= 256 || islower(uc))
    return *this;

  return (unsigned char)tolower(uc);
}

UChar UChar::toUpper() const
{
  if (uc >= 256 || isupper(uc))
    return *this;

  return (unsigned char)toupper(uc);
}

UCharReference& UCharReference::operator=(UChar c)
{
  str->detach();
  if (offset < str->rep->len)
    *(str->rep->dat + offset) = c;
  /* TODO: lengthen string ? */
  return *this;
}

UChar& UCharReference::ref() const
{
  if (offset < str->rep->len)
    return *(str->rep->dat + offset);
  else
    return UChar::null;
}

// return an uninitialized UChar array of size s
static inline UChar* allocateChars(int s)
{
  // work around default UChar constructor code
  return reinterpret_cast<UChar*>(new short[s]);
}

UString::Rep *UString::Rep::create(UChar *d, int l)
{
  Rep *r = new Rep;
  r->dat = d;
  r->len = l;
  r->capacity = l;
  r->rc = 1;
  r->_hash = 0;
  return r;
}

void UString::Rep::destroy()
{
  if (capacity == capacityForIdentifier)
    Identifier::remove(this);
  delete [] dat;
  delete this;
}

// Golden ratio - arbitrary start value to avoid mapping all 0's to all 0's
// or anything like that.
const unsigned PHI = 0x9e3779b9U;

// This hash algorithm comes from:
// http://burtleburtle.net/bob/hash/hashfaq.html
// http://burtleburtle.net/bob/hash/doobs.html
unsigned UString::Rep::computeHash(const UChar *s, int length)
{
    int prefixLength = length < 8 ? length : 8;
    int suffixPosition = length < 16 ? 8 : length - 8;

    unsigned h = PHI;
    h += length;
    h += (h << 10);
    h ^= (h << 6);

    for (int i = 0; i < prefixLength; i++) {
        h += s[i].uc;
	h += (h << 10);
	h ^= (h << 6);
    }
    for (int i = suffixPosition; i < length; i++){
        h += s[i].uc;
	h += (h << 10);
	h ^= (h << 6);
    }

    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);

    if (h == 0)
        h = 0x80000000;

    return h;
}

// This hash algorithm comes from:
// http://burtleburtle.net/bob/hash/hashfaq.html
// http://burtleburtle.net/bob/hash/doobs.html
unsigned UString::Rep::computeHash(const char *s)
{
    int length = strlen(s);
    int prefixLength = length < 8 ? length : 8;
    int suffixPosition = length < 16 ? 8 : length - 8;

    unsigned h = PHI;
    h += length;
    h += (h << 10);
    h ^= (h << 6);

    for (int i = 0; i < prefixLength; i++) {
        h += (unsigned char)s[i];
	h += (h << 10);
	h ^= (h << 6);
    }
    for (int i = suffixPosition; i < length; i++) {
        h += (unsigned char)s[i];
	h += (h << 10);
	h ^= (h << 6);
    }

    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);

    if (h == 0)
        h = 0x80000000;

    return h;
}

UString::UString()
{
  null.rep = &Rep::null;
  attach(&Rep::null);
}

UString::UString(char c)
{
    UChar *d = allocateChars(1);
    d[0] = c;
    rep = Rep::create(d, 1);
}

UString::UString(const char *c)
{
  if (!c) {
    attach(&Rep::null);
    return;
  }
  int length = strlen(c);
  if (length == 0) {
    attach(&Rep::empty);
    return;
  }
  UChar *d = new UChar[length];
  for (int i = 0; i < length; i++)
    d[i].uc = c[i];
  rep = Rep::create(d, length);
}

UString::UString(const UChar *c, int length)
{
  if (length == 0) {
    attach(&Rep::empty);
    return;
  }
  UChar *d = allocateChars(length);
  memcpy(d, c, length * sizeof(UChar));
  rep = Rep::create(d, length);
}

UString::UString(UChar *c, int length, bool copy)
{
  if (length == 0) {
    attach(&Rep::empty);
    return;
  }
  UChar *d;
  if (copy) {
    d = allocateChars(length);
    memcpy(d, c, length * sizeof(UChar));
  } else
    d = c;
  rep = Rep::create(d, length);
}

UString::UString(const UString &a, const UString &b)
{
  int aSize = a.size();
  int bSize = b.size();
  int length = aSize + bSize;
  if (length == 0) {
    attach(&Rep::empty);
    return;
  }
  UChar *d = allocateChars(length);
  memcpy(d, a.data(), aSize * sizeof(UChar));
  memcpy(d + aSize, b.data(), bSize * sizeof(UChar));
  rep = Rep::create(d, length);
}

UString UString::from(int i)
{
  return from((long)i);
}

UString UString::from(unsigned int u)
{
  UChar buf[20];
  UChar *end = buf + 20;
  UChar *p = end;

  if (u == 0) {
    *--p = '0';
  } else {
    while (u) {
      *--p = (unsigned short)((u % 10) + '0');
      u /= 10;
    }
  }

  return UString(p, end - p);
}

UString UString::from(long l)
{
  UChar buf[20];
  UChar *end = buf + 20;
  UChar *p = end;

  if (l == 0) {
    *--p = '0';
  } else {
    bool negative = false;
    if (l < 0) {
      negative = true;
      l = -l;
    }
    while (l) {
      *--p = (unsigned short)((l % 10) + '0');
      l /= 10;
    }
    if (negative) {
      *--p = '-';
    }
  }

  return UString(p, end - p);
}

UString UString::from(double d)
{
  char buf[80];
  int decimalPoint;
  int sign;

  char *result = kjs_dtoa(d, 0, 0, &decimalPoint, &sign, NULL);
  int length = strlen(result);

  int i = 0;
  if (sign) {
    buf[i++] = '-';
  }

  if (decimalPoint <= 0 && decimalPoint > -6) {
    buf[i++] = '0';
    buf[i++] = '.';
    for (int j = decimalPoint; j < 0; j++) {
      buf[i++] = '0';
    }
    strcpy(buf + i, result);
  } else if (decimalPoint <= 21 && decimalPoint > 0) {
    if (length <= decimalPoint) {
      strcpy(buf + i, result);
      i += length;
      for (int j = 0; j < decimalPoint - length; j++) {
	buf[i++] = '0';
      }
      buf[i] = '\0';
    } else {
      strncpy(buf + i, result, decimalPoint);
      i += decimalPoint;
      buf[i++] = '.';
      strcpy(buf + i, result + decimalPoint);
    }
  } else if (result[0] < '0' || result[0] > '9') {
    strcpy(buf + i, result);
  } else {
    buf[i++] = result[0];
    if (length > 1) {
      buf[i++] = '.';
      strcpy(buf + i, result + 1);
      i += length - 1;
    }

    buf[i++] = 'e';
    buf[i++] = (decimalPoint >= 0) ? '+' : '-';
    // decimalPoint can't be more than 3 digits decimal given the
    // nature of float representation
    int exponential = decimalPoint - 1;
    if (exponential < 0) {
      exponential = exponential * -1;
    }
    if (exponential >= 100) {
      buf[i++] = '0' + exponential / 100;
    }
    if (exponential >= 10) {
      buf[i++] = '0' + (exponential % 100) / 10;
    }
    buf[i++] = '0' + exponential % 10;
    buf[i++] = '\0';
  }

  kjs_freedtoa(result);

  return UString(buf);
}

UString &UString::append(const UString &t)
{
  int l = size();
  int tLen = t.size();
  int newLen = l + tLen;
  if (rep->rc == 1 && newLen <= rep->capacity) {
    memcpy(rep->dat+l, t.data(), tLen * sizeof(UChar));
    rep->len = newLen;
    rep->_hash = 0;
    return *this;
  }

  int newCapacity = (newLen * 3 + 1) / 2;
  UChar *n = allocateChars(newCapacity);
  memcpy(n, data(), l * sizeof(UChar));
  memcpy(n+l, t.data(), tLen * sizeof(UChar));
  release();
  rep = Rep::create(n, newLen);
  rep->capacity = newCapacity;

  return *this;
}

CString UString::cstring() const
{
  return ascii();
}

char *UString::ascii() const
{
  // Never make the buffer smaller than normalStatBufferSize.
  // Thus we almost never need to reallocate.
  int length = size();
  int neededSize = length + 1;
  if (neededSize < normalStatBufferSize) {
    neededSize = normalStatBufferSize;
  }
  if (neededSize != statBufferSize) {
    delete [] statBuffer;
    statBuffer = new char [neededSize];
    statBufferSize = neededSize;
  }

  const UChar *p = data();
  char *q = statBuffer;
  const UChar *limit = p + length;
  while (p != limit) {
    *q = p->uc;
    ++p;
    ++q;
  }
  *q = '\0';

  return statBuffer;
}

#ifdef KJS_DEBUG_MEM
void UString::globalClear()
{
  delete [] statBuffer;
  statBuffer = 0;
  statBufferSize = 0;
}
#endif

UString &UString::operator=(const char *c)
{
  int l = c ? strlen(c) : 0;
  UChar *d;
  if (rep->rc == 1 && l <= rep->capacity) {
    d = rep->dat;
    rep->_hash = 0;
  } else {
    release();
    d = allocateChars(l);
    rep = Rep::create(d, l);
  }
  for (int i = 0; i < l; i++)
    d[i].uc = c[i];

  return *this;
}

UString &UString::operator=(const UString &str)
{
  str.rep->ref();
  release();
  rep = str.rep;

  return *this;
}

bool UString::is8Bit() const
{
  const UChar *u = data();
  const UChar *limit = u + size();
  while (u < limit) {
    if (u->uc > 0xFF)
      return false;
    ++u;
  }

  return true;
}

UChar UString::operator[](int pos) const
{
  if (pos >= size())
    return UChar::null;

  return ((UChar *)data())[pos];
}

UCharReference UString::operator[](int pos)
{
  /* TODO: boundary check */
  return UCharReference(this, pos);
}

static int skipInfString(const char *start)
{
  const char *c = start;
  if (*c == '+' || *c == '-')
    c++;
  if (!strncmp(c,"Infinity",8))
    return c+8-start;

  while (*c >= '0' && *c <= '9')
    c++;
  if (*c == '.')
    c++;
  while (*c >= '0' && *c <= '9')
    c++;

  if (*c != 'e')
    return c-start;

  c++;
  if (*c == '+' || *c == '-')
    c++;
  while (*c >= '0' && *c <= '9')
    c++;
  return c-start;
}

double UString::toDouble(bool tolerateTrailingJunk, bool tolerateEmptyString) const
{
  double d;
  double sign = 1;

  // FIXME: If tolerateTrailingJunk is true, then we want to tolerate non-8-bit junk
  // after the number, so is8Bit is too strict a check.
  if (!is8Bit())
    return NaN;

  const char *c = ascii();

  // skip leading white space
  while (isspace(*c))
    c++;

  // empty string ?
  if (*c == '\0')
    return tolerateEmptyString ? 0.0 : NaN;

  if (*c == '-') {
    sign = -1;
    c++;
  }
  else if (*c == '+') {
    sign = 1;
    c++;
  }

  // hex number ?
  if (*c == '0' && (*(c+1) == 'x' || *(c+1) == 'X')) {
    c++;
    d = 0.0;
    while (*(++c)) {
      if (*c >= '0' && *c <= '9')
	d = d * 16.0 + *c - '0';
      else if ((*c >= 'A' && *c <= 'F') || (*c >= 'a' && *c <= 'f'))
	d = d * 16.0 + (*c & 0xdf) - 'A' + 10.0;
      else
	break;
    }
  } else {
    // regular number ?
    char *end;
    d = kjs_strtod(c, &end);
    if ((d != 0.0 || end != c) && d != HUGE_VAL && d != -HUGE_VAL) {
      c = end;
    } else {
      // infinity ?

      int count = skipInfString(c);
      if (count == 0)
	return NaN;
      d = Inf;
      c += count;
    }
  }

  // allow trailing white space
  while (isspace(*c))
    c++;
  // don't allow anything after - unless tolerant=true
  if (!tolerateTrailingJunk && *c != '\0')
    return NaN;

  return d*sign;
}

double UString::toDouble(bool tolerateTrailingJunk) const
{
  return toDouble(tolerateTrailingJunk, true);
}

double UString::toDouble() const
{
  return toDouble(false, true);
}

unsigned long UString::toULong(bool *ok, bool tolerateEmptyString) const
{
  double d = toDouble(false, tolerateEmptyString);
  bool b = true;

  if (isNaN(d) || d != static_cast<unsigned long>(d)) {
    b = false;
    d = 0;
  }

  if (ok)
    *ok = b;

  return static_cast<unsigned long>(d);
}

unsigned long UString::toULong(bool *ok) const
{
  return toULong(ok, true);
}

UString UString::toLower() const
{
  UString u = *this;
  for (int i = 0; i < size(); i++)
    u[i] = u[i].toLower();
  return u;
}

UString UString::toUpper() const
{
  UString u = *this;
  for (int i = 0; i < size(); i++)
    u[i] = u[i].toUpper();
  return u;
}

unsigned int UString::toUInt32(bool *ok) const
{
  double d = toDouble();
  bool b = true;

  if (isNaN(d) || d != static_cast<unsigned>(d)) {
    b = false;
    d = 0;
  }

  if (ok)
    *ok = b;

  return static_cast<unsigned>(d);
}

unsigned int UString::toStrictUInt32(bool *ok) const
{
  if (ok)
    *ok = false;

  // Empty string is not OK.
  int len = rep->len;
  if (len == 0)
    return 0;
  const UChar *p = rep->dat;
  unsigned short c = p->unicode();

  // If the first digit is 0, only 0 itself is OK.
  if (c == '0') {
    if (len == 1 && ok)
      *ok = true;
    return 0;
  }

  // Convert to UInt32, checking for overflow.
  unsigned int i = 0;
  while (1) {
    // Process character, turning it into a digit.
    if (c < '0' || c > '9')
      return 0;
    const unsigned d = c - '0';

    // Multiply by 10, checking for overflow out of 32 bits.
    if (i > 0xFFFFFFFFU / 10)
      return 0;
    i *= 10;

    // Add in the digit, checking for overflow out of 32 bits.
    const unsigned max = 0xFFFFFFFFU - d;
    if (i > max)
        return 0;
    i += d;

    // Handle end of string.
    if (--len == 0) {
      if (ok)
        *ok = true;
      return i;
    }

    // Get next character.
    c = (++p)->unicode();
  }
}

// Rule from ECMA 15.2 about what an array index is.
// Must exactly match string form of an unsigned integer, and be less than 2^32 - 1.
unsigned UString::toArrayIndex(bool *ok) const
{
  unsigned i = toStrictUInt32(ok);
  if (i >= 0xFFFFFFFFU && ok)
    *ok = false;
  return i;
}

int UString::find(const UString &f, int pos) const
{
  int sz = size();
  int fsz = f.size();
  if (sz < fsz)
    return -1;
  if (pos < 0)
    pos = 0;
  if (fsz == 0)
    return pos;
  const UChar *end = data() + sz - fsz;
  long fsizeminusone = (fsz - 1) * sizeof(UChar);
  const UChar *fdata = f.data();
  unsigned short fchar = fdata->uc;
  ++fdata;
  for (const UChar *c = data() + pos; c <= end; c++)
    if (c->uc == fchar && !memcmp(c + 1, fdata, fsizeminusone))
      return (c-data());

  return -1;
}

int UString::find(UChar ch, int pos) const
{
  if (pos < 0)
    pos = 0;
  const UChar *end = data() + size();
  for (const UChar *c = data() + pos; c < end; c++)
    if (*c == ch)
      return (c-data());

  return -1;
}

int UString::rfind(const UString &f, int pos) const
{
  int sz = size();
  int fsz = f.size();
  if (sz < fsz)
    return -1;
  if (pos < 0)
    pos = 0;
  if (pos > sz - fsz)
    pos = sz - fsz;
  if (fsz == 0)
    return pos;
  long fsizeminusone = (fsz - 1) * sizeof(UChar);
  const UChar *fdata = f.data();
  for (const UChar *c = data() + pos; c >= data(); c--) {
    if (*c == *fdata && !memcmp(c + 1, fdata + 1, fsizeminusone))
      return (c-data());
  }

  return -1;
}

int UString::rfind(UChar ch, int pos) const
{
  if (isEmpty())
    return -1;
  if (pos + 1 >= size())
    pos = size() - 1;
  for (const UChar *c = data() + pos; c >= data(); c--) {
    if (*c == ch)
      return (c-data());
  }

  return -1;
}

UString UString::substr(int pos, int len) const
{
  if (pos < 0)
    pos = 0;
  else if (pos >= (int) size())
    pos = size();
  if (len < 0)
    len = size();
  if (pos + len >= (int) size())
    len = size() - pos;

  UChar *tmp = allocateChars(len);
  memcpy(tmp, data()+pos, len * sizeof(UChar));
  UString result(tmp, len);
  delete [] tmp;

  return result;
}

void UString::attach(Rep *r)
{
  rep = r;
  rep->ref();
}

void UString::detach()
{
  if (rep->rc > 1) {
    int l = size();
    UChar *n = allocateChars(l);
    memcpy(n, data(), l * sizeof(UChar));
    release();
    rep = Rep::create(n, l);
  }
}

void UString::release()
{
  rep->deref();
}

bool KJS::operator==(const UString& s1, const UString& s2)
{
  if (s1.rep->len != s2.rep->len)
    return false;

  return (memcmp(s1.rep->dat, s2.rep->dat,
		 s1.rep->len * sizeof(UChar)) == 0);
}

bool KJS::operator==(const UString& s1, const char *s2)
{
  if (s2 == 0) {
    return s1.isEmpty();
  }

  const UChar *u = s1.data();
  const UChar *uend = u + s1.size();
  while (u != uend && *s2) {
    if (u->uc != (unsigned char)*s2)
      return false;
    s2++;
    u++;
  }

  return u == uend && *s2 == 0;
}

bool KJS::operator<(const UString& s1, const UString& s2)
{
  const int l1 = s1.size();
  const int l2 = s2.size();
  const int lmin = l1 < l2 ? l1 : l2;
  const UChar *c1 = s1.data();
  const UChar *c2 = s2.data();
  int l = 0;
  while (l < lmin && *c1 == *c2) {
    c1++;
    c2++;
    l++;
  }
  if (l < lmin)
    return (c1->uc < c2->uc);

  return (l1 < l2);
}

int KJS::compare(const UString& s1, const UString& s2)
{
  const int l1 = s1.size();
  const int l2 = s2.size();
  const int lmin = l1 < l2 ? l1 : l2;
  const UChar *c1 = s1.data();
  const UChar *c2 = s2.data();
  int l = 0;
  while (l < lmin && *c1 == *c2) {
    c1++;
    c2++;
    l++;
  }
  if (l < lmin)
    return (c1->uc > c2->uc) ? 1 : -1;

  if (l1 == l2) {
    return 0;
  }
  return (l1 < l2) ? 1 : -1;
}
