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
		QString match;
    	int pos = cursorPosition();
		KCompletion *comp = completionObject();
    	KGlobalSettings::Completion mode = completionMode();
    	
    	if( comp->hasMultipleMatches() && mode == KGlobalSettings::CompletionShell )
	    	match = comp->nextMatch();
	    else
   	    	match = comp->makeCompletion( text );
 	
        // If no match or the same text simply return without completing.
        if( match.isNull() || match == text )  return;

		int index = ( match.length() == 0 ) ? -1 : listBox()->index( listBox()->findItem( match ) );
		if( index > -1 )  setCurrentItem( index );
			
		if( mode == KGlobalSettings::CompletionAuto ||
			mode == KGlobalSettings::CompletionMan )
        {
            m_pEdit->setSelection( pos, match.length() );
            m_pEdit->setCursorPosition( pos );
        }
	}
    else if( m_pEdit == 0 )
    {
		if( text.isNull() ) return;
		int index = listBox()->index( listBox()->findItem( text ) );
		if( index >= 0 ) setCurrentItem( index );
    }

}

void KComboBox::rotateText( KCompletionBase::RotationEvent dir )
{
    if( m_pEdit != 0 )
    {
    	// Support for rotating through highlighted text!!  This means
    	// a person can rotate through all combinations of car, class,
    	// coffee, cookie, club by just pressing the rotation keys.
		KCompletion* comp = completionObject();
        if( m_pEdit->hasMarkedText() )
        {
			QString str = m_pEdit->text();
			QString input = ( dir == KCompletionBase::UpKeyEvent ) ? comp->previousMatch() : comp->nextMatch();
            if( input == str ) return; // Ignore rotating to the same text
            int pos = str.find( m_pEdit->markedText() );
            int index = input.find( str.remove( pos , m_pEdit->markedText().length() ) );
            if( index == -1 ) return;
            else if( index == 0 ) str = input;
            m_pEdit->validateAndSet( str, cursorPosition(), pos, str.length() );
        }
        else
        {
        	QString input = m_pEdit->text();
			QComboBox::Policy policy = insertionPolicy();
			// Workaround a Qt bug!! findItem in QListBox returns
			// the first element even if the argument is an empty
			// string.  This is not correct according their documentations.			
			int index = ( input.length() == 0 ) ? -1 : listBox()->index( listBox()->findItem( input ) );
			if( index == -1 )
			{
				// Only allow a previous match key press
				// match since we do not yet have a next
				// match at this point yet.
				if( dir == KCompletionBase::UpKeyEvent )
				{
					if( policy == QComboBox::AtTop )
						index = 0;
					else if( policy == QComboBox::AtBottom )
						index = listBox()->count() - 1;
					else
						// TODO: Figure out what to do with other insertion policies
						// For now just start from the currently selected item.
						index = currentItem();
				}
			}
			else
			{
				if( dir == KCompletionBase::UpKeyEvent )
				{
					if( policy == QComboBox::AtBottom )
						index -= 1;
					else
						index += 1;
				}
				else if( dir == KCompletionBase::DownKeyEvent )
				{
					if( policy == QComboBox::AtBottom )
						index += 1;
					else
						index -= 1;
				}
			
			}	
			// List is made so that it will not iterate back through			
			if( index > -1 && index < (int)listBox()->count() )
				setCurrentItem( index );
        }
    }
}

void KComboBox::itemSelected( QListBoxItem* item )
{
    if( item != 0 && m_pEdit != 0 )
    {
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

void KComboBox::selectedItem( int id )
{
    if( id == 0 ) id = KGlobalSettings::completionMode();
    setCompletionMode( (KGlobalSettings::Completion)id );
}

void KComboBox::keyPressEvent ( QKeyEvent * e )
{
	// Disable Qt's hard coded rotate-up key binding!  It is
	// configurable in KDE.
	if( e->state() == Qt::AltButton && e->key() == Qt::Key_Up )
		return;
    if( m_pEdit != 0 && m_pEdit->hasFocus() )
    {
        KGlobalSettings::Completion mode = completionMode();
        // On Return pressed event, emit both returnPressed( const QString& )
        // and returnPressed() signals
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
	                 m_pEdit->cursorPosition() == (int) m_pEdit->text().length() ) )
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
