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

#include "kfileview.h"
#include <qsignal.h>
#include <kapp.h>
#include "config-kfile.h"
#include <stdlib.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <assert.h>

#ifdef Unsorted // the "I hate X.h" modus
#undef Unsorted
#endif

QDir::SortSpec KFileView::defaultSortSpec = static_cast<QDir::SortSpec>(QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);

KFileView::KFileView()
{
    reversed   = false;        // defaults
    itemListDirty = true;
    mySortMode = Increasing;
    mySorting  = KFileView::defaultSortSpec;

    sig = new KFileViewSignaler();
    sig->setName("view-signaller");

    itemList = 0L;
    selectedList = 0L;
    filesNumber = 0;
    dirsNumber = 0;
    first = 0;
    // last = 0;

    view_mode = All;
    selection_mode = Single;
    viewname = i18n("Unknown View");
}

KFileView::~KFileView()
{
    delete sig;
    delete itemList;
}

void KFileView::setOperator(QObject *ops)
{
    if (ops) {
	QObject::connect(sig,
			 SIGNAL( activatedMenu(const KFileViewItem * ) ),
			 ops, SLOT(activatedMenu(const KFileViewItem * ) ) );
	
	QObject::connect(sig,
			 SIGNAL( dirActivated(const KFileViewItem *) ),
			 ops, SLOT( selectDir(const KFileViewItem*) ) );
	
	QObject::connect(sig,
			 SIGNAL( fileSelected(const KFileViewItem *) ),
			 ops, SLOT( selectFile(const KFileViewItem*) ) );
	
	QObject::connect(sig,
			 SIGNAL( fileHighlighted(const KFileViewItem *) ),
			 ops, SLOT( highlightFile(const KFileViewItem*) ) );
    } else
	sig->disconnect((QObject*)0);
}

void KFileView::activateMenu( const KFileViewItem *i )
{
    sig->activateMenu( i );
}

/*
bool KFileView::addItem( KFileViewItem *i )
{
    if (!updateNumbers(i))
	return false;

    return insertItem(i);
}
*/
bool KFileView::updateNumbers(const KFileViewItem *i)
{
    if (i->isHidden())
	return false;

    if (!( viewMode() & Files ) && i->isFile())
	return false;

    if (!( viewMode() & Directories ) && i->isDir())
	return false;

    if (i->isDir())
	dirsNumber++;
    else
	filesNumber++;

    return true;
}

void KFileView::addItemList(const KFileViewItemList *list)
{
    KFileViewItem *tmp, *tfirst = 0;
    int counter = 0;

    for (KFileViewItemListIterator it(*list); it.current(); ++it) {

	tmp = it.current();

	if (!updateNumbers(tmp))
	    continue;

	counter++;

	if (!tfirst) {
	    tfirst = tmp;
	    tfirst->setNext(0);
	    continue;
	}
	
	tmp->setNext(tfirst);
	tfirst = tmp;
    }
    insertSorted(tfirst, counter);
}

void qt_qstring_stats();

void KFileView::insertSorted(KFileViewItem *tfirst, uint counter)
{
    kDebugInfo(kfile_area, "insertedSorted %ld %d", time(0), counter);

    KFileViewItem **sortedArray = new KFileViewItem*[counter];
    KFileViewItem *it;
    uint index;

    for (it = tfirst, index = 0; it; index++, it = it->next())
	sortedArray[index] = it;

    ASSERT(index == counter);

    QuickSort(sortedArray, 0, counter - 1);
    tfirst = sortedArray[0];
    tfirst->setNext(0);

    KFileViewItem *tlast = tfirst;
    for (index = 1; index < counter; index++) {
	tlast->setNext(sortedArray[index]);
	tlast = sortedArray[index];
    }
    if ( counter >= 2 ) // terminate last item
	tlast->setNext(0);

    delete [] sortedArray;

    kDebugInfo(kfile_area, "inserting %ld %p", time(0), first);

#if 0
    for (it = first; it; it = it->next())
      removeItem(it);
#else
    clearView();
#endif

    first = mergeLists(first, tfirst);

    for (it = first; it; it = it->next())
	insertItem(it);

#ifdef Q2HELPER
    qt_qstring_stats();
#endif
}

