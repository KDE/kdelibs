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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kurldrag.h"
#include <qstrlist.h>
#include <qdragobject.h>
#include <qfont.h>
#include <unistd.h>

#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

class KURLDragPrivate
{
public:
    bool m_exportAsText;
};

KURLDrag::KURLDrag( const KURL::List &urls, QWidget* dragSource, const char * name )
    : QUriDrag(dragSource, name), m_metaData(), d( 0 )
{
    init(urls);
}

KURLDrag::KURLDrag( const KURL::List &urls, const QMap<QString,QString>& metaData,
                    QWidget* dragSource, const char * name )
    : QUriDrag(dragSource, name), m_metaData(metaData), d( 0 )
{
    init(urls);
}

KURLDrag::~KURLDrag()
{
    delete d;
}

void KURLDrag::init(const KURL::List &urls)
{
    KURL::List::ConstIterator uit = urls.begin();
    KURL::List::ConstIterator uEnd = urls.end();
    // Get each URL encoded in utf8 - and since we get it in escaped
    // form on top of that, .latin1() is fine.
    for ( ; uit != uEnd ; ++uit )
    {
        m_urls.append( urlToString(*uit).latin1() );
    }
    setUris(m_urls);
}

void KURLDrag::setExportAsText( bool exp )
{
    // For now d is only used here, so create it on demand
    if ( !d )
        d = new KURLDragPrivate;
    d->m_exportAsText = exp;
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
    if ( e->provides( "application/x-kde-urilist" ) ) {
        QByteArray payload = e->encodedData( "application/x-kde-urilist" );
        if ( payload.size() ) {
            uint c=0;
            const char* d = payload.data();
            while (c < payload.size() && d[c]) {
                uint f = c;
                // Find line end
                while (c < payload.size() && d[c] && d[c]!='\r'
                        && d[c] != '\n')
                    c++;
                QCString s(d+f,c-f+1);
                if ( s[0] != '#' ) // non-comment?
                    uris.append(stringToUrl(s));
                // Skip junk
                while (c < payload.size() && d[c] &&
                        (d[c]=='\n' || d[c]=='\r'))
                    c++;
            }
            return !uris.isEmpty();
        }
    }
    
    QStrList lst;
    QUriDrag::decode( e, lst );
    for (QStrListIterator it(lst); *it; ++it)
    {
      KURL url = stringToUrl( *it );
      if ( !url.isValid() )
      {
        uris.clear();
        break;
      }
      uris.append( url );
    }
    return !uris.isEmpty();
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
	QStringList::ConstIterator end(e.end());
    for(QStringList::ConstIterator it=e.begin(); it!=end; ++it)
    {
      KURL url = KURL( *it, 106 ); // 106 is mib enum for utf8 codec
      if ( !url.isValid() )
      {
        uris.clear();
        break;
      }
      uris.append( url );
    }
    return !uris.isEmpty();
}
#endif

////

const char * KURLDrag::format( int i ) const
{
    if ( i == 0 )
        return "text/uri-list";
    else if ( i == 1 )
        return "application/x-kio-metadata";
    if ( d && d->m_exportAsText == false )
        return 0;
    if ( i == 2 )
        return "text/plain";
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
           uris.append(stringToUrl(*it).prettyURL());

        QCString s = uris.join( "\n" ).local8Bit();
        if( uris.count() > 1 ) // terminate last line, unless it's the only line
            s.append( "\n" );
        a.resize( s.length());
        memcpy( a.data(), s.data(), s.length()); // no trailing zero in clipboard text
    }
    else if ( mimetype.lower() == "text/plain;charset=iso-8859-1")
    {
        QStringList uris;
        for (QStrListIterator it(m_urls); *it; ++it)
        for (QStrListIterator it(m_urls); *it; ++it)
           uris.append(stringToUrl(*it).url(0, 4)); // 4 is mib for latin1

        QCString s = uris.join( "\n" ).latin1();
        if( uris.count() > 1 )
            s.append( "\n" );
        a.resize( s.length());
        memcpy( a.data(), s.data(), s.length());
    }
    else if ( mimetype.lower() == "text/plain;charset=utf-8")
    {
        QStringList uris;
        for (QStrListIterator it(m_urls); *it; ++it)
           uris.append(stringToUrl(*it).prettyURL());

        QCString s = uris.join( "\n" ).utf8();
        if( uris.count() > 1 )
            s.append( "\n" );
        a.resize( s.length());
        memcpy( a.data(), s.data(), s.length());
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

KURL KURLDrag::stringToUrl(const QCString &s)
{
    if (strncmp(s.data(), "file:", 5) == 0)
       return KURL(s, KGlobal::locale()->fileEncodingMib());

    return KURL(s, 106); // 106 is mib enum for utf8 codec;
}

QString KURLDrag::urlToString(const KURL &url)
{
    if (url.isLocalFile())
    {
#if 1
        return url.url(0, KGlobal::locale()->fileEncodingMib());
#else
        // According to the XDND spec, file:/ URLs for DND must have
        // the hostname part. But in really it just breaks many apps,
        // so it's disabled for now.
        QString s = url.url(0, KGlobal::locale()->fileEncodingMib());
        if( !s.startsWith( "file://" ))
        {
            char hostname[257];
            if ( gethostname( hostname, 255 ) == 0 )
            {
	        hostname[256] = '\0';
                return QString( "file://" ) + hostname + s.mid( 5 );
            }
        }
#endif
    }

    if ( url.protocol() == "mailto" ) {
        return url.path();
    }

    return url.url(0, 106); // 106 is mib enum for utf8 codec
}

// deprecated ctor
KURLDrag::KURLDrag( const QStrList & urls, const QMap<QString,QString>& metaData,
                    QWidget * dragSource, const char* name ) :
QUriDrag( urls, dragSource, name ), m_urls( urls ), m_metaData( metaData ), d( 0 ) {}
