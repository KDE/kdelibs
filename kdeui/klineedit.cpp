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
#include <kdebug.h>

#include "klineedit.h"
#include "klineedit.moc"


class KLineEdit::KLineEditPrivate
{
public:
    bool grabReturnKeyEvents;
};


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

void KLineEdit::setCompletedText( const QString& text, bool marked )
{
    int pos = cursorPosition();
    setText( text );
    if( marked )
    {
        setSelection( pos, text.length() );
	    setCursorPosition( pos );
    }
}

void KLineEdit::setCompletedText( const QString& text )
{
    KGlobalSettings::Completion mode = completionMode();
    bool marked = ( mode == KGlobalSettings::CompletionAuto ||
                    mode == KGlobalSettings::CompletionMan );
    setCompletedText( text, marked );
}

void KLineEdit::rotateText( KCompletionBase::KeyBindingType type )
{
    KCompletion* comp = compObj();
    if( comp &&
       (type == KCompletionBase::PrevCompletionMatch ||
	type == KCompletionBase::NextCompletionMatch ) )
    {
       QString input = (type == KCompletionBase::PrevCompletionMatch) ? comp->previousMatch() : comp->nextMatch();
       // Skip rotation if previous/next match is null or the same tex
       if( input.isNull() || input == displayText() )
       {
	  return;
       }
       setCompletedText( input, hasMarkedText() );
    }
}

void KLineEdit::makeCompletion( const QString& text )
{

    KCompletion *comp = compObj();
    if( !comp )
    {
       return;  // No completion object...
    }

    QString match = comp->makeCompletion( text );

    // If no match or the same match, simply return
    // without completing.
    KGlobalSettings::Completion mode = completionMode();
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
    bool marked = ( mode == KGlobalSettings::CompletionAuto ||
                    mode == KGlobalSettings::CompletionMan );
    setCompletedText( match, marked );
}

void KLineEdit::connectSignals( bool handle ) const
{
    if( handle && !handleSignals() )
    {
        connect( this, SIGNAL( completion( const QString& ) ),
                 this, SLOT( makeCompletion( const QString& ) ) );
        connect( this, SIGNAL( textRotation( KCompletionBase::KeyBindingType ) ),
                 this, SLOT( rotateText( KCompletionBase::KeyBindingType ) ) );
    }
    else if( !handle && handleSignals() )
    {
        disconnect( this, SIGNAL( completion( const QString& ) ),
                    this, SLOT( makeCompletion( const QString& ) ) );
        disconnect( this, SIGNAL( textRotation( KCompletionBase::KeyBindingType ) ),
                    this, SLOT( rotateText( KCompletionBase::KeyBindingType ) ) );
    }
}

