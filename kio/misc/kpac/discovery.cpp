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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include <config.h>
#include <config-kpac.h>
#include <netdb.h>
#include <unistd.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <arpa/nameser.h>
#ifdef HAVE_ARPA_NAMESER8_COMPAT_H
#include <arpa/nameser8_compat.h>
#else
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#include <arpa/nameser_compat.h>
#endif
#endif
#ifdef HAVE_SYS_PARAM_H
// Basically, the BSDs need this before resolv.h
#include <sys/param.h>
#endif

#include <resolv.h>
#include <sys/utsname.h>

#include <QtCore/QTimer>
#include <QtNetwork/QHostInfo>

#include <klocale.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include "discovery.moc"

namespace KPAC
{
    Discovery::Discovery( QObject* parent )
        : Downloader( parent ),
          m_helper( new KProcess(this) )
    {
        m_helper->setOutputChannelMode(KProcess::SeparateChannels);
        connect( m_helper, SIGNAL(readyReadStandardOutput()), SLOT(helperOutput()) );
        connect( m_helper, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(failed()) );
        *m_helper << KStandardDirs::findExe("kpac_dhcp_helper");
        m_helper->start();
        if ( !m_helper->waitForStarted() )
            QTimer::singleShot( 0, this, SLOT(failed()) );
    }

    bool Discovery::initDomainName()
    {
        m_domainName = QHostInfo::localDomainName();
        return !m_domainName.isEmpty();
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

        int len = res_query( m_domainName.toLocal8Bit(), C_IN, T_SOA,
                             response.buf, sizeof( response.buf ) );
        if ( len <= int( sizeof( response.header ) ) ||
             ntohs( response.header.ancount ) != 1 )
            return true;

        unsigned char* pos = response.buf + sizeof( response.header );
        unsigned char* end = response.buf + len;

        // skip query section
        pos += dn_skipname( pos, end ) + QFIXEDSZ;
        if ( pos >= end )
            return true;

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
        const bool firstQuery = m_domainName.isEmpty();
        if ( ( firstQuery && !initDomainName() ) ||
             ( !firstQuery && !checkDomain() ) )
        {
            emit result( false );
            return;
        }

        const int dot = m_domainName.indexOf( '.' );
        if ( dot > -1 || firstQuery )
        {
            QString address (QLatin1String("http://wpad."));
            address += m_domainName;
            address += QLatin1String ("/wpad.dat");
            if ( dot > -1 )
                m_domainName.remove (0, dot + 1); // remove one domain level
            download( KUrl(address) );
            return;
        }

        emit result( false );
    }

    void Discovery::helperOutput()
    {
        m_helper->disconnect( this );
        const QByteArray line = m_helper->readLine();
        const KUrl url( QString::fromLocal8Bit(line.constData(), line.length()).trimmed() );
        download( url );
    }
}

// vim: ts=4 sw=4 et
