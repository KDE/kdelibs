/* This file is part of the KDE libraries
   Copyright (C) 1999 Ian Zepp (icszepp@islc.net)

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

#include <stdlib.h>		// random()

#include "kstringhandler.h"
#include "kglobal.h"

#include <qfontmetrics.h>
#include <qregexp.h>            // for the word ranges
#include <qstring.h>
#include <qstringlist.h>

static void parsePythonRange( const QByteArray &range, int &start, int &end )
{
    const int colon = range.find( ':' );
    if ( colon == -1 ) {
        start = range.toUInt();
        end = start;
    } else if ( colon == range.length() - 1 ) {
        start = range.left( colon ).toUInt();
    } else if ( colon == 0 ) {
        end = range.mid( 1 ).toUInt();
    } else {
        start = range.left( colon ).toInt();
        end = range.mid( colon + 1 ).toInt();
    }
}

QString KStringHandler::word( const QString &text , const char *range )
{
    // Format in: START:END
    // Note index starts a 0 (zero)
    //
    // 0:        first word to end
    // 1:3        second to fourth words
    QStringList list = text.split( " ", QString::KeepEmptyParts);
    QString tmp = "";
    QString r = range;

    if ( text.isEmpty() )
        return tmp;

    int pos = 0, cnt = list.count();
    parsePythonRange( range, pos, cnt );

    //
    // Extract words
    //
    int wordsToExtract = cnt-pos+1;

    while ( pos < list.count() && (wordsToExtract-- > 0))
    {
       tmp += list.at(pos);
       tmp += " ";
       ++pos;
    }

    return tmp.trimmed();
}

//
// Insertion and removal routines
//
QString KStringHandler::insword( const QString &text , const QString &word , int pos )
{
    if ( text.isEmpty() )
        return word;

    if ( word.isEmpty() )
        return text;

    // Split words and add into list
    QStringList list = text.split( " ", QString::KeepEmptyParts);

    if ( pos >= list.count() )
        list.append( word );
    else
        list.insert( pos, word );

    // Rejoin
    return list.join( " " );
}

QString KStringHandler::setword( const QString &text , const QString &word , int pos )
{
    if ( text.isEmpty() )
        return word;

    if ( word.isEmpty() )
        return text;

    // Split words and add into list
    QStringList list = text.split( " ", QString::KeepEmptyParts);

    if ( pos >= list.count() )
        list.append( word );
    else
    {
        list.insert( list.remove( list.at(pos) ) , word );
    }

    // Rejoin
    return list.join( " " );
}

QString KStringHandler::remrange( const QString &text , const char *range )
{
    // Format in: START:END
    // Note index starts a 0 (zero)
    //
    // 0:        first word to end
    // 1:3        second to fourth words
    QStringList list = text.split( " ", QString::KeepEmptyParts);
    QString tmp = "";
    QString r = range;

    if ( text.isEmpty() )
        return tmp;

    int pos = 0, cnt = list.count();
    parsePythonRange( range, pos, cnt );

    //
    // Remove that range of words
    //
    int wordsToDelete = cnt-pos+1;

    while ( !list.isEmpty() && (wordsToDelete-- > 0))
       list.removeAt(pos);

    return list.join( " " );
}

QString KStringHandler::remword( const QString &text , int pos )
{
    QString tmp = "";

    if ( text.isEmpty() )
        return tmp;

    // Split words and add into list
    QStringList list = text.split( " ", QString::KeepEmptyParts);

    if ( pos < list.count() )
        list.remove( list.at( pos ) );

    // Rejoin
    return list.join( " " );
}

QString KStringHandler::remword( const QString &text , const QString &word )
{
    QString tmp = "";

    if ( text.isEmpty() )
        return tmp;

    if ( word.isEmpty() )
        return text;

    // Split words and add into list
    QStringList list = text.split( " ", QString::KeepEmptyParts);

    QStringList::Iterator it = list.find(word);

    if (it != list.end())
       list.remove( it );

    // Rejoin
    return list.join( " " );
}

//
// Capitalization routines
//
QString KStringHandler::capwords( const QString &text )
{
    if ( text.isEmpty() ) {
        return text;
    }

    const QString strippedText = text.trimmed();
    const QStringList words = capwords( strippedText.split(' '));
		
					
    QString result = text;
    result.replace( strippedText, words.join( " " ) );
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

//
// Reverse routines
//
QString KStringHandler::reverse( const QString &text )
{
    QString tmp;

    if ( text.isEmpty() )
        return tmp;

    QStringList list;
    list = text.split( " ", QString::KeepEmptyParts);
    list = reverse( list );

    return list.join( " " );
}

QStringList KStringHandler::reverse( const QStringList &list )
{
    QStringList tmp;

    if ( list.count() == 0 )
        return tmp;

    for ( QStringList::ConstIterator it= list.begin();
          it != list.end();
          it++)
        tmp.prepend( *it );

    return tmp;
}


QString KStringHandler::center( const QString &text , int width )
{
    const QString s = text.trimmed();
    const int length = s.length();
    if ( width <= length ) {
        return s;
     }

    QString result;
    result.fill( ' ', ( width - length ) / 2 );
    result += s;

    return result.leftJustified( width );
}

QString KStringHandler::lsqueeze( const QString & str, int maxlen )
{
  if (str.length() > maxlen) {
    int part = maxlen-3;
    return QString("..." + str.right(part));
  }
  else return str;
}

QString KStringHandler::csqueeze( const QString & str, int maxlen )
{
  if (str.length() > maxlen && maxlen > 3) {
    int part = (maxlen-3)/2;
    return QString(str.left(part) + "..." + str.right(part));
  }
  else return str;
}

QString KStringHandler::rsqueeze( const QString & str, int maxlen )
{
  if (str.length() > maxlen) {
    int part = maxlen-3;
    return QString(str.left(part) + "...");
  }
  else return str;
}

QString KStringHandler::lEmSqueeze(const QString &name, const QFontMetrics& fontMetrics, int maxlen)
{
  return lPixelSqueeze(name, fontMetrics, fontMetrics.maxWidth() * maxlen);
}

QString KStringHandler::lPixelSqueeze(const QString& name, const QFontMetrics& fontMetrics, int maxPixels)
{
  int nameWidth = fontMetrics.width(name);

  if (maxPixels < nameWidth)
  {
    QString tmp = name;
    const int em = fontMetrics.maxWidth();
    maxPixels -= fontMetrics.width("...");

    while (maxPixels < nameWidth && !tmp.isEmpty())
    {
      int delta = (nameWidth - maxPixels) / em;
      delta = kClamp(delta, 1, delta); // no max

      tmp.remove(0, delta);
      nameWidth = fontMetrics.width(tmp);
    }

    return ("..." + tmp);
  }

  return name;
}

QString KStringHandler::cEmSqueeze(const QString& name, const QFontMetrics& fontMetrics, int maxlen)
{
  return cPixelSqueeze(name, fontMetrics, fontMetrics.maxWidth() * maxlen);
}

QString KStringHandler::cPixelSqueeze(const QString& s, const QFontMetrics& fm, int width)
{
  if ( s.isEmpty() || fm.width( s ) <= width ) {
    return s;
  }

  const unsigned int length = s.length();
  if ( length == 2 ) {
    return s;
  }

  const int maxWidth = width - fm.width( '.' ) * 3;
  if ( maxWidth <= 0 ) {
    return "...";
  }

  int leftIdx = 0, rightIdx = length;
  int leftWidth = fm.charWidth( s, leftIdx++ );
  int rightWidth = fm.charWidth( s, --rightIdx );
  while ( leftWidth + rightWidth < maxWidth ) {
    while ( leftWidth <= rightWidth && leftWidth + rightWidth < maxWidth ) {
      leftWidth += fm.charWidth( s, leftIdx++ );
    }
    while ( rightWidth <= leftWidth && leftWidth + rightWidth < maxWidth ) {
      rightWidth += fm.charWidth( s, --rightIdx );
    }
  }

  if ( leftWidth > rightWidth ) {
    --leftIdx;
  } else {
    ++rightIdx;
  }

  rightIdx = length - rightIdx;
  if ( leftIdx == 0 && rightIdx == 1 || leftIdx == 1 && rightIdx == 0 ) {
    return "...";
  }

  return s.left( leftIdx ) + "..." + s.right( rightIdx );
}

QString KStringHandler::rEmSqueeze(const QString& name, const QFontMetrics& fontMetrics, int maxlen)
{
  return rPixelSqueeze(name, fontMetrics, fontMetrics.maxWidth() * maxlen);
}

QString KStringHandler::rPixelSqueeze(const QString& name, const QFontMetrics& fontMetrics, int maxPixels)
{
  int nameWidth = fontMetrics.width(name);

  if (maxPixels < nameWidth)
  {
    QString tmp = name;
    const int em = fontMetrics.maxWidth();
    maxPixels -= fontMetrics.width("...");

    while (maxPixels < nameWidth && !tmp.isEmpty())
    {
      int length = tmp.length();
      int delta = em ? (nameWidth - maxPixels) / em : length;
      delta = kClamp(delta, 1, length) ;

      tmp.remove(length - delta, delta);
      nameWidth = fontMetrics.width(tmp);
    }

    return (tmp + "...");
  }

  return name;
}

///// File name patterns (like *.txt)

bool KStringHandler::matchFileName( const QString& filename, const QString& pattern  )
{
   int len = filename.length();
   int pattern_len = pattern.length();

   if (!pattern_len)
      return false;

   // Patterns like "Makefile*"
   if ( pattern[ pattern_len - 1 ] == '*' && len + 1 >= pattern_len ) {
      if ( pattern[ 0 ] == '*' )
      {
         return filename.find(pattern.mid(1, pattern_len - 2)) != -1;
      }

      const QChar *c1 = pattern.unicode();
      const QChar *c2 = filename.unicode();
      int cnt = 1;
      while ( cnt < pattern_len && *c1++ == *c2++ )
         ++cnt;
      return cnt == pattern_len;
   }

   // Patterns like "*~", "*.extension"
   if ( pattern[ 0 ] == '*' && len + 1 >= pattern_len )
   {
     const QChar *c1 = pattern.unicode() + pattern_len - 1;
     const QChar *c2 = filename.unicode() + len - 1;
     int cnt = 1;
     while ( cnt < pattern_len && *c1-- == *c2-- )
        ++cnt;
     return cnt == pattern_len;
  }

   // Patterns like "Makefile"
   return ( filename == pattern );
}

  QStringList
KStringHandler::perlSplit(const QString & sep, const QString & s, int max)
{
  bool ignoreMax = 0 == max;

  QStringList l;

  int searchStart = 0;

  int tokenStart = s.find(sep, searchStart);

  while (-1 != tokenStart && (ignoreMax || l.count() < max - 1))
  {
    if (!s.mid(searchStart, tokenStart - searchStart).isEmpty())
      l << s.mid(searchStart, tokenStart - searchStart);

    searchStart = tokenStart + sep.length();
    tokenStart = s.find(sep, searchStart);
  }

  if (!s.mid(searchStart, s.length() - searchStart).isEmpty())
    l << s.mid(searchStart, s.length() - searchStart);

  return l;
}

  QStringList
KStringHandler::perlSplit(const QChar & sep, const QString & s, int max)
{
  bool ignoreMax = 0 == max;

  QStringList l;

  int searchStart = 0;

  int tokenStart = s.find(sep, searchStart);

  while (-1 != tokenStart && (ignoreMax || l.count() < max - 1))
  {
    if (!s.mid(searchStart, tokenStart - searchStart).isEmpty())
      l << s.mid(searchStart, tokenStart - searchStart);

    searchStart = tokenStart + 1;
    tokenStart = s.find(sep, searchStart);
  }

  if (!s.mid(searchStart, s.length() - searchStart).isEmpty())
    l << s.mid(searchStart, s.length() - searchStart);

  return l;
}

  QStringList
KStringHandler::perlSplit(const QRegExp & sep, const QString & s, int max)
{
  bool ignoreMax = 0 == max;

  QStringList l;

  int searchStart = 0;
  int tokenStart = sep.search(s, searchStart);
  int len = sep.matchedLength();

  while (-1 != tokenStart && (ignoreMax || l.count() < max - 1))
  {
    if (!s.mid(searchStart, tokenStart - searchStart).isEmpty())
      l << s.mid(searchStart, tokenStart - searchStart);

    searchStart = tokenStart + len;
    tokenStart = sep.search(s, searchStart);
    len = sep.matchedLength();
  }

  if (!s.mid(searchStart, s.length() - searchStart).isEmpty())
    l << s.mid(searchStart, s.length() - searchStart);

  return l;
}

 QString
KStringHandler::tagURLs( const QString& text )
{
    /*static*/ QRegExp urlEx("(www\\.(?!\\.)|(fish|(f|ht)tp(|s))://)[\\d\\w\\./,:_~\\?=&;#@\\-\\+\\%\\$]+[\\d\\w/]");

    QString richText( text );
    int urlPos = 0, urlLen;
    while ((urlPos = urlEx.search(richText, urlPos)) >= 0)
    {
        urlLen = urlEx.matchedLength();
        QString href = richText.mid( urlPos, urlLen );
        // Qt doesn't support (?<=pattern) so we do it here
        if((urlPos > 0) && richText[urlPos-1].isLetterOrNumber()){
            urlPos++;
            continue;
        }
        // Don't use QString::arg since %01, %20, etc could be in the string
        QString anchor = "<a href=\"" + href + "\">" + href + "</a>";
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
    result += ( unicode[ i ].unicode() < 0x20 ) ? unicode[ i ] :
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
  for (i = 0; (c = buf[i]); i++) {
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

      for (n = 0; n < following; n++) {
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
    return QString::null;
  if (!*str) {
    static const QString &emptyString = KGlobal::staticQString("");
    return emptyString;
  }
  return KStringHandler::isUtf8( str ) ?
             QString::fromUtf8( str ) : 
             QString::fromLocal8Bit( str );
}
