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
    m_bShowContextMenu = false;
    m_bShowModeChanger = false;
    initialize();
}

KComboBox::KComboBox( bool rw, QWidget *parent, const char *name )
          :QComboBox( rw, parent, name )
{
    if ( rw )
    {
        QObjectList *list = queryList( "QLineEdit" );
        QObjectListIt it ( *list );
        m_pEdit = (QLineEdit*) it.current();
        connect( m_pEdit, SIGNAL( returnPressed() ), this, SIGNAL( returnPressed() ) );
        connect( m_pEdit, SIGNAL( returnPressed() ), this, SLOT( returnKeyPressed() ) );
        m_pEdit->installEventFilter( this );
        list = queryList( "QPopupMenu" );
        it = QObjectListIt( *list );
        m_pContextMenu = (QPopupMenu*) it.current();
        setEnableContextMenu( true ); // enable context menu by default
        setEnableModeChanger( true ); // enable mode changer by default
        m_iSubMenuId = -1;
        delete list;
    }
    else
    {
        m_pEdit = 0;
        m_pContextMenu = 0;
        m_bShowContextMenu = false;
        m_bShowModeChanger = false;
    }
    initialize();
}

KComboBox::~KComboBox()
{
    if( m_pEdit != 0 )
        m_pEdit->removeEventFilter( this );

    delete m_pEdit;
    delete m_pSubMenu;
    if( m_bAutoDelCompObj )
        delete m_pCompObj;
}

void KComboBox::initialize()
{
    // Permanently set some parameters in the parent object.
    setAutoCompletion( false );

    // Determines whether the completion object should
    // be deleted or not.
    m_bAutoDelCompObj = false;

    // By default emit completion signal
    m_bEmitCompletion = true;
    // By default emit rotation signals
    m_bEmitRotation = true;

    // Do not handle rotation & completion signals
    // internally by default.
    m_bHandleCompletionSignal = false;
    m_bHandleRotationSignal = false;

    // Initialize all key-bindings to 0 by default so that
    // the event filter will use the global settings.
    m_iCompletionKey = 0;
    m_iRotateUpKey = 0;
    m_iRotateDnKey = 0;

    // Initalize Variables used in auto-completion mode.
    // These values greatly simplify the logic used to
    // determine whether completion is needed when the
    // widget is in in auto-completion mode.
    m_iPrevpos = 0; // keeps cursor position whenever it changes.
    m_iPrevlen = 0; // keeps length of text as it changes.

    // Initialize the context Menu.  By default the popup
    // menu as well as the mode switching entry are enabled.
    m_pSubMenu = 0;

    // Assign the default completion type to use.
    m_iCompletionMode = KGlobal::completionMode();

    // Initialize the pointer to the completion object.
    m_pCompObj = 0;

    // Connect the signals and slots.
    connect( this, SIGNAL( textChanged( const QString& ) ), this, SLOT( entryChanged( const QString& ) ) );
    connect( listBox(), SIGNAL( returnPressed( QListBoxItem* ) ), this, SLOT( itemSelected( QListBoxItem* ) ) );
}

void KComboBox::setAutoCompletion( bool autocomplete )
{
    m_iCompletionMode = autocomplete ? KGlobal::CompletionAuto : KGlobal::completionMode();
}

void KComboBox::setCompletionObject( KCompletion* obj, bool autoDelete )
{
    if( m_pCompObj != 0 )
        disconnect( m_pCompObj, SIGNAL( destroyed() ), this, SLOT( completionDestroyed() ) );

    m_pCompObj = obj;
    m_bAutoDelCompObj = autoDelete;

    if( m_pCompObj != 0 )
    {
        setCompletionMode( m_iCompletionMode );
        connect( m_pCompObj, SIGNAL( destroyed() ), this, SLOT( completionDestroyed() ) );
    }
}

void KComboBox::setHandleCompletion( bool complete )
{
    if( m_pCompObj == 0 && complete )
        setCompletionObject ( new KCompletion(), true );

    if( complete && !m_bHandleCompletionSignal )
    {
        connect( this, SIGNAL( completion( const QString& ) ), this, SLOT( makeCompletion( const QString& ) ) );
        m_bHandleCompletionSignal = complete;
    }
    else if( !complete && m_bHandleCompletionSignal )
    {
        disconnect( this, SIGNAL( completion( const QString& ) ), this, SLOT( makeCompletion( const QString& ) ) );
        m_bHandleCompletionSignal = complete;
    }
}

void KComboBox::setHandleRotation( bool rotate )
{
    if( m_pCompObj == 0 && rotate )
        setCompletionObject ( new KCompletion(), true );

    if( rotate && !m_bHandleRotationSignal )
    {
        connect( this, SIGNAL( rotateUp() ), this, SLOT( iterateUpInList() ) );
        connect( this, SIGNAL( rotateDown() ), this, SLOT( iterateDownInList() ) );
        m_bHandleRotationSignal = true;
    }
    else if( !rotate && m_bHandleRotationSignal )
    {
        disconnect( this, SIGNAL( rotateUp() ), this, SLOT( iterateUpInList() ) );
        disconnect( this, SIGNAL( rotateDown() ), this, SLOT( iterateDownInList() ) );
        m_bHandleRotationSignal = false;
    }
}

