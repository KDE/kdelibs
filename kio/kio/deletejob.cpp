/* This file is part of the KDE libraries
    Copyright 2000       Stephan Kulow <coolo@kde.org>
    Copyright 2000-2006  David Faure <faure@kde.org>
    Copyright 2000       Waldo Bastian <bastian@kde.org>

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

#include "deletejob.h"

#include "kmimetype.h"
#include "scheduler.h"
#include "kdirwatch.h"
#include "kprotocolmanager.h"
#include "jobuidelegate.h"
#include <kdirnotify.h>
#include <kuiserverjobtracker.h>

#include <kauthorized.h>
#include <klocale.h>
#include <kdebug.h>
#include <kde_file.h>

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QPointer>

#include "job_p.h"

namespace KIO
{
    enum DeleteJobState {
        STATE_STATING,
        STATE_LISTING,
        STATE_DELETING_FILES,
        STATE_DELETING_DIRS
    };

    class DeleteJobPrivate: public KIO::JobPrivate
    {
    public:
        DeleteJobPrivate(const KUrl::List& src)
            : state( STATE_STATING )
            , m_totalSize( 0 )
            , m_processedSize( 0 )
            , m_fileProcessedSize( 0 )
            , m_processedFiles( 0 )
            , m_processedDirs( 0 )
            , m_totalFilesDirs( 0 )
            , m_srcList( src )
            , m_currentStat( m_srcList.begin() )
            , m_reportTimer( 0 )
        {
        }
        DeleteJobState state;
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_processedSize;
        KIO::filesize_t m_fileProcessedSize;
        int m_processedFiles;
        int m_processedDirs;
        int m_totalFilesDirs;
        KUrl m_currentURL;
        KUrl::List files;
        KUrl::List symlinks;
        KUrl::List dirs;
        KUrl::List m_srcList;
        KUrl::List::Iterator m_currentStat;
	QStringList m_parentDirs;
        QTimer *m_reportTimer;

        Q_DECLARE_PUBLIC(DeleteJob)
    };

} // namespace KIO

using namespace KIO;

DeleteJob::DeleteJob(const KUrl::List& src)
: Job(* new DeleteJobPrivate(src) )
{
    d_func()->m_reportTimer = new QTimer(this);
    connect(d_func()->m_reportTimer,SIGNAL(timeout()),this,SLOT(slotReport()));
    //this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
    d_func()->m_reportTimer->start( 200 );

    QTimer::singleShot(0, this, SLOT(slotStart()));
}

DeleteJob::~DeleteJob()
{
}

KUrl::List DeleteJob::urls() const
{
    return d_func()->m_srcList;
}

void DeleteJob::slotStart()
{
  statNextSrc();
}

//this is called often, so calling the functions
//from Observer here directly might improve the performance a little bit
//aleXXX
void DeleteJob::slotReport()
{
   Q_D(DeleteJob);
   emit deleting( this, d->m_currentURL );
   emitDeleting(d->m_currentURL);

   switch( d->state ) {
        case STATE_STATING:
        case STATE_LISTING:
            setTotalAmount(KJob::Bytes, d->m_totalSize);
            setTotalAmount(KJob::Files, d->files.count());
            setTotalAmount(KJob::Directories, d->dirs.count());
            break;
        case STATE_DELETING_DIRS:
            setProcessedAmount(KJob::Directories, d->m_processedDirs);
            emitPercent( d->m_processedFiles + d->m_processedDirs, d->m_totalFilesDirs );
            break;
        case STATE_DELETING_FILES:
            setProcessedAmount(KJob::Files, d->m_processedFiles);
            emitPercent( d->m_processedFiles, d->m_totalFilesDirs );
            break;
   }
}


void DeleteJob::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
    Q_D(DeleteJob);
    UDSEntryList::ConstIterator it = list.begin();
    const UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it)
    {
        const UDSEntry& entry = *it;
        const QString displayName = entry.stringValue( KIO::UDSEntry::UDS_NAME );

        assert(!displayName.isEmpty());
        if (displayName != ".." && displayName != ".")
        {
            KUrl url;
            const QString urlStr = entry.stringValue( KIO::UDSEntry::UDS_URL );
            if ( !urlStr.isEmpty() )
                url = urlStr;
            else {
                url = ((SimpleJob *)job)->url(); // assumed to be a dir
                url.addPath( displayName );
            }

            d->m_totalSize += (KIO::filesize_t)entry.numberValue( KIO::UDSEntry::UDS_SIZE, 0 );

            //kDebug(7007) << "DeleteJob::slotEntries " << displayName << " (" << url << ")" << endl;
            if ( entry.isLink() )
                d->symlinks.append( url );
            else if ( entry.isDir() )
                d->dirs.append( url );
            else
                d->files.append( url );
        }
    }
}


void DeleteJob::statNextSrc()
{
    Q_D(DeleteJob);
    //kDebug(7007) << "statNextSrc" << endl;
    if ( d->m_currentStat != d->m_srcList.end() )
    {
        d->m_currentURL = (*d->m_currentStat);

        // if the file system doesn't support deleting, we do not even stat
        if (!KProtocolManager::supportsDeleting(d->m_currentURL)) {
            QPointer<DeleteJob> that = this;
            ++d->m_currentStat;
            emit warning( this, buildErrorString(ERR_CANNOT_DELETE, d->m_currentURL.prettyUrl()) );
            if (that)
                statNextSrc();
            return;
        }
        // Stat it
        d->state = STATE_STATING;
        KIO::SimpleJob * job = KIO::stat( d->m_currentURL, true, 1, false );
        Scheduler::scheduleJob(job);
        //kDebug(7007) << "KIO::stat (DeleteJob) " << d->m_currentURL << endl;
        addSubjob(job);
    } else
    {
        d->m_totalFilesDirs = d->files.count()+d->symlinks.count() + d->dirs.count();
        slotReport();
        // Now we know which dirs hold the files we're going to delete.
        // To speed things up and prevent double-notification, we disable KDirWatch
        // on those dirs temporarily (using KDirWatch::self, that's the instanced
        // used by e.g. kdirlister).
        for ( QStringList::Iterator it = d->m_parentDirs.begin() ; it != d->m_parentDirs.end() ; ++it )
            KDirWatch::self()->stopDirScan( *it );
        d->state = STATE_DELETING_FILES;
        deleteNextFile();
    }
}

void DeleteJob::deleteNextFile()
{
    Q_D(DeleteJob);
    //kDebug(7007) << "deleteNextFile" << endl;
    if ( !d->files.isEmpty() || !d->symlinks.isEmpty() )
    {
        SimpleJob *job;
        do {
            // Take first file to delete out of list
            KUrl::List::Iterator it = d->files.begin();
            bool isLink = false;
            if ( it == d->files.end() ) // No more files
            {
                it = d->symlinks.begin(); // Pick up a symlink to delete
                isLink = true;
            }
            // Normal deletion
            // If local file, try do it directly
            if ( (*it).isLocalFile() && unlink( QFile::encodeName((*it).path()) ) == 0 ) {
                //kdDebug(7007) << "DeleteJob deleted " << (*it).path() << endl;
                job = 0;
                d->m_processedFiles++;
                if ( d->m_processedFiles % 300 == 0 || d->m_totalFilesDirs < 300) { // update progress info every 300 files
                    d->m_currentURL = *it;
                    slotReport();
                }
            } else
            { // if remote - or if unlink() failed (we'll use the job's error handling in that case)
                job = KIO::file_delete( *it, false /*no GUI*/);
                Scheduler::scheduleJob(job);
                d->m_currentURL=(*it);
            }
            if ( isLink )
                d->symlinks.erase(it);
            else
                d->files.erase(it);
            if ( job ) {
                addSubjob(job);
                return;
            }
            // loop only if direct deletion worked (job=0) and there is something else to delete
        } while (!job && (!d->files.isEmpty() || !d->symlinks.isEmpty()));
    }
    d->state = STATE_DELETING_DIRS;
    deleteNextDir();
}

