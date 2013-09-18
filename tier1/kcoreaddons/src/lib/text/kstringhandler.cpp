/* This file is part of the KDE libraries
   Copyright (C) 1999 Ian Zepp (icszepp@islc.net)
   Copyright (C) 2006 by Dominic Battre <dominic@battre.de>
   Copyright (C) 2006 by Martin Pool <mbp@canonical.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kstringhandler.h"

#include <stdlib.h>		// random()

#include <QtCore/QRegExp>            // for the word ranges
#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QCollator>


//
// Capitalization routines
//
QString KStringHandler::capwords( const QString &text )
{
    if ( text.isEmpty() ) {
        return text;
    }

    const QString strippedText = text.trimmed();
    const QString space = QString(QLatin1Char(' '));
    const QStringList words = capwords(strippedText.split(space));

    QString result = text;
    result.replace(strippedText, words.join(space));
    return result;
}

QStringList KStringHandler::capwords( const QStringList &list )
{
    QStringList tmp = list;
    for ( QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it ) {
        *it = ( *it )[ 0 ].toUpper() + ( *it ).mid( 1 );
    }
    return tmp;
}


QString KStringHandler::lsqueeze( const QString & str, int maxlen )
{
  if (str.length() > maxlen) {
    int part = maxlen-3;
    return QString::fromLatin1("...") + str.right(part);
  }
  else return str;
}

QString KStringHandler::csqueeze( const QString & str, int maxlen )
{
  if (str.length() > maxlen && maxlen > 3) {
    const int part = (maxlen-3)/2;
    return str.left(part) + QLatin1String("...") + str.right(part);
  }
  else return str;
}

QString KStringHandler::rsqueeze( const QString & str, int maxlen )
{
  if (str.length() > maxlen) {
    int part = maxlen-3;
    return str.left(part) + QLatin1String("...");
  }
  else return str;
}

QStringList KStringHandler::perlSplit(const QString & sep, const QString & s, int max)
{
  bool ignoreMax = 0 == max;

  QStringList l;

  int searchStart = 0;

  int tokenStart = s.indexOf(sep, searchStart);

  while (-1 != tokenStart && (ignoreMax || l.count() < max - 1))
  {
    if (!s.mid(searchStart, tokenStart - searchStart).isEmpty())
      l << s.mid(searchStart, tokenStart - searchStart);

    searchStart = tokenStart + sep.length();
    tokenStart = s.indexOf(sep, searchStart);
  }

  if (!s.mid(searchStart, s.length() - searchStart).isEmpty())
    l << s.mid(searchStart, s.length() - searchStart);

  return l;
}

QStringList KStringHandler::perlSplit(const QChar & sep, const QString & s, int max)
{
  bool ignoreMax = 0 == max;

  QStringList l;

  int searchStart = 0;

  int tokenStart = s.indexOf(sep, searchStart);

  while (-1 != tokenStart && (ignoreMax || l.count() < max - 1))
  {
    if (!s.mid(searchStart, tokenStart - searchStart).isEmpty())
      l << s.mid(searchStart, tokenStart - searchStart);

    searchStart = tokenStart + 1;
    tokenStart = s.indexOf(sep, searchStart);
  }

  if (!s.mid(searchStart, s.length() - searchStart).isEmpty())
    l << s.mid(searchStart, s.length() - searchStart);

  return l;
}

QStringList KStringHandler::perlSplit(const QRegExp & sep, const QString & s, int max)
{
  bool ignoreMax = 0 == max;

  QStringList l;

  int searchStart = 0;
  int tokenStart = sep.indexIn(s, searchStart);
  int len = sep.matchedLength();

  while (-1 != tokenStart && (ignoreMax || l.count() < max - 1))
  {
    if (!s.mid(searchStart, tokenStart - searchStart).isEmpty())
      l << s.mid(searchStart, tokenStart - searchStart);

    searchStart = tokenStart + len;
    tokenStart = sep.indexIn(s, searchStart);
    len = sep.matchedLength();
  }

  if (!s.mid(searchStart, s.length() - searchStart).isEmpty())
    l << s.mid(searchStart, s.length() - searchStart);

  return l;
}

QString KStringHandler::tagUrls( const QString& text )
{
    /*static*/ QRegExp urlEx(QLatin1String("(www\\.(?!\\.)|(fish|(f|ht)tp(|s))://)[\\d\\w\\./,:_~\\?=&;#@\\-\\+\\%\\$]+[\\d\\w/]"));

    QString richText( text );
    int urlPos = 0, urlLen;
    while ((urlPos = urlEx.indexIn(richText, urlPos)) >= 0)
    {
        urlLen = urlEx.matchedLength();
        QString href = richText.mid( urlPos, urlLen );
        // Qt doesn't support (?<=pattern) so we do it here
        if((urlPos > 0) && richText[urlPos-1].isLetterOrNumber()){
            urlPos++;
            continue;
        }
        // Don't use QString::arg since %01, %20, etc could be in the string
        QString anchor = QString::fromLatin1("<a href=\"") + href + QLatin1String("\">") + href + QLatin1String("</a>");
        richText.replace( urlPos, urlLen, anchor );


        urlPos += anchor.length();
    }
    return richText;
}

