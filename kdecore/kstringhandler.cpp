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

#include "kstringhandler.h"

KStringHandler::KStringHandler() {}
KStringHandler::~KStringHandler() {}


QString KStringHandler::word( QString &text , uint pos )
{
    QStrList list = split( text , " " );
    QString tmp = "";

    if ( pos < list.count() )
        tmp = list.at( pos );

    return tmp;
}
QString KStringHandler::word( QString &text , const char *range )
{
    // Format in: START:END
    // Note index starts a 0 (zero)
    //
    // 0:        first word to end
    // 1:3        second to fourth words
    QStrList list = split( text , " " );
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
        pos = atoi( r.left(at) );
        cnt = atoi( r.remove(0,at+1) );
    }
    else if ( r.find(QRegExp(":+[0-9]+")) != -1 )
    {
        at  = r.find(":");
        pos = 0;
        cnt = atoi( r.remove(0,at+1) );
    }
    else if ( r.find(QRegExp("[0-9]+:+")) != -1 )
    {
        at  = r.find(":");
        pos = atoi( r.left(at) );
        cnt = list.count(); // zero index
    }
    else if ( r.find(QRegExp("[0-9]+")) != -1 )
    {
        pos = atoi( r );
        cnt = pos;
    }
    else
    {
        return tmp; // not found/implemented
    }

    //
    // Extract words
    //
    while ( pos <= cnt )
    {
        if ((uint) pos >= list.count() )
            break;

        tmp += list.at(pos);
        tmp += " ";
        pos++;
    }

    return (tmp = tmp.stripWhiteSpace());
}

//
// Insertion and removal routines
//
QString KStringHandler::insword( QString &text , QString &word , uint pos )
{
    QString tmp = "";

    if ( text.isEmpty() )
        return (tmp = word);

    if ( word.isEmpty() )
        return (tmp = text);

    // Split words and add into list
    QStrList list = split( text );

    if ( pos >= list.count() )
        list.append( word );
    else
        list.insert( pos , word );

    // Rejoin
    return (tmp = join( list ));
}
QString KStringHandler::setword( QString &text , QString &word , uint pos )
{
    QString tmp = "";

    if ( text.isEmpty() )
        return (tmp = word);

    if ( word.isEmpty() )
        return (tmp = text);

    // Split words and add into list
    QStrList list = split( text );

    if ( pos >= list.count() )
        list.append( word );
    else
    {
        list.remove( pos );
        list.insert( pos , word );
    }

    // Rejoin
    return (tmp = join( list ));
}
QString KStringHandler::remrange( QString &text , const char *range )
{
    // Format in: START:END
    // Note index starts a 0 (zero)
    //
    // 0:        first word to end
    // 1:3        second to fourth words
    QStrList list = split( text , " " );
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
        pos = atoi( r.left(at) );
        cnt = atoi( r.remove(0,at+1) );
    }
    else if ( r.find(QRegExp(":+[0-9]+")) != -1 )
    {
        at  = r.find(":");
        pos = 0;
        cnt = atoi( r.remove(0,at+1) );
    }
    else if ( r.find(QRegExp("[0-9]+:+")) != -1 )
    {
        at  = r.find(":");
        pos = atoi( r.left(at) );
        cnt = list.count(); // zero index
    }
    else if ( r.find(QRegExp("[0-9]+")) != -1 )
    {
        pos = atoi( r );
        cnt = pos;
    }
    else
    {
        return (tmp = text); // not found/implemented
    }

    //
    // Remove that range of words
    //
    while ( pos <= cnt )
    {
        if ((uint) pos >= list.count() )
            break;
        else
        {
            list.remove( pos );
            cnt--; // the list size changes after you remove an item,
                   // so you have to decr this to stay in the right
                   // place inside the list;
        }
    }

    return (tmp = join( list));
}
QString KStringHandler::remword( QString &text , uint pos )
{
    QString tmp = "";

    if ( text.isEmpty() )
        return tmp;

    // Split words and add into list
    QStrList list = split( text );

    if ( pos < list.count() )
        list.remove( pos );

    // Rejoin
    return (tmp = join( list ));
}
QString KStringHandler::remword( QString &text , QString &word )
{
    QString tmp = "";

    if ( text.isEmpty() )
        return tmp;

    if ( word.isEmpty() )
        return (tmp = text);

    // Split words and add into list
    QStrList list = split( text );

    int pos = -1;

    if ( (pos = list.find(word)) != -1 )
        list.remove( pos );

    // Rejoin
    return (tmp = join( list ));
}

//
// Capitalization routines
//
QString KStringHandler::capwords( QString &text )
{
    QString tmp = "";
    QString word;

    if ( text.isEmpty() )
        return tmp;

    QStrList list = split( text );
    
    for ( uint i = 0 ; i < list.count() ; i++ )
    {
        word = list.at(i);
        word = word.left(1).upper() + word.remove(0,1);

        tmp += word + " ";
    }

    return (tmp = tmp.stripWhiteSpace());
}
QStrList KStringHandler::capwords( QStrList &list )
{
    QStrList tmp;
    QString word;

    if ( list.count() == 0 )
        return tmp;

    for ( uint i = 0 ; i < list.count() ; i++ )
    {
        word = list.at(i);
        word = word.left(1).upper() + word.remove(0,1);

        tmp.append( word ); // blank list to start with
    }

    return tmp;
}

//
// Reverse routines
//
QString KStringHandler::reverse( QString &text )
{
    QString tmp;

    if ( text.isEmpty() )
        return tmp;

    QStrList list;
    list = split( text );
    list = reverse( list );

    tmp = join( list );

    return tmp;
}
QStrList KStringHandler::reverse( QStrList &list )
{
    QStrList tmp;

    if ( list.count() == 0 )
        return tmp;

    for ( uint i = 0 ; i < list.count() ; i++ )
        tmp.insert( 0 , list.at(i) );

    return tmp;
}

//
// Left, Right, Center justification
//
QString KStringHandler::ljust( QString &text , uint width )
{
    QString tmp = text;
    tmp = tmp.stripWhiteSpace(); // remove leading/trailing spaces
    
    if ( tmp.length() >= width )
        return tmp;

    for ( uint pos = tmp.length() ; pos < width ; pos++ )
        tmp.append(" ");

    return tmp;
}
QString KStringHandler::rjust( QString &text , uint width )
{
    QString tmp = text;
    tmp = tmp.stripWhiteSpace(); // remove leading/trailing spaces
    
    if ( tmp.length() >= width )
        return tmp;

    for ( uint pos = tmp.length() ; pos < width ; pos++ )
        tmp.prepend(" ");

    return tmp;
}
QString KStringHandler::center( QString &text , uint width )
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

//
// Joins words in a list together
//
QString KStringHandler::join( QStrList &list , const char *sep )
{
    QString tmp = "";

    if ( list.count() == 0 )
        return tmp;
    
    // Add all items with a sep except the last one
    for ( uint i = 0 ; i < list.count() - 1 ; i++ )
    {
        tmp += list.at( i );
        tmp += sep;
    }

    // Add the last item
    tmp += list.at( list.count() - 1 );
    return tmp;
}

//
// Splits words in a string apart
//
QStrList KStringHandler::split( QString &text , const char *sep )
{
    QStrList tmp;
    
    if ( text.isEmpty() )
        return tmp;

    int pos = -1;
    int old = 0;

    // Append words up to (not including) last
    while ( (pos = text.find( sep , old )) != -1 )
    {
        tmp.append( text.mid(old,pos - old) );
        old = pos + 1;
    }

    // Append final word
    tmp.append( text.mid(old,text.length()) );
    
    return tmp;
}
