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

#include <qclipboard.h>

#include <klocale.h>
#include <kstdaccel.h>

#include "kcombobox.h"
#include "kcombobox.moc"


KComboBox::KComboBox( QWidget *parent, const char *name )
          :QComboBox( parent, name )
{
    init();
}

KComboBox::KComboBox( bool rw, QWidget *parent, const char *name )
          :QComboBox( rw, parent, name )
{
    if ( rw )
    {
        m_pEdit = QComboBox::lineEdit();
        m_pEdit->installEventFilter( this );
    }
    init();
}

KComboBox::~KComboBox()
{
    if( !m_pEdit )
    {
        m_pEdit->removeEventFilter( this );
    }
}

void KComboBox::init()
{
	// Do not put text in the list, but do not loose
	// it either when rotating up.
	m_strCurrentText = QString::null;
	
    // Permanently set some parameters in the parent object.
    QComboBox::setAutoCompletion( false );

    // Initialize enable popup menu to false.
    // Below it will be enabled if the widget
    // is editable.
    m_bEnableMenu = false;

    // Enable context menu by default if widget
    // is editable.
    setContextMenuEnabled( true );

    // Connect the signals and slots.
    connect( listBox(), SIGNAL( returnPressed( QListBoxItem* ) ), this, SLOT( itemSelected( QListBoxItem* ) ) );
    connect( listBox(), SIGNAL( clicked( QListBoxItem* ) ), this, SLOT( itemSelected( QListBoxItem* ) ) );
}

void KComboBox::setAutoCompletion( bool autocomplete )
{
    setCompletionMode( autocomplete ? KGlobalSettings::CompletionAuto : KGlobalSettings::completionMode() );
}

void KComboBox::setContextMenuEnabled( bool showMenu )
{
    if( m_pEdit )
        m_bEnableMenu = showMenu;
}

void KComboBox::makeCompletion( const QString& text )
{
    if( m_pEdit )
    {
	    KCompletion *comp = compObj();
	    // We test for zero length text because for some
	    // reason we get an extra text completion with an empty
	    // text when the insertion policy is set to "NoInsertion"	    
	    if( !comp || text.length() == 0 )
	    {
	        return; // No Completion object or empty completion text allowed!!
	    }
 
		QString match;
    	int pos = cursorPosition();		
    	KGlobalSettings::Completion mode = completionMode();

    	if( mode == KGlobalSettings::CompletionShell &&
    		comp->hasMultipleMatches() &&
    		text != comp->lastMatch() )
    	{
	    	match = comp->nextMatch();
	    }
	    else
	    {
	    	match = comp->makeCompletion( text );
	    }
	     	
        // If no match or the same text, simply return without completing.
        if( match.isNull() || match == text )
        {
       	    // Put the cursor at the end when in semi-automatic
		    // mode and completion is invoked with the same text.
    		if( mode == KGlobalSettings::CompletionMan )
	    	{
    			m_pEdit->end( false );
    		}    	
         	return;
      	}

		if( insertionPolicy() == KComboBox::NoInsertion )
		{
    		int index = ( count() > 0 && match.length() == 0 ) ? -1 : listBox()->index( listBox()->findItem( match ) );
		    if( index == -1 )
		    {
    		    setEditText( match );
            }
            else
            {
                setCurrentItem( index );
            }
            
            if( mode == KGlobalSettings::CompletionAuto ||
		       	mode == KGlobalSettings::CompletionMan )
            {
                m_pEdit->setSelection( pos, match.length() );
                m_pEdit->setCursorPosition( pos );
            }
		}
		else
		{
    		int index = ( match.length() == 0 ) ? -1 : listBox()->index( listBox()->findItem( match ) );
	    	if( index > -1 )
		    {
			    setCurrentItem( index );
    		}
			
	    	if( mode == KGlobalSettings::CompletionAuto ||
		    	mode == KGlobalSettings::CompletionMan )
            {
                m_pEdit->setSelection( pos, match.length() );
                m_pEdit->setCursorPosition( pos );
            }
       }       
	}
    else if( !m_pEdit )
    {
		if( text.isNull() )
		{
		    return;
		}
		int index = listBox()->index( listBox()->findItem( text ) );
		if( index >= 0 )
		{
		    setCurrentItem( index );
		}
    }

}