KFileViewItem *KFileView::mergeLists(KFileViewItem *list1, KFileViewItem *list2)
{
    // this is the central place where we know that our itemList is dirty
    itemListDirty = true;

    if (!list1)
	return list2;

    if (!list2)
	return list1;

    if ( list1 == list2 ) {
	debug("**** KFileView: FIX this bug (items in mergeLists are equal)");
    }

    KFileViewItem *newlist;

    if (compareItems(list1, list2) < 0) {
	newlist = list1;
	list1 = list1->next();
    } else {
	newlist = list2;
	list2 = list2->next();
    }

    KFileViewItem *newstart = newlist;

    while (list1 || list2) {

	if (!list1) { // first list empty
	    newlist->setNext(list2);
	    break;
	}
	if (!list2) { // second list empty
	    newlist->setNext(list1);
	    break;
	}
	if (compareItems(list1, list2) < 0) {
	    newlist->setNext(list1);
	    newlist = list1;
	    list1 = list1->next();
	} else {
	    newlist->setNext(list2);
	    newlist = list2;
	    list2 = list2->next();
	}
    }

    return newstart;
}

void KFileView::setSorting(QDir::SortSpec new_sort)
{
    mySorting = new_sort;

    if ( count() > 1 ) {
	KFileViewItem *firstItem = first;
	first = 0L; // sideeffect - insertSorted would merge
	            // firstItem and first, which are actually the same
	insertSorted(firstItem, count());
    }
}

void KFileView::sortReversed()
{
    reversed = !reversed;

    if ( count() > 1 ) {
	KFileViewItem *firstItem = first;
	first = 0L; // sideeffect - insertSorted would merge
	            // firstItem and first, which are actually the same
	insertSorted(firstItem, count());
    }
}

void KFileView::clear()
{
    clearView();
    filesNumber = 0;
    dirsNumber = 0;
    first = 0;
    itemListDirty = true;
}

// this implementation is from the jdk demo Sorting
void KFileView::QuickSort(KFileViewItem* a[], int lo0, int hi0) const
{
    int lo = lo0;
    int hi = hi0;
    const KFileViewItem *mid;

    if ( hi0 >= lo0)
	{
	
	    /* Arbitrarily establishing partition element as the midpoint of
	     * the array.
	     */
	    mid = a[ ( lo0 + hi0 ) / 2 ];
	
	    // loop through the array until indices cross
	    while( lo <= hi )
		{
		    /* find the first element that is greater than or equal to
		     * the partition element starting from the left Index.
		     */
		    while( ( lo < hi0 ) && ( compareItems(a[lo], mid) < 0 ) )
			++lo;
		
		    /* find an element that is smaller than or equal to
		     * the partition element starting from the right Index.
		     */
		    while( ( hi > lo0 ) &&  ( compareItems(a[hi], mid) > 0) )
			--hi;

		    // if the indexes have not crossed, swap
		    if( lo <= hi )
			{
			    if (lo != hi) {
				const KFileViewItem *T = a[lo];
				a[lo] = a[hi];
				a[hi] = const_cast<KFileViewItem*>(T);
			    }
			    ++lo;
			    --hi;
			}
		}
	
	    /* If the right index has not reached the left side of array
	     * must now sort the left partition.
	     */
	    if( lo0 < hi )
		QuickSort( a, lo0, hi );
	
	    /* If the left index has not reached the right side of array
	     * must now sort the right partition.
	     */
	    if( lo < hi0 )
		QuickSort( a, lo, hi0 );
	
	}
}

