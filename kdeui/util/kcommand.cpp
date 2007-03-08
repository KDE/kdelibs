/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <trobin@kde.org>
   Copyright (C) 2000,2006 David Faure <faure@kde.org>

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

#include "kcommand.h"
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>

#include "ktoolbarpopupaction.h"

KCommand::~KCommand()
{
}

KMacroCommand::KMacroCommand( const QString & name ) : KNamedCommand(name)
{
}

KMacroCommand::~KMacroCommand()
{
    qDeleteAll( m_commands );
}

void KMacroCommand::addCommand( KCommand *command )
{
    m_commands.append(command);
}

void KMacroCommand::execute()
{
    QListIterator<KCommand *> it( m_commands );
    while ( it.hasNext() )
        it.next()->execute();
}

void KMacroCommand::unexecute()
{
    QListIterator<KCommand *> it( m_commands );
    it.toBack();
    while ( it.hasPrevious() )
        it.previous()->unexecute();
}


class KCommandHistory::KCommandHistoryPrivate {
public:
    KCommandHistoryPrivate()
        : m_undoLimit(50), m_redoLimit(30),
        m_savedAt(-1), m_current(-1) {
    }
    ~KCommandHistoryPrivate() {
        qDeleteAll( m_commands );
    }

    QList<KCommand *> m_commands;
    int m_undoLimit, m_redoLimit;

    int m_savedAt;
    int m_current;
    /*
    If m_commands contains:  <c0> <c1> <c2> <c3>

    m_current = 1 means we are between <c1> and <c2>, i.e. undo would unexecute c1.
    So m_current is the index of the current undo command, m_current+1 the current redo command if any.

    Adding a command at this point would delete <c2> and <c3>.
       m_current compared to the commands:  -1 <c0> 0 <c1> 1 <c2> 2.

    m_savedAt = 1 means that we where at m_current == 1 when the document was saved.
    m_savedAt = -1 means that the document was saved with an empty history (initial state, too).
    m_savedAt = -2 means that the document wasn't saved in the current visible history
         (this happens when the undo history got truncated)
    */
};

////////////

KCommandHistory::KCommandHistory() :
    d( new KCommandHistoryPrivate )
{
    clear();
}

KCommandHistory::KCommandHistory(KActionCollection * actionCollection, bool withMenus) :
    d( new KCommandHistoryPrivate )
{
    if (withMenus)
    {
        // TODO instead of a menu this should show a listbox like koffice's KoCommandHistory does,
        // so that it's clearer that N actions will be undone together, not just action number N.

        // TODO also move this out of KCommandHistory, to make it core-only.

        new KUndoRedoAction( KUndoRedoAction::Undo, actionCollection, this );
        new KUndoRedoAction( KUndoRedoAction::Redo, actionCollection, this );
    }
    else
    {
        actionCollection->addAction(KStandardAction::Undo, this, SLOT(undo()));
        actionCollection->addAction(KStandardAction::Redo, this, SLOT(redo()));
    }
    clear();
}

KCommandHistory::~KCommandHistory() {
    delete d;
}

void KCommandHistory::clear() {
    qDeleteAll( d->m_commands );
    d->m_commands.clear();
    d->m_current = -1;
    d->m_savedAt = -1;
    emit commandHistoryChanged();
}

void KCommandHistory::addCommand(KCommand *command, bool execute) {
    if ( !command )
        return;

    ++d->m_current;
    d->m_commands.insert( d->m_current, command );
    // Truncate history
    int count = d->m_commands.count();
    for ( int i = d->m_current + 1; i < count; ++i )
        delete d->m_commands.takeLast();

    // Check whether we still can reach savedAt
    if ( d->m_current < d->m_savedAt )
        d->m_savedAt = -2;

    clipCommands();

    if ( execute )
    {
        command->execute();
        emit commandExecuted(command);
    }
}

KCommand * KCommandHistory::presentCommand() const
{
    if ( d->m_current >= 0 )
        return d->m_commands[ d->m_current ];
    return 0;
}

void KCommandHistory::undo() {
    Q_ASSERT( d->m_current >= 0 );

    KCommand* command = d->m_commands[ d->m_current ];

    command->unexecute();
    emit commandExecuted( command );

    --d->m_current;

    if ( d->m_current == d->m_savedAt )
        emit documentRestored();

    clipCommands(); // only needed here and in addCommand, NOT in redo
}

void KCommandHistory::redo() {
    KCommand* command = d->m_commands[ d->m_current + 1 ];
    command->execute();
    emit commandExecuted( command );

    ++d->m_current;

    if ( d->m_current == d->m_savedAt )
        emit documentRestored();

    emit commandHistoryChanged();
}

void KCommandHistory::documentSaved() {
    d->m_savedAt = d->m_current;
}

void KCommandHistory::setUndoLimit(int limit) {
    if ( limit>0 && limit != d->m_undoLimit ) {
        d->m_undoLimit = limit;
        clipCommands();
    }
}

void KCommandHistory::setRedoLimit(int limit) {
    if ( limit>0 && limit != d->m_redoLimit ) {
        d->m_redoLimit = limit;
        clipCommands();
    }
}

