// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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
#include <math.h>

namespace KJS {
  extern const double NaN;
  extern const double Inf;
};

using namespace KJS;

CString::CString(const char *c)
{
  data = new char[strlen(c)+1];
  strcpy(data, c);
}

CString::CString(const CString &b)
{
  data = new char[b.size()+1];
  strcpy(data, b.c_str());
}

CString::~CString()
{
  delete [] data;
}

CString &CString::append(const CString &t)
{
  char *n;
  if (data) {
    n = new char[strlen(data)+t.size()+1];
    strcpy(n, data);
  } else {
    n = new char[t.size()+1];
    n[0] = '\0';
  }
  strcat(n, t.c_str());

  delete [] data;
  data = n;

  return *this;
}

CString &CString::operator=(const char *c)
{
  if (data)
    delete [] data;
  data = new char[strlen(c)+1];
  strcpy(data, c);

  return *this;
}

CString &CString::operator=(const CString &str)
{
  if (this == &str)
    return *this;

  if (data)
    delete [] data;
  data = new char[str.size()+1];
  strcpy(data, str.c_str());

  return *this;
}

CString &CString::operator+=(const CString &str)
{
  return append(str.c_str());
}

int CString::size() const
{
  return strlen(data);
}

bool KJS::operator==(const KJS::CString& c1, const KJS::CString& c2)
{
  return (strcmp(c1.c_str(), c2.c_str()) == 0);
}

UChar UChar::null;
UString::Rep UString::Rep::null = { 0, 0, 1 };
UString UString::null;
static char *statBuffer = 0L;

UChar::UChar(const UCharReference &c)
    : uc( c.unicode() )
{
}

UChar UChar::toLower() const
{
  // ### properly supprot unicode tolower
  if (uc >= 256 || islower(uc))
    return *this;

  return UChar(tolower(uc));
}

