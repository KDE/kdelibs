/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2006, 2008 David Faure <faure@kde.org>

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

#include "fileundomanager.h"
#include "fileundomanager_p.h"
#include "fileundomanager_adaptor.h"

#include <kdatetime.h>
#include <kdebug.h>
#include <kdirnotify.h>
#include <kglobal.h>
#include <kio/copyjob.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kjobtrackerinterface.h>

#include <QtDBus/QtDBus>

#include <assert.h>

using namespace KIO;

static const char* undoStateToString(UndoState state) {
    static const char* const s_undoStateToString[] = { "MAKINGDIRS", "MOVINGFILES", "STATINGFILE", "REMOVINGDIRS", "REMOVINGLINKS" };
    return s_undoStateToString[state];
}

static QDataStream &operator<<(QDataStream &stream, const KIO::BasicOperation &op)
{
    stream << op.m_valid << (qint8)op.m_type << op.m_renamed
           << op.m_src << op.m_dst << op.m_target << (qint64)op.m_mtime;
    return stream;
}
static QDataStream &operator>>(QDataStream &stream, BasicOperation &op)
{
    qint8 type;
    qint64 mtime;
    stream >> op.m_valid >> type >> op.m_renamed
           >> op.m_src >> op.m_dst >> op.m_target >> mtime;
    op.m_type = static_cast<BasicOperation::Type>(type);
    op.m_mtime = mtime;
    return stream;
}

static QDataStream &operator<<(QDataStream &stream, const UndoCommand &cmd)
{
    stream << cmd.m_valid << (qint8)cmd.m_type << cmd.m_opStack << cmd.m_src << cmd.m_dst;
    return stream;
}

static QDataStream &operator>>(QDataStream &stream, UndoCommand &cmd)
{
    qint8 type;
    stream >> cmd.m_valid >> type >> cmd.m_opStack >> cmd.m_src >> cmd.m_dst;
    cmd.m_type = static_cast<FileUndoManager::CommandType>(type);
    return stream;
}

/**
 * checklist:
 * copy dir -> overwrite -> works
 * move dir -> overwrite -> works
 * copy dir -> rename -> works
 * move dir -> rename -> works
 *
 * copy dir -> works
 * move dir -> works
 *
 * copy files -> works
 * move files -> works (TODO: optimize (change FileCopyJob to use the renamed arg for copyingDone)
 *
 * copy files -> overwrite -> works (sorry for your overwritten file...)
 * move files -> overwrite -> works (sorry for your overwritten file...)
 *
 * copy files -> rename -> works
 * move files -> rename -> works
 *
 * -> see also fileundomanagertest, which tests some of the above (but not renaming).
 *
 */

class KIO::UndoJob : public KIO::Job
{
public:
    UndoJob(bool showProgressInfo) : KIO::Job() {
        if (showProgressInfo)
            KIO::getJobTracker()->registerJob(this);
    }
    virtual ~UndoJob() {}

    virtual void kill(bool) {
        FileUndoManager::self()->d->stopUndo(true);
        KIO::Job::doKill();
    }

    void emitCreatingDir(const KUrl &dir)
    { emit description(this, i18n("Creating directory"),
                       qMakePair(i18n("Directory"), dir.prettyUrl())); }
    void emitMoving(const KUrl &src, const KUrl &dest)
    { emit description(this, i18n("Moving"),
                       qMakePair(i18nc("The source of a file operation", "Source"), src.prettyUrl()),
                       qMakePair(i18nc("The destination of a file operation", "Destination"), dest.prettyUrl())); }
    void emitDeleting(const KUrl &url)
    { emit description(this, i18n("Deleting"),
                       qMakePair(i18n("File"), url.prettyUrl())); }
    void emitResult() { KIO::Job::emitResult(); }
};

CommandRecorder::CommandRecorder(FileUndoManager::CommandType op, const KUrl::List &src, const KUrl &dst, KIO::Job *job)
  : QObject(job)
{
  m_cmd.m_type = op;
  m_cmd.m_valid = true;
  m_cmd.m_serialNumber = FileUndoManager::self()->newCommandSerialNumber();
  m_cmd.m_src = src;
  m_cmd.m_dst = dst;
  connect(job, SIGNAL(result(KJob*)),
          this, SLOT(slotResult(KJob*)));

  if (op != FileUndoManager::Mkdir) {
      connect(job, SIGNAL(copyingDone(KIO::Job*,KUrl,KUrl,time_t,bool,bool)),
              this, SLOT(slotCopyingDone(KIO::Job*,KUrl,KUrl,time_t,bool,bool)));
      connect(job, SIGNAL(copyingLinkDone(KIO::Job *,KUrl,QString,KUrl)),
              this, SLOT(slotCopyingLinkDone(KIO::Job *,KUrl,QString,KUrl)));
  }
}

