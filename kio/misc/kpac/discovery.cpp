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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id$

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

        char buf[ 256 ];
        gethostname( buf, sizeof( buf ) );
        buf[ 255 ] = 0;
        m_hostname = QString::fromLocal8Bit( buf );

        if ( !m_helper->start() )
            QTimer::singleShot( 0, this, SLOT( failed() ) );
    }

    void Discovery::failed()
    {
        setError( i18n( "Could not find a usable proxy configuration script" ) );
        int dot = m_hostname.find( '.' );
        if ( dot >= 0 )
        {
            m_hostname.remove( 0, dot + 1 ); // remove one level
            dot = m_hostname.find( '.' );    // require TLD and SLD
            if ( dot >= 0 )
                download( "http://wpad." + m_hostname + "./wpad.dat" );
            else emit result( false );
        }
        else emit result( false );
    }

    void Discovery::helperOutput()
    {
        m_helper->disconnect( this );
        QString line;
        m_helper->readln( line );
        download( line.stripWhiteSpace() );
    }
}

// vim: ts=4 sw=4 et
