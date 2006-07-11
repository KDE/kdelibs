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

#include "copyjob.h"

#include <assert.h>

#include <qtimer.h>
#include <qfile.h>

#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kde_file.h>

#include "slave.h"
#include "scheduler.h"
#include "kdirwatch.h"
#include "kprotocolmanager.h"

#include "kio/observer.h"

#include <kdirnotify.h>
#include <ktempfile.h>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif

using namespace KIO;

//this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
#define REPORT_TIMEOUT 200

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( &packedArgs, QIODevice::WriteOnly ); stream

class CopyJob::CopyJobPrivate
{
public:
    CopyJobPrivate() {
        m_defaultPermissions = false;
        m_bURLDirty = false;
    }
    // This is the dest URL that was initially given to CopyJob
    // It is copied into m_dest, which can be changed for a given src URL
    // (when using the RENAME dialog in slotResult),
    // and which will be reset for the next src URL.
    KUrl m_globalDest;
    // The state info about that global dest
    CopyJob::DestinationState m_globalDestinationState;
    // See setDefaultPermissions
    bool m_defaultPermissions;
    // Whether URLs changed (and need to be emitted by the next slotReport call)
    bool m_bURLDirty;
    // Used after copying all the files into the dirs, to set mtime (TODO: and permissions?)
    // after the copy is done
    QLinkedList<CopyInfo> m_directoriesCopied;
};

CopyJob::CopyJob( const KUrl::List& src, const KUrl& dest, CopyMode mode, bool asMethod, bool showProgressInfo )
  : Job(showProgressInfo), m_mode(mode), m_asMethod(asMethod),
    destinationState(DEST_NOT_STATED), state(STATE_STATING),
    m_totalSize(0), m_processedSize(0), m_fileProcessedSize(0),
    m_processedFiles(0), m_processedDirs(0),
    m_srcList(src), m_currentStatSrc(m_srcList.begin()),
    m_bCurrentOperationIsLink(false), m_bSingleFileCopy(false), m_bOnlyRenames(mode==Move),
    m_dest(dest), m_bAutoSkip( false ), m_bOverwriteAll( false ),
    m_conflictError(0), m_reportTimer(0),d(new CopyJobPrivate)
{
    d->m_globalDest = dest;
    d->m_globalDestinationState = destinationState;

    if ( showProgressInfo ) {
        connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );

        connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );
    }
    QTimer::singleShot(0, this, SLOT(slotStart()));
    /**
       States:
       STATE_STATING for the dest
       STATE_STATING for each src url (statNextSrc)
            for each: if dir -> STATE_LISTING (filling 'dirs' and 'files')
            but if direct rename possible: STATE_RENAMING instead.
       STATE_CREATING_DIRS (createNextDir, iterating over 'dirs')
            if conflict: STATE_CONFLICT_CREATING_DIRS
       STATE_COPYING_FILES (copyNextFile, iterating over 'files')
            if conflict: STATE_CONFLICT_COPYING_FILES
       STATE_DELETING_DIRS (deleteNextDir) (if moving)
       STATE_SETTING_DIR_ATTRIBUTES (setNextDirAttribute, iterating over d->m_directoriesCopied)
       done.
    */
}

CopyJob::~CopyJob()
{
    delete d;
}

void CopyJob::slotStart()
{
    /**
       We call the functions directly instead of using signals.
       Calling a function via a signal takes approx. 65 times the time
       compared to calling it directly (at least on my machine). aleXXX
    */
    m_reportTimer = new QTimer(this);

    connect(m_reportTimer,SIGNAL(timeout()),this,SLOT(slotReport()));
    m_reportTimer->start(REPORT_TIMEOUT);

    // Stat the dest
    KIO::Job * job = KIO::stat( m_dest, false, 2, false );
    //kDebug(7007) << "CopyJob:stating the dest " << m_dest << endl;
    addSubjob(job);
}

// For unit test purposes
KIO_EXPORT bool kio_resolve_local_urls = true;

void CopyJob::slotResultStating( KJob *job )
{
    //kDebug(7007) << "CopyJob::slotResultStating" << endl;
    // Was there an error while stating the src ?
    if (job->error() && destinationState != DEST_NOT_STATED )
    {
        KUrl srcurl = ((SimpleJob*)job)->url();
        if ( !srcurl.isLocalFile() )
        {
            // Probably : src doesn't exist. Well, over some protocols (e.g. FTP)
            // this info isn't really reliable (thanks to MS FTP servers).
            // We'll assume a file, and try to download anyway.
            kDebug(7007) << "Error while stating source. Activating hack" << endl;
            removeSubjob( job );
            assert ( !hasSubjobs() ); // We should have only one job at a time ...
            struct CopyInfo info;
            info.permissions = (mode_t) -1;
            info.mtime = (time_t) -1;
            info.ctime = (time_t) -1;
            info.size = (KIO::filesize_t)-1;
            info.uSource = srcurl;
            info.uDest = m_dest;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR && !m_asMethod )
                info.uDest.addPath( srcurl.fileName() );

            files.append( info );
            statNextSrc();
            return;
        }
        // Local file. If stat fails, the file definitely doesn't exist.
        Job::slotResult( job ); // will set the error and emit result(this)
        return;
    }

    // Keep copy of the stat result
    const UDSEntry entry = static_cast<StatJob*>(job)->statResult();
    const QString sLocalPath = entry.stringValue( KIO::UDS_LOCAL_PATH );
    const bool isDir = entry.isDir();

    if ( destinationState == DEST_NOT_STATED )
        // we were stating the dest
    {
        if (job->error())
            destinationState = DEST_DOESNT_EXIST;
        else {
            // Treat symlinks to dirs as dirs here, so no test on isLink
            destinationState = isDir ? DEST_IS_DIR : DEST_IS_FILE;
            //kDebug(7007) << "CopyJob::slotResultStating dest is dir:" << bDir << endl;
        }
        const bool isGlobalDest = m_dest == d->m_globalDest;
        if ( isGlobalDest )
            d->m_globalDestinationState = destinationState;

        if ( !sLocalPath.isEmpty() && kio_resolve_local_urls ) {
            m_dest = KUrl();
            m_dest.setPath(sLocalPath);
            if ( isGlobalDest )
                d->m_globalDest = m_dest;
        }

        removeSubjob( job );
        assert ( !hasSubjobs() );

        // After knowing what the dest is, we can start stat'ing the first src.
        statCurrentSrc();
        return;
    }

    // Is it a file or a dir ?
    const QString sName = entry.stringValue( KIO::UDS_NAME );

    // We were stating the current source URL
    m_currentDest = m_dest; // used by slotEntries
    // Create a dummy list with it, for slotEntries
    UDSEntryList lst;
    lst.append(entry);

    // There 6 cases, and all end up calling slotEntries(job, lst) first :
    // 1 - src is a dir, destination is a directory,
    // slotEntries will append the source-dir-name to the destination
    // 2 - src is a dir, destination is a file, ERROR (done later on)
    // 3 - src is a dir, destination doesn't exist, then it's the destination dirname,
    // so slotEntries will use it as destination.

    // 4 - src is a file, destination is a directory,
    // slotEntries will append the filename to the destination.
    // 5 - src is a file, destination is a file, m_dest is the exact destination name
    // 6 - src is a file, destination doesn't exist, m_dest is the exact destination name
    // Tell slotEntries not to alter the src url
    m_bCurrentSrcIsDir = false;
    slotEntries(static_cast<KIO::Job*>( job ), lst);

    KUrl srcurl;
    if (!sLocalPath.isEmpty())
        srcurl.setPath(sLocalPath);
    else
        srcurl = ((SimpleJob*)job)->url();

    removeSubjob( job );
    assert ( !hasSubjobs() ); // We should have only one job at a time ...

    if ( isDir
         // treat symlinks as files (no recursion)
         && !entry.isLink()
         && m_mode != Link ) // No recursion in Link mode either.
    {
        //kDebug(7007) << " Source is a directory " << endl;

        m_bCurrentSrcIsDir = true; // used by slotEntries
        if ( destinationState == DEST_IS_DIR ) // (case 1)
        {
            if ( !m_asMethod )
            {
                // Use <desturl>/<directory_copied> as destination, from now on
                QString directory = srcurl.fileName();
                if ( !sName.isEmpty() && KProtocolManager::fileNameUsedForCopying( srcurl ) == KProtocolInfo::Name )
                {
                    directory = sName;
                }
                m_currentDest.addPath( directory );
            }
        }
        else if ( destinationState == DEST_IS_FILE ) // (case 2)
        {
            setError( ERR_IS_FILE );
            setErrorText( m_dest.prettyUrl() );
            emitResult();
            return;
        }
        else // (case 3)
        {
            // otherwise dest is new name for toplevel dir
            // so the destination exists, in fact, from now on.
            // (This even works with other src urls in the list, since the
            //  dir has effectively been created)
            destinationState = DEST_IS_DIR;
            if ( m_dest == d->m_globalDest )
                d->m_globalDestinationState = destinationState;
        }

        startListing( srcurl );
    }
    else
    {
        //kDebug(7007) << " Source is a file (or a symlink), or we are linking -> no recursive listing " << endl;
        statNextSrc();
    }
}