CommandRecorder::~CommandRecorder()
{
}

void CommandRecorder::slotResult(KJob *job)
{
    if (job->error())
        return;

    FileUndoManager::self()->d->addCommand(m_cmd);
}

void CommandRecorder::slotCopyingDone(KIO::Job *job, const KUrl &from, const KUrl &to, time_t mtime, bool directory, bool renamed)
{
  BasicOperation op;
  op.m_valid = true;
  op.m_type = directory ? BasicOperation::Directory : BasicOperation::File;
  op.m_renamed = renamed;
  op.m_src = from;
  op.m_dst = to;
  op.m_mtime = mtime;

  if (m_cmd.m_type == FileUndoManager::Trash)
  {
      Q_ASSERT(to.protocol() == "trash");
      const QMap<QString, QString> metaData = job->metaData();
      QMap<QString, QString>::ConstIterator it = metaData.find("trashURL-" + from.path());
      if (it != metaData.constEnd()) {
          // Update URL
          op.m_dst = it.value();
      }
  }

  m_cmd.m_opStack.prepend(op);
}

// TODO merge the signals?
void CommandRecorder::slotCopyingLinkDone(KIO::Job *, const KUrl &from, const QString &target, const KUrl &to)
{
  BasicOperation op;
  op.m_valid = true;
  op.m_type = BasicOperation::Link;
  op.m_renamed = false;
  op.m_src = from;
  op.m_target = target;
  op.m_dst = to;
  op.m_mtime = -1;
  m_cmd.m_opStack.prepend(op);
}

////

class KIO::FileUndoManagerSingleton
{
public:
    FileUndoManager self;
};
K_GLOBAL_STATIC(KIO::FileUndoManagerSingleton, globalFileUndoManager)

FileUndoManager *FileUndoManager::self()
{
    return &globalFileUndoManager->self;
}


// m_nextCommandIndex is initialized to a high number so that konqueror can
// assign low numbers to closed items loaded "on-demand" from a config file
// in KonqClosedWindowsManager::readConfig and thus maintaining the real
// order of the undo items.
FileUndoManagerPrivate::FileUndoManagerPrivate(FileUndoManager* qq)
    : m_uiInterface(new FileUndoManager::UiInterface()),
      m_undoJob(0), m_nextCommandIndex(1000), q(qq)
{
    m_syncronized = initializeFromKDesky();
    (void) new KIOFileUndoManagerAdaptor(this);
    const QString dbusPath = "/FileUndoManager";
    const QString dbusInterface = "org.kde.kio.FileUndoManager";

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(dbusPath, this);
    dbus.connect(QString(), dbusPath, dbusInterface, "lock", this, SLOT(slotLock()));
    dbus.connect(QString(), dbusPath, dbusInterface, "pop", this, SLOT(slotPop()));
    dbus.connect(QString(), dbusPath, dbusInterface, "push", this, SLOT(slotPush(QByteArray)));
    dbus.connect(QString(), dbusPath, dbusInterface, "unlock", this, SLOT(slotUnlock()));
}

FileUndoManager::FileUndoManager()
{
    d = new FileUndoManagerPrivate(this);
    d->m_lock = false;
    d->m_currentJob = 0;
}

FileUndoManager::~FileUndoManager()
{
    delete d;
}

void FileUndoManager::recordJob(CommandType op, const KUrl::List &src, const KUrl &dst, KIO::Job *job)
{
    // This records what the job does and calls addCommand when done
    (void) new CommandRecorder(op, src, dst, job);
    emit jobRecordingStarted(op);
}

void FileUndoManager::recordCopyJob(KIO::CopyJob* copyJob)
{
    CommandType commandType;
    switch (copyJob->operationMode()) {
    case CopyJob::Copy:
        commandType = Copy;
        break;
    case CopyJob::Move:
        commandType = Move;
        break;
    case CopyJob::Link:
    default: // prevent "wrong" compiler warning because of possibly uninitialized variable
        commandType = Link;
        break;
    }
    recordJob(commandType, copyJob->srcUrls(), copyJob->destUrl(), copyJob);
}

