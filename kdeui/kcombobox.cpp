/* This file is part of the KDE libraries

   Copyright (c) 2000 Dawit Alemayehu <adawit@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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
    m_trapReturnKey = false;

    init();
}

KComboBox::KComboBox( bool rw, QWidget *parent, const char *name )
          :QComboBox( rw, parent, name )
{
    m_trapReturnKey = false;

    if ( rw )
    {
        m_pEdit = QComboBox::lineEdit();
	// Qt-bug, QComboBox installs exactly the same event-filter and there
	// is no check for this, so we would end up with two event-filters and
	// eventFilter() being called twice for every event.
	//  m_pEdit->installEventFilter( this );
    }
    init();
}

KComboBox::~KComboBox()
{
}

void KComboBox::init()
{
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
    connect( listBox(), SIGNAL( returnPressed( QListBoxItem* ) ), SLOT( itemSelected( QListBoxItem* ) ) );
    connect( listBox(), SIGNAL( clicked( QListBoxItem* ) ), SLOT( itemSelected( QListBoxItem* ) ) );
}

bool KComboBox::contains( const QString& _text ) const
{
    if ( _text.isEmpty() )
	return false;

    for (int i = 0; i < count(); i++ ) {
	if ( text(i) == _text )
	    return true;
    }
    return false;
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
	  return; // No Completion object or empty completion text allowed!
       }

    	int pos = cursorPosition();
    	KGlobalSettings::Completion mode = completionMode();
		QString match = comp->makeCompletion( text );

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

      	// Set the current text to the one completed.
        setEditText( match );

	    // Hightlight the text whenever appropriate.
    	if( mode == KGlobalSettings::CompletionAuto ||
	    	mode == KGlobalSettings::CompletionMan )
	    {
            m_pEdit->setSelection( pos, match.length() );
            m_pEdit->setCursorPosition( pos );
        }
	}
	// Read - only combobox
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
   		    setEditText( input );
            // Re-mark the selection if it was
			// previously selected
            if( marked )
            {
                m_pEdit->setSelection( pos, input.length() );
                m_pEdit->setCursorPosition( pos );
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

void KComboBox::keyPressEvent ( QKeyEvent * e )
{
    if( m_pEdit && m_pEdit->hasFocus() )
    {
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

            // Handles previousMatch.
            key = ( keys[PrevCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::PrevCompletion) : keys[PrevCompletionMatch];
            if( KStdAccel::isEqual( e, key ) && fireSignals )
            {
                emit textRotation( KCompletionBase::PrevCompletionMatch );
                return;
            }
            // Handles nextMatch.
            key = ( keys[NextCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::NextCompletion) : keys[NextCompletionMatch];
            if( KStdAccel::isEqual( e, key ) && fireSignals )
            {
                emit textRotation( KCompletionBase::NextCompletionMatch );
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

    else if ( (o == this || o == m_pEdit) && ev->type() == QEvent::KeyPress ) {
	QKeyEvent *e = static_cast<QKeyEvent *>( ev );
	if ( e->key() == Key_Return || e->key() == Key_Enter) {

	    // On Return pressed event, emit both returnPressed(const QString&)
	    // and returnPressed() signals
	    qDebug("******** RETURN PRESSED ****** : %s : ", currentText().latin1());
            emit returnPressed();
            emit returnPressed( currentText() );

	    return m_trapReturnKey;
	}
    }

    return QComboBox::eventFilter( o, ev );
}


void KComboBox::setTrapReturnKey( bool grab )
{
    m_trapReturnKey = grab;
}

bool KComboBox::trapReturnKey() const
{
    return m_trapReturnKey;
}


// *********************************************************************
// *********************************************************************

// we are always read-write
KHistoryCombo::KHistoryCombo( QWidget *parent, const char *name )
    : KComboBox( true, parent, name )
{
    completionObject()->setOrder( KCompletion::Weighted );
    setInsertionPolicy( NoInsertion );
    myIterateIndex = -1;
    myRotated = false;

    connect( this, SIGNAL( activated(int) ), SLOT( slotReset() ));
    connect( this, SIGNAL( returnPressed(const QString&) ), SLOT(slotReset()));
}

void KHistoryCombo::setHistoryItems( QStringList items,
				     bool setCompletionList )
{
    clearHistory();

    while ( (int) items.count() > maxCount() && !items.isEmpty() )
	items.remove( items.begin() );

    insertStringList( items );

    if ( setCompletionList ) {
	// we don't have any weighting information here ;(
	KCompletion *comp = completionObject();
	comp->setOrder( KCompletion::Insertion );
	comp->setItems( items );
	comp->setOrder( KCompletion::Weighted );
    }

    clearEdit();

}

QStringList KHistoryCombo::historyItems() const
{
    QStringList list;
    for ( int i = 0; i < count(); i++ )
	list.append( text( i ) );

    return list;
}

void KHistoryCombo::clearHistory()
{
    KComboBox::clear();
    completionObject()->clear();
}

void KHistoryCombo::addToHistory( const QString& item )
{
    if ( item.isEmpty() || item == text(0) )
	return;

    int last;
    QString rmItem;
    while ( count() >= maxCount() ) {
	// remove the last item, as long as we are longer than maxCount()
	// remove the removed item from the completionObject if it isn't
	// anymore available at all in the combobox.
	last = count() - 1;
	rmItem = text( last );
	removeItem( last );
	if ( !contains( rmItem ) )
	    completionObject()->removeItem( item );
    }

    // now add the items
    insertItem( item, 0 );
    completionObject()->addItem( item );
    clearEdit();
}

bool KHistoryCombo::removeFromHistory( const QString& item )
{
    if ( item.isEmpty() )
	return false;

    bool removed = false;
    for ( int i = 0; i < count(); i++ ) {
	while ( item == text( i ) ) {
	    removed = true;
	    removeItem( i );
	}
    }

    if ( removed )
	completionObject()->removeItem( item );

    return removed;
}

void KHistoryCombo::keyPressEvent( QKeyEvent *e )
{
    // save the current text in the lineedit
    if ( myIterateIndex == -1 )
	myText = currentText();

    // going up in the history, rotating when reaching QListBox::count()
    if ( KStdAccel::isEqual( e, KStdAccel::rotateUp() ) ) {
	myIterateIndex++;
	if ( myIterateIndex >= count() ) {
	    myRotated = true;
	    myIterateIndex = -1;
	    setEditText( myText );
	}
	else
	    setEditText( text( myIterateIndex ));
    }


    // going down in the history, no rotation possible. Last item will be
    // the text that was in the lineedit before Up was called.
    else if ( KStdAccel::isEqual( e, KStdAccel::rotateDown() ) ) {
	myIterateIndex--;
	if ( myIterateIndex < 0 ) {
	    if ( myRotated && myIterateIndex == -2 ) {
		myRotated = false;
		myIterateIndex = count() - 1;
		setEditText( text(myIterateIndex) );
	    }
	    else {
		myIterateIndex = -1;
		setEditText( myText );
	    }
	}
	else
	    setEditText( text( myIterateIndex ));
    }

    else
	KComboBox::keyPressEvent( e );
}

void KHistoryCombo::slotReset()
{
    myIterateIndex = -1;
    myRotated = false;
    //    clearEdit(); // FIXME, use a timer for that? slotReset is called
    // before addToHistory() so we can't clear the edit here :-/
}
