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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "kjs.h"
#include "nodes.h"
#include "lexer.h"
#include "kjsstring.h"

// we can't specify the namespace in yacc's C output, so do it here
using namespace KJS;

#ifndef KDE_USE_FINAL
#include "grammar.h"
#endif

int yylex()
{
  return KJSWorld::lexer->lex();
}

KJSLexer::KJSLexer(const UString &c)
  : yylineno(0),
    size8(128), size16(128),
    pos8(0), pos16(0), pos(0),
    code(c)
{
  // allocate space for read buffers
  buffer8 = new char[size8];
  buffer16 = new UnicodeChar[size16];

  // read first characters
  shift(0);
}

KJSLexer::~KJSLexer()
{
  delete [] buffer8;
  delete [] buffer16;
}

void KJSLexer::shift(unsigned int p)
{
  pos += p;
  current = code[pos].unicode();
  // TODO: cycle these values
  next1 = code[pos+1].unicode();
  next2 = code[pos+2].unicode();
  next3 = code[pos+3].unicode();
}

void KJSLexer::setDone(State s)
{
  state = s;
  done = true;
}

int KJSLexer::lex()
{
  int token = 0;
  state = Start;
  unsigned short stringType = 0; // either single or double quotes
  pos8 = pos16 = 0;
  done = false;

  while (!done) {
    switch (state) {
    case Start:
      if (isWhiteSpace()) {
	// do nothing
      } else if (current == '/' && next1 == '/') {
	shift(1);
	state = InSingleLineComment;
      } else if (current == '/' && next1 == '*') {
	shift(1);
	state = InMultiLineComment;
      } else if (current == 0) {
	setDone(Eof);
      } else if (isLineTerminator()) {
	cout << "Line terminator" << endl;
	yylineno++;
      } else if (current == '"' || current == '\'') {
	state = InString;
	stringType = current;
      } else if (isIdentLetter()) {
	record8(current);
	state = InIdentifier;
      } else if (current == '0') {
	record8(current);
	state = InHexOrOctal;
      } else if (isDecimalDigit(current)) {
	record8(current);
	state = InNum;
      } else if (current == '.' && isDecimalDigit(next1)) {
	record8(current);
	state = InDecimal;
      } else {
	token = matchPunctuator(current, next1, next2, next3);
	if (token != -1) {
	  setDone(Other);
	} else {
	  cerr << "encountered unknown character" << endl;
	  setDone(Bad);
	}
      }
      break;
    case InString:
      if (current == stringType) {
	shift(1);
	setDone(String);
      } else if (current == 0 || isLineTerminator()) {
	setDone(Bad);
      } else if (current == '\\') {
	state = InEscapeSequence;
      } else {
	record16(current);
      }
      break;
    // Escape Sequences inside of strings
    case InEscapeSequence:
      if (isOctalDigit(current)) {
	if (current >= '0' && current <= '3' &&
	    isOctalDigit(next1) && isOctalDigit(next2)) {
	  record16(convertOctal(current, next1, next2));
	  shift(2);
	  state = InString;
	} else if (isOctalDigit(current) && isOctalDigit(next1)) {
	  record16(convertOctal('0', current, next1));
	  shift(1);
	  state = InString;
	} else if (isOctalDigit(current)) {
	  record16(convertOctal('0', '0', current));
	  state = InString;
	} else {
	  setDone(Bad);
	}
      } else if (current == 'x')
	state = InHexEscape;
      else if (current == 'u')
	state = InUnicodeEscape;
      else {
	record16(singleEscape(current));
	state = InString;
      }
      break;
    case InHexEscape:
      if (isHexDigit(current) && isHexDigit(next1)) {
	state = InString;
	record16(convertHex(current, next1));
	shift(1);
      } else {
	setDone(Bad);
      }
      break;
    case InUnicodeEscape:
      if (isHexDigit(current) && isHexDigit(next1) &&
	  isHexDigit(next2) && isHexDigit(next3)) {
	record16(convertUnicode(current, next1, next2, next3));
	shift(3);
	state = InString;
      } else {
	setDone(Bad);
      }
      break;
    case InSingleLineComment:
      if (isLineTerminator()) {
	yylineno++;
	state = Start;
      } else if (current == 0) {
	setDone(Eof);
      }
      break;
    case InMultiLineComment:
      if (current == 0) {
	setDone(Bad);
      } else if (isLineTerminator()) {
	yylineno++;
      } else if (current == '*' && next1 == '/') {
	state = Start;
	shift(1);
      }
      break;
    case InIdentifier:
      if (isIdentLetter() || isDecimalDigit(current)) {
	record8(current);
	break;
      }
      setDone(Identifier);
      break;
    case InHexOrOctal:
      if (current == 'x' || current == 'X') {
	record8(current);
	state = InHex;
      } else if (isOctalDigit(current)) {
	record8(current);
	state = InOctal;
      } else {
	setDone(Int);
      }
      break;
    case InHex:
      if (isHexDigit(current)) {
	record8(current);
      } else {
	setDone(Hex);
      }
      break;
    case InOctal:
      if (isOctalDigit(current)) {
	record8(current);
      } else
	setDone(Octal);
      break;
    case InNum:
      if (isDecimalDigit(current)) {
	record8(current);
      } else if (current == '.') {
	record8(current);
	state = InDecimal;
      } else if (current == 'e' || current == 'E') {
	record8(current);
	state = InExponentIndicator;
      } else
	setDone(Int);
      break;
    case InDecimal:
      if (isDecimalDigit(current)) {
	record8(current);
      } else if (current == 'e' || current == 'E') {
	record8(current);
	state = InExponentIndicator;
      } else
	setDone(Decimal);
      break;
    case InExponentIndicator:
      if (current == '+' || current == '-') {
	record8(current);
	shift(1);
      } else if (isDecimalDigit(current)) {
	record8(current);
	state = InExponent;
      } else
	setDone(Bad);
      break;
    case InExponent:
      if (isDecimalDigit(current)) {
	record8(current);
      } else
	setDone(Decimal);
      break;
    default:
      assert(!"Unhandled state in switch statement");
    }

    // move on to the next character
    if (!done) {
      shift(1);
    }
  }

  // terminate string
  buffer8[pos8] = '\0';

  fprintf(stderr, "line: %d ", lineNo());
  fprintf(stderr, "yytext (%x): ", buffer8[0]);
  if (state != Eol)
    fprintf(stderr, "%s ", buffer8);

  int i;
  // scan integer and hex numbers
  if (state == Int || state == Hex) {
    sscanf(buffer8, "%i", &i);
    state = Int;
  }
  // scan octal number
  if (state == Octal) {
    unsigned int ui;
    sscanf(buffer8, "%o", &ui);
    i = ui;
    state = Int;
  }

  switch (state) {
  case Eof:
    printf("(EOF)\n");
    return 0;
  case Eol:
    printf("(EOL)\n");
    return LF;
  case Other:
    printf("(Other)\n");
    return token;
  case Identifier:
    if (!(token = lookupKeyword(buffer8))) {
      printf("(Identifier)\n");
      yylval.cstr = new CString(buffer8);
      return IDENT;
    }
    printf("(Keyword)\n");
    return token;
  case String:
    printf("(String)\n");
    yylval.ustr = new UString(buffer16, pos16); return STRING;
  case Int:
    printf("(Int)\n");
    yylval.ival = i; return INTEGER;
  case Decimal:
    printf("(Decimal)\n");
    yylval.dval = strtod(buffer8, 0L); return DOUBLE;
  default:
    assert(!"unhandled numeration value in switch");
  }
  fprintf(stderr, "yylex: ERROR.\n");
  return Bad;
}

