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
#include "qstring.h"

namespace KJS {

  class KJSLexer {
  public:
    KJSLexer(const QString &code);
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
    bool isDecimalDigit(UnicodeChar c) const;
    bool isHexDigit(UnicodeChar c) const;
    bool isOctalDigit(UnicodeChar c) const;

    int matchPunctuator(UnicodeChar c1, UnicodeChar c2,
			UnicodeChar c3, UnicodeChar c4);
    UnicodeChar singleEscape(UnicodeChar c) const;
    UnicodeChar convertOctal(UnicodeChar c1, UnicodeChar c2,
			     UnicodeChar c3) const;
    long convertHex(UnicodeChar c1) const;
    UnicodeChar convertHex(UnicodeChar c1, UnicodeChar c2) const;
    UnicodeChar convertUnicode(UnicodeChar c1, UnicodeChar c2,
			       UnicodeChar c3, UnicodeChar c4) const;

    void record8(UnicodeChar c);
    void record16(UnicodeChar c);

    QString code;
    int yycolumn;

    // current and following unicode characters
    UnicodeChar current, next1, next2, next3;

    struct keyword {
      char  *name;
      int   token;
    };

    static struct keyword keytable[];
  };

}; // namespace

#endif
