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
#include <qtimer.h>
#include <qtooltip.h>

#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kfileviewitem.h>
#include <kglobalsettings.h>
#include <kio/previewjob.h>

#include "kfileiconview.h"
#include "config-kfile.h"


KFileIconViewItem::~KFileIconViewItem()
{
    fileInfo()->
	setViewItem(static_cast<KFileIconView*>(iconView()), (const void*)0);
}

class KFileIconView::KFileIconViewPrivate
{
public:
    KFileIconViewPrivate( KFileIconView *parent ) {
        previewIconSize = 60;
        job = 0L;

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

        previews = new KToggleAction( i18n("Thumbnail Previews"), 0,
                                      parent->actionCollection(),
                                      "show previews" );
        connect( previews, SIGNAL( toggled( bool )),
                 parent, SLOT( slotPreviewsToggled( bool )));

        previewTimer = new QTimer;
        connect( previewTimer, SIGNAL( timeout() ),
                 parent, SLOT( showPreviews() ));
    }

    ~KFileIconViewPrivate() {
        delete previewTimer;
        if ( job )
            job->kill();
    }

    KRadioAction *smallColumns, *largeRows;
    KToggleAction *previews;
    KIO::PreviewJob *job;
    QTimer *previewTimer;
    QStringList previewMimeTypes;
    int previewIconSize;
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
    d->previewIconSize = kc->readNumEntry( "Preview Size", 60 );

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
    // kc->writeEntry( "Preview Size", d->previewIconSize );
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
			      WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM );
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
    if ( d->job ) {
        d->job->kill();
        d->job = 0L;
    }
}

void KFileIconView::insertItem( KFileViewItem *i )
{
    int size = myIconSize;
    if ( d->previews->isChecked() && canPreview( i ) )
        size = myIconSize;

    KFileIconViewItem *item = new KFileIconViewItem( (QIconView*)this,
                                                     i->name(),
                                                     i->pixmap( size ), i);

    i->setViewItem( this, item );

    if ( d->previews->isChecked() )
        d->previewTimer->start( 10, true );
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

void KFileIconView::setCurrentItem( const KFileViewItem *item )
{
    if ( !item )
        return;
    KFileIconViewItem *it = (KFileIconViewItem*) item->viewItem( this );
    if ( it )
        KIconView::setCurrentItem( it );
}

KFileViewItem * KFileIconView::currentFileItem() const
{
    KFileIconViewItem *current = static_cast<KFileIconViewItem*>( currentItem() );
    if ( current )
        return current->fileInfo();

    return 0L;
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
            if ( d->previews->isChecked() ) {
                do {
                    int size = canPreview( item->fileInfo() ) ?
                               d->previewIconSize : myIconSize;
                    item ->setPixmap( (item->fileInfo())->pixmap( size ) );
                    item = static_cast<KFileIconViewItem *>(item->nextItem());
                } while ( item != 0L );
            }

            else {
                do {
                    item ->setPixmap( (item->fileInfo())->pixmap( myIconSize));
                    item = static_cast<KFileIconViewItem *>(item->nextItem());
                } while ( item != 0L );
            }
	}
    }
}

void KFileIconView::updateView( const KFileViewItem *i )
{
    if ( !i )
	return;
    KFileIconViewItem *item = (KFileIconViewItem*)i->viewItem( this );
    if ( item ) {
        int size = myIconSize;
        if ( d->previews->isChecked() && canPreview( i ) )
            size = myIconSize;

        item->setPixmap( i->pixmap( size ) );
    }
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
    updateIcons();
}

void KFileIconView::setPreviewSize( int size )
{
    d->previewIconSize = size;
    if ( d->previews->isChecked() )
        updateIcons();
}

void KFileIconView::updateIcons()
{
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

    if ( d->job ) {
        d->job->kill();
        d->job = 0L;
    }
}

void KFileIconView::slotLargeRows()
{
    setItemTextPos( Bottom );
    setArrangement( LeftToRight );
    setIconSize( KIcon::SizeMedium );
}

void KFileIconView::slotPreviewsToggled( bool on )
{
    if ( on )
        showPreviews();
    else {
        if ( d->job ) {
            d->job->kill();
            d->job = 0L;
        }
        slotLargeRows();
    }
}

void KFileIconView::showPreviews()
{
    if ( d->previewMimeTypes.isEmpty() )
        d->previewMimeTypes = KIO::PreviewJob::supportedMimeTypes();

    d->previews->setChecked( true );

    if ( !d->largeRows->isChecked() ) {
        d->largeRows->setChecked( true );
        slotLargeRows(); // also sets the icon size and updates the grid
    }
    else
        updateIcons();

    KFileItemList items;
    KFileViewItem *item = KFileView::firstItem();
    while( item ) {
        items.append( item );
        item = item->next();
    }

    if ( d->job )
        d->job->kill();

    d->job = KIO::filePreview( items, d->previewIconSize, d->previewIconSize );

    connect( d->job, SIGNAL( result( KIO::Job * )),
             this, SLOT( slotPreviewResult( KIO::Job * )));
    connect( d->job, SIGNAL( gotPreview( const KFileItem*, const QPixmap& )),
             SLOT( gotPreview( const KFileItem*, const QPixmap& ) ));
//     connect( d->job, SIGNAL( failed( const KFileItem* )),
//              this, SLOT( slotFailed( const KFileItem* ) ));
}

void KFileIconView::slotPreviewResult( KIO::Job *job )
{
    if ( job == d->job )
        d->job = 0L;
}

void KFileIconView::gotPreview( const KFileItem *item, const QPixmap& pix )
{
    // we can only hope that this cast works
    KFileIconViewItem *it = (KFileIconViewItem*) (static_cast<const KFileViewItem*>( item ))->viewItem( this );
    if ( it ) {
        it->setPixmap( pix );
    }
}

bool KFileIconView::canPreview( const KFileViewItem *item ) const
{
    QStringList::Iterator it = d->previewMimeTypes.begin();
    QRegExp r;
    r.setWildcard( true );

    for ( ; it != d->previewMimeTypes.end(); ++it ) {
        QString type = *it;
        // the "mimetype" can be "image/*"
        if ( type.at( type.length() - 1 ) == '*' ) {
            r.setPattern( type );
            if ( r.search( item->mimetype() ) != -1 )
                return true;
        }
        else
            if ( item->mimetype() == type )
                return true;
    }

    return false;
}

#include "kfileiconview.moc"
