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
#include <kpixmapprovider.h>
#include <kstdaccel.h>
#include <kdebug.h>

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

void KComboBox::setCompletedText( const QString& text, bool marked )
{
    int pos = cursorPosition();
    setEditText( text );
	// Hightlight the text whenever appropriate.
    if( marked && m_pEdit )
	{
        m_pEdit->setSelection( pos, text.length() );
        m_pEdit->setCursorPosition( pos );
    }
}

void KComboBox::setCompletedText( const QString& text )
{
    KGlobalSettings::Completion mode = completionMode();
    bool marked = ( mode == KGlobalSettings::CompletionAuto ||
                    mode == KGlobalSettings::CompletionMan );
    setCompletedText( text, marked );
}

void KComboBox::makeCompletion( const QString& text )
{
    if( m_pEdit )
    {
       KCompletion *comp = compObj();

       // We test for zero length text because for some
       // reason we get an extra text completion with an empty
       // text when the insertion policy is set to "NoInsertion"
       if( !comp )
       {
	  return; // No Completion object or empty completion text allowed!
       }

	QString match = comp->makeCompletion( text );
    KGlobalSettings::Completion mode = completionMode();
        // If no match or the same text, simply return without completing.
        if( match.isNull() || match == text )
        {
       	    // Put the cursor at the end when in semi-automatic
	    // mode and completion is invoked with the same text.
    	    if( mode == KGlobalSettings::CompletionMan ) {
		m_pEdit->end( false );
	    }
    	    return;
      	}
      	bool marked = ( mode == KGlobalSettings::CompletionAuto ||
      	                mode == KGlobalSettings::CompletionMan );
      	setCompletedText( match, marked );
    }

    // Read - only combobox
    else if( !m_pEdit )
    {
	if( text.isNull() )
	    return;

	int index = listBox()->index( listBox()->findItem( text ) );
	if( index >= 0 ) {
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
	       return;

            bool marked = m_pEdit->hasMarkedText(); // See if it had marked text
            setCompletedText( input, marked );
        }
    }
}

