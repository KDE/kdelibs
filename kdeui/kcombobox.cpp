/* This file is part of the KDE libraries

   Copyright (c) 2000 Dawit Alemayehu <adawit@earthlink.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qobjcoll.h>

#include <klocale.h>
#include <kstdaccel.h>

#include "kcombobox.h"
#include "kcombobox.moc"


KComboBox::KComboBox( QWidget *parent, const char *name )
          :QComboBox( parent, name )
{
    m_pEdit = 0;
    m_pContextMenu = 0;
    init();
}

KComboBox::KComboBox( bool rw, QWidget *parent, const char *name )
          :QComboBox( rw, parent, name )
{
    if ( rw )
    {
        m_pEdit = QComboBox::lineEdit();
        m_pEdit->installEventFilter( this );
        QObjectList *list = queryList( "QPopupMenu" );
        QObjectListIt it ( *list );
        m_pContextMenu = (QPopupMenu*) it.current();
        delete list;
    }
    else
    {
        m_pEdit = 0;
        m_pContextMenu = 0;
    }
    init();
}

KComboBox::~KComboBox()
{
    m_pContextMenu = 0; // Reset the pointer to NULL
    if( m_pEdit != 0 )
    {
        m_pEdit->removeEventFilter( this );
        m_pEdit = 0; // Reset the pointer to NULL
    }
}

void KComboBox::init()
{
    // Permanently set some parameters in the parent object.
    setAutoCompletion( false );

    // Initialize enable popup menu to false.
    // Below it will be enabled if the widget
    // is editable.
    m_bEnableMenu = false;

    // Enable context menu by default if widget
    // is editable.
    setEnableContextMenu( true );

    // Connect the signals and slots.
    connect( listBox(), SIGNAL( returnPressed( QListBoxItem* ) ), this, SLOT( itemSelected( QListBoxItem* ) ) );
    connect( listBox(), SIGNAL( clicked( QListBoxItem* ) ), this, SLOT( itemSelected( QListBoxItem* ) ) );
}

void KComboBox::setAutoCompletion( bool autocomplete )
{
    setCompletionMode( autocomplete ? KGlobalSettings::CompletionAuto : KGlobalSettings::completionMode() );
}

void KComboBox::setEnableContextMenu( bool showMenu )
{
    if( m_pEdit != 0 )
    {
        if( !m_bEnableMenu && showMenu )
        {
            connect ( m_pContextMenu, SIGNAL( aboutToShow() ), this, SLOT( aboutToShowMenu() ) );
            showModeChanger();
        }
        else if( m_bEnableMenu && !showMenu )
        {
            disconnect ( m_pContextMenu, SIGNAL( aboutToShow() ), this, SLOT( aboutToShowMenu() ) );
            hideModeChanger();
        }
        m_bEnableMenu = showMenu;
    }
}

void KComboBox::makeCompletion( const QString& text )
{
    if( m_pEdit != 0 )
    {
        QString match = completionObject()->makeCompletion( text );

        // If no match or the same match, simply return
        // without completing.
        if( match.isNull() || match == text )
            return;

        if( completionMode() == KGlobalSettings::CompletionShell )
        {
            m_pEdit->setText( match );
        }
        else
        {
            int pos = cursorPosition();
            m_pEdit->validateAndSet( match, pos, pos, match.length() );
        }
    }
    else if( m_pEdit == 0 )
    {
       if( text.isNull() )
        return;

       int index = listBox()->index( listBox()->findItem( text ) );
        if( index >= 0 )
            setCurrentItem( index );
    }
}

void KComboBox::rotateText( const QString& input, int dir )
{
    if( m_pEdit != 0 )
    {
        KCompletion* comp = completionObject();
        QString str;
        int len = m_pEdit->text().length();
        if( m_pEdit->hasMarkedText() && !input.isNull() )
        {
            str = m_pEdit->text();
            if( input == str ) return; // Skip rotation if same text
            int pos = str.find( m_pEdit->markedText() );
            int index = input.find( str.remove( pos , m_pEdit->markedText().length() ) );
            if( index == -1 ) return;
            else if( index == 0 ) str = input;
            m_pEdit->validateAndSet( str, cursorPosition(), pos, str.length() );
        }
        else
        {
            QStringList list = comp->items();
            int index = list.findIndex( m_pEdit->text() );
            if( index == -1 )
            {
                index = ( dir == 1 ) ? 0 : list.count()-1;
                str = ( len == 0 ) ? list[index] : input;
            }
            else
            {
                index += dir;
                if( index >= (int)list.count() ) index = 0; // rotate back to beginning
                else if( index < 0  ) index = list.count() - 1; // rotate back to the end
                str = list[index];
            }
            m_pEdit->setText( str );
        }
    }
    else // non-editable
    {
        int index = listBox()->index( listBox()->findItem( input ) );
        if( index >= 0 )
        {
            setCurrentItem( index );
        }
    }
}

void KComboBox::itemSelected( QListBoxItem* item )
{
    if( item != 0 && m_pEdit != 0 )
    {
        debug( "Item selected from list" );
        m_pEdit->setSelection( 0, m_pEdit->text().length() );
    }
}

void KComboBox::connectSignals( bool handle ) const
{
    if( handle && !handleSignals() )
    {
        connect( this, SIGNAL( completion( const QString& ) ), this, SLOT( makeCompletion( const QString& ) ) );
        connect( this, SIGNAL( rotateUp() ), this, SLOT( iterateUpInList() ) );
        connect( this, SIGNAL( rotateDown() ), this, SLOT( iterateDownInList() ) );
    }
    else if( !handle && handleSignals() )
    {
        disconnect( this, SIGNAL( completion( const QString& ) ), this, SLOT( makeCompletion( const QString& ) ) );
        disconnect( this, SIGNAL( rotateUp() ), this, SLOT( iterateUpInList() ) );
        disconnect( this, SIGNAL( rotateDown() ), this, SLOT( iterateDownInList() ) );
    }
}

void KComboBox::keyPressEvent ( QKeyEvent * e )
{
    if( m_pEdit != 0 && m_pEdit->hasFocus() )
    {
        KGlobalSettings::Completion mode = completionMode();
        // On Return pressed event, emit returnPressed( const QString& )
        if( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
        {
            emit returnPressed();
            emit returnPressed( m_pEdit->text() );
        }
        if( mode == KGlobalSettings::CompletionAuto )
        {
            QString keycode = e->text();
            if( !keycode.isNull() && keycode.unicode()->isPrint() && emitSignals() )
            {
                QComboBox::keyPressEvent ( e );
                emit completion( m_pEdit->text() );
                return;
            }
        }
        if( mode != KGlobalSettings::CompletionNone )
        {
            KCompletion* compObj = completionObject();
            bool fireSignals = emitSignals();
            int key = ( completionKey() == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : completionKey();
            if( KStdAccel::isEqual( e, key ) && fireSignals )
            {
                // Emit completion if the completion mode is NOT
                // CompletionAuto and if the mode is CompletionShell,
                // the cursor is at the end of the string.
                if( (mode == KGlobalSettings::CompletionMan &&
                    (compObj != 0 && compObj->lastMatch() != m_pEdit->displayText()) ) ||
                    (mode == KGlobalSettings::CompletionShell &&
                      m_pEdit->cursorPosition() == (int) m_pEdit->text().length() ))
                {
                    emit completion( m_pEdit->text() );
                    return;
                }
            }
            // Handles rotateUp.
            key = ( rotateUpKey() == 0 ) ? KStdAccel::key(KStdAccel::RotateUp) : rotateUpKey();
            if( KStdAccel::isEqual( e, key ) && fireSignals )
            {
                emit rotateUp ();
                e->accept();
                return;
            }
            // Handles rotateDown.
            key = ( rotateDownKey() == 0 ) ? KStdAccel::key(KStdAccel::RotateDown) : rotateDownKey();
            if( KStdAccel::isEqual( e, key ) && fireSignals )
            {
                emit rotateDown();
                e->accept();
                return;
            }
        }
    }
    else if( m_pEdit == 0 )
    {
        QString keycode = e->text();
        if ( !keycode.isNull() && keycode.unicode()->isPrint() )
        {
            emit completion ( keycode );
            e->accept();
            return;
        }
    }
    QComboBox::keyPressEvent( e );
}

bool KComboBox::eventFilter( QObject* o, QEvent* ev )
{
    if( o == m_pEdit && ev->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* e = (QMouseEvent*) ev;
        if( e->button() == Qt::RightButton && !m_bEnableMenu )
            return true;
    }
    return QComboBox::eventFilter( o, ev );
}
