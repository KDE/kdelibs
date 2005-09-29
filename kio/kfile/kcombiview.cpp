/* This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// $Id$

#include <assert.h>

#include "kfileitem.h"
#include "kcombiview.h"
#include "kfileiconview.h"
#include "kfiledetailview.h"
#include "config-kfile.h"

#include <qevent.h>

#include <qdir.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>

#include <qvaluelist.h>

KCombiView::KCombiView( QWidget *parent, const char *name)
  : QSplitter( parent, name),
    KFileView(),
    right(0),
    m_lastViewForNextItem(0),
    m_lastViewForPrevItem(0)
{
    left = new KFileIconView( this, "left" );
    left->setAcceptDrops(false);
    left->viewport()->setAcceptDrops(false);
    left->setGridX( 160 );
    left->KFileView::setViewMode( Directories );
    left->setArrangement( QIconView::LeftToRight );
    left->setParentView( this );
    left->setAcceptDrops(false);
    left->installEventFilter( this );
    
    connect( sig, SIGNAL( sortingChanged( QDir::SortSpec ) ),
             SLOT( slotSortingChanged( QDir::SortSpec ) ));
}

KCombiView::~KCombiView()
{
    delete right;
}

void KCombiView::setRight(KFileView *view)
{
    delete right;
    right = view;
    right->KFileView::setViewMode( Files );
    setViewName( right->viewName() );

    QValueList<int> lst;
    lst << left->gridX() + 2 * left->spacing();
    setSizes( lst );
    setResizeMode( left, QSplitter::KeepSize );

    right->setParentView( this );
    right->widget()->setAcceptDrops(acceptDrops());
    right->setDropOptions(dropOptions());
    right->widget()->installEventFilter( this );
}


void KCombiView::insertItem( KFileItem *item )
{
    KFileView::insertItem( item );

    if ( item->isDir() ) {
        left->updateNumbers( item );
        left->insertItem( item );
    }
    else {
        right->updateNumbers( item );
        right->insertItem( item );
    }
}

void KCombiView::setSorting( QDir::SortSpec sort )
{
    if ( !right )
        kdFatal() << "You need to call setRight( someview ) before!" << endl;
    right->setSorting( sort );
    left->setSorting( sort );

    KFileView::setSorting( right->sorting() );
}

void KCombiView::clearView()
{
    left->clearView();
    if ( right )
        right->clearView();
}

void KCombiView::updateView( bool b )
{
    left->updateView( b );
    if ( right )
        right->updateView( b );
}

void KCombiView::updateView( const KFileItem *i )
{
    left->updateView( i );
    if ( right )
        right->updateView( i );
}

void KCombiView::removeItem( const KFileItem *i )
{
    left->removeItem( i );
    if ( right )
        right->removeItem( i );
    KFileView::removeItem( i );
}

void KCombiView::listingCompleted()
{
    left->listingCompleted();
    if ( right )
        right->listingCompleted();
}

void KCombiView::clear()
{
    KFileView::clear();
    left->KFileView::clear();
    if ( right )
        right->clear();
}

void KCombiView::clearSelection()
{
    left->clearSelection();
    if ( right )
        right->clearSelection();
}

void KCombiView::selectAll()
{
    left->selectAll();
    if ( right )
        right->selectAll();
}

void KCombiView::invertSelection()
{
    left->invertSelection();
    if ( right )
        right->invertSelection();
}

bool KCombiView::isSelected( const KFileItem *item ) const
{
    assert( right ); // for performance reasons no if ( right ) check.
    return (right->isSelected( item ) || left->isSelected( item ));
}

void KCombiView::setSelectionMode( KFile::SelectionMode sm )
{
    // I think the left view (directories should always be in
    // Single-Mode, right?
    // left->setSelectionMode( sm );
    if ( !right )
        kdFatal() << "You need to call setRight( someview ) before!" << endl;
    right->setSelectionMode( sm );
}

void KCombiView::setSelected( const KFileItem *item, bool enable )
{
    left->setSelected( item, enable );
    if ( right )
        right->setSelected( item, enable );
}

void KCombiView::setCurrentItem( const KFileItem *item )
{
    left->setCurrentItem( item );
    if ( right )
        right->setCurrentItem( item );
}

KFileItem * KCombiView::currentFileItem() const
{
    // we can actually have two current items, one in each view. So we simply
    // prefer the fileview's item over the directory's.
    // Smarter: if the right view has focus, prefer that over the left.
    if ( !right )
        return left->currentFileItem();

    KFileView *preferredView = focusView( right );
    KFileItem *item = preferredView->currentFileItem();
    if ( !item && preferredView != left )
        item = left->currentFileItem();

    return item;
}

void KCombiView::ensureItemVisible(const KFileItem *item)
{
    left->ensureItemVisible( item );
    if ( right )
        right->ensureItemVisible( item );
}

KFileItem * KCombiView::firstFileItem() const
{
    if ( !right )
        return left->firstFileItem();

    KFileView *preferredView = focusView( left );
    KFileView *otherView = (preferredView == left) ? right : left;
    KFileItem *item = preferredView->firstFileItem();
    if ( !item )
        item = otherView->firstFileItem();

    return item;
}

KFileItem * KCombiView::nextItem( const KFileItem *fileItem ) const
{
    if ( !right )
        return left->nextItem( fileItem );

    KFileView *preferredView = focusView( left );
    KFileView *otherView = (preferredView == left) ? right : left;
    KFileItem *item = preferredView->nextItem( fileItem );

    if ( item )
        m_lastViewForNextItem = preferredView;
    else { // no item, check other view
        // when changing from one to another view, we need to continue
        // with the next view's first item!
        if ( m_lastViewForNextItem != otherView ) {
            m_lastViewForNextItem = otherView;
            return otherView->firstFileItem();
        }

        item = otherView->nextItem( fileItem );
        m_lastViewForNextItem = otherView;
    }

    return item;
}

KFileItem * KCombiView::prevItem( const KFileItem *fileItem ) const
{
    if ( !right )
        return left->nextItem( fileItem );

    KFileView *preferredView = focusView( left );
    KFileView *otherView = (preferredView == left) ? right : left;
    KFileItem *item = preferredView->prevItem( fileItem );
    if ( item )
        m_lastViewForPrevItem = preferredView;

    else { // no item, check other view
        // when changing from one to another view, we need to continue
        // with the next view's last item!
        if ( m_lastViewForPrevItem != otherView ) {
            fileItem = otherView->firstFileItem();
            while ( otherView->nextItem( fileItem ) ) // find the last item
                fileItem = otherView->nextItem( fileItem );
        }

        item = otherView->prevItem( fileItem );
        m_lastViewForPrevItem = otherView;
    }

    return item;
}

void KCombiView::slotSortingChanged( QDir::SortSpec sorting )
{
    KFileView::setSorting( sorting );
}

KFileView *KCombiView::focusView( KFileView *preferred ) const
{
    QWidget *w = focusWidget();
    KFileView *other = (right == preferred) ? left : right;
    return (preferred && w == preferred->widget()) ? preferred : other;
}

void KCombiView::readConfig( KConfig *config, const QString& group )
{
    left->readConfig( config, group );
    if ( right )
        right->readConfig( config, group );
}

void KCombiView::writeConfig( KConfig *config, const QString& group )
{
    left->writeConfig( config, group );
    if ( right )
        right->writeConfig( config, group );
}

KActionCollection * KCombiView::actionCollection() const
{
    return focusView( right )->actionCollection();
}

void KCombiView::setAcceptDrops(bool b)
{
    left->setAcceptDrops(b);
    if (right)
       right->widget()->setAcceptDrops(b);
    QSplitter::setAcceptDrops(b);
}

void KCombiView::setDropOptions_impl(int options)
{
    KFileView::setDropOptions_impl(options);
    left->setDropOptions(options);
    if (right)
       right->setDropOptions(options);
}

void KCombiView::virtual_hook( int id, void* data )
{
    switch(id) {
      case VIRTUAL_SET_DROP_OPTIONS:
         setDropOptions_impl(*(int *)data);
         break;
      default:
         KFileView::virtual_hook( id, data );
    }
}

bool KCombiView::eventFilter( QObject *o, QEvent *e )
{
    int type = e->type();
    
    // only the focused view may have a selection
    if ( type == QEvent::FocusIn )
    {
        if ( o == left )
            right->clearSelection();
        else if ( o == right->widget() )
            left->clearSelection();
    }
    
    return QSplitter::eventFilter( o, e );
}

#include "kcombiview.moc"

