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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// $Id$

#include <assert.h>

#include "kfileviewitem.h"
#include "kcombiview.h"
#include "kfileiconview.h"
#include "kfiledetailview.h"
#include "config-kfile.h"

#include <qpainter.h>
#include <qlistbox.h>

#include <qdir.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>

#include <qvaluelist.h>

KCombiView::KCombiView( QWidget *parent, const char *name)
  : QSplitter( parent, name), KFileView(), right(0)
{
    KFileIconView *dirs = new KFileIconView( (QSplitter*)this, "left" );
    dirs->KFileView::setViewMode( Directories );
    dirs->setArrangement( QIconView::LeftToRight );
    left = dirs;
    dirs->setOperator(this);
}

KCombiView::~KCombiView()
{
    delete right;
}

void KCombiView::setRight(KFileView *view)
{
    right = view;
    right->KFileView::setViewMode( Files );
    setViewName( right->viewName() );

    QValueList<int> lst;
    lst << left->gridX() + 2 * left->spacing();
    setSizes( lst );
    setResizeMode( left, QSplitter::KeepSize );

    right->setOperator(this);
    right->setOnlyDoubleClickSelectsFiles( onlyDoubleClickSelectsFiles() );
}

void KCombiView::insertSorted(KFileViewItem *tfirst, uint)
{
    kdDebug(kfile_area) << "KCombiView::insertSorted\n";
    KFileViewItem *f_first = 0, *d_first = 0;
    uint dirs = 0, files = 0;

    KFileViewItem *tmp;

    if ( !right )
        kdFatal() << "You need to call setRight( someview ) before!" << endl;

    for (KFileViewItem *it = tfirst; it;) {
	tmp = it->next();

	if (it->isDir()) {
            left->updateNumbers(it);
	    if (!d_first) {
		d_first = it;
		d_first->setNext(0);
	    } else {
		it->setNext(d_first);
		d_first = it;
	    }
	    dirs++;
	} else {
            right->updateNumbers(it);
	    if (!f_first) {
		f_first = it;
		f_first->setNext(0);
	    } else {
		it->setNext(f_first);
		f_first = it;
	    }
	    files++;
	}
	it = tmp;
    }

    if (dirs)
	left->insertSorted(d_first, dirs);
    if (files)
	right->insertSorted(f_first, files);

    // ### OUCH! With this, KFileView only knows about the files, not the dirs!
    // Dunno what this breaks, at least completing of dirs is broken.
    setFirstItem( right->firstItem() );
}

void KCombiView::insertItem( KFileViewItem * )
{
    kdDebug(kfile_area) << "KCombiView::insertItem not implemented (as not needed :)" << endl;
}

void KCombiView::setSorting( QDir::SortSpec sort )
{
    if ( !right )
        kdFatal() << "You need to call setRight( someview ) before!" << endl;
    right->setSorting( sort );
    left->KFileView::setSorting( sort );
    // don't call KFileView::setSorting()! It would resort all items, what
    // we don't want (the child-views do this themselves)
    mySorting = right->sorting();
}

void KCombiView::sortReversed()
{
    if ( !right )
        kdFatal() << "You need to call setRight( someview ) before!" << endl;
    right->sortReversed();
    left->sortReversed();
    reversed = right->isReversed();
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

void KCombiView::updateView( const KFileViewItem *i )
{
    left->updateView( i );
    if ( right )
        right->updateView( i );
}

void KCombiView::removeItem( const KFileViewItem *i )
{
    left->removeItem( i );
    if ( right )
        right->removeItem( i );
    KFileView::removeItem( i );
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

bool KCombiView::isSelected( const KFileViewItem *item ) const
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

void KCombiView::setSelected( const KFileViewItem *item, bool enable )
{
    left->setSelected( item, enable );
    if ( right )
        right->setSelected( item, enable );
}

void KCombiView::selectDir(const KFileViewItem* item)
{
    sig->activateDir(item);
}

void KCombiView::highlightFile(const KFileViewItem* item)
{
    sig->highlightFile(item);
}

void KCombiView::selectFile(const KFileViewItem* item)
{
    sig->activateFile(item);
}

void KCombiView::activatedMenu(const KFileViewItem *item)
{
    sig->activateMenu(item);
}

void KCombiView::ensureItemVisible(const KFileViewItem *item)
{
    left->ensureItemVisible( item );
    if ( right )
        right->ensureItemVisible( item );
}

// ***************************************************************************

#include "kcombiview.moc"

