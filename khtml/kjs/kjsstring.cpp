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

#include <string>

using namespace KJS;

std::string s;

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
  data = new char[b.length()+1];
  strcpy(data, b.ascii());
}

CString::CString(const CString *b)
{
  data = new char[b->length()+1];
  strcpy(data, b->ascii());
}

CString::CString(unsigned int u)
{
  char buf[20];
  sprintf(buf, "%u", u);
  data = new char[strlen(buf)+1];
  strcpy(data, buf);
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
    n = new char[strlen(data)+t.length()+1];
    strcpy(n, data);
  } else {
    n = new char[t.length()+1];
    n[0] = '\0';
  }
  strcat(n, t.ascii());

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
  data = new char[str.length()+1];
  strcpy(data, str.ascii());

  return *this;
}

CString &CString::operator+=(const CString &str)
{
  return append(str.ascii());
}

int CString::length() const
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

const char * CString::ascii() const
{
  return data;
}

bool KJS::operator==(const KJS::CString& c1, const KJS::CString& c2)
{
  return (strcmp(c1.ascii(), c2.ascii()) == 0);
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
  l = b.length();
  s = new UnicodeChar[l];
  memcpy(s, b.unicode(), l * sizeof(UnicodeChar));
}

UString::UString(const UString *b)
{
  l = b->length();
  s = new UnicodeChar[l];
  memcpy(s, b->unicode(), l * sizeof(UnicodeChar));
}

UString::~UString()
{
  delete [] s;
}

void UString::append(const UString &t)
{
  UnicodeChar *n = new UnicodeChar[l+t.length()];
  memcpy(n, s, l * sizeof(UnicodeChar));
  memcpy(n+l, t.unicode(), t.length() * sizeof(UnicodeChar));
  l += t.length();

  delete [] s;
  s = n;
}

CString UString::cstring() const
{
  char *c = new char[l+1];
  for(unsigned int i = 0; i < l; i++)
    c[i] = s[i].lo;
  c[l] = '\0';
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

  l = str.length();
  s = new UnicodeChar[l];
  memcpy(s, str.unicode(), l * sizeof(UnicodeChar));

  return *this;
}

int UString::length() const
{
  return l;
}

const UnicodeChar* UString::unicode() const
{
  return s;
}

bool UString::is8Bit() const
{
  UnicodeChar *u = s;
  for(unsigned int i = 0; i < l; i++, u++)
    if (u->hi)
      return false;

  return true;
}

UnicodeChar UString::operator[](unsigned int pos) const
{
  if (pos >= l)
    return UnicodeChar();

  return s[pos];
}

double UString::toDouble() const
{
  double d;

  if (!is8Bit())
    return NaN;

  CString str = cstring();
  const char *c = str.ascii();

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
  long fsize = f.l * sizeof(UnicodeChar);
  if (pos < 0)
    pos = 0;
  UnicodeChar *end = s + l - f.l;
  for (UnicodeChar *c = s + pos; c <= end; c++)
    if (!memcmp((void*)c, (void*)f.s, fsize))
      return (c-s);

  return -1;
}

int UString::rfind(const UString &f, int pos) const
{
  if (pos + f.l >= l)
    pos = l - f.l;
  long fsize = f.l * sizeof(UnicodeChar);
  for (UnicodeChar *c = s + pos; c >= s; c--) {
    if (!memcmp((void*)c, (void*)f.s, fsize))
      return (c-s);
  }

  return -1;
}

UString UString::substr(int pos, int len) const
{
  if (pos < 0)
    pos = 0;
  else if (pos >= (int) l)
    pos = l;
  if (len < 0)
    len = l;
  if (pos + len >= (int) l)
    len = l - pos;

  UnicodeChar *tmp = new UnicodeChar[len];
  memcpy(tmp, s+pos, len * sizeof(UnicodeChar));
  UString result(tmp, len);
  delete [] tmp;

  return result;
}

bool KJS::operator==(const UString& s1, const UString& s2)
{
  if (s1.length() != s2.length())
    return false;

  return (memcmp(s1.unicode(), s2.unicode(),
		 s1.length() * sizeof(UnicodeChar)) == 0);
}

UString KJS::operator+(const UString& s1, const UString& s2)
{
  UString tmp(s1);
  tmp.append(s2);

  return tmp;
}