UChar UChar::toUpper() const
{
  if (uc >= 256 || isupper(uc))
    return *this;

  return UChar(toupper(uc));
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

UString::Rep *UString::Rep::create(UChar *d, int l)
{
  Rep *r = new Rep;
  r->dat = d;
  r->len = l;
  r->rc = 1;

  return r;
}

UString::UString()
{
  null.rep = &Rep::null;
  attach(&Rep::null);
}

UString::UString(char c)
{
    UChar *d = new UChar[1];
    d[0] = UChar(0, c);
    rep = Rep::create(d, 1);
}

UString::UString(const char *c)
{
  attach(&Rep::null);
  operator=(c);
}

UString::UString(const UChar *c, int length)
{
  UChar *d = new UChar[length];
  memcpy(d, c, length * sizeof(UChar));
  rep = Rep::create(d, length);
}

UString::UString(UChar *c, int length, bool copy)
{
  UChar *d;
  if (copy) {
    d = new UChar[length];
    memcpy(d, c, length * sizeof(UChar));
  } else
    d = c;
  rep = Rep::create(d, length);
}

UString::UString(const UString &b)
{
  attach(b.rep);
}

UString::~UString()
{
  release();
}

UString UString::from(int i)
{
  char buf[40];
  sprintf(buf, "%d", i);

  return UString(buf);
}

UString UString::from(unsigned int u)
{
  char buf[40];
  sprintf(buf, "%u", u);

  return UString(buf);
}

UString UString::from(double d)
{
  char buf[40];

  if (d == -0)
    strcpy(buf,"0");
  else if (KJS::isNaN(d))
    strcpy(buf,"NaN");
  else if (KJS::isPosInf(d))
    strcpy(buf,"Infinity");
  else if (KJS::isNegInf(d))
    strcpy(buf,"-Infinity");
  else
    sprintf(buf, "%.16g", d);	// does the right thing

  // ECMA 3rd ed. 9.8.1 9 e: "with no leading zeros"
  int buflen = strlen(buf);
  if (buflen >= 4 && buf[buflen-4] == 'e' && buf[buflen-2] == '0') {
    buf[buflen-2] = buf[buflen-1];
    buf[buflen-1] = 0;
  }

  return UString(buf);
}

UString &UString::append(const UString &t)
{
  int l = size();
  UChar *n = new UChar[l+t.size()];
  memcpy(n, data(), l * sizeof(UChar));
  memcpy(n+l, t.data(), t.size() * sizeof(UChar));
  release();
  rep = Rep::create(n, l + t.size());

  return *this;
}

CString UString::cstring() const
{
  return CString(ascii());
}

char *UString::ascii() const
{
  if (statBuffer)
    delete [] statBuffer;

  statBuffer = new char[size()+1];
  for(int i = 0; i < size(); i++)
    statBuffer[i] = data()[i].low();
  statBuffer[size()] = '\0';

  return statBuffer;
}

#ifdef KJS_DEBUG_MEM
void UString::globalClear()
{
  delete [] statBuffer;
  statBuffer = 0L;
}
#endif

UString &UString::operator=(const char *c)
{
  release();
  int l = c ? strlen(c) : 0;
  UChar *d = new UChar[l];
  for (int i = 0; i < l; i++)
    d[i].uc = c[i];
  rep = Rep::create(d, l);

  return *this;
}

UString &UString::operator=(const UString &str)
{
  str.rep->ref();
  release();
  rep = str.rep;

  return *this;
}

UString &UString::operator+=(const UString &s)
{
  return append(s);
}

bool UString::is8Bit() const
{
  const UChar *u = data();
  for(int i = 0; i < size(); i++, u++)
    if (u->uc > 0xFF)
      return false;

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

double UString::toDouble( bool tolerant ) const
{
  double d;

  if (!is8Bit())
    return NaN;

  CString str = cstring();
  const char *c = str.c_str();

  // skip leading white space
  while (isspace(*c))
    c++;

  // empty string ?
  if (*c == '\0')
    return tolerant ? NaN : 0.0;

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
    d = strtod(c, &end);
    if ((d != 0.0 || end != c) && d != HUGE_VAL && d != -HUGE_VAL) {
      c = end;
    } else {
      // infinity ?
      d = 1.0;
      if (*c == '+')
	c++;
      else if (*c == '-') {
	d = -1.0;
	c++;
      }
      if (strncmp(c, "Infinity", 8) != 0)
	return NaN;
      d = d * Inf;
      c += 8;
    }
  }

  // allow trailing white space
  while (isspace(*c))
    c++;
  // don't allow anything after - unless tolerant=true
  if ( !tolerant && *c != '\0')
    d = NaN;

  return d;
}

unsigned long UString::toULong(bool *ok) const
{
  double d = toDouble();
  bool b = true;

  if (isNaN(d) || d != static_cast<unsigned long>(d)) {
    b = false;
    d = 0;
  }

  if (ok)
    *ok = b;

  return static_cast<unsigned long>(d);
}

int UString::find(const UString &f, int pos) const
{
  if (isNull())
    return -1;
  long fsize = f.size() * sizeof(UChar);
  if (pos < 0)
    pos = 0;
  const UChar *end = data() + size() - f.size();
  for (const UChar *c = data() + pos; c <= end; c++)
    if (!memcmp((void*)c, (void*)f.data(), fsize))
      return (c-data());

  return -1;
}

int UString::rfind(const UString &f, int pos) const
{
  if (isNull())
    return -1;
  if (pos + f.size() >= size())
    pos = size() - f.size();
  long fsize = f.size() * sizeof(UChar);
  for (const UChar *c = data() + pos; c >= data(); c--) {
    if (!memcmp((void*)c, (void*)f.data(), fsize))
      return (c-data());
  }

  return -1;
}

UString UString::substr(int pos, int len) const
{
  if (isNull())
    return UString();
  if (pos < 0)
    pos = 0;
  else if (pos >= (int) size())
    pos = size();
  if (len < 0)
    len = size();
  if (pos + len >= (int) size())
    len = size() - pos;

  UChar *tmp = new UChar[len];
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
    UChar *n = new UChar[l];
    memcpy(n, data(), l * sizeof(UChar));
    release();
    rep = Rep::create(n, l);
  }
}

void UString::release()
{
  if (!rep->deref()) {
    delete [] rep->dat;
    delete rep;
  }
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
  if (s2 == 0L && s1.isNull())
    return true;

  if (s1.size() != (int) strlen(s2))
    return false;

  const UChar *u = s1.data();
  while (*s2) {
    if (u->uc != *s2 )
      return false;
    s2++;
    u++;
  }

  return true;
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
    return (c1->unicode() < c2->unicode());

  return (l1 < l2);
}

UString KJS::operator+(const UString& s1, const UString& s2)
{
  UString tmp(s1);
  tmp.append(s2);

  return tmp;
}
