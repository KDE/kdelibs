// -*- c++ -*-
/* This file is part of the KDE libraries
   Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.	If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qfontmetrics.h>
#include <qkeycode.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtooltip.h>

#include <kaction.h>
#include <kapp.h>
#include <klocale.h>
#include <kfileviewitem.h>
#include <kglobalsettings.h>

#include "kfileiconview.h"
#include "config-kfile.h"


KFileIconViewItem::~KFileIconViewItem()
{
    const_cast<KFileViewItem*>(fileInfo())->
	setViewItem(static_cast<KFileIconView*>(iconView()), (const void*)0);
}

class KFileIconView::KFileIconViewPrivate
{
public:
    KFileIconViewPrivate( KFileIconView *parent ) {
	smallColumns = new KRadioAction( i18n("Small Columns"), 0, parent,
					 SLOT( slotSmallColumns() ), 
					 parent->actionCollection(),
					 "small columns" );

	largeRows = new KRadioAction( i18n("Large Rows"), 0, parent,
				      SLOT( slotLargeRows() ), 
				      parent->actionCollection(),
				      "large rows" );
	
	smallColumns->setExclusiveGroup(QString::fromLatin1("IconView mode"));
	largeRows->setExclusiveGroup(QString::fromLatin1("IconView mode"));
    }
    
    ~KFileIconViewPrivate() {}
    
    KRadioAction *smallColumns, *largeRows;
};

KFileIconView::KFileIconView(QWidget *parent, const char *name)
    : KIconView(parent, name), KFileView()
{
    d = new KFileIconViewPrivate( this );
    
    setViewName( i18n("Icon View") );

    toolTip = 0;
    setResizeMode( Adjust );
    setGridX( 120 );
    setWordWrapIconText( FALSE );
    setAutoArrange( TRUE );
    setItemsMovable( false );
    setMode( KIconView::Select );
    // as long as QIconView only shows tooltips when the cursor is over the
    // icon (and not the text), we have to create our own tooltips
    setShowToolTips( false );

    connect( this, SIGNAL( returnPressed(QIconViewItem *) ),
	     SLOT( selected( QIconViewItem *) ) );

    // we want single click _and_ double click (as convenience)
    connect( this, SIGNAL( clicked(QIconViewItem *, const QPoint&) ),
	     SLOT( selected( QIconViewItem *) ) );
    connect( this, SIGNAL( doubleClicked(QIconViewItem *, const QPoint&) ),
	     SLOT( slotDoubleClicked( QIconViewItem *) ) );

    connect( this, SIGNAL( onItem( QIconViewItem * ) ),
	     this, SLOT( showToolTip( QIconViewItem * ) ) );
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( removeToolTip() ) );
    connect( this, SIGNAL( rightButtonPressed( QIconViewItem*, const QPoint&)),
	     SLOT( slotRightButtonPressed( QIconViewItem* ) ) );

    KFile::SelectionMode sm = KFileView::selectionMode();
    switch ( sm ) {
    case KFile::Multi:
	QIconView::setSelectionMode( QIconView::Multi );
	break;
    case KFile::Extended:
	QIconView::setSelectionMode( QIconView::Extended );
	break;
    case KFile::NoSelection:
	QIconView::setSelectionMode( QIconView::NoSelection );
	break;
    default: // fall through
    case KFile::Single:
	QIconView::setSelectionMode( QIconView::Single );
	break;
    }

    if ( sm == KFile::Multi || sm == KFile::Extended )
	connect( this, SIGNAL( selectionChanged() ),
		 SLOT( slotSelectionChanged() ));
    else
	connect( this, SIGNAL( selectionChanged( QIconViewItem * )),
		 SLOT( highlighted( QIconViewItem * )));

    readConfig();
 }

KFileIconView::~KFileIconView()
{
    writeConfig();
    removeToolTip();
    delete d;
}

void KFileIconView::readConfig()
{
    KConfig *kc = KGlobal::config();
    KConfigGroupSaver cs( kc, "KFileIconView" );
    QString small = QString::fromLatin1("SmallColumns");

    if ( kc->readEntry("ViewMode", small ) == small ) {
	d->smallColumns->setChecked( true );
	slotSmallColumns();
    }
    else {
	d->largeRows->setChecked( true );
	slotLargeRows();
    }
}

void KFileIconView::writeConfig()
{
    KConfig *kc = KGlobal::config();
    KConfigGroupSaver cs( kc, "KFileIconView" );
    kc->writeEntry( "ViewMode", d->smallColumns->isChecked() ? 
		    QString::fromLatin1("SmallColumns") :
		    QString::fromLatin1("LargeRows") );
}

void KFileIconView::removeToolTip()
{
    delete toolTip;
    toolTip = 0;
}

void KFileIconView::showToolTip( QIconViewItem *item )
{
    delete toolTip;
    toolTip = 0;

    if ( !item )
	return;

    int w = maxItemWidth() - ( itemTextPos() == QIconView::Bottom ? 0 :
			       item->pixmapRect().width() ) - 4;
    if ( fontMetrics().width( item->text() ) >= w ) {
	toolTip = new QLabel( QString::fromLatin1(" %1 ").arg(item->text()), 0,
			      "myToolTip",
			      WStyle_Customize | WStyle_NoBorder |
			      WStyle_Tool );
	toolTip->setFrameStyle( QFrame::Plain | QFrame::Box );
	toolTip->setLineWidth( 1 );
	toolTip->setAlignment( AlignLeft | AlignTop );
	toolTip->move( QCursor::pos() + QPoint( 14, 14 ) );
	toolTip->adjustSize();
	toolTip->setFont( QToolTip::font() );
	toolTip->setPalette( QToolTip::palette(), TRUE );
	toolTip->show();
    }
}

void KFileIconView::slotRightButtonPressed( QIconViewItem* item )
{
    if ( !item ) {
	activateMenu( 0 );
	return;
    }
    KFileIconViewItem *i = (KFileIconViewItem*)item;
    activateMenu( i->fileInfo() );
}

void KFileIconView::hideEvent( QHideEvent *e )
{
    removeToolTip();
    KIconView::hideEvent( e );
}

void KFileIconView::keyPressEvent( QKeyEvent *e )
{
    KIconView::keyPressEvent( e );
    if ( e->key() == Key_Return || e->key() == Key_Enter )
        e->ignore();
}

void KFileIconView::setSelected( const KFileViewItem *info, bool enable )
{
    if ( !info )
	return;

    // we can only hope that this cast works
    KFileIconViewItem *item = (KFileIconViewItem*)info->viewItem( this );

    if ( item ) {
	if ( !item->isSelected() )
	    KIconView::setSelected( item, enable, true );
	if ( KIconView::currentItem() != item )
	    KIconView::setCurrentItem( item );
    }
}

void KFileIconView::clearSelection()
{
    QIconView::clearSelection();
}

void KFileIconView::clearView()
{
    for (KFileViewItem* first = KFileView::firstItem(); first; first = first->next())
	first->setViewItem(this, (const void*)0);

    QIconView::clear();
}

void KFileIconView::insertItem( KFileViewItem *i )
{
    KFileIconViewItem *item =
	new KFileIconViewItem( (QIconView*)this, i->name(),
			       i->pixmap( myIconSize ), i);

    i->setViewItem( this, item );
}

void KFileIconView::slotDoubleClicked( QIconViewItem *item )
{
    if ( !item )
	return;
    const KFileViewItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
    if ( fi )
	select( fi );
}

void KFileIconView::selected( QIconViewItem *item )
{
    if ( !item )
	return;

    if ( KGlobalSettings::singleClick() ) {
	const KFileViewItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
	if ( fi && (fi->isDir() || !onlyDoubleClickSelectsFiles()) )
	    select( fi );
    }
}

void KFileIconView::highlighted( QIconViewItem *item )
{
    if ( !item )
	return;
    const KFileViewItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
    if ( fi )
	highlight( fi );
}

void KFileIconView::setSelectionMode( KFile::SelectionMode sm )
{
    disconnect( this, SIGNAL( selectionChanged() ));
    disconnect( this, SIGNAL( selectionChanged( QIconViewItem * )));

    KFileView::setSelectionMode( sm );
    switch ( KFileView::selectionMode() ) {
    case KFile::Multi:
	QIconView::setSelectionMode( QIconView::Multi );
	break;
    case KFile::Extended:
	QIconView::setSelectionMode( QIconView::Extended );
	break;
    case KFile::NoSelection:
	QIconView::setSelectionMode( QIconView::NoSelection );
	break;
    default: // fall through
    case KFile::Single:
	QIconView::setSelectionMode( QIconView::Single );
	break;
    }

    if ( sm == KFile::Multi || sm == KFile::Extended )
	connect( this, SIGNAL( selectionChanged() ),
		 SLOT( slotSelectionChanged() ));
    else
	connect( this, SIGNAL( selectionChanged( QIconViewItem * )),
		 SLOT( highlighted( QIconViewItem * )));
}

bool KFileIconView::isSelected( const KFileViewItem *i ) const
{
    if ( !i )
	return false;
    KFileIconViewItem *item = (KFileIconViewItem*)i->viewItem( this );
    return (item && item->isSelected());
}

void KFileIconView::updateView( bool b )
{
    if ( b ) {
	KFileIconViewItem *item = static_cast<KFileIconViewItem*>(QIconView::firstItem());
	if ( item ) {
	    do {
		item ->setPixmap( (item->fileInfo())->pixmap( myIconSize ) );
		item = static_cast<KFileIconViewItem *>(item->nextItem());
	    } while ( item != 0L );
	}
    }
}

void KFileIconView::updateView( const KFileViewItem *i )
{
    if ( !i )
	return;
    KFileIconViewItem *item = (KFileIconViewItem*)i->viewItem( this );
    if ( item )
      item->setPixmap( i->pixmap( myIconSize ) );
}

void KFileIconView::removeItem( const KFileViewItem *i )
{
    if ( !i )
	return;
    delete (KFileIconViewItem*)i->viewItem( this );
    KFileView::removeItem( i );
}

void KFileIconView::setIconSize( int size )
{
    myIconSize = size;
    updateView( true );
    arrangeItemsInGrid();
}


void KFileIconView::ensureItemVisible( const KFileViewItem *i )
{
    if ( !i )
	return;
    KFileIconViewItem *item = (KFileIconViewItem*)i->viewItem( this );
    if ( item )
	KIconView::ensureItemVisible( item );
}

void KFileIconView::slotSelectionChanged()
{
    highlight( 0L );
}

void KFileIconView::slotSmallColumns()
{
    setItemTextPos( Right );
    setArrangement( TopToBottom );
    setIconSize( KIcon::SizeSmall );
}

void KFileIconView::slotLargeRows()
{
    setItemTextPos( Bottom );
    setArrangement( LeftToRight );
    setIconSize( KIcon::SizeMedium );
}

#include "kfileiconview.moc"
