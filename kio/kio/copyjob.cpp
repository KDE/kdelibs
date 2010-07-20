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
#include <errno.h>
#include "kdirlister.h"
#include "kfileitem.h"
#include "deletejob.h"

#include <klocale.h>
#include <kdesktopfile.h>
#include <kdebug.h>
#include <kde_file.h>

#include "slave.h"
#include "scheduler.h"
#include "kdirwatch.h"
#include "kprotocolmanager.h"

#include "jobuidelegate.h"

#include <kdirnotify.h>
#include <ktemporaryfile.h>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif
#include <assert.h>

#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <sys/stat.h> // mode_t
#include <QPointer>

#include "job_p.h"

using namespace KIO;

//this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
#define REPORT_TIMEOUT 200

enum DestinationState {
    DEST_NOT_STATED,
    DEST_IS_DIR,
    DEST_IS_FILE,
    DEST_DOESNT_EXIST
};

/**
 * States:
 *     STATE_STATING for the dest
 *     STATE_STATING for each src url (statNextSrc)
 *          for each: if dir -> STATE_LISTING (filling 'd->dirs' and 'd->files')
 *          but if direct rename possible: STATE_RENAMING instead.
 *     STATE_CREATING_DIRS (createNextDir, iterating over 'd->dirs')
 *          if conflict: STATE_CONFLICT_CREATING_DIRS
 *     STATE_COPYING_FILES (copyNextFile, iterating over 'd->files')
 *          if conflict: STATE_CONFLICT_COPYING_FILES
 *     STATE_DELETING_DIRS (deleteNextDir) (if moving)
 *     STATE_SETTING_DIR_ATTRIBUTES (setNextDirAttribute, iterating over d->m_directoriesCopied)
 *     done.
 */
enum CopyJobState {
    STATE_STATING,
    STATE_RENAMING,
    STATE_LISTING,
    STATE_CREATING_DIRS,
    STATE_CONFLICT_CREATING_DIRS,
    STATE_COPYING_FILES,
    STATE_CONFLICT_COPYING_FILES,
    STATE_DELETING_DIRS,
    STATE_SETTING_DIR_ATTRIBUTES
};

/** @internal */
class KIO::CopyJobPrivate: public KIO::JobPrivate
{
public:
    CopyJobPrivate(const KUrl::List& src, const KUrl& dest,
                   CopyJob::CopyMode mode, bool asMethod)
        : m_globalDest(dest)
        , m_globalDestinationState(DEST_NOT_STATED)
        , m_defaultPermissions(false)
        , m_bURLDirty(false)
        , m_mode(mode)
        , m_asMethod(asMethod)
        , destinationState(DEST_NOT_STATED)
        , state(STATE_STATING)
        , m_totalSize(0)
        , m_processedSize(0)
        , m_fileProcessedSize(0)
        , m_processedFiles(0)
        , m_processedDirs(0)
        , m_srcList(src)
        , m_currentStatSrc(m_srcList.constBegin())
        , m_bCurrentOperationIsLink(false)
        , m_bSingleFileCopy(false)
        , m_bOnlyRenames(mode==CopyJob::Move)
        , m_dest(dest)
        , m_bAutoRenameFiles(false)
        , m_bAutoRenameDirs(false)
        , m_bAutoSkipFiles( false )
        , m_bAutoSkipDirs( false )
        , m_bOverwriteAllFiles( false )
        , m_bOverwriteAllDirs( false )
        , m_conflictError(0)
        , m_reportTimer(0)
    {
    }

    // This is the dest URL that was initially given to CopyJob
    // It is copied into m_dest, which can be changed for a given src URL
    // (when using the RENAME dialog in slotResult),
    // and which will be reset for the next src URL.
    KUrl m_globalDest;
    // The state info about that global dest
    DestinationState m_globalDestinationState;
    // See setDefaultPermissions
    bool m_defaultPermissions;
    // Whether URLs changed (and need to be emitted by the next slotReport call)
    bool m_bURLDirty;
    // Used after copying all the files into the dirs, to set mtime (TODO: and permissions?)
    // after the copy is done
    QLinkedList<CopyInfo> m_directoriesCopied;
    QLinkedList<CopyInfo>::const_iterator m_directoriesCopiedIterator;

    CopyJob::CopyMode m_mode;
    bool m_asMethod;
    DestinationState destinationState;
    CopyJobState state;
    KIO::filesize_t m_totalSize;
    KIO::filesize_t m_processedSize;
    KIO::filesize_t m_fileProcessedSize;
    int m_processedFiles;
    int m_processedDirs;
    QList<CopyInfo> files;
    QList<CopyInfo> dirs;
    KUrl::List dirsToRemove;
    KUrl::List m_srcList;
    KUrl::List m_successSrcList; // Entries in m_srcList that have successfully been moved
    KUrl::List::const_iterator m_currentStatSrc;
    bool m_bCurrentSrcIsDir;
    bool m_bCurrentOperationIsLink;
    bool m_bSingleFileCopy;
    bool m_bOnlyRenames;
    KUrl m_dest;
    KUrl m_currentDest; // set during listing, used by slotEntries
    //
    QStringList m_skipList;
    QStringList m_overwriteList;
    bool m_bAutoRenameFiles;
    bool m_bAutoRenameDirs;
    bool m_bAutoSkipFiles;
    bool m_bAutoSkipDirs;
    bool m_bOverwriteAllFiles;
    bool m_bOverwriteAllDirs;
    int m_conflictError;

    QTimer *m_reportTimer;

    // The current src url being stat'ed or copied
    // During the stat phase, this is initially equal to *m_currentStatSrc but it can be resolved to a local file equivalent (#188903).
    KUrl m_currentSrcURL;
    KUrl m_currentDestURL;

    QSet<QString> m_parentDirs;

    void statCurrentSrc();
    void statNextSrc();

    // Those aren't slots but submethods for slotResult.
    void slotResultStating( KJob * job );
    void startListing( const KUrl & src );
    void slotResultCreatingDirs( KJob * job );
    void slotResultConflictCreatingDirs( KJob * job );
    void createNextDir();
    void slotResultCopyingFiles( KJob * job );
    void slotResultConflictCopyingFiles( KJob * job );
//     KIO::Job* linkNextFile( const KUrl& uSource, const KUrl& uDest, bool overwrite );
    KIO::Job* linkNextFile( const KUrl& uSource, const KUrl& uDest, JobFlags flags );
    void copyNextFile();
    void slotResultDeletingDirs( KJob * job );
    void deleteNextDir();
    void sourceStated(const UDSEntry& entry, const KUrl& sourceUrl);
    void skip(const KUrl & sourceURL, bool isDir);
    void slotResultRenaming( KJob * job );
    void slotResultSettingDirAttributes( KJob * job );
    void setNextDirAttribute();

    void startRenameJob(const KUrl &slave_url);
    bool shouldOverwriteDir( const QString& path ) const;
    bool shouldOverwriteFile( const QString& path ) const;
    bool shouldSkip( const QString& path ) const;
    void skipSrc(bool isDir);

    void slotStart();
    void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
    void addCopyInfoFromUDSEntry(const UDSEntry& entry, const KUrl& srcUrl, bool srcIsDir, const KUrl& currentDest);
    /**
     * Forward signal from subjob
     */
    void slotProcessedSize( KJob*, qulonglong data_size );
    /**
     * Forward signal from subjob
     * @param size the total size
     */
    void slotTotalSize( KJob*, qulonglong size );

    void slotReport();

    Q_DECLARE_PUBLIC(CopyJob)

    static inline CopyJob *newJob(const KUrl::List& src, const KUrl& dest,
                                  CopyJob::CopyMode mode, bool asMethod, JobFlags flags)
    {
        CopyJob *job = new CopyJob(*new CopyJobPrivate(src,dest,mode,asMethod));
        job->setUiDelegate(new JobUiDelegate);
        if (!(flags & HideProgressInfo))
            KIO::getJobTracker()->registerJob(job);
        if (flags & KIO::Overwrite) {
            job->d_func()->m_bOverwriteAllDirs = true;
            job->d_func()->m_bOverwriteAllFiles = true;
        }
        return job;
    }
};

CopyJob::CopyJob(CopyJobPrivate &dd)
    : Job(dd)
{
    setProperty("destUrl", d_func()->m_dest.url());
    QTimer::singleShot(0, this, SLOT(slotStart()));
}

CopyJob::~CopyJob()
{
}

KUrl::List CopyJob::srcUrls() const
{
    return d_func()->m_srcList;
}

KUrl CopyJob::destUrl() const
{
    return d_func()->m_dest;
}

void CopyJobPrivate::slotStart()
{
    Q_Q(CopyJob);
    /**
       We call the functions directly instead of using signals.
       Calling a function via a signal takes approx. 65 times the time
       compared to calling it directly (at least on my machine). aleXXX
    */
    m_reportTimer = new QTimer(q);

    q->connect(m_reportTimer,SIGNAL(timeout()),q,SLOT(slotReport()));
    m_reportTimer->start(REPORT_TIMEOUT);

    // Stat the dest
    KIO::Job * job = KIO::stat( m_dest, StatJob::DestinationSide, 2, KIO::HideProgressInfo );
    //kDebug(7007) << "CopyJob:stating the dest " << m_dest;
    q->addSubjob(job);
}