void CopyJob::suspend()
{
    slotReport();
    Job::suspend();
}

void CopyJob::slotReport()
{
    if ( isSuspended() )
        return;
    // If showProgressInfo was set, progressId() is > 0.
    Observer * observer = progressId() ? Observer::self() : 0L;
    switch (state) {
        case STATE_COPYING_FILES:
            emit processedFiles( this, m_processedFiles );
            if (observer) observer->slotProcessedFiles(this, m_processedFiles);
            if (d->m_bURLDirty)
            {
                // Only emit urls when they changed. This saves time, and fixes #66281
                d->m_bURLDirty = false;
                if (m_mode==Move)
                {
                    if (observer) observer->slotMoving( this, m_currentSrcURL, m_currentDestURL);
                    emit moving( this, m_currentSrcURL, m_currentDestURL);
                }
                else if (m_mode==Link)
                {
                    if (observer) observer->slotCopying( this, m_currentSrcURL, m_currentDestURL ); // we don't have a slotLinking
                    emit linking( this, m_currentSrcURL.path(), m_currentDestURL );
                }
                else
                {
                    if (observer) observer->slotCopying( this, m_currentSrcURL, m_currentDestURL );
                    emit copying( this, m_currentSrcURL, m_currentDestURL );
                }
            }
            break;

        case STATE_CREATING_DIRS:
            if (observer) observer->slotProcessedDirs( this, m_processedDirs );
            emit processedDirs( this, m_processedDirs );
            if (d->m_bURLDirty)
            {
                d->m_bURLDirty = false;
                emit creatingDir( this, m_currentDestURL );
                if (observer) observer->slotCreatingDir( this, m_currentDestURL);
            }
            break;

        case STATE_STATING:
        case STATE_LISTING:
            if (d->m_bURLDirty)
            {
                d->m_bURLDirty = false;
                if (observer) observer->slotCopying( this, m_currentSrcURL, m_currentDestURL );
            }
            emit totalSize( this, m_totalSize );
            emit totalFiles( this, files.count() );
            emit totalDirs( this, dirs.count() );
            break;

        default:
            break;
    }
}

void CopyJob::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
    UDSEntryList::ConstIterator it = list.begin();
    UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it) {
        const UDSEntry& entry = *it;
        struct CopyInfo info;
        info.permissions = entry.numberValue( KIO::UDS_ACCESS, -1 );
        info.mtime = (time_t) entry.numberValue( KIO::UDS_MODIFICATION_TIME, -1 );
        info.ctime = (time_t) entry.numberValue( KIO::UDS_CREATION_TIME, -1 );
        info.size = (KIO::filesize_t) entry.numberValue( KIO::UDS_SIZE, -1 );
        if ( info.size != (KIO::filesize_t) -1 )
            m_totalSize += info.size;

        // recursive listing, displayName can be a/b/c/d
        const QString displayName = entry.stringValue( KIO::UDS_NAME );
        const QString urlStr = entry.stringValue( KIO::UDS_URL );
        KUrl url;
        if ( !urlStr.isEmpty() )
            url = urlStr;
        QString localPath = entry.stringValue( KIO::UDS_LOCAL_PATH );
        const bool isDir = entry.isDir();
        info.linkDest = entry.stringValue( KIO::UDS_LINK_DEST );

        if (displayName != ".." && displayName != ".")
        {
            bool hasCustomURL = !url.isEmpty() || !localPath.isEmpty();
            if( !hasCustomURL ) {
                // Make URL from displayName
                url = ((SimpleJob *)job)->url();
                if ( m_bCurrentSrcIsDir ) { // Only if src is a directory. Otherwise uSource is fine as is
                    //kDebug(7007) << "adding path " << displayName << endl;
                    url.addPath( displayName );
                }
            }
            //kDebug(7007) << "displayName=" << displayName << " url=" << url << endl;
            if (!localPath.isEmpty() && kio_resolve_local_urls) {
                url = KUrl();
                url.setPath(localPath);
            }

            info.uSource = url;
            info.uDest = m_currentDest;
            //kDebug(7007) << " uSource=" << info.uSource << " uDest(1)=" << info.uDest << endl;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR &&
                 // "copy/move as <foo>" means 'foo' is the dest for the base srcurl
                 // (passed here during stating) but not its children (during listing)
                 ( ! ( m_asMethod && state == STATE_STATING ) ) )
            {
                QString destFileName;
                if ( hasCustomURL &&
                     KProtocolManager::fileNameUsedForCopying( url ) == KProtocolInfo::FromURL ) {
                    //destFileName = url.fileName(); // Doesn't work for recursive listing
                    // Count the number of prefixes used by the recursive listjob
                    int numberOfSlashes = displayName.count( '/' ); // don't make this a find()!
                    QString path = url.path();
                    int pos = 0;
                    for ( int n = 0; n < numberOfSlashes + 1; ++n ) {
                        pos = path.lastIndexOf( '/', pos - 1 );
                        if ( pos == -1 ) { // error
                            kWarning(7007) << "kioslave bug: not enough slashes in UDS_URL " << path << " - looking for " << numberOfSlashes << " slashes" << endl;
                            break;
                        }
                    }
                    if ( pos >= 0 ) {
                        destFileName = path.mid( pos + 1 );
                    }

                } else { // destination filename taken from UDS_NAME
                    destFileName = displayName;
                }

                // Here we _really_ have to add some filename to the dest.
                // Otherwise, we end up with e.g. dest=..../Desktop/ itself.
                // (This can happen when dropping a link to a webpage with no path)
                if ( destFileName.isEmpty() )
                    destFileName = KIO::encodeFileName( info.uSource.prettyUrl() );

                //kDebug(7007) << " adding destFileName=" << destFileName << endl;
                info.uDest.addPath( destFileName );
            }
            //kDebug(7007) << " uDest(2)=" << info.uDest << endl;
            //kDebug(7007) << " " << info.uSource << " -> " << info.uDest << endl;
            if ( info.linkDest.isEmpty() && isDir && m_mode != Link ) // Dir
            {
                dirs.append( info ); // Directories
                if (m_mode == Move)
                    dirsToRemove.append( info.uSource );
            }
            else {
                files.append( info ); // Files and any symlinks
            }
        }
    }
}

void CopyJob::skipSrc()
{
    m_dest = d->m_globalDest;
    destinationState = d->m_globalDestinationState;
    ++m_currentStatSrc;
    skip( m_currentSrcURL );
    statCurrentSrc();
}

