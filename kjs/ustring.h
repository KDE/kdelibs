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

#ifndef _KJS_STRING_H_
#define _KJS_STRING_H_

/**
 * @internal
 */
namespace DOM {
  class DOMString;
};
class KJScript;
class QString;
class QConstString;

namespace KJS {

  class UCharReference;
  class UString;

  /**
   * @short Unicode character.
   *
   * UChar represents a 16 bit Unicode character. It's internal data
   * representation is compatible to XChar2b and QChar. It's therefore
   * possible to exchange data with X and Qt with shallow copies.
   */
  struct UChar {
    /**
     * Construct a character with value 0.
     */ 
    UChar() : hi(0), lo(0) { }
    /**
     * Construct a character with the value denoted by the arguments.
     * @param h higher byte
     * @param l lower byte
     */ 
    UChar(unsigned char h , unsigned char l) : hi(h), lo(l) { }
    /**
     * Construct a character with the given value.
     * @param u 16 bit Unicode value
     */ 
    UChar(unsigned short u) : hi(u & 0xff00), lo(u & 0x00ff) { }
    UChar(const UCharReference &c);
    /**
     * @return The higher byte of the character.
     */
    unsigned char high() const { return hi; }
    /**
     * @return The lower byte of the character.
     */
    unsigned char low() const { return lo; }
    /**
     * @return the 16 bit Unicode value of the character
     */
    unsigned short unicode() const { return hi << 8 | lo; }
  public:
    /**
     * @return The character converted to lower case.
     */
    UChar toLower() const;
    /**
     * @return The character converted to upper case.
     */
    UChar toUpper() const;
    /**
     * A static instance of UChar(0).
     */
    static UChar null;
  private:
    friend UCharReference;
    friend UString;
    friend bool operator==(const UChar &c1, const UChar &c2);
    friend bool operator==(const UString& s1, const char *s2);
    unsigned char hi;
    unsigned char lo;
  };

  /**
   * @short Dynamic reference to a string character.
   */
  class UCharReference {
    friend UString;
    UCharReference(UString *s, unsigned int off) : str(s), offset(off) { }
  public:
    UCharReference& operator=(char c) { return operator=(UChar(c)); }
    UCharReference& operator=(UChar c);
    unsigned short unicode() const { return ref().unicode(); }
    unsigned char& low() const { return ref().lo; }
    unsigned char& high() const { return ref().hi; }
    UChar toLower() const { return ref().toLower(); }
    UChar toUpper() const  { return ref().toUpper(); }
  private:
    UChar& ref() const;
    UString *str;
    int offset;
  };

  /**
   * @short 8 bit char based string class
   */
  class CString {
  public:
    CString() : data(0L) { }
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

  /**
   * @short Unicode string class
   */
  class UString {
    friend bool operator==(const UString&, const UString&);
    friend UCharReference;
    /**
     * @internal
     */
    struct Rep {
      friend UString;
      friend bool operator==(const UString&, const UString&);
      static Rep *create(UChar *d, int l);
      inline UChar *data() const { return dat; }
      inline int size() const { return len; }

      inline void ref() { rc++; }
      inline int deref() { return --rc; }

      UChar *dat;
      int len;
      int rc;
      static Rep null;
    };

  public:
    UString();
    UString(char c);
    UString(int i);
    UString(const char *c);
    UString(const UChar *c, int length);
    UString(UChar *c, int length, bool copy);
    UString(const UString &);
    UString(const UString *);
    UString(const QString &);
    UString(const DOM::DOMString &);

    ~UString();

    static UString from(int i);
    static UString from(unsigned int u);
    static UString from(double d);

    UString &append(const UString &);

    // conversions to other string types
    CString cstring() const;
    char *ascii() const;
    DOM::DOMString string() const;
    QString qstring() const;
    QConstString qconststring() const;

    UString &operator=(const char *c);
    UString &operator=(const UString &);
    UString &operator+=(const UString &s);

    const UChar* data() const { return rep->data(); }
    bool isNull() const { return (rep == &Rep::null); }
    bool isEmpty() const { return (!rep->len); }
    bool is8Bit() const;
    int size() const { return rep->size(); }
    UChar operator[](int pos) const;
    UCharReference operator[](int pos);

    double toDouble() const;
    int find(const UString &f, int pos = 0) const;
    int rfind(const UString &f, int pos) const;
    UString substr(int pos = 0, int len = -1) const;
    static UString null;
  private:
    void attach(Rep *r);
    void detach();
    void release();
    Rep *rep;
  };

  bool operator==(const UChar &c1, const UChar &c2);
  bool operator==(const UString& s1, const UString& s2);
  bool operator==(const UString& s1, const char *s2);
  bool operator==(const char *s1, const UString& s2);
  bool operator==(const CString& s1, const CString& s2);
  UString operator+(const UString& s1, const UString& s2);

}; // namespace

#endif
