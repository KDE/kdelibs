/* This file is part of the KDE libraries
   Copyright (C) 2001, 2002 David Faure <faure@kde.org>

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

#ifndef klimitediodevice_h
#define klimitediodevice_h

#include <kdebug.h>
#include <qiodevice.h>
/**
 * A readonly device that reads from an underlying device
 * from a given point to another (e.g. to give access to a single
 * file inside an archive).
 * @author David Faure <faure@kde.org>
 * @since 3.1
 */
class KIO_EXPORT KLimitedIODevice : public QIODevice
{
public:
    /**
     * Creates a new KLimitedIODevice.
     * @param dev the underlying device, opened or not
     * This device itself auto-opens (in readonly mode), no need to open it.
     * @param start where to start reading (position in bytes)
     * @param length the length of the data to read (in bytes)
     */
    KLimitedIODevice( QIODevice *dev, int start, int length )
        : m_dev( dev ), m_start( start ), m_length( length )
    {
        //kDebug(7005) << "KLimitedIODevice::KLimitedIODevice start=" << start << " length=" << length << endl;
        open( QIODevice::ReadOnly );
    }
    virtual ~KLimitedIODevice() {}

    virtual bool open( QIODevice::OpenMode m ) {
        //kDebug(7005) << "KLimitedIODevice::open m=" << m << endl;
        if ( m & QIODevice::ReadOnly ) {
            /*bool ok = false;
            if ( m_dev->isOpen() )
                ok = ( m_dev->mode() == QIODevice::ReadOnly );
            else
                ok = m_dev->open( m );
            if ( ok )*/
                m_dev->seek( m_start ); // No concurrent access !
        }
        else
            kWarning(7005) << "KLimitedIODevice::open only supports QIODevice::ReadOnly!" << endl;
        setOpenMode( QIODevice::ReadOnly );
        return true;
    }
    virtual void close() {}

    virtual qint64 size() const { return m_length; }

    virtual qint64 readData ( char * data, qint64 maxlen )
    {
        maxlen = QMIN( maxlen, m_length - pos() ); // Apply upper limit
        return m_dev->read( data, maxlen );
    }
    virtual qint64 writeData ( const char *, qint64 ) { return -1; } // unsupported
    virtual int putChar( int ) { return -1; } // unsupported

    virtual int getChar() {
        char c[2];
        if ( readData(c, 1) == -1)
            return -1;
        else
            return c[0];
    }
    virtual void ungetChar( int c ) { m_dev->ungetChar(c); } // ## apply lower limit ?
    virtual qint64 pos() const { return m_dev->pos() - m_start; }
    virtual bool seek( qint64 pos ) {
        Q_ASSERT( pos <= m_length );
        pos = QMIN( pos, m_length ); // Apply upper limit
        return m_dev->at( m_start + pos );
    }
    virtual bool atEnd() const { return m_dev->at() >= m_start + m_length; }
private:
    QIODevice* m_dev;
    qint64 m_start;
    qint64 m_length;
};

#endif
