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
 */

#include <config.h>

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

UString KJS::int2String(int i)
{
  char buf[20];
  sprintf(buf, "%d", i);

  return UString(buf);
}

bool KJS::operator==(const KJS::CString& c1, const KJS::CString& c2)
{
  return (strcmp(c1.c_str(), c2.c_str()) == 0);
}

UChar UChar::null = UChar(0, 0);
UStringData UStringData::null = UStringData();
UString UString::null = UString();

UString::UString()
{
  attach(&UStringData::null);
}

UString::UString(char c)
{
  rep = new UStringData(new UChar(0, c), 1);
}

UString::UString(int i)
{
  char buffer[30];
  sprintf(buffer, "%d", i);
  attach(&UStringData::null);
  operator=(buffer);
}

UString::UString(const char *c)
{
  attach(&UStringData::null);
  operator=(c);
}

UString::UString(const UChar *c, int length)
{
  UChar *d = new UChar[length];
  memcpy(d, c, length * sizeof(UChar));
  rep = new UStringData(d, length);
}

UString::UString(const UString &b)
{
  attach(b.rep);
}

UString::UString(const UString *b)
{
  attach(b->rep);
}

UString::~UString()
{
  release();
}

void UString::append(const UString &t)
{
  unsigned l = size();
  UChar *n = new UChar[l+t.size()];
  memcpy(n, data(), l * sizeof(UChar));
  memcpy(n+l, t.data(), t.size() * sizeof(UChar));
  release();
  rep = new UStringData(n, l + t.size());
}

CString UString::cstring() const
{
  char *c = new char[size()+1];
  for(unsigned int i = 0; i < size(); i++)
    c[i] = data()[i].lo;
  c[size()] = '\0';
  CString cstr(c);
  delete [] c;

  return cstr;
}

UString &UString::operator=(const char *c)
{
  release();
  unsigned int l = strlen(c);
  UChar *d = new UChar[l];
  for (unsigned int i = 0; i < l; i++)
    d[i].lo = c[i];
  rep = new UStringData(d, l);

  return *this;
}

UString &UString::operator=(const UString &str)
{
  release();
  attach(str.rep);

  return *this;
}

bool UString::is8Bit() const
{
  const UChar *u = data();
  for(unsigned int i = 0; i < size(); i++, u++)
    if (u->hi)
      return false;

  return true;
}

UChar &UString::operator[](unsigned int pos) const
{
  if (pos >= size())
    return UChar::null;

  return ((UChar *)data())[pos];
}

double UString::toDouble() const
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
    return 0.0;

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
    if (d != 0.0 || end != c) {
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
  if (*c != '\0')
    d = NaN;

  return d;
}

int UString::find(const UString &f, int pos) const
{
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

void UString::attach(UStringData *r)
{
  rep = r;
  rep->ref();
}

void UString::release()
{
  if (rep)
    if (!rep->deref())
      delete rep;
  rep = 0L;
}

bool KJS::operator==(const UChar &c1, const UChar &c2)
{
  return ((c1.lo == c2.lo) & (c1.hi == c2.hi));
}

bool KJS::operator==(const UString& s1, const UString& s2)
{
  if (s1.rep->len != s2.rep->len)
    return false;

  return (memcmp(s1.rep->dat, s2.rep->dat,
		 s1.rep->len * sizeof(UChar)) == 0);
}

UString KJS::operator+(const UString& s1, const UString& s2)
{
  UString tmp(s1);
  tmp.append(s2);

  return tmp;
}
