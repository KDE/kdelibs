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

#include <qclipboard.h>

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
}

void KLineEdit::init()
{
    // Enable the context menu by default.
    setContextMenuEnabled( true );
}

void KLineEdit::setCompletionMode( KGlobalSettings::Completion mode )
{
    // If the widgets echo mode is not Normal, no completion
    // feature will be enabled even if one is requested.
    if ( echoMode() != QLineEdit::Normal )
        mode = KGlobalSettings::CompletionNone; // Override the request.

    KCompletionBase::setCompletionMode( mode );
}

void KLineEdit::rotateText( KCompletionBase::KeyBindingType type )
{
   	KCompletion* comp = compObj();
	if( comp &&
		(type == KCompletionBase::PrevCompletionMatch ||
		 type == KCompletionBase::NextCompletionMatch ) )
	{
		QString input = (type == KCompletionBase::PrevCompletionMatch) ? comp->previousMatch() : comp->nextMatch();
		// Skip rotation if previous/next match is null or the same text.
        if( input.isNull() || input == displayText() )
        {
            return;
        }
        bool marked = hasMarkedText(); // Note if current text has been marked
		int pos = cursorPosition(); // Note the current cursor position
		setText( input );
        if( marked )
        {
			setSelection( pos, input.length() );
			setCursorPosition( pos );				
		}
    }
}

void KLineEdit::makeCompletion( const QString& text )
{
	KCompletion *comp = compObj();
	if( !comp && text.length() == 0 )
	{
	    return;  // No completion object...
	}
	
	QString match;
   	int pos = cursorPosition();
   	KGlobalSettings::Completion mode = completionMode();
    	
	if( mode == KGlobalSettings::CompletionShell &&	
	   	comp->hasMultipleMatches() && text != comp->lastMatch() )
	{
    	match = comp->nextMatch();
 	}
    else
    {
	    match = comp->makeCompletion( text );
	}    	
    // If no match or the same match, simply return
    // without completing.
    if( match.isNull() || match == text )
    {
	    // Put the cursor at the end when in semi-automatic
	    // mode and completion is invoked with the same text.
    	if( mode == KGlobalSettings::CompletionMan )
    	{
    		end( false );
    	}
        return;
    }
    
    setText( match );
	if( mode == KGlobalSettings::CompletionAuto ||
		mode == KGlobalSettings::CompletionMan )
	{
		setSelection( pos, match.length() );
		setCursorPosition( pos );
	}
}

void KLineEdit::connectSignals( bool handle ) const
{
    if( handle && !handleSignals() )
    {
        connect( this, SIGNAL( completion( const QString& ) ), this, SLOT( makeCompletion( const QString& ) ) );
        connect( this, SIGNAL( previousMatch( KCompletionBase::KeyBindingType ) ), this, SLOT( rotateText( KCompletionBase::KeyBindingType ) ) );
        connect( this, SIGNAL( nextMatch( KCompletionBase::KeyBindingType ) ), this, SLOT( rotateText( KCompletionBase::KeyBindingType ) ) );
    }
    else if( !handle && handleSignals() )
    {
        disconnect( this, SIGNAL( completion( const QString& ) ), this, SLOT( makeCompletion( const QString& ) ) );
        disconnect( this, SIGNAL( previousMatch( KCompletionBase::KeyBindingType ) ), this, SLOT( rotateText( KCompletionBase::KeyBindingType ) ) );
        disconnect( this, SIGNAL( nextMatch( KCompletionBase::KeyBindingType ) ), this, SLOT( rotateText( KCompletionBase::KeyBindingType ) ) );
    }
}

void KLineEdit::keyPressEvent( QKeyEvent *e )
{
    // Trap RETURN/ENTER events.  Let people connect to
    // returnPressed() and returnPressed( const QString& )
    // as needed if they were relying on this event being
    // propagated up-stream.  This is also consistent with
    // KComboBox.
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
                emit completion( displayText() );
                return;
            }
        }
        // Handles completion.
        KeyBindingMap keys = getKeyBindings();
        int key = ( keys[TextCompletion] == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : keys[TextCompletion];
        if( KStdAccel::isEqual( e, key ) && fireSignals )
        {
            // Emit completion if the completion mode is NOT
            // CompletionAuto and if the mode is CompletionShell,
            // the cursor is at the end of the string.
            int len = displayText().length();
            if( (mode == KGlobalSettings::CompletionMan ||
                (mode == KGlobalSettings::CompletionShell &&
                cursorPosition() == len && len != 0 ) ) )
            {
                emit completion( displayText() );
                return;
            }
        }
        // Handles previous match
    	key = ( keys[PrevCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::PrevCompletion) : keys[PrevCompletionMatch];
        if( KStdAccel::isEqual( e, key ) && fireSignals )
        {
            emit previousMatch( KCompletionBase::PrevCompletionMatch );
            return;
        }
        // Handles next match
	    key = ( keys[NextCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::NextCompletion) : keys[NextCompletionMatch];
        if( KStdAccel::isEqual( e, key ) && fireSignals)
        {
            emit nextMatch( KCompletionBase::NextCompletionMatch );
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
        // Return if popup menu is not enabled !!
        if( !m_bEnableMenu )
            return;
            
        QPopupMenu *popup = new QPopupMenu( this );
        insertDefaultMenuItems( popup );
        bool flag = ( echoMode()==QLineEdit::Normal && !isReadOnly() );
        bool allMarked = ( markedText().length() == text().length() );        
        popup->setItemEnabled( KCompletionBase::Cut, flag && hasMarkedText() );
        popup->setItemEnabled( KCompletionBase::Copy, flag && hasMarkedText() );
        popup->setItemEnabled( KCompletionBase::Clear, flag && (text().length() > 0) );        
        popup->setItemEnabled( KCompletionBase::Paste, flag &&
                               (bool)QApplication::clipboard()->text().length() );
        popup->setItemEnabled( KCompletionBase::Unselect, hasMarkedText() );
        popup->setItemEnabled( KCompletionBase::SelectAll, flag && hasMarkedText() && !allMarked );
        
        int result = popup->exec( e->globalPos() );
        delete popup;
                
        if( result == KCompletionBase::Cut )
            cut();
        else if( result == KCompletionBase::Copy )
            copy();
        else if( result == KCompletionBase::Paste )
            paste();
        else if( result == KCompletionBase::Clear )
            clear();
        else if( result == KCompletionBase::Unselect )
            deselect();
        else if( result == KCompletionBase::SelectAll )
            selectAll();
        else if( result == KCompletionBase::Default )
            setCompletionMode( KGlobalSettings::completionMode() );            
        else if( result == KCompletionBase::NoCompletion )
            setCompletionMode( KGlobalSettings::CompletionNone );
        else if( result == KCompletionBase::AutoCompletion )
            setCompletionMode( KGlobalSettings::CompletionAuto );
        else if( result == KCompletionBase::SemiAutoCompletion )
            setCompletionMode( KGlobalSettings::CompletionMan );
        else if( result == KCompletionBase::ShellCompletion )
            setCompletionMode( KGlobalSettings::CompletionShell );
        return;      
    }
    QLineEdit::mousePressEvent( e );
}
