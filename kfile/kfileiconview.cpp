// -*- c++ -*-
/* This file is part of the KDE libraries
   Copyright (C) 1997 Stephan Kulow <coolo@kde.org>

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

#include "kfilereader.h"
#include <qpixmap.h>
#include "kfileiconview.h"
#include "qkeycode.h"
#include <qpainter.h>
#include <kapp.h>
#include "config-kfile.h"
#include <kfileviewitem.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qfontmetrics.h>

KFileIconViewItem::~KFileIconViewItem()
{
    const_cast<KFileViewItem*>(fileInfo())->
	setViewItem(static_cast<KFileIconView*>(iconView()), (const void*)0);
}

KFileIconView::KFileIconView(QWidget *parent, const char *name)
    : KIconView(parent, name), KFileView()
{
    toolTip = 0;
    setResizeMode( Adjust );
    // setAlignMode( South );
    setItemTextPos( Right );
    setGridX( 120 );
    setWordWrapIconText( FALSE );
    setAligning( TRUE );

    connect( this, SIGNAL( doubleClicked(QIconViewItem *) ),
	     SLOT( selected( QIconViewItem *) ) );
    connect( this, SIGNAL( returnPressed(QIconViewItem *) ),
	     SLOT( selected( QIconViewItem *) ) );
    connect( this, SIGNAL( currentChanged( QIconViewItem *) ),
	     this, SLOT( highlighted( QIconViewItem *)	) );
    connect( this, SIGNAL( onItem( QIconViewItem * ) ),
	     this, SLOT( showToolTip( QIconViewItem * ) ) );
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( removeToolTip() ) );
    connect( this, SIGNAL( rightButtonPressed( QIconViewItem*, const QPoint&) ),
	     SLOT( slotRightButtonPressed( QIconViewItem* ) ) );

    if ( KFileView::selectMode() == KFileView::Single )
	QIconView::setSelectionMode( QIconView::Single );
    else
	QIconView::setSelectionMode( QIconView::Extended );

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
	toolTip = new QLabel( QString::fromLatin1(" %1 ").arg(item->text()), 0, "myToolTip",
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

KFileIconView::~KFileIconView()
{
}

void KFileIconView::highlightItem( const KFileViewItem *info )
{
    if ( !info )
	return;

    // we can only hope that this casts works
    KFileIconViewItem *item = (KFileIconViewItem*)info->viewItem( this );
    if ( !item )
	return;

    if ( item != currentItem() ) {
        QIconView::setCurrentItem( item );
	QIconView::ensureItemVisible( item );
	setSelected( item, TRUE );
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
			       i->pixmap(), i);

    i->setViewItem( this, item );
}

void KFileIconView::selected( QIconViewItem *item )
{
    if ( !item )
	return;
    const KFileViewItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
    if ( fi ) {
	select( const_cast<KFileViewItem*>( fi ) );
	// FIXME: why is selected() called, when we just highlighted a file?
	// on highlighting, we can't scroll away...
	// setContentsPos( 0, 0 ); // scroll to top left
    }
}

void KFileIconView::highlighted( QIconViewItem *item )
{
    if ( !item )
	return;
    const KFileViewItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
    if ( fi )
	highlight( const_cast<KFileViewItem*>( fi ) );
}

void KFileIconView::setSelectMode( KFileView::SelectionMode sm )
{
    KFileView::setSelectMode( sm );
    if ( KFileView::selectMode() == KFileView::Single )
	QIconView::setSelectionMode( QIconView::Single );
    else
	QIconView::setSelectionMode( QIconView::Extended );
}


void KFileIconView::updateView( bool b )
{
    if ( b ) {
	KFileIconViewItem *item = static_cast<KFileIconViewItem*>(QIconView::firstItem());
	if ( item ) {
	    do {
		item ->setPixmap( const_cast<KFileViewItem*>(item->fileInfo())->pixmap() );
		item = static_cast<KFileIconViewItem *>(item->nextItem());
	    } while ( item != 0L );
	}
    }
}

void KFileIconView::updateView( const KFileViewItem *i )
{
    KFileViewItem *it = (KFileViewItem*)i->viewItem( this );
    if ( !it )
	return;
    KFileIconViewItem *item = (KFileIconViewItem*)it;
    if ( item )
	item ->setPixmap( item->fileInfo()->pixmap() );
}

#include "kfileiconview.moc"
