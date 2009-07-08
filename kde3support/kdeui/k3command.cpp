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

#include "k3command.h"
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>

#include "ktoolbarpopupaction.h"

K3Command::K3Command()
    : d( 0 )
{
}

K3Command::~K3Command()
{
}

class K3NamedCommand::Private
{
    public:
        QString name;
};

K3NamedCommand::K3NamedCommand( const QString &name )
    : K3Command(),
      d( new Private )
{
    d->name = name;
}

K3NamedCommand::~K3NamedCommand()
{
    delete d;
}

QString K3NamedCommand::name() const
{
    return d->name;
}

void K3NamedCommand::setName( const QString &name )
{
    d->name = name;
}

class K3MacroCommand::Private
{
    public:
        QList<K3Command *> commands;
};

K3MacroCommand::K3MacroCommand( const QString & name )
    : K3NamedCommand(name),
      d( new Private )
{
}

K3MacroCommand::~K3MacroCommand()
{
    qDeleteAll( d->commands );
}

void K3MacroCommand::addCommand( K3Command *command )
{
    d->commands.append(command);
}

void K3MacroCommand::execute()
{
    QListIterator<K3Command *> it( d->commands );
    while ( it.hasNext() ) {
        it.next()->execute();
    }
}

void K3MacroCommand::unexecute()
{
    QListIterator<K3Command *> it( d->commands );
    it.toBack();
    while ( it.hasPrevious() ) {
        it.previous()->unexecute();
    }
}

const QList<K3Command *> K3MacroCommand::commands() const
{
    return d->commands;
}


class K3CommandHistory::K3CommandHistoryPrivate {
public:
    K3CommandHistoryPrivate()
        : m_undoLimit(50), m_redoLimit(30),
        m_savedAt(-1), m_current(-1) {
    }
    ~K3CommandHistoryPrivate() {
        qDeleteAll( m_commands );
    }

