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

#ifndef _KJSLEXER_H_
#define _KJSLEXER_H_

#include "global.h"
#include "kjsstring.h"

namespace KJS {

  class KJSLexer {
  public:
    KJSLexer(const UString &c);
    ~KJSLexer();

    int lex();

    int lineNo() const { return yylineno + 1; }

    enum State { Start,
		 Identifier,
		 InIdentifier,
		 InSingleLineComment,
		 InMultiLineComment,
		 InNum,
		 InHexOrOctal,
		 InHex,
		 InOctal,
		 InDecimal,
		 InExponentIndicator,
		 InExponent,
		 Int,
		 Hex,
		 Octal,
		 Decimal,
		 String,
		 Eof,
		 Eol,
		 InString,
		 InEscapeSequence,
		 InHexEscape,
		 InUnicodeEscape,
		 Other,
		 Bad };

    int yylineno;

  private:
    bool done;
    char *buffer8;
    UnicodeChar *buffer16;
    unsigned int size8, size16;
    unsigned int pos8, pos16;

  private:
    State state;
    void setDone(State s);
    unsigned int pos;
    void shift(unsigned int p);
    int lookupKeyword(const char *);

    bool isWhiteSpace() const;
    bool isLineTerminator() const;
    bool isIdentLetter() const;
    bool isDecimalDigit(unsigned short c) const;
    bool isHexDigit(unsigned short c) const;
    bool isOctalDigit(unsigned short c) const;

    int matchPunctuator(unsigned short c1, unsigned short c2,
			unsigned short c3, unsigned short c4);
    unsigned char singleEscape(unsigned short c) const;
    unsigned short convertOctal(unsigned short c1, unsigned short c2,
                                unsigned short c3) const;
    unsigned char convertHex(unsigned short c1) const;
    unsigned char convertHex(unsigned short c1, unsigned short c2) const;
    UnicodeChar convertUnicode(unsigned short c1, unsigned short c2,
                               unsigned short c3, unsigned short c4) const;

    void record8(unsigned short c);
    void record16(unsigned char c);
    void record16(UnicodeChar c);

    UString code;
    int yycolumn;

    // current and following unicode characters
    unsigned short current, next1, next2, next3;

    struct keyword {
      const char *name;
      int token;
    };

    static struct keyword keytable[];
  };

}; // namespace

#endif
