/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kfilterdev.h"
#include "kfilterbase.h"
#include <kdebug.h>
#include <stdio.h> // for EOF
#include <stdlib.h>
#include <assert.h>
#include <qfile.h>

class KFilterDev::KFilterDevPrivate
{
public:
    KFilterDevPrivate() : bNeedHeader(true), bSkipHeaders(false), autoDeleteFilterBase(false) {}
    bool bNeedHeader;
    bool bSkipHeaders;
    bool autoDeleteFilterBase;
    QByteArray buffer; // Used as 'input buffer' when reading, as 'output buffer' when writing
    QCString ungetchBuffer;
    QCString origFileName;
    KFilterBase::Result result;
};

KFilterDev::KFilterDev( KFilterBase * _filter, bool autoDeleteFilterBase )
    : filter(_filter)
{
    assert(filter);
    d = new KFilterDevPrivate;
    d->autoDeleteFilterBase = autoDeleteFilterBase;
}

KFilterDev::~KFilterDev()
{
    if ( d->autoDeleteFilterBase )
        delete filter;
    delete d;
}

#ifndef KDE_NO_COMPAT
//this one is static
// Cumbersome API. To be removed in KDE 3.0.
QIODevice* KFilterDev::createFilterDevice(KFilterBase* base, QFile* file)
{
   if (file==0)
      return 0;

   //we don't need a filter
   if (base==0)
       return new QFile(file->name()); // A bit strange IMHO. We ask for a QFile but we create another one !?! (DF)

   base->setDevice(file);
   return new KFilterDev(base);
};
#endif

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

