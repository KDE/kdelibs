#include <qpainter.h>
#include <qdrawutl.h>
#include <qkeycode.h>
#include "kcombo.h"
#include "kcombo.moc"

// Author:	Steve Dodd <dirk@loth.demon.co.uk>
// See kcombo.h for change history

KCombo::KCombo( QWidget* parent, const char* name, WFlags f ) :
	QFrame( parent, name, f )
{
	popup = new QPopupMenu; lined = NULL; listBox = NULL;
	tf = AlignCenter | ExpandTabs | WordBreak;
	setFrameStyle( QFrame::Panel | QFrame::Raised );
	setLineWidth( 2 );
	setFocusPolicy( StrongFocus );
	connect( popup, SIGNAL(activated(int)), SLOT(select(int)) );
	connect( popup, SIGNAL(highlighted(int)), SIGNAL(highlighted(int)) );
	selected = 0;
}

KCombo::KCombo( bool readWrite, QWidget* parent, const char* name, WFlags f ) :
	QFrame( parent, name, f )
{
	policy = NoInsertion;
	popup = NULL; lined = readWrite ? new KLined( this, NULL ) : (KLined*)NULL;
	//* can we use WStyle_Tool here??:
	listBox = new QListBox( NULL, NULL, WType_Popup ); // must be top-level..
	listBox->installEventFilter( this );
	tf = AlignCenter | ExpandTabs | WordBreak;
	setFrameStyle( QFrame::Panel | QFrame::Raised );
	setLineWidth( 2 ); setFocusPolicy( StrongFocus );
	connect( listBox, SIGNAL(highlighted(int)), SLOT(select(int)) );
	connect( listBox, SIGNAL(selected(int)), SLOT(selectHide(int)) );
	connect( listBox, SIGNAL(highlighted(const char*)), SIGNAL(highlighted(const char*)) );
	connect( listBox, SIGNAL(highlighted(int)), SIGNAL(highlighted(int)) );
	if( lined ) {
		connect( lined, SIGNAL(returnPressed()), SLOT(selectTyped()) );
		connect( lined, SIGNAL(completion()), SLOT(complete()) );
		connect( lined, SIGNAL(completion()), SIGNAL(completion()) );
		connect( lined, SIGNAL(rotation()), SLOT(rotate()) );
		connect( lined, SIGNAL(rotation()), SIGNAL(rotation()) );
	}
	selected = 0;
	setSizeLimit( 10 );
}

KCombo::~KCombo()
{
	if( popup ) delete popup;
	if( lined ) delete lined;
	if( listBox ) delete listBox;
}

void KCombo::rotate()
{
	if( count() > 1 ) {
		if( selected < count() -1 ) {
			select( selected + 1 );
		} else {
			select( 0 );
		}
	}	
}

void KCombo::complete()
{
	uint i;			// item no
	QString entry;	// current item

	for( i = 0; i < listBox->count(); i++ ) {
		entry = listBox->text( i );
		if( entry.find( lined->text(), 0, FALSE ) == 0 ) {
			lined->setText( entry );
			selectTyped();
			break;
		}
	}
}

void KCombo::select( int item )
{
	// don't emit anything if the selection hasn't changed. This fixes the
	// problem caused by listBox->setCurrentItem emitting highlighted(), thus
	// triggering another select() call.
	if( selected != item ) {
		selectQuiet( item );
		emit activated( item );
		if( currentText() ) emit activated( currentText() );
	} else {
		selectQuiet( item );
	}
}

void KCombo::selectQuiet( int item )
{
	selected = item;
	repaint( FALSE );
	if( lined ) lined->setText( text( selected ) );
	if( listBox ) listBox->setCurrentItem( item );
}

void KCombo::selectHide( int item )
{
	if( popup ) popup->hide();
	else if( listBox ) listBox->hide();
	select( item );
}

void KCombo::selectTyped()
{
	uint	i;
	int		c;
	
	listBox->hide();

	for( i = 0; i < listBox->count(); i++ ) {
		if( qstrcmp( listBox->text( i ), currentText() ) == 0 ) {
			select( i );
			return;
		}
	}
	
	switch( policy ) {
		case NoInsertion:
			emit activated( currentText() );
			return;
		case AtTop:
			c = 0;
			break;
		case AtBottom:
			c = count();
			break;
		case AtCurrent:
			changeItem( currentText(), currentItem() );
			select( currentItem() );
			return;
		case AfterCurrent:
			c = currentItem() + 1;
			break;
		case BeforeCurrent:
			c = currentItem();
			break;
		default:
			return;	// KCombo::BizarrePolicy :)
	}

	insertItem( currentText(), c );
	select( c );
}

bool KCombo::eventFilter( QObject* o, QEvent* e )
{
	// - trap mouse button release in list box and hide the list box (as
	// list box just emits highlighted() other wise).
	// - Also trap clicking outside the list box.
	
	if( o && o == listBox ) {
		if( e->type() == Event_MouseButtonRelease ) {
			if( listBox->rect().contains( ((QMouseEvent*)e)->pos() ) ) {
				listBox->hide();
			}
		} else if( e->type() == Event_MouseButtonPress ) {
			if( !listBox->rect().contains( ((QMouseEvent*)e)->pos() ) ) {
				listBox->hide();
			}
		}
	}

	return FALSE;
}

void KCombo::keyPressEvent( QKeyEvent* e )
{
	// allow a closed combo to be scrolled by user, and dropped down with
	// space bar.
	
	e->ignore();
	if( e->key() == Key_Down && selected < ( count() -1 ) ) {
		select( selected + 1 );
		e->accept();
	} else if( e->key() == Key_Up && selected > 0 ) {
		select( selected - 1 );
		e->accept();
	} else if( e->key() == Key_Space ) drop();
}

