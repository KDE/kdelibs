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

#include "kfileinfocontents.h"
#include <qpixmap.h>
#include <qsignal.h>
#include <kapp.h>
#include "debug.h"
#include <stdlib.h>

#ifdef Unsorted // the "I hate X.h" modus
#undef Unsorted
#endif

QPixmap *KFileInfoContents::folder_pixmap = 0;
QPixmap *KFileInfoContents::locked_folder = 0;   
QPixmap *KFileInfoContents::file_pixmap = 0;
QPixmap *KFileInfoContents::locked_file = 0;   

KFileInfoContents::KFileInfoContents( bool use, QDir::SortSpec sorting )
{
    sorted_max = 10000;
    sortedArray = new KFileInfo*[sorted_max];
    sorted_length = 0;
    itemsList  = new KFileInfoList;
    reversed   = false;        // defaults
    mySortMode = Increasing;
    mySorting  = sorting; 
    keepDirsFirst = true; 

    useSingleClick = use;

    nameList = 0;

    // don't use IconLoader to always get the same icon,
    // it looks very strange, if the icons differ from application
    // to application.
    if (!folder_pixmap) 
        folder_pixmap = new QPixmap(KApplication::kde_icondir() +
                                    "/mini/folder.xpm"); 
    if (!locked_folder)
        locked_folder = new QPixmap(KApplication::kde_icondir() +
                                    "/mini/lockedfolder.xpm");
    if (!file_pixmap)
	file_pixmap = new QPixmap(KApplication::kde_icondir() +
				  "/mini/unknown.xpm");
    if (!locked_file)
	locked_file = new QPixmap(KApplication::kde_icondir() +
				  "/mini/locked.xpm");

    sig = new KFileInfoContentsSignaler();
    filesNumber = 0;
    dirsNumber = 0;
}

KFileInfoContents::~KFileInfoContents()
{
    delete [] sortedArray;
    delete itemsList;
    delete nameList;
}


bool KFileInfoContents::addItem(const KFileInfo *i) 
{
    if (!acceptsFiles() && i->isFile())
	return false;
    if (!acceptsDirs() && i->isDir())
	return false;

    if (i->isDir())
	dirsNumber++;
    else
	filesNumber++;

    itemsList->append(i);

    return addItemInternal(i);
}

void KFileInfoContents::addItemList(const KFileInfoList *list)
{
    setAutoUpdate(false);
    
    bool repaint_needed = false;
    KFileInfoListIterator it(*list);
    for (; it.current(); ++it) {
	debugC("insert %s", it.current()->fileName());
	repaint_needed |= addItem(it.current());
    }
    setAutoUpdate(true);

    if (repaint_needed)
	repaint(true);
}

void KFileInfoContents::setSorting(QDir::SortSpec new_sort)
{
    QDir::SortSpec old_sort = static_cast<QDir::SortSpec>(sorting() & QDir::SortByMask);
    QDir::SortSpec sortflags = static_cast<QDir::SortSpec>(sorting() & (~QDir::SortByMask));
    
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

    if ( mySorting & QDir::DirsFirst )
        keepDirsFirst = true;
    else
        keepDirsFirst = false;

    setAutoUpdate(false);
    clearView();

    debugC("qsort %ld", time(0));
    QuickSort(sortedArray, 0, sorted_length - 1);
    debugC("qsort %ld", time(0));
    for (uint i = 0; i < sorted_length; i++)
      insertItem(sortedArray[i], -1);
    debugC("qsort %ld", time(0));
    setAutoUpdate(true);
    repaint(true);
}

void KFileInfoContents::clear()
{
    sorted_length = 0;
    itemsList->clear();
    delete nameList;
    nameList = 0;
    clearView();
    filesNumber = 0;
    dirsNumber = 0;
}

void KFileInfoContents::connectDirSelected( QObject *receiver, 
					    const char *member)
{
    sig->connect(sig, SIGNAL(dirActivated(KFileInfo*)), receiver, member);
}

void KFileInfoContents::connectFileHighlighted( QObject *receiver, 
					 const char *member)
{
    sig->connect(sig, SIGNAL(fileHighlighted(KFileInfo*)), receiver, member);
}

void KFileInfoContents::connectFileSelected( QObject *receiver, 
				      const char *member)
{
    sig->connect(sig, SIGNAL(fileSelected(KFileInfo*)), receiver, member);
}

// this implementation is from the jdk demo Sorting
void KFileInfoContents::QuickSort(KFileInfo* a[], int lo0, int hi0)
{
    int lo = lo0;
    int hi = hi0;
    const KFileInfo *mid;
    
    if ( hi0 > lo0)
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
				const KFileInfo *T = a[lo];
				a[lo] = a[hi];
				a[hi] = const_cast<KFileInfo*>(T);
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

int KFileInfoContents::compareItems(const KFileInfo *fi1, const KFileInfo *fi2)
{
    static int counter = 0;
    counter++;
    if (counter % 1000 == 0)
	debugC("compare %d", counter);
    
    bool bigger = true;
    
    if (keepDirsFirst && (fi1->isDir() != fi2->isDir()))
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
	bigger = (stricmp(fi1->fileName(),
			  fi2->fileName()) > 0);
      }
    }
    
    if (reversed)
      bigger = !bigger;

    // debugC("compare %s against %s: %s", fi1->fileName(), fi2->fileName(), bigger ? "bigger" : "smaller");
    
    return (bigger ? 1 : -1);
}