QIODevice * KFilterDev::device( QIODevice* inDevice, const QString & mimetype)
{
    return device( inDevice, mimetype, true );
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

bool KFilterDev::open( int mode )
{
    //kdDebug(7005) << "KFilterDev::open " << mode << endl;
    if ( mode == IO_ReadOnly )
    {
        d->buffer.resize(0);
        d->ungetchBuffer.resize(0);
    }
    else
    {
        d->buffer.resize( 8*1024 );
        filter->setOutBuffer( d->buffer.data(), d->buffer.size() );
    }
    d->bNeedHeader = !d->bSkipHeaders;
    filter->init( mode );
    bool ret = !filter->device()->isOpen() ? filter->device()->open( mode ) : true;
    d->result = KFilterBase::OK;

    if ( !ret )
        kdWarning(7005) << "KFilterDev::open: Couldn't open underlying device" << endl;
    else
    {
        setState( IO_Open );
        setMode( mode );
    }
    ioIndex = 0;
    return ret;
}

void KFilterDev::close()
{
    //kdDebug(7005) << "KFilterDev::close" << endl;
    if ( filter->mode() == IO_WriteOnly )
        writeBlock( 0L, 0 ); // finish writing
    //kdDebug(7005) << "KFilterDev::close. Calling terminate()." << endl;

    filter->terminate();
    //kdDebug(7005) << "KFilterDev::close. Terminate() done. Closing device." << endl;
    filter->device()->close();
    setState( 0 ); // not IO_Open
}

void KFilterDev::flush()
{
    //kdDebug(7005) << "KFilterDev::flush" << endl;
    filter->device()->flush();
    // Hmm, might not be enough...
}

QIODevice::Offset KFilterDev::size() const
{
    // Well, hmm, Houston, we have a problem.
    // We can't know the size of the uncompressed data
    // before uncompressing it.......

    // But readAll, which is not virtual, needs the size.........

    kdWarning(7005) << "KFilterDev::size - can't be implemented !!!!!!!! Returning -1 " << endl;
    //abort();
    return (uint)-1;
}

QIODevice::Offset KFilterDev::at() const
{
    return ioIndex;
}

bool KFilterDev::at( QIODevice::Offset pos )
{
    Q_ASSERT ( filter->mode() == IO_ReadOnly );
    //kdDebug(7005) << "KFilterDev::at " << pos << "  currently at " << ioIndex << endl;

    if ( ioIndex == pos )
        return true;

    if ( pos == 0 )
    {
        ioIndex = 0;
        // We can forget about the cached data
        d->ungetchBuffer.resize(0);
        d->bNeedHeader = !d->bSkipHeaders;
        d->result = KFilterBase::OK;
        filter->setInBuffer(0L,0);
        filter->reset();
        return filter->device()->reset();
    }

    if ( ioIndex < pos ) // we can start from here
        pos = pos - ioIndex;
    else
    {
        // we have to start from 0 ! Ugly and slow, but better than the previous
        // solution (KTarGz was allocating everything into memory)
        if (!at(0)) // sets ioIndex to 0
            return false;
    }

    //kdDebug(7005) << "KFilterDev::at : reading " << pos << " dummy bytes" << endl;
    // #### Slow, and allocate a huge block of memory (potentially)
    // Maybe we could have a flag in the class to know we don't care about the
    // actual data
    QByteArray dummy( pos );
    return ( (QIODevice::Offset)readBlock( dummy.data(), pos ) == pos ) ;
}

bool KFilterDev::atEnd() const
{
    return filter->device()->atEnd() && (d->result == KFilterBase::END);
}

Q_LONG KFilterDev::readBlock( char *data, Q_ULONG maxlen )
{
    Q_ASSERT ( filter->mode() == IO_ReadOnly );
    //kdDebug(7005) << "KFilterDev::readBlock maxlen=" << maxlen << endl;
    // If we had an error, or came to the end of the stream, return 0.
    if ( d->result != KFilterBase::OK )
        return -1;

    filter->setOutBuffer( data, maxlen );

    bool readEverything = false;
    uint dataReceived = 0;
    uint availOut = maxlen;
    while ( dataReceived < maxlen )
    {
        if (filter->inBufferEmpty())
        {
            // Not sure about the best size to set there.
            // For sure, it should be bigger than the header size (see comment in readHeader)
            d->buffer.resize( 8*1024 );
            // Request data from underlying device
            int size = filter->device()->readBlock( d->buffer.data(),
                                                    d->buffer.size() );
            if ( size )
                filter->setInBuffer( d->buffer.data(), size );
            else
                readEverything = true;
            //kdDebug(7005) << "KFilterDev::readBlock got " << size << " bytes from device" << endl;
        }
        if (d->bNeedHeader)
        {
            (void) filter->readHeader();
            d->bNeedHeader = false;
        }

        d->result = filter->uncompress();

        if (d->result == KFilterBase::ERROR)
        {
            kdWarning(7005) << "KFilterDev: Error when uncompressing data" << endl;
            break;
        }

        // We got that much data since the last time we went here
        uint outReceived = availOut - filter->outBufferAvailable();
        //kdDebug(7005) << "avail_out = " << filter->outBufferAvailable() << " result=" << d->result << " outReceived=" << outReceived << endl;
        if( availOut < (uint)filter->outBufferAvailable() )
            kdWarning(7005) << " last availOut " << availOut << " smaller than new avail_out=" << filter->outBufferAvailable() << " !" << endl;

        // Move on in the output buffer
        data += outReceived;
        dataReceived += outReceived;
        ioIndex += outReceived;
        if (d->result == KFilterBase::END)
        {
            //kdDebug(7005) << "KFilterDev::readBlock got END. dataReceived=" << dataReceived << endl;
            break; // Finished.
        }
        if (readEverything && filter->inBufferEmpty() )
        {
            // We decoded everything there was to decode. So -> done.
            //kdDebug(7005) << "Seems we're done. dataReceived=" << dataReceived << endl;
            d->result = KFilterBase::END;
            break;
        }
        availOut = maxlen - dataReceived;
        filter->setOutBuffer( data, availOut );
    }

    return dataReceived;
}

Q_LONG KFilterDev::writeBlock( const char *data /*0 to finish*/, Q_ULONG len )
{
    Q_ASSERT ( filter->mode() == IO_WriteOnly );
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
            kdWarning(7005) << "KFilterDev: Error when compressing data" << endl;
            // What to do ?
            break;
        }

        // Wrote everything ?
        if (filter->inBufferEmpty() || (d->result == KFilterBase::END))
        {
            // We got that much data since the last time we went here
            uint wrote = availIn - filter->inBufferAvailable();

            //kdDebug(7005) << " Wrote everything for now. avail_in = " << filter->inBufferAvailable() << " result=" << d->result << " wrote=" << wrote << endl;

            // Move on in the input buffer
            data += wrote;
            dataWritten += wrote;
            ioIndex += wrote;

            availIn = len - dataWritten;
            //kdDebug(7005) << " KFilterDev::writeBlock availIn=" << availIn << " dataWritten=" << dataWritten << " ioIndex=" << ioIndex << endl;
            if ( availIn > 0 ) // Not sure this will ever happen
                filter->setInBuffer( data, availIn );
        }

        if (filter->outBufferFull() || (d->result == KFilterBase::END))
        {
            //kdDebug(7005) << " KFilterDev::writeBlock writing to underlying. avail_out=" << filter->outBufferAvailable() << endl;
            int towrite = d->buffer.size() - filter->outBufferAvailable();
            if ( towrite > 0 )
            {
                // Write compressed data to underlying device
                int size = filter->device()->writeBlock( d->buffer.data(), towrite );
                if ( size != towrite )
                    kdWarning(7005) << "KFilterDev::writeBlock. Could only write " << size << " out of " << towrite << " bytes" << endl;
                //else
                    //kdDebug(7005) << " KFilterDev::writeBlock wrote " << size << " bytes" << endl;
            }
            d->buffer.resize( 8*1024 );
            filter->setOutBuffer( d->buffer.data(), d->buffer.size() );
            if (d->result == KFilterBase::END)
            {
                //kdDebug(7005) << " KFilterDev::writeBlock END" << endl;
                Q_ASSERT(finish); // hopefully we don't get end before finishing
                break;
            }
        }
    }

    return dataWritten;
}

int KFilterDev::getch()
{
    Q_ASSERT ( filter->mode() == IO_ReadOnly );
    //kdDebug(7005) << "KFilterDev::getch" << endl;
    if ( !d->ungetchBuffer.isEmpty() ) {
        int len = d->ungetchBuffer.length();
        int ch = d->ungetchBuffer[ len-1 ];
        d->ungetchBuffer.truncate( len - 1 );
        //kdDebug(7005) << "KFilterDev::getch from ungetch: " << QString(QChar(ch)) << endl;
        return ch;
    }
    char buf[1];
    int ret = readBlock( buf, 1 ) == 1 ? buf[0] : EOF;
    //kdDebug(7005) << "KFilterDev::getch ret=" << QString(QChar(ret)) << endl;
    return ret;
}

int KFilterDev::putch( int c )
{
    //kdDebug(7005) << "KFilterDev::putch" << endl;
    char buf[1];
    buf[0] = c;
    return writeBlock( buf, 1 ) == 1 ? c : -1;
}

int KFilterDev::ungetch( int ch )
{
    //kdDebug(7005) << "KFilterDev::ungetch " << QString(QChar(ch)) << endl;
    if ( ch == EOF )                            // cannot unget EOF
        return ch;

    // pipe or similar => we cannot ungetch, so do it manually
    d->ungetchBuffer +=ch;
    return ch;
}

void KFilterDev::setOrigFileName( const QCString & fileName )
{
    d->origFileName = fileName;
}

void KFilterDev::setSkipHeaders()
{
    d->bSkipHeaders = true;
}