void CopyJob::statNextSrc()
{
    /* Revert to the global destination, the one that applies to all source urls.
     * Imagine you copy the items a b and c into /d, but /d/b exists so the user uses "Rename" to put it in /foo/b instead.
     * m_dest is /foo/b for b, but we have to revert to /d for item c and following.
     */
    m_dest = d->m_globalDest;
    destinationState = d->m_globalDestinationState;
    ++m_currentStatSrc;
    statCurrentSrc();
}

void CopyJob::statCurrentSrc()
{
    if ( m_currentStatSrc != m_srcList.end() )
    {
        m_currentSrcURL = (*m_currentStatSrc);
        d->m_bURLDirty = true;
        if ( m_mode == Link )
        {
            // Skip the "stating the source" stage, we don't need it for linking
            m_currentDest = m_dest;
            struct CopyInfo info;
            info.permissions = -1;
            info.mtime = (time_t) -1;
            info.ctime = (time_t) -1;
            info.size = (KIO::filesize_t)-1;
            info.uSource = m_currentSrcURL;
            info.uDest = m_currentDest;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR && !m_asMethod )
            {
                if (
                    (m_currentSrcURL.protocol() == info.uDest.protocol()) &&
                    (m_currentSrcURL.host() == info.uDest.host()) &&
                    (m_currentSrcURL.port() == info.uDest.port()) &&
                    (m_currentSrcURL.user() == info.uDest.user()) &&
                    (m_currentSrcURL.pass() == info.uDest.pass()) )
                {
                    // This is the case of creating a real symlink
                    info.uDest.addPath( m_currentSrcURL.fileName() );
                }
                else
                {
                    // Different protocols, we'll create a .desktop file
                    // We have to change the extension anyway, so while we're at it,
                    // name the file like the URL
                    info.uDest.addPath( KIO::encodeFileName( m_currentSrcURL.prettyUrl() )+".desktop" );
                }
            }
            files.append( info ); // Files and any symlinks
            statNextSrc(); // we could use a loop instead of a recursive call :)
            return;
        }
        else if ( m_mode == Move && (
                // Don't go renaming right away if we need a stat() to find out the destination filename
                KProtocolManager::fileNameUsedForCopying( m_currentSrcURL ) == KProtocolInfo::FromURL ||
                destinationState != DEST_IS_DIR || m_asMethod )
            )
        {
           // If moving, before going for the full stat+[list+]copy+del thing, try to rename
           // The logic is pretty similar to FileCopyJob::slotStart()
           if ( (m_currentSrcURL.protocol() == m_dest.protocol()) &&
              (m_currentSrcURL.host() == m_dest.host()) &&
              (m_currentSrcURL.port() == m_dest.port()) &&
              (m_currentSrcURL.user() == m_dest.user()) &&
              (m_currentSrcURL.pass() == m_dest.pass()) )
           {
              startRenameJob( m_currentSrcURL );
              return;
           }
           else if ( m_currentSrcURL.isLocalFile() && KProtocolManager::canRenameFromFile( m_dest ) )
           {
              startRenameJob( m_dest );
              return;
           }
           else if ( m_dest.isLocalFile() && KProtocolManager::canRenameToFile( m_currentSrcURL ) )
           {
              startRenameJob( m_currentSrcURL );
              return;
           }
        }

        // if the file system doesn't support deleting, we do not even stat
        if (m_mode == Move && !KProtocolManager::supportsDeleting(m_currentSrcURL)) {
            QPointer<CopyJob> that = this;
            emit warning( this, buildErrorString(ERR_CANNOT_DELETE, m_currentSrcURL.prettyUrl()) );
            if (that)
                statNextSrc(); // we could use a loop instead of a recursive call :)
            return;
        }

        // Stat the next src url
        Job * job = KIO::stat( m_currentSrcURL, true, 2, false );
        //kDebug(7007) << "KIO::stat on " << m_currentSrcURL << endl;
        state = STATE_STATING;
        addSubjob(job);
        m_currentDestURL=m_dest;
        m_bOnlyRenames = false;
        d->m_bURLDirty = true;
    }
    else
    {
        // Finished the stat'ing phase
        // First make sure that the totals were correctly emitted
        state = STATE_STATING;
        d->m_bURLDirty = true;
        slotReport();
        if (!dirs.isEmpty())
           emit aboutToCreate( this, dirs );
        if (!files.isEmpty())
           emit aboutToCreate( this, files );
        // Check if we are copying a single file
        m_bSingleFileCopy = ( files.count() == 1 && dirs.isEmpty() );
        // Then start copying things
        state = STATE_CREATING_DIRS;
        createNextDir();
    }
}

void CopyJob::startRenameJob( const KUrl& slave_url )
{
    KUrl dest = m_dest;
    // Append filename or dirname to destination URL, if allowed
    if ( destinationState == DEST_IS_DIR && !m_asMethod )
        dest.addPath( m_currentSrcURL.fileName() );
    kDebug(7007) << "This seems to be a suitable case for trying to rename before stat+[list+]copy+del" << endl;
    state = STATE_RENAMING;

    struct CopyInfo info;
    info.permissions = -1;
    info.mtime = (time_t) -1;
    info.ctime = (time_t) -1;
    info.size = (KIO::filesize_t)-1;
    info.uSource = m_currentSrcURL;
    info.uDest = dest;
    QList<CopyInfo> files;
    files.append(info);
    emit aboutToCreate( this, files );

    KIO_ARGS << m_currentSrcURL << dest << (qint8) false /*no overwrite*/;
    SimpleJob * newJob = new SimpleJob(slave_url, CMD_RENAME, packedArgs, false);
    Scheduler::scheduleJob(newJob);
    addSubjob( newJob );
    if ( m_currentSrcURL.directory() != dest.directory() ) // For the user, moving isn't renaming. Only renaming is.
        m_bOnlyRenames = false;
}

void CopyJob::startListing( const KUrl & src )
{
    state = STATE_LISTING;
    d->m_bURLDirty = true;
    ListJob * newjob = listRecursive( src, false );
    newjob->setUnrestricted(true);
    connect(newjob, SIGNAL(entries( KIO::Job *,
                                    const KIO::UDSEntryList& )),
            SLOT( slotEntries( KIO::Job*,
                               const KIO::UDSEntryList& )));
    addSubjob( newjob );
}

void CopyJob::skip( const KUrl & sourceUrl )
{
    // If this is one if toplevel sources,
    // remove it from m_srcList, for a correct FilesRemoved() signal
    //kDebug(7007) << "CopyJob::skip: looking for " << sourceUrl << endl;
    m_srcList.removeAll( sourceUrl );
    dirsToRemove.removeAll( sourceUrl );
}

bool CopyJob::shouldOverwrite( const QString& path ) const
{
    if ( m_bOverwriteAll )
        return true;
    QStringList::ConstIterator sit = m_overwriteList.begin();
    for( ; sit != m_overwriteList.end(); ++sit )
        if ( path.startsWith( *sit ) )
            return true;
    return false;
}

bool CopyJob::shouldSkip( const QString& path ) const
{
    QStringList::ConstIterator sit = m_skipList.begin();
    for( ; sit != m_skipList.end(); ++sit )
        if ( path.startsWith( *sit ) )
            return true;
    return false;
}

