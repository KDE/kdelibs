#include "klistbox.h"
#include <kglobal.h>
#include <kcursor.h>

KListBox::KListBox( QWidget *parent, const char *name, WFlags f )
    : QListBox( parent, name, f )
{
    useDouble = !KGlobal::useDoubleClicks();
    oldCursor = viewport()->cursor();
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QListBoxItem * ) ),
	     this, SLOT( slotOnItem( QListBoxItem * ) ) );
    checkClickMode();
}

void KListBox::checkClickMode()
{
    if ( KGlobal::useDoubleClicks() == useDouble )
	return;

    if ( !useDouble )
	disconnect( this, SIGNAL( clicked( QListBoxItem * ) ),
			this, SIGNAL( doubleClicked( QListBoxItem * ) ) );
    useDouble = KGlobal::useDoubleClicks();
    if ( !useDouble )
	connect( this, SIGNAL( clicked( QListBoxItem * ) ),
		 this, SIGNAL( doubleClicked( QListBoxItem * ) ) );
    else
	viewport()->setCursor( oldCursor );
}

void KListBox::slotOnItem( QListBoxItem *item )
{
    checkClickMode();
    if ( useDouble )
	return;
    if ( !item )
	return;
    viewport()->setCursor( KCursor().handCursor() );
}

void KListBox::slotOnViewport()
{
    checkClickMode();
    if ( useDouble )
	return;
    viewport()->setCursor( oldCursor );
}

//
// 2000-16-01 Espen Sand
// This widget is used in dialogs. It should ignore
// F1 (and combinations) and Escape since these are used
// to start help or close the dialog. This functionality
// should be done in QListView but it is not (at least now)
//
void KListBox::keyPressEvent(QKeyEvent *e)
{
  if( e->key() == Key_Escape )
  {
    e->ignore();
  }
  else if( e->key() == Key_F1 )
  {
    e->ignore();
  }
  else
  {
    QListBox::keyPressEvent(e);
  }
}