void KComboBox::rotateText( KCompletionBase::KeyBindingType type )
{
    if( m_pEdit )
    {
    	// Support for rotating through highlighted text!  This means
		// that if there are multiple matches for the portion of text
		// entered into the combo box,  one can simply rotate through
		// all the other possible matches using the previous/next match
		// keys.  BTW this feature only works if there is a completion
		// object available to begin with.
		KCompletion* comp = compObj();
		if( comp &&
			( type == KCompletionBase::PrevCompletionMatch ||
			  type == KCompletionBase::NextCompletionMatch ) )
		{
			QString input = ( type == KCompletionBase::PrevCompletionMatch ) ? comp->previousMatch() : comp->nextMatch();
			
			// Ignore rotating to the same text			
            if( input.isNull() || input == currentText() )
            {
                return;
            }
            bool marked = m_pEdit->hasMarkedText(); // See if it had marked text
            int pos = cursorPosition(); // Note the current cursor position
       		int index = ( count() > 0 && input.length() == 0 ) ? -1 : listBox()->index( listBox()->findItem( input ) );
		    if( index == -1 )
		    {
    		    setEditText( input );
            }
            else
            {
                setCurrentItem( index );
            }
            // Re-mark the selection if it was previously
            // selected
            if( marked )
            {
                m_pEdit->setSelection( pos, input.length() );
                m_pEdit->setCursorPosition( pos );				
			}
        }
        else
        {
        	// History list rotation feature:  The code below supports
        	// *nix shell like history list rotation.  This feature is
        	// only available when the insertion policy into the listbox
        	// of the combo widget it set to either "AtTop" or "AtBottom".
        	// Other insertion policies are not well equipped to support
        	// "save the last written text" feature of *nix like shells.        	
        	int index = -1;        	
			QComboBox::Policy policy = insertionPolicy();
			if( m_strCurrentText.isNull() &&
				( policy == QComboBox::AtTop ||
				  policy == QComboBox::AtBottom ) )
			{
				if( type == KCompletionBase::RotateUp )
				{
					index = ( policy == QComboBox::AtTop ) ? 0 : count() - 1;
				}
				if( index == 0 || index == count() - 1 )
				{
					m_strCurrentText = currentText();  //Store the current text.
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
					setEditText( m_strCurrentText );
					m_strCurrentText = QString::null;				
				}
			}
        }
    }
}

void KComboBox::itemSelected( QListBoxItem* item )
{
    if( item != 0 && m_pEdit )
    {
		m_pEdit->setSelection( 0, currentText().length() );
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

void KComboBox::keyPressEvent ( QKeyEvent * e )
{
	// Disable Qt's hard coded rotate-up key binding!  It is
	// configurable in KDE.
	if( e->state() == Qt::AltButton && e->key() == Qt::Key_Up )
	{
		e->accept();
		return;
	}

    if( m_pEdit && m_pEdit->hasFocus() )
    {
        // On Return pressed event, emit both returnPressed( const QString& )
        // and returnPressed() signals
        if( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
        {
            emit returnPressed();
            emit returnPressed( currentText() );
        }

    	KGlobalSettings::Completion mode = completionMode();
        if( mode == KGlobalSettings::CompletionAuto )
        {
            QString keycode = e->text();
            if( !keycode.isNull() && keycode.unicode()->isPrint() && emitSignals() )
            {
                QComboBox::keyPressEvent ( e );
                emit completion( currentText() );
                return;
            }
        }
        if( mode != KGlobalSettings::CompletionNone )
        {

		    bool fireSignals = emitSignals();
	        KeyBindingMap keys = getKeyBindings();
    	    int key = ( keys[TextCompletion] == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : keys[TextCompletion];
            if( KStdAccel::isEqual( e, key ) && fireSignals )
            {
                // Emit completion if the completion mode is completionShell or
                // CompletionMan and there is a completion object present the
                // current text is not the same as the previous the cursor is
                // at the end of the string.
                if( mode == KGlobalSettings::CompletionMan ||
                    (mode == KGlobalSettings::CompletionShell &&
	                 m_pEdit->cursorPosition() == (int) currentText().length() ) )
                {
                    emit completion( currentText() );
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
    else if( !m_pEdit )
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
        if( e->button() == Qt::RightButton )       
        {
            if( !m_bEnableMenu )
                return true;

            QPopupMenu *popup = new QPopupMenu( this );
            insertDefaultMenuItems( popup );
            bool flag = ( m_pEdit->echoMode()==QLineEdit::Normal && !m_pEdit->isReadOnly() );
            bool allMarked = ( m_pEdit->markedText().length() == currentText().length() );        
            popup->setItemEnabled( KCompletionBase::Cut, flag && m_pEdit->hasMarkedText() );
            popup->setItemEnabled( KCompletionBase::Copy, flag && m_pEdit->hasMarkedText() );
            popup->setItemEnabled( KCompletionBase::Paste, flag &&
                                   (bool)QApplication::clipboard()->text().length() );
            popup->setItemEnabled( KCompletionBase::Clear, flag && ( currentText().length() > 0) );
            popup->setItemEnabled( KCompletionBase::Unselect, m_pEdit->hasMarkedText() );
            popup->setItemEnabled( KCompletionBase::SelectAll, flag && m_pEdit->hasMarkedText() && !allMarked );
        
            int result = popup->exec( e->globalPos() );
            delete popup;
                
            if( result == KCompletionBase::Cut )
                m_pEdit->cut();
            else if( result == KCompletionBase::Copy )
                m_pEdit->copy();
            else if( result == KCompletionBase::Paste )
                m_pEdit->paste();
            else if( result == KCompletionBase::Clear )
                m_pEdit->clear();
            else if( result == KCompletionBase::Unselect )
                m_pEdit->deselect();
            else if( result == KCompletionBase::SelectAll )
                m_pEdit->selectAll();
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
                            
            return true;
        }
    }
    return QComboBox::eventFilter( o, ev );
}