void KLineEdit::keyPressEvent( QKeyEvent *e )
{
    // Filter key-events if EchoMode is normal & completion mode is not set to CompletionNone
    if( echoMode() == QLineEdit::Normal &&
        completionMode() != KGlobalSettings::CompletionNone )
    {
        bool fireSignals = emitSignals();
        KGlobalSettings::Completion mode = completionMode();
        if( mode == KGlobalSettings::CompletionAuto )
        {
            QString keycode = e->text();
            if( !keycode.isNull() && keycode.unicode()->isPrint() && fireSignals )
            {
                QLineEdit::keyPressEvent ( e );
                QString txt = text();
                if( !hasMarkedText() && txt.length() )
                {
                    kdDebug() << "Key Pressed: " << keycode.latin1() << endl;
                    kdDebug() << "Current text: " << txt.latin1() << endl;
                    emit completion( txt );
                }
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
            emit textRotation( KCompletionBase::PrevCompletionMatch );
            return;
        }
        // Handles next match
	    key = ( keys[NextCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::NextCompletion) : keys[NextCompletionMatch];
        if( KStdAccel::isEqual( e, key ) && fireSignals)
        {
            emit textRotation( KCompletionBase::NextCompletionMatch );
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
        popup->insertItem( i18n( "Cut" ), Cut );
        popup->insertItem( i18n( "Copy" ), Copy );
        popup->insertItem( i18n( "Clear" ), Clear );
        popup->insertItem( i18n( "Paste" ), Paste );
        // Create and insert the completion sub-menu iff
        // a completion object is present.
        if( compObj() )
        {
            QPopupMenu* subMenu = new QPopupMenu( popup );
            KGlobalSettings::Completion mode = completionMode();
            subMenu->insertItem( i18n("None"), NoCompletion );
            subMenu->setItemChecked( NoCompletion, mode == KGlobalSettings::CompletionNone );
            subMenu->insertItem( i18n("Manual"), ShellCompletion );
            subMenu->setItemChecked( ShellCompletion, mode == KGlobalSettings::CompletionShell );
            subMenu->insertItem( i18n("Automatic"), AutoCompletion );
            subMenu->setItemChecked( AutoCompletion, mode == KGlobalSettings::CompletionAuto );
            subMenu->insertItem( i18n("Semi-Automatic"), SemiAutoCompletion );
            subMenu->setItemChecked( SemiAutoCompletion, mode == KGlobalSettings::CompletionMan );        
            if( mode != KGlobalSettings::completionMode() )
            {
                subMenu->insertSeparator();
                subMenu->insertItem( i18n("Default"), Default );
            }
            popup->insertSeparator();                
            popup->insertItem( i18n("Completion"), subMenu );
        }
        popup->insertSeparator();
        popup->insertItem( i18n( "Unselect" ), Unselect );
        popup->insertItem( i18n( "Select All" ), SelectAll );
            
        bool flag = ( echoMode()==QLineEdit::Normal && !isReadOnly() );
        bool allMarked = ( markedText().length() == text().length() );
        popup->setItemEnabled( Cut, flag && hasMarkedText() );
        popup->setItemEnabled( Copy, flag && hasMarkedText() );
        popup->setItemEnabled( Paste, flag && (bool)QApplication::clipboard()->text().length() );
        popup->setItemEnabled( Clear, flag && ( text().length() > 0) );
        popup->setItemEnabled( Unselect, hasMarkedText() );
        popup->setItemEnabled( SelectAll, flag && hasMarkedText() && !allMarked );
            
        int result = popup->exec( e->globalPos() );
        delete popup;

        if( result == Cut )
            cut();
        else if( result == Copy )
            copy();
        else if( result == Paste )
            paste();
        else if( result == Clear )
            clear();
        else if( result == Unselect )
            deselect();
        else if( result == SelectAll )
            selectAll();
        else if( result == Default )
            setCompletionMode( KGlobalSettings::completionMode() );
        else if( result == NoCompletion )
            setCompletionMode( KGlobalSettings::CompletionNone );
        else if( result == AutoCompletion )
            setCompletionMode( KGlobalSettings::CompletionAuto );
        else if( result == SemiAutoCompletion )
            setCompletionMode( KGlobalSettings::CompletionMan );
        else if( result == ShellCompletion )
            setCompletionMode( KGlobalSettings::CompletionShell );

        return;
    }
    QLineEdit::mousePressEvent( e );
}

bool KLineEdit::eventFilter( QObject* o, QEvent* ev )
{    
    if ( o == this && ev->type() == QEvent::KeyPress )
    {
	QKeyEvent *e = static_cast<QKeyEvent *>( ev );
	if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
	{
	    // Emit both returnPressed(const QString&)
	    // and returnPressed() signals            
            emit returnPressed( displayText() );
            if( d->grabReturnKeyEvents )
            {
                // Because we trap the event we have
                // to emit this signal as well
                // emit QLineEdit::returnPressed();
                return true;
            }
	}
    }
    return QLineEdit::eventFilter( o, ev );
}


void KLineEdit::setTrapReturnKey( bool grab )
{
    d->grabReturnKeyEvents = grab;
}

bool KLineEdit::trapReturnKey() const
{
    return d->grabReturnKeyEvents;
}
