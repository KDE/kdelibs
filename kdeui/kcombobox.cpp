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
        connect( m_pEdit, SIGNAL( returnPressed() ), this, SIGNAL( returnPressed() ) );
        connect( m_pEdit, SIGNAL( returnPressed() ), this, SLOT( returnKeyPressed() ) );
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
        m_bShowModeChanger = false;
    }
    init();
}

KComboBox::~KComboBox()
{
    if( m_pEdit )
        m_pEdit->removeEventFilter( this );

    delete m_pEdit;
    delete m_pSubMenu;
}

void KComboBox::init()
{
    // Permanently set some parameters in the parent object.
    setAutoCompletion( false );

    // Hide menu by default
    m_bEnableMenu = false;

    // Initalize Variables used in auto-completion mode.
    // These values greatly simplify the logic used to
    // determine whether completion is needed when the
    // widget is in in auto-completion mode.
    m_iPrevpos = 0; // keeps cursor position whenever it changes.
    m_iPrevlen = 0; // keeps length of text as it changes.

    // Initialize the context Menu.  The popup menu as well as
    // the mode switching entry have to be manually enabled.
    m_pSubMenu = 0;
    m_iCompletionID = -1;

    // Connect the signals and slots.
    connect( this, SIGNAL( textChanged( const QString& ) ), this, SLOT( entryChanged( const QString& ) ) );
    connect( listBox(), SIGNAL( returnPressed( QListBoxItem* ) ), this, SLOT( itemSelected( QListBoxItem* ) ) );
}

void KComboBox::setAutoCompletion( bool autocomplete )
{
    m_iCompletionMode = autocomplete ? KGlobal::CompletionAuto : KGlobal::completionMode();
}

void KComboBox::setCompletionObject( KCompletion* compObj, bool hsig )
{
    if( m_pCompObj != 0 )
        disconnect( m_pCompObj, SIGNAL( destroyed() ), this, SLOT( completionDestroyed() ) );

    KCompletionBase::setCompletionObject( compObj, hsig );

    if( m_pCompObj != 0 )
        connect( m_pCompObj, SIGNAL( destroyed() ), this, SLOT( completionDestroyed() ) );
}

void KComboBox::setEnableContextMenu( bool showMenu )
{
    if( m_pEdit != 0 && showMenu )
    {
        connect ( m_pContextMenu, SIGNAL( aboutToShow() ), this, SLOT( aboutToShowMenu() ) );
        showModeChanger();
    }
    m_bEnableMenu = showMenu;
}

void KComboBox::hideModeChanger()
{
    if( m_pSubMenu != 0 )
    {
        delete m_pSubMenu;
        m_pSubMenu = 0;
        m_iCompletionID = -1;
    }
    m_bShowModeChanger = false;
}

void KComboBox::showModeChanger()
{
    if( m_pSubMenu == 0 )
        m_pSubMenu = new QPopupMenu();
    m_bShowModeChanger = true;
}

void KComboBox::setEditText( const QString& text )
{
    if( m_pEdit != 0 && !text.isNull() )
    {
        int pos = cursorPosition();
        m_pEdit->validateAndSet( text, pos, 0, text.length() );
    }
    else
    {
        QComboBox::setEditText( text );
    }
}

void KComboBox::aboutToShowMenu()
{
    if( m_bShowModeChanger && m_pCompObj != 0 && m_iCompletionID == -1 )
    {
        m_pContextMenu->insertSeparator( m_pContextMenu->count() - 1 );
        m_iCompletionID = m_pContextMenu->insertItem( i18n("Completion"), m_pSubMenu, -1, m_pContextMenu->count() - 2 );
        connect ( m_pContextMenu, SIGNAL( highlighted( int ) ), this, SLOT( aboutToShowSubMenu( int ) ) );
    }
}

void KComboBox::aboutToShowSubMenu( int itemID )
{
    if( itemID == m_iCompletionID )
    {
        m_pSubMenu->clear();
        int id = m_pSubMenu->insertItem( i18n("None"), this, SLOT(modeNone()) );
        m_pSubMenu->setItemChecked( id, m_iCompletionMode == KGlobal::CompletionNone );
        id = m_pSubMenu->insertItem( i18n("Manual"), this, SLOT(modeShell()) );
        m_pSubMenu->setItemChecked( id, m_iCompletionMode == KGlobal::CompletionShell );
        id = m_pSubMenu->insertItem( i18n("Automatic"), this, SLOT(modeAuto()) );
        m_pSubMenu->setItemChecked( id, m_iCompletionMode == KGlobal::CompletionAuto );
        id = m_pSubMenu->insertItem( i18n("Semi-Automatic"), this, SLOT(modeManual()) );
        m_pSubMenu->setItemChecked( id, m_iCompletionMode == KGlobal::CompletionMan );
        if( m_iCompletionMode != KGlobal::completionMode() )
        {
            m_pSubMenu->insertSeparator();
            m_pSubMenu->insertItem( i18n("Default"), this, SLOT(modeDefault()) );
        }
    }
}