void DeleteJob::deleteNextDir()
{
    Q_D(DeleteJob);
    if ( !d->dirs.isEmpty() ) // some dirs to delete ?
    {
        do {
            // Take first dir to delete out of list - last ones first !
            KUrl::List::Iterator it = --d->dirs.end();
            // If local dir, try to rmdir it directly
            if ( (*it).isLocalFile() && ::rmdir( QFile::encodeName((*it).path()) ) == 0 ) {

                d->m_processedDirs++;
                if ( d->m_processedDirs % 100 == 0 ) { // update progress info every 100 dirs
                    d->m_currentURL = *it;
                    slotReport();
                }
            } else {
                SimpleJob* job;
                if ( KProtocolManager::canDeleteRecursive( *it ) ) {
                    // If the ioslave supports recursive deletion of a directory, then
                    // we only need to send a single CMD_DEL command, so we use file_delete :)
                    job = KIO::file_delete( *it, false /*no gui*/ );
                } else {
                    job = KIO::rmdir( *it );
                }
                Scheduler::scheduleJob(job);
                d->dirs.erase(it);
                addSubjob( job );
                return;
            }
            d->dirs.erase(it);
        } while ( !d->dirs.isEmpty() );
    }

    // Re-enable watching on the dirs that held the deleted files
    for ( QStringList::Iterator it = d->m_parentDirs.begin() ; it != d->m_parentDirs.end() ; ++it )
        KDirWatch::self()->restartDirScan( *it );

    // Finished - tell the world
    if ( !d->m_srcList.isEmpty() )
    {
        //kDebug(7007) << "KDirNotify'ing FilesRemoved " << d->m_srcList.toStringList() << endl;
        org::kde::KDirNotify::emitFilesRemoved( d->m_srcList.toStringList() );
    }
    if (d->m_reportTimer!=0)
       d->m_reportTimer->stop();
    emitResult();
}