    QList<K3Command *> m_commands;
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

K3CommandHistory::K3CommandHistory() :
    d( new K3CommandHistoryPrivate )
{
    clear();
}

K3CommandHistory::K3CommandHistory(KActionCollection * actionCollection, bool withMenus) :
    d( new K3CommandHistoryPrivate )
{
    if (withMenus)
    {
        // TODO instead of a menu this should show a listbox like koffice's KoCommandHistory does,
        // so that it's clearer that N actions will be undone together, not just action number N.

        // TODO also move this out of K3CommandHistory, to make it core-only.

        new K3UndoRedoAction( K3UndoRedoAction::Undo, actionCollection, this );
        new K3UndoRedoAction( K3UndoRedoAction::Redo, actionCollection, this );
    }
    else
    {
        actionCollection->addAction(KStandardAction::Undo, this, SLOT(undo()));
        actionCollection->addAction(KStandardAction::Redo, this, SLOT(redo()));
    }
    clear();
}

K3CommandHistory::~K3CommandHistory() {
    delete d;
}

void K3CommandHistory::clear() {
    qDeleteAll( d->m_commands );
    d->m_commands.clear();
    d->m_current = -1;
    d->m_savedAt = -1;
    emit commandHistoryChanged();
}

void K3CommandHistory::addCommand(K3Command *command, bool execute) {
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

K3Command * K3CommandHistory::presentCommand() const
{
    if ( d->m_current >= 0 )
        return d->m_commands[ d->m_current ];
    return 0;
}

void K3CommandHistory::undo() {
    Q_ASSERT( d->m_current >= 0 );

    K3Command* command = d->m_commands[ d->m_current ];

    command->unexecute();
    emit commandExecuted( command );

    --d->m_current;

    if ( d->m_current == d->m_savedAt )
        emit documentRestored();

    clipCommands(); // only needed here and in addCommand, NOT in redo
}

void K3CommandHistory::redo() {
    K3Command* command = d->m_commands[ d->m_current + 1 ];
    command->execute();
    emit commandExecuted( command );

    ++d->m_current;

    if ( d->m_current == d->m_savedAt )
        emit documentRestored();

    emit commandHistoryChanged();
}

void K3CommandHistory::documentSaved() {
    d->m_savedAt = d->m_current;
}

void K3CommandHistory::setUndoLimit(int limit) {
    if ( limit>0 && limit != d->m_undoLimit ) {
        d->m_undoLimit = limit;
        clipCommands();
    }
}

void K3CommandHistory::setRedoLimit(int limit) {
    if ( limit>0 && limit != d->m_redoLimit ) {
        d->m_redoLimit = limit;
        clipCommands();
    }
}

void K3CommandHistory::clipCommands() {
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

void K3CommandHistory::updateActions()
{
    // it hasn't changed, but this updates all actions connected to this command history.
    emit commandHistoryChanged();
}

bool K3CommandHistory::isUndoAvailable() const
{
    return d->m_current >= 0;
}

bool K3CommandHistory::isRedoAvailable() const
{
    return d->m_current < d->m_commands.count() - 1;
}

QList<K3Command *> K3CommandHistory::undoCommands( int maxCommands ) const
{
    QList<K3Command *> lst;
    for ( int i = d->m_current; i >= 0; --i ) {
        lst.append( d->m_commands[i] );
        if ( maxCommands > 0 && lst.count() == maxCommands )
            break;
    }
    return lst;
}

QList<K3Command *> K3CommandHistory::redoCommands( int maxCommands ) const
{
    QList<K3Command *> lst;
    for ( int i = d->m_current + 1; i < d->m_commands.count(); ++i )
    {
        lst.append( d->m_commands[i] );
        if ( maxCommands > 0 && lst.count() == maxCommands )
            break;
    }
    return lst;
}

int K3CommandHistory::undoLimit() const
{
     return d->m_undoLimit;
}

int K3CommandHistory::redoLimit() const
{
     return d->m_redoLimit;
}

class K3UndoRedoAction::Private
{
    public:
        Private( K3UndoRedoAction::Type type, K3CommandHistory* commandHistory)
            : type( type ),
              commandHistory( commandHistory )
        {
        }

        Type type;
        K3CommandHistory* commandHistory;
};



K3UndoRedoAction::K3UndoRedoAction( Type type, KActionCollection* actionCollection, K3CommandHistory* commandHistory )
    : KToolBarPopupAction( KIcon( type == Undo ? "edit-undo" : "edit-redo" ),
                           QString(), // text is set in clear() on start
                           actionCollection),
      d( new Private( type, commandHistory ) )
{
    setShortcut( KStandardShortcut::shortcut( type == Undo ? KStandardShortcut::Undo : KStandardShortcut::Redo ) );
    if ( d->type == Undo ) {
        connect( this, SIGNAL(triggered(bool)), d->commandHistory, SLOT(undo()) );
    } else {
        connect( this, SIGNAL(triggered(bool)), d->commandHistory, SLOT(redo()) );
    }
    connect( this->menu(), SIGNAL(aboutToShow()), this, SLOT(slotAboutToShow()) );
    connect( this->menu(), SIGNAL(triggered(QAction*)), this, SLOT(slotActionTriggered(QAction*)) );

    connect( d->commandHistory, SIGNAL(commandHistoryChanged()), this, SLOT(slotCommandHistoryChanged()) );
    slotCommandHistoryChanged();
    actionCollection->addAction(KStandardAction::stdName(type == Undo ? KStandardAction::Undo : KStandardAction::Redo),
                                this);
}

void K3UndoRedoAction::slotAboutToShow()
{
    menu()->clear();
    // TODO make number of items configurable ?
    const int maxCommands = 9;
    if ( d->type == Undo ) {
        const QList<K3Command *> commands = d->commandHistory->undoCommands( maxCommands );
        for (int i = 0; i < commands.count(); ++i) {
            QAction *action = menu()->addAction( i18n("Undo: %1", commands[i]->name()) );
            action->setData( i );
        }
    } else {
        const QList<K3Command *> commands = d->commandHistory->redoCommands( maxCommands );
        for (int i = 0; i < commands.count(); ++i) {
            QAction *action = menu()->addAction( i18n("Redo: %1", commands[i]->name()) );
            action->setData( i );
        }
    }
}

void K3UndoRedoAction::slotActionTriggered( QAction *action )
{
    const int pos = action->data().toInt();
    //kDebug(230) << pos;
    if ( d->type == Undo ) {
        for ( int i = 0 ; i < pos+1; ++i ) {
            d->commandHistory->undo();
        }
    } else {
        for ( int i = 0 ; i < pos+1; ++i ) {
            d->commandHistory->redo();
        }
    }
}

void K3UndoRedoAction::slotCommandHistoryChanged()
{
    const bool isUndo = d->type == Undo;
    const bool enabled = isUndo ? d->commandHistory->isUndoAvailable() : d->commandHistory->isRedoAvailable();
    setEnabled(enabled);
    if (!enabled) {
        setText(isUndo ? i18n("&Undo") : i18n("&Redo"));
    } else {
        if (isUndo) {
            K3Command* presentCommand = d->commandHistory->presentCommand();
            Q_ASSERT(presentCommand);
            setText(i18n("&Undo: %1", presentCommand->name()));
        } else {
            K3Command* redoCommand = d->commandHistory->redoCommands(1).first();
            setText(i18n("&Redo: %1", redoCommand->name()));
        }
    }
}


void K3Command::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void K3NamedCommand::virtual_hook( int id, void* data )
{ K3Command::virtual_hook( id, data ); }

void K3MacroCommand::virtual_hook( int id, void* data )
{ K3NamedCommand::virtual_hook( id, data ); }

#include "k3command.moc"
