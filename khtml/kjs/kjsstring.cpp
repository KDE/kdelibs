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
#include <iostream.h>
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

int CString::length() const
{
  return strlen(data);
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

const char* UString::ascii() const
{
  char *memoryLeak = new char[l+1];
  for(unsigned int i = 0; i < l; i++)
    memoryLeak[i] = s[i].lo;
  memoryLeak[l] = '\0';

  return memoryLeak;
}

UnicodeChar UString::operator[](unsigned int pos) const
{
  if (pos >= l)
    return UnicodeChar();

  return s[pos];
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
