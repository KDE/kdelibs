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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/* AIX needs strings.h for str*casecmp(), and our config.h loads it on AIX
   So we don't need to include strings.h explicitly */
#include "config.h"

#include "kstringhandler.h"

QString KStringHandler::word( const QString &text , uint pos )
{
    QStringList list = QStringList::split( " ", text , true );

    if ( pos < list.count() )
        return list[ pos ];

    return "";
}

QString KStringHandler::word( const QString &text , const char *range )
{
    // Format in: START:END
    // Note index starts a 0 (zero)
    //
    // 0:        first word to end
    // 1:3        second to fourth words
    QStringList list = QStringList::split( " ", text , true );
    QString tmp = "";
    QString r = range;

    if ( text.isEmpty() )
        return tmp;

    // do stuff here
    QRegExp reg;

    int at = 0;
    int pos = 0;
    int cnt = 0;

    if ( r.find(QRegExp("[0-9]+:[0-9]+")) != -1 )
    {
        at  = r.find(":");
        pos = atoi( r.left(at).ascii() );
        cnt = atoi( r.remove(0,at+1).ascii() );
    }
    else if ( r.find(QRegExp(":+[0-9]+")) != -1 )
    {
        at  = r.find(":");
        pos = 0;
        cnt = atoi( r.remove(0,at+1).ascii() );
    }
    else if ( r.find(QRegExp("[0-9]+:+")) != -1 )
    {
        at  = r.find(":");
        pos = atoi( r.left(at).ascii() );
        cnt = list.count(); // zero index
    }
    else if ( r.find(QRegExp("[0-9]+")) != -1 )
    {
        pos = atoi( r.ascii() );
        cnt = pos;
    }
    else
    {
        return tmp; // not found/implemented
    }

    //
    // Extract words
    //
    int wordsToExtract = cnt-pos+1;
    QStringList::Iterator it = list.at( pos);

    while ( (it != list.end()) && (wordsToExtract-- > 0))
    {
       tmp += *it;
       tmp += " ";
       it++;
    }

    return tmp.stripWhiteSpace();
}

//
// Insertion and removal routines
//
QString KStringHandler::insword( const QString &text , const QString &word , uint pos )
{
    if ( text.isEmpty() )
        return word;

    if ( word.isEmpty() )
        return text;

    // Split words and add into list
    QStringList list = QStringList::split( " ", text, true );

    if ( pos >= list.count() )
        list.append( word );
    else
        list.insert( list.at(pos) , word );

    // Rejoin
    return list.join( " " );
}

QString KStringHandler::setword( const QString &text , const QString &word , uint pos )
{
    if ( text.isEmpty() )
        return word;

    if ( word.isEmpty() )
        return text;

    // Split words and add into list
    QStringList list = QStringList::split( " ", text, true );

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
    QStringList list = QStringList::split( " ", text , true );
    QString tmp = "";
    QString r = range;

    if ( text.isEmpty() )
        return tmp;

    // do stuff here
    QRegExp reg;

    int at = 0;
    int pos = 0;
    int cnt = 0;

    if ( r.find(QRegExp("[0-9]+:[0-9]+")) != -1 )
    {
        at  = r.find(':');
        pos = atoi( r.left(at).ascii() );
        cnt = atoi( r.remove(0,at+1).ascii() );
    }
    else if ( r.find(QRegExp(":+[0-9]+")) != -1 )
    {
        at  = r.find(':');
        pos = 0;
        cnt = atoi( r.remove(0,at+1).ascii() );
    }
    else if ( r.find(QRegExp("[0-9]+:+")) != -1 )
    {
        at  = r.find(':');
        pos = atoi( r.left(at).ascii() );
        cnt = list.count(); // zero index
    }
    else if ( r.find(QRegExp("[0-9]+")) != -1 )
    {
        pos = atoi( r.ascii() );
        cnt = pos;
    }
    else
    {
        return text; // not found/implemented
    }

    //
    // Remove that range of words
    //
    int wordsToDelete = cnt-pos+1;
    QStringList::Iterator it = list.at( pos);

    while ( (it != list.end()) && (wordsToDelete-- > 0))
       it = list.remove( it );

    return list.join( " " );
}