void KComboBox::itemSelected( QListBoxItem* item )
{
    kdDebug() << QString::fromLatin1( "KComobBox: Item Selected" ) << endl;
    if( item != 0 && m_pEdit )
    {
       m_pEdit->setSelection( 0, currentText().length() );
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
            if( !keycode.isNull() && keycode.unicode()->isPrint() )
            {
                QComboBox::keyPressEvent ( e );
                QString txt = currentText();
                if( !m_pEdit->hasMarkedText() && txt.length() )
                {
                    kdDebug() << "Key Pressed: " << keycode << endl;
                    kdDebug() << "Current text: " << txt << endl;
                    if( emitSignals() )
                        emit completion( txt ); // emit when requested...
                    if( handleSignals() )
                        makeCompletion( txt );  // handle when requested...
                }
                return;
            }
        }
        if( mode != KGlobalSettings::CompletionNone )
        {
            KeyBindingMap keys = getKeyBindings();
            int key = ( keys[TextCompletion] == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : keys[TextCompletion];
            if( KStdAccel::isEqual( e, key ) )
            {
                // Emit completion if the completion mode is completionShell or
                // CompletionMan, there is a completion object present, the
                // current text is not the same as the previous and the cursor
                // is at the end of the string.
                QString txt = currentText();
                if( mode == KGlobalSettings::CompletionMan ||
                    (mode == KGlobalSettings::CompletionShell &&
                    m_pEdit->cursorPosition() == (int) txt.length() ) )
                {
                    if( emitSignals() )
                        emit completion( txt ); // emit when requested...
                    if( handleSignals() )
                        makeCompletion( txt );  // handle when requested...		
                    return;
                }
            }
            // Handles previousMatch.
            key = ( keys[PrevCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::PrevCompletion) : keys[PrevCompletionMatch];
            if( KStdAccel::isEqual( e, key ) )
            {
                if( emitSignals() )
                    emit textRotation( KCompletionBase::PrevCompletionMatch );
                if( handleSignals() )
                    rotateText( KCompletionBase::PrevCompletionMatch );
                return;
            }
            // Handles nextMatch.
            key = ( keys[NextCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::NextCompletion) : keys[NextCompletionMatch];
            if( KStdAccel::isEqual( e, key ) )
            {
                if( emitSignals() )
                    emit textRotation( KCompletionBase::NextCompletionMatch );
                if( handleSignals() )
                    rotateText( KCompletionBase::PrevCompletionMatch );
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
            if( handleSignals() )
                makeCompletion( keycode );
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

            bool flag = ( m_pEdit->echoMode()==QLineEdit::Normal && !m_pEdit->isReadOnly() );
            bool allMarked = ( m_pEdit->markedText().length() == currentText().length() );
            popup->setItemEnabled( Cut, flag && m_pEdit->hasMarkedText() );
            popup->setItemEnabled( Copy, flag && m_pEdit->hasMarkedText() );
            popup->setItemEnabled( Paste, flag && (bool)QApplication::clipboard()->text().length() );
            popup->setItemEnabled( Clear, flag && ( currentText().length() > 0) );
            popup->setItemEnabled( Unselect, m_pEdit->hasMarkedText() );
            popup->setItemEnabled( SelectAll, flag && m_pEdit->hasMarkedText() && !allMarked );

            int result = popup->exec( e->globalPos() );
            delete popup;

            if( result == Cut )
                m_pEdit->cut();
            else if( result == Copy )
                m_pEdit->copy();
            else if( result == Paste )
                m_pEdit->paste();
            else if( result == Clear )
                m_pEdit->clear();
            else if( result == Unselect )
                m_pEdit->deselect();
            else if( result == SelectAll )
                m_pEdit->selectAll();
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

            return true;
        }
    }

    else if ( (o == this || o == m_pEdit) && ev->type() == QEvent::KeyPress ) {
	QKeyEvent *e = static_cast<QKeyEvent *>( ev );
	if ( e->key() == Key_Return || e->key() == Key_Enter) {

	    // On Return pressed event, emit both returnPressed(const QString&)
	    // and returnPressed() signals
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
    myPixProvider = 0L;

    connect( this, SIGNAL( activated(int) ), SLOT( slotReset() ));
    connect( this, SIGNAL( returnPressed(const QString&) ), SLOT(slotReset()));
}

KHistoryCombo::~KHistoryCombo()
{
    delete myPixProvider;
}

void KHistoryCombo::setHistoryItems( QStringList items,
				     bool setCompletionList )
{
    KComboBox::clear();

    // limit to maxCount()
    while ( (int) items.count() > maxCount() && !items.isEmpty() )
	items.remove( items.begin() );

    insertItems( items );

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
    if ( item.isEmpty() || (count() > 0 && item == text(0) ))
	return;

    int last;
    QString rmItem;
    while ( count() >= maxCount() && count() > 0 ) {
	// remove the last item, as long as we are longer than maxCount()
	// remove the removed item from the completionObject if it isn't
	// anymore available at all in the combobox.
	last = count() - 1;
	rmItem = text( last );
	removeItem( last );
	if ( !contains( rmItem ) )
	    completionObject()->removeItem( item );
    }

    // now add the item
    if ( myPixProvider )
	insertItem( myPixProvider->pixmapFor( item, KIcon::SizeSmall), item, 0 );
    else
	insertItem( item, 0 );
    completionObject()->addItem( item );
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
	
	    // if the typed text is the same as the first item, skip the first
	    if ( myText == text(0) )
		myIterateIndex = 0;

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
    // clearEdit(); // FIXME, use a timer for that? slotReset is called
    // before addToHistory() so we can't clear the edit here :-/
}


void KHistoryCombo::setPixmapProvider( KPixmapProvider *prov )
{
    if ( myPixProvider == prov )
	return;

    myPixProvider = prov;

    // re-insert all the items with/without pixmap
    // I would prefer to use changeItem(), but that doesn't honour the pixmap
    // when using an editable combobox (what we do)
    if ( count() > 0 ) {
	QStringList items( historyItems() );
	clear();
	insertItems( items );
    }
}

void KHistoryCombo::insertItems( const QStringList& items )
{
    QStringList::ConstIterator it = items.begin();
    QString item;
    while ( it != items.end() ) {
	item = *it;
	if ( !item.isEmpty() ) { // only insert non-empty items
	    if ( myPixProvider )
		insertItem( myPixProvider->pixmapFor(item, KIcon::SizeSmall),
			    item );
	    else
		insertItem( item );
	}
	++it;
    }
}