// For unit test purposes
KIO_EXPORT bool kio_resolve_local_urls = true;

void CopyJobPrivate::slotResultStating( KJob *job )
{
    Q_Q(CopyJob);
    //kDebug(7007);
    // Was there an error while stating the src ?
    if (job->error() && destinationState != DEST_NOT_STATED )
    {
        const KUrl srcurl = static_cast<SimpleJob*>(job)->url();
        if ( !srcurl.isLocalFile() )
        {
            // Probably : src doesn't exist. Well, over some protocols (e.g. FTP)
            // this info isn't really reliable (thanks to MS FTP servers).
            // We'll assume a file, and try to download anyway.
            kDebug(7007) << "Error while stating source. Activating hack";
            q->removeSubjob( job );
            assert ( !q->hasSubjobs() ); // We should have only one job at a time ...
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
        // yes, q->Job::, because we don't want to call our override
        q->Job::slotResult( job ); // will set the error and emit result(this)
        return;
    }

    // Keep copy of the stat result
    const UDSEntry entry = static_cast<StatJob*>(job)->statResult();

    if ( destinationState == DEST_NOT_STATED ) {
        const bool isGlobalDest = m_dest == m_globalDest;
        const bool isDir = entry.isDir();
        // we were stating the dest
        if (job->error()) {
            destinationState = DEST_DOESNT_EXIST;
            //kDebug(7007) << "dest does not exist";
        } else {
            // Treat symlinks to dirs as dirs here, so no test on isLink
            destinationState = isDir ? DEST_IS_DIR : DEST_IS_FILE;
            //kDebug(7007) << "dest is dir:" << isDir;

            const QString sLocalPath = entry.stringValue( KIO::UDSEntry::UDS_LOCAL_PATH );
            if ( !sLocalPath.isEmpty() && kio_resolve_local_urls && destinationState != DEST_DOESNT_EXIST ) {
                m_dest = KUrl();
                m_dest.setPath(sLocalPath);
                if ( isGlobalDest )
                    m_globalDest = m_dest;
            }
        }
        if ( isGlobalDest )
            m_globalDestinationState = destinationState;

        q->removeSubjob( job );
        assert ( !q->hasSubjobs() );

        // After knowing what the dest is, we can start stat'ing the first src.
        statCurrentSrc();
    } else {
        sourceStated(entry, static_cast<SimpleJob*>(job)->url());
        q->removeSubjob( job );
    }
}

void CopyJobPrivate::sourceStated(const UDSEntry& entry, const KUrl& sourceUrl)
{
    const QString sLocalPath = entry.stringValue( KIO::UDSEntry::UDS_LOCAL_PATH );
    const bool isDir = entry.isDir();

    // We were stating the current source URL
    // Is it a file or a dir ?

    // There 6 cases, and all end up calling addCopyInfoFromUDSEntry first :
    // 1 - src is a dir, destination is a directory,
    // slotEntries will append the source-dir-name to the destination
    // 2 - src is a dir, destination is a file -- will offer to overwrite, later on.
    // 3 - src is a dir, destination doesn't exist, then it's the destination dirname,
    // so slotEntries will use it as destination.

    // 4 - src is a file, destination is a directory,
    // slotEntries will append the filename to the destination.
    // 5 - src is a file, destination is a file, m_dest is the exact destination name
    // 6 - src is a file, destination doesn't exist, m_dest is the exact destination name

    KUrl srcurl;
    if (!sLocalPath.isEmpty() && destinationState != DEST_DOESNT_EXIST) {
        kDebug() << "Using sLocalPath. destinationState=" << destinationState;
        // Prefer the local path -- but only if we were able to stat() the dest.
        // Otherwise, renaming a desktop:/ url would copy from src=file to dest=desktop (#218719)
        srcurl.setPath(sLocalPath);
    } else {
        srcurl = sourceUrl;
    }
    addCopyInfoFromUDSEntry(entry, srcurl, false, m_dest);

    m_currentDest = m_dest;
    m_bCurrentSrcIsDir = false;

    if ( isDir
         // treat symlinks as files (no recursion)
         && !entry.isLink()
         && m_mode != CopyJob::Link ) // No recursion in Link mode either.
    {
        //kDebug(7007) << "Source is a directory";

        if (srcurl.isLocalFile()) {
            const QString parentDir = srcurl.toLocalFile(KUrl::RemoveTrailingSlash);
            m_parentDirs.insert(parentDir);
        }

        m_bCurrentSrcIsDir = true; // used by slotEntries
        if ( destinationState == DEST_IS_DIR ) // (case 1)
        {
            if ( !m_asMethod )
            {
                // Use <desturl>/<directory_copied> as destination, from now on
                QString directory = srcurl.fileName();
                const QString sName = entry.stringValue( KIO::UDSEntry::UDS_NAME );
                if (!sName.isEmpty() && KProtocolManager::fileNameUsedForCopying(srcurl) == KProtocolInfo::Name) {
                    directory = sName;
                }
                m_currentDest.addPath( directory );
            }
        }
        else // (case 3)
        {
            // otherwise dest is new name for toplevel dir
            // so the destination exists, in fact, from now on.
            // (This even works with other src urls in the list, since the
            //  dir has effectively been created)
            destinationState = DEST_IS_DIR;
            if ( m_dest == m_globalDest )
                m_globalDestinationState = destinationState;
        }

        startListing( srcurl );
    }
    else
    {
        //kDebug(7007) << "Source is a file (or a symlink), or we are linking -> no recursive listing";

        if (srcurl.isLocalFile()) {
            const QString parentDir = srcurl.directory(KUrl::ObeyTrailingSlash);
            m_parentDirs.insert(parentDir);
        }

        statNextSrc();
    }
}

bool CopyJob::doSuspend()
{
    Q_D(CopyJob);
    d->slotReport();
    return Job::doSuspend();
}

void CopyJobPrivate::slotReport()
{
    Q_Q(CopyJob);
    if ( q->isSuspended() )
        return;
    // If showProgressInfo was set, progressId() is > 0.
    switch (state) {
        case STATE_RENAMING:
            q->setTotalAmount(KJob::Files, m_srcList.count());
            // fall-through intended
        case STATE_COPYING_FILES:
            q->setProcessedAmount( KJob::Files, m_processedFiles );
            if (m_bURLDirty)
            {
                // Only emit urls when they changed. This saves time, and fixes #66281
                m_bURLDirty = false;
                if (m_mode==CopyJob::Move)
                {
                    emitMoving(q, m_currentSrcURL, m_currentDestURL);
                    emit q->moving( q, m_currentSrcURL, m_currentDestURL);
                }
                else if (m_mode==CopyJob::Link)
                {
                    emitCopying( q, m_currentSrcURL, m_currentDestURL ); // we don't have a delegate->linking
                    emit q->linking( q, m_currentSrcURL.path(), m_currentDestURL );
                }
                else
                {
                    emitCopying( q, m_currentSrcURL, m_currentDestURL );
                    emit q->copying( q, m_currentSrcURL, m_currentDestURL );
                }
            }
            break;

        case STATE_CREATING_DIRS:
            q->setProcessedAmount( KJob::Directories, m_processedDirs );
            if (m_bURLDirty)
            {
                m_bURLDirty = false;
                emit q->creatingDir( q, m_currentDestURL );
                emitCreatingDir( q, m_currentDestURL );
            }
            break;

        case STATE_STATING:
        case STATE_LISTING:
            if (m_bURLDirty)
            {
                m_bURLDirty = false;
                if (m_mode==CopyJob::Move)
                {
                    emitMoving( q, m_currentSrcURL, m_currentDestURL );
                }
                else
                {
                    emitCopying( q, m_currentSrcURL, m_currentDestURL );
                }
            }
            q->setTotalAmount(KJob::Bytes, m_totalSize);
            q->setTotalAmount(KJob::Files, files.count());
            q->setTotalAmount(KJob::Directories, dirs.count());
            break;

        default:
            break;
    }
}

void CopyJobPrivate::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
    //Q_Q(CopyJob);
    UDSEntryList::ConstIterator it = list.constBegin();
    UDSEntryList::ConstIterator end = list.constEnd();
    for (; it != end; ++it) {
        const UDSEntry& entry = *it;
        addCopyInfoFromUDSEntry(entry, static_cast<SimpleJob *>(job)->url(), m_bCurrentSrcIsDir, m_currentDest);
    }
}

