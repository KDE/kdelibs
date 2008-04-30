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

#ifndef FILEUNDOMANAGER_P_H
#define FILEUNDOMANAGER_P_H

#include "fileundomanager.h"
#include <QtCore/QStack>
#include <QUndoCommand>
#include <kurl.h>
#include <ctime>

class KJob;

namespace KIO {

class FileUndoManagerAdaptor;

struct BasicOperation
{
    typedef QStack<BasicOperation> Stack;

    BasicOperation()
    { m_valid = false; }

    bool m_valid;
    bool m_renamed;

    enum Type { File, Link, Directory };
    Type m_type:2;

    KUrl m_src;
    KUrl m_dst;
    QString m_target;
    time_t m_mtime;
};

class UndoCommand
{
public:
    typedef QStack<UndoCommand> Stack;

    UndoCommand()
    {
      m_valid = false;
    }

    // TODO: is ::TRASH missing?
    bool isMoveCommand() const { return m_type == FileUndoManager::Move || m_type == FileUndoManager::Rename; }

    bool m_valid;

    FileUndoManager::CommandType m_type;
    BasicOperation::Stack m_opStack;
    KUrl::List m_src;
    KUrl m_dst;
    quint64 m_serialNumber;
};


// This class listens to a job, collects info while it's running (for copyjobs)
// and when the job terminates, on success, it calls addCommand in the undomanager.
class CommandRecorder : public QObject
{
  Q_OBJECT
public:
  CommandRecorder( FileUndoManager::CommandType op, const KUrl::List &src, const KUrl &dst, KIO::Job *job );
  virtual ~CommandRecorder();

private Q_SLOTS:
  void slotResult( KJob *job );

  void slotCopyingDone( KIO::Job *, const KUrl &from, const KUrl &to, time_t, bool directory, bool renamed );
  void slotCopyingLinkDone( KIO::Job *, const KUrl &from, const QString &target, const KUrl &to );

private:
  UndoCommand m_cmd;
};

enum UndoState { MAKINGDIRS = 0, MOVINGFILES, STATINGFILE, REMOVINGDIRS, REMOVINGLINKS };

// The private class is, exceptionally, a real QObject
// so that it can be the class with the DBUS adaptor forwarding its signals.
class FileUndoManagerPrivate : public QObject
{
    Q_OBJECT
public:
    FileUndoManagerPrivate(FileUndoManager* qq);

    ~FileUndoManagerPrivate()
    {
        delete m_uiInterface;
    }

    void pushCommand( const UndoCommand& cmd );

    void broadcastPush( const UndoCommand &cmd );
    void broadcastPop();
    void broadcastLock();
    void broadcastUnlock();

    void addDirToUpdate( const KUrl& url );
    bool initializeFromKDesky();

    void undoStep();

    void stepMakingDirectories();
    void stepMovingFiles();
    void stepRemovingLinks();
    void stepRemovingDirectories();

    /// called by FileUndoManagerAdaptor
    QByteArray get() const;

    friend class UndoJob;
    /// called by UndoJob
    void stopUndo( bool step );

    friend class UndoCommandRecorder;
    /// called by UndoCommandRecorder
    void addCommand( const UndoCommand &cmd );

    bool m_syncronized;
    bool m_lock;

    UndoCommand::Stack m_commands;

    UndoCommand m_current;
    KIO::Job *m_currentJob;
    UndoState m_undoState;
    QStack<KUrl> m_dirStack;
    QStack<KUrl> m_dirCleanupStack;
    QStack<KUrl> m_linkCleanupStack;
    QList<KUrl> m_dirsToUpdate;
    FileUndoManager::UiInterface* m_uiInterface;

    UndoJob *m_undoJob;
    quint64 m_nextCommandIndex;

    FileUndoManager* q;

    // DBUS interface
Q_SIGNALS:
    /// DBUS signal
    void push(const QByteArray &command);
    /// DBUS signal
    void pop();
    /// DBUS signal
    void lock();
    /// DBUS signal
    void unlock();

public Q_SLOTS:
    // Those four slots are connected to DBUS signals
    void slotPush(QByteArray);
    void slotPop();
    void slotLock();
    void slotUnlock();

    void slotResult(KJob*);
};

} // namespace

#endif /* FILEUNDOMANAGER_P_H */