void KCombo::resizeEvent( QResizeEvent* e )
{
	QFrame::resizeEvent( e );
	
	QRect rect( rect() );

	boxRect.setRight( rect.right() - 10 );
	boxRect.setLeft( rect.right() - 24 );
	boxRect.setTop( ( ( rect.bottom() - rect.top() ) / 2 ) - 4 );
	boxRect.setBottom( boxRect.top() + 8 );
	textRect.setLeft( 10 ); textRect.setTop( 5 );
	textRect.setRight( boxRect.left() - 10 );
	textRect.setBottom( rect.bottom() - 5 );

	if( listBox ) listBox->resize( width() - 6, listBox->height() );
	if( lined ) lined->setGeometry( textRect ); 
}

void KCombo::drop()
{
	if( popup )
		popup->popup( mapToGlobal( QPoint( 0, 0 ) ), selected );
	else {
		listBox->move( mapToGlobal( QPoint( 2, height() - 2 ) ) );
		listBox->show();
	}
}

void KCombo::drawContents( QPainter* paint )
{
	QBrush  backBrush( colorGroup().background() );

	// NOTE: we get called from QFrame::paintEvent(). This only redraws the
	// frame itself if the update area was not wholy inside the frame (i.e.
	// it overlapped the frame). It only redraws the contents if necessary.
	// It never erases the background, so if someone calls repaint(FALSE),
	// we need to erase the background ourselves.
			
	if( popup )
		qDrawWinPanel( paint, boxRect, colorGroup(), TRUE, &backBrush );
	else
		qDrawArrow( paint, DownArrow, MotifStyle, TRUE, boxRect.x(),
			boxRect.y(), boxRect.width(), boxRect.height(), colorGroup(), TRUE );  

	if( !lined ) {  // draw text (read-only combo)
		paint->eraseRect( textRect );
		paint->drawText( textRect, tf, currentText() );
	}
	
	// draw focus rectangle:
	if( hasFocus() ) paint->drawRect( rect().right() - 29,
		rect().top() + 5, 24, rect().bottom() - rect().top() - 10 ); 
}

void KCombo::setSizeLimit( int lines )
{
	sizeLimitLines = lines;
	if( listBox->itemHeight() == -1 ) {
		KDEBUG( KDEBUG_WARN, 152, "KCombo: can't setSizeLimit, listBox items aren't all the same height.." );
	} else
		listBox->resize( listBox->width(), lines * listBox->itemHeight() );
}

// small public interface functions:

void KCombo::insertStrList( const QStrList* list, int index )
{
	QStrListIterator it( *list );

	while( it.current() ) {
		insertItem( it.current(), index == -1 ? -1 : index++ );
		++it;
	}
}

void KCombo::insertStrList( const char** strings, int numStrings, int index )
{
	// while current list item is not null, and numStrings is +ve:

	while( *strings && ( numStrings == -1 ? TRUE : numStrings-- ) ) {
		insertItem( *(strings++), index == -1 ? -1 : index++ );
	}	
} 

int KCombo::count() const
{
	if( popup ) return (int)popup->count();
	return (int)listBox->count();
}

void KCombo::insertItem( const char* text, int index = -1 )
{
	if( popup ) popup->insertItem( text, -1, index );
	else {
		listBox->insertItem( text, index );
		if( ( index == selected || ( index == -1 && count() == 0 ) ) && lined )
			lined->setText( text );
	}
}

void KCombo::insertItem( const QPixmap& pixmap, int index = -1 )
{
	if( popup ) popup->insertItem( pixmap, -1, index );
	else listBox->insertItem( pixmap, index );
}

void KCombo::removeItem( int index )
{
	if( popup ) popup->removeItemAt( index );
	else listBox->removeItem( index );
}

void KCombo::clear()
{
	if( popup ) popup->clear();
	else listBox->clear();
}

// get text at index in popup or list box
const char* KCombo::text( int index ) const
{
	if( popup ) return popup->text( popup->idAt( index ) );
	return listBox->text( index );
}

// if there's a lineedit, get text from that, else get selected text
// in listbox/popup. Call text( currentItem() ) to always
// ignore contents of lined.
const char* KCombo::currentText() const
{
	if( lined ) return lined->text();
	return text( selected );
}

const QPixmap* KCombo::pixmap( int index ) const
{
	if( popup ) return popup->pixmap( popup->idAt( index ) );
	return listBox->pixmap( index );
}

void KCombo::changeItem( const char* text, int index )
{
	if( popup ) popup->changeItem( text, popup->idAt( index ) );
	else listBox->changeItem( text, index );
}

void KCombo::changeItem( const QPixmap& pixmap, int index )
{
	if( popup ) popup->changeItem( pixmap, popup->idAt( index ) );
	else listBox->changeItem( pixmap, index );
}

// functions that were inlined in the header are now here, so binary compat.
// is more likely:

bool KCombo::autoResize() const
{
	return FALSE;
}

void KCombo::setAutoResize( bool enable )
{
	if( enable ) {
		KDEBUG( KDEBUG_WARN, 152, "KCombo: setAutoResize( TRUE ) not yet supported, sorry." );
	}
}

int KCombo::maxCount() const
{
	return -1;
}

void KCombo::setMaxCount( int lines )
{
	if( lines != -1 ) {
		KDEBUG( KDEBUG_WARN, 152, "KCombo: setMaxCount not yet supported for lines != -1, sorry." );
	}
}

void KCombo::setInsertionPolicy( Policy insertionPolicy )
{
	policy = insertionPolicy;
}

void KCombo::cursorAtEnd()
{
	if( lined ) lined->cursorAtEnd();
}
