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
#include "kio/observer.h"
#include <kdirnotify.h>

#include <kauthorized.h>
#include <klocale.h>
#include <kdebug.h>
#include <kde_file.h>

#include <qtimer.h>
#include <qfile.h>

#include <assert.h>
#include <stdlib.h>
#include <time.h>

using namespace KIO;

DeleteJob::DeleteJob( const KUrl::List& src, bool /*shred*/, bool showProgressInfo )
: Job(showProgressInfo), m_totalSize( 0 ), m_processedSize( 0 ), m_fileProcessedSize( 0 ),
  m_processedFiles( 0 ), m_processedDirs( 0 ), m_totalFilesDirs( 0 ),
  m_srcList(src), m_currentStat(m_srcList.begin()), m_reportTimer(0)
{
  if ( showProgressInfo ) {

     connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
              Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );

     connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
              Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

     // See slotReport
     /*connect( this, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
      m_observer, SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );

      connect( this, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
      m_observer, SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

      connect( this, SIGNAL( deleting( KIO::Job*, const KUrl& ) ),
      m_observer, SLOT( slotDeleting( KIO::Job*, const KUrl& ) ) );*/

     m_reportTimer=new QTimer(this);
     connect(m_reportTimer,SIGNAL(timeout()),this,SLOT(slotReport()));
     //this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
     m_reportTimer->start( 200 );
  }

  QTimer::singleShot(0, this, SLOT(slotStart()));
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
   if (progressId()==0)
      return;

   Observer * observer = Observer::self();

   emit deleting( this, m_currentURL );
   observer->slotDeleting(this,m_currentURL);

   switch( state ) {
        case STATE_STATING:
        case STATE_LISTING:
            emit totalSize( this, m_totalSize );
            emit totalFiles( this, files.count() );
            emit totalDirs( this, dirs.count() );
            break;
        case STATE_DELETING_DIRS:
            emit processedDirs( this, m_processedDirs );
            observer->slotProcessedDirs(this,m_processedDirs);
            emitPercent( m_processedFiles + m_processedDirs, m_totalFilesDirs );
            break;
        case STATE_DELETING_FILES:
            observer->slotProcessedFiles(this,m_processedFiles);
            emit processedFiles( this, m_processedFiles );
            emitPercent( m_processedFiles, m_totalFilesDirs );
            break;
   }
}


void DeleteJob::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
    UDSEntryList::ConstIterator it = list.begin();
    const UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it)
    {
        const UDSEntry& entry = *it;
        const QString displayName = entry.stringValue( KIO::UDS_NAME );

        assert(!displayName.isEmpty());
        if (displayName != ".." && displayName != ".")
        {
            KUrl url;
            const QString urlStr = entry.stringValue( KIO::UDS_URL );
            if ( !urlStr.isEmpty() )
                url = urlStr;
            else {
                url = ((SimpleJob *)job)->url(); // assumed to be a dir
                url.addPath( displayName );
            }

            m_totalSize += (KIO::filesize_t)entry.numberValue( KIO::UDS_SIZE, 0 );

            //kDebug(7007) << "DeleteJob::slotEntries " << displayName << " (" << url << ")" << endl;
            if ( entry.isLink() )
                symlinks.append( url );
            else if ( entry.isDir() )
                dirs.append( url );
            else
                files.append( url );
        }
    }
}


void DeleteJob::statNextSrc()
{
    //kDebug(7007) << "statNextSrc" << endl;
    if ( m_currentStat != m_srcList.end() )
    {
        m_currentURL = (*m_currentStat);

        // if the file system doesn't support deleting, we do not even stat
        if (!KProtocolManager::supportsDeleting(m_currentURL)) {
            QPointer<DeleteJob> that = this;
            ++m_currentStat;
            emit warning( this, buildErrorString(ERR_CANNOT_DELETE, m_currentURL.prettyUrl()) );
            if (that)
                statNextSrc();
            return;
        }
        // Stat it
        state = STATE_STATING;
        KIO::SimpleJob * job = KIO::stat( m_currentURL, true, 1, false );
        Scheduler::scheduleJob(job);
        //kDebug(7007) << "KIO::stat (DeleteJob) " << m_currentURL << endl;
        addSubjob(job);
        //if ( progressId() ) // Did we get an ID from the observer ?
        //  Observer::self()->slotDeleting( this, *it ); // show asap
    } else
    {
        m_totalFilesDirs = files.count()+symlinks.count() + dirs.count();
        slotReport();
        // Now we know which dirs hold the files we're going to delete.
        // To speed things up and prevent double-notification, we disable KDirWatch
        // on those dirs temporarily (using KDirWatch::self, that's the instanced
        // used by e.g. kdirlister).
        for ( QStringList::Iterator it = m_parentDirs.begin() ; it != m_parentDirs.end() ; ++it )
            KDirWatch::self()->stopDirScan( *it );
        state = STATE_DELETING_FILES;
        deleteNextFile();
    }
}