void KComboBox::entryChanged( const QString& text )
{
    if( m_bEmitSignals &&
        m_iCompletionMode == KGlobal::CompletionAuto )
    {
        int pos = cursorPosition();
        int len = text.length();
        if( pos > m_iPrevpos && len >= m_iPrevlen )
            emit completion( text );
        m_iPrevpos = pos;
        m_iPrevlen = len;
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

        if ( m_iCompletionMode == KGlobal::CompletionShell )
        {
            m_pEdit->setText( match );
            // TODO : deal with partial matches (multiple possible completions)
        }
        else
        {
            m_iPrevlen = match.length();
            m_iPrevpos = cursorPosition();
            m_pEdit->validateAndSet( match, m_iPrevpos, m_iPrevpos, m_iPrevlen );
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

void KComboBox::returnKeyPressed()
{
    if( m_pEdit && m_pEdit->text().length() != 0 )
    {
        emit returnPressed( m_pEdit->text() );
    }
}

void KComboBox::rotateText( const QString& input )
{
    if( input.length() == 0 )
        return;

    if( m_pEdit != 0 && m_pCompObj != 0 &&
        m_pCompObj->hasMultipleMatches() )
    {
        if( m_iCompletionMode == KGlobal::CompletionShell )
        {
            m_pEdit->setText( input );
        }
        else
        {
            int pos = cursorPosition();
            int len = input.length();
            m_pEdit->validateAndSet( input, pos, pos, len );
            m_iPrevlen = len;
            m_iPrevpos = pos;
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
        // This clears KCompletion so that if the user
        // deleted the current text and pressed the rotation
        // keys,  KCompletion will properly rotate through
        // all enteries.  Hack to the max :)
        if( m_pEdit != 0 && m_pEdit->text().length() == 0 &&
            m_pCompObj->lastMatch().length() != 0 )
        {
            m_pCompObj->makeCompletion( QString::null );
        }
        rotateText( m_pCompObj->previousMatch() );
    }
}

void KComboBox::iterateDownInList()
{
    if( m_pCompObj != 0 )
    {
        // This clears KCompletion so that if the user
        // deleted the current text and pressed the rotation
        // keys,  KCompletion will properly rotate through
        // all enteries.  Hack to the max :)
        if( m_pEdit != 0 && m_pEdit->text().length() == 0 &&
            m_pCompObj->lastMatch().length() != 0 )
        {
            m_pCompObj->makeCompletion( QString::null );
        }
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

bool KComboBox::eventFilter( QObject *o, QEvent *ev )
{
    if ( o == m_pEdit )
    {
        if( ev->type() == QEvent::KeyPress )
        {
            QKeyEvent *e = (QKeyEvent *) ev;
            int key = ( m_iCompletionKey == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : m_iCompletionKey;
            if( KStdAccel::isEqual( e, key ) && m_bEmitSignals )
            {
                // Emit completion if the completion mode is NOT
                // CompletionAuto and if the mode is CompletionShell,
                // the cursor is at the end of the string.
                if( m_iCompletionMode == KGlobal::CompletionMan ||
                    (m_iCompletionMode == KGlobal::CompletionShell &&
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
            // Always update the position holder if the user
            // pressed the END key in auto completion mode.
            if( m_iCompletionMode == KGlobal::CompletionAuto )
            {
                int pos = cursorPosition();
                int len = m_pEdit->text().length();
                if( m_iPrevpos != pos && pos == len )
                    m_iPrevpos = pos;
            }
        }
        else if ( ev->type() == QEvent::MouseButtonPress )
        {
            QMouseEvent *e = (QMouseEvent *) ev;
            if( e->button() == Qt::RightButton )
            {
                debug( m_bEnableMenu ? "Menu will be shown..." : "Menu will be hidden..." );
                if( m_bEnableMenu )
                    return false;
                return true;
            }
        }
    }
    return QComboBox::eventFilter( o, ev );
}

void KComboBox::keyPressEvent ( QKeyEvent * e )
{
    if( m_pEdit == 0 )
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
