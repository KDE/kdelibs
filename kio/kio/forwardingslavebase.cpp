/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

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

#include <kdebug.h>
#include <kio/job.h>
#include <kmimetype.h>
#include <kprotocolinfo.h>

#include <qapplication.h>
#include <qeventloop.h>

#include "forwardingslavebase.h"

namespace KIO
{

class ForwardingSlaveBasePrivate
{
};

ForwardingSlaveBase::ForwardingSlaveBase(const Q3CString &protocol,
                                         const Q3CString &poolSocket,
                                         const Q3CString &appSocket)
    : QObject(), SlaveBase(protocol, poolSocket, appSocket), eventLoop(this)
{
}

ForwardingSlaveBase::~ForwardingSlaveBase()
{
}

bool ForwardingSlaveBase::internalRewriteURL(const KURL &url, KURL &newURL)
{
    bool result = true;

    if ( url.protocol().ascii()==mProtocol )
    {
        result = rewriteURL(url, newURL);
    }
    else
    {
        newURL = url;
    }

    m_processedURL = newURL;
    m_requestedURL = url;
    return result;
}

void ForwardingSlaveBase::prepareUDSEntry(KIO::UDSEntry &entry,
                                          bool listing) const
{
    kdDebug() << "ForwardingSlaveBase::prepareUDSEntry: listing=="
              << listing << endl;

    const QString name = entry.stringValue( KIO::UDS_NAME );
    QString mimetype = entry.stringValue( KIO::UDS_MIME_TYPE );
    KURL url;
    const QString urlStr = entry.stringValue( KIO::UDS_URL );
    const bool url_found = !urlStr.isEmpty();
    if ( url_found )
    {
        url = urlStr;
        KURL new_url = m_requestedURL;
        if (listing)
            new_url.addPath(url.fileName());
        // ## Didn't find a way to use an iterator instead of re-doing a key lookup
        entry.insert( KIO::UDS_URL, new_url.url() );
        kdDebug() << "URL = " << url << endl;
        kdDebug() << "New URL = " << urlStr << endl;
    }

    if (mimetype.isEmpty())
    {
        KURL new_url = m_processedURL;
        if (url_found && listing)
        {
            new_url.addPath( url.fileName() );
        }
        else if (listing)
        {
            new_url.addPath( name );
        }

        mimetype = KMimeType::findByURL(new_url)->name();

        entry.insert( KIO::UDS_MIME_TYPE, mimetype );

        kdDebug() << "New Mimetype = " << mimetype << endl;
    }

    if ( m_processedURL.isLocalFile() )
    {
        KURL new_url = m_processedURL;
        if (listing)
        {
            new_url.addPath( name );
        }

        entry.insert( KIO::UDS_LOCAL_PATH, new_url.path() );
    }
}

void ForwardingSlaveBase::get(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::get: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::TransferJob *job = KIO::get(new_url, false, false);
        connectTransferJob(job);

        eventLoop.exec();
    }
}

void ForwardingSlaveBase::put(const KURL &url, int permissions,
                              bool overwrite, bool resume )
{
    kdDebug() << "ForwardingSlaveBase::put: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::TransferJob *job = KIO::put(new_url, permissions, overwrite,
                                         resume, false);
        connectTransferJob(job);

        eventLoop.exec();
    }
}

void ForwardingSlaveBase::stat(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::stat: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::SimpleJob *job = KIO::stat(new_url, false);
        connectSimpleJob(job);

        eventLoop.exec();
    }
}

void ForwardingSlaveBase::mimetype(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::mimetype: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::TransferJob *job = KIO::mimetype(new_url, false);
        connectTransferJob(job);

        eventLoop.exec();
    }
}

void ForwardingSlaveBase::listDir(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::listDir: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::ListJob *job = KIO::listDir(new_url, false);
        connectListJob(job);

        eventLoop.exec();
    }
}

void ForwardingSlaveBase::mkdir(const KURL &url, int permissions)
{
    kdDebug() << "ForwardingSlaveBase::mkdir: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::SimpleJob *job = KIO::mkdir(new_url, permissions);
        connectSimpleJob(job);

        eventLoop.exec();
    }
}

void ForwardingSlaveBase::rename(const KURL &src, const KURL &dest,
                                 bool overwrite)
{
    kdDebug() << "ForwardingSlaveBase::rename: " << src << ", " << dest << endl;

    KURL new_src, new_dest;
    if ( internalRewriteURL(src, new_src) && internalRewriteURL(dest, new_dest) )
    {
        KIO::Job *job = KIO::rename(new_src, new_dest, overwrite);
        connectJob(job);

        eventLoop.exec();
    }
}

void ForwardingSlaveBase::symlink(const QString &target, const KURL &dest,
                                  bool overwrite)
{
    kdDebug() << "ForwardingSlaveBase::symlink: " << target << ", " << dest << endl;

    KURL new_dest;
    if ( internalRewriteURL(dest, new_dest) )
    {
        KIO::SimpleJob *job = KIO::symlink(target, new_dest, overwrite, false);
        connectSimpleJob(job);

        eventLoop.exec();
    }
}

void ForwardingSlaveBase::chmod(const KURL &url, int permissions)
{
    kdDebug() << "ForwardingSlaveBase::chmod: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        KIO::SimpleJob *job = KIO::chmod(new_url, permissions);
        connectSimpleJob(job);

        eventLoop.exec();
    }
}

void ForwardingSlaveBase::copy(const KURL &src, const KURL &dest,
                               int permissions, bool overwrite)
{
    kdDebug() << "ForwardingSlaveBase::copy: " << src << ", " << dest << endl;

    KURL new_src, new_dest;
    if ( internalRewriteURL(src, new_src) && internalRewriteURL(dest, new_dest) )
    {
        KIO::Job *job = KIO::file_copy(new_src, new_dest, permissions,
                                       overwrite, false);
        connectJob(job);

        eventLoop.exec();
    }
}

