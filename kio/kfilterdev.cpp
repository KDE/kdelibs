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
    bool bNeedHeader;
    QByteArray inputBuffer;
    QCString ungetchBuffer;
    KFilterBase::Result result;

    //QByteArray outputBuffer;
};

KFilterDev::KFilterDev( KFilterBase * _filter )
    : filter(_filter)
{
    assert(filter);
    d = new KFilterDevPrivate;
    // Some setFlags calls are probably missing here,
    // for proper results of the state methods,
    // but the Qt doc says "internal" ??.
}

KFilterDev::~KFilterDev()
{
    delete d;
}

//this one is static
//we can make neither base nor file const, due to base->setDevice()
QIODevice* KFilterDev::createFilterDevice(KFilterBase* base, QFile* file)
{
   if (file==0)
      return 0;

   //we don't need a filter
   if (base==0)
      return new QFile(file->name());

   base->setDevice(file);
   return new KFilterDev(base);
};

bool KFilterDev::open( int mode )
{
    kdDebug() << "KFilterDev::open " << mode << endl;
    ASSERT( mode == IO_ReadOnly ); // for now
    filter->init();
    bool ret = filter->device()->open( mode );
    d->ungetchBuffer.resize(0);
    d->bNeedHeader = true;
    d->result = KFilterBase::OK;

    if ( !ret )
        kdWarning() << "Couldn't open underlying device" << endl;
    ioIndex = 0;
    return ret;
}

void KFilterDev::close()
{
    kdDebug() << "KFilterDev::close" << endl;

    filter->device()->close();
    filter->terminate();
}

void KFilterDev::flush()
{
    kdDebug() << "KFilterDev::flush" << endl;
    filter->device()->flush();
}

uint KFilterDev::size() const
{
    // Well, hmm, Houston, we have a problem.
    // We can't know the size of the uncompressed data
    // before uncompressing it.......

    // But readAll, which is not virtual, needs the size.........

    kdWarning() << "KFilterDev::size - can't be implemented !!!!!!!! Returning -1 " << endl;
    //abort();
    return (uint)-1;
}

int KFilterDev::at() const
{
    return ioIndex;
}

bool KFilterDev::at( int pos )
{
    kdDebug() << "KFilterDev::at " << pos << endl;

    if ( ioIndex == pos )
        return true;

    if ( pos == 0 )
    {
        ioIndex = 0;
        // We can forget about the cached data
        d->ungetchBuffer.resize(0);
        d->bNeedHeader = true;
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

    kdDebug() << "KFilterDev::at : reading " << pos << " dummy bytes" << endl;
    // #### Slow, and allocate a huge block of memory (potentially)
    // Maybe we could have a flag in the class to know we don't care about the
    // actual data
    QByteArray dummy( pos );
    return ( readBlock( dummy.data(), pos ) == pos ) ;
}

bool KFilterDev::atEnd() const
{
    return filter->device()->atEnd() && (d->result == KFilterBase::END);
}

int KFilterDev::readBlock( char *data, uint maxlen )
{
    // If we had an error, or came to the end of the stream, return 0.
    if ( d->result != KFilterBase::OK )
        return 0;

    filter->setOutBuffer( data, maxlen );

    uint dataReceived = 0;
    uint availOut = maxlen;
    while ( dataReceived < maxlen )
    {
        if (filter->inBufferEmpty())
        {
            // Not sure about the best size to set there.
            // For sure, it should be bigger than the header size (see comment in readHeader)
            d->inputBuffer.resize( 8*1024 );
            // Request data from underlying device
            int size = filter->device()->readBlock( d->inputBuffer.data(),
                                                    d->inputBuffer.size() );
            filter->setInBuffer( d->inputBuffer.data(), size );
            kdDebug() << "KFilterDev::readBlock got " << size << " bytes from device" << endl;
        }
        if (d->bNeedHeader)
        {
            (void) filter->readHeader();
            d->bNeedHeader = false;
        }

        if ( filter->inBufferEmpty() )
            d->result = KFilterBase::END;
        else
            d->result = filter->uncompress();

        if (d->result == KFilterBase::ERROR)
        {
            kdDebug() << "KFilterDev: Error when uncompressing data" << endl;
            // What to do ?
            break;
        }

        // No more space in output buffer, or finished ?
        if ((filter->outBufferFull()) || (d->result == KFilterBase::END))
        {
            // We got that much data since the last time we went here
            uint outReceived = availOut - filter->outBufferAvailable();

            //kdDebug() << "avail_out = " << filter->outBufferAvailable() << " result=" << result << " outReceived=" << outReceived << endl;

            // Move on in the output buffer
            data += outReceived;
            dataReceived += outReceived;
            ioIndex += outReceived;
            if (d->result == KFilterBase::END)
            {
                kdDebug() << "KFilterDev::readBlock got END. dataReceived=" << dataReceived << endl;
                break; // Finished.
            }
            availOut = maxlen - dataReceived;
            filter->setOutBuffer( data, availOut );
        }
    }

    return dataReceived;
}
int KFilterDev::writeBlock( const char *data, uint len )
{
    // not implemented
    (void) data;
    return len - len;
}

int KFilterDev::getch()
{
    //kdDebug() << "KFilterDev::getch" << endl;
    if ( !d->ungetchBuffer.isEmpty() ) {
        int len = d->ungetchBuffer.length();
        int ch = d->ungetchBuffer[ len-1 ];
        d->ungetchBuffer.truncate( len - 1 );
        //kdDebug() << "KFilterDev::getch from ungetch: " << QString(QChar(ch)) << endl;
        return ch;
    }
    char buf[1];
    int ret = readBlock( buf, 1 ) == 1 ? buf[0] : EOF;
    //kdDebug() << "KFilterDev::getch ret=" << QString(QChar(ret)) << endl;
    return ret;
}

int KFilterDev::putch( int )
{
    kdDebug() << "KFilterDev::putch" << endl;
    return -1;
}

int KFilterDev::ungetch( int ch )
{
    //kdDebug() << "KFilterDev::ungetch " << QString(QChar(ch)) << endl;
    if ( ch == EOF )                            // cannot unget EOF
        return ch;

    // pipe or similar => we cannot ungetch, so do it manually
    d->ungetchBuffer +=ch;
    return ch;
}

