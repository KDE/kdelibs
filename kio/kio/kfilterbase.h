/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kfilterbase__h
#define __kfilterbase__h

#include <qobject.h>
#include <qstring.h>
class QIODevice;

/**
 * This is the base class for compression filters
 * such as gzip and bzip2. It's pretty much internal.
 * Don't use directly, use KFilterDev instead.
 */
class KFilterBase : public QObject // needs to inherit QObject for KLibFactory::create
{
    Q_OBJECT
public:
    KFilterBase();
    virtual ~KFilterBase();

    /**
     * Sets the device on which the filter will work
     * If @p autodelete is set to true, @p dev is deleted when the filter is deleted
     */
    void setDevice( QIODevice * dev, bool autodelete = false );
    // Note that this isn't in the constructor, because of KLibFactory::create,
    // but it should be called before using the filterbase !

    QIODevice * device() { return m_dev; }
    virtual void init( int mode ) = 0;
    virtual int mode() const = 0;
    virtual void terminate() {}
    virtual void reset() {}
    virtual bool readHeader() = 0;
    virtual bool writeHeader( const QCString & filename ) = 0;
    virtual void setOutBuffer( char * data, uint maxlen ) = 0;
    virtual void setInBuffer( const char * data, uint size ) = 0;
    virtual bool inBufferEmpty() const { return inBufferAvailable() == 0; }
    virtual int  inBufferAvailable() const = 0;
    virtual bool outBufferFull() const { return outBufferAvailable() == 0; }
    virtual int  outBufferAvailable() const = 0;

    enum Result { OK, END, ERROR };
    virtual Result uncompress() = 0;
    virtual Result compress( bool finish ) = 0;

    /**
     * Call this to create the appropriate filter for the file
     * named @p fileName.
     */
    static KFilterBase * findFilterByFileName( const QString & fileName );

    /**
     * Call this to create the appropriate filter for the mimetype
     * @p mimeType. For instance application/x-gzip.
     */
    static KFilterBase * findFilterByMimeType( const QString & mimeType );

protected:
    QIODevice * m_dev;
    bool m_bAutoDel;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFilterBasePrivate;
};

#endif
