#include <qapplication.h>

#include <qscrollview.h>

#include "kcompletionbox.h"


KCompletionBox::KCompletionBox( QWidget *parent, const char *name )
    : KListBox( 0L, name, WType_Popup | WStyle_NoBorderEx )
{
    m_parent = parent;
    
    setColumnMode( 1 );
    setLineWidth( 1 );
    setFrameStyle( QFrame::Box | QFrame::Plain );

    setVScrollBarMode( Auto );
    setHScrollBarMode( AlwaysOff );

    connect( this, SIGNAL( selected( QListBoxItem * )),
	     SLOT( slotActivated( QListBoxItem * )) );
    connect( this, SIGNAL( clicked( QListBoxItem * )),
	     SLOT( slotActivated( QListBoxItem * )));

    // highlight on mouseover
    connect( this, SIGNAL( onItem( QListBoxItem * )),
	     SLOT( slotSetCurrentItem( QListBoxItem * )));

    installEventFilter( this );
}

KCompletionBox::~KCompletionBox()
{
}

QStringList KCompletionBox::items() const
{
    QStringList list;
    for ( uint i = 0; i < count(); i++ ) {
	list.append( text( i ) );
    }
    return list;
}

void KCompletionBox::slotActivated( QListBoxItem *item )
{
    if ( !item )
	return;

    hide();
    emit activated( item->text() );
}

bool KCompletionBox::eventFilter( QObject *o, QEvent *e )
{
    int type = e->type();
    
    switch( type ) {
     case QEvent::MouseButtonPress:
 	hide();
 	break;
    case QEvent::Show:
	releaseKeyboard(); // so that we get "dead keys" working
	break;
    case QEvent::KeyPress: {
 	QKeyEvent *ev = static_cast<QKeyEvent *>( e );
 	if ( ev->key() == Key_Escape )
 	    hide();
	else if ( ev->key() == Key_Up && currentItem() == 0 ) {
	    m_parent->setFocus();
	    setSelected( 0, false );
	}

	break;
    }
    default:
	break;
    }
    
    return KListBox::eventFilter( o, e );
}


void KCompletionBox::popup( QWidget *relativeWidget )
{
    QSize s = sizeHint();
    resize( QMAX(s.width(), relativeWidget->width()), s.height() );
    move( relativeWidget->mapToGlobal( QPoint(0, relativeWidget->height())) );
    raise();
    show();
}

#include "kcompletionbox.moc"