void FileUndoManagerPrivate::addCommand(const UndoCommand &cmd)
{
    broadcastPush(cmd);
    emit q->jobRecordingFinished(cmd.m_type);
}

bool FileUndoManager::undoAvailable() const
{
    return (d->m_commands.count() > 0) && !d->m_lock;
}

QString FileUndoManager::undoText() const
{
    if (d->m_commands.isEmpty())
        return i18n("Und&o");

    FileUndoManager::CommandType t = d->m_commands.top().m_type;
    if (t == FileUndoManager::Copy)
        return i18n("Und&o: Copy");
    else if (t == FileUndoManager::Link)
        return i18n("Und&o: Link");
    else if (t == FileUndoManager::Move)
        return i18n("Und&o: Move");
    else if (t == FileUndoManager::Rename)
        return i18n("Und&o: Rename");
    else if (t == FileUndoManager::Trash)
        return i18n("Und&o: Trash");
    else if (t == FileUndoManager::Mkdir)
        return i18n("Und&o: Create Folder");
    else
        assert(false);
    /* NOTREACHED */
    return QString();
}

quint64 FileUndoManager::newCommandSerialNumber()
{
    return ++(d->m_nextCommandIndex);
}

quint64 FileUndoManager::currentCommandSerialNumber() const
{
    if(!d->m_commands.isEmpty())
    {
        const UndoCommand& cmd = d->m_commands.top();
        assert(cmd.m_valid);
        return cmd.m_serialNumber;
    } else
    	return 0;
}

void FileUndoManager::undo()
{
    // Make a copy of the command to undo before broadcastPop() pops it.
    UndoCommand cmd = d->m_commands.top();
    assert(cmd.m_valid);
    d->m_current = cmd;

    BasicOperation::Stack& opStack = d->m_current.m_opStack;
    // Note that opStack is empty for simple operations like Mkdir.

    // Let's first ask for confirmation if we need to delete any file (#99898)
    KUrl::List fileCleanupStack;
    QStack<BasicOperation>::Iterator it = opStack.begin();
    for (; it != opStack.end() ; ++it) {
        BasicOperation::Type type = (*it).m_type;
        if (type == BasicOperation::File && d->m_current.m_type == FileUndoManager::Copy) {
            fileCleanupStack.append((*it).m_dst);
        }
    }
    if (d->m_current.m_type == FileUndoManager::Mkdir) {
        fileCleanupStack.append(d->m_current.m_dst);
    }
    if (!fileCleanupStack.isEmpty()) {
        if (!d->m_uiInterface->confirmDeletion(fileCleanupStack)) {
            return;
        }
    }

    d->broadcastPop();
    d->broadcastLock();

    d->m_dirCleanupStack.clear();
    d->m_dirStack.clear();
    d->m_dirsToUpdate.clear();

    d->m_undoState = MOVINGFILES;

    // Let's have a look at the basic operations we need to undo.
    // While we're at it, collect all links that should be deleted.

    it = opStack.begin();
    while (it != opStack.end()) // don't cache end() here, erase modifies it
    {
        bool removeBasicOperation = false;
        BasicOperation::Type type = (*it).m_type;
        if (type == BasicOperation::Directory && !(*it).m_renamed)
        {
            // If any directory has to be created/deleted, we'll start with that
            d->m_undoState = MAKINGDIRS;
            // Collect all the dirs that have to be created in case of a move undo.
            if (d->m_current.isMoveCommand())
                d->m_dirStack.push((*it).m_src);
            // Collect all dirs that have to be deleted
            // from the destination in both cases (copy and move).
            d->m_dirCleanupStack.prepend((*it).m_dst);
            removeBasicOperation = true;
        }
        else if (type == BasicOperation::Link)
        {
            d->m_linkCleanupStack.prepend((*it).m_dst);

            removeBasicOperation = !d->m_current.isMoveCommand();
        }

        if (removeBasicOperation)
            it = opStack.erase(it);
        else
            ++it;
    }

    kDebug(1203) << "starting with" << undoStateToString(d->m_undoState);
    d->m_undoJob = new UndoJob(d->m_uiInterface->showProgressInfo());
    d->undoStep();
}