int KFileInfoContents::findPosition(const KFileInfo *i, int left)
{
    int pos = left;
    int right = sorted_length - 1;

    while (left < right-1) {
	pos = (left + right) / 2;
	if (compareItems(i, sortedArray[pos]) < 0) 
	    right = pos;
	else
	    left = pos;
    }
    
    // if pos is the left side (rounded), it may be, that we haven't
    // compared to the right side and gave up too early
    if (pos == left && compareItems(i, sortedArray[right]) > 0)
	pos = right+1;
    else 
	pos = right;

    if (pos == -1)
	pos = sorted_length;

    // debugC("findPosition %s %d", i->fileName(), pos);
    return pos;
}

bool KFileInfoContents::addItemInternal(const KFileInfo *i)
{
    int pos = -1;

    if ( sorted_length > 1 && mySorting != QDir::Unsorted) {
	// insertation using log(n)
	pos = findPosition(i, 0);
    } else {
	if (sorted_length == 1 && compareItems(i, sortedArray[0]) < 0)
	    pos = 0;
	else
	    pos = sorted_length;
    }
    // debugC("insertItem %s %d %d", i->fileName(), pos, sorted_length);

    insertSortedItem(i, pos); 
    return insertItem(i, pos);
} 


const char *KFileInfoContents::text(uint index) const
{
    if (index < sorted_length)
	return sortedArray[index]->fileName();
    else
	return "";
}

void KFileInfoContents::select( int index )
{
    select(sortedArray[index]);
}

void KFileInfoContents::select( KFileInfo *entry)
{
    if ( entry->isDir() ) {
	debugC("selectDir %s",entry->fileName());
	sig->activateDir(entry);
    } else {
	sig->activateFile(entry);
    }
}

void KFileInfoContents::highlight( KFileInfo *entry )
{
    sig->highlightFile(entry);
}

void KFileInfoContents::highlight( int index )
{
    highlight(sortedArray[index]);
}


void  KFileInfoContents::repaint(bool f)
{
    widget()->repaint(f);
}

void KFileInfoContents::setCurrentItem(const char *item, 
				       const KFileInfo *entry)
{
    uint i;
    if (item != 0) {
	for (i = 0; i < sorted_length; i++)
	    if (sortedArray[i]->fileName() == item) {
		highlightItem(i);
		highlight(i);
		return;
	    }
    } else
	for (i = 0; i < sorted_length; i++)
	    if (sortedArray[i] == entry) {
		highlightItem(i);
		return;
	    }
    
    warning("setCurrentItem: no match found.");
}

QString KFileInfoContents::findCompletion( const char *base, 
					   bool activateFound )
{

    if (!nameList) {
	warning("not implemented yet");
	return "";
    }

    if ( strlen(base) == 0 ) return 0;

    QString remainder = base;
    const char *name;
    for ( name = nameList->first(); name; name = nameList->next() ) {
	if ( strlen(name) < remainder.length())
	    continue;
	if (strncmp(name, remainder, remainder.length()) == 0)
	    break;
    }
    
    
    if (name) {
	
        QString body = name;
        QString backup = name;

        // get the possible text completions and store the smallest 
	// common denominator in QString body
        
        unsigned int counter = strlen(base);
        for ( const char *extra = nameList->next(); extra; 
	      extra = nameList->next() ) {
	    
            counter = strlen(base);
            // case insensitive comparison needed because items are stored insensitively
            // so next instruction stop loop when first letter does no longer match
            if ( strnicmp( extra, remainder, 1 ) != 0 )  
                break;
            // this is case sensitive again!
            // goto next item if no completion possible with current item (->extra)
            if ( strncmp(extra, remainder, remainder.length()) != 0 )
                continue;
            // get smallest common denominator
            for ( ; counter <= strlen(extra) ; counter++ ) {
                if (strncmp( extra, body, counter) != 0)
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
            if ( extra && ( reversed ? (strcmp( extra, backup ) > 0) : (strcmp( extra, backup ) < 0) ) )
              backup = extra;
        }
	name = backup;
        
	debugC("completion base (%s) name (%s) body (%s)", base, name, body.data());

	bool matchExactly = (name == body);

	if (matchExactly && (activateFound || useSingleClick))
	    { 
	      for (uint j = 0; j < sorted_length; j++)
		{
		  KFileInfo *i = sortedArray[j];
		    
		    if (i->fileName() == name) {
			if (sortedArray[j]->isDir())
			    body += "/";
			highlightItem(j);
			if (activateFound)
			    select(j);
			else
			    highlight(j);
			break;
		    }
		}
	    } else
		setCurrentItem(name); // the first matching name

	return body; 
    } else {
	debugC("no completion for %s", base);
	return 0;
    }
    
}

void KFileInfoContents::insertSortedItem(const KFileInfo *item, uint pos)
{
    //  debug("insert %s %d", item->fileName(), pos);
    if (sorted_length == sorted_max) {
	sorted_max *= 2;
	KFileInfo **newArray = new KFileInfo*[sorted_max];
	int found = 0;
	for (uint j = 0; j < sorted_length; j++) {
	    if (j == pos) {
		found = 1;
		newArray[j] = const_cast<KFileInfo*>(item);
	    }
	    newArray[j+found] = sortedArray[j];
	}
	if (!found)
	    newArray[pos] = const_cast<KFileInfo*>(item);
	
	delete [] sortedArray;
	sortedArray = newArray;
	sorted_length++;
	return;
    }
    
    // faster repositioning (very fast :)
    memmove(sortedArray + pos+1,
	    sortedArray + pos,
	    (sorted_max - 1 - pos) * sizeof(KFileInfo*));
    
    sortedArray[pos] = const_cast<KFileInfo*>(item);
    sorted_length++;
}

#include "kfileinfocontents.moc" // for the signaler
