#include "klistview.h"
#include <kglobalsettings.h>
#include <kcursor.h>
#include <kapp.h>

KListView::KListView( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    oldCursor = viewport()->cursor();
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QListViewItem * ) ),
	     this, SLOT( slotOnItem( QListViewItem * ) ) );
    slotSettingsChanged(KApplication::SETTINGS_MOUSE);
    connect( kapp, SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );
}

void KListView::slotSettingsChanged(int category)
{
    if (category != KApplication::SETTINGS_MOUSE)
        return;
    m_bUseSingle = KGlobalSettings::singleClick();
    m_bChangeCursorOverItem = KGlobalSettings::changeCursorOverIcon();

    if ( !m_bUseSingle )
	disconnect( this, SIGNAL( clicked( QListViewItem * ) ),
                    this, SIGNAL( doubleClicked( QListViewItem * ) ) );
    else
	connect( this, SIGNAL( clicked( QListViewItem * ) ),
		 this, SIGNAL( doubleClicked( QListViewItem * ) ) );

    if( !m_bUseSingle || !m_bChangeCursorOverItem )
	viewport()->setCursor( oldCursor );
}

void KListView::slotOnItem( QListViewItem *item )
{
    if ( item && m_bChangeCursorOverItem && m_bUseSingle )
        viewport()->setCursor( KCursor().handCursor() );
}

void KListView::slotOnViewport()
{
    if ( m_bChangeCursorOverItem )
        viewport()->setCursor( oldCursor );
}

