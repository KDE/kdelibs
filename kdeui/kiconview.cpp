#include "kiconview.h"
#include <kglobal.h>
#include <kcursor.h>

KIconView::KIconView( QWidget *parent, const char *name, WFlags f )
    : QIconView( parent, name, f )
{
    // set it to the wrong value so that checkClickMode does something
    useDouble = !KGlobal::useDoubleClicks();
    oldCursor = viewport()->cursor();
    changeCursorSet = false;
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
    if (!changeCursorSet)
        changeCursorOverItem = !useDouble; // default : change cursor in single-click mode
}

void KIconView::setChangeCursor( bool c )
{
    changeCursorSet = true;
    changeCursorOverItem = c;
}

void KIconView::slotOnItem( QIconViewItem *item )
{
    checkClickMode();
    if ( item && changeCursorOverItem )
        viewport()->setCursor( KCursor().handCursor() );
    // TODO : Auto-select
}

void KIconView::slotOnViewport()
{
    checkClickMode();
    if ( changeCursorOverItem )
        viewport()->setCursor( oldCursor );
}
