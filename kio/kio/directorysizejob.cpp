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
#include <qtimer.h>

using namespace KIO;

DirectorySizeJob::DirectorySizeJob( const KUrl & directory )
    : KIO::Job(false /*No GUI*/), m_totalSize(0L), m_totalFiles(0L), m_totalSubdirs(0L), m_currentItem(0)
{
    startNextJob( directory );
}

DirectorySizeJob::DirectorySizeJob( const KFileItemList & lstItems )
    : KIO::Job(false /*No GUI*/), m_totalSize(0L), m_totalFiles(0L), m_totalSubdirs(0L), m_lstItems(lstItems), m_currentItem(0)
{
    QTimer::singleShot( 0, this, SLOT(processNextItem()) );
}


DirectorySizeJob::~DirectorySizeJob()
{
}

void DirectorySizeJob::processNextItem()
{
    while (m_currentItem < m_lstItems.count())
    {
        KFileItem * item = m_lstItems[m_currentItem++];
	if ( !item->isLink() )
	{
            if ( item->isDir() )
            {
                kDebug(7007) << "DirectorySizeJob::processNextItem dir -> listing" << endl;
                KUrl url = item->url();
                startNextJob( url );
                return; // we'll come back later, when this one's finished
            }
            else
            {
                m_totalSize += item->size();
                kDebug(7007) << "DirectorySizeJob::processNextItem file -> " << m_totalSize << endl;
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
            m_totalSize += size;
        else if ( name != ".." )
        {
            if (!entry.isLink())
              m_totalSize += size;
            if (!entry.isDir())
              m_totalFiles++;
            else
              m_totalSubdirs++;
            //kDebug(7007) << name << ":" << size << endl;
        }
    }
}

void DirectorySizeJob::slotResult( KJob * job )
{
    kDebug(7007) << " DirectorySizeJob::slotResult()" << endl;
    if (m_currentItem < m_lstItems.count())
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
    return new DirectorySizeJob( directory ); // useless - but consistent with other jobs
}

//static
DirectorySizeJob * KIO::directorySize( const KFileItemList & lstItems )
{
    return new DirectorySizeJob( lstItems );
}

#include "directorysizejob.moc"