void FileUndoManagerPrivate::stopUndo(bool step)
{
    m_current.m_opStack.clear();
    m_dirCleanupStack.clear();
    m_linkCleanupStack.clear();
    m_undoState = REMOVINGDIRS;
    m_undoJob = 0;

    if (m_currentJob)
        m_currentJob->kill();

    m_currentJob = 0;

    if (step)
        undoStep();
}

void FileUndoManagerPrivate::slotResult(KJob *job)
{
    m_currentJob = 0;
    if (job->error())
    {
        m_uiInterface->jobError(static_cast<KIO::Job*>(job));
        delete m_undoJob;
        stopUndo(false);
    }
    else if (m_undoState == STATINGFILE)
    {
        BasicOperation op = m_current.m_opStack.top();
        //kDebug(1203) << "stat result for " << op.m_dst;
        KIO::StatJob* statJob = static_cast<KIO::StatJob*>(job);
        time_t mtime = statJob->statResult().numberValue(KIO::UDSEntry::UDS_MODIFICATION_TIME, -1);
        if (mtime != op.m_mtime) {
            kDebug(1203) << op.m_dst << " was modified after being copied!";
            KDateTime srcTime; srcTime.setTime_t(op.m_mtime); srcTime = srcTime.toLocalZone();
            KDateTime destTime; destTime.setTime_t(mtime); destTime = destTime.toLocalZone();
            if (!m_uiInterface->copiedFileWasModified(op.m_src, op.m_dst, srcTime, destTime)) {
                stopUndo(false);
            }
        }
    }

    undoStep();
}


void FileUndoManagerPrivate::addDirToUpdate(const KUrl& url)
{
    if (!m_dirsToUpdate.contains(url))
        m_dirsToUpdate.prepend(url);
}

void FileUndoManagerPrivate::undoStep()
{
    m_currentJob = 0;

    if (m_undoState == MAKINGDIRS)
        stepMakingDirectories();

    if (m_undoState == MOVINGFILES || m_undoState == STATINGFILE)
        stepMovingFiles();

    if (m_undoState == REMOVINGLINKS)
        stepRemovingLinks();

    if (m_undoState == REMOVINGDIRS)
        stepRemovingDirectories();

    if (m_currentJob) {
        if (m_uiInterface)
            m_currentJob->ui()->setWindow(m_uiInterface->parentWidget());
        QObject::connect(m_currentJob, SIGNAL(result(KJob*)),
                         this, SLOT(slotResult(KJob*)));
    }
}

void FileUndoManagerPrivate::stepMakingDirectories()
{
    if (!m_dirStack.isEmpty()) {
        KUrl dir = m_dirStack.pop();
        kDebug(1203) << "creatingDir" << dir;
        m_currentJob = KIO::mkdir(dir);
        m_undoJob->emitCreatingDir(dir);
    }
    else
        m_undoState = MOVINGFILES;
}

// Misnamed method: It moves files back, but it also
// renames directories back, recreates symlinks,
// deletes copied files, and restores trashed files.
void FileUndoManagerPrivate::stepMovingFiles()
{
    if (!m_current.m_opStack.isEmpty())
    {
        BasicOperation op = m_current.m_opStack.top();
        BasicOperation::Type type = op.m_type;

        assert(op.m_valid);
        if (type == BasicOperation::Directory)
        {
            if (op.m_renamed)
            {
                kDebug(1203) << "rename" << op.m_dst << op.m_src;
                m_currentJob = KIO::rename(op.m_dst, op.m_src, KIO::HideProgressInfo);
                m_undoJob->emitMoving(op.m_dst, op.m_src);
            }
            else
                assert(0); // this should not happen!
        }
        else if (type == BasicOperation::Link)
        {
            kDebug(1203) << "symlink" << op.m_target << op.m_src;
            m_currentJob = KIO::symlink(op.m_target, op.m_src, KIO::Overwrite | KIO::HideProgressInfo);
        }
        else if (m_current.m_type == FileUndoManager::Copy)
        {
            if (m_undoState == MOVINGFILES) // dest not stat'ed yet
            {
                // Before we delete op.m_dst, let's check if it was modified (#20532)
                kDebug(1203) << "stat" << op.m_dst;
                m_currentJob = KIO::stat(op.m_dst, KIO::HideProgressInfo);
                m_undoState = STATINGFILE; // temporarily
                return; // no pop() yet, we'll finish the work in slotResult
            }
            else // dest was stat'ed, and the deletion was approved in slotResult
            {
                m_currentJob = KIO::file_delete(op.m_dst, KIO::HideProgressInfo);
                m_undoJob->emitDeleting(op.m_dst);
                m_undoState = MOVINGFILES;
            }
        }
        else if (m_current.isMoveCommand()
                  || m_current.m_type == FileUndoManager::Trash)
        {
            kDebug(1203) << "file_move" << op.m_dst << op.m_src;
            m_currentJob = KIO::file_move(op.m_dst, op.m_src, -1, KIO::Overwrite | KIO::HideProgressInfo);
            m_undoJob->emitMoving(op.m_dst, op.m_src);
        }

        m_current.m_opStack.pop();
        // The above KIO jobs are lowlevel, they don't trigger KDirNotify notification
        // So we need to do it ourselves (but schedule it to the end of the undo, to compress them)
        KUrl url(op.m_dst);
        url.setPath(url.directory());
        addDirToUpdate(url);

        url = op.m_src;
        url.setPath(url.directory());
        addDirToUpdate(url);
    }
    else
        m_undoState = REMOVINGLINKS;
}

