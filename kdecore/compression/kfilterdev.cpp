/* This file is part of the KDE libraries
   Copyright (C) 2000, 2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kfilterdev.h"
#include "kfilterbase.h"
#include <kdebug.h>
#include <stdio.h> // for EOF
#include <stdlib.h>
#include <assert.h>
#include <qfile.h>

#define BUFFER_SIZE 8*1024

class KFilterDev::KFilterDevPrivate
{
public:
    KFilterDevPrivate() : bNeedHeader(true), bSkipHeaders(false),
                          autoDeleteFilterBase(false), bOpenedUnderlyingDevice(false),
                          bIgnoreData(false){}
    bool bNeedHeader;
    bool bSkipHeaders;
    bool autoDeleteFilterBase;
    bool bOpenedUnderlyingDevice;
    bool bIgnoreData;
    QByteArray buffer; // Used as 'input buffer' when reading, as 'output buffer' when writing
    QByteArray origFileName;
    KFilterBase::Result result;
    KFilterBase *filter;
};

KFilterDev::KFilterDev( KFilterBase * _filter, bool autoDeleteFilterBase )
    : d(new KFilterDevPrivate)
{
    assert(_filter);
    d->filter = _filter;
    d->autoDeleteFilterBase = autoDeleteFilterBase;
}

KFilterDev::~KFilterDev()
{
    if ( isOpen() )
        close();
    if ( d->autoDeleteFilterBase )
        delete d->filter;
    delete d;
}

//static
QIODevice * KFilterDev::deviceForFile( const QString & fileName, const QString & mimetype,
                                       bool forceFilter )
{
    QFile * f = new QFile( fileName );
    KFilterBase * base = mimetype.isEmpty() ? KFilterBase::findFilterByFileName( fileName )
                         : KFilterBase::findFilterByMimeType( mimetype );
    if ( base )
    {
        base->setDevice(f, true);
        return new KFilterDev(base, true);
    }
    if(!forceFilter)
        return f;
    else
    {
        delete f;
        return 0L;
    }
}

QIODevice * KFilterDev::device( QIODevice* inDevice, const QString & mimetype, bool autoDeleteInDevice )
{
   if (inDevice==0)
      return 0;
   KFilterBase * base = KFilterBase::findFilterByMimeType(mimetype);
   if ( base )
   {
      base->setDevice(inDevice, autoDeleteInDevice);
      return new KFilterDev(base, true /* auto-delete "base" */);
   }
   return 0;
}

bool KFilterDev::open( QIODevice::OpenMode mode )
{
    //kDebug(7005) << "KFilterDev::open " << mode << endl;
    if ( mode == QIODevice::ReadOnly )
    {
        d->buffer.resize(0);
    }
    else
    {
        d->buffer.resize( BUFFER_SIZE );
        d->filter->setOutBuffer( d->buffer.data(), d->buffer.size() );
    }
    d->bNeedHeader = !d->bSkipHeaders;
    d->filter->init( mode );
    d->bOpenedUnderlyingDevice = !d->filter->device()->isOpen();
    bool ret = d->bOpenedUnderlyingDevice ? d->filter->device()->open( mode ) : true;
    d->result = KFilterBase::OK;

    if ( !ret )
        kWarning(7005) << "KFilterDev::open: Couldn't open underlying device" << endl;
    else
        setOpenMode( mode );

    return ret;
}

void KFilterDev::close()
{
    if ( !isOpen() )
        return;
    //kDebug(7005) << "KFilterDev::close" << endl;
    if ( d->filter->mode() == QIODevice::WriteOnly )
        write( 0L, 0 ); // finish writing
    //kDebug(7005) << "KFilterDev::close. Calling terminate()." << endl;

    d->filter->terminate();
    if ( d->bOpenedUnderlyingDevice )
        d->filter->device()->close();
    setOpenMode( QIODevice::NotOpen );
}

bool KFilterDev::seek( qint64 pos )
{
    qint64 ioIndex = this->pos(); // current position
    if ( ioIndex == pos )
        return true;

    kDebug(7005) << "KFilterDev::seek(" << pos << ") called" << endl;

    Q_ASSERT ( d->filter->mode() == QIODevice::ReadOnly );

    if ( pos == 0 )
    {
        // We can forget about the cached data
        d->bNeedHeader = !d->bSkipHeaders;
        d->result = KFilterBase::OK;
        d->filter->setInBuffer(0L,0);
        d->filter->reset();
        QIODevice::seek(pos);
        return d->filter->device()->reset();
    }

    if ( ioIndex > pos ) // we can start from here
        pos = pos - ioIndex;
    else
    {
        // we have to start from 0 ! Ugly and slow, but better than the previous
        // solution (KTarGz was allocating everything into memory)
        if (!seek(0)) // recursive
            return false;
    }

    //kDebug(7005) << "KFilterDev::at : reading " << pos << " dummy bytes" << endl;
    QByteArray dummy( qMin( pos, (qint64)3*BUFFER_SIZE ), 0 );
    d->bIgnoreData = true;
    bool result = ( read( dummy.data(), pos ) == pos );
    d->bIgnoreData = false;
    QIODevice::seek(pos);
    return result;
}

bool KFilterDev::atEnd() const
{
    return (d->result == KFilterBase::END)
        && QIODevice::atEnd() // take QIODevice's internal buffer into account
        && d->filter->device()->atEnd();
}