void CopyJob::slotResultCreatingDirs( KJob * job )
{
    // The dir we are trying to create:
    QList<CopyInfo>::Iterator it = dirs.begin();
    // Was there an error creating a dir ?
    if ( job->error() )
    {
        m_conflictError = job->error();
        if ( (m_conflictError == ERR_DIR_ALREADY_EXIST)
             || (m_conflictError == ERR_FILE_ALREADY_EXIST) ) // can't happen?
        {
            KUrl oldURL = ((SimpleJob*)job)->url();
            // Should we skip automatically ?
            if ( m_bAutoSkip ) {
                // We don't want to copy files in this directory, so we put it on the skip list
              m_skipList.append( oldURL.path( KUrl::AddTrailingSlash ) );
                skip( oldURL );
                dirs.erase( it ); // Move on to next dir
            } else {
                // Did the user choose to overwrite already?
                const QString destFile = (*it).uDest.path();
                if ( shouldOverwrite( destFile ) ) { // overwrite => just skip
                    emit copyingDone( this, ( *it ).uSource, ( *it ).uDest, true /* directory */, false /* renamed */ );
                    dirs.erase( it ); // Move on to next dir
                } else {
                    if ( !isInteractive() ) {
                        Job::slotResult( job ); // will set the error and emit result(this)
                        return;
                    }

                    assert( ((SimpleJob*)job)->url().url() == (*it).uDest.url() );
                    removeSubjob( job );
                    assert ( !hasSubjobs() ); // We should have only one job at a time ...

                    // We need to stat the existing dir, to get its last-modification time
                    KUrl existingDest( (*it).uDest );
                    SimpleJob * newJob = KIO::stat( existingDest, false, 2, false );
                    Scheduler::scheduleJob(newJob);
                    kDebug(7007) << "KIO::stat for resolving conflict on " << existingDest << endl;
                    state = STATE_CONFLICT_CREATING_DIRS;
                    addSubjob(newJob);
                    return; // Don't move to next dir yet !
                }
            }
        }
        else
        {
            // Severe error, abort
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
    }
    else // no error : remove from list, to move on to next dir
    {
        //this is required for the undo feature
        emit copyingDone( this, (*it).uSource, (*it).uDest, true, false );
        d->m_directoriesCopied.append( *it );
        dirs.erase( it );
    }

    m_processedDirs++;
    //emit processedDirs( this, m_processedDirs );
    removeSubjob( job );
    assert( !hasSubjobs() ); // We should have only one job at a time ...
    createNextDir();
}

void CopyJob::slotResultConflictCreatingDirs( KJob * job )
{
    // We come here after a conflict has been detected and we've stated the existing dir

    // The dir we were trying to create:
    QList<CopyInfo>::Iterator it = dirs.begin();

    const UDSEntry entry = ((KIO::StatJob*)job)->statResult();

    // Its modification time:
    const time_t destmtime = (time_t) entry.numberValue( KIO::UDS_MODIFICATION_TIME, -1 );
    const time_t destctime = (time_t) entry.numberValue( KIO::UDS_CREATION_TIME, -1 );

    const KIO::filesize_t destsize = entry.numberValue( KIO::UDS_SIZE );
    const QString linkDest = entry.stringValue( KIO::UDS_LINK_DEST );

    removeSubjob( job );
    assert ( !hasSubjobs() ); // We should have only one job at a time ...

    // Always multi and skip (since there are files after that)
    RenameDlg_Mode mode = (RenameDlg_Mode)( M_MULTI | M_SKIP );
    // Overwrite only if the existing thing is a dir (no chance with a file)
    if ( m_conflictError == ERR_DIR_ALREADY_EXIST )
    {
        if( (*it).uSource == (*it).uDest ||
            ((*it).uSource.protocol() == (*it).uDest.protocol() &&
              (*it).uSource.path( KUrl::RemoveTrailingSlash ) == linkDest) )
          mode = (RenameDlg_Mode)( mode | M_OVERWRITE_ITSELF);
        else
          mode = (RenameDlg_Mode)( mode | M_OVERWRITE );
    }

    QString existingDest = (*it).uDest.path();
    QString newPath;
    if (m_reportTimer)
        m_reportTimer->stop();
    RenameDlg_Result r = Observer::self()->open_RenameDlg( this, i18n("Folder Already Exists"),
                                         (*it).uSource.url(),
                                         (*it).uDest.url(),
                                         mode, newPath,
                                         (*it).size, destsize,
                                         (*it).ctime, destctime,
                                         (*it).mtime, destmtime );
    if (m_reportTimer)
        m_reportTimer->start(REPORT_TIMEOUT);
    switch ( r ) {
        case R_CANCEL:
            setError( ERR_USER_CANCELED );
            emitResult();
            return;
        case R_RENAME:
        {
          QString oldPath = (*it).uDest.path( KUrl::AddTrailingSlash );
            KUrl newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            emit renamed( this, (*it).uDest, newUrl ); // for e.g. kpropsdlg

            // Change the current one and strip the trailing '/'
            (*it).uDest.setPath( newUrl.path( KUrl::RemoveTrailingSlash ) );
            newPath = newUrl.path( KUrl::AddTrailingSlash ); // With trailing slash
            QList<CopyInfo>::Iterator renamedirit = it;
            ++renamedirit;
            // Change the name of subdirectories inside the directory
            for( ; renamedirit != dirs.end() ; ++renamedirit )
            {
                QString path = (*renamedirit).uDest.path();
                if ( path.startsWith( oldPath ) ) {
                    QString n = path;
                    n.replace( 0, oldPath.length(), newPath );
                    kDebug(7007) << "dirs list: " << (*renamedirit).uSource.path()
                                  << " was going to be " << path
                                  << ", changed into " << n << endl;
                    (*renamedirit).uDest.setPath( n );
                }
            }
            // Change filenames inside the directory
            QList<CopyInfo>::Iterator renamefileit = files.begin();
            for( ; renamefileit != files.end() ; ++renamefileit )
            {
                QString path = (*renamefileit).uDest.path();
                if ( path.startsWith( oldPath ) ) {
                    QString n = path;
                    n.replace( 0, oldPath.length(), newPath );
                    kDebug(7007) << "files list: " << (*renamefileit).uSource.path()
                                  << " was going to be " << path
                                  << ", changed into " << n << endl;
                    (*renamefileit).uDest.setPath( n );
                }
            }
            if (!dirs.isEmpty())
                emit aboutToCreate( this, dirs );
            if (!files.isEmpty())
                emit aboutToCreate( this, files );
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkip = true;
            // fall through
        case R_SKIP:
            m_skipList.append( existingDest );
            skip( (*it).uSource );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        case R_OVERWRITE:
            m_overwriteList.append( existingDest );
            emit copyingDone( this, ( *it ).uSource, ( *it ).uDest, true /* directory */, false /* renamed */ );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        case R_OVERWRITE_ALL:
            m_bOverwriteAll = true;
            emit copyingDone( this, ( *it ).uSource, ( *it ).uDest, true /* directory */, false /* renamed */ );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        default:
            assert( 0 );
    }
    state = STATE_CREATING_DIRS;
    //emit processedDirs( this, m_processedDirs );
    createNextDir();
}

void CopyJob::createNextDir()
{
    KUrl udir;
    if ( !dirs.isEmpty() )
    {
        // Take first dir to create out of list
        QList<CopyInfo>::Iterator it = dirs.begin();
        // Is this URL on the skip list or the overwrite list ?
        while( it != dirs.end() && udir.isEmpty() )
        {
            const QString dir = (*it).uDest.path();
            if ( shouldSkip( dir ) ) {
                dirs.erase( it );
                it = dirs.begin();
            } else
                udir = (*it).uDest;
        }
    }
    if ( !udir.isEmpty() ) // any dir to create, finally ?
    {
        // Create the directory - with default permissions so that we can put files into it
        // TODO : change permissions once all is finished; but for stuff coming from CDROM it sucks...
        KIO::SimpleJob *newjob = KIO::mkdir( udir, -1 );
        Scheduler::scheduleJob(newjob);

        m_currentDestURL = udir;
        d->m_bURLDirty = true;

        addSubjob(newjob);
        return;
    }
    else // we have finished creating dirs
    {
        emit processedDirs( this, m_processedDirs ); // make sure final number appears
        if (progressId()) Observer::self()->slotProcessedDirs( this, m_processedDirs );

        state = STATE_COPYING_FILES;
        m_processedFiles++; // Ralf wants it to start at 1, not 0
        copyNextFile();
    }
}

void CopyJob::slotResultCopyingFiles( KJob * job )
{
    // The file we were trying to copy:
    QList<CopyInfo>::Iterator it = files.begin();
    if ( job->error() )
    {
        // Should we skip automatically ?
        if ( m_bAutoSkip )
        {
            skip( (*it).uSource );
            m_fileProcessedSize = (*it).size;
            files.erase( it ); // Move on to next file
        }
        else
        {
            if ( !isInteractive() ) {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            m_conflictError = job->error(); // save for later
            // Existing dest ?
            if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
                 || ( m_conflictError == ERR_DIR_ALREADY_EXIST )
                 || ( m_conflictError == ERR_IDENTICAL_FILES ) )
            {
                removeSubjob( job );
                assert ( !hasSubjobs() );
                // We need to stat the existing file, to get its last-modification time
                KUrl existingFile( (*it).uDest );
                SimpleJob * newJob = KIO::stat( existingFile, false, 2, false );
                Scheduler::scheduleJob(newJob);
                kDebug(7007) << "KIO::stat for resolving conflict on " << existingFile << endl;
                state = STATE_CONFLICT_COPYING_FILES;
                addSubjob(newJob);
                return; // Don't move to next file yet !
            }
            else
            {
                if ( m_bCurrentOperationIsLink && qobject_cast<KIO::DeleteJob*>( job ) )
                {
                    // Very special case, see a few lines below
                    // We are deleting the source of a symlink we successfully moved... ignore error
                    m_fileProcessedSize = (*it).size;
                    files.erase( it );
                } else {
                    // Go directly to the conflict resolution, there is nothing to stat
                    slotResultConflictCopyingFiles( job );
                    return;
                }
            }
        }
    } else // no error
    {
        // Special case for moving links. That operation needs two jobs, unlike others.
        if ( m_bCurrentOperationIsLink && m_mode == Move
             && !qobject_cast<KIO::DeleteJob *>( job ) // Deleting source not already done
             )
        {
            removeSubjob( job );
            assert ( !hasSubjobs() );
            // The only problem with this trick is that the error handling for this del operation
            // is not going to be right... see 'Very special case' above.
            KIO::Job * newjob = KIO::del( (*it).uSource, false /*don't shred*/, false /*no GUI*/ );
            addSubjob( newjob );
            return; // Don't move to next file yet !
        }

        if ( m_bCurrentOperationIsLink )
        {
            QString target = ( m_mode == Link ? (*it).uSource.path() : (*it).linkDest );
            //required for the undo feature
            emit copyingLinkDone( this, (*it).uSource, target, (*it).uDest );
        }
        else
            //required for the undo feature
            emit copyingDone( this, (*it).uSource, (*it).uDest, false, false );
        // remove from list, to move on to next file
        files.erase( it );
    }
    m_processedFiles++;

    // clear processed size for last file and add it to overall processed size
    m_processedSize += m_fileProcessedSize;
    m_fileProcessedSize = 0;

    //kDebug(7007) << files.count() << " files remaining" << endl;

    removeSubjob( job, true ); // merge metadata
    assert( !hasSubjobs() ); // We should have only one job at a time ...
    copyNextFile();
}

void CopyJob::slotResultConflictCopyingFiles( KJob * job )
{
    // We come here after a conflict has been detected and we've stated the existing file
    // The file we were trying to create:
    QList<CopyInfo>::Iterator it = files.begin();

    RenameDlg_Result res;
    QString newPath;

    if (m_reportTimer)
        m_reportTimer->stop();

    if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
         || ( m_conflictError == ERR_DIR_ALREADY_EXIST )
         || ( m_conflictError == ERR_IDENTICAL_FILES ) )
    {
        // Its modification time:
        const UDSEntry entry = ((KIO::StatJob*)job)->statResult();

        const time_t destmtime = (time_t) entry.numberValue( KIO::UDS_MODIFICATION_TIME, -1 );
        const time_t destctime = (time_t) entry.numberValue( KIO::UDS_CREATION_TIME, -1 );
        const KIO::filesize_t destsize = entry.numberValue( KIO::UDS_SIZE );
        const QString linkDest = entry.stringValue( KIO::UDS_LINK_DEST );

        // Offer overwrite only if the existing thing is a file
        // If src==dest, use "overwrite-itself"
        RenameDlg_Mode mode;
        bool isDir = true;

        if( m_conflictError == ERR_DIR_ALREADY_EXIST )
            mode = (RenameDlg_Mode) 0;
        else
        {
            if ( (*it).uSource == (*it).uDest  ||
                 ((*it).uSource.protocol() == (*it).uDest.protocol() &&
                   (*it).uSource.path( KUrl::RemoveTrailingSlash ) == linkDest) )
                mode = M_OVERWRITE_ITSELF;
            else
                mode = M_OVERWRITE;
            isDir = false;
        }

        if ( m_bSingleFileCopy )
            mode = (RenameDlg_Mode) ( mode | M_SINGLE );
        else
            mode = (RenameDlg_Mode) ( mode | M_MULTI | M_SKIP );

        res = Observer::self()->open_RenameDlg( this, !isDir ?
                                i18n("File Already Exists") : i18n("Already Exists as Folder"),
                                (*it).uSource.url(),
                                (*it).uDest.url(),
                                mode, newPath,
                              (*it).size, destsize,
                              (*it).ctime, destctime,
                              (*it).mtime, destmtime );

    }
    else
    {
        if ( job->error() == ERR_USER_CANCELED )
            res = R_CANCEL;
        else if ( !isInteractive() ) {
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
        else
        {
            SkipDlg_Result skipResult = Observer::self()->open_SkipDlg( this, files.count() > 1,
                                                                        job->errorString() );

            // Convert the return code from SkipDlg into a RenameDlg code
            res = ( skipResult == S_SKIP ) ? R_SKIP :
                         ( skipResult == S_AUTO_SKIP ) ? R_AUTO_SKIP :
                                        R_CANCEL;
        }
    }

    if (m_reportTimer)
        m_reportTimer->start(REPORT_TIMEOUT);

    removeSubjob( job );
    assert ( !hasSubjobs() );
    switch ( res ) {
        case R_CANCEL:
            setError( ERR_USER_CANCELED );
            emitResult();
            return;
        case R_RENAME:
        {
            KUrl newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            emit renamed( this, (*it).uDest, newUrl ); // for e.g. kpropsdlg
            (*it).uDest = newUrl;

            QList<CopyInfo> files;
            files.append(*it);
            emit aboutToCreate( this, files );
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkip = true;
            // fall through
        case R_SKIP:
            // Move on to next file
            skip( (*it).uSource );
            m_processedSize += (*it).size;
            files.erase( it );
            m_processedFiles++;
            break;
       case R_OVERWRITE_ALL:
            m_bOverwriteAll = true;
            break;
        case R_OVERWRITE:
            // Add to overwrite list, so that copyNextFile knows to overwrite
            m_overwriteList.append( (*it).uDest.path() );
            break;
        default:
            assert( 0 );
    }
    state = STATE_COPYING_FILES;
    //emit processedFiles( this, m_processedFiles );
    copyNextFile();
}

void CopyJob::copyNextFile()
{
    bool bCopyFile = false;
    //kDebug(7007) << "CopyJob::copyNextFile()" << endl;
    // Take the first file in the list
    QList<CopyInfo>::Iterator it = files.begin();
    // Is this URL on the skip list ?
    while (it != files.end() && !bCopyFile)
    {
        const QString destFile = (*it).uDest.path();
        bCopyFile = !shouldSkip( destFile );
        if ( !bCopyFile ) {
            files.erase( it );
            it = files.begin();
        }
    }

    if (bCopyFile) // any file to create, finally ?
    {
        // Do we set overwrite ?
        bool bOverwrite;
        const QString destFile = (*it).uDest.path();
        kDebug(7007) << "copying " << destFile << endl;
        if ( (*it).uDest == (*it).uSource )
            bOverwrite = false;
        else
            bOverwrite = shouldOverwrite( destFile );

        m_bCurrentOperationIsLink = false;
        KIO::Job * newjob = 0L;
        if ( m_mode == Link )
        {
            //kDebug(7007) << "Linking" << endl;
            if (
                ((*it).uSource.protocol() == (*it).uDest.protocol()) &&
                ((*it).uSource.host() == (*it).uDest.host()) &&
                ((*it).uSource.port() == (*it).uDest.port()) &&
                ((*it).uSource.user() == (*it).uDest.user()) &&
                ((*it).uSource.pass() == (*it).uDest.pass()) )
            {
                // This is the case of creating a real symlink
                KIO::SimpleJob *newJob = KIO::symlink( (*it).uSource.path(), (*it).uDest, bOverwrite, false /*no GUI*/ );
                newjob = newJob;
                Scheduler::scheduleJob(newJob);
                //kDebug(7007) << "CopyJob::copyNextFile : Linking target=" << (*it).uSource.path() << " link=" << (*it).uDest << endl;
                //emit linking( this, (*it).uSource.path(), (*it).uDest );
                m_bCurrentOperationIsLink = true;
                m_currentSrcURL=(*it).uSource;
                m_currentDestURL=(*it).uDest;
                d->m_bURLDirty = true;
                //Observer::self()->slotCopying( this, (*it).uSource, (*it).uDest ); // should be slotLinking perhaps
            } else {
                //kDebug(7007) << "CopyJob::copyNextFile : Linking URL=" << (*it).uSource << " link=" << (*it).uDest << endl;
                if ( (*it).uDest.isLocalFile() )
                {
                    // if the source is a devices url, handle it a littlebit special

                    QString path = (*it).uDest.path();
                    //kDebug(7007) << "CopyJob::copyNextFile path=" << path << endl;
                    QFile f( path );
                    if ( f.open( QIODevice::ReadWrite ) )
                    {
                        f.close();
                        KSimpleConfig config( path );
                        config.setDesktopGroup();
                        KUrl url = (*it).uSource;
                        url.setPass( "" );
                        config.writePathEntry( QString::fromLatin1("URL"), url.url() );
                        config.writeEntry( QString::fromLatin1("Name"), url.url() );
                        config.writeEntry( QString::fromLatin1("Type"), QString::fromLatin1("Link") );
                        QString protocol = (*it).uSource.protocol();
                        if ( protocol == QString::fromLatin1("ftp") )
                            config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("ftp") );
                        else if ( protocol == QString::fromLatin1("http") )
                            config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("www") );
                        else if ( protocol == QString::fromLatin1("info") )
                            config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("info") );
                        else if ( protocol == QString::fromLatin1("mailto") )   // sven:
                            config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("kmail") ); // added mailto: support
                        else
                            config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("unknown") );
                        config.sync();
                        files.erase( it );
                        m_processedFiles++;
                        //emit processedFiles( this, m_processedFiles );
                        copyNextFile();
                        return;
                    }
                    else
                    {
                        kDebug(7007) << "CopyJob::copyNextFile ERR_CANNOT_OPEN_FOR_WRITING" << endl;
                        setError( ERR_CANNOT_OPEN_FOR_WRITING );
                        setErrorText( (*it).uDest.path() );
                        emitResult();
                        return;
                    }
                } else {
                    // Todo: not show "link" on remote dirs if the src urls are not from the same protocol+host+...
                    setError( ERR_CANNOT_SYMLINK );
                    setErrorText( (*it).uDest.prettyUrl() );
                    emitResult();
                    return;
                }
            }
        }
        else if ( !(*it).linkDest.isEmpty() &&
                  ((*it).uSource.protocol() == (*it).uDest.protocol()) &&
                  ((*it).uSource.host() == (*it).uDest.host()) &&
                  ((*it).uSource.port() == (*it).uDest.port()) &&
                  ((*it).uSource.user() == (*it).uDest.user()) &&
                  ((*it).uSource.pass() == (*it).uDest.pass()))
            // Copying a symlink - only on the same protocol/host/etc. (#5601, downloading an FTP file through its link),
        {
            KIO::SimpleJob *newJob = KIO::symlink( (*it).linkDest, (*it).uDest, bOverwrite, false /*no GUI*/ );
            Scheduler::scheduleJob(newJob);
            newjob = newJob;
            //kDebug(7007) << "CopyJob::copyNextFile : Linking target=" << (*it).linkDest << " link=" << (*it).uDest << endl;
            //emit linking( this, (*it).linkDest, (*it).uDest );
            m_currentSrcURL = KUrl( (*it).linkDest );
            m_currentDestURL=(*it).uDest;
            d->m_bURLDirty = true;
            //Observer::self()->slotCopying( this, m_currentSrcURL, (*it).uDest ); // should be slotLinking perhaps
            m_bCurrentOperationIsLink = true;
            // NOTE: if we are moving stuff, the deletion of the source will be done in slotResultCopyingFiles
        } else if (m_mode == Move) // Moving a file
        {
            KIO::FileCopyJob * moveJob = KIO::file_move( (*it).uSource, (*it).uDest, (*it).permissions, bOverwrite, false, false/*no GUI*/ );
            moveJob->setSourceSize( (*it).size );
            newjob = moveJob;
            //kDebug(7007) << "CopyJob::copyNextFile : Moving " << (*it).uSource << " to " << (*it).uDest << endl;
            //emit moving( this, (*it).uSource, (*it).uDest );
            m_currentSrcURL=(*it).uSource;
            m_currentDestURL=(*it).uDest;
            d->m_bURLDirty = true;
            //Observer::self()->slotMoving( this, (*it).uSource, (*it).uDest );
        }
        else // Copying a file
        {
            // If source isn't local and target is local, we ignore the original permissions
            // Otherwise, files downloaded from HTTP end up with -r--r--r--
            bool remoteSource = !KProtocolManager::supportsListing((*it).uSource);
            int permissions = (*it).permissions;
            if ( d->m_defaultPermissions || ( remoteSource && (*it).uDest.isLocalFile() ) )
                permissions = -1;
            KIO::FileCopyJob * copyJob = KIO::file_copy( (*it).uSource, (*it).uDest, permissions, bOverwrite, false, false/*no GUI*/ );
            copyJob->setParentJob( this ); // in case of rename dialog
            copyJob->setSourceSize( (*it).size );
            copyJob->setModificationTime( (*it).mtime );
            newjob = copyJob;
            //kDebug(7007) << "CopyJob::copyNextFile : Copying " << (*it).uSource << " to " << (*it).uDest << endl;
            m_currentSrcURL=(*it).uSource;
            m_currentDestURL=(*it).uDest;
            d->m_bURLDirty = true;
        }
        addSubjob(newjob);
        connect( newjob, SIGNAL( processedSize( KJob*, qulonglong ) ),
                 this, SLOT( slotProcessedSize( KJob*, qulonglong ) ) );
        connect( newjob, SIGNAL( totalSize( KJob*, qulonglong ) ),
                 this, SLOT( slotTotalSize( KJob*, qulonglong ) ) );
    }
    else
    {
        // We're done
        //kDebug(7007) << "copyNextFile finished" << endl;
        deleteNextDir();
    }
}

