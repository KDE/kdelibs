/* This file is part of the KDE libraries
    Copyright (C) 2000, 2006 David Faure <faure@kde.org>

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

#include "directorysizejob.h"
#include <kdebug.h>
#include <QtCore/QTimer>

#include "jobuidelegate.h"

namespace KIO
{
    class DirectorySizeJobPrivate
    {
    public:
        DirectorySizeJobPrivate()
            : m_totalSize(0L)
            , m_totalFiles(0L)
            , m_totalSubdirs(0L)
            , m_currentItem(0)
        {
        }
        DirectorySizeJobPrivate( const QList<KFileItem> & lstItems )
            : m_totalSize(0L)
            , m_totalFiles(0L)
            , m_totalSubdirs(0L)
            , m_lstItems(lstItems)
            , m_currentItem(0)
        {
        }
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_totalFiles;
        KIO::filesize_t m_totalSubdirs;
        QList<KFileItem> m_lstItems;
        int m_currentItem;
    };

} // namespace KIO


using namespace KIO;

DirectorySizeJob::DirectorySizeJob( const KUrl & directory )
    : KIO::Job(), d( new DirectorySizeJobPrivate )
{
    startNextJob( directory );
}

DirectorySizeJob::DirectorySizeJob( const QList<KFileItem> & lstItems )
    : KIO::Job(), d( new DirectorySizeJobPrivate(lstItems) )
{
    QTimer::singleShot( 0, this, SLOT(processNextItem()) );
}


DirectorySizeJob::~DirectorySizeJob()
{
    delete d;
}

KIO::filesize_t DirectorySizeJob::totalSize() const
{
    return d->m_totalSize;
}

KIO::filesize_t DirectorySizeJob::totalFiles() const
{
    return d->m_totalFiles;
}

KIO::filesize_t DirectorySizeJob::totalSubdirs() const
{
    return d->m_totalSubdirs;
}

void DirectorySizeJob::processNextItem()
{
    while (d->m_currentItem < d->m_lstItems.count())
    {
        const KFileItem item = d->m_lstItems[d->m_currentItem++];
	if ( !item.isLink() )
	{
            if ( item.isDir() )
            {
                kDebug(7007) << "DirectorySizeJob::processNextItem dir -> listing" << endl;
                KUrl url = item.url();
                startNextJob( url );
                return; // we'll come back later, when this one's finished
            }
            else
            {
                d->m_totalSize += item.size();
                kDebug(7007) << "DirectorySizeJob::processNextItem file -> " << d->m_totalSize << endl;
            }
	}
    }
    kDebug(7007) << "DirectorySizeJob::processNextItem finished" << endl;
    emitResult();
}

void DirectorySizeJob::startNextJob( const KUrl & url )
{
    kDebug(7007) << "DirectorySizeJob::startNextJob " << url << endl;
    KIO::ListJob * listJob = KIO::listRecursive( url, false /* no GUI */ );
    connect( listJob, SIGNAL(entries( KIO::Job *,
                                      const KIO::UDSEntryList& )),
             SLOT( slotEntries( KIO::Job*,
                                const KIO::UDSEntryList& )));
    addSubjob( listJob );
}

void DirectorySizeJob::slotEntries( KIO::Job*, const KIO::UDSEntryList & list )
{
    KIO::UDSEntryList::ConstIterator it = list.begin();
    const KIO::UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it) {

        const KIO::UDSEntry& entry = *it;
        const KIO::filesize_t size = entry.numberValue( KIO::UDS_SIZE, -1 );
        const QString name = entry.stringValue( KIO::UDS_NAME );
        if ( name == "." )
            d->m_totalSize += size;
        else if ( name != ".." )
        {
            if (!entry.isLink())
              d->m_totalSize += size;
            if (!entry.isDir())
              d->m_totalFiles++;
            else
              d->m_totalSubdirs++;
            //kDebug(7007) << name << ":" << size << endl;
        }
    }
}

void DirectorySizeJob::slotResult( KJob * job )
{
    kDebug(7007) << " DirectorySizeJob::slotResult()" << endl;
    if (d->m_currentItem < d->m_lstItems.count())
    {
        removeSubjob(job);
        processNextItem();
    }
    else
    {
        if (job->error()) {
            setError( job->error() );
            setErrorText( job->errorText() );
        }
        emitResult();
    }
}

//static
DirectorySizeJob * KIO::directorySize( const KUrl & directory )
{
    DirectorySizeJob *job = new DirectorySizeJob(directory); // useless - but consistent with other jobs
    job->setUiDelegate(new JobUiDelegate());
    return job;
}

//static
DirectorySizeJob * KIO::directorySize( const QList<KFileItem> & lstItems )
{
    DirectorySizeJob *job = new DirectorySizeJob(lstItems);
    job->setUiDelegate(new JobUiDelegate());
    return job;
}

#include "directorysizejob.moc"