void DeleteJob::deleteNextFile()
{
    //kDebug(7007) << "deleteNextFile" << endl;
    if ( !files.isEmpty() || !symlinks.isEmpty() )
    {
        SimpleJob *job;
        do {
            // Take first file to delete out of list
            KUrl::List::Iterator it = files.begin();
            bool isLink = false;
            if ( it == files.end() ) // No more files
            {
                it = symlinks.begin(); // Pick up a symlink to delete
                isLink = true;
            }
            // Normal deletion
            // If local file, try do it directly
            if ( (*it).isLocalFile() && unlink( QFile::encodeName((*it).path()) ) == 0 ) {
                //kdDebug(7007) << "DeleteJob deleted " << (*it).path() << endl;
                job = 0;
                m_processedFiles++;
                if ( m_processedFiles % 300 == 0 || m_totalFilesDirs < 300) { // update progress info every 300 files
                    m_currentURL = *it;
                    slotReport();
                }
            } else
            { // if remote - or if unlink() failed (we'll use the job's error handling in that case)
                job = KIO::file_delete( *it, false /*no GUI*/);
                Scheduler::scheduleJob(job);
                m_currentURL=(*it);
            }
            if ( isLink )
                symlinks.erase(it);
            else
                files.erase(it);
            if ( job ) {
                addSubjob(job);
                return;
            }
            // loop only if direct deletion worked (job=0) and there is something else to delete
        } while (!job && (!files.isEmpty() || !symlinks.isEmpty()));
    }
    state = STATE_DELETING_DIRS;
    deleteNextDir();
}

void DeleteJob::deleteNextDir()
{
    if ( !dirs.isEmpty() ) // some dirs to delete ?
    {
        do {
            // Take first dir to delete out of list - last ones first !
            KUrl::List::Iterator it = --dirs.end();
            // If local dir, try to rmdir it directly
            if ( (*it).isLocalFile() && ::rmdir( QFile::encodeName((*it).path()) ) == 0 ) {

                m_processedDirs++;
                if ( m_processedDirs % 100 == 0 ) { // update progress info every 100 dirs
                    m_currentURL = *it;
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
                dirs.erase(it);
                addSubjob( job );
                return;
            }
            dirs.erase(it);
        } while ( !dirs.isEmpty() );
    }

    // Re-enable watching on the dirs that held the deleted files
    for ( QStringList::Iterator it = m_parentDirs.begin() ; it != m_parentDirs.end() ; ++it )
        KDirWatch::self()->restartDirScan( *it );

    // Finished - tell the world
    if ( !m_srcList.isEmpty() )
    {
        //kDebug(7007) << "KDirNotify'ing FilesRemoved " << m_srcList.toStringList() << endl;
        org::kde::KDirNotify::emitFilesRemoved( m_srcList.toStringList() );
    }
    if (m_reportTimer!=0)
       m_reportTimer->stop();
    emitResult();
}

void DeleteJob::slotProcessedSize( KJob*, qulonglong data_size )
{
   // Note: this is the same implementation as CopyJob::slotProcessedSize but
   // it's different from FileCopyJob::slotProcessedSize - which is why this
   // is not in Job.

   m_fileProcessedSize = data_size;
   setProcessedSize(m_processedSize + m_fileProcessedSize);

   //kDebug(7007) << "DeleteJob::slotProcessedSize " << (unsigned int) (m_processedSize + m_fileProcessedSize) << endl;

   emit processedSize( this, m_processedSize + m_fileProcessedSize );

   // calculate percents
   unsigned long ipercent = percent();

   if ( m_totalSize == 0 )
      setPercent( 100 );
   else
      setPercent( (unsigned long)(( (float)(m_processedSize + m_fileProcessedSize) / (float)m_totalSize ) * 100.0) );

   if ( percent() > ipercent )
   {
      emit percent( this, percent() );
      //kDebug(7007) << "DeleteJob::slotProcessedSize - percent =  " << (unsigned int) m_percent << endl;
   }

}

void DeleteJob::slotResult( KJob *job )
{
    switch ( state )
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
            dirs.append( url );
            if ( url.isLocalFile() && !m_parentDirs.contains( url.path(KUrl::RemoveTrailingSlash) ) )
              m_parentDirs.append( url.path(KUrl::RemoveTrailingSlash) );

            if ( !KProtocolManager::canDeleteRecursive( url ) ) {
                //kDebug(7007) << " Target is a directory " << endl;
                // List it
                state = STATE_LISTING;
                ListJob *newjob = listRecursive( url, false );
                newjob->setUnrestricted(true); // No KIOSK restrictions
                Scheduler::scheduleJob(newjob);
                connect(newjob, SIGNAL(entries( KIO::Job *,
                                                const KIO::UDSEntryList& )),
                        SLOT( slotEntries( KIO::Job*,
                                           const KIO::UDSEntryList& )));
                addSubjob(newjob);
            } else {
                ++m_currentStat;
                statNextSrc();
            }
        }
        else
        {
            if ( isLink ) {
                //kDebug(7007) << " Target is a symlink" << endl;
                symlinks.append( url );
            } else {
                //kDebug(7007) << " Target is a file" << endl;
                files.append( url );
            }
            if ( url.isLocalFile() && !m_parentDirs.contains( url.directory(KUrl::ObeyTrailingSlash) ) )
                m_parentDirs.append( url.directory(KUrl::ObeyTrailingSlash) );
            ++m_currentStat;
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
        ++m_currentStat;
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
        m_processedFiles++;

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
        m_processedDirs++;
        //emit processedDirs( this, m_processedDirs );
        //if (!m_shred)
        //emitPercent( m_processedFiles + m_processedDirs, m_totalFilesDirs );

        deleteNextDir();
        break;
    default:
        assert(0);
    }
}

DeleteJob *KIO::del( const KUrl& src, bool shred, bool showProgressInfo )
{
  KUrl::List srcList;
  srcList.append( src );
  DeleteJob *job = new DeleteJob( srcList, shred, showProgressInfo );
  return job;
}

DeleteJob *KIO::del( const KUrl::List& src, bool shred, bool showProgressInfo )
{
  DeleteJob *job = new DeleteJob( src, shred, showProgressInfo );
  return job;
}

#include "deletejob.moc"