void FileUndoManagerPrivate::stepRemovingLinks()
{
    kDebug(1203) << "REMOVINGLINKS";
    if (!m_linkCleanupStack.isEmpty())
    {
        KUrl file = m_linkCleanupStack.pop();
        kDebug(1203) << "file_delete" << file;
        m_currentJob = KIO::file_delete(file, KIO::HideProgressInfo);
        m_undoJob->emitDeleting(file);

        KUrl url(file);
        url.setPath(url.directory());
        addDirToUpdate(url);
    }
    else
    {
        m_undoState = REMOVINGDIRS;

        if (m_dirCleanupStack.isEmpty() && m_current.m_type == FileUndoManager::Mkdir)
            m_dirCleanupStack << m_current.m_dst;
    }
}

void FileUndoManagerPrivate::stepRemovingDirectories()
{
    if (!m_dirCleanupStack.isEmpty())
    {
        KUrl dir = m_dirCleanupStack.pop();
        kDebug(1203) << "rmdir" << dir;
        m_currentJob = KIO::rmdir(dir);
        m_undoJob->emitDeleting(dir);
        addDirToUpdate(dir);
    }
    else
    {
        m_current.m_valid = false;
        m_currentJob = 0;
        if (m_undoJob)
        {
            kDebug(1203) << "deleting undojob";
            m_undoJob->emitResult();
            m_undoJob = 0;
        }
        QList<KUrl>::ConstIterator it = m_dirsToUpdate.constBegin();
        for(; it != m_dirsToUpdate.constEnd(); ++it) {
            kDebug() << "Notifying FilesAdded for " << *it;
            org::kde::KDirNotify::emitFilesAdded((*it).url());
        }
        emit q->undoJobFinished();
        broadcastUnlock();
    }
}

// const ref doesn't work due to QDataStream
void FileUndoManagerPrivate::slotPush(QByteArray data)
{
    QDataStream strm(&data, QIODevice::ReadOnly);
    UndoCommand cmd;
    strm >> cmd;
    pushCommand(cmd);
}

void FileUndoManagerPrivate::pushCommand(const UndoCommand& cmd)
{
    m_commands.push(cmd);
    emit q->undoAvailable(true);
    emit q->undoTextChanged(q->undoText());
}

void FileUndoManagerPrivate::slotPop()
{
    m_commands.pop();
    emit q->undoAvailable(q->undoAvailable());
    emit q->undoTextChanged(q->undoText());
}

void FileUndoManagerPrivate::slotLock()
{
//  assert(!m_lock);
    m_lock = true;
    emit q->undoAvailable(q->undoAvailable());
}

void FileUndoManagerPrivate::slotUnlock()
{
//  assert(m_lock);
    m_lock = false;
    emit q->undoAvailable(q->undoAvailable());
}

QByteArray FileUndoManagerPrivate::get() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << m_commands;
    return data;
}

