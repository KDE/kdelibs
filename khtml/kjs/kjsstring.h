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

#ifndef _KJSSTRING_H_
#define _KJSSTRING_H_

#include "global.h"

namespace DOM {
  class DOMString;
};
class QString;

namespace KJS {

  class CString {
  public:
    CString();
    CString(const char *c);
    CString(const CString &);

    ~CString();

    CString &append(const CString &);
    CString &operator=(const char *c);
    CString &operator=(const CString &);
    CString &operator+=(const CString &);

    int size() const;
    const char *c_str() const { return data; }
  private:
    char *data;
  };

  class UString;

  class UStringData {
    friend UString;
  public:
    UStringData() : dat(0L), len(0), rc(1) { }
    UStringData(UChar *d, unsigned int l) : dat(d), len(l), rc(1) { }
    ~UStringData() { delete dat; }
  private:
    UChar *data() const { return dat; }
    unsigned int size() const { return len; }

    void ref() { rc++; }
    unsigned int deref() { return --rc; }

    UChar *dat;
    unsigned int len;
    unsigned int rc;
    static UStringData null;
  };

  class UString {
  public:
    UString();
    UString(char c);
    UString(const char *c);
    UString(const UChar *c, int length);
    UString(const UString &);
    UString(const UString *);
    UString(const DOM::DOMString &);

    ~UString();

    void append(const UString &);

    // conversions to other string types
    CString cstring() const;
    DOM::DOMString string() const;
    QString qstring() const;

    UString &operator=(const char *c);
    UString &operator=(const UString &);

    const UChar* data() const { return rep->data(); }
    bool is8Bit() const;
    unsigned int size() const { return rep->size(); }
    UChar &operator[](unsigned int pos) const;

    double toDouble() const;
    int find(const UString &f, int pos = 0) const;
    int rfind(const UString &f, int pos) const;
    UString substr(int pos = 0, int len = -1) const;
    static UString null;
  private:
    void attach(UStringData *r);
    void release();
    UStringData *rep;
  };

  bool operator==(const UChar &c1, const UChar &c2);
  bool operator==(const UString& s1, const UString& s2);
  bool operator==(const CString& s1, const CString& s2);
  UString operator+(const UString& s1, const UString& s2);

  UString int2String(int i);

}; // namespace

#endif