bool KJSLexer::isWhiteSpace() const
{
  return (current == ' ' || current == '\t' ||
	  current == 0x0b || current == 0x0c);
}

bool KJSLexer::isLineTerminator() const
{
  return (current == '\n' || current == '\r');
}

bool KJSLexer::isIdentLetter() const
{
  return (current >= 'a' && current <= 'z' ||
	  current >= 'A' && current <= 'Z' ||
	  current == '$' || current == '_');
}

bool KJSLexer::isDecimalDigit(unsigned short c) const
{
  return (c >= '0' && c <= '9');
}

bool KJSLexer::isHexDigit(unsigned short c) const
{
  return (c >= '0' && c <= '9' ||
	  c >= 'a' && c <= 'f' ||
	  c >= 'A' && c <= 'F');
}

bool KJSLexer::isOctalDigit(unsigned short c) const
{
  return (c >= '0' && c <= '7');
}

int KJSLexer::matchPunctuator(unsigned short c1, unsigned short c2,
			      unsigned short c3, unsigned short c4)
{
  if (c1 == '>' && c2 == '>' && c3 == '>' && c4 == '=') {
    shift(4);
    return URSHIFTEQUAL;
  } else if (c1 == '>' && c2 == '>' && c3 == '>') {
    shift(3);
    return URSHIFT;
  } else if (c1 == '<' && c2 == '<' && c3 == '=') {
    shift(3);
    return LSHIFTEQUAL;
  } else if (c1 == '>' && c2 == '>' && c3 == '=') {
    shift(3);
    return RSHIFTEQUAL;
  } else if (c1 == '<' && c2 == '=') {
    shift(2);
    return LE;
  } else if (c1 == '>' && c2 == '=') {
    shift(2);
    return GE;
  } else if (c1 == '!' && c2 == '=') {
    shift(2);
    return NE;
  } else if (c1 == '+' && c2 == '+') {
    shift(2);
    return PLUSPLUS;
  } else if (c1 == '-' && c2 == '-') {
    shift(2);
    return MINUSMINUS;
  } else if (c1 == '=' && c2 == '=') {
    shift(2);
    return EQEQ;
  } else if (c1 == '+' && c2 == '=') {
    shift(2);
    return PLUSEQUAL;
  } else if (c1 == '-' && c2 == '=') {
    shift(2);
    return MINUSEQUAL;
  } else if (c1 == '*' && c2 == '=') {
    shift(2);
    return MULTEQUAL;
  } else if (c1 == '/' && c2 == '=') {
    shift(2);
    return DIVEQUAL;
  } else if (c1 == '&' && c2 == '=') {
    shift(2);
    return ANDEQUAL;
  } else if (c1 == '^' && c2 == '=') {
    shift(2);
    return XOREQUAL;
  } else if (c1 == '%' && c2 == '=') {
    shift(2);
    return MODEQUAL;
  } else if (c1 == '<' && c2 == '=') {
    shift(2);
    return OREQUAL;
  } else if (c1 == '<' && c2 == '<') {
    shift(2);
    return LSHIFT;
  } else if (c1 == '>' && c2 == '>') {
    shift(2);
    return RSHIFT;
  } else if (c1 == '&' && c2 == '&') {
    shift(2);
    return AND;
  } else if (c1 == '|' && c2 == '|') {
    shift(2);
    return OR;
  }

  switch(c1) {
    case '=':
    case '>':
    case '<':
    case ',':
    case '!':
    case '~':
    case '?':
    case ':':
    case '.':
    case '+':
    case '-':
    case '*':
    case '/':
    case '&':
    case '|':
    case '^':
    case '%':
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case ';':
      shift(1);
      return static_cast<int>(c1);
    default:
      return -1;
  }
}