QString KStringHandler::remword( const QString &text , uint pos )
{
    QString tmp = "";

    if ( text.isEmpty() )
        return tmp;

    // Split words and add into list
    QStringList list = QStringList::split( " ", text, true );

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
    QStringList list = QStringList::split( " ", text, true );

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
    QString tmp = "";

    if ( text.isEmpty() )
        return tmp;

    QStringList list = QStringList::split( " ", text, true );

    return capwords( QStringList::split( " ", text, true )).join( " " );
}

QStringList KStringHandler::capwords( const QStringList &list )
{
    QStringList tmp;
    QString word;

    if ( list.count() == 0 )
        return tmp;

    for ( QStringList::ConstIterator it= list.begin();
          it != list.end();
          it++)
    {
        word = *it;
        word = word.left(1).upper() + word.remove(0,1);

        tmp.append( word ); // blank list to start with
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
    list = QStringList::split( " ", text, true );
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

//
// Left, Right, Center justification
//
QString KStringHandler::ljust( const QString &text , uint width )
{
    QString tmp = text;
    tmp = tmp.stripWhiteSpace(); // remove leading/trailing spaces

    if ( tmp.length() >= width )
        return tmp;

    for ( uint pos = tmp.length() ; pos < width ; pos++ )
        tmp.append(" ");

    return tmp;
}

QString KStringHandler::rjust( const QString &text , uint width )
{
    QString tmp = text;
    tmp = tmp.stripWhiteSpace(); // remove leading/trailing spaces

    if ( tmp.length() >= width )
        return tmp;

    for ( uint pos = tmp.length() ; pos < width ; pos++ )
        tmp.prepend(" ");

    return tmp;
}

QString KStringHandler::center( const QString &text , uint width )
{
    // Center is slightly different, in that it will add
    // spaces to the RIGHT side (left-justified) before
    // it adds a space to the LEFT side.

    QString tmp = text;
    tmp = tmp.stripWhiteSpace(); // remove leading/trailing spaces

    if ( tmp.length() >= width )
        return tmp;

    bool left = false; // start at right side.

    for ( uint pos = tmp.length() ; pos < width ; pos++ )
    {
        if ( left )
            tmp.prepend(" ");
        else
            tmp.append(" ");

        // Reverse bool
        left = !left;
    }

    return tmp;
}

QString KStringHandler::lsqueeze( const QString & str, uint maxlen )
{
  if (str.length() > maxlen) {
    int part = maxlen-3;
    return QString("..." + str.right(part));
  }
  else return str;
}

QString KStringHandler::csqueeze( const QString & str, uint maxlen )
{
  if (str.length() > maxlen && maxlen > 3) {
    int part = (maxlen-3)/2;
    return QString(str.left(part) + "..." + str.right(part));
  }
  else return str;
}

QString KStringHandler::rsqueeze( const QString & str, uint maxlen )
{
  if (str.length() > maxlen) {
    int part = maxlen-3;
    return QString(str.left(part) + "...");
  }
  else return str;
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
KStringHandler::perlSplit(const QString & sep, const QString & s, uint max)
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
KStringHandler::perlSplit(const QChar & sep, const QString & s, uint max)
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
KStringHandler::perlSplit(const QRegExp & sep, const QString & s, uint max)
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
    /*static*/ QRegExp urlEx("(www\\.|(f|ht)tp(|s)://)[\\d\\w\\./,:_~\\?=&;#@-]+[\\d\\w/]");

    QString richText( text );
    int urlPos = 0, urlLen;
    while ((urlPos = urlEx.search(richText, urlPos)) >= 0)
    {
        urlLen = urlEx.matchedLength();
        QString href = richText.mid( urlPos, urlLen );
        QString anchor( "<a href=\"%1\">%2</a>" );
        anchor = anchor.arg( href ).arg( href );
        richText.replace( urlPos, urlLen, anchor );

        urlPos += anchor.length();
    }
    return richText;
}
