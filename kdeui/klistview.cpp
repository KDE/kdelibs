#include "klistview.h"
#include <kglobalsettings.h>
#include <kcursor.h>

KListView::KListView( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    useDouble = KGlobalSettings::singleClick();
    oldCursor = viewport()->cursor();
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QListViewItem * ) ),
	     this, SLOT( slotOnItem( QListViewItem * ) ) );
    checkClickMode();
}

void KListView::checkClickMode()
{
    if ( !KGlobalSettings::singleClick() == useDouble )
	return;

    if ( !useDouble )
	disconnect( this, SIGNAL( clicked( QListViewItem * ) ),
			this, SIGNAL( doubleClicked( QListViewItem * ) ) );
    useDouble = !KGlobalSettings::singleClick();
    if ( !useDouble )
	connect( this, SIGNAL( clicked( QListViewItem * ) ),
		 this, SIGNAL( doubleClicked( QListViewItem * ) ) );
    else
	viewport()->setCursor( oldCursor );
}

void KListView::slotOnItem( QListViewItem *item )
{
    checkClickMode();
    if ( useDouble )
	return;
    if ( !item )
	return;
    viewport()->setCursor( KCursor().handCursor() );
}

void KListView::slotOnViewport()
{
    checkClickMode();
    if ( useDouble )
	return;
    viewport()->setCursor( oldCursor );
}

