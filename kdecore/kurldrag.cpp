/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kurldrag.h"
#include <qstrlist.h>
#include <qdragobject.h>
#include <qfont.h>

QUriDrag * KURLDrag::newDrag( const KURL::List &urls, QWidget* dragSource, const char * name )
{
    QStrList uris;
    KURL::List::ConstIterator uit = urls.begin();
    KURL::List::ConstIterator uEnd = urls.end();
    // Get each URL encoded in utf8 - and since we get it in escaped
    // form on top of that, .latin1() is fine.
    for ( ; uit != uEnd ; ++uit )
        uris.append( (*uit).url(0, QFont::Unicode).latin1() );
    return new KURLDrag( uris, dragSource, name );
}

bool KURLDrag::decode( const QMimeSource *e, KURL::List &uris )
{
    QStrList lst;
    bool ret = QUriDrag::decode( e, lst );
    for (QStrListIterator it(lst); *it; ++it)
      uris.append(KURL(*it, QFont::Unicode));
    return ret;
}

#ifdef Q_WS_QWS
bool KURLDrag::decode( QStringList const &e, KURL::List &uris )
{
    for(QStringList::ConstIterator it=e.begin(); it!=e.end(); it++)
      uris.append(KURL(*it, QFont::Unicode));
    return true;
}
#endif

////

const char * KURLDrag::format( int i ) const
{
    if ( i == 0 )
        return "text/uri-list";
    else if ( i == 1 )
        return "text/plain";
    else return 0;
}

QByteArray KURLDrag::encodedData( const char* mime ) const
{
    QByteArray a;
    QCString mimetype( mime );
    if ( mimetype == "text/uri-list" )
        return QUriDrag::encodedData( mime );
    else if ( mimetype == "text/plain" )
    {
	QStringList uris;
        for (QStrListIterator it(m_urls); *it; ++it)
         uris.append(KURL(*it, QFont::Unicode).prettyURL());
        QCString s = uris.join( "\n" ).local8Bit();
        a.resize( s.length() + 1 ); // trailing zero
        memcpy( a.data(), s.data(), s.length() + 1 );
    }
    return a;
}

