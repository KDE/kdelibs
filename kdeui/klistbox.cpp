#include "klistbox.h"
#include <kglobalsettings.h>
#include <kcursor.h>
#include <kapp.h>
#include <kipc.h>

KListBox::KListBox( QWidget *parent, const char *name, WFlags f )
    : QListBox( parent, name, f )
{
    oldCursor = viewport()->cursor();
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QListBoxItem * ) ),
	     this, SLOT( slotOnItem( QListBoxItem * ) ) );
    slotSettingsChanged(KApplication::SETTINGS_MOUSE);
    connect( kapp, SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );
    kapp->addKipcEventMask( KIPC::SettingsChanged );
}

void KListBox::slotSettingsChanged(int category)
{
    if (category != KApplication::SETTINGS_MOUSE)
        return;
    m_bUseSingle = KGlobalSettings::singleClick();
    m_bChangeCursorOverItem = KGlobalSettings::changeCursorOverIcon();

    if ( !m_bUseSingle )
	disconnect( this, SIGNAL( clicked( QListBoxItem * ) ),
			this, SIGNAL( doubleClicked( QListBoxItem * ) ) );
    else
	connect( this, SIGNAL( clicked( QListBoxItem * ) ),
		 this, SIGNAL( doubleClicked( QListBoxItem * ) ) );

    if( !m_bUseSingle || !m_bChangeCursorOverItem )
	viewport()->setCursor( oldCursor );
}

void KListBox::slotOnItem( QListBoxItem *item )
{
    if ( item && m_bChangeCursorOverItem && m_bUseSingle )
        viewport()->setCursor( KCursor().handCursor() );
}

void KListBox::slotOnViewport()
{
    if ( m_bChangeCursorOverItem )
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