void CopyJob::deleteNextDir()
{
    if ( m_mode == Move && !dirsToRemove.isEmpty() ) // some dirs to delete ?
    {
        state = STATE_DELETING_DIRS;
        d->m_bURLDirty = true;
        // Take first dir to delete out of list - last ones first !
        KUrl::List::Iterator it = --dirsToRemove.end();
        SimpleJob *job = KIO::rmdir( *it );
        Scheduler::scheduleJob(job);
        dirsToRemove.erase(it);
        addSubjob( job );
    }
    else
    {
        // This step is done, move on
        setNextDirAttribute();
    }
}

void CopyJob::setNextDirAttribute()
{
    if ( !d->m_directoriesCopied.isEmpty() )
    {
        state = STATE_SETTING_DIR_ATTRIBUTES;
#ifdef Q_OS_UNIX
        // TODO KDE4: this should use a SlaveBase method, but we have none yet in KDE3.
        QLinkedList<CopyInfo>::Iterator it = d->m_directoriesCopied.begin();
        for ( ; it != d->m_directoriesCopied.end() ; ++it ) {
            const KUrl& url = (*it).uDest;
            if ( url.isLocalFile() && (*it).mtime != (time_t)-1 ) {
                const QByteArray path = QFile::encodeName( url.path() );
                KDE_struct_stat statbuf;
                if (KDE_lstat(path, &statbuf) == 0) {
                    struct utimbuf utbuf;
                    utbuf.actime = statbuf.st_atime; // access time, unchanged
                    utbuf.modtime = (*it).mtime; // modification time
                    utime( path, &utbuf );
                }

            }
        }
#endif
        d->m_directoriesCopied.clear();
    }

    // No "else" here, since the above is a simple sync loop

    {
        // Finished - tell the world
        if ( !m_bOnlyRenames )
        {
            KUrl url( d->m_globalDest );
            if ( d->m_globalDestinationState != DEST_IS_DIR || m_asMethod )
                url.setPath( url.directory() );
            //kDebug(7007) << "KDirNotify'ing FilesAdded " << url << endl;
            org::kde::KDirNotify::emitFilesAdded( url.url() );

            if ( m_mode == Move && !m_srcList.isEmpty() ) {
                //kDebug(7007) << "KDirNotify'ing FilesRemoved " << m_srcList.toStringList() << endl;
                org::kde::KDirNotify::emitFilesRemoved( m_srcList.toStringList() );
            }
        }
        if (m_reportTimer)
            m_reportTimer->stop();
        --m_processedFiles; // undo the "start at 1" hack
        slotReport(); // display final numbers, important if progress dialog stays up

        emitResult();
    }
}