void KCommandHistory::clipCommands() {
    int count = d->m_commands.count();
    if ( count <= d->m_undoLimit && count <= d->m_redoLimit ) {
        emit commandHistoryChanged();
        return;
    }

    if ( d->m_current >= d->m_undoLimit ) {
        const int toRemove = (d->m_current - d->m_undoLimit) + 1;
        for ( int i = 0; i < toRemove; ++i ) {
            delete d->m_commands.takeFirst();
            --d->m_savedAt;
            --d->m_current;
        }
        Q_ASSERT( d->m_current >= -1 );
        count = d->m_commands.count(); // update count for the redo branch below
        if ( d->m_savedAt < 0 )
            d->m_savedAt = -1; // savedAt went out of the history
    }

    if ( d->m_current + d->m_redoLimit + 1 < count ) {
        if ( d->m_savedAt > (d->m_current + d->m_redoLimit) )
            d->m_savedAt = -1;
        const int toRemove = count - (d->m_current + d->m_redoLimit + 1);
        for ( int i = 0; i< toRemove; ++i )
            delete d->m_commands.takeLast();
    }
    emit commandHistoryChanged();
}

void KCommandHistory::updateActions()
{
    // it hasn't changed, but this updates all actions connected to this command history.
    emit commandHistoryChanged();
}

bool KCommandHistory::isUndoAvailable() const
{
    return d->m_current >= 0;
}

bool KCommandHistory::isRedoAvailable() const
{
    return d->m_current < d->m_commands.count() - 1;
}

QList<KCommand *> KCommandHistory::undoCommands( int maxCommands ) const
{
    QList<KCommand *> lst;
    for ( int i = d->m_current; i >= 0; --i ) {
        lst.append( d->m_commands[i] );
        if ( maxCommands > 0 && lst.count() == maxCommands )
            break;
    }
    return lst;
}

QList<KCommand *> KCommandHistory::redoCommands( int maxCommands ) const
{
    QList<KCommand *> lst;
    for ( int i = d->m_current + 1; i < d->m_commands.count(); ++i )
    {
        lst.append( d->m_commands[i] );
        if ( maxCommands > 0 && lst.count() == maxCommands )
            break;
    }
    return lst;
}

int KCommandHistory::undoLimit() const
{
     return d->m_undoLimit;
}

int KCommandHistory::redoLimit() const
{
     return d->m_redoLimit;
}

KUndoRedoAction::KUndoRedoAction( Type type, KActionCollection* actionCollection, KCommandHistory* commandHistory )
    : KToolBarPopupAction( KIcon( type == Undo ? "edit-undo" : "edit-redo" ),
                           QString(), // text is set in clear() on start
                           actionCollection),
      m_type( type ),
      m_commandHistory( commandHistory ),
      d(0)
{
    setShortcut( KStandardShortcut::shortcut( type == Undo ? KStandardShortcut::Undo : KStandardShortcut::Redo ) );
    if (m_type == Undo)
        connect( this, SIGNAL(triggered(bool)), m_commandHistory, SLOT(undo()) );
    else
        connect( this, SIGNAL(triggered(bool)), m_commandHistory, SLOT(redo()) );
    connect( this->menu(), SIGNAL(aboutToShow()), this, SLOT(slotAboutToShow()) );
    connect( this->menu(), SIGNAL(triggered(QAction*)), this, SLOT(slotActionTriggered(QAction*)) );

    connect( m_commandHistory, SIGNAL(commandHistoryChanged()), this, SLOT(slotCommandHistoryChanged()) );
    slotCommandHistoryChanged();
    actionCollection->addAction(KStandardAction::stdName(type == Undo ? KStandardAction::Undo : KStandardAction::Redo),
                                this);
}

void KUndoRedoAction::slotAboutToShow()
{
    menu()->clear();
    // TODO make number of items configurable ?
    const int maxCommands = 9;
    if (m_type == Undo) {
        const QList<KCommand *> commands = m_commandHistory->undoCommands( maxCommands );
        for (int i = 0; i < commands.count(); ++i) {
            QAction *action = menu()->addAction( i18n("Undo: %1", commands[i]->name()) );
            action->setData( i );
        }
    } else {
        const QList<KCommand *> commands = m_commandHistory->redoCommands( maxCommands );
        for (int i = 0; i < commands.count(); ++i) {
            QAction *action = menu()->addAction( i18n("Redo: %1", commands[i]->name()) );
            action->setData( i );
        }
    }
}

void KUndoRedoAction::slotActionTriggered( QAction *action )
{
    const int pos = action->data().toInt();
    kDebug(230) << "KUndoRedoAction::slotActionTriggered " << pos << endl;
    if (m_type == Undo) {
        for ( int i = 0 ; i < pos+1; ++i )
            m_commandHistory->undo();
    } else {
        for ( int i = 0 ; i < pos+1; ++i )
            m_commandHistory->redo();
    }
}

void KUndoRedoAction::slotCommandHistoryChanged()
{
    const bool isUndo = m_type == Undo;
    const bool enabled = isUndo ? m_commandHistory->isUndoAvailable() : m_commandHistory->isRedoAvailable();
    setEnabled(enabled);
    if (!enabled) {
        setText(isUndo ? i18n("&Undo") : i18n("&Redo"));
    } else {
        if (isUndo) {
            KCommand* presentCommand = m_commandHistory->presentCommand();
            Q_ASSERT(presentCommand);
            setText(i18n("&Undo: %1", presentCommand->name()));
        } else {
            KCommand* redoCommand = m_commandHistory->redoCommands(1).first();
            setText(i18n("&Redo: %1", redoCommand->name()));
        }
    }
}


void KCommand::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KNamedCommand::virtual_hook( int id, void* data )
{ KCommand::virtual_hook( id, data ); }

void KMacroCommand::virtual_hook( int id, void* data )
{ KNamedCommand::virtual_hook( id, data ); }

#include "kcommand.moc"
