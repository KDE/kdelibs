/* This file is part of the KDE libraries

   Copyright (C) 1997 Sven Radej (sven.radej@iname.com)
   Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 2000 Dawit Alemayehu <adawit@earthlink.net>

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

#include <qobjcoll.h>

#include <klocale.h>
#include <kstdaccel.h>

#include "klineedit.h"
#include "klineedit.moc"


KLineEdit::KLineEdit( const QString &string, QWidget *parent, const char *name )
          : QLineEdit( string, parent, name )
{
    init();
}

KLineEdit::KLineEdit( QWidget *parent, const char *name )
          : QLineEdit( parent, name )
{
    init();
}

KLineEdit::~KLineEdit ()
{
    m_pContextMenu = 0; // Reset the pointer to NULL;
}

void KLineEdit::init()
{
    // Hack and recover the built-in popup-menu to add
    // your own item to it.  What a piece of work :))
    QObjectList *list = queryList( "QPopupMenu" );
    QObjectListIt it ( *list );
    m_pContextMenu = (QPopupMenu*) it.current();
    delete list;

    m_bEnableMenu = false;
    setEnableContextMenu( true ); // Enable the context menu by default
}

void KLineEdit::setEnableContextMenu( bool showMenu )
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

void KLineEdit::setCompletionMode( KGlobalSettings::Completion mode )
{
    // If the widgets echo mode is not Normal, no completion
    // feature will be enabled even if one is requested.
    if ( echoMode() != QLineEdit::Normal )
        mode = KGlobalSettings::CompletionNone; // override the request.

    KCompletionBase::setCompletionMode( mode );
}

void KLineEdit::rotateText( const QString& input, int dir )
{
    KCompletion* comp = completionObject();
    QString str;
    int len = text().length();
    if( hasMarkedText() && !input.isNull() )
    {
        str = text();
        if( input == str ) return; // Skip rotation if same text
        int pos = str.find( markedText() );
        int index = input.find( str.remove( pos , markedText().length() ) );
        if( index == -1 ) return;
        else if( index == 0 ) str = input;
        validateAndSet( str, cursorPosition(), pos, str.length() );
    }
    else
    {
        QStringList list = comp->items();
        if( list.count() == 0 ) return;
        int index = list.findIndex( text() );
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
        setText( str );
    }
}

void KLineEdit::makeCompletion( const QString& text )
{
    QString match = completionObject()->makeCompletion( text );
    // If no match or the same match, simply return
    // without completing.
    if( match.length() == 0 || match == text )
        return;

    if( completionMode() == KGlobalSettings::CompletionShell )
    {
        setText( match );
    }
    else
    {
        int pos = cursorPosition();
        validateAndSet( match, pos, pos, match.length() );
    }
}

void KLineEdit::connectSignals( bool handle ) const
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

void KLineEdit::selectedItem( int id )
{
    if( id == 0 ) id = KGlobalSettings::completionMode();
    setCompletionMode( (KGlobalSettings::Completion)id );
}

void KLineEdit::keyPressEvent( QKeyEvent *e )
{
    // Trap RETURN/ENTER events.  Let people connect to
    // returnPressed() and returnPressed( const QString& )
    // as needed if they were relying on this event being
    // propagated up-stream.  This is also consistent with
    // KLineEdit.
    if( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
    {
        emit returnPressed( displayText() );
    }
    // Filter key-events if EchoMode is normal & completion mode is not set to CompletionNone
    else if( echoMode() == QLineEdit::Normal && completionMode() != KGlobalSettings::CompletionNone )
    {
        bool fireSignals = emitSignals();
        KGlobalSettings::Completion mode = completionMode();
        if( mode == KGlobalSettings::CompletionAuto )
        {
            QString keycode = e->text();
            if( !keycode.isNull() && keycode.unicode()->isPrint() && fireSignals )
            {
                QLineEdit::keyPressEvent ( e );
                emit completion( text() );
                return;
            }
        }
        // Handles completion.
        KCompletion* comp = completionObject();
        int key = ( completionKey() == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion)	: completionKey();
        if( KStdAccel::isEqual( e, key ) && fireSignals )
        {
            // Emit completion if the completion mode is NOT
            // CompletionAuto and if the mode is CompletionShell,
            // the cursor is at the end of the string.
            int len = text().length();
            if( (mode == KGlobalSettings::CompletionMan && (comp != 0 && comp->lastMatch() != displayText() ) ) ||
                (mode == KGlobalSettings::CompletionShell && cursorPosition() == len && len != 0 ) )
            {
                emit completion( text() );
                return;
            }
        }
        // Handles rotateUp.
    	key = ( rotateUpKey() == 0 ) ? KStdAccel::key(KStdAccel::RotateUp) : rotateUpKey();
        if( KStdAccel::isEqual( e, key ) && fireSignals )
        {
            emit rotateUp ();
            return;
        }
        // Handles rotateDown.
	    key = ( rotateDownKey() == 0 ) ? KStdAccel::key(KStdAccel::RotateDown) : rotateDownKey();
        if( KStdAccel::isEqual( e, key ) && fireSignals)
        {
            emit rotateDown();
            return;
        }
    }
    // Let QLineEdit handle any other keys events.
    QLineEdit::keyPressEvent ( e );
}

void KLineEdit::mousePressEvent( QMouseEvent* e )
{
    if( e->button() == Qt::RightButton )
    {
        if( !m_bEnableMenu )
            return;
    }
    QLineEdit::mousePressEvent( e );
}