void CopyJobPrivate::addCopyInfoFromUDSEntry(const UDSEntry& entry, const KUrl& srcUrl, bool srcIsDir, const KUrl& currentDest)
{
    struct CopyInfo info;
    info.permissions = entry.numberValue(KIO::UDSEntry::UDS_ACCESS, -1);
    info.mtime = (time_t) entry.numberValue(KIO::UDSEntry::UDS_MODIFICATION_TIME, -1);
    info.ctime = (time_t) entry.numberValue(KIO::UDSEntry::UDS_CREATION_TIME, -1);
    info.size = (KIO::filesize_t) entry.numberValue(KIO::UDSEntry::UDS_SIZE, -1);
    if (info.size != (KIO::filesize_t) -1)
        m_totalSize += info.size;

    // recursive listing, displayName can be a/b/c/d
    const QString displayName = entry.stringValue(KIO::UDSEntry::UDS_NAME);
    const QString urlStr = entry.stringValue(KIO::UDSEntry::UDS_URL);
    KUrl url;
    if (!urlStr.isEmpty())
        url = urlStr;
    QString localPath = entry.stringValue(KIO::UDSEntry::UDS_LOCAL_PATH);
    const bool isDir = entry.isDir();
    info.linkDest = entry.stringValue(KIO::UDSEntry::UDS_LINK_DEST);

    if (displayName != QLatin1String("..") && displayName != QLatin1String(".")) {
        const bool hasCustomURL = !url.isEmpty() || !localPath.isEmpty();
        if (!hasCustomURL) {
            // Make URL from displayName
            url = srcUrl;
            if (srcIsDir) { // Only if src is a directory. Otherwise uSource is fine as is
                //kDebug(7007) << "adding path" << displayName;
                url.addPath(displayName);
            }
        }
        //kDebug(7007) << "displayName=" << displayName << "url=" << url;
        if (!localPath.isEmpty() && kio_resolve_local_urls && destinationState != DEST_DOESNT_EXIST) {
            url = KUrl(localPath);
        }

        info.uSource = url;
        info.uDest = currentDest;
        //kDebug(7007) << "uSource=" << info.uSource << "uDest(1)=" << info.uDest;
        // Append filename or dirname to destination URL, if allowed
        if (destinationState == DEST_IS_DIR &&
             // "copy/move as <foo>" means 'foo' is the dest for the base srcurl
             // (passed here during stating) but not its children (during listing)
             (! (m_asMethod && state == STATE_STATING)))
        {
            QString destFileName;
            if (hasCustomURL &&
                 KProtocolManager::fileNameUsedForCopying(url) == KProtocolInfo::FromUrl) {
                //destFileName = url.fileName(); // Doesn't work for recursive listing
                // Count the number of prefixes used by the recursive listjob
                int numberOfSlashes = displayName.count('/'); // don't make this a find()!
                QString path = url.path();
                int pos = 0;
                for (int n = 0; n < numberOfSlashes + 1; ++n) {
                    pos = path.lastIndexOf('/', pos - 1);
                    if (pos == -1) { // error
                        kWarning(7007) << "kioslave bug: not enough slashes in UDS_URL" << path << "- looking for" << numberOfSlashes << "slashes";
                        break;
                    }
                }
                if (pos >= 0) {
                    destFileName = path.mid(pos + 1);
                }

            } else { // destination filename taken from UDS_NAME
                destFileName = displayName;
            }

            // Here we _really_ have to add some filename to the dest.
            // Otherwise, we end up with e.g. dest=..../Desktop/ itself.
            // (This can happen when dropping a link to a webpage with no path)
            if (destFileName.isEmpty()) {
                destFileName = KIO::encodeFileName(info.uSource.prettyUrl());
            }

            //kDebug(7007) << " adding destFileName=" << destFileName;
            info.uDest.addPath(destFileName);
        }
        //kDebug(7007) << " uDest(2)=" << info.uDest;
        //kDebug(7007) << " " << info.uSource << "->" << info.uDest;
        if (info.linkDest.isEmpty() && isDir && m_mode != CopyJob::Link) { // Dir
            dirs.append(info); // Directories
            if (m_mode == CopyJob::Move) {
                dirsToRemove.append(info.uSource);
            }
        } else {
            files.append(info); // Files and any symlinks
        }
    }
}

void CopyJobPrivate::skipSrc(bool isDir)
{
    m_dest = m_globalDest;
    destinationState = m_globalDestinationState;
    skip(*m_currentStatSrc, isDir);
    ++m_currentStatSrc;
    statCurrentSrc();
}

void CopyJobPrivate::statNextSrc()
{
    /* Revert to the global destination, the one that applies to all source urls.
     * Imagine you copy the items a b and c into /d, but /d/b exists so the user uses "Rename" to put it in /foo/b instead.
     * d->m_dest is /foo/b for b, but we have to revert to /d for item c and following.
     */
    m_dest = m_globalDest;
    destinationState = m_globalDestinationState;
    ++m_currentStatSrc;
    statCurrentSrc();
}

void CopyJobPrivate::statCurrentSrc()
{
    Q_Q(CopyJob);
    if (m_currentStatSrc != m_srcList.constEnd()) {
        m_currentSrcURL = (*m_currentStatSrc);
        m_bURLDirty = true;
        if (m_mode == CopyJob::Link) {
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
            if (destinationState == DEST_IS_DIR && !m_asMethod) {
                if (
                    (m_currentSrcURL.protocol() == info.uDest.protocol()) &&
                    (m_currentSrcURL.host() == info.uDest.host()) &&
                    (m_currentSrcURL.port() == info.uDest.port()) &&
                    (m_currentSrcURL.user() == info.uDest.user()) &&
                    (m_currentSrcURL.pass() == info.uDest.pass()) ) {
                    // This is the case of creating a real symlink
                    info.uDest.addPath( m_currentSrcURL.fileName() );
                } else {
                    // Different protocols, we'll create a .desktop file
                    // We have to change the extension anyway, so while we're at it,
                    // name the file like the URL
                    info.uDest.addPath(KIO::encodeFileName(m_currentSrcURL.prettyUrl()) + ".desktop");
                }
            }
            files.append( info ); // Files and any symlinks
            statNextSrc(); // we could use a loop instead of a recursive call :)
            return;
        }

        // Let's see if we can skip stat'ing, for the case where a directory view has the info already
        const KFileItem cachedItem = KDirLister::cachedItemForUrl(m_currentSrcURL);
        KIO::UDSEntry entry;
        if (!cachedItem.isNull()) {
            entry = cachedItem.entry();
            if (destinationState != DEST_DOESNT_EXIST) { // only resolve src if we could resolve dest (#218719)
                bool dummyIsLocal;
                m_currentSrcURL = cachedItem.mostLocalUrl(dummyIsLocal); // #183585
            }
        }

        if (m_mode == CopyJob::Move && (
                // Don't go renaming right away if we need a stat() to find out the destination filename
                KProtocolManager::fileNameUsedForCopying(m_currentSrcURL) == KProtocolInfo::FromUrl ||
                destinationState != DEST_IS_DIR || m_asMethod)
            ) {
           // If moving, before going for the full stat+[list+]copy+del thing, try to rename
           // The logic is pretty similar to FileCopyJobPrivate::slotStart()
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
        if (m_mode == CopyJob::Move && !KProtocolManager::supportsDeleting(m_currentSrcURL)) {
            QPointer<CopyJob> that = q;
            emit q->warning( q, buildErrorString(ERR_CANNOT_DELETE, m_currentSrcURL.prettyUrl()) );
            if (that)
                statNextSrc(); // we could use a loop instead of a recursive call :)
            return;
        }

        m_bOnlyRenames = false;

        // Testing for entry.count()>0 here is not good enough; KFileItem inserts
        // entries for UDS_USER and UDS_GROUP even on initially empty UDSEntries (#192185)
        if (entry.contains(KIO::UDSEntry::UDS_NAME)) {
            kDebug(7007) << "fast path! found info about" << m_currentSrcURL << "in KDirLister";
            sourceStated(entry, m_currentSrcURL);
            return;
        }

        // Stat the next src url
        Job * job = KIO::stat( m_currentSrcURL, StatJob::SourceSide, 2, KIO::HideProgressInfo );
        //kDebug(7007) << "KIO::stat on" << m_currentSrcURL;
        state = STATE_STATING;
        q->addSubjob(job);
        m_currentDestURL = m_dest;
        m_bURLDirty = true;
    }
    else
    {
        // Finished the stat'ing phase
        // First make sure that the totals were correctly emitted
        state = STATE_STATING;
        m_bURLDirty = true;
        slotReport();
        if (!dirs.isEmpty())
           emit q->aboutToCreate( q, dirs );
        if (!files.isEmpty())
           emit q->aboutToCreate( q, files );
        // Check if we are copying a single file
        m_bSingleFileCopy = ( files.count() == 1 && dirs.isEmpty() );
        // Then start copying things
        state = STATE_CREATING_DIRS;
        createNextDir();
    }
}

void CopyJobPrivate::startRenameJob( const KUrl& slave_url )
{
    Q_Q(CopyJob);

    // Silence KDirWatch notifications, otherwise performance is horrible
    if (m_currentSrcURL.isLocalFile()) {
        const QString parentDir = m_currentSrcURL.directory(KUrl::ObeyTrailingSlash);
        if (!m_parentDirs.contains(parentDir)) {
            KDirWatch::self()->stopDirScan(parentDir);
            m_parentDirs.insert(parentDir);
        }
    }

    KUrl dest = m_dest;
    // Append filename or dirname to destination URL, if allowed
    if ( destinationState == DEST_IS_DIR && !m_asMethod )
        dest.addPath( m_currentSrcURL.fileName() );
    m_currentDestURL = dest;
    kDebug(7007) << "This seems to be a suitable case for trying to rename before stat+[list+]copy+del";
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
    emit q->aboutToCreate( q, files );

    KIO_ARGS << m_currentSrcURL << dest << (qint8) false /*no overwrite*/;
    SimpleJob * newJob = SimpleJobPrivate::newJobNoUi(slave_url, CMD_RENAME, packedArgs);
    Scheduler::scheduleJob(newJob);
    q->addSubjob( newJob );
    if ( m_currentSrcURL.directory() != dest.directory() ) // For the user, moving isn't renaming. Only renaming is.
        m_bOnlyRenames = false;
}

void CopyJobPrivate::startListing( const KUrl & src )
{
    Q_Q(CopyJob);
    state = STATE_LISTING;
    m_bURLDirty = true;
    ListJob * newjob = listRecursive(src, KIO::HideProgressInfo);
    newjob->setUnrestricted(true);
    q->connect(newjob, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)),
               SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)));
    q->addSubjob( newjob );
}

