/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
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

#include "global.h"
#include "kjsstring.h"

using namespace KJS;

CString::CString()
{
  data = 0L;
}

CString::CString(char c)
{
  data = new char[2];
  data[0] = c;
  data[1] = '\0';
}

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

CString::CString(const CString *b)
{
  data = new char[b->size()+1];
  strcpy(data, b->c_str());
}

CString::CString(int i)
{
  char buf[20];
  sprintf(buf, "%d", i);
  data = new char[strlen(buf)+1];
  strcpy(data, buf);
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

void CString::resize(unsigned int l)
{
  char *tmp = new char[l];
  strncpy(tmp, data, l-1);
  tmp[l-1] = '\0';
  delete [] data;
  data = tmp;
}

const char * CString::c_str() const
{
  return data;
}

CString KJS::int2String(int i)
{
  char buf[20];
  sprintf(buf, "%d", i);
  return CString(buf);
}

bool KJS::operator==(const KJS::CString& c1, const KJS::CString& c2)
{
  return (strcmp(c1.c_str(), c2.c_str()) == 0);
}

UString::UString() : s(0L), l(0)
{
}

UString::UString(char c)
{
  s = new UnicodeChar[1];
  s[0].lo = c;
  s[0].hi = 0;
  l = 1;
}

UString::UString(const char *c)
{
  l = strlen(c);
  s = new UnicodeChar[l];
  for (unsigned int i = 0; i < l; i++)
    s[i].lo = c[i];
}

UString::UString(const UnicodeChar *c, int length)
{
  l = length;
  s = new UnicodeChar[l];
  memcpy(s, c, l * sizeof(UnicodeChar));
}

UString::UString(const UString &b)
{
  l = b.size();
  s = new UnicodeChar[l];
  memcpy(s, b.data(), l * sizeof(UnicodeChar));
}

UString::UString(const UString *b)
{
  l = b->size();
  s = new UnicodeChar[l];
  memcpy(s, b->data(), l * sizeof(UnicodeChar));
}

UString::~UString()
{
  delete [] s;
}

void UString::append(const UString &t)
{
  UnicodeChar *n = new UnicodeChar[size()+t.size()];
  memcpy(n, s, size() * sizeof(UnicodeChar));
  memcpy(n+l, t.data(), t.size() * sizeof(UnicodeChar));
  l += t.size();

  delete [] s;
  s = n;
}

CString UString::cstring() const
{
  char *c = new char[l+1];
  for(unsigned int i = 0; i < size(); i++)
    c[i] = s[i].lo;
  c[size()] = '\0';
  CString cstr(c);
  delete [] c;

  return cstr;
}

UString &UString::operator=(const char *c)
{
  if (s)
    delete [] s;

  l = strlen(c);
  s = new UnicodeChar[l];
  for (unsigned int i = 0; i < l; i++)
    s[i].lo = c[i];

  return *this;
}

UString &UString::operator=(const UString &str)
{
  if (s)
    delete [] s;

  l = str.size();
  s = new UnicodeChar[l];
  memcpy(s, str.data(), l * sizeof(UnicodeChar));

  return *this;
}

bool UString::is8Bit() const
{
  const UnicodeChar *u = data();
  for(unsigned int i = 0; i < size(); i++, u++)
    if (u->hi)
      return false;

  return true;
}

UnicodeChar UString::operator[](unsigned int pos) const
{
  if (pos >= size())
    return UnicodeChar();

  return data()[pos];
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
  long fsize = f.size() * sizeof(UnicodeChar);
  if (pos < 0)
    pos = 0;
  const UnicodeChar *end = data() + size() - f.size();
  for (const UnicodeChar *c = data() + pos; c <= end; c++)
    if (!memcmp((void*)c, (void*)f.data(), fsize))
      return (c-data());

  return -1;
}

int UString::rfind(const UString &f, int pos) const
{
  if (pos + f.size() >= size())
    pos = size() - f.size();
  long fsize = f.size() * sizeof(UnicodeChar);
  for (const UnicodeChar *c = data() + pos; c >= data(); c--) {
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
  if (pos + len >= (int) l)
    len = size() - pos;

  UnicodeChar *tmp = new UnicodeChar[len];
  memcpy(tmp, data()+pos, len * sizeof(UnicodeChar));
  UString result(tmp, len);
  delete [] tmp;

  return result;
}

bool KJS::operator==(const UString& s1, const UString& s2)
{
  if (s1.size() != s2.size())
    return false;

  return (memcmp(s1.data(), s2.data(),
		 s1.size() * sizeof(UnicodeChar)) == 0);
}

UString KJS::operator+(const UString& s1, const UString& s2)
{
  UString tmp(s1);
  tmp.append(s2);

  return tmp;
}
