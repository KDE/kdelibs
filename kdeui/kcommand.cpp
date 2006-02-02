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
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>

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
    KCommandHistoryPrivate() {
        m_savedAt = -1;
        m_current = -1;
    }
    ~KCommandHistoryPrivate() {}
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
    m_undo(0), m_redo(0), m_undoLimit(50), m_redoLimit(30),
    d( new KCommandHistoryPrivate )
{
    clear();
}

KCommandHistory::KCommandHistory(KActionCollection * actionCollection, bool withMenus) :
    m_undoLimit(50), m_redoLimit(30),
    d( new KCommandHistoryPrivate )
{
    if (withMenus)
    {
        // TODO instead of a menu this should show a listbox like koffice's KoCommandHistory does,
        // so that it's clearer that N actions will be undone together, not just action number N.

        KToolBarPopupAction * undo = new KToolBarPopupAction( i18n("&Undo"), "undo",
                                          KStdAccel::shortcut(KStdAccel::Undo), this, SLOT( undo() ),
                                          actionCollection, KStdAction::stdName( KStdAction::Undo ) );
        connect( undo->popupMenu(), SIGNAL( aboutToShow() ), this, SLOT( slotUndoAboutToShow() ) );
        connect( undo->popupMenu(), SIGNAL( activated( int ) ), this, SLOT( slotUndoActivated( int ) ) );
        m_undo = undo;
        m_undoPopup = undo->popupMenu();

        KToolBarPopupAction * redo = new KToolBarPopupAction( i18n("&Redo"), "redo",
                                          KStdAccel::shortcut(KStdAccel::Redo), this, SLOT( redo() ),
                                          actionCollection, KStdAction::stdName( KStdAction::Redo ) );
        connect( redo->popupMenu(), SIGNAL( aboutToShow() ), this, SLOT( slotRedoAboutToShow() ) );
        connect( redo->popupMenu(), SIGNAL( activated( int ) ), this, SLOT( slotRedoActivated( int ) ) );
        m_redo = redo;
        m_redoPopup = redo->popupMenu();
    }
    else
    {
        m_undo = KStdAction::undo( this, SLOT( undo() ), actionCollection );
        m_redo = KStdAction::redo( this, SLOT( redo() ), actionCollection );
        m_undoPopup = 0;
        m_redoPopup = 0;
    }
    clear();
}

KCommandHistory::~KCommandHistory() {
    qDeleteAll( m_commands );
    delete d;
}

void KCommandHistory::clear() {
    if ( m_undo ) {
        m_undo->setEnabled(false);
        m_undo->setText( i18n("&Undo") );
    }
    if ( m_redo ) {
        m_redo->setEnabled(false);
        m_redo->setText( i18n("&Redo") );
    }
    qDeleteAll( m_commands );
    m_commands.clear();
    d->m_current = -1;
    d->m_savedAt = -1;
}

void KCommandHistory::addCommand(KCommand *command, bool execute) {
    if ( !command )
        return;

    ++d->m_current;
    m_commands.insert( d->m_current, command );
    // Truncate history
    int count = m_commands.count();
    for ( int i = d->m_current + 1; i < count; ++i )
        delete m_commands.takeLast();

    // Check whether we still can reach savedAt
    if ( d->m_current < d->m_savedAt )
        d->m_savedAt = -2;

    if ( m_undo ) {
        m_undo->setEnabled(true);
        m_undo->setText( i18n("&Undo: %1").arg(command->name()) );
    }
    if ( m_redo && m_redo->isEnabled() ) {
        m_redo->setEnabled(false);
        m_redo->setText( i18n("&Redo") );
    }
    clipCommands();

    if ( execute )
    {
        command->execute();
        emit commandExecuted(command);
    }
}

void KCommandHistory::undo() {
    Q_ASSERT( d->m_current >= 0 );

    KCommand* command = m_commands[ d->m_current ];

    command->unexecute();
    emit commandExecuted( command );
    if ( m_redo ) {
        m_redo->setEnabled(true);
        m_redo->setText( i18n("&Redo: %1").arg(command->name()) );
    }

    --d->m_current;
    if ( d->m_current >= 0 ) {
        // undoing further is possible
        if (m_undo ) {
            KCommand* command = m_commands[ d->m_current ];
            m_undo->setEnabled(true);
            m_undo->setText( i18n("&Undo: %1").arg(command->name()) );
        }
    } else {
        // undoing further is not possible
        if ( m_undo ) {
            m_undo->setEnabled(false);
            m_undo->setText( i18n("&Undo") );
        }
    }

    if ( d->m_current == d->m_savedAt )
        emit documentRestored();

    clipCommands(); // only needed here and in addCommand, NOT in redo
}