void FileUndoManagerPrivate::broadcastPush(const UndoCommand &cmd)
{
    if (!m_syncronized) {
        pushCommand(cmd);
        return;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << cmd;
    emit push(data); // DBUS signal
}

void FileUndoManagerPrivate::broadcastPop()
{
    if (!m_syncronized) {
        slotPop();
        return;
    }

    emit pop(); // DBUS signal
}

void FileUndoManagerPrivate::broadcastLock()
{
//  assert(!m_lock);

    if (!m_syncronized) {
        slotLock();
        return;
    }
    emit lock(); // DBUS signal
}

void FileUndoManagerPrivate::broadcastUnlock()
{
//  assert(m_lock);

    if (!m_syncronized) {
        slotUnlock();
        return;
    }
    emit unlock(); // DBUS signal
}

bool FileUndoManagerPrivate::initializeFromKDesky()
{
    // ### workaround for dcop problem and upcoming 2.1 release:
    // in case of huge io operations the amount of data sent over
    // dcop (containing undo information broadcasted for global undo
    // to all konqueror instances) can easily exceed the 64kb limit
    // of dcop. In order not to run into trouble we disable global
    // undo for now! (Simon)
    // ### FIXME: post 2.1
    // TODO KDE4: port to DBUS and test
    return false;
#if 0
    DCOPClient *client = kapp->dcopClient();

    if (client->appId() == "kdesktop") // we are master :)
        return true;

    if (!client->isApplicationRegistered("kdesktop"))
        return false;

    d->m_commands = DCOPRef("kdesktop", "FileUndoManager").call("get");
    return true;
#endif
}

void FileUndoManager::setUiInterface(UiInterface* ui)
{
    delete d->m_uiInterface;
    d->m_uiInterface = ui;
}

FileUndoManager::UiInterface* FileUndoManager::uiInterface() const
{
    return d->m_uiInterface;
}

////

class FileUndoManager::UiInterface::UiInterfacePrivate
{
public:
    UiInterfacePrivate()
        : m_parentWidget(0), m_showProgressInfo(true)
    {}
    QWidget* m_parentWidget;
    bool m_showProgressInfo;
};

FileUndoManager::UiInterface::UiInterface()
    : d(new UiInterfacePrivate)
{
}

FileUndoManager::UiInterface::~UiInterface()
{
    delete d;
}

void FileUndoManager::UiInterface::jobError(KIO::Job* job)
{
    job->ui()->showErrorMessage();
}

bool FileUndoManager::UiInterface::copiedFileWasModified(const KUrl& src, const KUrl& dest, const KDateTime& srcTime, const KDateTime& destTime)
{
    Q_UNUSED(srcTime); // not sure it should appear in the msgbox
    // Possible improvement: only show the time if date is today
    const QString timeStr = KGlobal::locale()->formatDateTime(destTime, KLocale::ShortDate);
    return KMessageBox::warningContinueCancel(
        d->m_parentWidget,
        i18n("The file %1 was copied from %2, but since then it has apparently been modified at %3.\n"
              "Undoing the copy will delete the file, and all modifications will be lost.\n"
              "Are you sure you want to delete %4?", dest.pathOrUrl(), src.pathOrUrl(), timeStr, dest.pathOrUrl()),
        i18n("Undo File Copy Confirmation"),
        KStandardGuiItem::cont(),
        KStandardGuiItem::cancel(),
        QString(),
        KMessageBox::Notify | KMessageBox::Dangerous) == KMessageBox::Continue;
}

bool FileUndoManager::UiInterface::confirmDeletion(const KUrl::List& files)
{
    KIO::JobUiDelegate uiDelegate;
    uiDelegate.setWindow(d->m_parentWidget);
    // Because undo can happen with an accidental Ctrl-Z, we want to always confirm.
    return uiDelegate.askDeleteConfirmation(files, KIO::JobUiDelegate::Delete, KIO::JobUiDelegate::ForceConfirmation);
}

QWidget* FileUndoManager::UiInterface::parentWidget() const
{
    return d->m_parentWidget;
}

void FileUndoManager::UiInterface::setParentWidget(QWidget* parentWidget)
{
    d->m_parentWidget = parentWidget;
}

void FileUndoManager::UiInterface::setShowProgressInfo(bool b)
{
    d->m_showProgressInfo = b;
}

bool FileUndoManager::UiInterface::showProgressInfo() const
{
    return d->m_showProgressInfo;
}

void FileUndoManager::UiInterface::virtual_hook(int, void*)
{
}

#include "fileundomanager_p.moc"
#include "fileundomanager.moc"
