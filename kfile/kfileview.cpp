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
#include <kstddirs.h>
#include <kglobal.h>
#include <assert.h>

#ifdef Unsorted // the "I hate X.h" modus
#undef Unsorted
#endif

KFileView::KFileView()
{
    reversed   = false;        // defaults
    mySortMode = Increasing;
    mySorting  = QDir::Name;

    sig = new KFileViewSignaler();
    sig->setName("view-signaller");

    filesNumber = 0;
    dirsNumber = 0;
    first = 0;
    // last = 0;

    view_mode = All;
    selection_mode = Single;

}

KFileView::~KFileView()
{
    delete sig;
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
    debugC("insertedSorted %ld %d", time(0), counter);

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
	sortedArray[index]->setNext(0);
    }
    delete [] sortedArray;

    debugC("inserting %ld %p", time(0), first);

#if 0
      for (it = first; it; it = it->next()) {
      removeItem(it);
      }
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
    if (!list1)
	return list2;

    if (!list2)
	return list1;

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
    QDir::SortSpec old_sort =
	static_cast<QDir::SortSpec>(sorting() & QDir::SortByMask);
    QDir::SortSpec sortflags =
	static_cast<QDir::SortSpec>(sorting() & (~QDir::SortByMask));

    if (mySortMode == Switching) {
	if (new_sort == old_sort)
	    reversed = !reversed;
	else
	    reversed = false;
    } else
	reversed = (mySortMode == Decreasing);

    mySorting = static_cast<QDir::SortSpec>(new_sort | sortflags);

    if (count() <= 1) // nothing to do in this case
	return;

    insertSorted(first, count());
}

void KFileView::clear()
{
    clearView();
    filesNumber = 0;
    dirsNumber = 0;
    first = 0;
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

    if (fi1 == fi2)
	return 0;
	
    if (fi1->isDir() != fi2->isDir())
	bigger = !fi1->isDir();
    else {

      QDir::SortSpec sort = static_cast<QDir::SortSpec>(mySorting & QDir::SortByMask);

      if (fi1->isDir())
	sort = QDir::Name;

      switch (sort) {
      case QDir::Unsorted:
	bigger = true;  // nothing
	break;
      case QDir::Size:
	bigger = (fi1->size() > fi2->size());
	break;
      case QDir::Name:
      default:
	bigger = ( fi1->name() > fi2->name() );
	break;
      }
    }

    if (reversed)
      bigger = !bigger;

    // debugC("compare " + fi1->fileName() + " against " +
    // fi2->fileName() + ": " + ( bigger ? "bigger" : "smaller"));

    return (bigger ? 1 : -1);
}

void KFileView::select( const KFileViewItem *entry )
{
    debugC("select %s", entry->name().ascii());
    assert(entry);

    if ( entry->isDir() ) {
	debugC("selectDir %s", entry->name().ascii());
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
    debug("repaint not reimplemented in %s", widget()->name("KFileView"));
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

#if 0
QString KFileView::findCompletion( const char *base,
				   bool activateFound )
{

    if (!nameList) {
	uint i;
	
	nameList = new QStrIList();
	// prepare an array for quicksort. This is much faster than
	// calling n times inSort
	const char **nameArray = new const char*[sorted_length];
	// fill it
	for (i = 0; i < sorted_length; i++)
	    nameArray[i] = sortedArray[i]->fileName().ascii();
	
	qsort(nameArray, sorted_length, sizeof(const char*), (int (*)(const void *, const void *)) stricmp);
	
	// insert into list. Keeps deep copies
	for (i = 0; i < sorted_length; i++)
	    nameList->append(nameArray[i]);

	delete [] nameArray;
    }

    if ( strlen(base) == 0 ) return QString();

    QString remainder = base;
    const char *name;
    for ( name = nameList->first(); name; name = nameList->next() ) {
	if ( strlen(name) < remainder.length())
	    continue;
	if (strncmp(name, remainder.ascii(), remainder.length()) == 0)
	    break;
    }


    if (name) {
	
        QCString body = name;
        QCString backup = name;

        // get the possible text completions and store the smallest
	// common denominator in QString body

        unsigned int counter = strlen(base);
        for ( const char *extra = nameList->next(); extra;
	      extra = nameList->next() ) {
	
            counter = strlen(base);
            // case insensitive comparison needed because items are stored insensitively
            // so next instruction stop loop when first letter does no longer match
            if ( strnicmp( extra, remainder.ascii(), 1 ) != 0 )
                break;
            // this is case sensitive again!
            // goto next item if no completion possible with current item (->extra)
            if ( strncmp(extra, remainder.ascii(), remainder.length()) != 0 )
                continue;
            // get smallest common denominator
            for ( ; counter <= strlen(extra) ; counter++ ) {
                if (strncmp( extra, body.data(), counter) != 0)
                    break;
            }
            // truncate body, we have the smalles common denominator so far
            if ( counter == 1 )
                body.truncate(counter);
            else
            	body.truncate(counter-1);
            // this is needed because we want to highlight the first item in list
            // so we separately keep the "smallest" item separate,
            // we need the biggest in case the list is reversed
            if ( extra && ( reversed ? (QString::compare(extra, backup) > 0) : (QString::compare( extra, backup ) < 0) ) )
              backup = extra;
        }
	name = backup;

	debugC("completion base (%s) name (%s) body (%s)", base, name, body.data());

	bool matchExactly = (name == body);

	if (matchExactly && activateFound)
	    {
	      for (uint j = 0; j < sorted_length; j++)
		{
		  KFileViewItem *i = sortedArray[j];
		
		    if (i->fileName() == name) {
			if (sortedArray[j]->isDir())
			    body += "/";
			highlightItem(i);
			if (activateFound)
			    select(i);
			else
			    highlight(i);
			break;
		    }
		}
	    } else
		setCurrentItem(name); // the first matching name

	return body;
    } else {
	debugC("no completion for %s", base);
	return QString();
    }

}
#endif

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