void CopyJob::slotProcessedSize( KJob*, qulonglong data_size )
{
  //kDebug(7007) << "CopyJob::slotProcessedSize " << data_size << endl;
  m_fileProcessedSize = data_size;
  setProcessedSize(m_processedSize + m_fileProcessedSize);

  if ( m_processedSize + m_fileProcessedSize > m_totalSize )
  {
    m_totalSize = m_processedSize + m_fileProcessedSize;
    //kDebug(7007) << "Adjusting m_totalSize to " << m_totalSize << endl;
    emit totalSize( this, m_totalSize ); // safety
  }
  //kDebug(7007) << "emit processedSize " << (unsigned long) (m_processedSize + m_fileProcessedSize) << endl;
  emit processedSize( this, m_processedSize + m_fileProcessedSize );
  emitPercent( m_processedSize + m_fileProcessedSize, m_totalSize );
}

void CopyJob::slotTotalSize( KJob*, qulonglong size )
{
  //kDebug(7007) << "slotTotalSize: " << size << endl;
  // Special case for copying a single file
  // This is because some protocols don't implement stat properly
  // (e.g. HTTP), and don't give us a size in some cases (redirection)
  // so we'd rather rely on the size given for the transfer
  if ( m_bSingleFileCopy && size > m_totalSize)
  {
    //kDebug(7007) << "slotTotalSize: updating totalsize to " << size << endl;
    m_totalSize = size;
    emit totalSize( this, size );
  }
}