unsigned char KJSLexer::singleEscape(unsigned short c) const
{
  switch(c) {
  case 'b':
    return 0x08;
    break;
  case 't':
    return 0x09;
    break;
  case 'n':
    return 0x0A;
    break;
  case 'f':
    return 0x0C;
    break;
  case 'r':
    return 0x0D;
    break;
  case '"':
    return 0x22;
    break;
  case '\'':
    return 0x27;
    break;
  case '\\':
    return 0x5C;
    break;
  default:
    return c;
  }
}

unsigned short KJSLexer::convertOctal(unsigned short c1, unsigned short c2,
                                      unsigned short c3) const
{
  return ((c1 - '0') * 64 + (c2 - '0') * 8 + c3 - '0');
}

unsigned char KJSLexer::convertHex(unsigned short c) const
{
  if (c >= '0' && c <= '9')
    return (c - '0');
  else if (c >= 'a' && c <= 'f')
    return (c - 'a' + 10);
  else
    return (c - 'A' + 10);
}

unsigned char KJSLexer::convertHex(unsigned short c1, unsigned short c2) const
{
  return (convertHex(c1) << 4 + convertHex(c2));
}

UnicodeChar KJSLexer::convertUnicode(unsigned short c1, unsigned short c2,
                                     unsigned short c3, unsigned short c4) const
{
  return UnicodeChar(convertHex(c1) << 4 + convertHex(c2),
                     convertHex(c3) << 4 + convertHex(c4));
}