void KComboBox::setCompletionMode( KGlobal::Completion mode )
{
    m_iCompletionMode = mode;
    // Always sync up KCompletion mode with ours as long as we
    // are performing completions.
    if( m_pCompObj != 0 && m_iCompletionMode != KGlobal::CompletionNone )
        m_pCompObj->setCompletionMode( m_iCompletionMode );
}

bool KComboBox::setCompletionKey( int ckey )
{
    if( m_pEdit != 0 && ( ckey >= 0 && ckey != m_iRotateDnKey && ckey != m_iRotateUpKey) )
    {
        m_iCompletionKey = ckey;
        return true;
    }
    return false;
}

bool KComboBox::setRotateUpKey( int rUpKey )
{
    if( m_pEdit != 0 && rUpKey == 0 ||
        (rUpKey > 0 && rUpKey != m_iRotateDnKey && rUpKey != m_iCompletionKey) )
    {
        m_iRotateUpKey = rUpKey;
        return true;
    }
    return false;
}

bool KComboBox::setRotateDownKey( int rDnKey )
{
    if ( m_pEdit != 0 && rDnKey == 0 ||
        (rDnKey > 0 && rDnKey != m_iRotateUpKey && rDnKey != m_iCompletionKey) )
    {
        m_iRotateDnKey = rDnKey;
        return true;
    }
    return false;
}

void KComboBox::setEnableContextMenu( bool showMenu )
{
    if( m_pEdit != 0 && m_pCompObj != 0 )
    {
        if( showMenu )
        {
            connect ( m_pContextMenu, SIGNAL( aboutToShow() ), this, SLOT( aboutToShowMenu() ) );
            m_bShowContextMenu = showMenu;
        }
        else
        {
            disconnect ( m_pContextMenu, SIGNAL(aboutToShow()), this, SLOT( aboutToShowMenu()) );
            if( m_pSubMenu != 0 )
            {
                disconnect ( m_pContextMenu, SIGNAL( highlighted( int ) ), this, SLOT( aboutToShowSubMenu( int ) ) );
                delete m_pSubMenu;
                m_iSubMenuId = -1;
                m_pSubMenu = 0;
            }
            m_bShowContextMenu = showMenu;
        }
    }
}

void KComboBox::setEnableModeChanger( bool showChanger )
{
    if ( !showChanger && m_bShowContextMenu )
    {
        if( m_pSubMenu != 0 )
        {
            disconnect( m_pContextMenu, SIGNAL( highlighted( int ) ), this, SLOT( aboutToShowSubMenu( int ) ) );
            delete m_pSubMenu;
            m_pSubMenu = 0;
        }
    }
    m_bShowModeChanger = showChanger;
}

void KComboBox::setEditText( const QString& text )
{
    if( m_pEdit != 0 && !text.isNull() )
    {
        int pos = cursorPosition();
        m_pEdit->validateAndSet( text, pos, 0, text.length() );
    }
    else
        QComboBox::setEditText( text );
}

void KComboBox::aboutToShowMenu()
{
    if( m_bShowModeChanger && m_pCompObj != 0 )
    {
        if( m_pSubMenu == 0 )
            m_pSubMenu = new QPopupMenu();
        if( m_iSubMenuId == -1 )
        {
            m_pContextMenu->insertSeparator( m_pContextMenu->count() - 1 );
            m_iSubMenuId = m_pContextMenu->insertItem( i18n("Completion Mode"), m_pSubMenu, -1, m_pContextMenu->count() - 2 );
            connect ( m_pContextMenu, SIGNAL( highlighted( int ) ), this, SLOT( aboutToShowSubMenu( int ) ) );
        }
    }
}

void KComboBox::aboutToShowSubMenu( int itemID )
{
    if( itemID == m_iSubMenuId )
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
    }
}

void KComboBox::entryChanged( const QString& text )
{
    if( m_bEmitCompletion &&
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

void KComboBox::multipleCompletions( const QStringList& )
{
    //TODO: POP-UP LISTBOX FOR MULTIPLE MATCHES
}

void KComboBox::returnKeyPressed()
{
    if( m_pEdit && m_pEdit->text().length() != 0 )
        emit returnPressed( m_pEdit->text() );
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
            setCurrentItem( index );
    }
}

void KComboBox::iterateUpInList()
{
    if( m_pCompObj != 0 )
        rotateText( m_pCompObj->previousMatch() );
}

void KComboBox::iterateDownInList()
{
    if( m_pCompObj != 0 )
        rotateText( m_pCompObj->nextMatch() );
}

void KComboBox::itemSelected( QListBoxItem* item )
{
    if( item != 0 && m_pEdit != 0 )
        m_pEdit->setSelection(0, m_pEdit->text().length() );
}


bool KComboBox::eventFilter( QObject *o, QEvent *ev )
{
    if ( o == m_pEdit )
    {
        if( ev->type() == QEvent::KeyPress )
        {
            QKeyEvent *e = (QKeyEvent *) ev;
            int key = ( m_iCompletionKey == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : m_iCompletionKey;
            if( KStdAccel::isEqual( e, key ) && m_bEmitCompletion )
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
            if( KStdAccel::isEqual( e, key ) && m_bEmitRotation )
            {
                emit rotateUp ();
                return true;
            }
            // Handles rotateDown.
            key = ( m_iRotateDnKey == 0 ) ? KStdAccel::key(KStdAccel::RotateDown) : m_iRotateDnKey;
            if( KStdAccel::isEqual( e, key ) && m_bEmitRotation )
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
                    if( m_bShowContextMenu )
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
