#include <qapplication.h>

#include <qscrollview.h>

#include "kcompletionbox.h"


KCompletionBox::KCompletionBox( const char *name )
    : KListBox( 0L, name, WType_Popup )
{
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
    if ( e->type() == QEvent::MouseButtonPress )
	hide();

    return KListBox::eventFilter( o, e );
}


void KCompletionBox::keyPressEvent( QKeyEvent *e )
{
    KListBox::keyPressEvent( e );
    if ( e->key() == Key_Escape )
	hide();
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
