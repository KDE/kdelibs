// -*- c++ -*-
/* This file is part of the KDE libraries
   Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
                 2000,2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>

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
#include <qregexp.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kglobalsettings.h>
#include <kio/previewjob.h>

#include "kfileiconview.h"
#include "config-kfile.h"


KFileIconViewItem::~KFileIconViewItem()
{
    fileInfo()->removeExtraData( iconView() );
}

class KFileIconView::KFileIconViewPrivate
{
public:
    KFileIconViewPrivate( KFileIconView *parent ) {
        previewIconSize = 60;
        job = 0L;

	smallColumns = new KRadioAction( i18n("Small Icons"), 0, parent,
					 SLOT( slotSmallColumns() ),
					 parent->actionCollection(),
					 "small columns" );

	largeRows = new KRadioAction( i18n("Large Icons"), 0, parent,
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
    setGridX( 160 );
    setWordWrapIconText( false );
    setArrangement( TopToBottom );
    setAutoArrange( true );
    setItemsMovable( false );
    setMode( KIconView::Select );
    KIconView::setSorting( true );
    // as long as QIconView only shows tooltips when the cursor is over the
    // icon (and not the text), we have to create our own tooltips
    setShowToolTips( false );
    slotSmallColumns();
    d->smallColumns->setChecked( true );

    connect( this, SIGNAL( returnPressed(QIconViewItem *) ),
	     SLOT( slotActivate( QIconViewItem *) ) );

    // we want single click _and_ double click (as convenience)
    connect( this, SIGNAL( clicked(QIconViewItem *, const QPoint&) ),
	     SLOT( selected( QIconViewItem *) ) );
    connect( this, SIGNAL( doubleClicked(QIconViewItem *, const QPoint&) ),
	     SLOT( slotActivate( QIconViewItem *) ) );

    connect( this, SIGNAL( onItem( QIconViewItem * ) ),
	     SLOT( showToolTip( QIconViewItem * ) ) );
    connect( this, SIGNAL( onViewport() ),
	     SLOT( removeToolTip() ) );
    connect( this, SIGNAL( contextMenuRequested(QIconViewItem*,const QPoint&)),
	     SLOT( slotActivateMenu( QIconViewItem*, const QPoint& ) ) );

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

    viewport()->installEventFilter( this );

    // for mimetype resolving
    m_resolver = new KMimeTypeResolver<KFileIconViewItem,KFileIconView>(this);

//###    readConfig();
}

KFileIconView::~KFileIconView()
{
// ###    writeConfig();
    delete m_resolver;
    removeToolTip();
    delete d;
}

void KFileIconView::readConfig( KConfig *kc, const QString& group )
{
    QString gr = group.isEmpty() ? QString("KFileIconView") : group;
    KConfigGroupSaver cs( kc, gr );
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

void KFileIconView::writeConfig( KConfig *kc, const QString& group )
{
    QString gr = group.isEmpty() ? QString("KFileIconView") : group;
    KConfigGroupSaver cs( kc, gr );
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
	QRect screen = QApplication::desktop()->screenGeometry(
			QApplication::desktop()->screenNumber(QCursor::pos()));
	if (toolTip->x()+toolTip->width() > screen.right()) {
		toolTip->move(toolTip->x()+screen.right()-toolTip->x()-toolTip->width(), toolTip->y());
	}
	if (toolTip->y()+toolTip->height() > screen.bottom()) {
		toolTip->move(toolTip->x(), screen.bottom()-toolTip->y()-toolTip->height()+toolTip->y());
	}
	toolTip->setFont( QToolTip::font() );
	toolTip->setPalette( QToolTip::palette(), TRUE );
	toolTip->show();
    }
}

void KFileIconView::slotActivateMenu( QIconViewItem* item, const QPoint& pos )
{
    if ( !item ) {
	sig->activateMenu( 0, pos );
	return;
    }
    KFileIconViewItem *i = (KFileIconViewItem*) item;
    sig->activateMenu( i->fileInfo(), pos );
}

void KFileIconView::hideEvent( QHideEvent *e )
{
    removeToolTip();
    KIconView::hideEvent( e );
}

void KFileIconView::keyPressEvent( QKeyEvent *e )
{
    KIconView::keyPressEvent( e );

    // ignore Ctrl-Return so that the dialog can catch it.
    if ( (e->state() & ControlButton) &&
         (e->key() == Key_Return || e->key() == Key_Enter) )
        e->ignore();
}

void KFileIconView::setSelected( const KFileItem *info, bool enable )
{
    KFileIconViewItem *item = viewItem( info );
    if ( item )
        KIconView::setSelected( item, enable, true );
}

void KFileIconView::selectAll()
{
    if (KFileView::selectionMode() == KFile::NoSelection ||
        KFileView::selectionMode() == KFile::Single)
	return;

    KIconView::selectAll( true );
}

void KFileIconView::clearSelection()
{
    KIconView::clearSelection();
}

void KFileIconView::invertSelection()
{
    KIconView::invertSelection();
}

void KFileIconView::clearView()
{
    m_resolver->m_lstPendingMimeIconItems.clear();

    KIconView::clear();
    stopPreview();
}

void KFileIconView::insertItem( KFileItem *i )
{
    KFileView::insertItem( i );

    KFileIconViewItem *item = new KFileIconViewItem( (QIconView*)this, i );
    initItem( item, i );

    if ( !i->isMimeTypeKnown() )
        m_resolver->m_lstPendingMimeIconItems.append( item );

    i->setExtraData( this, item );
}

void KFileIconView::slotActivate( QIconViewItem *item )
{
    if ( !item )
	return;
    const KFileItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
    if ( fi )
	sig->activate( fi );
}

void KFileIconView::selected( QIconViewItem *item )
{
    if ( !item )
	return;

    if ( KGlobalSettings::singleClick() ) {
	const KFileItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
	if ( fi && (fi->isDir() || !onlyDoubleClickSelectsFiles()) )
	    sig->activate( fi );
    }
}

void KFileIconView::setCurrentItem( const KFileItem *item )
{
    KFileIconViewItem *it = viewItem( item );
    if ( it )
        KIconView::setCurrentItem( it );
}

KFileItem * KFileIconView::currentFileItem() const
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
    const KFileItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
    if ( fi )
	sig->highlightFile( fi );
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

bool KFileIconView::isSelected( const KFileItem *i ) const
{
    KFileIconViewItem *item = viewItem( i );
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

void KFileIconView::updateView( const KFileItem *i )
{
    KFileIconViewItem *item = viewItem( i );
    if ( item )
        initItem( item, i );
}

void KFileIconView::removeItem( const KFileItem *i )
{
    if ( !i )
	return;

    KFileIconViewItem *item = viewItem( i );
    m_resolver->m_lstPendingMimeIconItems.remove( item );
    delete item;

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

void KFileIconView::ensureItemVisible( const KFileItem *i )
{
    KFileIconViewItem *item = viewItem( i );
    if ( item )
	KIconView::ensureItemVisible( item );
}

void KFileIconView::slotSelectionChanged()
{
    sig->highlightFile( 0L );
}

void KFileIconView::slotSmallColumns()
{
    // Make sure to uncheck previews if selected
    if ( d->previews->isChecked() )
    {
        stopPreview();
        d->previews->setChecked( false );
    }
    setItemTextPos( Right );
    setArrangement( TopToBottom );
    setWordWrapIconText( false );
    setGridX( 160 );
    setIconSize( KIcon::SizeSmall );

    arrangeItemsInGrid();
}

void KFileIconView::slotLargeRows()
{
    setItemTextPos( Bottom );
    setArrangement( LeftToRight );
    setWordWrapIconText( true );
    setGridX( KGlobal::iconLoader()->currentSize( KIcon::Desktop ) + 50 );
    setIconSize( KIcon::SizeMedium );

    arrangeItemsInGrid();
}

void KFileIconView::stopPreview()
{
    if ( d->job ) {
        d->job->kill();
        d->job = 0L;
    }
}

void KFileIconView::slotPreviewsToggled( bool on )
{
    if ( on )
        showPreviews();
    else {
        stopPreview();
        slotLargeRows();
    }
}

void KFileIconView::showPreviews()
{
    if ( d->previewMimeTypes.isEmpty() )
        d->previewMimeTypes = KIO::PreviewJob::supportedMimeTypes();

    stopPreview();
    d->previews->setChecked( true );

    if ( !d->largeRows->isChecked() ) {
        d->largeRows->setChecked( true );
        slotLargeRows(); // also sets the icon size and updates the grid
    }
    else
        updateIcons();

    d->job = KIO::filePreview(*items(), d->previewIconSize,d->previewIconSize);

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
    KFileIconViewItem *it = viewItem( item );
    if ( it )
        it->setPixmap( pix );
}

bool KFileIconView::canPreview( const KFileItem *item ) const
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

KFileItem * KFileIconView::firstFileItem() const
{
    KFileIconViewItem *item = static_cast<KFileIconViewItem*>( firstItem() );
    if ( item )
        return item->fileInfo();
    return 0L;
}

KFileItem * KFileIconView::nextItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        KFileIconViewItem *item = viewItem( fileItem );
        if ( item && item->nextItem() )
            return ((KFileIconViewItem*) item->nextItem())->fileInfo();
    }
    return 0L;
}

KFileItem * KFileIconView::prevItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        KFileIconViewItem *item = viewItem( fileItem );
        if ( item && item->prevItem() )
            return ((KFileIconViewItem*) item->prevItem())->fileInfo();
    }
    return 0L;
}

void KFileIconView::setSorting( QDir::SortSpec spec )
{
    KFileView::setSorting( spec );
    KFileItemListIterator it( *items() );

    KFileItem *item;

    if ( spec & QDir::Time ) {
        for ( ; (item = it.current()); ++it )
            viewItem(item)->setKey( sortingKey( item->time( KIO::UDS_MODIFICATION_TIME ), item->isDir(), spec ));
    }

    else if ( spec & QDir::Size ) {
        for ( ; (item = it.current()); ++it )
            viewItem(item)->setKey( sortingKey( item->size(), item->isDir(),
                                                spec ));
    }
    else { // Name or Unsorted
        for ( ; (item = it.current()); ++it )
            viewItem(item)->setKey( sortingKey( item->text(), item->isDir(),
                                                spec ));
    }

    KIconView::setSorting( true, !isReversed() );
    sort( !isReversed() );
}

//
// mimetype determination on demand
//
void KFileIconView::mimeTypeDeterminationFinished()
{
    // anything to do?
}

void KFileIconView::determineIcon( KFileIconViewItem *item )
{
    (void) item->fileInfo()->determineMimeType();
    updateView( item->fileInfo() );
}

void KFileIconView::listingCompleted()
{
    // QIconView doesn't set the current item automatically, so we have to do
    // that. We don't want to emit selectionChanged() tho.
    if ( !currentItem() ) {
        bool block = signalsBlocked();
        blockSignals( true );
        QIconViewItem *item = viewItem( firstFileItem() );
        KIconView::setCurrentItem( item );
        KIconView::setSelected( item, false );
        blockSignals( block );
    }

    arrangeItemsInGrid();
    m_resolver->start( d->previews->isChecked() ? 0 : 10 );
}

// need to remove our tooltip, eventually
bool KFileIconView::eventFilter( QObject *o, QEvent *e )
{
    if ( o == viewport() || o == this ) {
        int type = e->type();
        if ( type == QEvent::Leave ||
             type == QEvent::FocusOut )
            removeToolTip();
    }

    return KIconView::eventFilter( o, e );
}

/////////////////////////////////////////////////////////////////

// ### workaround for Qt3 Bug
void KFileIconView::showEvent( QShowEvent *e )
{
    KIconView::showEvent( e );
#if QT_VERSION <= 302
    sort( !isReversed() );
#endif
}


void KFileIconView::initItem( KFileIconViewItem *item, const KFileItem *i )
{
    int size = myIconSize;
    if ( d->previews->isChecked() && canPreview( i ) )
        size = d->previewIconSize;

    item->setText( i->text() );
    item->setPixmap( i->pixmap( size ) );

    // see also setSorting()
    QDir::SortSpec spec = KFileView::sorting();

    if ( spec & QDir::Time )
        item->setKey( sortingKey( i->time( KIO::UDS_MODIFICATION_TIME ),
                                  i->isDir(), spec ));
    else if ( spec & QDir::Size )
        item->setKey( sortingKey( i->size(), i->isDir(), spec ));

    else // Name or Unsorted
        item->setKey( sortingKey( i->text(), i->isDir(), spec ));

    //qDebug("** key for: %s: %s", i->text().latin1(), item->key().latin1());

    if ( d->previews->isChecked() )
        d->previewTimer->start( 10, true );
}

void KFileIconView::virtual_hook( int id, void* data )
{ KIconView::virtual_hook( id, data );
  KFileView::virtual_hook( id, data ); }

#include "kfileiconview.moc"