void KCommandHistory::redo() {
    KCommand* command = m_commands[ d->m_current + 1 ];
    command->execute();
    emit commandExecuted( command );

    if ( m_undo ) {
        m_undo->setEnabled(true);
        m_undo->setText( i18n("&Undo: %1").arg(command->name()) );
    }

    ++d->m_current;

    if ( d->m_current == d->m_savedAt )
        emit documentRestored();

    if ( d->m_current + 1 < m_commands.count() ) {
        if ( m_redo ) {
            command = m_commands[ d->m_current + 1 ];
            m_redo->setEnabled(true);
            m_redo->setText( i18n("&Redo: %1").arg(command->name()) );
        }
    } else {
        if( m_redo ) {
            m_redo->setEnabled(false);
            m_redo->setText( i18n("&Redo") );
        }
    }
}

void KCommandHistory::documentSaved() {
    d->m_savedAt = d->m_current;
}

void KCommandHistory::setUndoLimit(int limit) {
    if ( limit>0 && limit != m_undoLimit ) {
        m_undoLimit = limit;
        clipCommands();
    }
}

void KCommandHistory::setRedoLimit(int limit) {
    if ( limit>0 && limit != m_redoLimit ) {
        m_redoLimit = limit;
        clipCommands();
    }
}

void KCommandHistory::clipCommands() {
    int count = m_commands.count();
    if ( count <= m_undoLimit && count <= m_redoLimit )
        return;

    if ( d->m_current >= m_undoLimit ) {
        const int toRemove = (d->m_current - m_undoLimit) + 1;
        for ( int i = 0; i < toRemove; ++i ) {
            delete m_commands.takeFirst();
            --d->m_savedAt;
            --d->m_current;
        }
        Q_ASSERT( d->m_current >= -1 );
        count = m_commands.count(); // update count for the redo branch below
        if ( d->m_savedAt < 0 )
            d->m_savedAt = -1; // savedAt went out of the history
    }

    if ( d->m_current + m_redoLimit + 1 < count ) {
        if ( d->m_savedAt > (d->m_current + m_redoLimit) )
            d->m_savedAt = -1;
        const int toRemove = count - (d->m_current + m_redoLimit + 1);
        for ( int i = 0; i< toRemove; ++i )
            delete m_commands.takeLast();
    }
}

void KCommandHistory::slotUndoAboutToShow()
{
    m_undoPopup->clear();
    // Start at d->m_current and go back until d->m_current - 9 included (or 0 if bigger).
    // TODO make number of items configurable ?
    const int end = qMax( d->m_current - 9, 0 );
    for ( int i = d->m_current; i >= end; --i ) {
        m_undoPopup->insertItem( i18n("Undo: %1").arg(m_commands[i]->name()) );
    }
}

void KCommandHistory::slotUndoActivated( int pos )
{
    kDebug(230) << "KCommandHistory::slotUndoActivated " << pos << endl;
    for ( int i = 0 ; i < pos+1; ++i )
        undo();
}

void KCommandHistory::slotRedoAboutToShow()
{
    m_redoPopup->clear();
    // Start at d->m_current + 1 and go up until d->m_current + 10 included (or count-1 if bigger).
    // TODO make number of items configurable ?
    const int end = qMin( d->m_current + 10, m_commands.count() - 1 );
    for ( int i = d->m_current + 1; i < end; ++i ) {
        m_redoPopup->insertItem( i18n("Redo: %1").arg(m_commands[i]->name()) );
    }
}

void KCommandHistory::slotRedoActivated( int pos )
{
    kDebug(230) << "KCommandHistory::slotRedoActivated " << pos << endl;
    for ( int i = 0 ; i < pos+1; ++i )
        redo();
}

void KCommandHistory::updateActions()
{
    if ( m_undo )
        m_undo->setEnabled( d->m_current >= 0 );
    if ( m_redo )
        m_redo->setEnabled( d->m_current < m_commands.count() - 1 );
}

void KCommand::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KNamedCommand::virtual_hook( int id, void* data )
{ KCommand::virtual_hook( id, data ); }

void KMacroCommand::virtual_hook( int id, void* data )
{ KNamedCommand::virtual_hook( id, data ); }

void KCommandHistory::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kcommand.moc"