qint64 KFilterDev::readData( char *data, qint64 maxlen )
{
    Q_ASSERT ( d->filter->mode() == QIODevice::ReadOnly );
    //kDebug(7005) << "KFilterDev::read maxlen=" << maxlen << endl;
    KFilterBase* filter = d->filter;

    uint dataReceived = 0;

    // We came to the end of the stream
    if ( d->result == KFilterBase::END )
        return dataReceived;

    // If we had an error, return -1.
    if ( d->result != KFilterBase::OK )
        return -1;


    qint64 outBufferSize;
    if ( d->bIgnoreData )
    {
        outBufferSize = qMin( maxlen, (qint64)3*BUFFER_SIZE );
    }
    else
    {
        outBufferSize = maxlen;
    }
    outBufferSize -= dataReceived;
    qint64 availOut = outBufferSize;
    filter->setOutBuffer( data, outBufferSize );

    bool decompressedAll = false;
    while ( dataReceived < maxlen )
    {
        if (filter->inBufferEmpty())
        {
            // Not sure about the best size to set there.
            // For sure, it should be bigger than the header size (see comment in readHeader)
            d->buffer.resize( BUFFER_SIZE );
            // Request data from underlying device
            int size = filter->device()->read( d->buffer.data(),
                                               d->buffer.size() );
            if ( size )
                filter->setInBuffer( d->buffer.data(), size );
            else {
                if ( decompressedAll )
                {
                    // We decoded everything there was to decode. So -> done.
                    //kDebug(7005) << "Seems we're done. dataReceived=" << dataReceived << endl;
                    d->result = KFilterBase::END;
                    break;
                }
            }
            //kDebug(7005) << "KFilterDev::read got " << size << " bytes from device" << endl;
        }
        if (d->bNeedHeader)
        {
            (void) filter->readHeader();
            d->bNeedHeader = false;
        }

        d->result = filter->uncompress();

        if (d->result == KFilterBase::ERROR)
        {
            kWarning(7005) << "KFilterDev: Error when uncompressing data" << endl;
            break;
        }

        // We got that much data since the last time we went here
        uint outReceived = availOut - filter->outBufferAvailable();
        //kDebug(7005) << "avail_out = " << filter->outBufferAvailable() << " result=" << d->result << " outReceived=" << outReceived << endl;
        if( availOut < (uint)filter->outBufferAvailable() )
            kWarning(7005) << " last availOut " << availOut << " smaller than new avail_out=" << filter->outBufferAvailable() << " !" << endl;

        dataReceived += outReceived;
        if ( !d->bIgnoreData )  // Move on in the output buffer
        {
            data += outReceived;
            availOut = maxlen - dataReceived;
        }
        else if ( maxlen - dataReceived < outBufferSize )
        {
            availOut = maxlen - dataReceived;
        }
        if (d->result == KFilterBase::END)
        {
            //kDebug(7005) << "KFilterDev::read got END. dataReceived=" << dataReceived << endl;
            break; // Finished.
        }
        if (filter->inBufferEmpty() && filter->outBufferAvailable() != 0 )
        {
            decompressedAll = true;
        }
        filter->setOutBuffer( data, availOut );
    }

    return dataReceived;
}

qint64 KFilterDev::writeData( const char *data /*0 to finish*/, qint64 len )
{
    KFilterBase* filter = d->filter;
    Q_ASSERT ( filter->mode() == QIODevice::WriteOnly );
    // If we had an error, return 0.
    if ( d->result != KFilterBase::OK )
        return 0;

    bool finish = (data == 0L);
    if (!finish)
    {
        filter->setInBuffer( data, len );
        if (d->bNeedHeader)
        {
            (void)filter->writeHeader( d->origFileName );
            d->bNeedHeader = false;
        }
    }

    uint dataWritten = 0;
    uint availIn = len;
    while ( dataWritten < len || finish )
    {

        d->result = filter->compress( finish );

        if (d->result == KFilterBase::ERROR)
        {
            kWarning(7005) << "KFilterDev: Error when compressing data" << endl;
            // What to do ?
            break;
        }

        // Wrote everything ?
        if (filter->inBufferEmpty() || (d->result == KFilterBase::END))
        {
            // We got that much data since the last time we went here
            uint wrote = availIn - filter->inBufferAvailable();

            //kDebug(7005) << " Wrote everything for now. avail_in = " << filter->inBufferAvailable() << " result=" << d->result << " wrote=" << wrote << endl;

            // Move on in the input buffer
            data += wrote;
            dataWritten += wrote;

            availIn = len - dataWritten;
            //kDebug(7005) << " KFilterDev::write availIn=" << availIn << " dataWritten=" << dataWritten << " ioIndex=" << pos() << endl;
            if ( availIn > 0 ) // Not sure this will ever happen
                filter->setInBuffer( data, availIn );
        }

        if (filter->outBufferFull() || (d->result == KFilterBase::END))
        {
            //kDebug(7005) << " KFilterDev::write writing to underlying. avail_out=" << filter->outBufferAvailable() << endl;
            int towrite = d->buffer.size() - filter->outBufferAvailable();
            if ( towrite > 0 )
            {
                // Write compressed data to underlying device
                int size = filter->device()->write( d->buffer.data(), towrite );
                if ( size != towrite ) {
                    kWarning(7005) << "KFilterDev::write. Could only write " << size << " out of " << towrite << " bytes" << endl;
                    return 0; // indicate an error (happens on disk full)
                }
                //else
                    //kDebug(7005) << " KFilterDev::write wrote " << size << " bytes" << endl;
            }
            d->buffer.resize( 8*1024 );
            filter->setOutBuffer( d->buffer.data(), d->buffer.size() );
            if (d->result == KFilterBase::END)
            {
                //kDebug(7005) << " KFilterDev::write END" << endl;
                Q_ASSERT(finish); // hopefully we don't get end before finishing
                break;
            }
        }
    }

    return dataWritten;
}

void KFilterDev::setOrigFileName( const QByteArray & fileName )
{
    d->origFileName = fileName;
}

void KFilterDev::setSkipHeaders()
{
    d->bSkipHeaders = true;
}