void KJSLexer::record8(unsigned short c)
{
  assert(c <= 0xff);

  // enlarge buffer if full
  if (pos8 >= size8 - 1) {
    char *tmp = new char[2 * size8];
    memcpy(tmp, buffer8, size8 * sizeof(char));
    delete [] buffer8;
    buffer8 = tmp;
    size8 *= 2;
  }

  buffer8[pos8++] = (char) c;
}

void KJSLexer::record16(unsigned char c)
{
  record16(UnicodeChar(0, c));
}

void KJSLexer::record16(UnicodeChar c)
{
  // enlarge buffer if full
  if (pos16 >= size16 - 1) {
    UnicodeChar *tmp = new UnicodeChar[2 * size16];
    memcpy(tmp, buffer16, size16 * sizeof(UnicodeChar));
    delete [] buffer16;
    buffer16 = tmp;
    size16 *= 2;
  }

  buffer16[pos16++] = c;
}

int KJSLexer::lookupKeyword(const char *text)
{
  struct keyword *p = keytable;

  while (p->name) {
    if (!strcmp(text, p->name))
      return p->token;
    else
      p++;
  }
  return 0;
}

struct KJSLexer::keyword KJSLexer::keytable[] = {
  // literals
  { "null",      NULLTOKEN },
  { "true",      TRUETOKEN },
  { "false",     FALSETOKEN },
  // keywords
  { "break",     BREAK },
  { "for",       FOR },
  { "new",       NEW },
  { "var",       VAR },
  { "continue",  CONTINUE },
  { "function",  FUNCTION },
  { "return",    RETURN },
  { "void",      VOID },
  { "delete",    DELETE },
  { "if",        IF },
  { "this",      THIS },
  { "while",     WHILE },
  { "else",      ELSE },
  { "in",        IN },
  { "typeof",    TYPEOF },
  { "with",      WITH },
  // reserved for future use
  { "case",      RESERVED },
  { "debugger",  RESERVED },
  { "export",    RESERVED },
  { "super",     RESERVED },
  { "catch",     RESERVED },
  { "default",   RESERVED },
  { "extends",   RESERVED },
  { "switch",    RESERVED },
  { "class",     RESERVED },
  { "do",        RESERVED },
  { "finally",   RESERVED },
  { "throw",     RESERVED },
  { "const",     RESERVED },
  { "enum",      RESERVED },
  { "import",    RESERVED },
  { "try",       RESERVED },
  // extension for debugging purposes
  { "print",     PRINT },
  { "alert",     ALERT },
  { 0,             0      }
};