void CopyJob::slotResultDeletingDirs( KJob * job )
{
    if (job->error())
    {
        // Couldn't remove directory. Well, perhaps it's not empty
        // because the user pressed Skip for a given file in it.
        // Let's not display "Could not remove dir ..." for each of those dir !
    }
    removeSubjob( job );
    assert ( !hasSubjobs() );
    deleteNextDir();
}

#if 0 // TODO KDE4
void CopyJob::slotResultSettingDirAttributes( KJob * job )
{
    if (job->error())
    {
        // Couldn't set directory attributes. Ignore the error, it can happen
        // with inferior file systems like VFAT.
        // Let's not display warnings for each dir like "cp -a" does.
    }
    subjobs.remove( job );
    assert ( subjobs.isEmpty() );
    setNextDirAttribute();
}
#endif

void CopyJob::slotResultRenaming( KJob* job )
{
    int err = job->error();
    const QString errText = job->errorText();
    removeSubjob( job, true ); // merge metadata
    assert ( !hasSubjobs() );
    // Determine dest again
    KUrl dest = m_dest;
    if ( destinationState == DEST_IS_DIR && !m_asMethod )
        dest.addPath( m_currentSrcURL.fileName() );
    if ( err )
    {
        // Direct renaming didn't work. Try renaming to a temp name,
        // this can help e.g. when renaming 'a' to 'A' on a VFAT partition.
        // In that case it's the _same_ dir, we don't want to copy+del (data loss!)
      if ( m_currentSrcURL.isLocalFile() && m_currentSrcURL.url(KUrl::RemoveTrailingSlash) != dest.url(KUrl::RemoveTrailingSlash) &&
           m_currentSrcURL.url(KUrl::RemoveTrailingSlash).toLower() == dest.url(KUrl::RemoveTrailingSlash).toLower() &&
             ( err == ERR_FILE_ALREADY_EXIST ||
               err == ERR_DIR_ALREADY_EXIST ||
               err == ERR_IDENTICAL_FILES ) )
        {
            kDebug(7007) << "Couldn't rename directly, dest already exists. Detected special case of lower/uppercase renaming in same dir, try with 2 rename calls" << endl;
            QByteArray _src( QFile::encodeName(m_currentSrcURL.path()) );
            QByteArray _dest( QFile::encodeName(dest.path()) );
            KTempFile tmpFile( m_currentSrcURL.directory(KUrl::ObeyTrailingSlash) );
            QByteArray _tmp( QFile::encodeName(tmpFile.name()) );
            kDebug(7007) << "CopyJob::slotResult KTempFile status:" << tmpFile.status() << " using " << _tmp << " as intermediary" << endl;
            tmpFile.unlink();
            if ( ::rename( _src, _tmp ) == 0 )
            {
                if ( !QFile::exists( _dest ) && ::rename( _tmp, _dest ) == 0 )
                {
                    kDebug(7007) << "Success." << endl;
                    err = 0;
                }
                else
                {
                    // Revert back to original name!
                    if ( ::rename( _tmp, _src ) != 0 ) {
                        kError(7007) << "Couldn't rename " << tmpFile.name() << " back to " << _src << " !" << endl;
                        // Severe error, abort
                        Job::slotResult( job ); // will set the error and emit result(this)
                        return;
                    }
                }
            }
        }
    }
    if ( err )
    {
        // This code is similar to CopyJob::slotResultConflictCopyingFiles
        // but here it's about the base src url being moved/renamed
        // (*m_currentStatSrc) and its dest (m_dest), not about a single file.
        // It also means we already stated the dest, here.
        // On the other hand we haven't stated the src yet (we skipped doing it
        // to save time, since it's not necessary to rename directly!)...

        Q_ASSERT( m_currentSrcURL == *m_currentStatSrc );

        // Existing dest?
        if ( ( err == ERR_DIR_ALREADY_EXIST ||
               err == ERR_FILE_ALREADY_EXIST ||
               err == ERR_IDENTICAL_FILES )
             && isInteractive() )
        {
            if (m_reportTimer)
                m_reportTimer->stop();

            // Should we skip automatically ?
            if ( m_bAutoSkip ) {
                // Move on to next file
                skipSrc();
                return;
            } else if ( m_bOverwriteAll ) {
                ; // nothing to do, stat+copy+del will overwrite
            } else {
                QString newPath;
                // If src==dest, use "overwrite-itself"
                RenameDlg_Mode mode = (RenameDlg_Mode)
                                      ( ( m_currentSrcURL == dest ) ? M_OVERWRITE_ITSELF : M_OVERWRITE );

                if ( m_srcList.count() > 1 )
                    mode = (RenameDlg_Mode) ( mode | M_MULTI | M_SKIP );
                else
                    mode = (RenameDlg_Mode) ( mode | M_SINGLE );

                // we lack mtime info for both the src (not stated)
                // and the dest (stated but this info wasn't stored)
                // Let's do it for local files, at least
                KIO::filesize_t sizeSrc = (KIO::filesize_t) -1;
                KIO::filesize_t sizeDest = (KIO::filesize_t) -1;
                time_t ctimeSrc = (time_t) -1;
                time_t ctimeDest = (time_t) -1;
                time_t mtimeSrc = (time_t) -1;
                time_t mtimeDest = (time_t) -1;

                KDE_struct_stat stat_buf;
                if ( m_currentSrcURL.isLocalFile() &&
                    KDE_stat(QFile::encodeName(m_currentSrcURL.path()), &stat_buf) == 0 ) {
                    sizeSrc = stat_buf.st_size;
                    ctimeSrc = stat_buf.st_ctime;
                    mtimeSrc = stat_buf.st_mtime;
                }
                if ( dest.isLocalFile() &&
                    KDE_stat(QFile::encodeName(dest.path()), &stat_buf) == 0 ) {
                    sizeDest = stat_buf.st_size;
                    ctimeDest = stat_buf.st_ctime;
                    mtimeDest = stat_buf.st_mtime;
                }

                RenameDlg_Result r = Observer::self()->open_RenameDlg(
                    this,
                    err != ERR_DIR_ALREADY_EXIST ? i18n("File Already Exists") : i18n("Already Exists as Folder"),
                    m_currentSrcURL.url(),
                    dest.url(),
                    mode, newPath,
                    sizeSrc, sizeDest,
                    ctimeSrc, ctimeDest,
                    mtimeSrc, mtimeDest );
                if (m_reportTimer)
                    m_reportTimer->start(REPORT_TIMEOUT);

                switch ( r )
                {
                case R_CANCEL:
                {
                    setError( ERR_USER_CANCELED );
                    emitResult();
                    return;
                }
                case R_RENAME:
                {
                    // Set m_dest to the chosen destination
                    // This is only for this src url; the next one will revert to d->m_globalDest
                    m_dest.setPath( newPath );
                    KIO::Job* job = KIO::stat( m_dest, false, 2, false );
                    state = STATE_STATING;
                    destinationState = DEST_NOT_STATED;
                    addSubjob(job);
                    return;
                }
                case R_AUTO_SKIP:
                    m_bAutoSkip = true;
                    // fall through
                case R_SKIP:
                    // Move on to next file
                    skipSrc();
                    return;
                case R_OVERWRITE_ALL:
                    m_bOverwriteAll = true;
                    break;
                case R_OVERWRITE:
                    // Add to overwrite list
                    // Note that we add dest, not m_dest.
                    // This ensures that when moving several urls into a dir (m_dest),
                    // we only overwrite for the current one, not for all.
                    // When renaming a single file (m_asMethod), it makes no difference.
                    kDebug(7007) << "adding to overwrite list: " << dest.path() << endl;
                    m_overwriteList.append( dest.path() );
                    break;
                default:
                    //assert( 0 );
                    break;
                }
            }
        } else if ( err != KIO::ERR_UNSUPPORTED_ACTION ) {
            kDebug(7007) << "Couldn't rename " << m_currentSrcURL << " to " << dest << ", aborting" << endl;
            setError( err );
            setErrorText( errText );
            emitResult();
            return;
        }
        kDebug(7007) << "Couldn't rename " << m_currentSrcURL << " to " << dest << ", reverting to normal way, starting with stat" << endl;
        //kDebug(7007) << "KIO::stat on " << m_currentSrcURL << endl;
        KIO::Job* job = KIO::stat( m_currentSrcURL, true, 2, false );
        state = STATE_STATING;
        addSubjob(job);
        m_bOnlyRenames = false;
    }
    else
    {
        //kDebug(7007) << "Renaming succeeded, move on" << endl;
        emit copyingDone( this, *m_currentStatSrc, dest, true, true );
        statNextSrc();
    }
}

