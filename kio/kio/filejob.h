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

#ifndef __kio_filejob_h
#define __kio_filejob_h

#include <kurl.h>
#include <kio/jobclasses.h>

namespace KIO {

/**
 *  The file-job is an asynchronious version of normal file handling.
 *  It allows block-wise reading and writing, and allows seeking. Results are returned through signals.
 *
 *  Should always be created using KIO::open(KUrl&)
 */

class KIO_EXPORT FileJob : public SimpleJob
{
Q_OBJECT

public:
    /**
     * @internal
     */
    FileJob(const KUrl& url, const QByteArray &packedArgs);
    ~FileJob();

    /**
     * Read block
     *
     * The slave emits the data through data().
     * @param size the requested amount of data
     */
    void read( int size );

    /**
     * Write block
     *
     * @param data the data to write
     */
    void write( const QByteArray &data );

    /**
     * Close
     *
     * Closes the file-slave
     */
    void close();

    /**
     * Seek
     *
     * The slave emits position()
     * @param offset the position from start to go to
     */
    void seek( int offset );

    /**
     * Size
     *
     * @return the file size
     */
    KIO::filesize_t size();

    /**
     * @internal
     * Called by the scheduler when a @p slave gets to
     * work on this job.
     * @param slave the slave that starts working on this job
     */
    virtual void start(Slave *slave);

signals:
    /**
     * Data from the slave has arrived.
     * @param job the job that emitted this signal
     * @param data data received from the slave.
     */
    void data( KIO::Job *job, const QByteArray &data );

    /**
     * Signals the file is a redirection.
     * Follow this url manually to reach data
     * @param job the job that emitted this signal
     * @param url the new URL
     */
    void redirection( KIO::Job *job, const KUrl &url );

    /**
     * Mimetype determined.
     * @param job the job that emitted this signal
     * @param type the mime type
     */
    void mimetype( KIO::Job *job, const QString &type );

    /**
     * File is open, metadata has been determined and the
     * file-slave is ready to receive commands.
     * @param job the job that emitted this signal
     */
    void open(KIO::Job *job);

    /**
     * File is closed and will accept no more commands
     * @param job the job that emitted this signal
     */
    void close(KIO::Job *job);

    /**
     * The file has reached this position. Emitted after seek.
     * @param job the job that emitted this signal
     * @param offeset the new position
     */
    void position( KIO::Job *job, KIO::filesize_t offset);

private slots:
    virtual void slotRedirection( const KUrl &url);
    virtual void slotData( const QByteArray &data);
    virtual void slotMimetype( const QString &mimetype );
    virtual void slotOpen( );
    virtual void slotFinished( );
    virtual void slotPosition( KIO::filesize_t );
    virtual void slotTotalSize( KIO::filesize_t );

protected:
    bool m_open;
    QString m_mimetype;
    KIO::filesize_t m_size;
};

} // namespace

#endif