void CopyJobPrivate::skip(const KUrl & sourceUrl, bool isDir)
{
    KUrl dir = sourceUrl;
    if (!isDir) {
        // Skipping a file: make sure not to delete the parent dir (#208418)
        dir.setPath(dir.directory());
    }
    while (dirsToRemove.removeAll(dir) > 0) {
        // Do not rely on rmdir() on the parent directories aborting.
        // Exclude the parent dirs explicitly.
        dir.setPath(dir.directory());
    }
}

bool CopyJobPrivate::shouldOverwriteDir( const QString& path ) const
{
    if ( m_bOverwriteAllDirs )
        return true;
    return m_overwriteList.contains(path);
}

bool CopyJobPrivate::shouldOverwriteFile( const QString& path ) const
{
    if ( m_bOverwriteAllFiles )
        return true;
    return m_overwriteList.contains(path);
}

bool CopyJobPrivate::shouldSkip( const QString& path ) const
{
    Q_FOREACH(const QString& skipPath, m_skipList) {
        if ( path.startsWith(skipPath) )
            return true;
    }
    return false;
}

void CopyJobPrivate::slotResultCreatingDirs( KJob * job )
{
    Q_Q(CopyJob);
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
            if ( m_bAutoSkipDirs ) {
                // We don't want to copy files in this directory, so we put it on the skip list
                m_skipList.append( oldURL.path( KUrl::AddTrailingSlash ) );
                skip(oldURL, true);
                dirs.erase( it ); // Move on to next dir
            } else {
                // Did the user choose to overwrite already?
                const QString destDir = (*it).uDest.path();
                if ( shouldOverwriteDir( destDir ) ) { // overwrite => just skip
                    emit q->copyingDone( q, (*it).uSource, (*it).uDest, (*it).mtime, true /* directory */, false /* renamed */ );
                    dirs.erase( it ); // Move on to next dir
                } else {
                    if (m_bAutoRenameDirs) {
                        KUrl newUrl((*it).uDest);
                        QString oldPath = (*it).uDest.path(KUrl::AddTrailingSlash);

                        QString newName = KIO::RenameDialog::suggestName((*it).uDest, (*it).uDest.fileName());

                        newUrl.setFileName(newName);
                        emit q->renamed(q, (*it).uDest, newUrl); // for e.g. kpropsdlg

                        // Change the current one and strip the trailing '/'
                        (*it).uDest.setPath(newUrl.path(KUrl::RemoveTrailingSlash));

                        QString newPath = newUrl.path(KUrl::AddTrailingSlash); // With trailing slash
                        QList<CopyInfo>::Iterator renamedirit = it;
                        ++renamedirit;
                        // Change the name of subdirectories inside the directory
                        for(; renamedirit != dirs.end() ; ++renamedirit) {
                            QString path = (*renamedirit).uDest.path();
                            if (path.startsWith(oldPath)) {
                                QString n = path;
                                n.replace(0, oldPath.length(), newPath);
                                kDebug(7007) << "dirs list:" << (*renamedirit).uSource.path()
                                              << "was going to be" << path
                                              << ", changed into" << n;
                                (*renamedirit).uDest.setPath(n);
                            }
                        }
                        // Change filenames inside the directory
                        QList<CopyInfo>::Iterator renamefileit = files.begin();
                        for(; renamefileit != files.end() ; ++renamefileit) {
                            QString path = (*renamefileit).uDest.path();
                            if (path.startsWith(oldPath)) {
                                QString n = path;
                                n.replace(0, oldPath.length(), newPath);
                                kDebug(7007) << "files list:" << (*renamefileit).uSource.path()
                                              << "was going to be" << path
                                              << ", changed into" << n;
                                (*renamefileit).uDest.setPath(n);
                            }
                        }
                        if (!dirs.isEmpty()) {
                            emit q->aboutToCreate(q, dirs);
                        }
                        if (!files.isEmpty()) {
                            emit q->aboutToCreate(q, files);
                        }

                    }
                    else {
                        if (!q->isInteractive()) {
                            q->Job::slotResult(job); // will set the error and emit result(this)
                            return;
                        }

                        assert(((SimpleJob*)job)->url().url() == (*it).uDest.url());
                        q->removeSubjob(job);
                        assert (!q->hasSubjobs()); // We should have only one job at a time ...

                        // We need to stat the existing dir, to get its last-modification time
                        KUrl existingDest((*it).uDest);
                        SimpleJob * newJob = KIO::stat(existingDest, StatJob::DestinationSide, 2, KIO::HideProgressInfo);
                        Scheduler::scheduleJob(newJob);
                        kDebug(7007) << "KIO::stat for resolving conflict on " << existingDest;
                        state = STATE_CONFLICT_CREATING_DIRS;
                        q->addSubjob(newJob);
                        return; // Don't move to next dir yet !
                    }
                }
            }
        }
        else
        {
            // Severe error, abort
            q->Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
    }
    else // no error : remove from list, to move on to next dir
    {
        //this is required for the undo feature
        emit q->copyingDone( q, (*it).uSource, (*it).uDest, (*it).mtime, true, false );
        m_directoriesCopied.append( *it );
        dirs.erase( it );
    }

    m_processedDirs++;
    //emit processedAmount( this, KJob::Directories, m_processedDirs );
    q->removeSubjob( job );
    assert( !q->hasSubjobs() ); // We should have only one job at a time ...
    createNextDir();
}

void CopyJobPrivate::slotResultConflictCreatingDirs( KJob * job )
{
    Q_Q(CopyJob);
    // We come here after a conflict has been detected and we've stated the existing dir

    // The dir we were trying to create:
    QList<CopyInfo>::Iterator it = dirs.begin();

    const UDSEntry entry = ((KIO::StatJob*)job)->statResult();

    // Its modification time:
    const time_t destmtime = (time_t) entry.numberValue( KIO::UDSEntry::UDS_MODIFICATION_TIME, -1 );
    const time_t destctime = (time_t) entry.numberValue( KIO::UDSEntry::UDS_CREATION_TIME, -1 );

    const KIO::filesize_t destsize = entry.numberValue( KIO::UDSEntry::UDS_SIZE );
    const QString linkDest = entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST );

    q->removeSubjob( job );
    assert ( !q->hasSubjobs() ); // We should have only one job at a time ...

    // Always multi and skip (since there are files after that)
    RenameDialog_Mode mode = (RenameDialog_Mode)( M_MULTI | M_SKIP | M_ISDIR );
    // Overwrite only if the existing thing is a dir (no chance with a file)
    if ( m_conflictError == ERR_DIR_ALREADY_EXIST )
    {
        if( (*it).uSource == (*it).uDest ||
            ((*it).uSource.protocol() == (*it).uDest.protocol() &&
              (*it).uSource.path( KUrl::RemoveTrailingSlash ) == linkDest) )
          mode = (RenameDialog_Mode)( mode | M_OVERWRITE_ITSELF);
        else
          mode = (RenameDialog_Mode)( mode | M_OVERWRITE );
    }

    QString existingDest = (*it).uDest.path();
    QString newPath;
    if (m_reportTimer)
        m_reportTimer->stop();
    RenameDialog_Result r = q->ui()->askFileRename( q, i18n("Folder Already Exists"),
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
            q->setError( ERR_USER_CANCELED );
            q->emitResult();
            return;
        case R_AUTO_RENAME:
            m_bAutoRenameDirs = true;
            // fall through
        case R_RENAME:
        {
            QString oldPath = (*it).uDest.path( KUrl::AddTrailingSlash );
            KUrl newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            emit q->renamed( q, (*it).uDest, newUrl ); // for e.g. kpropsdlg

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
                    kDebug(7007) << "dirs list:" << (*renamedirit).uSource.path()
                                  << "was going to be" << path
                                  << ", changed into" << n;
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
                    kDebug(7007) << "files list:" << (*renamefileit).uSource.path()
                                  << "was going to be" << path
                                  << ", changed into" << n;
                    (*renamefileit).uDest.setPath( n );
                }
            }
            if (!dirs.isEmpty())
                emit q->aboutToCreate( q, dirs );
            if (!files.isEmpty())
                emit q->aboutToCreate( q, files );
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkipDirs = true;
            // fall through
        case R_SKIP:
            m_skipList.append( existingDest );
            skip((*it).uSource, true);
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        case R_OVERWRITE:
            m_overwriteList.append( existingDest );
            emit q->copyingDone( q, (*it).uSource, (*it).uDest, (*it).mtime, true /* directory */, false /* renamed */ );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        case R_OVERWRITE_ALL:
            m_bOverwriteAllDirs = true;
            emit q->copyingDone( q, (*it).uSource, (*it).uDest, (*it).mtime, true /* directory */, false /* renamed */ );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        default:
            assert( 0 );
    }
    state = STATE_CREATING_DIRS;
    //emit processedAmount( this, KJob::Directories, m_processedDirs );
    createNextDir();
}

