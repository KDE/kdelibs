/*
   Copyright (c) 2003 Malte Starostik <malte@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "config.h"

#include <netdb.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <arpa/nameser.h>
#ifdef HAVE_ARPA_NAMESER8_COMPAT_H
#include <arpa/nameser8_compat.h>
#endif
#ifdef HAVE_SYS_PARAM_H
// Basically, the BSDs need this before resolv.h
#include <sys/param.h>
#endif
#include <resolv.h>
#include <sys/utsname.h>

#include <qtimer.h>

#include <klocale.h>
#include <kprocio.h>
#include <kurl.h>

#include "discovery.moc"

namespace KPAC
{
    Discovery::Discovery( QObject* parent )
        : Downloader( parent ),
          m_helper( new KProcIO )
    {
        connect( m_helper, SIGNAL( readReady( KProcIO* ) ), SLOT( helperOutput() ) );
        connect( m_helper, SIGNAL( processExited( KProcess* ) ), SLOT( failed() ) );
        *m_helper << "kpac_dhcp_helper";

        if ( !m_helper->start() )
            QTimer::singleShot( 0, this, SLOT( failed() ) );
    }

    bool Discovery::initHostName()
    {
        struct utsname uts;

        if (uname (&uts) > -1)
        {
            struct hostent *hent = gethostbyname (uts.nodename);
            if (hent != 0)
                m_hostname = QString::fromLocal8Bit( hent->h_name );
        }

        // If no hostname, try gethostname as a last resort.
        if (m_hostname.isEmpty())
        {
            char buf [256];
            if (gethostname (buf, sizeof(buf)) == 0)
            {
                buf[255] = '\0';
                m_hostname = QString::fromLocal8Bit( buf );
            }
        }
        return !m_hostname.isEmpty();
    }

    bool Discovery::checkDomain() const
    {
        // If a domain has a SOA record, don't traverse any higher.
        // Returns true if no SOA can be found (domain is "ok" to use)
        // Stick to old resolver interface for portability reasons.
        union
        {
            HEADER header;
            unsigned char buf[ PACKETSZ ];
        } response;
        int len = res_query( m_hostname.local8Bit(), C_IN, T_SOA,
                             response.buf, sizeof( response.buf ) );
        if ( len <= int( sizeof( response.header ) ) ||
             ntohs( response.header.ancount ) != 1 ) return true;
        unsigned char* pos = response.buf + sizeof( response.header );
        unsigned char* end = response.buf + len;
        // skip query section
        pos += dn_skipname( pos, end ) + QFIXEDSZ;
        if ( pos >= end ) return true;
        // skip answer domain
        pos += dn_skipname( pos, end );
        short type;
        GETSHORT( type, pos );
        return type != T_SOA;
    }

    void Discovery::failed()
    {
        setError( i18n( "Could not find a usable proxy configuration script" ) );

        // If this is the first DNS query, initialize our host name or abort
        // on failure. Otherwise abort if the current domain (which was already
        // queried for a host called "wpad" contains a SOA record)
        bool firstQuery = m_hostname.isEmpty();
        if ( ( firstQuery && !initHostName() ) ||
             ( !firstQuery && !checkDomain() ) )
        {
            emit result( false );
            return;
        }

        int dot = m_hostname.find( '.' );
        if ( dot >= 0 )
        {
            m_hostname.remove( 0, dot + 1 ); // remove one domain level
            download( KURL( "http://wpad." + m_hostname + "./wpad.dat" ) );
        }
        else emit result( false );
    }

    void Discovery::helperOutput()
    {
        m_helper->disconnect( this );
        QString line;
        m_helper->readln( line );
        download( KURL( line.trimmed() ) );
    }
}

// vim: ts=4 sw=4 et
