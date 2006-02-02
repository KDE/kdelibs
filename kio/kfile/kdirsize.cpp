/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "kdirsize.h"
#include <kdebug.h>
#include <kglobal.h>
#include <qapplication.h>
#include <qtimer.h>
#include <config-kfile.h>

using namespace KIO;

KDirSize::KDirSize( const KUrl & directory )
    : KIO::Job(false /*No GUI*/), m_bAsync(true), m_totalSize(0L), m_totalFiles(0L), m_totalSubdirs(0L)
{
    startNextJob( directory );
}

KDirSize::KDirSize( const KFileItemList & lstItems )
    : KIO::Job(false /*No GUI*/), m_bAsync(true), m_totalSize(0L), m_totalFiles(0L), m_totalSubdirs(0L), m_lstItems(lstItems)
{
    QTimer::singleShot( 0, this, SLOT(processList()) );
}

void KDirSize::processList()
{
    while (!m_lstItems.isEmpty())
    {
        KFileItem * item = m_lstItems.first();
        m_lstItems.removeFirst();
	if ( !item->isLink() )
	{
            if ( item->isDir() )
            {
                kDebug(kfile_area) << "KDirSize::processList dir -> listing" << endl;
                KUrl url = item->url();
                startNextJob( url );
                return; // we'll come back later, when this one's finished
            }
            else
            {
                m_totalSize += item->size();
// no long long with kDebug()
//            kDebug(kfile_area) << "KDirSize::processList file -> " << m_totalSize << endl;
            }
	}
    }
    kDebug(kfile_area) << "KDirSize::processList finished" << endl;
    if ( !m_bAsync )
        qApp->exit_loop();
    emitResult();
}

void KDirSize::startNextJob( const KUrl & url )
{
    KIO::ListJob * listJob = KIO::listRecursive( url, false /* no GUI */ );
    connect( listJob, SIGNAL(entries( KIO::Job *,
                                      const KIO::UDSEntryList& )),
             SLOT( slotEntries( KIO::Job*,
                                const KIO::UDSEntryList& )));
    addSubjob( listJob );
}

void KDirSize::slotEntries( KIO::Job*, const KIO::UDSEntryList & list )
{
    static const QString& dot = KGlobal::staticQString( "." );
    static const QString& dotdot = KGlobal::staticQString( ".." );
    KIO::UDSEntryList::ConstIterator it = list.begin();
    const KIO::UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it) {

        const KIO::UDSEntry& entry = *it;
        const KIO::filesize_t size = entry.numberValue( KIO::UDS_SIZE, -1 );
        const QString name = entry.stringValue( KIO::UDS_NAME );
        if ( name == dot )
            m_totalSize += size;
        else if ( name != dotdot )
        {
            if (!entry.isLink())
              m_totalSize += size;
            if (!entry.isDir())
              m_totalFiles++;
            else
              m_totalSubdirs++;
            //kDebug(kfile_area) << name << ":" << size << endl;
        }
    }
}

//static
KDirSize * KDirSize::dirSizeJob( const KUrl & directory )
{
    return new KDirSize( directory ); // useless - but consistent with other jobs
}

//static
KDirSize * KDirSize::dirSizeJob( const KFileItemList & lstItems )
{
    return new KDirSize( lstItems );
}

//static
KIO::filesize_t KDirSize::dirSize( const KUrl & directory )
{
    KDirSize * dirSize = dirSizeJob( directory );
    dirSize->setSync();
    qApp->enter_loop();
    return dirSize->totalSize();
}


void KDirSize::slotResult( KIO::Job * job )
{
    kDebug(kfile_area) << " KDirSize::slotResult( KIO::Job * job ) m_lstItems:" << m_lstItems.count() << endl;
    if ( !m_lstItems.isEmpty() )
    {
        removeSubjob(job);
        processList();
    }
    else
    {
        if ( !m_bAsync )
            qApp->exit_loop();
        KIO::Job::slotResult( job );
    }
}

void KDirSize::virtual_hook( int id, void* data )
{ KIO::Job::virtual_hook( id, data ); }

#include "kdirsize.moc"