void ForwardingSlaveBase::del(const KURL &url, bool isfile)
{
    kdDebug() << "ForwardingSlaveBase::del: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        if (isfile)
        {
            KIO::DeleteJob *job = KIO::del(new_url, false, false);
            connectJob(job);
        }
        else
        {
            KIO::SimpleJob *job = KIO::rmdir(new_url);
            connectSimpleJob(job);
        }

        eventLoop.exec();
    }
}


//////////////////////////////////////////////////////////////////////////////

void ForwardingSlaveBase::connectJob(KIO::Job *job)
{
    // We will forward the warning message, no need to let the job
    // display it itself
    job->setInteractive(false);

    connect( job, SIGNAL( result(KIO::Job *) ),
             this, SLOT( slotResult(KIO::Job *) ) );
    connect( job, SIGNAL( warning(KIO::Job *, const QString &) ),
             this, SLOT( slotWarning(KIO::Job *, const QString &) ) );
    connect( job, SIGNAL( infoMessage(KIO::Job *, const QString &) ),
             this, SLOT( slotInfoMessage(KIO::Job *, const QString &) ) );
    connect( job, SIGNAL( totalSize(KIO::Job *, KIO::filesize_t) ),
             this, SLOT( slotTotalSize(KIO::Job *, KIO::filesize_t) ) );
    connect( job, SIGNAL( processedSize(KIO::Job *, KIO::filesize_t) ),
             this, SLOT( slotProcessedSize(KIO::Job *, KIO::filesize_t) ) );
    connect( job, SIGNAL( speed(KIO::Job *, unsigned long) ),
             this, SLOT( slotSpeed(KIO::Job *, unsigned long) ) );
}

void ForwardingSlaveBase::connectSimpleJob(KIO::SimpleJob *job)
{
    connectJob(job);
    connect( job, SIGNAL( redirection(KIO::Job *, const KURL &) ),
             this, SLOT( slotRedirection(KIO::Job *, const KURL &) ) );
}

void ForwardingSlaveBase::connectListJob(KIO::ListJob *job)
{
    connectSimpleJob(job);
    connect( job, SIGNAL( entries(KIO::Job *, const KIO::UDSEntryList &) ),
             this, SLOT( slotEntries(KIO::Job *, const KIO::UDSEntryList &) ) );
}

void ForwardingSlaveBase::connectTransferJob(KIO::TransferJob *job)
{
    connectSimpleJob(job);
    connect( job, SIGNAL( data(KIO::Job *, const QByteArray &) ),
             this, SLOT( slotData(KIO::Job *, const QByteArray &) ) );
    connect( job, SIGNAL( dataReq(KIO::Job *, QByteArray &) ),
             this, SLOT( slotDataReq(KIO::Job *, QByteArray &) ) );
    connect( job, SIGNAL( mimetype(KIO::Job *, const QString &) ),
             this, SLOT( slotMimetype(KIO::Job *, const QString &) ) );
    connect( job, SIGNAL( canResume(KIO::Job *, KIO::filesize_t) ),
             this, SLOT( slotCanResume(KIO::Job *, KIO::filesize_t) ) );
}

//////////////////////////////////////////////////////////////////////////////

void ForwardingSlaveBase::slotResult(KIO::Job *job)
{
    if ( job->error() != 0)
    {
        error( job->error(), job->errorText() );
    }
    else
    {
        KIO::StatJob *stat_job = dynamic_cast<KIO::StatJob *>(job);
        if ( stat_job!=0L )
        {
            KIO::UDSEntry entry = stat_job->statResult();
	    prepareUDSEntry(entry);
            statEntry( entry );
        }
        finished();
    }

    eventLoop.exit();
}

void ForwardingSlaveBase::slotWarning(KIO::Job* /*job*/, const QString &msg)
{
    warning(msg);
}

void ForwardingSlaveBase::slotInfoMessage(KIO::Job* /*job*/, const QString &msg)
{
    infoMessage(msg);
}

void ForwardingSlaveBase::slotTotalSize(KIO::Job* /*job*/, KIO::filesize_t size)
{
    totalSize(size);
}

void ForwardingSlaveBase::slotProcessedSize(KIO::Job* /*job*/, KIO::filesize_t size)
{
    processedSize(size);
}

void ForwardingSlaveBase::slotSpeed(KIO::Job* /*job*/, unsigned long bytesPerSecond)
{
    speed(bytesPerSecond);
}

void ForwardingSlaveBase::slotRedirection(KIO::Job* /*job*/, const KURL &url)
{
    redirection(url);
}

void ForwardingSlaveBase::slotEntries(KIO::Job* /*job*/,
                                      const KIO::UDSEntryList &entries)
{
    KIO::UDSEntryList final_entries = entries;

    KIO::UDSEntryList::iterator it = final_entries.begin();
    const KIO::UDSEntryList::iterator end = final_entries.end();

    for(; it!=end; ++it)
    {
        prepareUDSEntry(*it, true);
    }

    listEntries( final_entries );
}

void ForwardingSlaveBase::slotData(KIO::Job* /*job*/, const QByteArray &d)
{
    data(d);
}

void ForwardingSlaveBase::slotDataReq(KIO::Job* /*job*/, QByteArray &data)
{
    dataReq();
    readData(data);
}

void ForwardingSlaveBase::slotMimetype (KIO::Job* /*job*/, const QString &type)
{
    mimeType(type);
}

void ForwardingSlaveBase::slotCanResume (KIO::Job* /*job*/, KIO::filesize_t offset)
{
    canResume(offset);
}

}

#include "forwardingslavebase.moc"