void CopyJob::slotResult( KJob *job )
{
    //kDebug(7007) << "CopyJob::slotResult() state=" << (int) state << endl;
    // In each case, what we have to do is :
    // 1 - check for errors and treat them
    // 2 - removeSubjob(job);
    // 3 - decide what to do next

    switch ( state ) {
        case STATE_STATING: // We were trying to stat a src url or the dest
            slotResultStating( job );
            break;
        case STATE_RENAMING: // We were trying to do a direct renaming, before even stat'ing
        {
            slotResultRenaming( job );
            break;
        }
        case STATE_LISTING: // recursive listing finished
            //kDebug(7007) << "totalSize: " << (unsigned int) m_totalSize << " files: " << files.count() << " dirs: " << dirs.count() << endl;
            // Was there an error ?
            if (job->error())
            {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            removeSubjob( job );
            assert ( !hasSubjobs() );

            statNextSrc();
            break;
        case STATE_CREATING_DIRS:
            slotResultCreatingDirs( job );
            break;
        case STATE_CONFLICT_CREATING_DIRS:
            slotResultConflictCreatingDirs( job );
            break;
        case STATE_COPYING_FILES:
            slotResultCopyingFiles( job );
            break;
        case STATE_CONFLICT_COPYING_FILES:
            slotResultConflictCopyingFiles( job );
            break;
        case STATE_DELETING_DIRS:
            slotResultDeletingDirs( job );
            break;
        case STATE_SETTING_DIR_ATTRIBUTES: // TODO KDE4
            assert( 0 );
            //slotResultSettingDirAttributes( job );
            break;
        default:
            assert( 0 );
    }
}

void KIO::CopyJob::setDefaultPermissions( bool b )
{
    d->m_defaultPermissions = b;
}

CopyJob *KIO::copy(const KUrl& src, const KUrl& dest, bool showProgressInfo )
{
    //kDebug(7007) << "KIO::copy src=" << src << " dest=" << dest << endl;
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Copy, false, showProgressInfo );
}

CopyJob *KIO::copyAs(const KUrl& src, const KUrl& dest, bool showProgressInfo )
{
    //kDebug(7007) << "KIO::copyAs src=" << src << " dest=" << dest << endl;
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Copy, true, showProgressInfo );
}

CopyJob *KIO::copy( const KUrl::List& src, const KUrl& dest, bool showProgressInfo )
{
    //kdDebug(7007) << src << " " << dest << endl;
    return new CopyJob( src, dest, CopyJob::Copy, false, showProgressInfo );
}

CopyJob *KIO::move(const KUrl& src, const KUrl& dest, bool showProgressInfo )
{
    //kDebug(7007) << src << " " << dest << endl;
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Move, false, showProgressInfo );
}

CopyJob *KIO::moveAs(const KUrl& src, const KUrl& dest, bool showProgressInfo )
{
    //kDebug(7007) << src << " " << dest << endl;
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Move, true, showProgressInfo );
}

CopyJob *KIO::move( const KUrl::List& src, const KUrl& dest, bool showProgressInfo )
{
    //kdDebug(7007) << src << " " << dest << endl;
    return new CopyJob( src, dest, CopyJob::Move, false, showProgressInfo );
}

CopyJob *KIO::link(const KUrl& src, const KUrl& destDir, bool showProgressInfo )
{
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, destDir, CopyJob::Link, false, showProgressInfo );
}

CopyJob *KIO::link(const KUrl::List& srcList, const KUrl& destDir, bool showProgressInfo )
{
    return new CopyJob( srcList, destDir, CopyJob::Link, false, showProgressInfo );
}

CopyJob *KIO::linkAs(const KUrl& src, const KUrl& destDir, bool showProgressInfo )
{
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, destDir, CopyJob::Link, false, showProgressInfo );
}

CopyJob *KIO::trash(const KUrl& src, bool showProgressInfo )
{
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, KUrl( "trash:/" ), CopyJob::Move, false, showProgressInfo );
}

CopyJob *KIO::trash(const KUrl::List& srcList, bool showProgressInfo )
{
    return new CopyJob( srcList, KUrl( "trash:/" ), CopyJob::Move, false, showProgressInfo );
}

#include "copyjob.moc"