void DeleteJob::slotProcessedSize( KJob*, qulonglong data_size )
{
   Q_D(DeleteJob);
   // Note: this is the same implementation as CopyJob::slotProcessedSize but
   // it's different from FileCopyJob::slotProcessedSize - which is why this
   // is not in Job.

   d->m_fileProcessedSize = data_size;
   setProcessedAmount(KJob::Bytes, d->m_processedSize + d->m_fileProcessedSize);

   //kDebug(7007) << "DeleteJob::slotProcessedSize " << (unsigned int) (d->m_processedSize + d->m_fileProcessedSize) << endl;

   setProcessedAmount(KJob::Bytes, d->m_processedSize + d->m_fileProcessedSize);

   // calculate percents
   if ( d->m_totalSize == 0 )
      setPercent( 100 );
   else
      setPercent( (unsigned long)(( (float)(d->m_processedSize + d->m_fileProcessedSize) / (float)d->m_totalSize ) * 100.0) );
}

void DeleteJob::slotResult( KJob *job )
{
    Q_D(DeleteJob);
    switch ( d->state )
    {
    case STATE_STATING:
    {
        // Was there an error while stating ?
        if (job->error() )
        {
            // Probably : doesn't exist
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }

        const UDSEntry entry = static_cast<StatJob*>(job)->statResult();
        const KUrl url = static_cast<SimpleJob*>(job)->url();
        const bool isLink = entry.isLink();

        removeSubjob( job );
        assert( !hasSubjobs() );

        // Is it a file or a dir ?
        if (entry.isDir() && !isLink)
        {
            // Add toplevel dir in list of dirs
            d->dirs.append( url );
            if ( url.isLocalFile() && !d->m_parentDirs.contains( url.path(KUrl::RemoveTrailingSlash) ) )
              d->m_parentDirs.append( url.path(KUrl::RemoveTrailingSlash) );

            if ( !KProtocolManager::canDeleteRecursive( url ) ) {
                //kDebug(7007) << " Target is a directory " << endl;
                // List it
                d->state = STATE_LISTING;
                ListJob *newjob = listRecursive( url, false );
                newjob->setUnrestricted(true); // No KIOSK restrictions
                Scheduler::scheduleJob(newjob);
                connect(newjob, SIGNAL(entries( KIO::Job *,
                                                const KIO::UDSEntryList& )),
                        SLOT( slotEntries( KIO::Job*,
                                           const KIO::UDSEntryList& )));
                addSubjob(newjob);
            } else {
                ++d->m_currentStat;
                statNextSrc();
            }
        }
        else
        {
            if ( isLink ) {
                //kDebug(7007) << " Target is a symlink" << endl;
                d->symlinks.append( url );
            } else {
                //kDebug(7007) << " Target is a file" << endl;
                d->files.append( url );
            }
            if ( url.isLocalFile() && !d->m_parentDirs.contains( url.directory(KUrl::ObeyTrailingSlash) ) )
                d->m_parentDirs.append( url.directory(KUrl::ObeyTrailingSlash) );
            ++d->m_currentStat;
            statNextSrc();
        }
    }
        break;
    case STATE_LISTING:
        if ( job->error() )
        {
            // Try deleting nonetheless, it may be empty (and non-listable)
        }
        removeSubjob( job );
        assert( !hasSubjobs() );
        ++d->m_currentStat;
        statNextSrc();
        break;
    case STATE_DELETING_FILES:
        if ( job->error() )
        {
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
        removeSubjob( job );
        assert( !hasSubjobs() );
        d->m_processedFiles++;

        deleteNextFile();
        break;
    case STATE_DELETING_DIRS:
        if ( job->error() )
        {
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
        removeSubjob( job );
        assert( !hasSubjobs() );
        d->m_processedDirs++;
        //emit processedAmount( this, KJob::Directories, d->m_processedDirs );
        //if (!m_shred)
        //emitPercent( d->m_processedFiles + d->m_processedDirs, d->m_totalFilesDirs );

        deleteNextDir();
        break;
    default:
        assert(0);
    }
}

DeleteJob *KIO::del( const KUrl& src, bool /*shred*/, bool showProgressInfo )
{
  KUrl::List srcList;
  srcList.append( src );
  DeleteJob *job = new DeleteJob(srcList);
  job->setUiDelegate(new JobUiDelegate());
  if (showProgressInfo) {
      KIO::getJobTracker()->registerJob(job);
  }
  return job;
}

DeleteJob *KIO::del( const KUrl::List& src, bool /*shred*/, bool showProgressInfo )
{
  DeleteJob *job = new DeleteJob(src);
  job->setUiDelegate(new JobUiDelegate());
  if (showProgressInfo) {
      KIO::getJobTracker()->registerJob(job);
  }
  return job;
}

#include "deletejob.moc"