QString KStringHandler::obscure( const QString &str )
{
  QString result;
  const QChar *unicode = str.unicode();
  for ( int i = 0; i < str.length(); ++i )
    // yes, no typo. can't encode ' ' or '!' because
    // they're the unicode BOM. stupid scrambling. stupid.
    result += ( unicode[ i ].unicode() <= 0x21 ) ? unicode[ i ] :
        QChar( 0x1001F - unicode[ i ].unicode() );

  return result;
}


bool KStringHandler::isUtf8( const char *buf )
{
  int i, n;
  register unsigned char c;
  bool gotone = false;

  if (!buf)
    return true; // whatever, just don't crash

#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

  static const unsigned char text_chars[256] = {
        /*                  BEL BS HT LF    FF CR    */
        F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
        /*                              ESC          */
        F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
        /*            NEL                            */
        X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
  };

  /* *ulen = 0; */
  for (i = 0; (c = buf[i]); ++i) {
    if ((c & 0x80) == 0) {        /* 0xxxxxxx is plain ASCII */
      /*
       * Even if the whole file is valid UTF-8 sequences,
       * still reject it if it uses weird control characters.
       */

      if (text_chars[c] != T)
        return false;

    } else if ((c & 0x40) == 0) { /* 10xxxxxx never 1st byte */
      return false;
    } else {                           /* 11xxxxxx begins UTF-8 */
      int following;

    if ((c & 0x20) == 0) {             /* 110xxxxx */
      following = 1;
    } else if ((c & 0x10) == 0) {      /* 1110xxxx */
      following = 2;
    } else if ((c & 0x08) == 0) {      /* 11110xxx */
      following = 3;
    } else if ((c & 0x04) == 0) {      /* 111110xx */
      following = 4;
    } else if ((c & 0x02) == 0) {      /* 1111110x */
      following = 5;
    } else
      return false;

      for (n = 0; n < following; ++n) {
        i++;
        if (!(c = buf[i]))
          goto done;

        if ((c & 0x80) == 0 || (c & 0x40))
          return false;
      }
      gotone = true;
    }
  }
done:
  return gotone;   /* don't claim it's UTF-8 if it's all 7-bit */
}

#undef F
#undef T
#undef I
#undef X

QString KStringHandler::from8Bit( const char *str )
{
  if (!str)
    return QString();
  if (!*str) {
    static const QLatin1String emptyString("");
    return emptyString;
  }
  return KStringHandler::isUtf8( str ) ?
             QString::fromUtf8( str ) :
             QString::fromLocal8Bit( str );
}

int KStringHandler::naturalCompare(const QString &a, const QString &b, Qt::CaseSensitivity caseSensitivity)
{
    QCollator c;
    c.setCaseSensitivity(caseSensitivity);
    c.setNumericMode(true);
    return c.compare(a, b);
}

QString KStringHandler::preProcessWrap(const QString &text)
{
    const QChar zwsp(0x200b);

    QString result;
    result.reserve(text.length());

    for (int i = 0; i < text.length(); i++) {
        const QChar c = text[i];
        bool openingParens = (c == QLatin1Char('(') || c == QLatin1Char('{') || c == QLatin1Char('['));
        bool singleQuote = (c == QLatin1Char('\'') );
        bool closingParens = (c == QLatin1Char(')') || c == QLatin1Char('}') || c == QLatin1Char(']'));
        bool breakAfter   = (closingParens || c.isPunct() || c.isSymbol());
        bool nextIsSpace  = (i == (text.length() - 1) || text[i + 1].isSpace());
        bool prevIsSpace  = (i == 0 || text[i - 1].isSpace() || result[result.length() - 1] == zwsp);

        // Provide a breaking opportunity before opening parenthesis
        if (openingParens && !prevIsSpace)
            result += zwsp;
        
        // Provide a word joiner before the single quote
        if (singleQuote && !prevIsSpace)
            result += QChar(0x2060);

        result += c;

        if (breakAfter && !openingParens && !nextIsSpace && !singleQuote) 
            result += zwsp;
    }

    return result;
}