void CopyJobPrivate::createNextDir()
{
    Q_Q(CopyJob);
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
        if (shouldOverwriteFile(udir.path())) { // if we are overwriting an existing file or symlink
            newjob->addMetaData("overwrite", "true");
        }

        m_currentDestURL = udir;
        m_bURLDirty = true;

        q->addSubjob(newjob);
        return;
    }
    else // we have finished creating dirs
    {
        q->setProcessedAmount( KJob::Directories, m_processedDirs ); // make sure final number appears

        if (m_mode == CopyJob::Move) {
            // Now we know which dirs hold the files we're going to delete.
            // To speed things up and prevent double-notification, we disable KDirWatch
            // on those dirs temporarily (using KDirWatch::self, that's the instanced
            // used by e.g. kdirlister).
            for ( QSet<QString>::const_iterator it = m_parentDirs.constBegin() ; it != m_parentDirs.constEnd() ; ++it )
                KDirWatch::self()->stopDirScan( *it );
        }

        state = STATE_COPYING_FILES;
        m_processedFiles++; // Ralf wants it to start at 1, not 0
        copyNextFile();
    }
}

void CopyJobPrivate::slotResultCopyingFiles( KJob * job )
{
    Q_Q(CopyJob);
    // The file we were trying to copy:
    QList<CopyInfo>::Iterator it = files.begin();
    if ( job->error() )
    {
        // Should we skip automatically ?
        if ( m_bAutoSkipFiles )
        {
            skip((*it).uSource, false);
            m_fileProcessedSize = (*it).size;
            files.erase( it ); // Move on to next file
        }
        else
        {
            if ( !q->isInteractive() ) {
                q->Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            m_conflictError = job->error(); // save for later
            // Existing dest ?
            if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
                 || ( m_conflictError == ERR_DIR_ALREADY_EXIST )
                 || ( m_conflictError == ERR_IDENTICAL_FILES ) )
            {
                if (m_bAutoRenameFiles) {
                    QString newName = KIO::RenameDialog::suggestName((*it).uDest, (*it).uDest.fileName());

                    KUrl newUrl((*it).uDest);
                    newUrl.setFileName(newName);

                    emit q->renamed(q, (*it).uDest, newUrl); // for e.g. kpropsdlg
                    (*it).uDest = newUrl;

                    QList<CopyInfo> files;
                    files.append(*it);
                    emit q->aboutToCreate(q, files);
                }
                else {
                    q->removeSubjob(job);
                    assert (!q->hasSubjobs());
                    // We need to stat the existing file, to get its last-modification time
                    KUrl existingFile((*it).uDest);
                    SimpleJob * newJob = KIO::stat(existingFile, StatJob::DestinationSide, 2, KIO::HideProgressInfo);
                    Scheduler::scheduleJob(newJob);
                    kDebug(7007) << "KIO::stat for resolving conflict on " << existingFile;
                    state = STATE_CONFLICT_COPYING_FILES;
                    q->addSubjob(newJob);
                    return; // Don't move to next file yet !
                }
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
        if ( m_bCurrentOperationIsLink && m_mode == CopyJob::Move
             && !qobject_cast<KIO::DeleteJob *>( job ) // Deleting source not already done
             )
        {
            q->removeSubjob( job );
            assert ( !q->hasSubjobs() );
            // The only problem with this trick is that the error handling for this del operation
            // is not going to be right... see 'Very special case' above.
            KIO::Job * newjob = KIO::del( (*it).uSource, HideProgressInfo );
            q->addSubjob( newjob );
            return; // Don't move to next file yet !
        }

        if ( m_bCurrentOperationIsLink )
        {
            QString target = ( m_mode == CopyJob::Link ? (*it).uSource.path() : (*it).linkDest );
            //required for the undo feature
            emit q->copyingLinkDone( q, (*it).uSource, target, (*it).uDest );
        }
        else {
            //required for the undo feature
            emit q->copyingDone( q, (*it).uSource, (*it).uDest, (*it).mtime, false, false );
            if (m_mode == CopyJob::Move)
                org::kde::KDirNotify::emitFileMoved( (*it).uSource.url(), (*it).uDest.url() );
            m_successSrcList.append((*it).uSource);
        }
        // remove from list, to move on to next file
        files.erase( it );
    }
    m_processedFiles++;

    // clear processed size for last file and add it to overall processed size
    m_processedSize += m_fileProcessedSize;
    m_fileProcessedSize = 0;

    //kDebug(7007) << files.count() << "files remaining";

    // Merge metadata from subjob
    KIO::Job* kiojob = dynamic_cast<KIO::Job*>(job);
    Q_ASSERT(kiojob);
    m_incomingMetaData += kiojob->metaData();
    q->removeSubjob( job );
    assert( !q->hasSubjobs() ); // We should have only one job at a time ...
    copyNextFile();
}

void CopyJobPrivate::slotResultConflictCopyingFiles( KJob * job )
{
    Q_Q(CopyJob);
    // We come here after a conflict has been detected and we've stated the existing file
    // The file we were trying to create:
    QList<CopyInfo>::Iterator it = files.begin();

    RenameDialog_Result res;
    QString newPath;

    if (m_reportTimer)
        m_reportTimer->stop();

    if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
         || ( m_conflictError == ERR_DIR_ALREADY_EXIST )
         || ( m_conflictError == ERR_IDENTICAL_FILES ) )
    {
        // Its modification time:
        const UDSEntry entry = ((KIO::StatJob*)job)->statResult();

        const time_t destmtime = (time_t) entry.numberValue( KIO::UDSEntry::UDS_MODIFICATION_TIME, -1 );
        const time_t destctime = (time_t) entry.numberValue( KIO::UDSEntry::UDS_CREATION_TIME, -1 );
        const KIO::filesize_t destsize = entry.numberValue( KIO::UDSEntry::UDS_SIZE );
        const QString linkDest = entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST );

        // Offer overwrite only if the existing thing is a file
        // If src==dest, use "overwrite-itself"
        RenameDialog_Mode mode;
        bool isDir = true;

        if( m_conflictError == ERR_DIR_ALREADY_EXIST )
            mode = M_ISDIR;
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

        if ( !m_bSingleFileCopy )
            mode = (RenameDialog_Mode) ( mode | M_MULTI | M_SKIP );

        res = q->ui()->askFileRename( q, !isDir ?
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
        else if ( !q->isInteractive() ) {
            q->Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
        else
        {
            SkipDialog_Result skipResult = q->ui()->askSkip( q, files.count() > 1,
                                                          job->errorString() );

            // Convert the return code from SkipDialog into a RenameDialog code
            res = ( skipResult == S_SKIP ) ? R_SKIP :
                         ( skipResult == S_AUTO_SKIP ) ? R_AUTO_SKIP :
                                        R_CANCEL;
        }
    }

    if (m_reportTimer)
        m_reportTimer->start(REPORT_TIMEOUT);

    q->removeSubjob( job );
    assert ( !q->hasSubjobs() );
    switch ( res ) {
        case R_CANCEL:
            q->setError( ERR_USER_CANCELED );
            q->emitResult();
            return;
        case R_AUTO_RENAME:
            m_bAutoRenameFiles = true;
            // fall through
        case R_RENAME:
        {
            KUrl newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            emit q->renamed( q, (*it).uDest, newUrl ); // for e.g. kpropsdlg
            (*it).uDest = newUrl;

            QList<CopyInfo> files;
            files.append(*it);
            emit q->aboutToCreate( q, files );
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkipFiles = true;
            // fall through
        case R_SKIP:
            // Move on to next file
            skip((*it).uSource, false);
            m_processedSize += (*it).size;
            files.erase( it );
            m_processedFiles++;
            break;
       case R_OVERWRITE_ALL:
            m_bOverwriteAllFiles = true;
            break;
        case R_OVERWRITE:
            // Add to overwrite list, so that copyNextFile knows to overwrite
            m_overwriteList.append( (*it).uDest.path() );
            break;
        default:
            assert( 0 );
    }
    state = STATE_COPYING_FILES;
    copyNextFile();
}

KIO::Job* CopyJobPrivate::linkNextFile( const KUrl& uSource, const KUrl& uDest, JobFlags flags )
{
    //kDebug(7007) << "Linking";
    if (
        (uSource.protocol() == uDest.protocol()) &&
        (uSource.host() == uDest.host()) &&
        (uSource.port() == uDest.port()) &&
        (uSource.user() == uDest.user()) &&
        (uSource.pass() == uDest.pass()) )
    {
        // This is the case of creating a real symlink
        KIO::SimpleJob *newJob = KIO::symlink( uSource.path(), uDest, flags|HideProgressInfo /*no GUI*/ );
        Scheduler::scheduleJob(newJob);
        //kDebug(7007) << "Linking target=" << uSource.path() << "link=" << uDest;
        //emit linking( this, uSource.path(), uDest );
        m_bCurrentOperationIsLink = true;
        m_currentSrcURL=uSource;
        m_currentDestURL=uDest;
        m_bURLDirty = true;
        //Observer::self()->slotCopying( this, uSource, uDest ); // should be slotLinking perhaps
        return newJob;
    } else {
        Q_Q(CopyJob);
        //kDebug(7007) << "Linking URL=" << uSource << "link=" << uDest;
        if ( uDest.isLocalFile() ) {
            // if the source is a devices url, handle it a littlebit special

            QString path = uDest.toLocalFile();
            //kDebug(7007) << "path=" << path;
            QFile f( path );
            if ( f.open( QIODevice::ReadWrite ) )
            {
                f.close();
                KDesktopFile desktopFile( path );
                KConfigGroup config = desktopFile.desktopGroup();
                KUrl url = uSource;
                url.setPass( "" );
                config.writePathEntry( "URL", url.url() );
                config.writeEntry( "Name", url.url() );
                config.writeEntry( "Type", QString::fromLatin1("Link") );
                QString protocol = uSource.protocol();
                if ( protocol == QLatin1String("ftp") )
                    config.writeEntry( "Icon", QString::fromLatin1("folder-remote") );
                else if ( protocol == QLatin1String("http") )
                    config.writeEntry( "Icon", QString::fromLatin1("text-html") );
                else if ( protocol == QLatin1String("info") )
                    config.writeEntry( "Icon", QString::fromLatin1("text-x-texinfo") );
                else if ( protocol == QLatin1String("mailto") )   // sven:
                    config.writeEntry( "Icon", QString::fromLatin1("internet-mail") ); // added mailto: support
                else
                    config.writeEntry( "Icon", QString::fromLatin1("unknown") );
                config.sync();
                files.erase( files.begin() ); // done with this one, move on
                m_processedFiles++;
                //emit processedAmount( this, KJob::Files, m_processedFiles );
                copyNextFile();
                return 0;
            }
            else
            {
                kDebug(7007) << "ERR_CANNOT_OPEN_FOR_WRITING";
                q->setError( ERR_CANNOT_OPEN_FOR_WRITING );
                q->setErrorText( uDest.toLocalFile() );
                q->emitResult();
                return 0;
            }
        } else {
            // Todo: not show "link" on remote dirs if the src urls are not from the same protocol+host+...
            q->setError( ERR_CANNOT_SYMLINK );
            q->setErrorText( uDest.prettyUrl() );
            q->emitResult();
            return 0;
        }
    }
}

void CopyJobPrivate::copyNextFile()
{
    Q_Q(CopyJob);
    bool bCopyFile = false;
    //kDebug(7007);
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
        const KUrl& uSource = (*it).uSource;
        const KUrl& uDest = (*it).uDest;
        // Do we set overwrite ?
        bool bOverwrite;
        const QString destFile = uDest.path();
        // kDebug(7007) << "copying" << destFile;
        if ( uDest == uSource )
            bOverwrite = false;
        else
            bOverwrite = shouldOverwriteFile( destFile );

        m_bCurrentOperationIsLink = false;
        KIO::Job * newjob = 0;
        if ( m_mode == CopyJob::Link ) {
            // User requested that a symlink be made
            const JobFlags flags = bOverwrite ? Overwrite : DefaultFlags;
            newjob = linkNextFile(uSource, uDest, flags);
            if (!newjob)
                return;
        } else if ( !(*it).linkDest.isEmpty() &&
                  (uSource.protocol() == uDest.protocol()) &&
                  (uSource.host() == uDest.host()) &&
                  (uSource.port() == uDest.port()) &&
                  (uSource.user() == uDest.user()) &&
                  (uSource.pass() == uDest.pass()))
            // Copying a symlink - only on the same protocol/host/etc. (#5601, downloading an FTP file through its link),
        {
            const JobFlags flags = bOverwrite ? Overwrite : DefaultFlags;
            KIO::SimpleJob *newJob = KIO::symlink( (*it).linkDest, uDest, flags | HideProgressInfo /*no GUI*/ );
            Scheduler::scheduleJob(newJob);
            newjob = newJob;
            //kDebug(7007) << "Linking target=" << (*it).linkDest << "link=" << uDest;
            m_currentSrcURL = KUrl( (*it).linkDest );
            m_currentDestURL = uDest;
            m_bURLDirty = true;
            //emit linking( this, (*it).linkDest, uDest );
            //Observer::self()->slotCopying( this, m_currentSrcURL, uDest ); // should be slotLinking perhaps
            m_bCurrentOperationIsLink = true;
            // NOTE: if we are moving stuff, the deletion of the source will be done in slotResultCopyingFiles
        } else if (m_mode == CopyJob::Move) // Moving a file
        {
            JobFlags flags = bOverwrite ? Overwrite : DefaultFlags;
            KIO::FileCopyJob * moveJob = KIO::file_move( uSource, uDest, (*it).permissions, flags | HideProgressInfo/*no GUI*/ );
            moveJob->setSourceSize( (*it).size );
            if ((*it).mtime != -1) {
                moveJob->setModificationTime( QDateTime::fromTime_t( (*it).mtime ) ); // #55804
            }
            newjob = moveJob;
            //kDebug(7007) << "Moving" << uSource << "to" << uDest;
            //emit moving( this, uSource, uDest );
            m_currentSrcURL=uSource;
            m_currentDestURL=uDest;
            m_bURLDirty = true;
            //Observer::self()->slotMoving( this, uSource, uDest );
        }
        else // Copying a file
        {
            // If source isn't local and target is local, we ignore the original permissions
            // Otherwise, files downloaded from HTTP end up with -r--r--r--
            bool remoteSource = !KProtocolManager::supportsListing(uSource);
            int permissions = (*it).permissions;
            if ( m_defaultPermissions || ( remoteSource && uDest.isLocalFile() ) )
                permissions = -1;
            JobFlags flags = bOverwrite ? Overwrite : DefaultFlags;
            KIO::FileCopyJob * copyJob = KIO::file_copy( uSource, uDest, permissions, flags | HideProgressInfo/*no GUI*/ );
            copyJob->setParentJob( q ); // in case of rename dialog
            copyJob->setSourceSize( (*it).size );
            if ((*it).mtime != -1) {
                copyJob->setModificationTime( QDateTime::fromTime_t( (*it).mtime ) );
            }
            newjob = copyJob;
            //kDebug(7007) << "Copying" << uSource << "to" << uDest;
            m_currentSrcURL=uSource;
            m_currentDestURL=uDest;
            m_bURLDirty = true;
        }
        q->addSubjob(newjob);
        q->connect( newjob, SIGNAL( processedSize( KJob*, qulonglong ) ),
                    SLOT( slotProcessedSize( KJob*, qulonglong ) ) );
        q->connect( newjob, SIGNAL( totalSize( KJob*, qulonglong ) ),
                    SLOT( slotTotalSize( KJob*, qulonglong ) ) );
    }
    else
    {
        // We're done
        //kDebug(7007) << "copyNextFile finished";
        deleteNextDir();
    }
}

void CopyJobPrivate::deleteNextDir()
{
    Q_Q(CopyJob);
    if ( m_mode == CopyJob::Move && !dirsToRemove.isEmpty() ) // some dirs to delete ?
    {
        state = STATE_DELETING_DIRS;
        m_bURLDirty = true;
        // Take first dir to delete out of list - last ones first !
        KUrl::List::Iterator it = --dirsToRemove.end();
        SimpleJob *job = KIO::rmdir( *it );
        Scheduler::scheduleJob(job);
        dirsToRemove.erase(it);
        q->addSubjob( job );
    }
    else
    {
        // This step is done, move on
        state = STATE_SETTING_DIR_ATTRIBUTES;
        m_directoriesCopiedIterator = m_directoriesCopied.constBegin();
        setNextDirAttribute();
    }
}

void CopyJobPrivate::setNextDirAttribute()
{
    Q_Q(CopyJob);
    while (m_directoriesCopiedIterator != m_directoriesCopied.constEnd() &&
           (*m_directoriesCopiedIterator).mtime == -1) {
        ++m_directoriesCopiedIterator;
    }
    if ( m_directoriesCopiedIterator != m_directoriesCopied.constEnd() ) {
        const KUrl url = (*m_directoriesCopiedIterator).uDest;
        const time_t mtime = (*m_directoriesCopiedIterator).mtime;
        const QDateTime dt = QDateTime::fromTime_t(mtime);
        ++m_directoriesCopiedIterator;

        KIO::SimpleJob *job = KIO::setModificationTime( url, dt );
        Scheduler::scheduleJob(job);
        q->addSubjob( job );


#if 0 // ifdef Q_OS_UNIX
        // TODO: can be removed now. Or reintroduced as a fast path for local files
        // if launching even more jobs as done above is a performance problem.
        //
        QLinkedList<CopyInfo>::const_iterator it = m_directoriesCopied.constBegin();
        for ( ; it != m_directoriesCopied.constEnd() ; ++it ) {
            const KUrl& url = (*it).uDest;
            if ( url.isLocalFile() && (*it).mtime != (time_t)-1 ) {
                KDE_struct_stat statbuf;
                if (KDE::lstat(url.path(), &statbuf) == 0) {
                    struct utimbuf utbuf;
                    utbuf.actime = statbuf.st_atime; // access time, unchanged
                    utbuf.modtime = (*it).mtime; // modification time
                    utime( path, &utbuf );
                }

            }
        }
        m_directoriesCopied.clear();
        // but then we need to jump to the else part below. Maybe with a recursive call?
#endif
    } else {
        if (m_reportTimer)
            m_reportTimer->stop();
        --m_processedFiles; // undo the "start at 1" hack
        slotReport(); // display final numbers, important if progress dialog stays up

        q->emitResult();
    }
}

void CopyJob::emitResult()
{
    Q_D(CopyJob);
    // Before we go, tell the world about the changes that were made.
    // Even if some error made us abort midway, we might still have done
    // part of the job so we better update the views! (#118583)
    if (!d->m_bOnlyRenames) {
        KUrl url(d->m_globalDest);
        if (d->m_globalDestinationState != DEST_IS_DIR || d->m_asMethod)
            url.setPath(url.directory());
        //kDebug(7007) << "KDirNotify'ing FilesAdded" << url;
        org::kde::KDirNotify::emitFilesAdded( url.url() );

        if (d->m_mode == CopyJob::Move && !d->m_successSrcList.isEmpty()) {
            kDebug(7007) << "KDirNotify'ing FilesRemoved" << d->m_successSrcList.toStringList();
            org::kde::KDirNotify::emitFilesRemoved(d->m_successSrcList.toStringList());
        }

        // Re-enable watching on the dirs that held the deleted files
        if (d->m_mode == CopyJob::Move) {
            for (QSet<QString>::const_iterator it = d->m_parentDirs.constBegin() ; it != d->m_parentDirs.constEnd() ; ++it)
                KDirWatch::self()->restartDirScan( *it );
        }
    }
    Job::emitResult();
}

void CopyJobPrivate::slotProcessedSize( KJob*, qulonglong data_size )
{
  Q_Q(CopyJob);
  //kDebug(7007) << data_size;
  m_fileProcessedSize = data_size;
  q->setProcessedAmount(KJob::Bytes, m_processedSize + m_fileProcessedSize);

  if ( m_processedSize + m_fileProcessedSize > m_totalSize )
  {
    // Example: download any attachment from bugs.kde.org
    m_totalSize = m_processedSize + m_fileProcessedSize;
    //kDebug(7007) << "Adjusting m_totalSize to" << m_totalSize;
    q->setTotalAmount(KJob::Bytes, m_totalSize); // safety
  }
  //kDebug(7007) << "emit processedSize" << (unsigned long) (m_processedSize + m_fileProcessedSize);
  q->setProcessedAmount(KJob::Bytes, m_processedSize + m_fileProcessedSize);
}

void CopyJobPrivate::slotTotalSize( KJob*, qulonglong size )
{
  Q_Q(CopyJob);
  //kDebug(7007) << size;
  // Special case for copying a single file
  // This is because some protocols don't implement stat properly
  // (e.g. HTTP), and don't give us a size in some cases (redirection)
  // so we'd rather rely on the size given for the transfer
  if ( m_bSingleFileCopy && size != m_totalSize)
  {
    //kDebug(7007) << "slotTotalSize: updating totalsize to" << size;
    m_totalSize = size;
    q->setTotalAmount(KJob::Bytes, size);
  }
}

void CopyJobPrivate::slotResultDeletingDirs( KJob * job )
{
    Q_Q(CopyJob);
    if (job->error()) {
        // Couldn't remove directory. Well, perhaps it's not empty
        // because the user pressed Skip for a given file in it.
        // Let's not display "Could not remove dir ..." for each of those dir !
    } else {
        m_successSrcList.append(static_cast<KIO::SimpleJob*>(job)->url());
    }
    q->removeSubjob( job );
    assert( !q->hasSubjobs() );
    deleteNextDir();
}

void CopyJobPrivate::slotResultSettingDirAttributes( KJob * job )
{
    Q_Q(CopyJob);
    if (job->error())
    {
        // Couldn't set directory attributes. Ignore the error, it can happen
        // with inferior file systems like VFAT.
        // Let's not display warnings for each dir like "cp -a" does.
    }
    q->removeSubjob( job );
    assert( !q->hasSubjobs() );
    setNextDirAttribute();
}

// We were trying to do a direct renaming, before even stat'ing
void CopyJobPrivate::slotResultRenaming( KJob* job )
{
    Q_Q(CopyJob);
    int err = job->error();
    const QString errText = job->errorText();
    // Merge metadata from subjob
    KIO::Job* kiojob = dynamic_cast<KIO::Job*>(job);
    Q_ASSERT(kiojob);
    m_incomingMetaData += kiojob->metaData();
    q->removeSubjob( job );
    assert ( !q->hasSubjobs() );
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
            kDebug(7007) << "Couldn't rename directly, dest already exists. Detected special case of lower/uppercase renaming in same dir, try with 2 rename calls";
            const QString _src( m_currentSrcURL.toLocalFile() );
            const QString _dest( dest.toLocalFile() );
            const QString _tmpPrefix = m_currentSrcURL.directory(KUrl::ObeyTrailingSlash|KUrl::AppendTrailingSlash);
            KTemporaryFile tmpFile;
            tmpFile.setPrefix(_tmpPrefix);
            const bool openOk = tmpFile.open();
            if (!openOk) {
                kWarning(7007) << "Couldn't open temp file in" << _tmpPrefix;
            } else {
                const QString _tmp( tmpFile.fileName() );
                tmpFile.close();
                tmpFile.remove();
                kDebug(7007) << "KTemporaryFile using" << _tmp << "as intermediary";
                if (KDE::rename( _src, _tmp ) == 0) {
                    //kDebug(7007) << "Renaming" << _src << "to" << _tmp << "succeeded";
                    if (!QFile::exists( _dest ) && KDE::rename(_tmp, _dest) == 0) {
                        err = 0;
                        org::kde::KDirNotify::emitFileRenamed(m_currentSrcURL.url(), dest.url());
                    } else {
                        kDebug(7007) << "Didn't manage to rename" << _tmp << "to" << _dest << ", reverting";
                        // Revert back to original name!
                        if (KDE::rename( _tmp, _src ) != 0) {
                            kError(7007) << "Couldn't rename" << _tmp << "back to" << _src << '!';
                            // Severe error, abort
                            q->Job::slotResult(job); // will set the error and emit result(this)
                            return;
                        }
                    }
                } else {
                    kDebug(7007) << "mv" << _src << _tmp << "failed:" << strerror(errno);
                }
            }
        }
    }
    if ( err )
    {
        // This code is similar to CopyJobPrivate::slotResultConflictCopyingFiles
        // but here it's about the base src url being moved/renamed
        // (m_currentSrcURL) and its dest (m_dest), not about a single file.
        // It also means we already stated the dest, here.
        // On the other hand we haven't stated the src yet (we skipped doing it
        // to save time, since it's not necessary to rename directly!)...

        // Existing dest?
        if ( err == ERR_DIR_ALREADY_EXIST ||
               err == ERR_FILE_ALREADY_EXIST ||
               err == ERR_IDENTICAL_FILES )
        {
            // Should we skip automatically ?
            bool isDir = (err == ERR_DIR_ALREADY_EXIST); // ## technically, isDir means "source is dir", not "dest is dir" #######
            if ((isDir && m_bAutoSkipDirs) || (!isDir && m_bAutoSkipFiles)) {
                // Move on to next source url
                skipSrc(isDir);
                return;
            } else if ((isDir && m_bOverwriteAllDirs) || (!isDir && m_bOverwriteAllFiles)) {
                ; // nothing to do, stat+copy+del will overwrite
            } else if ((isDir && m_bAutoRenameDirs) || (!isDir && m_bAutoRenameFiles)) {
                KUrl destDirectory(m_dest);
                destDirectory.setPath(destDirectory.directory());
                QString newName = KIO::RenameDialog::suggestName(destDirectory, m_dest.fileName());

                m_dest.setFileName(newName);
                KIO::Job* job = KIO::stat(m_dest, StatJob::DestinationSide, 2, KIO::HideProgressInfo);
                state = STATE_STATING;
                destinationState = DEST_NOT_STATED;
                q->addSubjob(job);
                return;
            } else if ( q->isInteractive() ) {
                QString newPath;
                // we lack mtime info for both the src (not stated)
                // and the dest (stated but this info wasn't stored)
                // Let's do it for local files, at least
                KIO::filesize_t sizeSrc = (KIO::filesize_t) -1;
                KIO::filesize_t sizeDest = (KIO::filesize_t) -1;
                time_t ctimeSrc = (time_t) -1;
                time_t ctimeDest = (time_t) -1;
                time_t mtimeSrc = (time_t) -1;
                time_t mtimeDest = (time_t) -1;

                bool destIsDir = err == ERR_DIR_ALREADY_EXIST;

                // ## TODO we need to stat the source using KIO::stat
                // so that this code is properly network-transparent.

                KDE_struct_stat stat_buf;
                if ( m_currentSrcURL.isLocalFile() &&
                    KDE::stat(m_currentSrcURL.toLocalFile(), &stat_buf) == 0 ) {
                    sizeSrc = stat_buf.st_size;
                    ctimeSrc = stat_buf.st_ctime;
                    mtimeSrc = stat_buf.st_mtime;
                    isDir = S_ISDIR(stat_buf.st_mode);
                }
                if ( dest.isLocalFile() &&
                    KDE::stat(dest.toLocalFile(), &stat_buf) == 0 ) {
                    sizeDest = stat_buf.st_size;
                    ctimeDest = stat_buf.st_ctime;
                    mtimeDest = stat_buf.st_mtime;
                    destIsDir = S_ISDIR(stat_buf.st_mode);
                }

                // If src==dest, use "overwrite-itself"
                RenameDialog_Mode mode = ( m_currentSrcURL == dest ) ? M_OVERWRITE_ITSELF : M_OVERWRITE;
                if (!isDir && destIsDir) {
                    // We can't overwrite a dir with a file.
                    mode = (RenameDialog_Mode) 0;
                }

                if ( m_srcList.count() > 1 )
                    mode = (RenameDialog_Mode) ( mode | M_MULTI | M_SKIP );
                if (destIsDir)
                    mode = (RenameDialog_Mode) ( mode | M_ISDIR );

                if (m_reportTimer)
                    m_reportTimer->stop();

                RenameDialog_Result r = q->ui()->askFileRename(
                    q,
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
                    q->setError( ERR_USER_CANCELED );
                    q->emitResult();
                    return;
                }
                case R_AUTO_RENAME:
                    if (isDir) {
                        m_bAutoRenameDirs = true;
                    }
                    else {
                        m_bAutoRenameFiles = true;
                    }
                    // fall through
                case R_RENAME:
                {
                    // Set m_dest to the chosen destination
                    // This is only for this src url; the next one will revert to m_globalDest
                    m_dest.setPath( newPath );
                    KIO::Job* job = KIO::stat( m_dest, StatJob::DestinationSide, 2, KIO::HideProgressInfo );
                    state = STATE_STATING;
                    destinationState = DEST_NOT_STATED;
                    q->addSubjob(job);
                    return;
                }
                case R_AUTO_SKIP:
                    if (isDir)
                        m_bAutoSkipDirs = true;
                    else
                        m_bAutoSkipFiles = true;
                    // fall through
                case R_SKIP:
                    // Move on to next url
                    skipSrc(isDir);
                    return;
                case R_OVERWRITE_ALL:
                    if (destIsDir)
                        m_bOverwriteAllDirs = true;
                    else
                        m_bOverwriteAllFiles = true;
                    break;
                case R_OVERWRITE:
                    // Add to overwrite list
                    // Note that we add dest, not m_dest.
                    // This ensures that when moving several urls into a dir (m_dest),
                    // we only overwrite for the current one, not for all.
                    // When renaming a single file (m_asMethod), it makes no difference.
                    kDebug(7007) << "adding to overwrite list: " << dest.path();
                    m_overwriteList.append( dest.path() );
                    break;
                default:
                    //assert( 0 );
                    break;
                }
            } else if ( err != KIO::ERR_UNSUPPORTED_ACTION ) {
                // Dest already exists, and job is not interactive -> abort with error
                q->setError( err );
                q->setErrorText( errText );
                q->emitResult();
                return;
            }
        } else if ( err != KIO::ERR_UNSUPPORTED_ACTION ) {
            kDebug(7007) << "Couldn't rename" << m_currentSrcURL << "to" << dest << ", aborting";
            q->setError( err );
            q->setErrorText( errText );
            q->emitResult();
            return;
        }
        kDebug(7007) << "Couldn't rename" << m_currentSrcURL << "to" << dest << ", reverting to normal way, starting with stat";
        //kDebug(7007) << "KIO::stat on" << m_currentSrcURL;
        KIO::Job* job = KIO::stat( m_currentSrcURL, StatJob::SourceSide, 2, KIO::HideProgressInfo );
        state = STATE_STATING;
        q->addSubjob(job);
        m_bOnlyRenames = false;
    }
    else
    {
        kDebug(7007) << "Renaming succeeded, move on";
        ++m_processedFiles;
        emit q->copyingDone( q, *m_currentStatSrc, dest, -1 /*mtime unknown, and not needed*/, true, true );
        m_successSrcList.append(*m_currentStatSrc);
        statNextSrc();
    }
}

