/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2006 Allan Sandfeld Jensen <kde@carewolf.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 **/

#include "kio/slaveinterface.h"
#include "kio/slavebase.h"
#include "kio/connection.h"
#include "kio/scheduler.h"
#include "kio/slave.h"

#include "kio/filejob.h"

#include <QTimer>
#include <kdebug.h>


using namespace KIO;

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( &packedArgs, QIODevice::WriteOnly ); stream

FileJob::FileJob( const KUrl& url, const QByteArray &packedArgs  )
        : SimpleJob(url, CMD_OPEN, packedArgs, false), m_open(false), m_size(0)
{
}

FileJob::~FileJob()
{
}

void FileJob::read(int size)
{
    if (!m_open) return;

    KIO_ARGS << size;
    m_slave->send( CMD_READ, packedArgs );
}


void FileJob::write(const QByteArray &_data)
{
    if (!m_open) return;

    m_slave->send( CMD_WRITE, _data );
}

void FileJob::seek(int offset)
{
    if (!m_open) return;

    KIO_ARGS << offset;
    m_slave->send( CMD_SEEK, packedArgs) ;
}

void FileJob::close()
{
    if (!m_open) return;

    m_slave->send( CMD_CLOSE );
    // ###  close?
}

KIO::filesize_t FileJob::size()
{
    if (!m_open) return 0;

    return m_size;
}

// Slave sends data
void FileJob::slotData( const QByteArray &_data)
{
    emit data( this, _data);
}

void FileJob::slotRedirection( const KUrl &url)
{
    kDebug(7007) << "FileJob::slotRedirection(" << url << ")" << endl;
    emit redirection(this, url);
}

void FileJob::slotMimetype( const QString& type )
{
    m_mimetype = type;
    emit mimetype( this, m_mimetype);
}

void FileJob::slotPosition( KIO::filesize_t pos )
{
    emit position( this, pos);
}

void FileJob::slotTotalSize( KIO::filesize_t t_size )
{
    m_size = t_size;
}

void FileJob::slotOpen( )
{
    m_open = true;
    emit open( this );
}

void FileJob::slotWritten( KIO::filesize_t t_written )
{
    emit written( this, t_written);
}

void FileJob::slotFinished()
{
    kDebug(7007) << "FileJob::slotFinished(" << this << ", " << m_url << ")" << endl;
    emit close( this );
    // Return slave to the scheduler
    slaveDone();
//     Scheduler::doJob(this);
}

void FileJob::start(Slave *slave)
{
    connect( slave, SIGNAL( data( const QByteArray & ) ),
             SLOT( slotData( const QByteArray & ) ) );

    connect( slave, SIGNAL( redirection(const KUrl &) ),
             SLOT( slotRedirection(const KUrl &) ) );

    connect( slave, SIGNAL(mimeType( const QString& ) ),
             SLOT( slotMimetype( const QString& ) ) );

    connect( slave, SIGNAL(open() ),
             SLOT( slotOpen() ) );

    connect( slave, SIGNAL(position(KIO::filesize_t) ),
             SLOT( slotPosition(KIO::filesize_t) ) );

    connect( slave, SIGNAL(written(KIO::filesize_t) ),
             SLOT( slotWritten(KIO::filesize_t) ) );

    connect( slave, SIGNAL(totalSize(KIO::filesize_t) ),
             SLOT( slotTotalSize(KIO::filesize_t) ) );

    SimpleJob::start(slave);

}

#include "filejob.moc"

