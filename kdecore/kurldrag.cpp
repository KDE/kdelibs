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

KURLDrag::KURLDrag( const KURL::List &urls, QWidget* dragSource, const char * name )
    : QUriDrag(dragSource, name), m_metaData()
{
    init(urls);
}

KURLDrag::KURLDrag( const KURL::List &urls, const QMap<QString,QString>& metaData,
                    QWidget* dragSource, const char * name )
    : QUriDrag(dragSource, name), m_metaData(metaData)
{
    init(urls);
}

KURLDrag::~KURLDrag()
{
}

void KURLDrag::init(const KURL::List &urls)
{
    KURL::List::ConstIterator uit = urls.begin();
    KURL::List::ConstIterator uEnd = urls.end();
    // Get each URL encoded in utf8 - and since we get it in escaped
    // form on top of that, .latin1() is fine.
    for ( ; uit != uEnd ; ++uit )
        m_urls.append( (*uit).url(0, 106).latin1() ); // 106 is mib enum for utf8 codec
    setUris(m_urls);
}

KURLDrag * KURLDrag::newDrag( const KURL::List &urls, QWidget* dragSource, const char * name )
{
    return new KURLDrag( urls, QMap<QString, QString>(), dragSource, name );
}

KURLDrag * KURLDrag::newDrag( const KURL::List &urls, const QMap<QString, QString>& metaData,
                              QWidget* dragSource, const char * name )
{
    return new KURLDrag( urls, metaData, dragSource, name );
}

bool KURLDrag::decode( const QMimeSource *e, KURL::List &uris )
{
    QStrList lst;
    bool ret = QUriDrag::decode( e, lst );
    for (QStrListIterator it(lst); *it; ++it)
      uris.append(KURL(*it, 106)); // 106 is mib enum for utf8 codec
    return ret;
}

bool KURLDrag::decode( const QMimeSource *e, KURL::List &uris, QMap<QString,QString>& metaData )
{
    if ( decode( e, uris ) ) // first decode the URLs (see above)
    {
        QByteArray ba = e->encodedData( "application/x-kio-metadata" );
        if ( ba.size() )
        {
            QString s = ba.data();
            QStringList l = QStringList::split( "$@@$", s );
            QStringList::ConstIterator it = l.begin();
            bool readingKey = true; // true, then false, then true, etc.
            QString key;
            for ( ; it != l.end(); ++it ) {
                if ( readingKey )
                    key = *it;
                else
                    metaData.replace( key, *it );
                readingKey = !readingKey;
            }
            Q_ASSERT( readingKey ); // an odd number of items would be, well, odd ;-)
        }
        return true; // Success, even if no metadata was found
    }
    return false; // Couldn't decode the URLs
}

#ifdef Q_WS_QWS
bool KURLDrag::decode( QStringList const &e, KURL::List &uris )
{
    for(QStringList::ConstIterator it=e.begin(); it!=e.end(); it++)
      uris.append(KURL(*it, 106)); // 106 is mib enum for utf8 codec
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
    else if ( i == 2 )
        return "application/x-kio-metadata";
    else if ( i == 3 ) //Support this for apps that use plain XA_STRING clipboard
        return "text/plain;charset=ISO-8859-1";
    else if ( i == 4 ) //Support this for apps that use the UTF_STRING clipboard
        return "text/plain;charset=UTF-8"; 

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
         uris.append(KURL(*it, 106).prettyURL()); // 106 is mib enum for utf8 codec
        QCString s = uris.join( "\n" ).local8Bit();
        a.resize( s.length() + 1 ); // trailing zero
        memcpy( a.data(), s.data(), s.length() + 1 );
    }
    else if ( mimetype.lower() == "text/plain;charset=iso-8859-1")
    {
        QStringList uris;
        for (QStrListIterator it(m_urls); *it; ++it)
         uris.append(KURL(*it, 106).url(0, 4)); // 106 is mib enum for utf8 codec; 4 for latin1

        QCString s = uris.join( "\n" ).latin1();
        a.resize( s.length() + 1 ); // trailing zero
        memcpy( a.data(), s.data(), s.length() + 1 );    
    }
    else if ( mimetype.lower() == "text/plain;charset=utf-8")
    {
        QStringList uris;
        for (QStrListIterator it(m_urls); *it; ++it)
         uris.append(KURL(*it, 106).prettyURL()); // 106 is mib enum for utf8 codec
        QCString s = uris.join( "\n" ).utf8();
        a.resize( s.length() + 1 ); // trailing zero
        memcpy( a.data(), s.data(), s.length() + 1 );    
    }    
    else if ( mimetype == "application/x-kio-metadata" )
    {
        if ( !m_metaData.isEmpty() )
        {
            QString s;
            QMap<QString,QString>::ConstIterator it;
            for( it = m_metaData.begin(); it != m_metaData.end(); ++it )
            {
                s += it.key();
                s += "$@@$";
                s += it.data();
                s += "$@@$";
            }
	    a.resize( s.length() + 1 );
	    memcpy( a.data(), s.latin1(), a.size() );
        }
    }
    return a;
}

