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


#include <cstdlib>
#include <cstring>

#include <qtextcodec.h>

#include <kcharsets.h>
#include <kglobal.h>
#include <klocale.h>
#include <kio/job.h>

#include "downloader.moc"

namespace KPAC
{
    Downloader::Downloader( QObject* parent )
        : QObject( parent )
    {
    }

    void Downloader::download( const KURL& url )
    {
        m_data.resize( 0 );
        m_script.clear();
        m_scriptURL = url;

        KIO::TransferJob* job = KIO::get( url, false, false );
        connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
                 SLOT( data( KIO::Job*, const QByteArray& ) ) );
        connect( job, SIGNAL( result( KIO::Job* ) ), SLOT( result( KIO::Job* ) ) );
    }

    void Downloader::failed()
    {
        emit result( false );
    }

    void Downloader::setError( const QString& error )
    {
        m_error = error;
    }

    void Downloader::data( KIO::Job*, const QByteArray& data )
    {
        unsigned offset = m_data.size();
        m_data.resize( offset + data.size() );
        std::memcpy( m_data.data() + offset, data.data(), data.size() );
    }

    void Downloader::result( KIO::Job* job )
    {
        if ( !job->error() && !static_cast< KIO::TransferJob* >( job )->isErrorPage() )
        {
            bool dummy;
            m_script = KGlobal::charsets()->codecForName(
                job->queryMetaData( "charset" ), dummy )->toUnicode( m_data );
            emit result( true );
        }
        else
        {
            if ( job->error() )
                setError( i18n( "Could not download the proxy configuration script:\n%1" )
                              .arg( job->errorString() ) );
            else setError( i18n( "Could not download the proxy configuration script" ) ); // error page
            failed();
        }
    }
}

// vim: ts=4 sw=4 et
