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
#include <kiconeffect.h>
#include <kglobalsettings.h>
#include <kurldrag.h>
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
        job = 0;
        dropItem = 0;

        noArrangement = false;
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
        zoomIn = KStdAction::zoomIn( parent, SLOT( zoomIn() ),
                                     parent->actionCollection(), "zoomIn" );
        zoomOut = KStdAction::zoomOut( parent, SLOT( zoomOut() ),
                                     parent->actionCollection(), "zoomOut" );

        previews->setGroup("previews");
        zoomIn->setGroup("previews");
        zoomOut->setGroup("previews");

        connect( previews, SIGNAL( toggled( bool )),
                 parent, SLOT( slotPreviewsToggled( bool )));

        connect( &previewTimer, SIGNAL( timeout() ),
                 parent, SLOT( showPreviews() ));
        connect( &autoOpenTimer, SIGNAL( timeout() ),
                 parent, SLOT( slotAutoOpen() ));
    }

    ~KFileIconViewPrivate() {
        if ( job )
            job->kill();
    }

    KRadioAction *smallColumns, *largeRows;
    KAction *zoomIn, *zoomOut;
    KToggleAction *previews;
    KIO::PreviewJob *job;
    KFileIconViewItem *dropItem;
    QTimer previewTimer;
    QTimer autoOpenTimer;
    QStringList previewMimeTypes;
    int previewIconSize;
    bool noArrangement :1;
};

KFileIconView::KFileIconView(QWidget *parent, const char *name)
    : KIconView(parent, name), KFileView()
{
    d = new KFileIconViewPrivate( this );

    setViewName( i18n("Icon View") );

    toolTip = 0;
    setResizeMode( Adjust );
    setMaxItemWidth( 300 );
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
}

KFileIconView::~KFileIconView()
{
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
    d->previews->setChecked( kc->readBoolEntry( "ShowPreviews", false ) );

    if ( kc->readEntry("ViewMode", small ) == small ) {
	d->smallColumns->setChecked( true );
	slotSmallColumns();
    }
    else {
	d->largeRows->setChecked( true );
	slotLargeRows();
    }

    if ( d->previews->isChecked() )
        showPreviews();
}

void KFileIconView::writeConfig( KConfig *kc, const QString& group )
{
    QString gr = group.isEmpty() ? QString("KFileIconView") : group;
    KConfigGroupSaver cs( kc, gr );
    kc->writeEntry( "ViewMode", d->smallColumns->isChecked() ?
		    QString::fromLatin1("SmallColumns") :
		    QString::fromLatin1("LargeRows") );
    kc->writeEntry( "Preview Size", d->previewIconSize );
    kc->writeEntry( "ShowPreviews", d->previews->isChecked() );
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
	toolTip->setPalette( QToolTip::palette(), true );
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

    QIconView* qview = static_cast<QIconView*>( this );
    // Since creating and initializing an item leads to a repaint, 
    // we disable updates on the IconView for a while.
    qview->setUpdatesEnabled( false );
    KFileIconViewItem *item = new KFileIconViewItem( qview, i );
    initItem( item, i, true );
    qview->setUpdatesEnabled( true );

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
    if ( !item || (KApplication::keyboardModifiers() & (KApplication::ShiftModifier | KApplication::ControlModifier)) != 0 )
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
    disconnect( SIGNAL( selectionChanged() ), this );
    disconnect( SIGNAL( selectionChanged( QIconViewItem * )), this );

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
    if ( !b )
        return; // eh?

    KFileIconViewItem *item = static_cast<KFileIconViewItem*>(QIconView::firstItem());
    if ( item ) {
        do {
            if ( d->previews->isChecked() ) {
                if ( canPreview( item->fileInfo() ) )
                    item->setPixmapSize( QSize( d->previewIconSize, d->previewIconSize ) );
            }
            else {
                // unset pixmap size (used for previews)
                if ( !item->pixmapSize().isNull() )
                    item->setPixmapSize( QSize( 0, 0 ) );
            }
            // recalculate item parameters but avoid an in-place repaint
            item->setPixmap( (item->fileInfo())->pixmap( myIconSize ), true, false );
            item = static_cast<KFileIconViewItem *>(item->nextItem());
        } while ( item != 0L );
    }
}

void KFileIconView::updateView( const KFileItem *i )
{
    KFileIconViewItem *item = viewItem( i );
    if ( item )
        initItem( item, i, true );
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
    if ( size < 30 )
        size = 30; // minimum

    d->previewIconSize = size;
    if ( d->previews->isChecked() )
        showPreviews();
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
    // setItemTextPos(), setArrangement(), setWordWrapIconText() and
    // setIconSize() all call arrangeItemsInGrid() :( Prevent this.
    d->noArrangement = true; // stop arrangeItemsInGrid()!

    // Make sure to uncheck previews if selected
    if ( d->previews->isChecked() )
    {
        stopPreview();
        d->previews->setChecked( false );
    }
    setGridX( -1 );
    setMaxItemWidth( 300 );
    setItemTextPos( Right );
    setArrangement( TopToBottom );
    setWordWrapIconText( false );
    setSpacing( 0 );

    d->noArrangement = false; // now we can arrange
    setIconSize( KIcon::SizeSmall );
}

