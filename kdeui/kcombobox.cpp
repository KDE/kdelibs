/* This file is part of the KDE libraries

   Copyright (c) 2000 Dawit Alemayehu <adawit@kde.org>

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
	// Do not put text in the list, but do not loose
	// it either when rotating up.
	m_strCurrentText = QString::null;
	
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

    	if( mode == KGlobalSettings::CompletionShell &&
    		comp->hasMultipleMatches() && text != comp->lastMatch() )
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

void KComboBox::rotateText( KCompletionBase::KeyBindingType type )
{
    if( m_pEdit != 0 )
    {
    	// Support for rotating through highlighted text!  This means
		// that if there are multiple matches for the portion of text
		// entered into the combo box,  one can simply rotate through
		// all the other possible matches using the previous/next match
		// keys.
		KCompletion* comp = completionObject();
		if( comp &&
			( type == KCompletionBase::PrevCompletionMatch ||
			  type == KCompletionBase::NextCompletionMatch ) )
		{
			QString input = ( type == KCompletionBase::PrevCompletionMatch ) ? comp->previousMatch() : comp->nextMatch();
			// Ignore rotating to the same text			
            if( input.isNull() || input == m_pEdit->text() ) return;
            int pos = cursorPosition();
            if( m_pEdit->hasMarkedText() )
				m_pEdit->validateAndSet( input, pos, pos, input.length() );
			else
				m_pEdit->setText( input );
        }
        else
        {
        	// History list rotation feature:  The code below supports
        	// *nix shell like history list rotation.  This feature is
        	// only available when the insertion policy into the listbox
        	// of the combo widget it set to either "AtTop" or "AtBottom".
        	// Other insertion cannot support save the last written text
        	// feature of *nix like shells.        	
        	int index = -1;        	
			QComboBox::Policy policy = insertionPolicy();
			if( m_strCurrentText.isNull() &&
				( policy == QComboBox::AtTop ||
				  policy == QComboBox::AtBottom ) )
			{
				if( type == KCompletionBase::RotateUp )
				{
					if( policy == QComboBox::AtTop )
						index = 0;
					else if( policy == QComboBox::AtBottom )
						index = count() - 1;
				}
                debug( "Current item is : %i\tTotal items in list : %i", index, count() );
				if( index == 0 || index == count() - 1 )
				{
					m_strCurrentText = m_pEdit->text();  //Store the current text.
				    debug( "Storing the current text : %s ", m_strCurrentText.latin1() );					
				}
			}
			else			
			{
				if( type == KCompletionBase::RotateUp )
				{
					if( policy == QComboBox::AtTop )
						index = currentItem() + 1;
					else
						index = currentItem() - 1;
				}
				else if( type == KCompletionBase::RotateDown )
				{
					if( policy == QComboBox::AtTop )
						index = currentItem() - 1;
					else
						index = currentItem() + 1;
				}
			
			}	
			if( index > -1 && index < count() )
			{			
				setCurrentItem( index );
			}
			else
			{
				if( !m_strCurrentText.isNull() &&
					( ( policy == QComboBox::AtTop && index < 0 ) ||
					  ( policy == QComboBox::AtBottom && index >= count() ) ) )
				{
				    debug( "Setting the text editor to : %s", m_strCurrentText.latin1() );
					m_pEdit->setText( m_strCurrentText );
					m_strCurrentText = QString::null;				
				}
			}
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
        connect( this, SIGNAL( previousMatch( KeyBindingType ) ), this, SLOT( rotateText( KeyBindingType ) ) );
        connect( this, SIGNAL( nextMatch( KeyBindingType ) ), this, SLOT( rotateText( KeyBindingType ) ) );
        connect( this, SIGNAL( rotateUp( KeyBindingType ) ), this, SLOT( rotateText( KeyBindingType ) ) );
        connect( this, SIGNAL( rotateDown( KeyBindingType ) ), this, SLOT( rotateText( KeyBindingType ) ) );
    }
    else if( !handle && handleSignals() )
    {
        disconnect( this, SIGNAL( completion( const QString& ) ), this, SLOT( makeCompletion( const QString& ) ) );
        disconnect( this, SIGNAL( previousMatch( KeyBindingType ) ), this, SLOT( rotateText( KeyBindingType ) ) );
        disconnect( this, SIGNAL( nextMatch( KeyBindingType ) ), this, SLOT( rotateText( KeyBindingType ) ) );
        disconnect( this, SIGNAL( rotateUp( KeyBindingType ) ), this, SLOT( rotateText( KeyBindingType ) ) );
        disconnect( this, SIGNAL( rotateDown( KeyBindingType ) ), this, SLOT( rotateText( KeyBindingType ) ) );
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
	        KeyBindingMap keys = getKeyBindings();
    	    int key = ( keys[TextCompletion] == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : keys[TextCompletion];
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
            key = ( keys[RotateUp] == 0 ) ? KStdAccel::key(KStdAccel::RotateUp) : keys[RotateUp];
            if( KStdAccel::isEqual( e, key ) && fireSignals )
            {
                emit rotateUp( KCompletionBase::RotateUp );
                e->accept();
                return;
            }
            // Handles rotateDown.
            key = ( keys[RotateDown] == 0 ) ? KStdAccel::key(KStdAccel::RotateDown) : keys[RotateDown];
            if( KStdAccel::isEqual( e, key ) && fireSignals )
            {
                emit rotateDown( KCompletionBase::RotateDown );
                e->accept();
                return;
            }
            // Handles rotateUp.
            key = ( keys[PrevCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::PrevCompletion) : keys[PrevCompletionMatch];
            if( KStdAccel::isEqual( e, key ) && fireSignals )
            {
                emit previousMatch( KCompletionBase::PrevCompletionMatch );
                e->accept();
                return;
            }
            // Handles rotateDown.
            key = ( keys[NextCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::NextCompletion) : keys[NextCompletionMatch];
            if( KStdAccel::isEqual( e, key ) && fireSignals )
            {
                emit nextMatch( KCompletionBase::NextCompletionMatch );
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