void CopyJob::slotResult( KJob *job )
{
    Q_D(CopyJob);
    //kDebug(7007) << "d->state=" << (int) d->state;
    // In each case, what we have to do is :
    // 1 - check for errors and treat them
    // 2 - removeSubjob(job);
    // 3 - decide what to do next

    switch ( d->state ) {
        case STATE_STATING: // We were trying to stat a src url or the dest
            d->slotResultStating( job );
            break;
        case STATE_RENAMING: // We were trying to do a direct renaming, before even stat'ing
        {
            d->slotResultRenaming( job );
            break;
        }
        case STATE_LISTING: // recursive listing finished
            //kDebug(7007) << "totalSize:" << (unsigned int) d->m_totalSize << "files:" << d->files.count() << "d->dirs:" << d->dirs.count();
            // Was there an error ?
            if (job->error())
            {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            removeSubjob( job );
            assert ( !hasSubjobs() );

            d->statNextSrc();
            break;
        case STATE_CREATING_DIRS:
            d->slotResultCreatingDirs( job );
            break;
        case STATE_CONFLICT_CREATING_DIRS:
            d->slotResultConflictCreatingDirs( job );
            break;
        case STATE_COPYING_FILES:
            d->slotResultCopyingFiles( job );
            break;
        case STATE_CONFLICT_COPYING_FILES:
            d->slotResultConflictCopyingFiles( job );
            break;
        case STATE_DELETING_DIRS:
            d->slotResultDeletingDirs( job );
            break;
        case STATE_SETTING_DIR_ATTRIBUTES:
            d->slotResultSettingDirAttributes( job );
            break;
        default:
            assert( 0 );
    }
}

void KIO::CopyJob::setDefaultPermissions( bool b )
{
    d_func()->m_defaultPermissions = b;
}

KIO::CopyJob::CopyMode KIO::CopyJob::operationMode() const
{
    return d_func()->m_mode;
}

void KIO::CopyJob::setAutoSkip(bool autoSkip)
{
    d_func()->m_bAutoSkipFiles = autoSkip;
    d_func()->m_bAutoSkipDirs = autoSkip;
}

void KIO::CopyJob::setWriteIntoExistingDirectories(bool overwriteAll) // #65926
{
    d_func()->m_bOverwriteAllDirs = overwriteAll;
}

CopyJob *KIO::copy(const KUrl& src, const KUrl& dest, JobFlags flags)
{
    //kDebug(7007) << "src=" << src << "dest=" << dest;
    KUrl::List srcList;
    srcList.append( src );
    return CopyJobPrivate::newJob(srcList, dest, CopyJob::Copy, false, flags);
}

CopyJob *KIO::copyAs(const KUrl& src, const KUrl& dest, JobFlags flags)
{
    //kDebug(7007) << "src=" << src << "dest=" << dest;
    KUrl::List srcList;
    srcList.append( src );
    return CopyJobPrivate::newJob(srcList, dest, CopyJob::Copy, true, flags);
}

CopyJob *KIO::copy( const KUrl::List& src, const KUrl& dest, JobFlags flags )
{
    //kDebug(7007) << src << dest;
    return CopyJobPrivate::newJob(src, dest, CopyJob::Copy, false, flags);
}

CopyJob *KIO::move(const KUrl& src, const KUrl& dest, JobFlags flags)
{
    //kDebug(7007) << src << dest;
    KUrl::List srcList;
    srcList.append( src );
    return CopyJobPrivate::newJob(srcList, dest, CopyJob::Move, false, flags);
}

CopyJob *KIO::moveAs(const KUrl& src, const KUrl& dest, JobFlags flags)
{
    //kDebug(7007) << src << dest;
    KUrl::List srcList;
    srcList.append( src );
    return CopyJobPrivate::newJob(srcList, dest, CopyJob::Move, true, flags);
}

CopyJob *KIO::move( const KUrl::List& src, const KUrl& dest, JobFlags flags)
{
    //kDebug(7007) << src << dest;
    return CopyJobPrivate::newJob(src, dest, CopyJob::Move, false, flags);
}

CopyJob *KIO::link(const KUrl& src, const KUrl& destDir, JobFlags flags)
{
    KUrl::List srcList;
    srcList.append( src );
    return CopyJobPrivate::newJob(srcList, destDir, CopyJob::Link, false, flags);
}

CopyJob *KIO::link(const KUrl::List& srcList, const KUrl& destDir, JobFlags flags)
{
    return CopyJobPrivate::newJob(srcList, destDir, CopyJob::Link, false, flags);
}

CopyJob *KIO::linkAs(const KUrl& src, const KUrl& destDir, JobFlags flags )
{
    KUrl::List srcList;
    srcList.append( src );
    return CopyJobPrivate::newJob(srcList, destDir, CopyJob::Link, false, flags);
}

CopyJob *KIO::trash(const KUrl& src, JobFlags flags)
{
    KUrl::List srcList;
    srcList.append( src );
    return CopyJobPrivate::newJob(srcList, KUrl( "trash:/" ), CopyJob::Move, false, flags);
}

CopyJob *KIO::trash(const KUrl::List& srcList, JobFlags flags)
{
    return CopyJobPrivate::newJob(srcList, KUrl( "trash:/" ), CopyJob::Move, false, flags);
}

#include "copyjob.moc"