void KFileIconView::slotLargeRows()
{
    // setItemTextPos(), setArrangement(), setWordWrapIconText() and
    // setIconSize() all call arrangeItemsInGrid() :( Prevent this.
    d->noArrangement = true; // stop arrangeItemsInGrid()!

    setGridX( KGlobal::iconLoader()->currentSize( KIcon::Desktop ) + 50 );
    setItemTextPos( Bottom );
    setArrangement( LeftToRight );
    setWordWrapIconText( true );
    setSpacing( 5 ); // default in QIconView

    d->noArrangement = false; // now we can arrange
    setIconSize( KIcon::SizeMedium );
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
    else {
        updateIcons();
    }

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
        if( item->overlays() & KIcon::HiddenOverlay )
        {
            QPixmap p( pix );

            KIconEffect::semiTransparent( p );
            it->setPixmap( p );
        }
        else
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
            // warning, time_t is often signed -> cast it
            viewItem(item)->setKey( sortingKey( (unsigned long)item->time( KIO::UDS_MODIFICATION_TIME ), item->isDir(), spec ));
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
    arrangeItemsInGrid();
    
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
}


void KFileIconView::initItem( KFileIconViewItem *item, const KFileItem *i,
                              bool updateTextAndPixmap )
{
    if ( d->previews->isChecked() && canPreview( i ) )
        item->setPixmapSize( QSize( d->previewIconSize, d->previewIconSize ) );

    if ( updateTextAndPixmap )
    {
        // this causes a repaint of the item, which we want to avoid during
        // directory listing, when all items are created. We want to paint all
        // items at once, not every single item in that case.
        item->setText( i->text() , false, false );
        item->setPixmap( i->pixmap( myIconSize ) );
    }

    // see also setSorting()
    QDir::SortSpec spec = KFileView::sorting();

    if ( spec & QDir::Time )
        // warning, time_t is often signed -> cast it
        item->setKey( sortingKey( (unsigned long) i->time( KIO::UDS_MODIFICATION_TIME ),
                                  i->isDir(), spec ));
    else if ( spec & QDir::Size )
        item->setKey( sortingKey( i->size(), i->isDir(), spec ));

    else // Name or Unsorted
        item->setKey( sortingKey( i->text(), i->isDir(), spec ));

    //qDebug("** key for: %s: %s", i->text().latin1(), item->key().latin1());

    if ( d->previews->isChecked() )
        d->previewTimer.start( 10, true );
}

void KFileIconView::arrangeItemsInGrid( bool update )
{
    if ( d->noArrangement )
        return;

    KIconView::arrangeItemsInGrid( update );
}

void KFileIconView::zoomIn()
{
    setPreviewSize( d->previewIconSize + 30 );
}

void KFileIconView::zoomOut()
{
    setPreviewSize( d->previewIconSize - 30 );
}

QDragObject *KFileIconView::dragObject()
{
    // create a list of the URL:s that we want to drag
    KURL::List urls;
    KFileItemListIterator it( * KFileView::selectedItems() );
    for ( ; it.current(); ++it ){
        urls.append( (*it)->url() );
    }
    QPixmap pixmap;
    if( urls.count() > 1 )
        pixmap = DesktopIcon( "kmultiple", iconSize() );
    if( pixmap.isNull() )
        pixmap = currentFileItem()->pixmap( iconSize() );

    QPoint hotspot;
    hotspot.setX( pixmap.width() / 2 );
    hotspot.setY( pixmap.height() / 2 );
    QDragObject* myDragObject = new KURLDrag( urls, widget() );
    myDragObject->setPixmap( pixmap, hotspot );
    return myDragObject;
}

void KFileIconView::slotAutoOpen()
{
    d->autoOpenTimer.stop();
    if( !d->dropItem )
        return;

    KFileItem *fileItem = d->dropItem->fileInfo();
    if (!fileItem)
        return;

    if( fileItem->isFile() )
        return;

    if ( fileItem->isDir() || fileItem->isLink())
        sig->activate( fileItem );
}

bool KFileIconView::acceptDrag(QDropEvent* e) const
{
   return KURLDrag::canDecode( e ) &&
       (e->source()!=const_cast<KFileIconView*>(this)) &&
       ( e->action() == QDropEvent::Copy
      || e->action() == QDropEvent::Move
      || e->action() == QDropEvent::Link );
}

void KFileIconView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    KFileIconViewItem *item = dynamic_cast<KFileIconViewItem*>(findItem( contentsToViewport( e->pos() ) ));
    if ( item ) {  // are we over an item ?
       d->dropItem = item;
       d->autoOpenTimer.start( autoOpenDelay() ); // restart timer
    }
    else
    {
       d->dropItem = 0;
       d->autoOpenTimer.stop();
    }
}

void KFileIconView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    KFileIconViewItem *item = dynamic_cast<KFileIconViewItem*>(findItem( contentsToViewport( e->pos() ) ));
    if ( item ) {  // are we over an item ?
       if (d->dropItem != item)
       {
           d->dropItem = item;
           d->autoOpenTimer.start( autoOpenDelay() ); // restart timer
       }
    }
    else
    {
       d->dropItem = 0;
       d->autoOpenTimer.stop();
    }
}

void KFileIconView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();
}

void KFileIconView::contentsDropEvent( QDropEvent *e )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();

    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    KFileIconViewItem *item = dynamic_cast<KFileIconViewItem*>(findItem( contentsToViewport( e->pos() ) ));
    KFileItem * fileItem = 0;
    if (item)
        fileItem = item->fileInfo();

    emit dropped(e, fileItem);

    KURL::List urls;
    if (KURLDrag::decode( e, urls ) && !urls.isEmpty())
    {
        emit dropped(e, urls, fileItem ? fileItem->url() : KURL());
        sig->dropURLs(fileItem, e, urls);
    }
}

void KFileIconView::virtual_hook( int id, void* data )
{ KIconView::virtual_hook( id, data );
  KFileView::virtual_hook( id, data ); }

#include "kfileiconview.moc"
