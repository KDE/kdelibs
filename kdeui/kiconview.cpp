#include "kiconview.h"
#include <kglobal.h>
#include <kcursor.h>

KIconView::KIconView( QWidget *parent, const char *name, WFlags f )
    : QIconView( parent, name, f )
{
    useDouble = !KGlobal::useDoubleClicks();
    oldCursor = viewport()->cursor();
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QIconViewItem * ) ),
	     this, SLOT( slotOnItem( QIconViewItem * ) ) );
    checkClickMode();
}

void KIconView::checkClickMode()
{
    if ( KGlobal::useDoubleClicks() == useDouble )
	return;

    if ( !useDouble )
	disconnect( this, SIGNAL( clicked( QIconViewItem * ) ),
			this, SIGNAL( doubleClicked( QIconViewItem * ) ) );
    useDouble = KGlobal::useDoubleClicks();
    if ( !useDouble )
	connect( this, SIGNAL( clicked( QIconViewItem * ) ),
		 this, SIGNAL( doubleClicked( QIconViewItem * ) ) );
    else
	viewport()->setCursor( oldCursor );
}

void KIconView::slotOnItem( QIconViewItem *item )
{
    checkClickMode();
    if ( useDouble )
	return;
    if ( !item )
	return;
    viewport()->setCursor( KCursor().handCursor() );
}

void KIconView::slotOnViewport()
{
    checkClickMode();
    if ( useDouble )
	return;
    viewport()->setCursor( oldCursor );
}
