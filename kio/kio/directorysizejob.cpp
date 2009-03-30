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
#include "job_p.h"

namespace KIO
{
    class DirectorySizeJobPrivate: public KIO::JobPrivate
    {
    public:
        DirectorySizeJobPrivate()
            : m_totalSize(0L)
            , m_totalFiles(0L)
            , m_totalSubdirs(0L)
            , m_currentItem(0)
        {
        }
        DirectorySizeJobPrivate( const KFileItemList & lstItems )
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
        KFileItemList m_lstItems;
        int m_currentItem;

        void startNextJob( const KUrl & url );
        void slotEntries( KIO::Job * , const KIO::UDSEntryList &);
        void processNextItem();

        Q_DECLARE_PUBLIC(DirectorySizeJob)

        static inline DirectorySizeJob *newJob( const KUrl & directory )
        {
            DirectorySizeJobPrivate *d = new DirectorySizeJobPrivate;
            DirectorySizeJob *job = new DirectorySizeJob(*d);
            job->setUiDelegate(new JobUiDelegate);
            d->startNextJob(directory);
            return job;
        }

        static inline DirectorySizeJob *newJob( const KFileItemList & lstItems )
        {
            DirectorySizeJobPrivate *d = new DirectorySizeJobPrivate(lstItems);
            DirectorySizeJob *job = new DirectorySizeJob(*d);
            job->setUiDelegate(new JobUiDelegate);
            QTimer::singleShot( 0, job, SLOT(processNextItem()) );
            return job;
        }
    };

} // namespace KIO


using namespace KIO;

DirectorySizeJob::DirectorySizeJob(DirectorySizeJobPrivate &dd)
    : KIO::Job(dd)
{
}

DirectorySizeJob::~DirectorySizeJob()
{
}

KIO::filesize_t DirectorySizeJob::totalSize() const
{
    return d_func()->m_totalSize;
}

KIO::filesize_t DirectorySizeJob::totalFiles() const
{
    return d_func()->m_totalFiles;
}

KIO::filesize_t DirectorySizeJob::totalSubdirs() const
{
    return d_func()->m_totalSubdirs;
}

void DirectorySizeJobPrivate::processNextItem()
{
    Q_Q(DirectorySizeJob);
    while (m_currentItem < m_lstItems.count())
    {
        const KFileItem item = m_lstItems[m_currentItem++];
	if ( !item.isLink() )
	{
            if ( item.isDir() )
            {
                //kDebug(7007) << "dir -> listing";
                KUrl url = item.url();
                startNextJob( url );
                return; // we'll come back later, when this one's finished
            }
            else
            {
                m_totalSize += item.size();
                m_totalFiles++;
                //kDebug(7007) << "file -> " << m_totalSize;
            }
        } else {
            m_totalFiles++;
        }
    }
    //kDebug(7007) << "finished";
    q->emitResult();
}

void DirectorySizeJobPrivate::startNextJob( const KUrl & url )
{
    Q_Q(DirectorySizeJob);
    //kDebug(7007) << url;
    KIO::ListJob * listJob = KIO::listRecursive( url, KIO::HideProgressInfo );
    q->connect( listJob, SIGNAL(entries( KIO::Job *, const KIO::UDSEntryList& )),
                SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& )));
    q->addSubjob( listJob );
}

void DirectorySizeJobPrivate::slotEntries( KIO::Job*, const KIO::UDSEntryList & list )
{
    KIO::UDSEntryList::ConstIterator it = list.begin();
    const KIO::UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it) {

        const KIO::UDSEntry& entry = *it;
        const KIO::filesize_t size = entry.numberValue(KIO::UDSEntry::UDS_SIZE, 0);
        const QString name = entry.stringValue( KIO::UDSEntry::UDS_NAME );
        if (name == ".") {
            m_totalSize += size;
            //kDebug(7007) << "'.': added" << size << "->" << m_totalSize;
        } else if (name != "..") {
            if (!entry.isLink())
              m_totalSize += size;
            if (!entry.isDir())
              m_totalFiles++;
            else
              m_totalSubdirs++;
            //kDebug(7007) << name << ":" << size << "->" << m_totalSize;
        }
    }
}

void DirectorySizeJob::slotResult( KJob * job )
{
    Q_D(DirectorySizeJob);
    //kDebug(7007) << d->m_totalSize;
    removeSubjob(job);
    if (d->m_currentItem < d->m_lstItems.count())
    {
        d->processNextItem();
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
    return DirectorySizeJobPrivate::newJob(directory); // useless - but consistent with other jobs
}

//static
DirectorySizeJob * KIO::directorySize( const KFileItemList & lstItems )
{
    return DirectorySizeJobPrivate::newJob(lstItems);
}

#include "directorysizejob.moc"