int KFileView::compareItems(const KFileViewItem *fi1, const KFileViewItem *fi2) const
{
    bool bigger = true;
    bool keepFirst = false;

    if (fi1 == fi2)
	return 0;

    bool dirsFirst = ((mySorting & QDir::DirsFirst) == QDir::DirsFirst);
    if ( fi1->isDir() != fi2->isDir() && dirsFirst ) {
	bigger = fi2->isDir();
	keepFirst = true;
    }
    else {

      QDir::SortSpec sort = static_cast<QDir::SortSpec>(mySorting & QDir::SortByMask);

      if (fi1->isDir() || fi2->isDir())
      sort = static_cast<QDir::SortSpec>(KFileView::defaultSortSpec & QDir::SortByMask);

      switch (sort) {
      case QDir::Unsorted:
	bigger = true;  // nothing
	break;
      case QDir::Size:
	bigger = (fi1->size() > fi2->size());
	break;
      case QDir::Time:
	bigger = (fi1->mTime() > fi2->mTime());
	break;
      case QDir::Name:
      default:
	if ( (mySorting & QDir::IgnoreCase) == QDir::IgnoreCase )
	  bigger = (fi1->name().lower() > fi2->name().lower());
	else
	  bigger = (fi1->name() > fi2->name());
	break;
      }
    }

    if (reversed && !keepFirst ) // don't reverse dirs to the end!
      bigger = !bigger;

    // kDebugInfo(kfile_area, "compare " + fi1->fileName() + " against " +
    // fi2->fileName() + ": " + ( bigger ? "bigger" : "smaller"));

    return (bigger ? 1 : -1);
}

void KFileView::select( const KFileViewItem *entry )
{
    kDebugInfo(kfile_area, "select %s", entry->name().ascii());
    assert(entry);

    if ( entry->isDir() ) {
	kDebugInfo(kfile_area, "selectDir %s", entry->name().ascii());
	sig->activateDir(entry);
    } else {
	sig->activateFile(entry);
    }
}

void KFileView::highlight( const KFileViewItem *entry )
{
    sig->highlightFile(entry);
}

void  KFileView::updateView(bool f)
{
    widget()->repaint(f);
}

void KFileView::updateView(const KFileViewItem *)
{
    kDebugInfo(kfile_area, "repaint not reimplemented in %s", widget()->name("KFileView"));
}

void KFileView::setCurrentItem(const QString &item,
			       const KFileViewItem *entry)
{
    if (!item.isNull()) {
	KFileViewItem *it = first;
	while (it) {
	    if (it->name() == item) {
		highlightItem(it);
		highlight(it);
		return;
	    }
	    it = it->next();
	}
    } else {
	highlightItem(entry);
	return;
    }

    warning("setCurrentItem: no match found.");
}

const KFileViewItemList * KFileView::items() const
{
    if ( itemListDirty ) {
	if ( !itemList )
	    itemList = new KFileViewItemList;

	itemListDirty = false;
	itemList->clear();
	if ( first ) {
	    KFileViewItem *item = 0L;
	    for (item = first; item; item = item->next())
		itemList->append( item );
	}
    }
    return itemList;
}


const KFileViewItemList * KFileView::selectedItems() const
{
    if ( !selectedList )
	selectedList = new KFileViewItemList;

    selectedList->clear();

    if ( first ) {
	KFileViewItem *item = 0L;
	for (item = first; item; item = item->next()) {
	    if ( isSelected( item ) )
		selectedList->append( item );
	}
    }

    return selectedList;
}

void KFileView::selectAll()
{
    KFileViewItem *item = 0L;
    for (item = first; item; item = item->next())
	highlightItem( item );
}

void KFileView::setSelectMode( SelectionMode sm )
{
    selection_mode = sm;
}

KFileView::SelectionMode KFileView::selectMode() const
{
    return selection_mode;
}

void KFileView::setViewMode( ViewMode vm )
{
    view_mode = vm;
}

KFileView::ViewMode KFileView::viewMode() const
{
    return view_mode;
}

#include "kfileview.moc"

