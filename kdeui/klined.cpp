/* This file is part of the KDE libraries

   Copyright (C) 1997 Sven Radej (sven.radej@iname.com)
   Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999 Dawit Alemayehu <adawit@earthlink.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
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

#include <qclipboard.h>

#include <klocale.h>
#include <kstdaccel.h>

#include "klined.h"
#include "klined.moc"


KLineEdit::KLineEdit( const QString &string,
                      QWidget *parent,
                      const char *name,
                      bool showMenu,
                      bool showChanger ) : QLineEdit (string, parent, name)
{
    initialize( showMenu, showChanger );
}

KLineEdit::KLineEdit( QWidget *parent,
                      const char *name,
                      bool showMenu,
                      bool showChanger ) : QLineEdit (parent, name)
{
    initialize( showMenu, showChanger );
}

KLineEdit::~KLineEdit ()
{
    delete contextMenu;
    delete subMenu;
    if( !m_bIsForeignCompObj )
        delete comp;
}

void KLineEdit::setCompletionObject ( KCompletion* obj, bool autoDelete )
{
    // Ignore NULL assignments.  If programmer needs to delete
    // the completion object, (s)he should invoke disableCompletion().
    if( obj == 0 )
        return;

    comp = obj;
    m_bIsForeignCompObj = autoDelete;
}

void KLineEdit::disableCompletion()
{
    delete comp;
    comp = 0;
}

void KLineEdit::enableCompletion()
{
    if( comp == 0 )
    {
        comp = new KCompletion();
        setCompletionMode( m_iCompletionMode );  // forces a completion mode sync w/ KCompletion.
    }
}

void KLineEdit::showModeChanger( bool showChanger )
{
    if ( !showChanger )
    {
        if( subMenu != 0 )
        {
            delete subMenu;
            subMenu = 0;
        }
    }
    m_bShowModeChanger = showChanger;
}

void KLineEdit::showContextMenu( bool showMenu )
{
    if( showMenu )
    {
        if( contextMenu == 0 )
        {
            contextMenu = new QPopupMenu();
            connect( contextMenu, SIGNAL(aboutToShow()), this, SLOT(slotShowContextMenu()) );
            connect( contextMenu, SIGNAL( highlighted( int ) ), this, SLOT( slotShowSubMenu( int ) ) );
        }
    }
    else
    {
        if( contextMenu != 0 )
        {
            disconnect ( contextMenu, SIGNAL(aboutToShow()), this, SLOT(slotShowContextMenu()) );
            disconnect( contextMenu, SIGNAL( highlighted( int ) ), this, SLOT( slotShowSubMenu( int ) ) );
            delete contextMenu;
            contextMenu = 0;
        }
    }
    m_bShowContextMenu = showMenu;
}

void KLineEdit::useGlobalSettings()
{
    m_iCompletionKey = 0;
    m_iRotateUpKey = 0;
    m_iRotateDnKey = 0;
}

void KLineEdit::setCompletionMode( KGlobal::Completion mode )
{
    // If the widgets echo mode is not Normal, no completion
    // feature will be enabled even if one is requested.
    if ( echoMode() != QLineEdit::Normal )
        mode = KGlobal::CompletionNone; // override the request.

    m_iCompletionMode = mode;

    // Always sync up KCompletion mode with ours as long as we
    // are performing completions.
    if( comp != 0 && m_iCompletionMode != KGlobal::CompletionNone )
    {
        comp->setCompletionMode( m_iCompletionMode );
    }
}

bool KLineEdit::setCompletionKey( int ckey )
{
    if( ckey == 0 || (ckey > 0 && ckey != m_iRotateUpKey && ckey != m_iRotateDnKey) )
    {
        m_iCompletionKey = ckey;
        return true;
    }
    return false;
}

bool KLineEdit::setRotateUpKey( int rUpKey )
{
    if( rUpKey == 0 || (rUpKey > 0 && rUpKey != m_iRotateDnKey && rUpKey != m_iCompletionKey) )
    {
        m_iRotateUpKey = rUpKey;
        return true;
    }
    return false;
}

bool KLineEdit::setRotateDownKey( int rDnKey )
{
    if ( rDnKey == 0 || (rDnKey > 0 && rDnKey != m_iRotateUpKey && rDnKey != m_iCompletionKey) )
    {
        m_iRotateDnKey = rDnKey;
        return true;
    }
    return false;
}

void KLineEdit::slotShowSubMenu( int itemID )
{
    if( itemID == subMenuID )
    {
        int id;
        subMenu->clear();
        id = subMenu->insertItem( i18n("None"), this, SLOT(slotModeNone()) );
        subMenu->setItemChecked(id, m_iCompletionMode == KGlobal::CompletionNone );
        id = subMenu->insertItem( i18n("Manual"), this, SLOT(slotModeShell()) );
        subMenu->setItemChecked(id, m_iCompletionMode == KGlobal::CompletionShell );
        id = subMenu->insertItem( i18n("Automatic"), this, SLOT(slotModeAuto()) );
        subMenu->setItemChecked(id, m_iCompletionMode == KGlobal::CompletionAuto );
        id = subMenu->insertItem( i18n("Semi-Automatic"), this, SLOT(slotModeManual()) );
        subMenu->setItemChecked(id, m_iCompletionMode == KGlobal::CompletionMan );
    }
}

void KLineEdit::slotShowContextMenu()
{
    int id;
    bool isNotEmpty = ( text().length() != 0 );
    bool isNormal = (echoMode() == QLineEdit::Normal);
    contextMenu->clear();
    id = contextMenu->insertItem( i18n("Cut"), this, SLOT(slotCut()) );
    contextMenu->setItemEnabled( id, hasMarkedText() && isNormal );
    id = contextMenu->insertItem( i18n("Copy"), this, SLOT(slotCopy()) );
    contextMenu->setItemEnabled( id, hasMarkedText() && isNormal );
    id = contextMenu->insertItem( i18n("Paste"), this, SLOT(slotPaste()) );
    // contextMenu->setItemEnabled( id, );
    id = contextMenu->insertItem( i18n("Clear"), this, SLOT(slotClear()) );
    contextMenu->setItemEnabled( id, isNotEmpty );
    contextMenu->insertSeparator();
    if( m_bShowModeChanger && comp != 0 )
    {
        if( subMenu == 0 )
            subMenu = new QPopupMenu();
        subMenu->clear();
        // Dummy place holder so that "-->" is shown !!!
        subMenu->insertItem( QString::null );
        subMenuID = contextMenu->insertItem( i18n("Completion Mode"), subMenu );
        contextMenu->insertSeparator();
    }
    id = contextMenu->insertItem( i18n("Select All"), this, SLOT(slotSelect()) );
    contextMenu->setItemEnabled( id, isNotEmpty );
    id = contextMenu->insertItem( i18n("Unselect"), this, SLOT(slotUnselect()) );
    contextMenu->setItemEnabled( id, hasMarkedText() );
}

void KLineEdit::rotateText( const QString& input )
{
    if( input.length() == 0 || input == text() )
        return;

    if( m_iCompletionMode != KGlobal::CompletionNone &&
	    comp != 0 && comp->hasMultipleMatches() )
    {
        if( m_iCompletionMode == KGlobal::CompletionShell )
        {
            setText( input );
        }
        else
        {
            int pos = cursorPosition();
            int len = input.length();
            validateAndSet( input, pos, pos, len );
            prevlen = len;
            prevpos = pos;
        }
    }
}

void KLineEdit::slotRotateUp()
{
    rotateText( comp->previousMatch() );
}

void KLineEdit::slotRotateDown()
{
    rotateText( comp->nextMatch() );
}

void KLineEdit::slotReturnPressed()
{
    // When the user presses enter, if the completion mode is not
    // CompletionNone, the input has changed since the last time
    // the ENTER key is pressed, and completion has not been diabled
    // attempt to add the text to KCompletion's list.
    if( m_bHasInputChanged && comp != 0 &&
        m_iCompletionMode != KGlobal::CompletionNone )
    {
        // add the text to KCompletion's list.
        comp->addItem ( text() );
        // Set the input changed flag to false.
        m_bHasInputChanged = false;
    }
}

void KLineEdit::slotTextChanged( const QString& text )
{
    // Set the input changed flag to true.
    m_bHasInputChanged = true;
    if( m_iCompletionMode == KGlobal::CompletionAuto &&
        echoMode() == QLineEdit::Normal )
    {
        int pos = cursorPosition();
        int len = text.length();
        if( pos > prevpos && len >= prevlen )
            emit completion( text );
        prevpos = pos;
        prevlen = len;
    }
}

void KLineEdit::slotMakeCompletion( const QString& text )
{
    if( comp != 0 )
    {
        QString match = comp->makeCompletion( text );

        // If no match or the same match, simply return
        // without completing.
        if( match.isNull() || match == text )
            return;

        if ( m_iCompletionMode == KGlobal::CompletionShell )
        {
            setText( match );
            // TODO : deal with partial matches (multiple possible completions)
        }
        else
        {
            prevlen = match.length();
            prevpos = cursorPosition();
            validateAndSet( match, prevpos, prevpos, prevlen );
        }
    }
}

void KLineEdit::initialize( bool showMenu, bool showChanger )
{
    // Determines whether the user is simply pressing
    // return without changing text.
    m_bHasInputChanged = false;

    // Delete any internal reference to KCompletion object
    //
    m_bIsForeignCompObj = false;
    // Initialize all key-bindings to 0 by default so that
    // the event filter will use the global settings.
    m_iCompletionKey = 0;
    m_iRotateUpKey = 0;
    m_iRotateDnKey = 0;

    // Initalize Variables used in auto-completion mode.
    // These values greatly simplify the logic used to
    // determine whether completion is needed when the
    // widget is in in auto-completion mode.
    prevpos = 0; // keeps cursor position whenever it changes.
    prevlen = 0; // keeps length of text as it changes.

    // Initialize the context Menu.  By default the popup
    // menu as well as the mode switching entry are enabled.
    contextMenu = 0;
    subMenu = 0;
    showContextMenu( showMenu );
    showModeChanger( showChanger );

    // Assign the default completion type to use.
    m_iCompletionMode = KGlobal::completionMode();

    // Initialize the pointer to the completion object.
    comp = 0;

    // Connect the signals and slots.
    connect( this, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotTextChanged( const QString& ) ) );
    connect( this, SIGNAL( completion( const QString& ) ), this, SLOT( slotMakeCompletion( const QString& ) ) );
    connect( this, SIGNAL( returnPressed() ), this, SLOT( slotReturnPressed() ) );
    connect( this, SIGNAL( rotateUp() ), this, SLOT( slotRotateUp() ) );
    connect( this, SIGNAL( rotateDown() ), this, SLOT( slotRotateDown() ) );
}

void KLineEdit::keyPressEvent( QKeyEvent *ev )
{
    // Filter key-events if EchoMode is normal and
    // the completion mode is not CompletionNone.
    if(  m_iCompletionMode != KGlobal::CompletionNone &&
         echoMode() == QLineEdit::Normal )
    {
        // Handles completion.
        int key = ( m_iCompletionKey == 0 ) ? KStdAccel::completion() : m_iCompletionKey;
        if( KStdAccel::isEqual( ev, key ) )
        {
            // Emit completion if the completion mode is NOT
            // CompletionAuto and if the mode is CompletionShell,
            // the cursor is at the end of the string.
            int len = text().length();
            if( m_iCompletionMode == KGlobal::CompletionMan ||
                (m_iCompletionMode == KGlobal::CompletionShell &&
                len != 0 && len == cursorPosition() ) )
            {
                emit completion( text() );
                return;
            }
        }
        // Handles rotateUp.
        key = ( m_iRotateUpKey == 0 ) ? KStdAccel::rotateUp() : m_iRotateUpKey;
        if( KStdAccel::isEqual( ev, key ) )
        {
            emit rotateUp ();
            return;
        }
        // Handles rotateDown.
        key = ( m_iRotateDnKey == 0 ) ? KStdAccel::rotateDown() : m_iRotateDnKey;
        if( KStdAccel::isEqual( ev, key ) )
        {
            emit rotateDown();
            return;
        }
    }
    // Let QLineEdit handle the other keys.
    QLineEdit::keyPressEvent ( ev );
    // Always update the position holder if the user
    // pressed the END key in auto completion mode.
    if( m_iCompletionMode == KGlobal::CompletionAuto )
    {
        int pos = cursorPosition();
        int len = text().length();
        if( prevpos != pos && pos == len )
            prevpos = pos;
    }
}

void KLineEdit::mousePressEvent( QMouseEvent *ev )
{
    if( m_bShowContextMenu )
    {
        if( ev->button() == RightButton )
        {
            contextMenu->popup ( QCursor::pos() );
        }
    }
}
