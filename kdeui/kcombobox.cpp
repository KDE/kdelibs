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
    m_bShowModeChanger = false;
    init();
}

KComboBox::KComboBox( bool rw, QWidget *parent, const char *name )
          :QComboBox( rw, parent, name )
{
    if ( rw )
    {
        m_pEdit = QComboBox::lineEdit();
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
        m_pEdit = 0; // Reset the pointer to NULL
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
    if( m_pEdit != 0 )
        setEnableContextMenu( true );

    // Connect the signals and slots.
    connect( listBox(), SIGNAL( returnPressed( QListBoxItem* ) ), this, SLOT( itemSelected( QListBoxItem* ) ) );
    connect( listBox(), SIGNAL( clicked( QListBoxItem* ) ), this, SLOT( itemSelected( QListBoxItem* ) ) );
}

void KComboBox::setAutoCompletion( bool autocomplete )
{
    m_iCompletionMode = autocomplete ? KGlobalSettings::CompletionAuto : KGlobalSettings::completionMode();
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

void KComboBox::aboutToShowMenu()
{
    if( m_bShowModeChanger && m_pCompObj != 0 && m_iCompletionID == -1 )
    {
        // Ask the parent class to add the completion menu item and
        // simply connect it to the showCompeltionMenu slot to have
        // appropriate items inserted and checked as necessary!!
        insertCompletionMenu( this, SLOT( showCompletionMenu() ), m_pContextMenu, m_pContextMenu->count() - 1 );
    }
}

void KComboBox::makeCompletion( const QString& text )
{
    if( m_pEdit != 0 && m_pCompObj != 0 )
    {
        QString match = m_pCompObj->makeCompletion( text );

        // If no match or the same match, simply return
        // without completing.
        if( match.isNull() || match == text )
            return;

        if( m_iCompletionMode == KGlobalSettings::CompletionShell )
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

void KComboBox::rotateText( const QString& input )
{
    if( input.length() == 0 )
        return;

    if( m_pEdit != 0 && m_pCompObj != 0 )
    {
        if( m_iCompletionMode == KGlobalSettings::CompletionShell )
        {
            m_pEdit->setText( input );
        }
        else
        {
            int pos = cursorPosition();
            m_pEdit->validateAndSet( input, pos, pos, input.length() );
        }
    }
    else if( m_pEdit == 0 )
    {
        int index = listBox()->index( listBox()->findItem( input ) );
        if( index >= 0 )
        {
            setCurrentItem( index );
        }
    }
}

void KComboBox::iterateUpInList()
{
    if( m_pCompObj != 0 )
    {
        rotateText( m_pCompObj->previousMatch() );
    }
}

void KComboBox::iterateDownInList()
{
    if( m_pCompObj != 0 )
    {
        rotateText( m_pCompObj->nextMatch() );
    }
}

void KComboBox::itemSelected( QListBoxItem* item )
{
    if( item != 0 && m_pEdit != 0 )
    {
        m_pEdit->setSelection(0, m_pEdit->text().length() );
    }
}

void KComboBox::connectSignals( bool handle ) const
{
    if( handle && !m_bHandleSignals )
    {
        connect( this, SIGNAL( completion( const QString& ) ), this, SLOT( makeCompletion( const QString& ) ) );
        connect( this, SIGNAL( rotateUp() ), this, SLOT( iterateUpInList() ) );
        connect( this, SIGNAL( rotateDown() ), this, SLOT( iterateDownInList() ) );
    }
    else if( !handle && m_bHandleSignals )
    {
        disconnect( this, SIGNAL( completion( const QString& ) ), this, SLOT( makeCompletion( const QString& ) ) );
        disconnect( this, SIGNAL( rotateUp() ), this, SLOT( iterateUpInList() ) );
        disconnect( this, SIGNAL( rotateDown() ), this, SLOT( iterateDownInList() ) );
    }
}

void KComboBox::keyPressEvent ( QKeyEvent * e )
{
    if( m_pEdit != 0 && m_pEdit->hasFocus() &&
        m_iCompletionMode != KGlobalSettings::CompletionNone )
    {
        // On Return pressed event, emit returnPressed( const QString& )
        if( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
        {
            emit returnPressed( m_pEdit->text() );
        }
        if( m_iCompletionMode == KGlobalSettings::CompletionAuto )
        {
            QString keycode = e->text();
            if( !keycode.isNull() && keycode.unicode()->isPrint() && m_bEmitSignals )
            {
                QComboBox::keyPressEvent ( e );
                emit completion( m_pEdit->text() );
                return;
            }
        }
        int key = ( m_iCompletionKey == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : m_iCompletionKey;
        if( KStdAccel::isEqual( e, key ) && m_bEmitSignals )
        {
            // Emit completion if the completion mode is NOT
            // CompletionAuto and if the mode is CompletionShell,
            // the cursor is at the end of the string.
            if( (m_iCompletionMode == KGlobalSettings::CompletionMan &&
                 (m_pCompObj != 0 && m_pCompObj->lastMatch() != m_pEdit->displayText()) ) ||
                 (m_iCompletionMode == KGlobalSettings::CompletionShell &&
                  m_pEdit->cursorPosition() == (int) m_pEdit->text().length() ))
            {
                emit completion( m_pEdit->text() );
                return true;
            }
        }
        // Handles rotateUp.
        key = ( m_iRotateUpKey == 0 ) ? KStdAccel::key(KStdAccel::RotateUp) : m_iRotateUpKey;
        if( KStdAccel::isEqual( e, key ) && m_bEmitSignals )
        {
            emit rotateUp ();
            return true;
        }
        // Handles rotateDown.
        key = ( m_iRotateDnKey == 0 ) ? KStdAccel::key(KStdAccel::RotateDown) : m_iRotateDnKey;
        if( KStdAccel::isEqual( e, key ) && m_bEmitSignals )
        {
            emit rotateDown();
            return true;
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

void KComboBox::mousePressEvent( QMouseEvent* e )
{
    if( m_pEdit != 0 && m_pEdit->hasFocus() &&
        e->button() == Qt::RightButton )
    {
        if( m_bEnableMenu )
            return false;
        return true;
    }
    QComboBox::mousePressEvent( e );
}
